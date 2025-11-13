// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include "bluez.h"
class QString;
class BluetoothManager;

class BluetoothManagerPrivate
{
public:

    BluetoothManagerPrivate(BluetoothManager *);

    void onObjectInterfaceAdded(const QString object_path, const QStringList &interfaces);
    void onObjectInterfaceRemoved(const QString object_path, const QStringList &interfaces);

    BluetoothManager *q;
    OrgFreedesktopDBusObjectManagerInterface object_manager;
};
