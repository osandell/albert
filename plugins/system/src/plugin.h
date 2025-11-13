// Copyright (c) 2017-2025 Manuel Schneider

#pragma once
#include <QStringList>
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <albert/notification.h>
#include <albert/property.h>
#include <array>


enum class SupportedCommands {
    LOCK,
    LOGOUT,
    SUSPEND,
#if not defined(Q_OS_MAC)
    HIBERNATE,
#endif
    REBOOT,
    POWEROFF
};

struct Command
{
    SupportedCommands id;
    QString config_key_enabled;
    QString config_key_title;
    QString config_key_command;
    QString icon_name;
    QString default_title;
    QString description;
    QString command;
};


class Plugin : public albert::util::ExtensionPlugin,
               public albert::util::IndexQueryHandler
{
    ALBERT_PLUGIN

public:

    Plugin();

    void updateIndexItems() override;
    QWidget* buildConfigWidget() override;

    const std::array<Command, (int)SupportedCommands::POWEROFF + 1> commands;

};
