// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothcontrollerprivate.h"
#include "bluetoothmanager.h"
#include "bluetoothmanagerprivate.h"
#include <IOBluetooth/IOBluetooth.h>
using namespace std;


BluetoothManager::BluetoothManager()
{
    // On macos there is only one active host controller
    addController(make_unique<BluetoothControllerPrivate>(this, [IOBluetoothHostController defaultController]));
}

BluetoothManager::~BluetoothManager()
{

}
