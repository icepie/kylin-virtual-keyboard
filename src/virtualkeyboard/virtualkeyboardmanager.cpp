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

#include "virtualkeyboardmanager.h"

#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

#include "animation/disabledanimator.h"
#include "animation/enabledanimator.h"
#include "animation/expansionanimationfactory.h"
#include "animation/floatanimationfactory.h"
#include "screenwatcher.h"
#include "utils.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"
#include "virtualkeyboardstrategy.h"
#include "workspaceadjuster/waylandworkspaceadjuster.h"
#include "workspaceadjuster/x11workspaceadjuster.h"
#include "workspaceadjuster/layershellworkspaceadjuster.h"

VirtualKeyboardManager::VirtualKeyboardManager(
    HideVirtualKeyboardCallback hideVirtualKeyboardCallback)
    : hideVirtualKeyboardCallback_(std::move(hideVirtualKeyboardCallback)) {
    initThemeWatcher();
    initVirtualKeyboardModel();

    initWorkspaceAdjuster();

    initVirtualKeyboardView();

    initScreenSignalConnections();
}

VirtualKeyboardManager::~VirtualKeyboardManager() {
    hideVirtualKeyboard();

    workspaceAdjuster_.reset();

    view_.reset();
    model_.reset();
}

void VirtualKeyboardManager::showVirtualKeyboard() {
    if (isVirtualKeyboardVisible()) {
        return;
    }

    view_->show();

    visibiltyChanged();
}

void VirtualKeyboardManager::hideVirtualKeyboard() {
    if (!isVirtualKeyboardVisible()) {
        return;
    }

    if (workspaceAdjuster_ != nullptr && !view_->isFloatMode()) {
        workspaceAdjuster_->fallInputArea();
    }

    view_->hide();

    visibiltyChanged();
}

void VirtualKeyboardManager::hide() {
    if (!hideVirtualKeyboardCallback_) {
        return;
    }

    hideVirtualKeyboardCallback_();
}

void VirtualKeyboardManager::flipPlacementMode() { view_->flip(); }

void VirtualKeyboardManager::pressed() { view_->pressed(); }

void VirtualKeyboardManager::moveBy(int offsetX, int offsetY) {
    view_->moveBy(offsetX, offsetY);
}

void VirtualKeyboardManager::endDrag() { view_->endDrag(); }

void VirtualKeyboardManager::visibiltyChanged() {
    emit virtualKeyboardVisibiltyChanged(isVirtualKeyboardVisible());
}

bool VirtualKeyboardManager::isVirtualKeyboardVisible() const {
    return view_->isVisible();
}

void VirtualKeyboardManager::updatePreeditCaret(int index) {
    model_->setPreeditCaret(index);
}

void VirtualKeyboardManager::updatePreeditArea(const QString &preeditText) {
    model_->setPreeditText(preeditText);
}

void VirtualKeyboardManager::updateCandidateArea(
    const QStringList &candidateTextList, bool hasPrev, bool hasNext,
    int pageIndex, int globalCursorIndex) {
    model_->updateCandidateArea(QVariant(candidateTextList), hasPrev, hasNext,
                                pageIndex, globalCursorIndex);
}

void VirtualKeyboardManager::notifyIMActivated(const QString &uniqueName) {
    model_->setUniqueName(uniqueName);
}

void VirtualKeyboardManager::notifyIMDeactivated(
    const QString & /*uniqueName*/) {
    emit model_->imDeactivated();
}

void VirtualKeyboardManager::notifyIMListChanged() {
    model_->syncCurrentIMList();
}

void VirtualKeyboardManager::initWorkspaceAdjuster() {
    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        // UKUI Wayland：使用 UKUI 私有 Wayland 协议
        workspaceAdjuster_.reset(new WaylandWlcomWorkspaceAdjuster());
    } else if (isWlrootsWayland()) {
        // Sway / 其他 wlroots 合成器：使用 wlr-layer-shell 协议
        workspaceAdjuster_.reset(new LayerShellWorkspaceAdjuster());
    } else if (getDesktopType() == DesktopType::WAYLAND) {
        // 其他 Wayland 合成器上不使用 X11 的工作区调整接口，避免误调用 X11 能力。
        workspaceAdjuster_.reset();
    } else {
        // X11 环境回退
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        workspaceAdjuster_.reset(new X11Kf6WorkspaceAdjuster());
#else
        workspaceAdjuster_.reset(new X11Kf5WorkspaceAdjuster());
#endif
    }
}

std::unique_ptr<PlacementModeManager>
VirtualKeyboardManager::createPlacementModeManager() {
    return std::unique_ptr<PlacementModeManager>(
        new PlacementModeManager(viewSettings_));
}

Scaler VirtualKeyboardManager::createFloatModeScaler() {
    return Scaler(
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        },
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        },
        []() {
            return VirtualKeyboardSettings::getInstance()
                .calculateVirtualKeyboardScaleFactor();
        });
}

Scaler VirtualKeyboardManager::createExpansionModeScaler() {
    return Scaler([]() { return 1.0f; },
                  []() {
                      return VirtualKeyboardSettings::getInstance()
                          .calculateVirtualKeyboardScaleFactor();
                  },
                  []() {
                      return VirtualKeyboardSettings::getInstance()
                          .calculateVirtualKeyboardScaleFactor();
                  });
}

std::unique_ptr<ExpansionGeometryManager>
VirtualKeyboardManager::createExpansionGeometryManager() {
    return std::unique_ptr<ExpansionGeometryManager>(
        new ExpansionGeometryManager(createExpansionModeScaler()));
}

std::unique_ptr<FloatGeometryManager>
VirtualKeyboardManager::createFloatGeometryManger() {
    return std::unique_ptr<FloatGeometryManager>(new FloatGeometryManager(
        std::unique_ptr<FloatGeometryManager::Strategy>(
            new VirtualKeyboardStrategy()),
        viewSettings_, createFloatModeScaler()));
}

void VirtualKeyboardManager::initThemeWatcher() {
    themeWatcher_.reset(new ThemeWatcher(this));
}

void VirtualKeyboardManager::initVirtualKeyboardModel() {
    model_.reset(new VirtualKeyboardModel(this));

    connect(model_.get(), SIGNAL(backendConnectionDisconnected()), this,
            SLOT(hideVirtualKeyboard()));
}

void VirtualKeyboardManager::initVirtualKeyboardView() {
    view_.reset(
        new VirtualKeyboardView(*this, *model_, createPlacementModeManager(),
                                createExpansionGeometryManager(),
                                createFloatGeometryManger(), *themeWatcher_));
    view_->setAnimator(createAnimator());

    connectVirtualKeyboardModelSignals();

    connectVirtualKeyboardViewSignals();

    connectVirtualKeyboardSettingsSignals();
}

void VirtualKeyboardManager::connectVirtualKeyboardModelSignals() {
    connect(model_.get(), SIGNAL(updateCandidateArea(const QVariant &, int)),
            view_.get(), SIGNAL(updateCandidateArea(const QVariant &, int)));
    connect(model_.get(), SIGNAL(imDeactivated()), view_.get(),
            SIGNAL(imDeactivated()));
}

void VirtualKeyboardManager::connectVirtualKeyboardViewSignals() {
    connect(view_.get(), &VirtualKeyboardView::raiseAppRequested, this,
            [this]() {
                if (workspaceAdjuster_ == nullptr) {
                    return;
                }

                workspaceAdjuster_->raiseInputArea(view_->view(),
                                                   view_->geometry());
            });

    connect(view_.get(), &VirtualKeyboardView::fallAppRequested, this,
            [this]() {
                if (workspaceAdjuster_ == nullptr) {
                    return;
                }

                workspaceAdjuster_->fallInputArea();
            });
    connect(view_.get(), &VirtualKeyboardView::positionChanged, this,
            [](const QPoint &position) {
                ScreenWatcher::getInstance().markScreen(position);
            });
    connect(view_.get(), &VirtualKeyboardView::sizeChanged, this,
            []() { ScreenWatcher::getInstance().notifyScreenMarkChanged(); });
}

void VirtualKeyboardManager::connectVirtualKeyboardSettingsSignals() {
    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::scaleFactorChanged, view_.get(),
            [this]() {
                view_->updateGeometry();
                raiseInputAreaIfNecessary();
            });

    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::animationAvailabilityChanged, this,
            [this]() { view_->setAnimator(createAnimator()); });
}

void VirtualKeyboardManager::handleScreensChanged() {
    if (!view_->isVisible()) {
        return;
    }
    KVKBD_DEBUG("update view geometry");
    view_->updateGeometry();
    raiseInputAreaIfNecessary();
}

void VirtualKeyboardManager::handleMarkedScreenChanged() {
    if (isVirtualKeyboardVisible()) {
        return;
    }
    // 虚拟键盘不可见时，标记屏幕改变是由悬浮球触发的，此时需要更新视图的边缘比例
    KVKBD_DEBUG(
        "marked screen changed, update virtual keyboard view margin ratio");
    view_->updateMarginRatio();
}

void VirtualKeyboardManager::initScreenSignalConnections() {
    ScreenWatcher &screenWatcher = ScreenWatcher::getInstance();
    connect(&screenWatcher, &ScreenWatcher::screensChanged, this,
            &VirtualKeyboardManager::handleScreensChanged);
    connect(&screenWatcher, &ScreenWatcher::screenMarkChanged, this,
            &VirtualKeyboardManager::handleMarkedScreenChanged);
}

void VirtualKeyboardManager::raiseInputAreaIfNecessary() {
    if (!view_->isVisible()) {
        return;
    }

    if (view_->isFloatMode()) {
        return;
    }

    if (workspaceAdjuster_ == nullptr) {
        return;
    }

    workspaceAdjuster_->raiseInputArea(view_->view(), view_->geometry());
}

std::unique_ptr<AnimationFactory>
VirtualKeyboardManager::createAnimationFactory() {
    if (view_->isFloatMode()) {
        return std::unique_ptr<AnimationFactory>(new FloatAnimationFactory());
    } else {
        return std::unique_ptr<AnimationFactory>(
            new ExpansionAnimationFactory());
    }
}

std::unique_ptr<Animator> VirtualKeyboardManager::createEnabledAnimator() {
    auto animator = std::unique_ptr<EnabledAnimator>(new EnabledAnimator(
        [this]() { return view_->isFloatMode(); }, createAnimationFactory()));

    EnabledAnimator *enabledAnimator = animator.get();
    connect(view_.get(), &VirtualKeyboardView::isFloatModeChanged,
            enabledAnimator, [this, enabledAnimator]() {
                enabledAnimator->setAnimationFactory(createAnimationFactory());
            });

    return animator;
}
std::unique_ptr<Animator> VirtualKeyboardManager::createDisabledAnimator() {
    return std::unique_ptr<Animator>(new DisabledAnimator());
}

std::unique_ptr<Animator> VirtualKeyboardManager::createAnimator() {
    if (VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
        return createEnabledAnimator();
    } else {
        return createDisabledAnimator();
    }
}
