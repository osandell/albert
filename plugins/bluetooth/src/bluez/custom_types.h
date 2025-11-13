// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QVariantMap>
#include <QDBusObjectPath>
#include <QDBusMetaType>

using namespace Qt::StringLiterals;

static const auto bluez_service = u"org.bluez"_s;
static const auto bluez_object_manager_path = u"/"_s;

using NestedVariantMap = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(NestedVariantMap)

using ManagedObjects = QMap<QDBusObjectPath, NestedVariantMap>;
Q_DECLARE_METATYPE(ManagedObjects)

inline void registerBluezTypes() {
    qDBusRegisterMetaType<NestedVariantMap>();
    qDBusRegisterMetaType<ManagedObjects>();
}

class OrgFreedesktopDBusPropertiesInterface;
class OrgFreedesktopDBusObjectManagerInterface;
class OrgBluezAdapter1Interface;
class OrgBluezDevice1Interface;

using IProperties    = OrgFreedesktopDBusPropertiesInterface;
using IObjectManager = OrgFreedesktopDBusObjectManagerInterface;
using IAdapter       = OrgBluezAdapter1Interface;
using IDevice1       = OrgBluezDevice1Interface;

