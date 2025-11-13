// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <bluetoothdevice.h>
#include <QString>
#include "bluez.h"
class BluetoothController;

class BluetoothDevicePrivate : public BluetoothDevice
{
public:

    BluetoothDevicePrivate(BluetoothController *controller, const OrgBluezDevice1Interface &device);

    QString address() override;
    uint32_t classOfDevice() override;
    std::optional<QString> connectDevice() override;
    std::optional<QString> disconnectDevice() override;

    OrgBluezDevice1Interface device_;
    OrgFreedesktopDBusPropertiesInterface properties_;

};
