// Copyright (c) 2023-2025 Manuel Schneider

#include "plugin.h"
#include "vpnitem.h"
#include "nm.h"
#include <QDBusObjectPath>
#include <albert/item.h>
#include <albert/logging.h>
#include <map>
ALBERT_LOGGING_CATEGORY("vpn")
using enum VpnItem::State;
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

static const auto service              = u"org.freedesktop.NetworkManager"_s;
static const auto object_path_manager  = u"/org/freedesktop/NetworkManager"_s;
static const auto object_path_settings = u"/org/freedesktop/NetworkManager/Settings"_s;

static VpnItem::State toState(int status)
{
    // https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NMActiveConnectionState
    enum NM_ACTIVE_CONNECTION_STATE
    {
        NM_ACTIVE_CONNECTION_STATE_UNKNOWN = 0,
        NM_ACTIVE_CONNECTION_STATE_ACTIVATING = 1,
        NM_ACTIVE_CONNECTION_STATE_ACTIVATED = 2,
        NM_ACTIVE_CONNECTION_STATE_DEACTIVATIN = 3,
        NM_ACTIVE_CONNECTION_STATE_DEACTIVATED = 4
    };

    switch (status) {
    case NM_ACTIVE_CONNECTION_STATE_UNKNOWN: return Invalid;
    case NM_ACTIVE_CONNECTION_STATE_ACTIVATING: return Connecting;
    case NM_ACTIVE_CONNECTION_STATE_ACTIVATED: return Connected;
    case NM_ACTIVE_CONNECTION_STATE_DEACTIVATIN: return Disconnecting;
    case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED: return Disconnected;
    }
    return Invalid;
}

class VpnItemXdg : public VpnItem
{
public:

    std::unique_ptr<IActiveConnection> active_connection;

    VpnItemXdg(const QString &id, const QString &name):
        VpnItem(id, name)
    {
        setState(Disconnected);
    }

    void setConnected(bool connect) const
    {
        IManager manager(service, object_path_manager, QDBusConnection::systemBus());
        if (connect)
            manager.ActivateConnection(QDBusObjectPath(id()),
                                       QDBusObjectPath("/"),  // ignored
                                       QDBusObjectPath("/"));  // auto choice
        else if (active_connection)
            manager.DeactivateConnection(QDBusObjectPath(active_connection->path()));
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

    using VpnItem::setState; // expose protected base member
};

class Plugin::Private : public QObject
{
public:
    IManager manager;
    IProperties properties;
    ISettings settings;
    vector<shared_ptr<VpnItemXdg>> items;

    Private():
        manager(service, object_path_manager, QDBusConnection::systemBus()),
        properties(service, object_path_manager, QDBusConnection::systemBus()),
        settings(service, object_path_settings, QDBusConnection::systemBus())
    {
    }

    vector<shared_ptr<VpnItemXdg>> getVpnConnectionItems()
    {
        vector<shared_ptr<VpnItemXdg>> connection_items;
        auto reply = settings.ListConnections();
        reply.waitForFinished();
        for (auto object_path : reply.value())
        {
            auto connection = IConnection(service, object_path.path(), QDBusConnection::systemBus());
            auto r = connection.GetSettings();
            r.waitForFinished();
            const auto conn_settings = r.value();
            try
            {
                auto name = conn_settings.value(u"connection"_s).value(u"id"_s).toString();
                auto type = conn_settings.value(u"connection"_s).value(u"type"_s).toString();
                if (type == u"wireguard"_s || type == u"vpn"_s)
                    connection_items.emplace_back(make_shared<VpnItemXdg>(object_path.path(), name));
            }
            catch (...){}
        }
        return connection_items;
    }

    void handleActiveConnectionsChanged(const QList<QDBusObjectPath> &active_connection_paths)
    {
        // Removals
        for (auto &item : items)
            if (item->active_connection
                && ranges::none_of(active_connection_paths,
                                   [&](auto &o){ return o.path() == item->active_connection->path(); }))
            {
                item->active_connection.reset();
                item->setState(Disconnected);
            }

        // Additions
        for (const auto &acp : active_connection_paths)
        {
            auto ac = make_unique<IActiveConnection>(service, acp.path(), QDBusConnection::systemBus());

            auto it = ranges::find_if(items, [&](auto &i) { return i->id() == ac->connection().path(); });

            if (it != items.end() && !(*it)->active_connection) // sconn exists. aconn not yet
            {
                auto &item = *it;

                connect(ac.get(), &IActiveConnection::StateChanged,
                        this, [&item](uint state, uint /*reason*/){
                            CRIT << "IActiveConnection::StateChanged" << item->id() << VpnItem::stateString(toState(state));
                            item->setState(toState(state));
                        });

                item->setState(toState(ac->state()));
                item->active_connection = ::move(ac);
            }
        }
    }
};

Plugin::Plugin():
    d(make_unique<Private>())
{
    if (!QDBusConnection::systemBus().isConnected())
        throw runtime_error("Failed to connect to the system bus.");

    qRegisterMetaType<NestedVariantMap>("NestedVariantMap");
    qDBusRegisterMetaType<NestedVariantMap>();
    qDBusRegisterMetaType<QVariantList>();

    connect(&d->properties, &IProperties::PropertiesChanged,
            this, [this](const QString &interface, const QVariantMap &changed, const QStringList &)
    {
        if (interface == QString::fromUtf8(IManager::staticInterfaceName()))
            if (auto it = changed.find(u"ActiveConnections"_s); it != changed.end())
                d->handleActiveConnectionsChanged(qdbus_cast<QList<QDBusObjectPath>>(*it));
    });

    d->items = d->getVpnConnectionItems();  // initialize items
    d->handleActiveConnectionsChanged(d->manager.activeConnections());  // initialize states

}

Plugin::~Plugin() = default;

void Plugin::updateIndexItems()
{
    vector<IndexItem> items;
    for (const auto &item : d->items)
        items.emplace_back(item, item->text());
    setIndexItems(::move(items));
}
