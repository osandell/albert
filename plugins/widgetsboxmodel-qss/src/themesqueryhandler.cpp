// Copyright (c) 2022-2025 Manuel Schneider

#include "themesqueryhandler.h"
#include "window.h"
#include <albert/iconutil.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

ThemesQueryHandler::ThemesQueryHandler(Window *w) : window(w) {}

QString ThemesQueryHandler::id() const
{ return QStringLiteral("widgetboxmodel_themes"); }

QString ThemesQueryHandler::name() const
{ return QStringLiteral("Themes"); }

QString ThemesQueryHandler::description() const
{ return Window::tr("Switch themes"); }

QString ThemesQueryHandler::defaultTrigger() const { return u"theme "_s; }

void ThemesQueryHandler::handleTriggerQuery(Query &query)
{
    vector<shared_ptr<Item>> items;
    Matcher matcher(query);
    for (const auto &[name, path] : window->themes)
        if (auto m = matcher.match(name); m)
        {
            vector<Action> actions;

            actions.emplace_back(u"setlight"_s,
                                 Window::tr("Use in light mode"),
                                 [=, this]{ window->setLightTheme(name); },
                                 false);

            actions.emplace_back(u"setdark"_s,
                                 Window::tr("Use in dark mode"),
                                 [=, this]{ window->setDarkTheme(name); },
                                 false);

            if (window->darkMode())
                std::swap(actions[0], actions[1]);

            actions.emplace_back(u"open"_s, Window::tr("Open theme file"), [p = path] { open(p); });

            items.emplace_back(StandardItem::make(u"theme_%1"_s.arg(name),
                                                  name,
                                                  path,
                                                  []{ return makeGraphemeIcon(u"🎨"_s); },
                                                  ::move(actions)));
        }

    query.add(items);
}
