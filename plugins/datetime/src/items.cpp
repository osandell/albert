// Copyright (c) 2023-2025 Manuel Schneider

#include "items.h"
#include <QDateTime>
#include <QLocale>
#include <albert/iconutil.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

namespace{
static QString trCopy(){ return DateTimeItemBase::tr("Copy"); }
static QString trCopyPaste(){ return DateTimeItemBase::tr("Copy and paste"); }
}

DateTimeItemBase::DateTimeItemBase(const QString &id, const QString &text, const QString &subtext):
    id_(id),
    text_(text),
    subtext_(subtext)
{ startTimer(1s); }

void DateTimeItemBase::addObserver(Observer *observer) { observers.insert(observer); }

void DateTimeItemBase::removeObserver(Observer *observer) { observers.erase(observer); }

QString DateTimeItemBase::id() const { return id_; }

QString DateTimeItemBase::text() const { return text_; }

QString DateTimeItemBase::subtext() const { return subtext_; }

unique_ptr<Icon> DateTimeItemBase::icon() const { return makeImageIcon(u":datetime"_s); }

QString DateTimeItemBase::inputActionText() const { return subtext_; }

vector<Action> DateTimeItemBase::actions() const
{
    vector<Action> actions;

    actions.emplace_back(u"c"_s, trCopy(),
                         [this]{ setClipboardText(text()); });

    if (havePasteSupport())
        actions.emplace_back(u"cp"_s, trCopyPaste(),
                             [this]{ setClipboardTextAndPaste(text());});

    return actions;
}

void DateTimeItemBase::timerEvent(QTimerEvent*)
{
    if (auto t = makeText();
        text_ != t)
    {
        text_ = t;
        for (auto observer : observers)
            observer->notify(this);
    }
}

// ------------------------------------------------------------------------------------------------

DateItem::DateItem() : DateTimeItemBase(u"d"_s, makeText(), trName()) {}

QString DateItem::makeText()
{
    // return QLocale().toString(QDateTime::currentDateTime().date(), QLocale::ShortFormat);
    return QLocale().toString(QDateTime::currentDateTime().date(), QLocale::LongFormat);
}

QString DateItem::trName() { return DateTimeItemBase::tr("Date"); }

// ------------------------------------------------------------------------------------------------

TimeItem::TimeItem() : DateTimeItemBase(u"t"_s, makeText(), trName())
{}

QString TimeItem::makeText()
{
    // return QLocale().toString(QDateTime::currentDateTime().time(), QLocale::ShortFormat);
    return QDateTime::currentDateTime().time().toString(u"hh:mm:ss"_s);
}

QString TimeItem::trName() { return DateTimeItemBase::tr("Time"); }

// ------------------------------------------------------------------------------------------------

DateTimeItem::DateTimeItem() : DateTimeItemBase(u"dt"_s, makeText(), trName()) {}

QString DateTimeItem::makeText()
{
    return QLocale().toString(QDateTime::currentDateTime(), QLocale::LongFormat);
}

QString DateTimeItem::trName() { return DateTimeItemBase::tr("Date and time"); }

// ------------------------------------------------------------------------------------------------

UtcItem::UtcItem() : DateTimeItemBase(u"u"_s, makeText(), trName()) {}

QString UtcItem::makeText()
{
    return QLocale().toString(QDateTime::currentDateTimeUtc(), QLocale::ShortFormat);
}

QString UtcItem::trName() { return DateTimeItemBase::tr("UTC date and time"); }

// ------------------------------------------------------------------------------------------------

EpochItem::EpochItem() : DateTimeItemBase(u"e"_s, makeText(), trName()) {}

QString EpochItem::makeText() { return QString::number(QDateTime::currentSecsSinceEpoch()); }

QString EpochItem::trName() { return DateTimeItemBase::tr("Unix time"); }

// ------------------------------------------------------------------------------------------------

shared_ptr<Item> makeFromEpochItem(ulong epoch)
{
    const auto s = QLocale().toString(QDateTime::fromSecsSinceEpoch(epoch), QLocale::LongFormat);

    vector<Action> actions;
    actions.emplace_back(u"c"_s, trCopy(), [=]{ setClipboardText(s); });
    if (havePasteSupport())
        actions.emplace_back(u"cp"_s, trCopyPaste(),
                             [=]{ setClipboardTextAndPaste(s);});

    return StandardItem::make(
        u"u2dt"_s,
        s,
        DateTimeItemBase::tr("Date and time from unix time"),
        []{ return makeImageIcon(u":datetime"_s); },
        ::move(actions)
    );
}
