// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <map>
class BluetoothManager;
class BluetoothDevice;

class BluetoothController : public QObject
{
    Q_OBJECT
    class Private;

public:

    enum class State { PoweredOff, PoweringOn, PoweredOn, PoweringOff };

    BluetoothController(BluetoothManager *manager, const QString &id, const QString &name, State state);
    ~BluetoothController();

    inline const QString &id() const { return id_; }
    inline const QString &name() const { return name_; }
    inline State state() const { return state_; }
    QString stateString() const;

    inline const std::map<QString, std::shared_ptr<BluetoothDevice>> &devices() const { return devices_; }

    virtual QString address() = 0;
    virtual std::optional<QString> powerOn() = 0;
    virtual std::optional<QString> powerOff() = 0;

signals:

    void nameChanged(const QString &);
    void stateChanged(State);
    void devicesChanged();

protected:

    void setAddress(const QString &address);
    void setName(const QString &name);
    void setState(State s);

    bool addDevice(const std::shared_ptr<BluetoothDevice> &device);
    bool removeDevice(const QString &id);

private:

    BluetoothManager &manager_;
    QString id_;
    QString name_;
    State state_;
    std::map<QString, std::shared_ptr<BluetoothDevice>> devices_;

    friend class BluetoothManager;
    friend class BluetoothManagerPrivate;
    friend class BluetoothControllerPrivate;

};
