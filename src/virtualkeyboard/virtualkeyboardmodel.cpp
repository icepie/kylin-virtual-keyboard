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
#include <QTimer>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QDir>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include "log.h"
#include "utils.h"

VirtualKeyboardModel::VirtualKeyboardModel(QObject *parent) : QObject(parent) {
    initFcitx5Controller();
    initUkuiMenuServiceProxy();
    initDBusServiceWatcher();

    QTimer::singleShot(0, this, &VirtualKeyboardModel::syncFcitxInputMethodState);
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
    auto reply = fcitx5Controller_->SetCurrentIM(imName);
    reply.waitForFinished();
    if (!reply.isValid()) {
        KVKBD_WARN("SetCurrentIM failed:{}", reply.error().message().toStdString());
        return;
    }

    syncUniqueName();
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

    if (!virtualKeyboardBackendInterface_ ||
        !virtualKeyboardBackendInterface_->isValid()) {
        KVKBD_WARN("virtual keyboard backend is unavailable, fallback to xdotool. keycode:{}, isRelease:{}",
                   keycode, isRelease);
        sendX11Input(keycode, isRelease);
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


bool VirtualKeyboardModel::sendX11Input(int keycode, bool isRelease) const {
    if (keycode <= 0) {
        KVKBD_WARN("unsupported x11 fallback keycode:{}", keycode);
        return false;
    }

    const QString action = isRelease ? QStringLiteral("keyup") : QStringLiteral("keydown");
    const QString keycodeArgument = QStringLiteral("keycode %1").arg(keycode);
    const int exitCode = QProcess::execute(QStringLiteral("xdotool"),
                                           QStringList() << action << keycodeArgument);
    if (exitCode != 0) {
        KVKBD_WARN("xdotool failed with exit code:{}, action:{}, keycode:{}",
                   exitCode, action.toStdString(), keycode);
        return false;
    }

    return true;
}

QString VirtualKeyboardModel::keycodeToX11KeyName(int keycode) {
    switch (keycode) {
    case 9: return QStringLiteral("Escape");
    case 10: return QStringLiteral("1");
    case 11: return QStringLiteral("2");
    case 12: return QStringLiteral("3");
    case 13: return QStringLiteral("4");
    case 14: return QStringLiteral("5");
    case 15: return QStringLiteral("6");
    case 16: return QStringLiteral("7");
    case 17: return QStringLiteral("8");
    case 18: return QStringLiteral("9");
    case 19: return QStringLiteral("0");
    case 20: return QStringLiteral("minus");
    case 21: return QStringLiteral("equal");
    case 22: return QStringLiteral("BackSpace");
    case 23: return QStringLiteral("Tab");
    case 24: return QStringLiteral("q");
    case 25: return QStringLiteral("w");
    case 26: return QStringLiteral("e");
    case 27: return QStringLiteral("r");
    case 28: return QStringLiteral("t");
    case 29: return QStringLiteral("y");
    case 30: return QStringLiteral("u");
    case 31: return QStringLiteral("i");
    case 32: return QStringLiteral("o");
    case 33: return QStringLiteral("p");
    case 34: return QStringLiteral("bracketleft");
    case 35: return QStringLiteral("bracketright");
    case 36: return QStringLiteral("Return");
    case 37: return QStringLiteral("Control_L");
    case 38: return QStringLiteral("a");
    case 39: return QStringLiteral("s");
    case 40: return QStringLiteral("d");
    case 41: return QStringLiteral("f");
    case 42: return QStringLiteral("g");
    case 43: return QStringLiteral("h");
    case 44: return QStringLiteral("j");
    case 45: return QStringLiteral("k");
    case 46: return QStringLiteral("l");
    case 47: return QStringLiteral("semicolon");
    case 48: return QStringLiteral("apostrophe");
    case 49: return QStringLiteral("grave");
    case 50: return QStringLiteral("Shift_L");
    case 51: return QStringLiteral("backslash");
    case 52: return QStringLiteral("z");
    case 53: return QStringLiteral("x");
    case 54: return QStringLiteral("c");
    case 55: return QStringLiteral("v");
    case 56: return QStringLiteral("b");
    case 57: return QStringLiteral("n");
    case 58: return QStringLiteral("m");
    case 59: return QStringLiteral("comma");
    case 60: return QStringLiteral("period");
    case 61: return QStringLiteral("slash");
    case 62: return QStringLiteral("Shift_R");
    case 64: return QStringLiteral("Alt_L");
    case 65: return QStringLiteral("space");
    case 66: return QStringLiteral("Caps_Lock");
    default: return QString();
    }
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
    QDBusConnection::sessionBus().connect(
        QStringLiteral("org.fcitx.Fcitx5"), QStringLiteral("/controller"),
        QStringLiteral("org.fcitx.Fcitx.Controller1"),
        QStringLiteral("InputMethodGroupsChanged"), this,
        SLOT(syncFcitxInputMethodState()));
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

    syncFcitxInputMethodState();
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

void VirtualKeyboardModel::syncFcitxInputMethodState() {
    syncCurrentIMList();
    syncUniqueName();
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
    QStringList inputMethodNames;
    QFile profileFile(QDir::homePath() + QStringLiteral("/.config/fcitx5/profile"));
    if (profileFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&profileFile);
        const QRegularExpression namePattern(QStringLiteral("^Name=(.+)$"));
        while (!stream.atEnd()) {
            const QString line = stream.readLine().trimmed();
            const auto match = namePattern.match(line);
            if (match.hasMatch()) {
                const QString name = match.captured(1).trimmed();
                if (name.startsWith(QStringLiteral("keyboard-")) &&
                    !inputMethodNames.contains(name)) {
                    inputMethodNames.append(name);
                }
            }
        }
    }

    if (inputMethodNames.isEmpty()) {
        inputMethodNames << QStringLiteral("keyboard-us")
                         << QStringLiteral("keyboard-cn-tib")
                         << QStringLiteral("keyboard-cn-tib_asciinum");
    }

    QStringList stringList;
    for (const auto &uniqueName : inputMethodNames) {
        QString localName = uniqueName;
        QString label;
        QString icon = QStringLiteral("input-keyboard");

        if (uniqueName == QStringLiteral("keyboard-us")) {
            localName = QStringLiteral("English");
            label = QStringLiteral("us");
        } else if (uniqueName == QStringLiteral("keyboard-cn-tib")) {
            localName = QStringLiteral("藏语");
            label = QStringLiteral("bo");
        } else if (uniqueName == QStringLiteral("keyboard-cn-tib_asciinum")) {
            localName = QStringLiteral("藏语 ASCII 数字");
            label = QStringLiteral("bo");
        }

        stringList.append(uniqueName + "|" + localName + "|" + label + "|" + icon);
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
