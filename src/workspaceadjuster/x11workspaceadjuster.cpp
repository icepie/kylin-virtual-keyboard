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

#include "x11workspaceadjuster.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KWindowEffects>
#include <KX11Extras>
#include <NETWM>
#else
#include <KWindowSystem>
#endif

// X11Kf5WorkspaceAdjuster 仅在 Qt5 / KF5 下编译，Qt6 使用 KX11Extras
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
X11Kf5WorkspaceAdjuster::X11Kf5WorkspaceAdjuster()
    : dummyWidget_(nullptr), oneshotTimer_(nullptr) {
    KWindowSystem::setType(dummyWidget_.winId(), NET::Dock);
    dummyWidget_.setWindowFlags(Qt::FramelessWindowHint);
    dummyWidget_.setAttribute(Qt::WA_TranslucentBackground);
    oneshotTimer_.setSingleShot(true);
    connectSignal();
}

void X11Kf5WorkspaceAdjuster::connectSignal() {
    QObject::connect(&oneshotTimer_, &QTimer::timeout, this, [this]() {
        dummyWidget_.setGeometry(rect_);
        dummyWidget_.show();
        // 使用KWin接口调整工作区域，仅在X11下有效
        // 该接口对全屏应用无效
        // 该接口需在winId对象显示前后调用，否则可能不生效
        KWindowSystem::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, rect_.height(), rect_.x(),
                                        rect_.width() - 1);
    });
}

void X11Kf5WorkspaceAdjuster::raiseInputArea(QWindow */*window*/, const QRect &rect) {
    rect_ = rect;

    oneshotTimer_.start(SHOW_DELAY_TIME);
}

void X11Kf5WorkspaceAdjuster::fallInputArea() {
    KWindowSystem::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0);
    dummyWidget_.hide();
    oneshotTimer_.stop();
}
#endif // QT_VERSION < 6

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
X11Kf6WorkspaceAdjuster::X11Kf6WorkspaceAdjuster() {
    KX11Extras::setType(dummyWidget_.winId(), NET::Dock);
}

void X11Kf6WorkspaceAdjuster::connectSignal() {
    QObject::connect(&oneshotTimer_, &QTimer::timeout, this, [this]() {
        dummyWidget_.setGeometry(rect_);
        dummyWidget_.show();
        KX11Extras::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, rect_.height(), rect_.x(),
                                     rect_.width() - 1);
    });
}

void X11Kf6WorkspaceAdjuster::raiseInputArea(QWindow */*window*/, const QRect &rect) {
    rect_ = rect;

    oneshotTimer_.start(SHOW_DELAY_TIME);
}

void X11Kf6WorkspaceAdjuster::fallInputArea() {
    KX11Extras::setExtendedStrut(dummyWidget_.winId(), 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0);

    dummyWidget_.hide();
    oneshotTimer_.stop();
}

#endif
