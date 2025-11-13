// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include "bluetoothdevice.h"
@class BluetoothConnectionObserver;
@class BluetoothDeviceObserver;
@class IOBluetoothDevice;
class BluetoothController;

class BluetoothDevicePrivate : public BluetoothDevice
{
public:

    BluetoothDevicePrivate(BluetoothController *controller, IOBluetoothDevice *device);
    ~BluetoothDevicePrivate();

    QString address() override;
    uint32_t classOfDevice() override;
    std::optional<QString> connectDevice() override;
    std::optional<QString> disconnectDevice() override;

    // Delegate/Observer callbacks
    void onDeviceDisconnected();
    void onConnectionComplete(std::optional<QString> error);

    __strong IOBluetoothDevice *native_device_;
    __strong BluetoothConnectionObserver *connection_observer_;
    __strong BluetoothDeviceObserver *disconnect_observer_;

    friend class BluetoothControllerPrivate;  // setstate

};
