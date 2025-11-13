// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include "qnotifications_export.h"
#include <QObject>
#include <memory>

///
/// The notification class.
/// The notification is visible as long as this object exists.
///
class QNOTIFICATIONS_EXPORT QNotification final : public QObject
{
    Q_OBJECT

    /// @copybrief title
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    /// @copybrief text
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:

    QNotification(const QString &title = {},
                  const QString &text = {},
                  QObject *parent = nullptr);
    ~QNotification();

    /// The title of the notification.
    /// @return @copybrief title
    const QString &title() const;

    /// Set the title of the notification.
    /// @param title @copybrief title
    void setTitle(const QString &title);

    /// The text of the notification.
    /// @return @copybrief text
    const QString &text() const;

    /// Set the text of the notification.
    /// @param text @copybrief text
    void setText(const QString &text);

    /// Send the notification to the notification server.
    /// This will add the notification to the notification server
    /// and present it to the user (Subject to the users settings).
    void send();

    /// Dismiss the notification.
    /// This will remove the notification from the notification server.
    void dismiss();

signals:

    /// Emitted when the title of the notification changed.
    void titleChanged();

    /// Emitted when the text of the notification changed.
    void textChanged();

    /// Emitted when the notification is activated.
    /// I.e. the user clicked on the notification.
    void activated();

private:

    class QNOTIFICATIONS_NO_EXPORT Private;
    QNOTIFICATIONS_NO_EXPORT std::unique_ptr<Private> d;
    friend class QNotificationManager;

};
