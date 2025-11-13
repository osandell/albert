// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include "bluetoothmanager.h"
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <vector>

class Plugin : public albert::util::ExtensionPlugin,
               public albert::util::IndexQueryHandler
{
    ALBERT_PLUGIN

public:

    QString defaultTrigger() const override;
    void updateIndexItems() override;

private:

    void reconnect();

    BluetoothManager bt_mgr;
    std::vector<QMetaObject::Connection> connections;

};
