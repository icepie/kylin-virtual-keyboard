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

#include "layershellworkspaceadjuster.h"

#include "../log.h"

#ifdef HAVE_LAYER_SHELL
#include <LayerShellQt/Window>
#endif

LayerShellWorkspaceAdjuster::LayerShellWorkspaceAdjuster() {
    KVKBD_INFO("layer-shell workspace adjuster.");
}

void LayerShellWorkspaceAdjuster::raiseInputArea(QWindow *window,
                                                  const QRect &rect) {
    KVKBD_INFO("layer-shell: raise input area, height={}.", rect.height());
    if (window == nullptr) {
        KVKBD_WARN("window is null, skip raiseInputArea.");
        return;
    }
    window_ = window;

#ifdef HAVE_LAYER_SHELL
    auto *layerWindow = LayerShellQt::Window::get(window_);
    if (layerWindow == nullptr) {
        KVKBD_WARN("failed to get LayerShellQt::Window.");
        return;
    }
    layerWindow->setExclusiveZone(rect.height());
    KVKBD_INFO("layer-shell: exclusive zone set to {}.", rect.height());
#else
    KVKBD_WARN("layer-shell: built without LayerShellQt, exclusive zone not supported.");
#endif
}

void LayerShellWorkspaceAdjuster::fallInputArea() {
    KVKBD_INFO("layer-shell: fall input area.");
    if (window_ == nullptr) {
        KVKBD_WARN("window_ is null, skip fallInputArea.");
        return;
    }

#ifdef HAVE_LAYER_SHELL
    auto *layerWindow = LayerShellQt::Window::get(window_);
    if (layerWindow == nullptr) {
        KVKBD_WARN("failed to get LayerShellQt::Window.");
        return;
    }
    layerWindow->setExclusiveZone(0);
    KVKBD_INFO("layer-shell: exclusive zone cleared.");
#endif
}
