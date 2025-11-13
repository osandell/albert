// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include "bluetoothcontroller.h"
#include "bluez.h"

class BluetoothControllerPrivate : public BluetoothController
{
public:

    BluetoothControllerPrivate(BluetoothManager *manager, const OrgBluezAdapter1Interface &adapter);

    QString address() override;
    std::optional<QString> powerOn() override;
    std::optional<QString> powerOff() override;

    OrgBluezAdapter1Interface adapter_;
    OrgFreedesktopDBusPropertiesInterface properties_;

};
