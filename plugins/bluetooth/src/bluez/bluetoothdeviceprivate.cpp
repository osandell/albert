// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothdevice.h"
#include "bluetoothdeviceprivate.h"
#include "bluez.h"
#include <QTimer>
#include <albert/logging.h>
#include <albert/notification.h>
class BluetoothController;
using enum BluetoothDevice::State;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

BluetoothDevicePrivate::BluetoothDevicePrivate(BluetoothController *controller, const OrgBluezDevice1Interface &device):
    BluetoothDevice(controller,
                    device.path(),
                    device.name(),
                    device.connected() ? Connected : Disconnected),
    device_(bluez_service, device.path(), QDBusConnection::systemBus()),
    properties_(bluez_service, device.path(), QDBusConnection::systemBus())
{
    connect(&properties_, &IProperties::PropertiesChanged,
            this, [this](const QString &interface, const QVariantMap &changed, const QStringList &)
    {
        if (interface == QString::fromUtf8(IDevice1::staticInterfaceName()))
            for (const auto &[property, value] : changed.asKeyValueRange())
            {
                if (property == u"Connected"_s)
                    setState(value.toBool() ? Connected : Disconnected);
                else if (property == u"Alias"_s)
                    setName(value.toString());
            }
    });
}

QString BluetoothDevicePrivate::address() { return device_.address(); }

std::optional<QString> BluetoothDevicePrivate::connectDevice()
{
    setState(Connecting);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(device_.Connect(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call)
    {
        QDBusPendingReply<void> pending_reply = *call;
        connectDeviceCallback(pending_reply.isError()
                                  ? pending_reply.error().name() + pending_reply.error().message()
                                  : optional<QString>{});
        call->deleteLater();
    });
    return {};
}

std::optional<QString> BluetoothDevicePrivate::disconnectDevice()
{
    setState(Disconnecting);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(device_.Disconnect(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *call)
    {
        QDBusPendingReply<void> pending_reply = *call;
        disconnectDeviceCallback(pending_reply.isError()
                                     ? pending_reply.error().name() + pending_reply.error().message()
                                     : optional<QString>{});
        call->deleteLater();
    });
    return {};
}

uint32_t BluetoothDevicePrivate::classOfDevice() { return device_.deviceClass(); }
