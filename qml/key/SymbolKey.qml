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

import QtQuick 2.0
import QtQuick.Controls 2.0

CharKey {
    id: symbolKey

    property alias shiftLabel: shiftLabel_

    state: virtualKeyboard.symbolState
    states: [
        State {
            name: "NORMAL"

            PropertyChanges {
                target: keyLabel
                text: virtualKeyboard.tibetanKeyLabel(label)
            }

            PropertyChanges {
                target: symbolKey
                inputText: label
            }

            PropertyChanges {
                target: shiftLabel
                text: virtualKeyboard.tibetanShiftedKeyLabel(shiftedText)
            }

        },
        State {
            name: "SHIFT"

            PropertyChanges {
                target: keyLabel
                text: virtualKeyboard.tibetanShiftedKeyLabel(shiftedText)
            }

            PropertyChanges {
                target: symbolKey
                inputText: shiftedText
            }

            PropertyChanges {
                target: shiftLabel
                text: ""
            }

        }
    ]

    Label {
        id: shiftLabel_

        text: virtualKeyboard.tibetanShiftedKeyLabel(shiftedText)
        color: virtualKeyboard.fontSecondaryColor
        font.pointSize: virtualKeyboard.shiftFontSize
        font.weight: Font.Light
        visible: true

        anchors {
            left: parent.left
            leftMargin: virtualKeyboard.shiftLeftMargin
            top: parent.top
            topMargin: virtualKeyboard.topLeftMargin
        }

    }

}
