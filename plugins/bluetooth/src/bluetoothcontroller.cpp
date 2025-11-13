// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothdevice.h"
#include <albert/logging.h>
using namespace Qt::StringLiterals;
using enum BluetoothController::State;
using namespace std;

BluetoothController::BluetoothController(BluetoothManager *manager,
                                         const QString &id,
                                         const QString &name,
                                         State state) :
    manager_(*manager),
    id_(id),
    name_(::move(name)),
    state_(state)
{}

BluetoothController::~BluetoothController() {}

void BluetoothController::setName(const QString &name)
{
    if (name_ != name)
    {
        name_ = name;
        DEBG << u"Bluetooth controller name changed to '%1' (%2)"_s.arg(name_, address());
        emit nameChanged(name_);
    }
}

void BluetoothController::setState(State s)
{
    if (state_ != s)
    {
        state_ = s;
        DEBG << u"Bluetooth controller state changed to '%1' ('%2' %3)"_s.arg(stateString(), name_, address());
        emit stateChanged(s);
    }
}

QString BluetoothController::stateString() const
{
    switch (state_) {
    case PoweredOff: return tr("Powered off");
    case PoweringOn: return tr("Powering on");
    case PoweredOn: return tr("Powered on");
    case PoweringOff: return tr("Powering off");
    }
    return {};
}

bool BluetoothController::addDevice(const shared_ptr<BluetoothDevice> &dev)
{
    const auto &[it, success] = devices_.emplace(dev->id(), dev);
    if (success)
    {
        DEBG << u"Device added: '%1' (%2)"_s.arg(dev->name(), dev->id());
        emit devicesChanged();
    }
    else
        DEBG << u"Device already exists: '%1' (%2)"_s.arg(dev->name(), dev->id());

    return success;
}

bool BluetoothController::removeDevice(const QString &id)
{
    const auto it = devices_.find(id);
    if (it != devices_.end())
    {
        DEBG << u"Device removed: '%1' (%2)"_s.arg(it->second->name(), id);
        devices_.erase(it);
        emit devicesChanged();
        return true;
    }
    else
        DEBG << u"Device to remove not found: %1"_s.arg(id);
    return it != devices_.end();
}
