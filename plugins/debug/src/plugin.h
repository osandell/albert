// Copyright (c) 2025 Manuel Schneider
#pragma once
#include <albert/extensionplugin.h>
#include <albert/triggerqueryhandler.h>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN
public:

    Plugin();
    ~Plugin() override;

    bool allowTriggerRemap() const override;
    QString synopsis(const QString &) const override;
    void handleTriggerQuery(albert::Query &) override;

};
