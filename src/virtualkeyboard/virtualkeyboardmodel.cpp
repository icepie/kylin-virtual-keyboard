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

#include "virtualkeyboardmodel.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include "log.h"
#include "utils.h"

VirtualKeyboardModel::VirtualKeyboardModel(QObject *parent) : QObject(parent) {
    initFcitx5Controller();
    initUkuiMenuServiceProxy();
    initDBusServiceWatcher();
}

void VirtualKeyboardModel::updateCandidateArea(
    const QVariant &candidateTextList, bool /*hasPrev*/, bool /*hasNext*/,
    int /*pageIndex*/, int globalCursorIndex) {
    emit updateCandidateArea(candidateTextList, globalCursorIndex);
}

void VirtualKeyboardModel::selectCandidate(int index) {
    virtualKeyboardBackendInterface_->asyncCall("SelectCandidate", index);
}

void VirtualKeyboardModel::setCurrentIM(const QString &imName) {
    fcitx5Controller_->SetCurrentIM(imName);
}

void VirtualKeyboardModel::processKeyEvent(int keysym, int keycode, int state,
                                           bool isRelease, int time) {
    // TODO：未来支持全局快捷键之后需要把这里的逻辑全部去掉  hantengc
    if (keycode == 133 && isRelease && state == 64) {
        KVKBD_INFO("will change ukui menu visiblity.");
        changeUkuiMenuVisiblity();
        return;
    }

    if (shouldUseDirectWaylandInput(keysym, state, isRelease) &&
        sendWaylandInput(keysym)) {
        return;
    }

    virtualKeyboardBackendInterface_->asyncCall("ProcessKeyEvent", (uint)keysym,
                                                (uint)keycode, (uint)state,
                                                isRelease, (uint)time);
}

bool VirtualKeyboardModel::shouldUseDirectWaylandInput(int keysym, int state,
                                                       bool isRelease) const {
    if (isRelease || !isWlrootsWayland() ||
        !shouldBypassFcitxForWaylandTextInput()) {
        return false;
    }

    switch (state) {
    case 0:
    case 1:
    case 2:
        break;
    default:
        return false;
    }

    return !keysymToWaylandText(keysym).isEmpty() ||
           !keysymToWaylandKeyName(keysym).isEmpty();
}

bool VirtualKeyboardModel::shouldBypassFcitxForWaylandTextInput() const {
    // 仅在纯英文键盘输入法下绕过 fcitx5，避免拼音等输入法拿不到预编辑。
    return uniqueName_.isEmpty() || uniqueName_ == QStringLiteral("keyboard-us");
}

bool VirtualKeyboardModel::sendWaylandInput(int keysym) const {
    const QString text = keysymToWaylandText(keysym);
    QStringList arguments;

    if (!text.isEmpty()) {
        arguments << text;
    } else {
        const QString keyName = keysymToWaylandKeyName(keysym);
        if (keyName.isEmpty()) {
            return false;
        }
        arguments << QStringLiteral("-k") << keyName;
    }

    const int exitCode = QProcess::execute("wtype", arguments);
    if (exitCode != 0) {
        KVKBD_WARN("wtype failed with exit code:{}, keysym:{}",
                   exitCode, keysym);
        return false;
    }

    if (!text.isEmpty()) {
        KVKBD_INFO("wtype text input success:{}", text.toStdString());
    } else {
        KVKBD_INFO("wtype key input success:{}", keysymToWaylandKeyName(keysym).toStdString());
    }
    return true;
}

QString VirtualKeyboardModel::keysymToWaylandText(int keysym) {
    switch (keysym) {
    case 0x0020:
        return QStringLiteral(" ");
    case 0x000d:
        return QStringLiteral("\n");
    case 0x0009:
        return QStringLiteral("\t");
    default:
        break;
    }

    if (keysym >= 0x20 && keysym <= 0x7e) {
        return QString(QChar(keysym));
    }

    return QString();
}

QString VirtualKeyboardModel::keysymToWaylandKeyName(int keysym) {
    switch (keysym) {
    case 0xff08:
        return QStringLiteral("BackSpace");
    case 0xff09:
        return QStringLiteral("Tab");
    case 0xff0d:
        return QStringLiteral("Return");
    case 0xff1b:
        return QStringLiteral("Escape");
    case 0xff51:
        return QStringLiteral("Left");
    case 0xff52:
        return QStringLiteral("Up");
    case 0xff53:
        return QStringLiteral("Right");
    case 0xff54:
        return QStringLiteral("Down");
    case 0xffff:
        return QStringLiteral("Delete");
    default:
        return QString();
    }
}

void VirtualKeyboardModel::initFcitx5Controller() {
    registerKvkbdFcitxQtDBusTypes();
    fcitx5Controller_.reset(new FcitxControllerServiceProxy(this));
}

void VirtualKeyboardModel::initUkuiMenuServiceProxy() {
    ukuiMenuServiceProxy_.reset(new UkuiMenuServiceProxy(this));
}

void VirtualKeyboardModel::initDBusServiceWatcher() {
    serviceWatcher_.reset(new QDBusServiceWatcher(this));
    serviceWatcher_->setConnection(QDBusConnection::sessionBus());
    serviceWatcher_->addWatchedService(virtualKeyboardBackendService);
    serviceWatcher_->setWatchMode(QDBusServiceWatcher::WatchForRegistration |
                                  QDBusServiceWatcher::WatchForUnregistration);
    connect(serviceWatcher_.get(), SIGNAL(serviceRegistered(const QString &)),
            this, SLOT(backendServiceRegistered(const QString &)));
    connect(serviceWatcher_.get(), SIGNAL(serviceUnregistered(const QString &)),
            this, SLOT(backendServiceUnregistered(const QString &)));
}

void VirtualKeyboardModel::initVirtualKeyboardBackendInterface() {
    virtualKeyboardBackendInterface_.reset(new QDBusInterface(
        virtualKeyboardBackendService, virtualKeyboardBackendServicePath,
        virtualKeyboardBackendServiceInterface, QDBusConnection::sessionBus(),
        this));
}

void VirtualKeyboardModel::backendServiceRegistered(
    const QString &serviceName) {
    if (serviceName != virtualKeyboardBackendService) {
        return;
    }
    initVirtualKeyboardBackendInterface();

    syncCurrentIMList();
    syncUniqueName();
}

void VirtualKeyboardModel::backendServiceUnregistered(
    const QString &serviceName) {
    if (serviceName != virtualKeyboardBackendService) {
        return;
    }

    emit backendConnectionDisconnected();

    virtualKeyboardBackendInterface_.reset();
}

QString VirtualKeyboardModel::getUniqueName() const { return uniqueName_; }

void VirtualKeyboardModel::setUniqueName(const QString &uniqueName) {
    if (uniqueName_ == uniqueName) {
        return;
    }

    uniqueName_ = uniqueName;

    emit uniqueNameChanged();
}

void VirtualKeyboardModel::syncUniqueName() {
    QDBusPendingReply<QString> reply = fcitx5Controller_->CurrentInputMethod();
    reply.waitForFinished();
    if (!reply.isValid()) {
        KVKBD_WARN("reply error:{}", reply.error().message().toStdString());
        return;
    }
    setUniqueName(reply.value());
}

QVariant VirtualKeyboardModel::getCurrentIMList() const {
    return currentIMList_;
}

void VirtualKeyboardModel::setCurrentIMList(
    const QVariant &currentInputMethodList) {
    if (currentIMList_ == currentInputMethodList) {
        return;
    }

    currentIMList_ = currentInputMethodList;

    emit currentIMListChanged();
}

int VirtualKeyboardModel::getPreeditCaret() const { return preeditCaret_; }

void VirtualKeyboardModel::setPreeditCaret(int preeditCaret) {
    preeditCaret_ = preeditCaret;

    emit preeditCaretChanged();
}

QString VirtualKeyboardModel::getPreeditText() const { return preeditText_; }

void VirtualKeyboardModel::setPreeditText(const QString &preeditText) {
    preeditText_ = preeditText;

    emit preeditTextChanged();
}

void VirtualKeyboardModel::syncCurrentIMList() {
    auto reply = fcitx5Controller_->FullInputMethodGroupInfo("");
    reply.waitForFinished();
    if (!reply.isValid()) {
        KVKBD_WARN("reply error:{}", reply.error().message().toStdString());
        return;
    }

    QStringList stringList;
    auto inputMethodEntryList = reply.argumentAt<4>();
    for (const auto &inputMethodEntry : inputMethodEntryList) {
        stringList.append(
            inputMethodEntry.uniqueName() + "|" + inputMethodEntry.name() +
            "|" + inputMethodEntry.label() + "|" + inputMethodEntry.icon());
    }

    setCurrentIMList(QVariant(stringList));
}

void VirtualKeyboardModel::changeUkuiMenuVisiblity() {
    if (ukuiMenuServiceProxy_ == nullptr) {
        KVKBD_INFO("ukui menu service proxy is nullptr,will return and not to "
                   "show ukui menu.");
        return;
    }

    ukuiMenuServiceProxy_->toggle();
}
