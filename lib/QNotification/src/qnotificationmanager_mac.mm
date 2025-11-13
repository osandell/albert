// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#include "qnotification.h"
#include "qnotification_p.h"
#include "qnotificationmanager.h"
#include <UserNotifications/UserNotifications.h>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(LC, "QNotifications")
using namespace std;
#if  ! __has_feature(objc_arc)
    #error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"


@interface NotificationCenterDelegate : NSObject<NSUserNotificationCenterDelegate>
@property (nonatomic) QNotificationManagerPrivate *qdnmp;
- (instancetype)initWithQNotificationManagerPrivate:(QNotificationManagerPrivate*)qdnmp;
@end


class QNotificationManagerPrivate
{
public:
    std::map<uint, QNotification*> sent;
    NotificationCenterDelegate *delegate;

    void activated(NSUserNotification *nsn)
    {
        switch (nsn.activationType)
        {
        case NSUserNotificationActivationTypeNone:
            qCDebug(LC,).noquote() << "NSUserNotificationActivationTypeNone";
            break;
        case NSUserNotificationActivationTypeContentsClicked:
        {
            qCDebug(LC,).noquote() << QString("Notification activated (Id: %1, Title: %2)")
                                      .arg(QString::fromNSString(nsn.identifier),
                                           QString::fromNSString(nsn.title));

            auto it = sent.find(nsn.identifier.intValue);
            if (it == sent.end())
                qCCritical(LC,).noquote() << "Logic error: Notification not found in sent notifications.";
            else
            {
                auto *qdn = it->second;
                qdn->dismiss();  // invalidates the iterator
                emit qdn->activated();
            }
            break;
        }
        case NSUserNotificationActivationTypeActionButtonClicked:
            qCDebug(LC,).noquote() << "NSUserNotificationActivationTypeActionButtonClicked";
            break;
        case NSUserNotificationActivationTypeReplied:
            qCDebug(LC,).noquote() << "NSUserNotificationActivationTypeReplied";
            break;
        case NSUserNotificationActivationTypeAdditionalActionClicked:
            qCDebug(LC,).noquote() << "NSUserNotificationActivationTypeAdditionalActionClicked";
            break;
        default:
            break;
        }
    }
};


@implementation NotificationCenterDelegate

- (instancetype)initWithQNotificationManagerPrivate:(QNotificationManagerPrivate*)qdnmp
{
    self = [super init];
    _qdnmp = qdnmp;
    return self;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
    _qdnmp->activated(notification);
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center
                               shouldPresentNotification:(NSUserNotification *)notification {
  return YES;
}

@end


QNotificationManager::QNotificationManager():
    d(new QNotificationManagerPrivate)
{
    d->delegate = [[NotificationCenterDelegate alloc] initWithQNotificationManagerPrivate:d.get()];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:d->delegate];
}

QNotificationManager::~QNotificationManager()
{
    [NSUserNotificationCenter.defaultUserNotificationCenter setDelegate:nil];
    [NSUserNotificationCenter.defaultUserNotificationCenter removeAllDeliveredNotifications];
    d->sent.clear();
}

QNotificationManager &QNotificationManager::instance()
{
    static QNotificationManager instance;
    return instance;
}

void QNotificationManager::send(QNotification &qdn)
{
    // dismiss(qdn);

    auto *nsn = [[NSUserNotification alloc] init];
    nsn.title = qdn.title().toNSString();
    nsn.informativeText = qdn.text().toNSString();
    nsn.soundName = NSUserNotificationDefaultSoundName;
    nsn.hasActionButton = NO;
    nsn.identifier = [NSString stringWithFormat:@"%d", qdn.d->id];

    [NSUserNotificationCenter.defaultUserNotificationCenter deliverNotification:nsn];

    qCDebug(LC,).noquote() << QString("Notification sent (Id: %1, Title: %2)")
                              .arg(QString::fromNSString(nsn.identifier),
                                   QString::fromNSString(nsn.title));

    d->sent.emplace(qdn.d->id, &qdn);
}

void QNotificationManager::dismiss(QNotification &qdn)
{
    auto it = d->sent.find(qdn.d->id);
    if (it == d->sent.end())
        qCDebug(LC,).noquote() << QString("Notification to be dismissed has not been sent (Id: %1, Title: %2)")
                                  .arg(qdn.d->id).arg(qdn.d->title);
    else
    {
        d->sent.erase(it);

        // go through delivered notifications and delete the one with the same id
        for (NSUserNotification *nsn in NSUserNotificationCenter.defaultUserNotificationCenter.deliveredNotifications)
        {
            if ([nsn.identifier isEqualToString:[NSString stringWithFormat:@"%d", qdn.d->id]])
            {
                [NSUserNotificationCenter.defaultUserNotificationCenter removeDeliveredNotification:nsn];
                qCDebug(LC,).noquote() << QString("Delivered notification dismissed (Id: %1, Title: %2)")
                                          .arg(qdn.d->id).arg(qdn.d->title);

                return;
            }
        }
    }
}

#pragma clang diagnostic pop
