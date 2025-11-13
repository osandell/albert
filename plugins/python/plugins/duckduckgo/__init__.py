# -*- coding: utf-8 -*-
# Copyright (c) 2024 Manuel Schneider

"""
Inline DuckDuckGo web search using the 'duckduckgo-search' library.
"""

from albert import *
from pathlib import Path
from duckduckgo_search import DDGS
from itertools import islice
from time import sleep

md_iid = "4.0"
md_version = "2.1.1"
md_name = 'DuckDuckGo'
md_description = 'Inline DuckDuckGo web search'
md_license = "MIT"
md_url = 'https://github.com/albertlauncher/albert-plugin-python-duckduckgo'
md_lib_dependencies = ["duckduckgo-search"]
md_authors = ["@ManuelSchneid3r"]
md_maintainers = ["@ManuelSchneid3r"]


class Plugin(PluginInstance, TriggerQueryHandler):

    icon_path = Path(__file__).parent / "duckduckgo.svg"

    def __init__(self):
        PluginInstance.__init__(self)
        TriggerQueryHandler.__init__(self)
        self.ddg = DDGS()

    def defaultTrigger(self):
        return "ddg "

    def handleTriggerQuery(self, query):

        stripped = query.string.strip()
        if stripped:

            # dont flood
            for _ in range(25):
                sleep(0.01)
                if not query.isValid:
                    return

            for r in islice(self.ddg.text(stripped, safesearch='off'), 10):
                query.add(
                    StandardItem(
                        id=self.id(),
                        text=r['title'],
                        subtext=r['body'],
                        icon_factory=lambda: makeImageIcon(Plugin.icon_path),
                        actions=[Action("open", "Open link", lambda u=r['href']: openUrl(u))]
                    )
                )
