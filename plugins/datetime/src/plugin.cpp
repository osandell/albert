// Copyright (c) 2023-2025 Manuel Schneider

#include "items.h"
#include "plugin.h"
#include "ui_configwidget.h"
#include <albert/matcher.h>
#include <albert/widgetsutil.h>
using namespace albert::util;
using namespace albert;
using namespace std;


Plugin::Plugin()
{
    restore_show_date_on_empty_query(settings());
}

QString Plugin::synopsis(const QString &q) const
{
    return q.isEmpty() ? QStringLiteral(u"<\u2009filter\u2009|\u2009epoch\u2009>") : QString{};
}

template<typename T>
static void addItem(vector<RankItem>& items, const Matcher &matcher)
{
    if (const auto m = matcher.match(T::trName()); m)
    {
        auto item = make_shared<T>();
        item->moveToThread(qApp->thread());
        items.emplace_back(::move(item), m);
    }
}

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> r;
    Matcher matcher(query);

    addItem<DateItem>(r, matcher);
    addItem<TimeItem>(r, matcher);
    addItem<DateTimeItem>(r, matcher);
    addItem<UtcItem>(r, matcher);

    bool isNumber;
    const ulong unixtime = query.string().toULong(&isNumber);
    if (isNumber)
        r.emplace_back(makeFromEpochItem(unixtime), 0.);

    return r;
}

vector<shared_ptr<Item>> Plugin::handleEmptyQuery()
{
    if (show_date_on_empty_query())
        return { make_shared<DateTimeItem>() };
    return {};
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget();
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    bind(ui.checkBox_emptyQuery,
         this,
         &Plugin::show_date_on_empty_query,
         &Plugin::set_show_date_on_empty_query,
         &Plugin::show_date_on_empty_query_changed);

    return w;
}
