// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <albert/extensionplugin.h>
#include <albert/triggerqueryhandler.h>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::TriggerQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    void handleTriggerQuery(albert::Query &) override;

};
