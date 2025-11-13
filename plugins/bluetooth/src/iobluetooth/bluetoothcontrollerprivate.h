// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include "bluetoothcontroller.h"
@class BluetoothControllerObserver;
@class IOBluetoothDevice;
@class IOBluetoothHostController;


class BluetoothControllerPrivate  : public BluetoothController
{
public:

    BluetoothControllerPrivate(BluetoothManager *manager, IOBluetoothHostController *controller);
    ~BluetoothControllerPrivate();

    QString address() override;
    std::optional<QString> powerOn() override;
    std::optional<QString> powerOff() override;

    __strong IOBluetoothHostController *controller_;
    __strong BluetoothControllerObserver* observer_;

    // Callbacks for observer
    void onPoweredChanged(bool);
    void onDeviceConnected(IOBluetoothDevice*);
    void onDeviceDisconnected(IOBluetoothDevice*);
};
