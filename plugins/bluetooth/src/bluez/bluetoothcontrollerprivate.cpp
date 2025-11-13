// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothcontrollerprivate.h"
#include "bluetoothdevice.h"
#include "bluetoothdeviceprivate.h"
#include "bluez.h"
#include <albert/logging.h>
using enum BluetoothController::State;
using namespace std;


BluetoothControllerPrivate::BluetoothControllerPrivate(BluetoothManager *manager,
                                                       const OrgBluezAdapter1Interface &adapter) :
    BluetoothController(manager,
                        adapter.path(),
                        adapter.name(),
                        adapter.powered() ? PoweredOn : PoweredOff),
    adapter_(bluez_service, adapter.path(), QDBusConnection::systemBus()),
    properties_(bluez_service, adapter.path(), QDBusConnection::systemBus())
{
    connect(&properties_, &IProperties::PropertiesChanged,
            this, [this](const QString &interface, const QVariantMap &changed, const QStringList &)
    {
        if (interface == QString::fromUtf8(IAdapter::staticInterfaceName()))
        {
            for (const auto &[property, value] : changed.asKeyValueRange())
                if (property == u"Powered"_s)
                    setState(value.toBool() ? PoweredOn : PoweredOff);
                else if (property == u"Alias"_s)
                    setName(value.toString());
        }
    });
}

QString BluetoothControllerPrivate::address() { return adapter_.address(); }

std::optional<QString> BluetoothControllerPrivate::powerOn()
{
    setState(PoweringOn);
    adapter_.setPowered(true);
    return {};
}

std::optional<QString> BluetoothControllerPrivate::powerOff()
{
    setState(PoweringOff);
    adapter_.setPowered(false);
    return {};
}

