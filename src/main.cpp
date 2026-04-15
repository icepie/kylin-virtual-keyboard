/*
 * Copyright (c) KylinSoft Co., Ltd. 2022.All rights reserved.
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

#include <csignal>
#include <iostream>
#include <QByteArray>
#include <QLocale>
#include <QTranslator>

#include "commandlinehandler.h"
#include "errorhandler.h"
#include "ipc/dbusservice.h"
#include "ipc/fcitxvirtualkeyboardserviceproxy.h"
#include "log.h"
#include "messagehandler.h"
#include "qtsingleapplication/src/QtSingleApplication"
#include "utils.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/virtualkeyboardentrymanager.h"

const QString APP_ID = "kylin-virtual-keyboard";
const QString APP_VERSION = "4.20.1.0";

static void setupQtPlatformForSession() {
    if (!qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        return;
    }

    switch (getDesktopType()) {
    case DesktopType::WAYLAND:
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("wayland"));
        break;
    case DesktopType::X11:
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("xcb"));
        break;
    case DesktopType::UNKNOWN:
        break;
    }
}

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtSingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    setupQtPlatformForSession();

    QtSingleApplication app(APP_ID, argc, argv);
    QtSingleApplication::setApplicationName(APP_ID);
    QtSingleApplication::setApplicationVersion(APP_VERSION);

    // 命令行处理器
    CommandLineHandler commandHandler;
    commandHandler.process(app);

    // 检查是否应该继续执行(单实例和命令行参数检查)
    if (!commandHandler.shouldContinueExecution(app)) {
        return 0;
    }

    // 异常处理器，堆栈信息记录到:~/.log/kylin-virtual-keyboard-error.log
    ErrorHandler::init();

    SpdlogProxy::getInstance();
    KVKBD_INFO("{},---START---", APP_ID.toStdString());

    // 消息处理器，绑定接收二次运行时程序发送的消息
    MessageHandler messageHandler;
    commandHandler.bindMessageHandler(app, messageHandler);

    QTranslator translator;
    if (translator.load(QLocale::system(), "translation", "_",
                        ":/translations/translations")) {
        app.installTranslator(&translator);
    }

    FcitxVirtualKeyboardServiceProxy virtualKeyboardService;
    VirtualKeyboardManager virtualKeyboardManager([&virtualKeyboardService]() {
        virtualKeyboardService.hideVirtualKeyboard();
    });
    VirtualKeyboardEntryManager entryManager(virtualKeyboardManager,
                                             virtualKeyboardService);

    DBusService dbusService(&virtualKeyboardManager);

    return app.exec();
}
