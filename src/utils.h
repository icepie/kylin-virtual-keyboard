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

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

enum class DesktopType { X11, WAYLAND, UNKNOWN };

enum class DesktopEnvironment { GNOME, KDE, UKUI, SWAY, UNKNOWN };

static std::string toLower(const char *str) {
    if (str == nullptr) {
        return std::string();
    }
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

static std::vector<std::string> splitString(const std::string &str,
                                            char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        if (end != start) {
            tokens.push_back(str.substr(start, end - start));
        }
        start = end + 1;
        end = str.find(delimiter, start);
    }

    if (start < str.length()) {
        tokens.push_back(str.substr(start));
    }

    return tokens;
}

static inline DesktopEnvironment getDesktopEnvironment() {
    // 在ubuntu 上该环境变量会返回gnome:GNOME
    std::string lowerDesktop = toLower(std::getenv("XDG_CURRENT_DESKTOP"));
    auto desktops = splitString(lowerDesktop, ':');

    for (const auto &name : desktops) {
        if (name == "kde") {
            return DesktopEnvironment::KDE;
        } else if (name == "gnome") {
            return DesktopEnvironment::GNOME;
        } else if (name == "ukui") {
            return DesktopEnvironment::UKUI;
        } else if (name == "sway") {
            return DesktopEnvironment::SWAY;
        }
    }

    return DesktopEnvironment::UNKNOWN;
}

static inline DesktopType getDesktopType() {
    std::string type = toLower(std::getenv("XDG_SESSION_TYPE"));
    if (type == "x11") {
        return DesktopType::X11;
    } else if (type == "wayland") {
        return DesktopType::WAYLAND;
    }

    return DesktopType::UNKNOWN;
}

/**
 * @brief 判断当前是否运行在基于 wlroots 的 Wayland 合成器上
 *        包括 Sway 以及其他非 UKUI/KDE/GNOME 的 Wayland 环境
 */
static inline bool isWlrootsWayland() {
    if (getDesktopType() != DesktopType::WAYLAND) {
        return false;
    }
    auto env = getDesktopEnvironment();
    return env == DesktopEnvironment::SWAY ||
           env == DesktopEnvironment::UNKNOWN;
}