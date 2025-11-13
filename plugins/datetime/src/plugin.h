// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/property.h>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();

    QWidget *buildConfigWidget() override;
    QString synopsis(const QString &) const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;
    std::vector<std::shared_ptr<albert::Item>> handleEmptyQuery() override;

    ALBERT_PLUGIN_PROPERTY(bool, show_date_on_empty_query, false)

};
