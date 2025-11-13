// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothmanager.h"
#include <albert/logging.h>
using namespace Qt::StringLiterals;
using namespace std;

bool BluetoothManager::addController(const shared_ptr<BluetoothController> &ctlr)
{
    const auto &[it, success] = controllers_.emplace(ctlr->id(), ctlr);
    if (success)
    {
        DEBG << u"Controller added: '%1' (%2)"_s.arg(ctlr->name(), ctlr->id());
        emit controllersChanged();
    }
    else
        DEBG << u"Controller already exists: '%1' (%2)"_s.arg(ctlr->name(), ctlr->id());

    return success;
}

bool BluetoothManager::removeController(const QString &id)
{
    const auto it = controllers_.find(id);
    if (it != controllers_.end())
    {
        DEBG << u"Controller removed: '%1' (%2)"_s.arg(it->second->name(), id);
        controllers_.erase(it);
        emit controllersChanged();
    }
    else
        DEBG << u"Controller to remove not found: %1"_s.arg(id);

    return it != controllers_.end();  // success
}
