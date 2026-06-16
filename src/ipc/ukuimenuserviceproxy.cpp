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

#include "ukuimenuserviceproxy.h"
#include <QDBusPendingCall>

const QString g_ukuiMenuServiceName = "org.ukui.menu";
const QString g_ukuiMenuServicePath = "/org/ukui/menu";
const char *g_ukuiMenuServiceInterface = "org.ukui.menu";

UkuiMenuServiceProxy::UkuiMenuServiceProxy(QObject *parent)
    : QDBusAbstractInterface(g_ukuiMenuServiceName, g_ukuiMenuServicePath,
                             g_ukuiMenuServiceInterface,
                             QDBusConnection::sessionBus(), parent) {}

void UkuiMenuServiceProxy::toggle() {
    QList<QVariant> argumentList;
    argumentList << "";
    asyncCallWithArgumentList("active", argumentList);
}