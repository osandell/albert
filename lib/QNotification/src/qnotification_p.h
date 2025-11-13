// SPDX-FileCopyrightText: 2024 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QString>
#include "qnotification.h"

class QNotification::Private
{
public:
    const uint id;
    QString title;
    QString text;
};
