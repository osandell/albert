// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <memory>
class QNotification;
class QNotificationManagerPrivate;

class QNotificationManager
{
public:

    void send(QNotification &qdn);
    void dismiss(QNotification &qdn);
    static QNotificationManager &instance();

private:

    QNotificationManager();
    ~QNotificationManager();

    std::unique_ptr<QNotificationManagerPrivate> d;

};
