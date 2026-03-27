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

#ifndef X11WORKSPACEADJUSTER_H
#define X11WORKSPACEADJUSTER_H

#include "workspaceadjuster.h"

#include <QObject>
#include <QTimer>
#include <QWidget>

class X11Kf5WorkspaceAdjuster : public WorkspaceAdjuster {
public:
    X11Kf5WorkspaceAdjuster();
    ~X11Kf5WorkspaceAdjuster() override = default;

    void raiseInputArea(QWindow */*window*/, const QRect &rect) override;
    void fallInputArea() override;

private:
    void connectSignal();

private:
    QRect rect_;
    QWidget dummyWidget_;
    QTimer oneshotTimer_;
    static const int SHOW_DELAY_TIME = 200;
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class X11Kf6WorkspaceAdjuster : public WorkspaceAdjuster {
public:
    X11Kf6WorkspaceAdjuster();
    ~X11Kf6WorkspaceAdjuster() override = default;

    void raiseInputArea(QWindow *window, const QRect &rect) override;
    void fallInputArea() override;

private:
    void connectSignal();

private:
    QRect rect_;
    QWidget dummyWidget_;
    QTimer oneshotTimer_;
    static const int SHOW_DELAY_TIME = 200;
};
#endif
#endif
