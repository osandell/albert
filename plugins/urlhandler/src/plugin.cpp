// Copyright (c) 2022-2025 Manuel Schneider

#include "plugin.h"
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <albert/iconutil.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

Plugin::Plugin()
{
    QFile file(u":tlds"_s);
    if (!file.open(QIODevice::ReadOnly))
        throw runtime_error("Unable to read tld resource");
    valid_tlds << QTextStream(&file).readAll().split(u'\n');
    std::sort(valid_tlds.begin(), valid_tlds.end());
}

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> results;
    auto trimmed = query.string().trimmed();
    auto url = QUrl::fromUserInput(trimmed);

    // Check syntax and TLD validity
    if (!url.isValid())
        return results;

    if ((url.scheme() == u"http"_s || url.scheme() == u"https"_s)){

        auto tld = url.host().section(u'.', -1, -1);

        // Skip tld only queries
        if (trimmed.size() == tld.size())
            return results;

        // validate top level domain if scheme is not given (http assumed)
        if (tld.size() == 0 || (!trimmed.startsWith(u"http"_s)
                                && !binary_search(valid_tlds.begin(), valid_tlds.end(), tld)))
            return results;

        results.emplace_back(
            StandardItem::make(
                u"url_hanlder"_s,
                tr("Open URL in browser"),
                tr("Open %1").arg(url.authority()),
                []{ return makeImageIcon(u":default"_s); },
                {{u"open_url"_s, tr("Open URL"), [url](){ open(url); }}}
            ),
            1.0f
        );
    }
    return results;
}
