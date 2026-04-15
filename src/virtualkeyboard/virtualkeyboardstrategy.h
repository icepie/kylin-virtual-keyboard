/*
 * Copyright 2022 KylinSoft Co., Ltd.
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

#ifndef VIRTUALKEYBOARDSTRATEGY_H
#define VIRTUALKEYBOARDSTRATEGY_H

#include "geometrymanager/floatgeometrymanager.h"
#include "screenwatcher.h"

class VirtualKeyboardStrategy : public FloatGeometryManager::Strategy {
public:
    VirtualKeyboardStrategy() = default;
    ~VirtualKeyboardStrategy() override = default;

    int getDefaultRightMargin() const override {
        const int screenWidth =
            ScreenWatcher::getInstance().getOptimalScreenGeometry().width();

        return screenWidth * defaultRightMarginRatio_;
    }

    int getDefaultBottomMargin() const override {
        const int screenHeight =
            ScreenWatcher::getInstance().getOptimalScreenGeometry().height();

        return screenHeight * defaultBottomMarginRatio_;
    }

private:
    int getUnitWidth(const QRect &screenGeo) const override {
        return screenGeo.width();
    }

    float getViewWidthRatio() const override { return 1458.0 / 1620.0; }

    int getUnitHeight(const QRect &screenGeo) const override {
        return std::max(screenGeo.width(), screenGeo.height());
    }

    float getViewHeightRatio() const override { return 680.0 / 1620.0; }

private:
    static constexpr float defaultRightMarginRatio_ = 0.25f;
    static constexpr float defaultBottomMarginRatio_ = 0.15f;
};

#endif // VIRTUALKEYBOARDSTRATEGY_H
