// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#include "qnotification.h"
#include "qnotification_p.h"
#include "qnotificationmanager.h"
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(LC)

static uint notification_counter = 0;

QNotification::QNotification(const QString &title, const QString &text, QObject *parent):
    QObject(parent), d(new Private{notification_counter++, title, text})
{

}

QNotification::~QNotification()
{
    dismiss();
}

const QString &QNotification::title() const
{
    return d->title;
}

void QNotification::setTitle(const QString &title)
{
    if (d->title != title)
    {
        d->title = title;
        emit titleChanged();
    }
}

const QString &QNotification::text() const
{
    return d->text;
}

void QNotification::setText(const QString &text)
{
    if (d->text != text)
    {
        d->text = text;
        emit textChanged();
    }
}

void QNotification::send()
{
    QNotificationManager::instance().send(*this);
}

void QNotification::dismiss()
{
    QNotificationManager::instance().dismiss(*this);
}
