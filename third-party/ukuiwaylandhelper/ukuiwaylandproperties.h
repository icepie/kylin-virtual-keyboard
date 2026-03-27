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

#ifndef UKUIWAYLANDPROPERTIES_H
#define UKUIWAYLANDPROPERTIES_H

#include <QMetaType>
#include <QPair>
#include <QVector>

/**
 * @brief UKUI 窗口状态枚举
 */
enum class UkuiWindowState : uint32_t {
    Minimizable = 0x1,
    Maximizable = 0x2,
    Closeable = 0x4,
    Fullscreenable = 0x8,
    Movable = 0x10,
    Resizable = 0x20,
    Focusable = 0x40,
    Activatable = 0x80,
    KeepAbove = 0x100,
    KeepBelow = 0x200,
};

Q_DECLARE_FLAGS(UkuiWindowStates, UkuiWindowState)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiWindowStates)

// Qt6 中 Q_DECLARE_OPERATORS_FOR_FLAGS 已生成 constexpr operator&，重复定义会导致歧义
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
inline UkuiWindowState operator&(UkuiWindowState lhs, UkuiWindowState rhs) {
    return static_cast<UkuiWindowState>(static_cast<uint32_t>(lhs) &
                                        static_cast<uint32_t>(rhs));
}
#endif

inline UkuiWindowState operator~(UkuiWindowState rhs) {
    return static_cast<UkuiWindowState>(~static_cast<uint32_t>(rhs));
}

constexpr uint32_t UKUI_WINDOW_STATE_MASK_ALL = 0x3ff;
constexpr uint32_t UKUI_WINDOW_STATE_DEFAULT = 0xff;

/**
 * @brief UKUI Wayland 协议相关的属性名称定义
 *
 * 这些属性名称用于通过 QWindow::setProperty() 与 UKUI Compositor 通信
 */
namespace UkuiWaylandProperty {

// 窗口属性名称
constexpr const char *SURFACE_ANCHOR = "ukui_surface_anchor"; // 锚定和布局
constexpr const char *SURFACE_NO_TITLEBAR =
    "ukui_surface_no_titlebar";                             // 无标题栏
constexpr const char *SURFACE_ROLE = "ukui_surface_role";   // 窗口角色
constexpr const char *SURFACE_STATE = "ukui_surface_state"; // 窗口状态
constexpr const char *SURFACE_SKIP_TASKBAR =
    "ukui_surface_skip_taskbar"; // 跳过任务栏
constexpr const char *SURFACE_SKIP_SWITCHER =
    "ukui_surface_skip_switcher";                         // 跳过窗口切换器
constexpr const char *SURFACE_BLUR = "ukui_surface_blur"; // 毛玻璃

// 窗口角色
namespace Role {
constexpr const char *INPUT_PANEL = "inputpanel"; // 输入面板
constexpr const char *DOCK = "dock";              // 停靠栏
constexpr const char *PANEL = "panel";            // 面板
constexpr const char *POPUP = "popup";            // 弹出窗口
} // namespace Role

/**
 * @brief UKUI Surface 锚定属性
 *
 * 用于设置窗口的锚定位置和边距
 */
struct SurfaceProperty {
    int32_t screen_id = 0; // 屏幕ID
    int32_t width = 0;     // 宽度
    int32_t height = 0;    // 高度
    int32_t anchor = 14; // 锚定位置（位标志：top=1, bottom=2, left=4, right=8）
    int32_t area = 0;    // 是否覆盖任务栏 （0: 不覆盖, 1: 覆盖）
    int32_t zone = 0;    // 保留区域大小
    int32_t margin_top = 0;    // 上边距
    int32_t margin_right = 0;  // 右边距
    int32_t margin_bottom = 0; // 下边距
    int32_t margin_left = 0;   // 左边距
    int32_t enabled = 0;       // 是否启用

    QVector<int32_t> toVector() const {
        return {screen_id,     width,       height,     anchor,
                area,          zone,        margin_top, margin_right,
                margin_bottom, margin_left, enabled};
    }
};

} // namespace UkuiWaylandProperty

#endif // UKUIWAYLANDPROPERTIES_H
