// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include "vpnitem.h"
#include <QApplication>
#include <SystemConfiguration/SystemConfiguration.h>
#include <albert/albert.h>
#include <albert/logging.h>
#include <albert/messagebox.h>
ALBERT_LOGGING_CATEGORY("vpn")
using enum VpnItem::State;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

class VpnItemMac : public VpnItem
{
    SCNetworkConnectionRef connection;
    SCNetworkConnectionContext context{0, NULL, NULL, NULL, NULL};

    static VpnItem::State toState(SCNetworkConnectionStatus status)
    {
        switch (status) {
        case kSCNetworkConnectionInvalid:
            return Invalid;
        case kSCNetworkConnectionDisconnected:
            return Disconnected;
        case kSCNetworkConnectionConnecting:
            return Connecting;
        case kSCNetworkConnectionConnected:
            return Connected;
        case kSCNetworkConnectionDisconnecting:
            return Disconnecting;
        }
        return Invalid;
    }

    static void connectionCallback(SCNetworkConnectionRef /*connection*/,
                                   SCNetworkConnectionStatus status, void *info)
    { static_cast<VpnItemMac *>(info)->setState(toState(status)); }

public:

    VpnItemMac(const QString &id, const QString &name):
        VpnItem(id, name)
    {
        context.info = this;
        connection = SCNetworkConnectionCreateWithServiceID(NULL,
                                                            id_.toCFString(),
                                                            connectionCallback,
                                                            &context);

        if (!connection)
            throw runtime_error("SCNetworkConnectionCreateWithServiceID failed!");

        CFRunLoopRef runLoop = CFRunLoopGetCurrent();
        if (!SCNetworkConnectionScheduleWithRunLoop(connection, runLoop, kCFRunLoopCommonModes))
            throw runtime_error("Failed to schedule SCNetworkConnection with CFRunLoop!");

        // Set initial state
        setState(toState(SCNetworkConnectionGetStatus(connection)));
    }

    ~VpnItemMac() { CFRelease(connection); }

    void setConnected(bool connect) const
    {
        if (connect)
        {
            if (SCNetworkConnectionStart(connection, NULL, TRUE)) // stay open even on app exit
                INFO << "Successfully started connecting:" << name_;
            else
            {
                const auto *msg = QT_TRANSLATE_NOOP("VpnItem", "Failed connecting '%1': %2.");
                const auto *err = SCErrorString(SCError());
                WARN << QString::fromUtf8(msg).arg(name_, err);
                warning(Plugin::tr(msg).arg(name_, err));
            }
        }
        else
        {
            if (SCNetworkConnectionStop(connection, TRUE)) // force stop
                INFO << u"Successfully stopped connection: %1"_s.arg(name_);
            else
            {
                const auto *msg = QT_TRANSLATE_NOOP("VpnItem", "Failed disconnecting '%1': %2.");
                const auto *err = SCErrorString(SCError());
                WARN << QString::fromUtf8(msg).arg(name_, err);
                warning(Plugin::tr(msg).arg(name_, err));
            }
        }
    }

    vector<Action> actions() const
    {
        switch (state()) {
        case Connected:
            return {{u"disconnect"_s, VpnItem::tr("Disconnect"), [this] { setConnected(false); }}};
        case Disconnected:
            return {{u"connect"_s, VpnItem::tr("Connect"), [this] { setConnected(true); }}};
        default:
            return {};
        }
    }
};


class Plugin::Private{};

Plugin::Plugin() : d(make_unique<Private>()) {}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems()
{
    vector<shared_ptr<Item>> items;

    // Iterate network services in the default system preferences
    SCPreferencesRef preferences = SCPreferencesCreate(NULL, CFSTR("albert"), NULL);
    if (preferences) {

        CFArrayRef services = SCNetworkServiceCopyAll(preferences);
        if (services) {

            for (CFIndex i = 0; i < CFArrayGetCount(services); i++){
                auto service = (SCNetworkServiceRef)CFArrayGetValueAtIndex(services, i);

                // If enabled
                if (SCNetworkServiceGetEnabled(service)){

                    // If has service id
                    CFStringRef service_id = SCNetworkServiceGetServiceID(service);
                    if (service_id){

                        // If has interface
                        SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
                        if (interface){

                            // If interface type is VPN
                            CFStringRef interface_type = SCNetworkInterfaceGetInterfaceType(interface);
                            if (CFStringCompare(interface_type, CFSTR("VPN"), 0) == kCFCompareEqualTo
                                || CFStringCompare(interface_type, CFSTR("IPSec"), 0) == kCFCompareEqualTo)
                                items.push_back(make_shared<VpnItemMac>(
                                    QString::fromCFString(service_id),
                                    QString::fromCFString(SCNetworkServiceGetName(service))
                                    ));
                            else
                                DEBG << "Skipping interface type" << interface_type << service_id;
                        }
                    }
                }
            }
            CFRelease(services);
        }
        CFRelease(preferences);
    }

    vector<IndexItem> index_items;
    for (const auto &item : items)
        index_items.emplace_back(item, item->text());

    setIndexItems(::move(index_items));
}
