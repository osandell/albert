// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QCoreApplication>
#include <albert/item.h>
#include <albert/query.h>
class VpnItemPrivate;

class VpnItem : public albert::detail::DynamicItem
{
    Q_DECLARE_TR_FUNCTIONS(VpnItem)

public:

    VpnItem(const QString &id, const QString &name);

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    std::unique_ptr<albert::Icon> icon() const override;

    enum class State {
        Invalid,
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    static QString stateString(State state);
    static QString trStateString(State state);

protected:

    State state() const;
    void setState(State state);

    const QString id_;
    const QString name_;
    State state_;

};
