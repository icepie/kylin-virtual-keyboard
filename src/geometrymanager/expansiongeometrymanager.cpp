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

#include "expansiongeometrymanager.h"
#include "screenwatcher.h"

ExpansionGeometryManager::ExpansionGeometryManager(Scaler &&scaler)
    : GeometryManager(std::move(scaler)) {}

int ExpansionGeometryManager::calculateViewWidth() const {
    return ScreenWatcher::getInstance()
        .getOptimalScreenGeometry(GeometryManager::currentPosition_)
        .width();
}

int ExpansionGeometryManager::calculateViewHeight() const {
    QRect screenGeo = ScreenWatcher::getInstance().getOptimalScreenGeometry(
        GeometryManager::currentPosition_);
    return screenGeo.height() * viewHeightRatio_;
}

QPoint ExpansionGeometryManager::calculateViewPosition() const {
    QRect viewPortRec = ScreenWatcher::getInstance().getOptimalScreenGeometry(
        GeometryManager::currentPosition_);
    return QPoint(viewPortRec.left(), viewPortRec.y() + viewPortRec.height() -
                                          calculateScaledViewHeight());
}

QRect ExpansionGeometryManager::getScreenGeometry() const {
    return ScreenWatcher::getInstance().getOptimalScreenGeometry(
        GeometryManager::currentPosition_);
}
