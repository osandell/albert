# -*- coding: utf-8 -*-
# Copyright (c) 2017-2014 Manuel Schneider

from pathlib import Path

from albert import *

md_iid = "4.0"
md_version = "2.2.0"
md_name = "Python Eval"
md_description = "Evaluate Python code"
md_license = "BSD-3"
md_url = "https://github.com/albertlauncher/albert-plugin-python-python-eval"
md_authors = ["@ManuelSchneid3r"]
md_maintainers = ["@ManuelSchneid3r"]


class Plugin(PluginInstance, TriggerQueryHandler):

    icon = Path(__file__).parent / "python.svg"

    def __init__(self):
        PluginInstance.__init__(self)
        TriggerQueryHandler.__init__(self)

        self._modules = self.readConfig('modules', str)
        if self._modules:
            self._load_modules()
        else:
            self._modules = ""

    def _load_modules(self):
        for mod in [m.strip() for m in self._modules.split(',') if m.strip()]:
            try:
                globals().update(vars(__import__(mod)))
            except ImportError as ex:
                warning(f"Failed to import module '{mod}': {ex}")

    @property
    def modules(self):
        return self._modules

    @modules.setter
    def modules(self, value):
        self._modules = value
        self.writeConfig('modules', value)
        self._load_modules()

    def configWidget(self):
        return [
            {
                'type': 'lineedit',
                'property': 'modules',
                'label': 'Load modules'
            }
        ]

    def synopsis(self, query):
        return "<Python expression>"

    def defaultTrigger(self):
        return "py "

    def handleTriggerQuery(self, query):
        stripped = query.string.strip()
        if stripped:
            try:
                result = eval(stripped)
            except Exception as ex:
                result = ex

            result_str = str(result)

            query.add(StandardItem(
                id=self.id(),
                text=result_str,
                subtext=type(result).__name__,
                input_action_text=result_str,
                icon_factory=lambda: makeImageIcon(Plugin.icon),
                actions = [
                    Action("copy", "Copy result to clipboard", lambda r=result_str: setClipboardText(r)),
                    Action("exec", "Execute python code", lambda r=result_str: exec(stripped)),
                ]
            ))
