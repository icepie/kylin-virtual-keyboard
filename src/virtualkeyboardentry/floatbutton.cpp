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

#include "floatbutton.h"

#include <QBitmap>
#include <QCursor>
#include <QEnterEvent>
#include <QPainter>
#include <QVariant>
#include <QWindow>
#include <QShowEvent>
#include <QHideEvent>
#include "log.h"
#include "ukuiwaylandhelper/ukuiwaylandproperties.h"
#include "utils.h"

FloatButton::FloatButton(MouseClickedCallback mouseClickedCallback)
    : mouseClickedCallback_(std::move(mouseClickedCallback)) {
    initAttributes();
}

bool FloatButton::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::Enter:
        hovered_ = true;
        update();
        break;
    case QEvent::Leave:
        hovered_ = false;
        pressed_ = false;
        update();
        break;
    default:
        break;
    }

    return QPushButton::event(event);
}

void FloatButton::move(int x, int y) {
    KVKBD_DEBUG("position:{},{}", x, y);
    QPushButton::move(x, y);
}

void FloatButton::resize(int width, int height) {
    setFixedSize(width, height);
    setIconSize(size());
    QPushButton::resize(width, height);
}

void FloatButton::showEvent(QShowEvent *event) {
    KVKBD_INFO("float button show event, pos:{},{} size:{}x{}", x(), y(),
               width(), height());
    QPushButton::showEvent(event);
}

void FloatButton::hideEvent(QHideEvent *event) {
    KVKBD_INFO("float button hide event, pos:{},{} size:{}x{}", x(), y(),
               width(), height());
    QPushButton::hideEvent(event);
}

void FloatButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pressed_ = true;
        update();
        startX_ = event->pos().x();
        startY_ = event->pos().y();
        emit mousePressed();
        startGlobalX_ = windowHandle()->position().x();
        startGlobalY_ = windowHandle()->position().y();

        startClickTimer();

        if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
            getDesktopType() == DesktopType::WAYLAND) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            windowHandle()->startSystemMove();
#endif
        }
    }

    QPushButton::mousePressEvent(event);
}

bool FloatButton::shouldPerformMouseClick() const {
    return !isFloatButtonMoved() && clickTimer_->isActive();
}

void FloatButton::processMouseReleaseEvent() {
    emit mouseReleased(windowHandle()->position());

    manhattonLength = 0;
    pressed_ = false;
    hovered_ = rect().contains(mapFromGlobal(QCursor::pos()));
    update();

    startX_ = -1;
    startY_ = -1;
    startGlobalX_ = -1;
    startGlobalY_ = -1;
}

void FloatButton::processMouseClickEvent() {
    if (!mouseClickedCallback_) {
        return;
    }
    mouseClickedCallback_();
}

void FloatButton::mouseReleaseEvent(QMouseEvent *event) {
    QPushButton::mouseReleaseEvent(event);

    if (event->button() != Qt::LeftButton) {
        return;
    }

    bool couldPerformMouseClick = shouldPerformMouseClick();
    stopClickTimer();

    if (couldPerformMouseClick) {
        pressed_ = false;
        hovered_ = rect().contains(event->pos());
        update();
        processMouseClickEvent();

        return;
    }

    if (isFloatButtonMoved()) {
        processMouseReleaseEvent();
        return;
    }

    pressed_ = false;
    hovered_ = rect().contains(event->pos());
    update();
}

void FloatButton::updateManhattonLength(QMouseEvent *event) {
    int offsetX = event->pos().x() - startX_;
    int offsetY = event->pos().y() - startY_;

    manhattonLength += std::abs(offsetX) + std::abs(offsetY);
}

bool FloatButton::isFloatButtonMoved() const {
    auto offsetX = windowHandle()->position().x() - startGlobalX_;
    auto offsetY = windowHandle()->position().y() - startGlobalY_;
    auto offset = std::abs(offsetX) + std::abs(offsetY);
    return manhattonLength > manhattonLengthThreshold || offset > 0;
}

void FloatButton::processMouseMoveEvent(QMouseEvent *event) {
    if (!isFloatButtonMoved()) {
        updateManhattonLength(event);
    } else {
        emit mouseMoved(event->pos().x() - startX_, event->pos().y() - startY_);
    }
}

void FloatButton::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        processMouseMoveEvent(event);
    } else {
        hovered_ = rect().contains(event->pos());
        update();
    }
    QPushButton::mouseMoveEvent(event);
}

void FloatButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const QIcon *icon = &defaultIcon_;
    if (pressed_ && !pressedIcon_.isNull()) {
        icon = &pressedIcon_;
    } else if (hovered_ && !hoveredIcon_.isNull()) {
        icon = &hoveredIcon_;
    }

    if (!icon->isNull()) {
        const QRect iconRect = rect().adjusted(1, 1, -1, -1);
        icon->paint(&painter, iconRect, Qt::AlignCenter,
                    isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }
}

void FloatButton::updateThemeStyle(const QString &themeColor) {
    const QString themePrefix = QString(":/img/%1/").arg(themeColor);
    defaultIcon_ = QIcon(themePrefix + "floatbuttondefault.svg");
    hoveredIcon_ = QIcon(themePrefix + "floatbuttonhovered.svg");
    pressedIcon_ = QIcon(themePrefix + "floatbuttonpressed.svg");
    update();
}

void FloatButton::initAttributes() {
    setWindowTitle("kylin-virtual-keyboard-float-button");
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);
    setToolTip(tr("Click to show virtual keyboard"));
    setFocusPolicy(Qt::NoFocus);
    setFlat(true);

    Qt::WindowFlags flags = Qt::FramelessWindowHint;
    if (getDesktopType() == DesktopType::WAYLAND) {
        flags |= Qt::Window | Qt::WindowStaysOnTopHint |
                 Qt::WindowDoesNotAcceptFocus;
    } else {
        flags |= Qt::Tool;
        flags |= Qt::BypassWindowManagerHint;
    }
    setWindowFlags(flags);
    setStyleSheet(QStringLiteral("QPushButton{background:transparent;border:none;}"));

    // 主题框架默认禁用了move消息，因此，QPushButton需要禁用主题框架
    setProperty("useStyleWindowManager", QVariant(false));

    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        getDesktopType() == DesktopType::WAYLAND) {
        setProperty(UkuiWaylandProperty::SURFACE_ROLE,
                    UkuiWaylandProperty::Role::INPUT_PANEL);
        // inputpanel 默认禁止移动，需要设置窗口属性为可移动
        UkuiWindowStates defaultState = UkuiWindowState::Movable;
        QPair<uint32_t, uint32_t> pair(UKUI_WINDOW_STATE_MASK_ALL,
                                       defaultState);
        setProperty(UkuiWaylandProperty::SURFACE_STATE,
                    QVariant::fromValue(pair));
        // TODO: 合成器目前圆角是固定值，而且没有给inputpanel窗口设置圆角
        // 设置阴影和毛玻璃会有圆角问题，暂时屏蔽
        //        setProperty(UkuiWaylandProperty::SURFACE_NO_TITLEBAR, true);
        //                QPair<QRegion, int> pair(QRegion(), 0);
        //                setProperty(UkuiWaylandProperty::SURFACE_BLUR,
        //                            QVariant::fromValue(pair));
    }
}

void FloatButton::startClickTimer() {
    clickTimer_.reset(new QTimer());
    clickTimer_->setSingleShot(true);
    clickTimer_->start(clickTimeThreshold_);
}

void FloatButton::stopClickTimer() {
    if (clickTimer_ != nullptr) {
        clickTimer_->stop();
        clickTimer_.reset();
    }
}
