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

#include "floatgeometrymanager.h"
#include <QMap>
#include "log.h"
#include "screenwatcher.h"

// static
const QString FloatGeometryManager::floatGeometryGroup = "floatGeometry";
// static
const QString FloatGeometryManager::marginRatioMapKey = "marginRatioMap";
// static
const QString FloatGeometryManager::leftMarginRatioKey = "leftMarginRatio";
// static
const QString FloatGeometryManager::topMarginRatioKey = "topMarginRatio";

const QString FloatGeometryManager::lastPositionMapKey = "lastPositionMap";

const QString FloatGeometryManager::lastPositionXKey = "lastPositionX";

const QString FloatGeometryManager::lastPositionYKey = "lastPositionY";

const int FloatGeometryManager::defaultCoordinate = -99999999;

FloatGeometryManager::FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                                           LocalSettings &viewSettings)
    : FloatGeometryManager(std::move(strategy), viewSettings, Scaler()) {}

FloatGeometryManager::FloatGeometryManager(std::unique_ptr<Strategy> strategy,
                                           LocalSettings &viewSettings,
                                           Scaler &&scaler)
    : GeometryManager(std::move(scaler)), strategy_(std::move(strategy)),
      viewSettings_(viewSettings) {
    if (strategy_->loadLastPosition()) {
        loadLastPostionMap();
    }
    loadMarginRatioMap();
}

FloatGeometryManager::~FloatGeometryManager() {}

void FloatGeometryManager::pressed() { lastSize_ = calculateViewSize(); }

void FloatGeometryManager::moveBy(int offsetX, int offsetY) {
    const QPoint offset(offsetX, offsetY);
    const auto currentPosition = calculateCurrentPosition();
    updateCurrentPostion(currentPosition);
    KVKBD_DEBUG("current position:{},{}, offset:{},{}", currentPosition.x(),
                currentPosition.y(), offset.x(), offset.y());
    moveView(QPoint(currentPosition + offset));
}
void FloatGeometryManager::endDrag(const QPoint &position) {
    updateCurrentPostion(position);
    const auto viewRect = QRect(GeometryManager::currentPosition_, lastSize_);
    const auto adjustedPosition = calculateOptimalScreenAtPosition(viewRect);
    updateCurrentPostion(adjustedPosition);
    moveView(adjustedPosition);
    saveMarginRatioMap();
    saveLastPostionMap();
    updateGeometry();
}

int FloatGeometryManager::calculateViewWidth() const {
    return strategy_->getViewWidth(getScreenGeometry());
}

int FloatGeometryManager::calculateViewHeight() const {
    return strategy_->getViewHeight(getScreenGeometry());
}

QRect FloatGeometryManager::getScreenGeometry() const {
    return ScreenWatcher::getInstance().getOptimalScreenGeometry(
        GeometryManager::currentPosition_);
}

void FloatGeometryManager::updateViewMarginRatio() {
    const auto viewPortGeo =
        ScreenWatcher::getInstance().getOptimalScreenGeometry();
    const auto viewSize = calculateViewSize();

    const int leftMargin =
        viewPortGeo.left() + viewPortGeo.width() -
        (viewSize.width() + strategy_->getDefaultRightMargin());
    const int topMargin =
        viewPortGeo.top() + viewPortGeo.height() -
        (viewSize.height() + strategy_->getDefaultBottomMargin());

    leftMarginRatio_ = calculateLeftMarginRatio(leftMargin);
    topMarginRatio_ = calculateTopMarginRatio(topMargin);
    KVKBD_DEBUG("leftMarginRatio_:{}, topMarginRatio_:{}", leftMarginRatio_,
                topMarginRatio_);
    saveLastPostionMap();
    saveMarginRatioMap();
}

QPoint FloatGeometryManager::calculateNormalizedPosition(
    const QPoint &position) const {
    const auto viewRect = QRect(position, calculateViewSize());
    const auto adjustedPosition = adjustToScreenEdges(viewRect).topLeft();
    return adjustedPosition;
}

QPoint FloatGeometryManager::calculateCurrentPosition() const {
    if (GeometryManager::currentPosition_.x() == defaultCoordinate ||
        GeometryManager::currentPosition_.y() == defaultCoordinate) {
        return calculateNormalizedPosition(
            calculatePositionFromRatio(leftMarginRatio_, topMarginRatio_));
    }
    return calculatePositionFromRatio(leftMarginRatio_, topMarginRatio_);
}

QPoint
FloatGeometryManager::calculatePositionFromRatio(float leftMarginRatio,
                                                 float topMarginRatio) const {
    const QSize marginSize = calculateMarginSize();
    return QPoint(marginSize.width() * leftMarginRatio,
                  marginSize.height() * topMarginRatio);
}

QPoint FloatGeometryManager::calculateNormalizedPositionFromRatio(
    float leftMarginRatio, float topMarginRatio) const {
    return calculateNormalizedPosition(
        calculatePositionFromRatio(leftMarginRatio, topMarginRatio));
}

QPoint FloatGeometryManager::calculateViewPosition() const {
    return calculateNormalizedPosition(calculateCurrentPosition());
}

QSize FloatGeometryManager::calculateMarginSize() const {
    const auto viewPortRect = getScreenGeometry();
    const auto viewSize = calculateViewSize();

    const int horizontalMargin =
        viewPortRect.left() + viewPortRect.width() - viewSize.width();
    const int verticalMargin =
        viewPortRect.top() + viewPortRect.height() - viewSize.height();

    KVKBD_DEBUG("horizontalMargin:{},verticalMargin:{}, viewPortRect: "
                "leftxtop:{}x{}, widthxheight:{}x{}, viewSize:{}x{}",
                horizontalMargin, verticalMargin, viewPortRect.left(),
                viewPortRect.top(), viewPortRect.width(), viewPortRect.height(),
                viewSize.width(), viewSize.height());
    return QSize(horizontalMargin, verticalMargin);
}

QMap<QString, QVariant> FloatGeometryManager::getMarginRatioMap() const {
    QMap<QString, QVariant> marginRatioMap = {
        {leftMarginRatioKey, leftMarginRatio_},
        {topMarginRatioKey, topMarginRatio_}};

    return marginRatioMap;
}

QMap<QString, QVariant> FloatGeometryManager::getLastPositionMap() const {
    QMap<QString, QVariant> lastPositionMap = {
        {lastPositionXKey, GeometryManager::currentPosition_.x()},
        {lastPositionYKey, GeometryManager::currentPosition_.y()}};

    return lastPositionMap;
}

float FloatGeometryManager::calculateLeftMarginRatio(float leftMargin) const {
    return leftMargin / calculateMarginSize().width();
}

float FloatGeometryManager::calculateTopMarginRatio(float topMargin) const {
    return topMargin / calculateMarginSize().height();
}

void FloatGeometryManager::updateMarginRatio(const QPoint &targetPosition) {
    leftMarginRatio_ = calculateLeftMarginRatio(targetPosition.x());
    topMarginRatio_ = calculateTopMarginRatio(targetPosition.y());
    KVKBD_DEBUG("leftMarginRatio_:{},topMarginRatio_:{}", leftMarginRatio_,
                topMarginRatio_);
}

void FloatGeometryManager::updateCurrentPostion(const QPoint &position) {
    GeometryManager::currentPosition_ = position;
}

void FloatGeometryManager::saveMarginRatioMap() {
    viewSettings_.setValue(floatGeometryGroup, marginRatioMapKey,
                           getMarginRatioMap());
}

void FloatGeometryManager::saveLastPostionMap() {
    viewSettings_.setValue(floatGeometryGroup, lastPositionMapKey,
                           getLastPositionMap());
}

QMap<QString, QVariant> FloatGeometryManager::getDefaultMarginRatioMap() const {
    const auto viewPortGeo =
        ScreenWatcher::getInstance().getOptimalScreenGeometry();
    const auto viewSize = calculateViewSize();

    const int leftMargin =
        viewPortGeo.left() + viewPortGeo.width() -
        (viewSize.width() + strategy_->getDefaultRightMargin());
    const int topMargin =
        viewPortGeo.top() + viewPortGeo.height() -
        (viewSize.height() + strategy_->getDefaultBottomMargin());

    const float defaultLeftMarginRatio = calculateLeftMarginRatio(leftMargin);
    const float defaultTopMarginRatio = calculateTopMarginRatio(topMargin);

    QMap<QString, QVariant> viewDefaultMarginRatioMap = {
        {leftMarginRatioKey, defaultLeftMarginRatio},
        {topMarginRatioKey, defaultTopMarginRatio}};

    return viewDefaultMarginRatioMap;
}

QMap<QString, QVariant>
FloatGeometryManager::getDefaultLastPositionMap() const {
    QMap<QString, QVariant> viewDefaultLastPositionMap = {
        {lastPositionXKey, defaultCoordinate},
        {lastPositionYKey, defaultCoordinate}};

    return viewDefaultLastPositionMap;
}

void FloatGeometryManager::loadMarginRatioMap() {
    const auto marginRatioMap =
        viewSettings_
            .getValue(floatGeometryGroup, marginRatioMapKey,
                      getDefaultMarginRatioMap())
            .toMap();

    const float leftMarginRatio = marginRatioMap[leftMarginRatioKey].toFloat();
    const float topMarginRatio = marginRatioMap[topMarginRatioKey].toFloat();
    KVKBD_DEBUG("leftMarginRatio:{}, topMarginRatio:{}", leftMarginRatio,
                topMarginRatio);
    updateMarginRatio(
        calculateNormalizedPositionFromRatio(leftMarginRatio, topMarginRatio));
}

void FloatGeometryManager::loadLastPostionMap() {
    const auto lastPositionMap =
        viewSettings_
            .getValue(floatGeometryGroup, lastPositionMapKey,
                      getDefaultLastPositionMap())
            .toMap();

    const auto lastPostionX = lastPositionMap[lastPositionXKey].toInt();
    const auto lastPostionY = lastPositionMap[lastPositionYKey].toInt();
    KVKBD_DEBUG("lastPostionX:{}, lastPostionY:{}", lastPostionX, lastPostionY);
    updateCurrentPostion(QPoint(lastPostionX, lastPostionY));
}

void FloatGeometryManager::moveView(const QPoint &targetPoint) {
    updateMarginRatio(targetPoint);
    emit viewMoved(targetPoint.x(), targetPoint.y());
}

QRect FloatGeometryManager::adjustToScreenEdges(const QRect &windowRect) const {
    QRect screenGeo = ScreenWatcher::getInstance().getOptimalScreenGeometry(
        windowRect.topLeft());
    QRect adjusted = windowRect;

    if (adjusted.left() < screenGeo.left())
        adjusted.moveLeft(screenGeo.left());
    if (adjusted.right() > screenGeo.right())
        adjusted.moveRight(screenGeo.right());
    if (adjusted.top() < screenGeo.top())
        adjusted.moveTop(screenGeo.top());
    if (adjusted.bottom() > screenGeo.bottom())
        adjusted.moveBottom(screenGeo.bottom());

    return adjusted;
}

QPoint FloatGeometryManager::calculateOptimalScreenAtPosition(
    const QRect &windowRect) const {
    QRect screenGeo = ScreenWatcher::getInstance().getOptimalScreenGeometry(
        windowRect.center());

    if (screenGeo.isEmpty() || screenGeo.contains(windowRect)) {
        return windowRect.topLeft();
    }

    QRect adjusted = windowRect;
    QPoint center = adjusted.center();

    // 水平方向调整
    if (adjusted.width() > screenGeo.width()) {
        // 窗口比屏幕宽，对齐到中心点最近的边缘
        int distToLeft = qAbs(center.x() - screenGeo.left());
        int distToRight = qAbs(center.x() - screenGeo.right());
        if (distToLeft < distToRight) {
            adjusted.moveLeft(screenGeo.left());
        } else {
            adjusted.moveRight(screenGeo.right());
        }
    } else {
        // 窗口不比屏幕宽，正常处理边界
        if (adjusted.left() < screenGeo.left())
            adjusted.moveLeft(screenGeo.left());
        if (adjusted.right() > screenGeo.right())
            adjusted.moveRight(screenGeo.right());
    }

    // 垂直方向调整
    if (adjusted.height() > screenGeo.height()) {
        // 窗口比屏幕高，对齐到中心点最近的边缘
        int distToTop = qAbs(center.y() - screenGeo.top());
        int distToBottom = qAbs(center.y() - screenGeo.bottom());
        if (distToTop < distToBottom) {
            adjusted.moveTop(screenGeo.top());
        } else {
            adjusted.moveBottom(screenGeo.bottom());
        }
    } else {
        // 窗口不比屏幕高，正常处理边界
        if (adjusted.top() < screenGeo.top())
            adjusted.moveTop(screenGeo.top());
        if (adjusted.bottom() > screenGeo.bottom())
            adjusted.moveBottom(screenGeo.bottom());
    }

    return adjusted.topLeft();
}
