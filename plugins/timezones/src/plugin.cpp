// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>
#include <albert/iconutil.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

QString Plugin::defaultTrigger() const { return tr("tz "); }

void Plugin::handleTriggerQuery(Query &query)
{
    vector<shared_ptr<Item>> items;

    QLocale loc;
    auto utc = QDateTime::currentDateTimeUtc();
    const auto tr_copy = tr("Copy to clipboard");
    const auto tr_copy_placeholder = tr("Copy '%1' to clipboard");

    for (auto &tz_id_barray: QTimeZone::availableTimeZoneIds())
    {
        if (!query.isValid())
            return;

        const auto tz = QTimeZone(tz_id_barray);
        const auto dt = utc.toTimeZone(tz);

        const auto tz_id = QString::fromLocal8Bit(tz_id_barray).replace(u'_', u' ');
        const auto tz_name_sf = tz.displayName(dt, QTimeZone::ShortName, loc);
        const auto tz_name_lf = tz.displayName(dt, QTimeZone::LongName, loc);
        const auto tz_name_of = tz.displayName(dt, QTimeZone::OffsetName, loc);

        if (auto m = Matcher(query).match(tz_id, tz_name_sf, tz_name_lf); m)
        {
            QStringList tz_info{tz_id, tz_name_lf, tz_name_sf, tz_name_of};
            tz_info.removeDuplicates();

            const auto sf = loc.toString(dt, QLocale::ShortFormat);
            const auto lf = loc.toString(dt, QLocale::LongFormat);

            items.emplace_back(
                StandardItem::make(tz_id,
                                   lf,
                                   tz_info.join(u", "_s),
                                   []{ return makeImageIcon(u":timezones"_s); },
                                   {{u"cl"_s, tr_copy, [=] { setClipboardText(lf); }},
                                    {u"cl"_s, tr_copy_placeholder.arg(sf), [=] { setClipboardText(sf); }}},
                                   tz_id));
        }
    }

    query.add(::move(items));
}
