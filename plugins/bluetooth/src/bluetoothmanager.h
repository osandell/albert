// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <map>
#include <memory>
class BluetoothController;
class BluetoothManagerPrivate;

class BluetoothManager : public QObject
{
    Q_OBJECT

public:

    BluetoothManager();
    ~BluetoothManager();

    inline const std::map<QString, std::shared_ptr<BluetoothController>> &controllers() const { return controllers_; }

signals:

    void controllersChanged();

private:

    bool addController(const std::shared_ptr<BluetoothController> &controller);
    bool removeController(const QString &id);

    std::map<QString, std::shared_ptr<BluetoothController>> controllers_;
    std::unique_ptr<BluetoothManagerPrivate> d;

    friend class BluetoothManagerPrivate;

};
