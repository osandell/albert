// Copyright (c) 2024-2025 Manuel Schneider

#include "bluetoothcontroller.h"
#include "bluetoothdevice.h"
#include "items.h"
#include <QEventLoop>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/messagebox.h>
#include <albert/networkutil.h>
#include <albert/systemutil.h>
using enum BluetoothController::State;
using enum BluetoothDevice::State;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;


static void showError(const optional<QString> &err){
    if (err)
        albert::util::warning(*err);
};

// -------------------------------------------------------------------------------------------------

const QString BluetoothControllerItem::tr_bluetooth = tr("Bluetooth");

BluetoothControllerItem::BluetoothControllerItem(shared_ptr<BluetoothController> c) :
    controller(::move(c))
{
    connect(controller.get(), &BluetoothController::stateChanged,
            this, [this]{ dataChanged(); });
    connect(controller.get(), &BluetoothController::nameChanged,
            this, [this]{ dataChanged(); });
}

QString BluetoothControllerItem::id() const { return tr_bluetooth; }

QString BluetoothControllerItem::text() const { return tr_bluetooth; }

QString BluetoothControllerItem::subtext() const
{
    if (controller->name().isEmpty())
        return controller->stateString();
    else
        return u"%1  ·  %2"_s.arg(controller->stateString(), controller->name());
}

unique_ptr<albert::Icon> BluetoothControllerItem::icon() const
{
    switch (controller->state()) {
    case PoweredOff:  return makeImageIcon(u":bt-inactive"_s);
    case PoweringOn:  return makeImageIcon(u":bt-change"_s);
    case PoweredOn:   return makeImageIcon(u":bt-active"_s);
    case PoweringOff: return makeImageIcon(u":bt-change"_s);
    }
    return {};
}

QString BluetoothControllerItem::inputActionText() const { return tr_bluetooth; }

vector<Action> BluetoothControllerItem::actions() const
{
    vector<Action> actions;

    actions.emplace_back(u"pow"_s, tr("Toggle"), [this] { showError(toggle()); });

#ifdef Q_OS_MACOS
    actions.emplace_back(
        u"bluetooth"_s,
        tr("Open Bluetooth settings"),
        [] { openUrl(u"x-apple.systempreferences:com.apple.Bluetooth"_s); }
        );
#endif

    return actions;
}

optional<QString> BluetoothControllerItem::toggle() const
{
    switch (controller->state()) {
    case PoweredOn:
    case PoweringOn:
        return controller->powerOff();
    case PoweredOff:
    case PoweringOff:
        return controller->powerOn();
    }
    return {};
}


// -------------------------------------------------------------------------------------------------

inline static unique_ptr<Icon> connectionStateIcon(BluetoothDevice::State state)
{
    switch (state) {
    case Disconnected:  return makeImageIcon(u":bt-inactive"_s);
    case Connecting:    return makeImageIcon(u":bt-change"_s);
    case Connected:     return makeImageIcon(u":bt-active"_s);
    case Disconnecting: return makeImageIcon(u":bt-change"_s);
    }
    return {};
}

static unique_ptr<albert::Icon> deviceClassIcon(uint32_t class_of_device)
{
    // https://www.ampedrftech.com/datasheets/cod_definition.pdf

    switch (((class_of_device) & 0b0001111100000000) >> 8) {
    case 0x01: // Computer (desktop,notebook, PDA, organizers, .... )
        switch (((class_of_device) & 0b011111100) >> 2) {
        case 0x03:  // Laptop.
            return makeGraphemeIcon(u"💻"_s);
        case 0x04:  // Handheld PC/PDA (clam shell).
        case 0x05:  // Palm-size PC/PDA.
            return makeGraphemeIcon(u"📱"_s);
        case 0x06:  // Wearable computer (watch size).
            return makeGraphemeIcon(u"⌚"_s);
        case 0x01:  // Desktop workstation.
        case 0x02:  // Server-class computer.
        default:
            return makeGraphemeIcon(u"🖥️"_s);
        }

    case 0x02: // Phone (cellular, cordless, payphone, modem, ...)
        switch (((class_of_device) & 0b11111100) >> 2) {
        case 0x01:  // Cellular.
        case 0x03:  // Smart phone.
            return makeGraphemeIcon(u"📱"_s);
        case 0x02:  // Cordless.
        case 0x04:  // Wired modem or voice gateway.
        case 0x05:  // Common ISDN access.
        default:
            return makeGraphemeIcon(u"📞"_s);
        }

    case 0x03:  // LAN/Network Access point
        return makeGraphemeIcon(u"🛜"_s);

    case 0x04:  // Audio/Video (headset,speaker,stereo, video display, vcr…
        switch (((class_of_device) & 0b11111100) >> 2) {
        case 0b00001:  // Wearable Headset Device.
            return makeGraphemeIcon(u"🎧"_s);
        case 0b00010: // Hands-free Device.
            return makeGraphemeIcon(u"🗣️"_s);
        //case 0b00011:  // (Reserved)
        case 0b00100:  // Microphone
            return makeGraphemeIcon(u"🎤"_s);
        case 0b00101:  // Loudspeaker
            return makeGraphemeIcon(u"🔊"_s);
        case 0b00110:  // Headphones
            return makeGraphemeIcon(u"🎧"_s);
        case 0b00111:  // Portable Audio
            return makeGraphemeIcon(u"🔊"_s);
        case 0b01000:  // Car audio
            return makeComposedIcon(makeGraphemeIcon(u"🚗"_s), makeGraphemeIcon(u"🔊"_s));
        case 0b01001:  // Set-top box
            return makeGraphemeIcon(u"📺"_s);
        case 0b01010:  // HiFi Audio Device
            return makeGraphemeIcon(u"🎵"_s);
        case 0b01011:  // VCR
            return makeGraphemeIcon(u"📼"_s);
        case 0b01100:  // Video Camera
            return makeGraphemeIcon(u"📹"_s);
        case 0b01101:  // Camcorder
            return makeGraphemeIcon(u"📷"_s);
        case 0b01110:  // Video Monitor
        case 0b01111:  // Video Display and Loudspeaker
        case 0b10000:  // Video Conferencing
            return makeGraphemeIcon(u"📺"_s);
        // case 0b10001:  // (Reserved)
        case 0b10010:  // Gaming/Toy
            return makeGraphemeIcon(u"🧸"_s);
        default:
            return makeGraphemeIcon(u"🎵"_s);
        }

    case 0x05:  // Peripheral (mouse, joystick, keyboards, ...)
        switch ((class_of_device & 0b11000000) >> 6) {
        case 0x00:  // "Not a keyboard or pointing device."
            switch ((class_of_device & 0x00111100) >> 2) {
            case 0x01: // Joystick.
                return makeGraphemeIcon(u"🕹️"_s);
            case 0x02: // Gamepad.
                return makeGraphemeIcon(u"🎮"_s);
            default:
                return makeGraphemeIcon(u"❔"_s);
            }
        case 0x01:  // Keyboard.
            return makeGraphemeIcon(u"⌨️"_s);
        case 0x02:  // Pointing device.
            switch ((class_of_device & 0b00111100) >> 2) {
            case 0x05: // Digitizer tablet.
                return makeGraphemeIcon(u"✍️"_s);
            default: // Mouse.
                return makeGraphemeIcon(u"🖱️"_s);
            }
        case 0x03:  // Combo device.
            return makeComposedIcon(makeGraphemeIcon(u"⌨️"_s), makeGraphemeIcon(u"🖱️"_s));
        default:
            return makeGraphemeIcon(u"❔"_s);
        }

    case 0x06:  // Imaging (printer, scanner, camera, display, ...)
        return makeGraphemeIcon(u"🖼️"_s);

    case 0x07:  // Wearable (watch, pager, ...)
        switch (((class_of_device) & 0b11111100) >> 2) {
        case 2:  // Pager.
            return makeGraphemeIcon(u"📟"_s);
        case 3:  // Jacket.
            return makeGraphemeIcon(u"🧥"_s);
        case 4:  // Helmet.
            return makeGraphemeIcon(u"🪖"_s);
        case 5:  // Glasses.
            return makeGraphemeIcon(u"🕶️"_s);
        case 1:  // Wrist watch.
        default:
            return makeGraphemeIcon(u"⌚"_s);
        }

    case 0x08:  // Toy (robot, vehicle, ...)
        switch (((class_of_device) & 0b11111100) >> 2) {
        case 0x01:  // Robot
            return makeGraphemeIcon(u"🤖"_s);
        case 0x02:  // Vehicle
            return makeGraphemeIcon(u"🚗"_s);
        case 0x03:  // Doll / Action figure
            return makeGraphemeIcon(u"🧸"_s);
        case 0x04:  // Controller
            return makeGraphemeIcon(u"🎮"_s);
        case 0x05:  // Game
        default:
            return makeGraphemeIcon(u"🪀"_s);
        }

    case 0x09:
        // Health (heart rate sensor, blood pressure, thermometers, ...)
        switch (((class_of_device) & 0b11111100) >> 2) {
        // case 1:  // Blood Pressure Monitor
        case 2:  // Thermometer
            return makeGraphemeIcon(u"🌡️"_s);
        // case 3:  // Weighing Scale
        // case 4:  // Glucose Meter
        // case 5:  // Pulse Oximeter
        // case 6:  // Heart/Pulse Rate Monitor
        // case 7:  // Health Data Display
        default:
            return makeGraphemeIcon(u"❤️"_s);
        }

    default:
        return makeGraphemeIcon(u"❔"_s);
    }
}

BluetoothDeviceItem::BluetoothDeviceItem(shared_ptr<BluetoothDevice> dev):
    device(::move(dev))
{
    connect(device.get(), &BluetoothDevice::stateChanged,
            this, [this]{ dataChanged(); });
}

QString BluetoothDeviceItem::id() const { return device->name(); }

unique_ptr<albert::Icon> BluetoothDeviceItem::icon() const
{ return makeComposedIcon(connectionStateIcon(device->state()), deviceClassIcon(device->classOfDevice())); }

QString BluetoothDeviceItem::text() const { return device->name(); }

QString BluetoothDeviceItem::subtext() const
{
    switch (device->state()) {
    case Connecting:
    case Disconnecting:
        return device->stateString() + u'…';
    case Connected:
    case Disconnected:
        return device->stateString();
    }
    return {};
}

QString BluetoothDeviceItem::inputActionText() const { return device->name(); }

vector<Action> BluetoothDeviceItem::actions() const
{
    switch (device->state()) {
    case Disconnected:
    case Disconnecting:
        return {{u"toogle"_s, tr("Connect"), [this] { showError(toggle()); }}};
    case Connected:
    case Connecting:
        return {{u"toogle"_s, tr("Disconnect"), [this] { showError(toggle()); }}};
    }
    return {};
}

optional<QString> BluetoothDeviceItem::toggle() const
{
    switch (device->state()) {
    case Connected:
    case Connecting:
        return device->disconnectDevice();
    case Disconnected:
    case Disconnecting:
        if (device->controller().state() == PoweredOn)
            return device->connectDevice();
        else
        {
            QEventLoop loop;
            connect(&device->controller(), &BluetoothController::stateChanged, &loop, &QEventLoop::quit);
            if (auto ret = device->controller().powerOn(); ret)
                return ret;
            loop.exec();

            return device->controller().state() == PoweredOn ? device->connectDevice()
                                                             : tr("Failed to power on controller.");
        }
    }
    return {};
}
