// Copyright (c) 2024-2025 Manuel Schneider

#include "plugin.h"
#include <IOBluetooth/IOBluetooth.h>
#include <QTimer>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/notification.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
ALBERT_LOGGING_CATEGORY("bluetooth")
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
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


@interface BluetoothConnectionHandler : NSObject
- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status;
@end


class Plugin::Private
{
public:
    QString tr_bt = Plugin::tr("Bluetooth");
    __strong BluetoothConnectionHandler* delegate;
    bool fuzzy;
};

static QStringList icons(bool active)
{
    return QStringList(active ? u":bt-active"_s
                              : u":bt-inactive"_s);
}

Plugin::Plugin() : d(make_unique<Private>())
{
    d->delegate = [[BluetoothConnectionHandler alloc] init];

    // Touch once to request permissions
    IOBluetoothPreferenceGetControllerPowerState();
}

Plugin::~Plugin() = default;

QString Plugin::defaultTrigger() const { return u"bt "_s; }

bool Plugin::supportsFuzzyMatching() const { return true; }

void Plugin::setFuzzyMatching(bool val) { d->fuzzy = val; }

vector<RankItem> Plugin::handleGlobalQuery(const Query &query)
{
    vector<RankItem> r;

    bool enabled = IOBluetoothPreferenceGetControllerPowerState();

    Matcher matcher(query.string(), {.fuzzy = d->fuzzy});

    if (auto m = matcher.match(d->tr_bt))
    {
        auto desc = enabled ? tr("Enabled") : tr("Disabled");
        r.emplace_back(StandardItem::make(
            id(), d->tr_bt, desc, icons(enabled),
            {
                {
                    u"pow"_s, enabled ? tr("Disable") : tr("Enable"),
                    [=]{ IOBluetoothPreferenceSetControllerPowerState(enabled ? 0 : 1); }
                },
                {
                    u"sett"_s, tr("Open settings"),
                    [=]{ openUrl(u"x-apple.systempreferences:com.apple.Bluetooth"_s); }
                }
            }),
            m.score()
        );
    }

    if (!enabled)
        return r;

    for (IOBluetoothDevice *device : [IOBluetoothDevice pairedDevices])
        @autoreleasepool
    {
        auto device_name = QString::fromNSString(device.name);
        if (auto m = matcher.match(device_name))
        {
            auto desc = device.isConnected
                    ? tr("Connected")
                    : tr("Not connected");

            auto action = [=, this]{
                if (device.isConnected)
                {
                    auto status = [device closeConnection];
                    if (status == kIOReturnSuccess)
                        INFO <<  u"Successfully disconnected '%1'."_s
                                    .arg(device_name);
                    else
                        WARN <<  u"Failed disconnecting '%1': Status '%2'"_s
                                    .arg(device_name, status);
                }
                else
                {
                    auto status = [device openConnection:d->delegate];
                    if (status == kIOReturnSuccess)
                        INFO << u"Successfully issued CREATE_CONNECTION command for '%1'."_s
                                .arg(device_name);
                    else
                        WARN << u"Failed issuing CREATE_CONNECTION command for '%1': Status '%2'"_s
                                .arg(device_name, status);
                }
            };

            r.emplace_back(
                StandardItem::make(
                    device_name, device_name, desc, icons(device.isConnected),
                    {{
                        u"toogle"_s,
                        device.isConnected ? tr("Disconnect") : tr("Connect"),
                        action
                    }}
                ),
                m.score()
            );
        }
    }
    return r;
}


// Do not move upwards
// Interface implenmentation before plugin implementation confuses lupdate

@implementation BluetoothConnectionHandler

- (void)connectionComplete:(IOBluetoothDevice *)device status:(IOReturn)status
{
    auto name = QString::fromNSString(device.name);
    if (status == kIOReturnSuccess)
        INFO << u"Successfully connected to device: '%1'"_s.arg(name);
    else
    {
        WARN << u"Failed to connect to device: '%1', status: '%2'"_s.arg(name).arg(status);

        auto *n = new Notification(name, Plugin::tr("Connection failed"));
        QTimer::singleShot(5000, n, [n] { n->deleteLater(); });
        n->send();
    }
}

@end
