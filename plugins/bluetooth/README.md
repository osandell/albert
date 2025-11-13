# Albert plugin: Bluetooth

## Features

- Power on/off bluetooth adapters
- Connect/Disconnect paired devices

## Supported stacks

- IOBluetooth (macOS)
- BlueZ (Linux)

## Known issues

- `IOBluetoothDevice.name` does not reflect changes in the Bluetooth settings.
- `IOBluetoothHostController.nameAsString` is not set if powered off on start.
- `IOBluetoothHostController` does not provide notifications for unpaired devices.

