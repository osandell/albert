// Copyright (c) 2022-2025 Manuel Schneider

#include "plugin.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QMetaEnum>
#include <albert/iconutil.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
#include <memory>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

static int algo_count = QMetaEnum::fromType<QCryptographicHash::Algorithm>().keyCount()-1;

static shared_ptr<Item> buildItem(int algo_index, const QString& string_to_hash)
{
    auto algo_name = QString::fromLatin1(QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(algo_index));
    int algo_enum_value = QMetaEnum::fromType<QCryptographicHash::Algorithm>().value(algo_index);

    QCryptographicHash hash(static_cast<QCryptographicHash::Algorithm>(algo_enum_value));
    hash.addData(string_to_hash.toUtf8());
    auto hashString = QString::fromLatin1(hash.result().toHex());

    static const auto tr_c = Plugin::tr("Copy");
    static const auto tr_cs = Plugin::tr("Copy short form (8 char)");

    return StandardItem::make(
        algo_name,
        hashString,
        algo_name,
        []{ return makeImageIcon(u":hash"_s); },
        {
            {
                u"c"_s, tr_c,
                [hashString]{ setClipboardText(QString(hashString)); }
            },
            {
                u"cs"_s, tr_cs,
                [hashString]{ setClipboardText(QString(hashString.left(8))); }
            }
        }
    );
}

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> results;
    for (int i = 0; i < algo_count; ++i)
    {
        const auto u8_algo_name = QMetaEnum::fromType<QCryptographicHash::Algorithm>().key(i);
        const auto algo_name = QString::fromUtf8(u8_algo_name);
        auto prefix = algo_name.toLower() + u' ';

        if (query.string().size() >= prefix.size() && query.string().startsWith(prefix, Qt::CaseInsensitive)) {
            QString string_to_hash = query.string().mid(prefix.size());
            results.emplace_back(buildItem(i, string_to_hash), 1.0f);
        }
    }
    return results;
}

void Plugin::handleTriggerQuery(Query &query)
{
    for (int i = 0; i < algo_count; ++i)
        query.add(buildItem(i, query.string()));
}
