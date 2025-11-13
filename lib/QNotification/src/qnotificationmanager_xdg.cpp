// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#include "qnotification.h"
#include "qnotification_p.h"
#include "qnotificationmanager.h"
#include <QLoggingCategory>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include "xdgnotificationsinterface.h"
Q_LOGGING_CATEGORY(LC, "QNotifications")
using namespace std;


class QNotificationManagerPrivate
{
public:
    std::map<uint, QNotification*> sent;
    org::freedesktop::Notifications server;
};


QNotificationManager::QNotificationManager():
    d(new QNotificationManagerPrivate{
        {},
        {
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("/org/freedesktop/Notifications"),
            QDBusConnection::sessionBus()
        }
    })
{
    if(!d->server.isValid())
        qCCritical(LC,).noquote() << d->server.lastError();

    QObject::connect(&d->server, &org::freedesktop::Notifications::ActionInvoked,
                     &d->server, [this](uint id, const QString &action_key)
    {
        try {
            auto notification = d->sent.at(id);
            emit notification->activated();
            notification->dismiss();
        } catch (const out_of_range&) {
            qCWarning(LC,).noquote() << "Action invoked, but not found in sent notifications.";
        }
    });
}

QNotificationManager::~QNotificationManager()
{
    d->sent.clear();
    for (auto &[id, _] : d->sent)
        d->server.CloseNotification(id);
    d->sent.clear();
}

QNotificationManager &QNotificationManager::instance()
{
    static QNotificationManager instance;
    return instance;
}

void QNotificationManager::send(QNotification &qdn)
{
    auto reply = d->server.Notify(
        QCoreApplication::applicationName(),
        qdn.d->id,  // replaces id, no need to dismiss
        QCoreApplication::applicationName(),  // icon
        qdn.title(),
        qdn.text(),
        QStringList(),
        QVariantMap(),
        0
    );

    reply.waitForFinished();

    if (reply.isError())
        qCWarning(LC,).noquote() << "Sending notification failed." << reply.error();
    else if (reply.value() != qdn.d->id)
        qCCritical(LC,).noquote() << "Server returned new id!";
    else
        d->sent.emplace(qdn.d->id, &qdn);
}

void QNotificationManager::dismiss(QNotification &qdn)
{
    auto reply = d->server.CloseNotification(qdn.d->id);

    reply.waitForFinished();

    if (reply.isError())
        qCWarning(LC,).noquote() << "Closing notification failed." << reply.error();

    d->sent.erase(qdn.d->id);
}

