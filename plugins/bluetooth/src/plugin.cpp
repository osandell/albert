// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothdevice.h"
#include "bluetoothmanager.h"
#include "items.h"
#include "plugin.h"
#include <albert/logging.h>
#include <ranges>
ALBERT_LOGGING_CATEGORY("bluetooth")
using namespace albert::util;
using namespace std;

QString Plugin::defaultTrigger() const { return QStringLiteral("bt "); }

void Plugin::updateIndexItems()
{
    reconnect();

    vector<IndexItem> index_items;

    for (const auto &controller : bt_mgr.controllers() | views::values)
    {
        auto item = make_shared<BluetoothControllerItem>(controller);
        if (!controller->name().isEmpty())
            index_items.emplace_back(item, controller->name());
        index_items.emplace_back(item, QStringLiteral("Bluetooth"));

        for (const auto &device : controller->devices() | views::values)
            index_items.emplace_back(make_shared<BluetoothDeviceItem>(device), device->name());
    }

    setIndexItems(::move(index_items));
}

void Plugin::reconnect()
{
    for (const auto &conn : connections)
        QObject::disconnect(conn);

    connections.clear();

    connections.emplace_back(connect(&bt_mgr, &BluetoothManager::controllersChanged,
                                     this, &Plugin::updateIndexItems));

    for (auto &controller : bt_mgr.controllers() | views::values)
    {
        connections.emplace_back(connect(controller.get(), &BluetoothController::devicesChanged,
                                         this, &Plugin::updateIndexItems));

        for (const auto &device : controller->devices() | views::values)
            connections.emplace_back(connect(device.get(), &BluetoothDevice::nameChanged,
                                             this, &Plugin::updateIndexItems));
    }
}
