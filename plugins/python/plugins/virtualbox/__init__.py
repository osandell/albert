# -*- coding: utf-8 -*-
# Copyright (c) 2024 Manuel Schneider

from albert import *
from sys import platform
from contextlib import contextmanager

md_iid = "4.0"
md_version = "2.2.0"
md_name = "VirtualBox"
md_description = "Manage your VirtualBox machines"
md_license = "MIT"
md_url = "https://github.com/albertlauncher/albert-plugin-python-virtualbox"
md_readme_url = "https://github.com/albertlauncher/albert-plugin-python-virtualbox/blob/main/README.md"
md_authors = ["@ManuelSchneid3r"]


class Plugin(PluginInstance, GlobalQueryHandler):

    def __init__(self):
        PluginInstance.__init__(self)
        GlobalQueryHandler.__init__(self)

        if platform == "darwin":
            self.icon_factory = lambda: makeFileTypeIcon("/Applications/VirtualBox.app")
        elif platform == "linux":
            self.icon_factory = lambda: makeThemeIcon("virtualbox")
        else:
            raise NotImplementedError("Unhandled platform. Port me!")

        try:
            import vboxapi
        except ImportError:
            self.installSDK()
            try:
                import vboxapi
            except ImportError:
                raise ImportError(f"Failed to import vboxapi despite successful SDK installation.")


        self.vbox_manager = vboxapi.VirtualBoxManager(None, None)
        self.vbox_constants = self.vbox_manager.constants
        self.vbox = self.vbox_manager.getVirtualBox()

    def installSDK(self):
        import hashlib
        import os
        import shutil
        import zipfile
        import urllib.request
        import subprocess

        cache_location = self.cacheLocation()
        cache_location.mkdir(parents=True)

        try:
            url = "https://download.virtualbox.org/virtualbox/7.0.26/VirtualBoxSDK-7.0.26-168464.zip"
            sha_expected = "504a5a7ea468ad1d19041379c8204287c73eb18926b51d51f269269dbcde9b96"
            filename = cache_location / os.path.basename(url)

            info(f"Downloading SDK ...")
            urllib.request.urlretrieve(url, filename)

            sha256 = hashlib.sha256()
            with open(filename, "rb") as f:
                for block in iter(lambda: f.read(8192), b""):
                    sha256.update(block)
            sha_actual = sha256.hexdigest()

            if sha_actual.lower() != sha_expected.lower():
                raise ValueError("Downloaded file failed integrity check.")

            info(f"Extracting SDK ...")
            with zipfile.ZipFile(filename, 'r') as zip_ref:
                zip_ref.extractall(cache_location)

            info(f"Installing SDK ...")
            if platform == "darwin":
                VBOX_INSTALL_PATH = "/Applications/VirtualBox.app/Contents/MacOS/"
            elif platform == "linux":
                VBOX_INSTALL_PATH = "/usr/lib/virtualbox"
            else:
                raise NotImplementedError("Unhandled platform. See README.")

            # 7.2 has pyproject toml echo 'VBOX_INSTALL_PATH="{VBOX_INSTALL_PATH}" pip install "{cache_location / "sdk/installer/vboxapi"}"';
            # Okt 25 TODO
            script = f"""
                source "{self.dataLocation().parent}/python/venv/bin/activate"
                pip install setuptools
                cd "{cache_location / "sdk/installer"}"
                VBOX_INSTALL_PATH="{VBOX_INSTALL_PATH}" python vboxapisetup.py install
            """
            subprocess.run(["sh", "-c", script], check=True)

        finally:
            shutil.rmtree(cache_location)

    def defaultTrigger(self):
        return 'vbox '

    def configWidget(self):
        return [{
            'type': 'label',
            'text': f"Please read the [README.md]({md_readme_url}) for details.",
            'widget_properties': { 'textFormat': 'Qt::MarkdownText' }
        }]

    def buildItem(self, vm):
        actions = []
        state = vm.state
        if state in [self.vbox_constants.MachineState_PoweredOff, self.vbox_constants.MachineState_Aborted]:
            actions.append(Action("startvm", "Start virtual machine", lambda m=vm: self.startVm(m)))
        if state == self.vbox_constants.MachineState_Saved:
            actions.append(Action("restorevm", "Start saved virtual machine", lambda m=vm: self.startVm(m)))
            actions.append(Action("discardvm", "Discard saved state", lambda m=vm: self.discardSavedVm(m)))
        if state == self.vbox_constants.MachineState_Running:
            actions.extend([
                Action("savevm", "Save virtual machine", lambda m=vm: self.saveVm(m)),
                Action("poweroffvm", "Power off via ACPI", lambda m=vm: self.acpiPowerVm(m)),
                Action("stopvm", "Turn off virtual machine", lambda m=vm: self.stopVm(m)),
                Action("pausevm", "Pause virtual machine", lambda m=vm: self.pauseVm(m))
            ])
        if state == self.vbox_constants.MachineState_Paused:
            actions.append(Action("resumevm", "Resume virtual machine", lambda m=vm: self.resumeVm(m)))

        return StandardItem(
            id=vm.id,
            text=vm.name,
            subtext=self.vbox_manager.getEnumValueName("MachineState", vm.state),
            input_action_text=vm.name,
            icon_factory=self.icon_factory,
            actions=actions
        )

    def handleGlobalQuery(self, query):
        items = []
        matcher = Matcher(query.string)
        for vm in self.vbox_manager.getArray(self.vbox, "machines"):
            if m := matcher.match(vm.name):
                items.append(RankItem(self.buildItem(vm), m))
        return items

    def startVm(self, vm):
        try:
            info(f"Starting VM {vm.name} {type(vm)}...")
            session = self.vbox_manager.getSessionObject(self.vbox)
            progress = vm.launchVMProcess(session, 'gui', '')
            progress.waitForCompletion(-1)
        except Exception as e:
            warning(str(e))

    @contextmanager
    def sharedLockedSession(self, vm):
        session = self.vbox_manager.getSessionObject(self.vbox)
        try:
            vm.lockMachine(session, self.vbox_constants.LockType_Shared)
            yield session
        finally:
            session.unlockMachine()

    def acpiPowerVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.console.powerButton()

    def stopVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.console.powerDown()

    def saveVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.console.saveState()

    def discardSavedVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.machine.discardSavedState()

    def resumeVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.console.resume()

    def pauseVm(self, vm):
        with self.sharedLockedSession(vm) as s:
            s.console.pause()

