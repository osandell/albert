// Copyright (c) 2025 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
#include <albert/plugin/applications.h>
#include <albert/plugindependency.h>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
public:

    Plugin();

    QString defaultTrigger() const override;
    std::vector<albert::RankItem> handleGlobalQuery(const albert::Query &) override;

private:

    void openTermAt(const std::filesystem::path &loc) const;

    albert::util::StrongDependency<applications::Plugin> apps_plugin;

    struct {
        const QString cache;
        const QString cached;
        const QString config;
        const QString configd;
        const QString data;
        const QString datad;
        const QString open;
        const QString topen;
        const QString quit;
        const QString quitd;
        const QString restart;
        const QString restartd;
        const QString settings;
        const QString settingsd;
    } const strings;

};
