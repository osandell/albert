// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontrollerprivate.h"
#include "bluetoothdeviceprivate.h"
#include <IOBluetooth/IOBluetooth.h>
#include <albert/logging.h>
using enum BluetoothController::State;
using enum BluetoothDevice::State;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

extern "C" {
int IOBluetoothPreferencesAvailable();
int IOBluetoothPreferenceGetControllerPowerState();
void IOBluetoothPreferenceSetControllerPowerState(int state);
int IOBluetoothPreferenceGetDiscoverableState();
void IOBluetoothPreferenceSetDiscoverableState(int state);
}


// ---------------------------------------------------------------------------------------------------------------------

@interface BluetoothControllerObserver : NSObject
- (instancetype)initWith:(BluetoothControllerPrivate*)controller;
- (void)dealloc;
@end

@implementation BluetoothControllerObserver {
    BluetoothControllerPrivate *controller_;
    __strong IOBluetoothUserNotification *connect_notification;
}

- (instancetype)initWith:(BluetoothControllerPrivate*)controller
{
    self = [super init];
    if (self) {
        controller_ = controller;

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(bluetoothPowerOn:)
                                                     name:IOBluetoothHostControllerPoweredOnNotification
                                                   object:nil];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(bluetoothPowerOff:)
                                                     name:IOBluetoothHostControllerPoweredOffNotification
                                                   object:nil];

        connect_notification =
            [IOBluetoothDevice registerForConnectNotifications:self
                                                      selector:@selector(deviceConnected:device:)];

        if (!connect_notification)
            CRIT << "Failed to register connect notifications.";
    }
    return self;
}

- (void)dealloc
{
    [connect_notification unregister];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:IOBluetoothHostControllerPoweredOnNotification
                                                  object:nil];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:IOBluetoothHostControllerPoweredOffNotification
                                                  object:nil];
}

- (void)bluetoothPowerOn:(NSNotification *)notification { controller_->onPoweredChanged(true); }

- (void)bluetoothPowerOff:(NSNotification *)notification { controller_->onPoweredChanged(false); }

- (void)deviceConnected:(IOBluetoothUserNotification *)notification
                 device:(IOBluetoothDevice *)device
{
    controller_->onDeviceConnected(device);
}

- (void)deviceDisconnected:(IOBluetoothUserNotification *)notification
                    device:(IOBluetoothDevice *)device
{
    controller_->onDeviceDisconnected(device);
}

@end


// ---------------------------------------------------------------------------------------------------------------------

BluetoothControllerPrivate::BluetoothControllerPrivate(BluetoothManager *manager, IOBluetoothHostController *controller)
    : BluetoothController(manager,
                          QString::fromNSString(controller.addressAsString),
                          QString::fromNSString(controller.nameAsString),
                          controller.powerState == kBluetoothHCIPowerStateON ? PoweredOn : PoweredOff)
    , controller_(controller)
{
    observer_ = [[BluetoothControllerObserver alloc] initWith:this];

    for (IOBluetoothDevice *native_device : [IOBluetoothDevice pairedDevices])
        addDevice(make_shared<BluetoothDevicePrivate>(this, native_device));
}

BluetoothControllerPrivate::~BluetoothControllerPrivate() {}

QString BluetoothControllerPrivate::address() { return QString::fromNSString(controller_.addressAsString); }

optional<QString> BluetoothControllerPrivate::powerOn()
{
    setState(PoweringOn);
    IOBluetoothPreferenceSetControllerPowerState(1);
    return {};
}

optional<QString> BluetoothControllerPrivate::powerOff()
{
    setState(PoweringOff);

    // Set all connected devices to disconnecting
    for (auto& [_, device] : devices_)
        if (device->state() == Connected)
            static_pointer_cast<BluetoothDevicePrivate>(device)->setState(Disconnecting);

    IOBluetoothPreferenceSetControllerPowerState(0);
    return {};
}

void BluetoothControllerPrivate::onPoweredChanged(bool pow)
{
    setState(pow ? PoweredOn : PoweredOff);
    if (!pow)
        // Set all devices to disconnected
        for (auto& [_, device] : devices_)
            static_pointer_cast<BluetoothDevicePrivate>(device)->setState(Disconnected);
}

void BluetoothControllerPrivate::onDeviceConnected(IOBluetoothDevice *device)
{
    const auto address = QString::fromNSString(device.addressString);

    if (auto it = devices_.find(address); it == devices_.end())
        // New device, 'connected' should be always true here
        addDevice(make_shared<BluetoothDevicePrivate>(this, device));
    else
        // Device already exists
        static_pointer_cast<BluetoothDevicePrivate>(it->second)->setState(device.isConnected ? Connected : Disconnected);
}

void BluetoothControllerPrivate::onDeviceDisconnected(IOBluetoothDevice *)
{

}
