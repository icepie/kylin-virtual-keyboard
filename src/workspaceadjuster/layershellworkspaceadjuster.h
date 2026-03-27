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

#ifndef LAYERSHELLWORKSPACEADJUSTER_H
#define LAYERSHELLWORKSPACEADJUSTER_H

#include "workspaceadjuster.h"

/**
 * @brief 基于 wlr-layer-shell 协议的工作区调整器
 *        适用于 Sway 及其他基于 wlroots 的 Wayland 合成器
 *
 * 通过设置 LayerShellQt::Window 的 exclusiveZone，
 * 在键盘显示时将其他窗口推开，隐藏时恢复。
 */
class LayerShellWorkspaceAdjuster : public WorkspaceAdjuster {
public:
    LayerShellWorkspaceAdjuster();
    ~LayerShellWorkspaceAdjuster() override = default;

    void raiseInputArea(QWindow *window, const QRect &rect) override;
    void fallInputArea() override;

private:
    QWindow *window_ = nullptr;
};

#endif // LAYERSHELLWORKSPACEADJUSTER_H
