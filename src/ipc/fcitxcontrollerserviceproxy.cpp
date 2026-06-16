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

#include "fcitxcontrollerserviceproxy.h"

const QString g_fcitxControllerServiceName = "org.fcitx.Fcitx5";
const QString g_fcitxControllerServicePath = "/controller";
const char *g_fcitxControllerServiceInterface = "org.fcitx.Fcitx.Controller1";

FcitxControllerServiceProxy::FcitxControllerServiceProxy(QObject *parent)
    : QDBusAbstractInterface(g_fcitxControllerServiceName,
                             g_fcitxControllerServicePath,
                             g_fcitxControllerServiceInterface,
                             QDBusConnection::sessionBus(), parent) {}

QDBusPendingReply<QString> FcitxControllerServiceProxy::CurrentInputMethod() {
    QList<QVariant> argumentList;
    return asyncCallWithArgumentList(QStringLiteral("CurrentInputMethod"),
                                     argumentList);
}


QDBusPendingReply<>
FcitxControllerServiceProxy::SetCurrentIM(const QString &imName) {
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(imName);
    return asyncCallWithArgumentList(QStringLiteral("SetCurrentIM"),
                                     argumentList);
}
