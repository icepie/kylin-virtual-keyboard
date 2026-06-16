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
import "../js/utils.js" as Utils

BaseKey {
    property string shiftedText
    property color normalColor: virtualKeyboard.charKeyNormalColor
    property color pressedColor: virtualKeyboard.charKeyPressedColor
    property color hoverColor: virtualKeyboard.charKeyHoverColor

    function sendKeyEvent(isRelease) {
        var keysym = Utils.getKeysymByKeyLabel(inputText)
        var modifierKeyState = Utils.getModifierKeyStates()
        virtualKeyboard.processKeyEvent(keysym, keycode, modifierKeyState, isRelease, Date())
    }

    function sendKeyPressEvent() {
        sendKeyEvent(false)
    }

    function sendKeyReleaseEvent() {
        sendKeyEvent(true);
    }

    MouseArea {
        id: keyMouseArea
        anchors.fill: parent
        hoverEnabled: !virtualKeyboard.isShiftKeyLongPressed

        onEntered: {
            keyBackground.state = "HOVER"
        }

        onExited: {
            keyBackground.state = "NORMAL"
        }
    }

    MultiPointTouchArea {
        anchors.fill: parent
        maximumTouchPoints: 1
        onReleased: {
            console.debug('MultiPointTouchArea,onReleased')
            keyBackground.state = "NORMAL"

            resetGlobalTimersIfNecessary()

            sendKeyReleaseEvent()
            charKeyClicked()
        }

        onPressed: {
            console.debug('MultiPointTouchArea,onPressed')
            keyBackground.state = "PRESSED"

            startGlobalLongPressTimer()

            sendKeyPressEvent()
        }
    }

    keyBackground.state: "NORMAL"
    keyBackground.states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: keyBackground
                color: normalColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: virtualKeyboard.dropShadowVerticalOffset
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: keyBackground
                color: pressedColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: -virtualKeyboard.dropShadowVerticalOffset
            }
        },
        State {
            name: "HOVER"
            PropertyChanges {
                target: keyBackground
                color: hoverColor
            }
        }
    ]

    function resetGlobalLongPressTimer() {
        if (globalLongPressTimer) {
            globalLongPressTimer.stop()
            globalLongPressTimer = null
        }
    }

    function resetGlobalKeyRepeaterTimer() {
        if (globalKeyRepeaterTimer) {
            globalKeyRepeaterTimer.stop()
            globalKeyRepeaterTimer = null
        }
    }

    function resetGlobalTimersIfNecessary() {
        if (globalLongPressTimer === longPressTimer) {
            resetGlobalLongPressTimer()
        }

        if (globalKeyRepeaterTimer === keyRepeaterTimer) {
            resetGlobalKeyRepeaterTimer()
        }
    }

    function startGlobalLongPressTimer() {
        resetGlobalLongPressTimer()
        resetGlobalKeyRepeaterTimer()

        globalLongPressTimer = longPressTimer
        globalLongPressTimer.start()
    }

    function startGlobalKeyRepeaterTimer() {
        resetGlobalLongPressTimer()

        globalKeyRepeaterTimer = keyRepeaterTimer
        globalKeyRepeaterTimer.start()
    }

    Timer {
        id: longPressTimer
        interval: virtualKeyboard.longPressInterval
        repeat: false
        onTriggered: startGlobalKeyRepeaterTimer()
    }

    Timer {
        id: keyRepeaterTimer
        interval: 50
        repeat: true
        onTriggered: {
            sendKeyPressEvent()
        }
    }
}
