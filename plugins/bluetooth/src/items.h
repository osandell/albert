// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <albert/item.h>
#include <memory>
class BluetoothController;
class BluetoothDevice;


class BluetoothControllerItem : public QObject, public albert::detail::DynamicItem
{
    std::shared_ptr<BluetoothController> controller;

public:

    BluetoothControllerItem(std::shared_ptr<BluetoothController>);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    std::unique_ptr<albert::Icon> icon() const override;
    QString inputActionText() const override;
    std::vector<albert::Action> actions() const override;

    std::optional<QString> toggle() const;

    static const QString tr_bluetooth;

};

class BluetoothDeviceItem : public QObject, public albert::detail::DynamicItem
{
    std::shared_ptr<BluetoothDevice> device;

public:

    BluetoothDeviceItem(std::shared_ptr<BluetoothDevice>);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    std::unique_ptr<albert::Icon> icon() const override;
    QString inputActionText() const override;
    std::vector<albert::Action> actions() const override;

    std::optional<QString> toggle() const;

};
