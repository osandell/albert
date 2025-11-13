// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothdevice.h"
#include "bluetoothdeviceprivate.h"
#include <IOBluetooth/IOBluetooth.h>
#include <albert/logging.h>
class BluetoothController;
using enum BluetoothDevice::State;
using namespace Qt::StringLiterals;
using namespace std;
#if  ! __has_feature(objc_arc)
#error This file must be compiled with ARC.
#endif

// ---------------------------------------------------------------------------------------------------------------------

@interface BluetoothConnectionObserver : NSObject
- (instancetype)initWith:(BluetoothDevicePrivate*)device;
- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status;
@end
@implementation BluetoothConnectionObserver {
    BluetoothDevicePrivate *device_;
}

- (instancetype)initWith:(BluetoothDevicePrivate *)device
{
    self = [super init];
    if (self) {
        device_ = device;
    }
    return self;
}

- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status
{
    device_->onConnectionComplete(
        status == kIOReturnSuccess ? nullopt : optional<QString>(QString::number(status)));
}

@end
// ---------------------------------------------------------------------------------------------------------------------

@interface BluetoothDeviceObserver : NSObject
- (instancetype)initWith:(BluetoothDevicePrivate*)device;
- (void)dealloc;
- (void)deviceDisconnected:(IOBluetoothUserNotification *)notification
                    device:(IOBluetoothDevice *)device;
@end
@implementation BluetoothDeviceObserver {
    BluetoothDevicePrivate *device_;
    __strong IOBluetoothUserNotification *disconnect_notification_;
}

- (instancetype)initWith:(BluetoothDevicePrivate*)device
{
    self = [super init];
    if (self) {
        device_ = device;
        disconnect_notification_ = [device_->native_device_
            registerForDisconnectNotification:self
                                     selector:@selector(deviceDisconnected:device:)];
        if (!disconnect_notification_)
            CRIT << "Failed to register for disconnect notifications.";
    }
    return self;
}

- (void)dealloc
{
    [disconnect_notification_ unregister];
    disconnect_notification_ = nil;
}

- (void)deviceDisconnected:(IOBluetoothUserNotification *)notification
                    device:(IOBluetoothDevice *)device
{
    device_->onDeviceDisconnected();
}

@end


// ---------------------------------------------------------------------------------------------------------------------

BluetoothDevicePrivate::BluetoothDevicePrivate(BluetoothController *controller,
                                               IOBluetoothDevice *device)
    : BluetoothDevice(controller,
                      QString::fromNSString(device.addressString),
                      QString::fromNSString(device.name),
                      device.isConnected ? Connected : Disconnected)
    , native_device_(device)
{
    connection_observer_ = [[BluetoothConnectionObserver alloc] initWith:this];
    disconnect_observer_ = [[BluetoothDeviceObserver alloc] initWith:this];
}

BluetoothDevicePrivate::~BluetoothDevicePrivate()
{
    connection_observer_ = nil;
    disconnect_observer_ = nil;
}

QString BluetoothDevicePrivate::address() { return QString::fromNSString(native_device_.addressString); }

uint32_t BluetoothDevicePrivate::classOfDevice() { return native_device_.classOfDevice; }

optional<QString> BluetoothDevicePrivate::connectDevice()
{
    if (auto status = [native_device_ openConnection:connection_observer_];  // async
        status == kIOReturnSuccess)
    {
        DEBG << u"Successfully issued CREATE_CONNECTION command for '%1'."_s.arg(name());
        setState(Connecting);
        return {};
    }
    else
    {
        const auto msg = u"Failed issuing CREATE_CONNECTION command for '%1': Status '%2'"_s.arg(name(), status);
        WARN << msg;
        return msg;
    }
}

optional<QString> BluetoothDevicePrivate::disconnectDevice()
{
    if (auto status = [native_device_ closeConnection];  // sync
        status == kIOReturnSuccess)
    {
        setState(Disconnecting);
        return {};
    }
    else
    {
        const auto msg =  u"Failed disconnecting '%1': Status '%2'"_s.arg(name(), status);
        WARN << msg;
        return msg;
    }
}

void BluetoothDevicePrivate::onDeviceDisconnected() { setState(Disconnected); }

void BluetoothDevicePrivate::onConnectionComplete(optional<QString> error) { connectDeviceCallback(error); }
