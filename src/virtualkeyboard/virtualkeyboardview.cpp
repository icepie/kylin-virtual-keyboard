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

#include "virtualkeyboard/virtualkeyboardview.h"
#include <QQmlContext>
#include <QQuickItem>
#include "log.h"
#include "screenwatcher.h"
#include "shadowborderitem.h"
#include "themewatcher.h"
#include "ukuiwaylandhelper/ukuiwaylandproperties.h"
#include "utils.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

#ifdef HAVE_LAYER_SHELL
#include <LayerShellQt/Window>
#endif

VirtualKeyboardView::VirtualKeyboardView(
    QObject &manager, QObject &model,
    std::unique_ptr<PlacementModeManager> placementModeManager,
    std::unique_ptr<ExpansionGeometryManager> expansionGeometryManager,
    std::unique_ptr<FloatGeometryManager> floatGeometryManager,
    ThemeWatcher &themeWatcher)
    : manager_(manager), model_(model), themeWatcher_(themeWatcher),
      placementModeManager_(std::move(placementModeManager)),
      expansionGeometryManager_(std::move(expansionGeometryManager)),
      floatGeometryManager_(std::move(floatGeometryManager)) {

    connect(placementModeManager_.get(),
            &PlacementModeManager::isFloatModeChanged, this,
            &VirtualKeyboardView::isFloatModeChanged);
    connect(floatGeometryManager_.get(), &FloatGeometryManager::viewMoved, this,
            &VirtualKeyboardView::move);
    connect(floatGeometryManager_.get(), &FloatGeometryManager::viewResized,
            this, &VirtualKeyboardView::resize);
    connect(
        &VirtualKeyboardSettings::getInstance(),
        &VirtualKeyboardSettings::animationAvailabilityChanged, this, [this]() {
            if (!VirtualKeyboardSettings::getInstance().isAnimationEnabled() &&
                view_) {
                view_->setOpacity(1.0f);
            }
        });
    initState();
    if (VirtualKeyboardSettings::getInstance().isPreloadViewEnabled()) {
        KVKBD_INFO("will preload quickview");
        initView();
    }
}

VirtualKeyboardView::~VirtualKeyboardView() {
    destroyView();
    if (view_ != nullptr) {
        view_.release()->deleteLater();
    }

    currentState_.reset();
    visibleState_.reset();
    hidingState_.reset();
    showingState_.reset();
    invisibleState_.reset();
    flippingState_.reset();
}

void VirtualKeyboardView::moveBy(int offsetX, int offsetY) {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->moveBy(offsetX, offsetY);
}

void VirtualKeyboardView::endDrag() {
    if (!isFloatMode()) {
        return;
    }

    floatGeometryManager_->endDrag(view_->position());
}

QRect VirtualKeyboardView::geometry() const {
    return getCurrentGeometryManager().geometry();
}

QRect VirtualKeyboardView::screenGeometry() const {
    return getCurrentGeometryManager().screenGeometry();
}

void VirtualKeyboardView::updateGeometry() {
    if (!isVisible()) {
        return;
    }

    QRect geo = geometry();
    view_->setGeometry(geo);
    emit positionChanged(geo.topLeft());
    emit sizeChanged();
    emitContentGeometrySignals();
}

void VirtualKeyboardView::updateMarginRatio() {
    floatGeometryManager_->updateViewMarginRatio();
}

void VirtualKeyboardView::updateExpansionFlippingStartGeometry() {
    auto geometry = floatGeometryManager_->geometry();
    view_->setGeometry(geometry.x(), view_->y(), geometry.width(),
                       geometry.height());

    emitContentGeometrySignals();
}

void VirtualKeyboardView::move(int x, int y) {
    if (!view_) {
        KVKBD_WARN("view_ is null!");
        return;
    }
    KVKBD_DEBUG("position:{},{}", x, y);
    view_->setX(x);
    view_->setY(y);
    emit positionChanged(QPoint(x, y));
}

void VirtualKeyboardView::resize(int width, int height) {
    if (!view_) {
        KVKBD_WARN("view_ is null!");
        return;
    }

    view_->resize(width, height);
    emitContentGeometrySignals();
    emit sizeChanged();
}

void VirtualKeyboardView::initView() {
    const auto preloadViewEnabled =
        VirtualKeyboardSettings::getInstance().isPreloadViewEnabled();
    if (view_ != nullptr && preloadViewEnabled) {
        return;
    }
    KVKBD_INFO("preloadViewEnabled:{}", preloadViewEnabled);

    qmlRegisterType<ShadowBorderItem>("VirtualKeyboard.ShadowBorderItem", 1, 0,
                                      "ShadowBorderItem");

    view_.reset(new QQuickView());
    view_->rootContext()->setContextProperty("manager", &manager_);
    view_->rootContext()->setContextProperty("model", &model_);
    view_->rootContext()->setContextProperty("view", this);
    view_->rootContext()->setContextProperty("themeWatcher", &themeWatcher_);

    view_->rootContext()->setContextProperty("QT_VERSION_MAJOR",
                                             QT_VERSION_MAJOR);
    view_->rootContext()->setContextProperty("QT_VERSION_MINOR",
                                             QT_VERSION_MINOR);
    view_->rootContext()->setContextProperty("QT_VERSION_PATCH",
                                             QT_VERSION_PATCH);

    view_->setTitle("kylin-virtual-keyboard");
    view_->setColor(QColor(Qt::transparent));
    view_->setSource(QUrl("qrc:/qml/VirtualKeyboard.qml"));

    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        // UKUI Wayland：使用 UKUI 私有 Wayland 扩展属性
        view_->setProperty(UkuiWaylandProperty::SURFACE_ROLE,
                           UkuiWaylandProperty::Role::INPUT_PANEL);
        UkuiWindowStates defaultState = UkuiWindowState::Movable;
        QPair<uint32_t, uint32_t> moveablePair(UKUI_WINDOW_STATE_MASK_ALL,
                                               defaultState);
        view_->setProperty(UkuiWaylandProperty::SURFACE_STATE,
                           QVariant::fromValue(moveablePair));
        view_->setProperty(UkuiWaylandProperty::SURFACE_NO_TITLEBAR, true);
        QPair<QRegion, int> blurPair(QRegion(), 0);
        view_->setProperty(UkuiWaylandProperty::SURFACE_BLUR,
                           QVariant::fromValue(blurPair));
    } else if (isWlrootsWayland()) {
        // Sway / wlroots：使用 wlr-layer-shell 协议
        // 窗口需要在 show() 之前完成 layer-shell 配置
#ifdef HAVE_LAYER_SHELL
        auto *layerWindow = LayerShellQt::Window::get(view_.get());
        if (layerWindow) {
            // 置于所有普通窗口之上
            layerWindow->setLayer(LayerShellQt::Window::LayerTop);
            // 锚定到屏幕底部，左右延伸撑满
            // 显式构造 Anchors（QFlags）避免 operator| 返回 int 的类型歧义
            LayerShellQt::Window::Anchors anchors(LayerShellQt::Window::AnchorBottom);
            anchors |= LayerShellQt::Window::AnchorLeft;
            anchors |= LayerShellQt::Window::AnchorRight;
            layerWindow->setAnchors(anchors);
            // 初始不占用 exclusive zone，显示时由 WorkspaceAdjuster 设置
            layerWindow->setExclusiveZone(0);
            // 不抢占键盘焦点，保持输入法焦点在目标应用
            layerWindow->setKeyboardInteractivity(
                LayerShellQt::Window::KeyboardInteractivityNone);
            KVKBD_INFO("layer-shell window configured for wlroots compositor.");
        } else {
            KVKBD_WARN("failed to get LayerShellQt::Window, falling back to basic flags.");
            view_->setFlags(getWaylandFallbackWindowFlags());
        }
#else
        // 未编译 layer-shell 支持时的回退方案
        // 需要在启动前设置环境变量:
        // QT_WAYLAND_SHELL_INTEGRATION=zwlr-layer-shell
        KVKBD_WARN("built without LayerShellQt, using basic Wayland window flags.");
        view_->setFlags(getWaylandFallbackWindowFlags());
#endif
    } else if (getDesktopType() == DesktopType::WAYLAND) {
        // 其他 Wayland 合成器回退到普通窗口提示；Wayland 下不能复用 X11 的 bypass 语义。
        view_->setFlags(getWaylandFallbackWindowFlags());
    } else {
        // X11 环境
        view_->setFlags(Qt::Window | Qt::WindowDoesNotAcceptFocus |
                        Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                        Qt::BypassWindowManagerHint);
    }

    view_->setGeometry(calculateInitialGeometry());
    emit positionChanged(geometry().topLeft());
    setViewOpacity();

    connectSignals();
}

void VirtualKeyboardView::pressed() {
    floatGeometryManager_->pressed();
    if (view_ == nullptr) {
        return;
    }
    raiseWindowIfNecessary();
    // UKUI Wayland 和 wlroots（Sway）均使用系统级拖动
    if (getDesktopType() == DesktopType::WAYLAND) {
        KVKBD_DEBUG("moveStart (Wayland)");
        view_->startSystemMove();
    }
}

Qt::WindowFlags VirtualKeyboardView::getWaylandFallbackWindowFlags() const {
    return Qt::Window | Qt::Tool | Qt::WindowDoesNotAcceptFocus |
           Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;
}

bool VirtualKeyboardView::shouldForceRaiseWindow() const {
    if (view_ == nullptr) {
        return false;
    }

    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        return false;
    }

    return getDesktopType() != DesktopType::X11;
}

void VirtualKeyboardView::raiseWindowIfNecessary() {
    if (!shouldForceRaiseWindow()) {
        return;
    }

    view_->raise();
}

QRect VirtualKeyboardView::calculateInitialGeometry() {
    auto geo = geometry();
    auto screenHeight = getScreenRelativeHeight();
    auto yOffset = geo.height();
    if (isFloatMode()) {
        yOffset = 40;
    }
    int normalizedY = std::min(screenHeight, geo.y() + yOffset);

    return QRect(geo.x(), normalizedY, geo.width(), geo.height());
}

int VirtualKeyboardView::getScreenRelativeHeight() {
    auto screenRect = screenGeometry();
    return screenRect.y() + screenRect.height();
}

void VirtualKeyboardView::connectSignals() {
    auto *rootObject = view_->rootObject();

    connect(this, SIGNAL(updateCandidateArea(const QVariant &, int)),
            rootObject, SIGNAL(qmlUpdateCandidateList(QVariant, int)));
    connect(this, SIGNAL(imDeactivated()), rootObject,
            SIGNAL(qmlImDeactivated()));
}

void VirtualKeyboardView::destroyView() {
    if (view_ == nullptr) {
        return;
    }

    if (view_->isVisible()) {
        view_->hide();
    }

    if (!VirtualKeyboardSettings::getInstance().isPreloadViewEnabled()) {
        view_.release()->deleteLater();
    }
}

void VirtualKeyboardView::emitContentGeometrySignals() {
    emit contentHeightChanged();
    emit contentWidthChanged();
}

int VirtualKeyboardView::getContentWidth() {
    return getCurrentGeometryManager().getViewContentWidth();
}

int VirtualKeyboardView::getContentHeight() {
    return getCurrentGeometryManager().getViewContentHeight();
}

void VirtualKeyboardView::setViewOpacity() {
    if (VirtualKeyboardSettings::getInstance().isAnimationEnabled()) {
        view_->setOpacity(isFloatMode() ? 0.0f : 1.0f);
    }
}

GeometryManager &VirtualKeyboardView::getCurrentGeometryManager() const {
    if (isFloatMode()) {
        return *floatGeometryManager_;
    } else {
        return *expansionGeometryManager_;
    }
}
