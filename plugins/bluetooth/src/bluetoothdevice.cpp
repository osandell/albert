// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothdevice.h"
#include <albert/logging.h>
#include <albert/networkutil.h>
#include <albert/notification.h>
using enum BluetoothDevice::State;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace std;

BluetoothDevice::BluetoothDevice(BluetoothController *controller,
                                 const QString &id,
                                 const QString &name,
                                 State state) :
    controller_(*controller),
    id_(id),
    name_(name),
    state_(state)
{
}

BluetoothDevice::~BluetoothDevice() {}

void BluetoothDevice::setName(const QString &n)
{
    if (name_ != n)
    {
        name_ = n;
        DEBG << u"Bluetooth device name changed to '%1' (%2)"_s.arg(name(), address());
        emit nameChanged(name_);
    }
}

void BluetoothDevice::setState(State s)
{
    if (state_ != s)
    {
        notification_.reset();  // make sure notification does not dislay outdated information

        state_ = s;
        DEBG << u"Bluetooth device state changed to '%1' (%2 %3)"_s.arg(stateString(), name(), address());
        emit stateChanged(s);
    }
}

QString BluetoothDevice::stateString() const
{
    switch (state_) {
    case Connected: return tr("Connected");
    case Connecting: return tr("Connecting");
    case Disconnected: return tr("Disconnected");
    case Disconnecting: return tr("Disconnecting");
    }
    return {};
}

void BluetoothDevice::connectDeviceCallback(optional<QString> error)
{
    if (error)
    {
        WARN << u"Failed to connect device: '%1'. Error: '%2'"_s.arg(name(), error.value());
        setState(Disconnected);
        sendStateNotification(tr("Connecting device failed"));
    }
    else
    {
        INFO << u"Successfully connected device: '%1'"_s.arg(name());
        setState(Connected);
        sendStateNotification(tr("Connected"));
    }
}

void BluetoothDevice::disconnectDeviceCallback(optional<QString> error)
{
    if (error)
    {
        WARN << u"Failed to disconnect device: '%1'. Error: '%2'"_s.arg(name(), error.value());
        setState(Connected);
        sendStateNotification(tr("Disconnecting device failed"));
    }
    else
    {
        INFO << u"Successfully disconnected device: '%1'"_s.arg(name());
        setState(Disconnected);
        sendStateNotification(tr("Disconnected"));
    }
}

void BluetoothDevice::sendStateNotification(const QString &msg)
{
    notification_ = make_unique<Notification>(name(), msg);
    auto delete_notification = [this]{ notification_.reset(); };
    connect(notification_.get(), &Notification::activated, this, delete_notification);
    notification_->send();
}
