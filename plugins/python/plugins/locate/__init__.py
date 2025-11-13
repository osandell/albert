# -*- coding: utf-8 -*-
# Copyright (c) 2022-2025 Manuel Schneider

import shlex
import subprocess
from pathlib import Path

from albert import (PluginInstance, TriggerQueryHandler, StandardItem, Action, openFile,
                    makeFileTypeIcon, makeGraphemeIcon, makeComposedIcon, Matcher)

md_iid = "4.0"
md_version = "3.1.1"
md_name = "Locate"
md_description = "Find files using locate"
md_license = "MIT"
md_url = "https://github.com/albertlauncher/albert-plugin-python-locate"
md_bin_dependencies = ["locate"]
md_authors = ["@ManuelSchneid3r"]



class Plugin(PluginInstance, TriggerQueryHandler):

    def __init__(self):
        PluginInstance.__init__(self)
        TriggerQueryHandler.__init__(self)

    def defaultTrigger(self):
        return "'"

    def handleTriggerQuery(self, query):
        try:
            args = shlex.split(query.string)
        except ValueError:
            return

        if args and all(len(token) > 2 for token in args):

            # Fetch results from locate and filter them using Matcher

            matcher = Matcher(query.string)
            items = []
            with subprocess.Popen(['locate', *args], stdout=subprocess.PIPE, text=True) as proc:
                for line in proc.stdout:
                    if not query.isValid:
                        return

                    path = line.strip()
                    filename = Path(path).name
                    if m := matcher.match(filename, path):
                        items.append((
                            StandardItem(
                                id=path,
                                text=filename,
                                subtext=path,
                                icon_factory=lambda: makeFileTypeIcon(path),
                                actions=[
                                    Action("open", "Open", lambda p=path: openFile(p))
                                ]
                            ),
                            float(m)
                        ))

            # Filter using the matcher

            items = sorted(items, key=lambda x: x[1], reverse=True)

            if not query.isValid:
                return

            query.add([i[0] for i in items])

        else:
            query.add(
                StandardItem(
                    id="locate.info",
                    text="Token is too short",
                    subtext="Each token must have at least three characters",
                    icon_factory=lambda: makeComposedIcon(makeGraphemeIcon("🔎"), makeGraphemeIcon("⚠️"), 1.0)
                )
            )
