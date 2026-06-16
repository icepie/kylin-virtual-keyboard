/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef FCITXCONTROLLERSERVICEPROXY_H
#define FCITXCONTROLLERSERVICEPROXY_H

#include <QDBusAbstractInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include "fcitxqtdbustypes.h"

class FcitxControllerServiceProxy : public QDBusAbstractInterface {
    Q_OBJECT
public:
    explicit FcitxControllerServiceProxy(QObject *parent = nullptr);
    ~FcitxControllerServiceProxy() override = default;

public:
    QDBusPendingReply<QString> CurrentInputMethod();
    QDBusPendingReply<> SetCurrentIM(const QString &imName);
};

#endif // FCITXCONTROLLERSERVICEPROXY_H
