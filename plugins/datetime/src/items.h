// Copyright (c) 2023-2025 Manuel Schneider

#pragma once
#include <QObject>
#include <albert/item.h>
#include <memory>
#include <set>

class DateTimeItemBase : public QObject, public albert::Item
{
    Q_OBJECT
public:
    DateTimeItemBase(const QString &id, const QString &text, const QString &subtext);

    void addObserver(albert::Item::Observer*) override;
    void removeObserver(albert::Item::Observer*) override;

    std::set<albert::Item::Observer*> observers;

    QString id() const override;
    QString text() const override;
    QString subtext() const override;
    QString inputActionText() const override;
    std::unique_ptr<albert::Icon> icon() const override;
    std::vector<albert::Action> actions() const override;
    void timerEvent(QTimerEvent*) override final;

    virtual QString makeText() = 0;

    QString id_;
    QString text_;
    QString subtext_;
};

class DateItem : public DateTimeItemBase
{
public:
    DateItem();
    QString makeText() override final;
    static QString trName();
};

class TimeItem : public DateTimeItemBase
{
public:
    TimeItem();
    QString makeText() override final;
    static QString trName();
};

class DateTimeItem : public DateTimeItemBase
{
public:
    DateTimeItem();
    QString makeText() override final;
    static QString trName();
};

class UtcItem : public DateTimeItemBase
{
public:
    UtcItem();
    QString makeText() override final;
    static QString trName();
};

class EpochItem : public DateTimeItemBase
{
public:
    EpochItem();
    QString makeText() override final;
    static QString trName();
};

std::shared_ptr<albert::Item> makeFromEpochItem(ulong epoch);
