// Copyright (c) 2017-2025 Manuel Schneider

#include "plugin.h"
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/networkutil.h>
#include <albert/iconutil.h>
#include <albert/standarditem.h>
ALBERT_LOGGING_CATEGORY("mediaplayerremote")
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;



static inline shared_ptr<Item> makeItem(Player &player,
                                        const QString &cmd,
                                        const QString &tr_cmd,
                                        const QString &grapheme,
                                        function<void()> &&action)
{
    auto ico_fac = [p=&player, g=grapheme]{ return makeComposedIcon(p->icon(), makeGraphemeIcon(g), .9, .6); };
    return StandardItem::make(cmd, tr_cmd, player.name(), ::move(ico_fac), {{cmd, tr_cmd, ::move(action)}});
}

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    struct {
        QString play     = u"Play"_s;
        QString pause    = u"Pause"_s;
        QString next     = u"Next"_s;
        QString previous = u"Previous"_s;
    } static const strings;

    struct {
        QString play     = tr("Play");
        QString pause    = tr("Pause");
        QString next     = tr("Next");
        QString previous = tr("Previous");
    } static const tr_strings;

    Matcher matcher(query);
    vector<RankItem> results;
    shared_lock lock(players_mtx_);

    for (const auto &[_, p] : players_)
    {
        if (auto m = matcher.match(strings.next, tr_strings.next, p->name());
            m && p->canGoNext())
            results.emplace_back(makeItem(*p, strings.next, tr_strings.next,
                                          u"⏭️"_s, [=] { p->next(); }),
                                 m);

        if (auto m = matcher.match(strings.previous, tr_strings.previous, p->name());
            m && p->canGoPrevious())
            results.emplace_back(makeItem(*p, strings.previous, tr_strings.previous,
                                          u"⏮️"_s, [=] { p->previous(); }),
                                 m);

        if (p->isPlaying())
        {
            if (auto m = matcher.match(strings.pause, tr_strings.pause, p->name());
                m && p->canPause())
                results.emplace_back(makeItem(*p, strings.pause, tr_strings.pause,
                                              u"⏸️"_s, [=] { p->pause(); }),
                                     m);
        }
        else
        {
            if (auto m = matcher.match(strings.play, tr_strings.play, p->name());
                m && p->canPlay())
                results.emplace_back(makeItem(*p, strings.play, tr_strings.play,
                                              u"▶️"_s, [=] { p->play(); }),
                                     m);
        }
    }

    return results;
}
