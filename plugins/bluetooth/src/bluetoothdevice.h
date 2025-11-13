// Copyright (c) 2024-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <memory>
#include <albert/notification.h>
class BluetoothController;

class BluetoothDevice : public QObject
{
    Q_OBJECT
public:
    enum class State { Disconnected, Connecting, Connected, Disconnecting };

    BluetoothDevice(BluetoothController *controller,
                    const QString &id,
                    const QString &name,
                    State state);
    ~BluetoothDevice();

    inline BluetoothController &controller() const { return controller_; }
    inline const QString &id() const { return id_; }
    inline const QString &name() const { return name_; }
    inline State state() const { return state_; }
    QString stateString() const;

    virtual QString address() = 0;
    virtual uint32_t classOfDevice() = 0;
    virtual std::optional<QString> connectDevice() = 0; // async, triggers connectDeviceCallback
    virtual std::optional<QString> disconnectDevice() = 0; // async, triggers disconnectDeviceCallback

signals:

    void nameChanged(const QString &name);
    void stateChanged(State);

protected:

    void setName(const QString &name);
    void setState(State s);

    void connectDeviceCallback(std::optional<QString> error);
    void disconnectDeviceCallback(std::optional<QString> error);

private:

    void sendStateNotification(const QString &msg);

    BluetoothController &controller_;
    QString id_;
    QString name_;
    State state_;
    std::unique_ptr<albert::util::Notification> notification_;

};
