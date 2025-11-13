// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothcontrollerprivate.h"
#include "bluetoothdevice.h"
#include "bluetoothdeviceprivate.h"
#include "bluetoothmanager.h"
#include "bluetoothmanagerprivate.h"
#include "custom_types.h"
#include <QDBusConnection>
#include <albert/logging.h>
#include <memory>
#include <ranges>
using namespace std;

BluetoothManager::BluetoothManager() : d(make_unique<BluetoothManagerPrivate>(this)) {}

BluetoothManager::~BluetoothManager() {}

BluetoothManagerPrivate::BluetoothManagerPrivate(BluetoothManager *manager)
    : q(manager), object_manager(bluez_service, bluez_object_manager_path, QDBusConnection::systemBus())
{
    registerBluezTypes();

    // Device tracking

    QObject::connect(&object_manager, &IObjectManager::InterfacesAdded,
                     q, [this](const QDBusObjectPath &object_path, NestedVariantMap interfaces)
                     { onObjectInterfaceAdded(object_path.path(), interfaces.keys()); });

    QObject::connect(&object_manager, &IObjectManager::InterfacesRemoved,
                     q, [this](const QDBusObjectPath &object_path, const QStringList &interfaces)
                     { onObjectInterfaceRemoved(object_path.path(), interfaces); });


    // Initial population

    auto reply = object_manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError())
        throw runtime_error("Call to GetManagedObjects failed");
    auto managed_objects = reply.value();

    for (const auto &[object_path, interfaces] : managed_objects.asKeyValueRange())
        onObjectInterfaceAdded(object_path.path(), interfaces.keys());
}

void BluetoothManagerPrivate::onObjectInterfaceAdded(const QString object_path,
                                                     const QStringList &interfaces)
{
    for (const auto &interface : interfaces)
        if (interface == QString::fromLatin1(IAdapter::staticInterfaceName()))
        {
            OrgBluezAdapter1Interface adapter(bluez_service, object_path, QDBusConnection::systemBus());;
            q->addController(make_shared<BluetoothControllerPrivate>(q, adapter));
        }
        else if (interface == QString::fromLatin1(IDevice1::staticInterfaceName()))
        {
            const auto device = IDevice1(bluez_service, object_path, QDBusConnection::systemBus());
            const auto adapter_path = device.adapter().path();
            if (auto it = q->controllers_.find(adapter_path); it != q->controllers_.end())
                it->second->addDevice(make_shared<BluetoothDevicePrivate>(it->second.get(), device));
            else
                CRIT << "No controller found for added device" << object_path;
        }
}

void BluetoothManagerPrivate::onObjectInterfaceRemoved(const QString object_path, const QStringList &interfaces)
{
    for (const auto &interface : interfaces)
        if (interface == QString::fromLatin1(IAdapter::staticInterfaceName()))
            q->removeController(object_path);
        else if (interface == QString::fromLatin1(IDevice1::staticInterfaceName()))
        {
            for (const auto &controller : q->controllers() | views::values)
                if (auto it = controller->devices().find(object_path);
                    it != controller->devices().end())
                {
                    controller->removeDevice(object_path);
                    goto success;
                }
            CRIT << "No device found for removed device id " << object_path;
            success: continue;
        }
}
