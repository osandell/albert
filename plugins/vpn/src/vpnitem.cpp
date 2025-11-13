// Copyright (c) 2023-2025 Manuel Schneider

#include "vpnitem.h"
#include <albert/iconutil.h>
#include <albert/logging.h>
using enum VpnItem::State;
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

VpnItem::VpnItem(const QString &id, const QString &name):
    id_(id), name_(name), state_(Invalid){}

QString VpnItem::id() const { return id_; }

QString VpnItem::text() const { return name_; }

QString VpnItem::subtext() const { return tr("VPN connection: %1").arg(trStateString(state_)); }

std::unique_ptr<Icon> VpnItem::icon() const
{
    switch (state_) {
    case Connecting:
    case Disconnecting:
        return makeImageIcon(u":shield-half"_s);
    case Connected:
        return makeImageIcon(u":shield-full"_s);
    default:
        return makeImageIcon(u":shield-empty"_s);
    }
}

VpnItem::State VpnItem::state() const { return state_; }

void VpnItem::setState(State state)
{
    if (state_ != state)
    {
        state_ = state;
        DEBG << "State changed:" << text() << stateString(state);
        dataChanged();
    }
}

QString VpnItem::stateString(State state)
{
    switch (state) {
    case Invalid: return u"Invalid"_s;
    case Disconnected: return u"Disconnected"_s;
    case Connecting: return u"Connecting…"_s;
    case Connected: return u"Connected"_s;
    case Disconnecting: return u"Disconnecting…"_s;
    default: qFatal("VpnItem::stateString: Invalid state %d", (int)state);
    }
}

QString VpnItem::trStateString(State state)
{
    switch (state) {
    case Invalid: return tr("Invalid");
    case Disconnected: return tr("Disconnected");
    case Connecting: return tr("Connecting…");
    case Connected: return tr("Connected");
    case Disconnecting: return tr("Disconnecting…");
    default: qFatal("VpnItem::stateString: Invalid state %d", (int)state);
    }
}

