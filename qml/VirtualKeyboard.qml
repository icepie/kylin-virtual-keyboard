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
import QtQuick.Window 2.0
import "js/utils.js" as Utils
import "theme" as ThemeModule

Rectangle {
    id: virtualKeyboard

    property bool isFloatMode: view.isFloatMode
    //候选词
    property var candidateList
    //候选列表高亮候选的索引
    property int globalCursorIndex: -1
    //可用输入法列表
    property var currentIMList: model.currentIMList
    //当前输入法名称
    property string uniqueName: model.uniqueName
    //当前输入法，组成为"uniqueName|localName|label"
    //TOOD 目前以“|”分割数据
    property string currentIM: "|||"
    //大小相关
    //设置默认值，防止报错，fontSize为0时会报错，加载qml文件时height为0，会导致fontSize为0
    property real floatWidthUnit: width == 0 ? 8 : width / 1458
    property real floatHeightUnit: height == 0 ? 8 : height / 548
    property real expansionHeightUnit: height == 0 ? 8 : height / 64
    property real cardinalNumber: isFloatMode
                                  ? Math.min(floatWidthUnit, floatHeightUnit) * 8
                                  : expansionHeightUnit
    property int dragBarHeight: cardinalNumber * 4.5
    property int preeditHeight: cardinalNumber * 5
    property int toolAndCandidateHeight: cardinalNumber * 8
    property int keyboardLayoutHeight: cardinalNumber * 49
    property int keySpacing: cardinalNumber
    property int keyWidth: (virtualKeyboardContent.width - 5 * cardinalNumber) * 2 / 31 - cardinalNumber
    property int spaceKeyWidth: keyWidth * 5.5 + keySpacing * 5
    property int keyHeight: cardinalNumber * 9
    property int firstRowKeyHeight: keyHeight * 7 / 9
    property int keyLableAlignment: keyWidth / 3
    property int keyIconAlignment: keyWidth / 3
    property int virtualKeyboardAvailableHeight: isFloatMode ? virtualKeyboard.height - dragBar.height : virtualKeyboard.height
    property int imListItemHeight: virtualKeyboardAvailableHeight * 1 / 10
    property int imListItemWidth: virtualKeyboardAvailableHeight * 5 / 13
    property real fontSize: keyHeight * 6 / 11 * 7 / 12
    property real shiftFontSize: fontSize / 2
    property real switchKeyFontSize: fontSize * 3 / 4
    property real fnSymbolKeyFontSize: fontSize * 3 / 4
    property real actionKeyFontSize: fontSize * 3 / 4
    property real preeditTextFontSize: fontSize * 3 / 4
    property real candidateFontSize: fontSize * 3 / 4
    property real imListFontSize: fontSize * 3 / 4
    property int preeditX: cardinalNumber * 3.5
    property int candidateListWidth: virtualKeyboardContent.width - 7 * cardinalNumber
    property int actionKeySize: cardinalNumber * 3
    property int toolbarSize: cardinalNumber * 6
    property int candidateListX: cardinalNumber * 2
    property int candidateListSpacing: cardinalNumber * 3
    property int keyRadius: cardinalNumber
    property int longPressInterval: 1000
    property int shiftLeftMargin: cardinalNumber
    property int shiftTopMargin: cardinalNumber / 2
    property int imLeftMargin: cardinalNumber
    property int dropShadowVerticalOffset: cardinalNumber / 3
    property int dragBarIndicatorWidth: cardinalNumber * 7
    property int dragBarIndicatorHeight: cardinalNumber / 2
    // 颜色相关
    property color virtualKeyboardColor: themeWrapper.virtualKeyboardColor
    property color charKeyNormalColor: themeWrapper.charKeyNormalColor
    property color charKeyPressedColor: themeWrapper.charKeyPressedColor
    property color charKeyHoverColor: themeWrapper.charKeyHoverColor
    property color charKeyDropShadowColor: themeWrapper.charKeyDropShadowColor
    property color actionKeyNormalColor: themeWrapper.actionKeyNormalColor
    property color actionKeyPressedColor: themeWrapper.actionKeyPressedColor
    property color actionKeyHoverColor: themeWrapper.actionKeyHoverColor
    property color switchKeyNormalColor: themeWrapper.switchKeyNormalColor
    property color switchKeyPressedColor: themeWrapper.switchKeyPressedColor
    property color switchKeyOpenColor: themeWrapper.switchKeyOpenColor
    property color switchKeyOpenPressedColor: themeWrapper.switchKeyOpenPressedColor
    property color switchKeyHoverNormalColor: themeWrapper.switchKeyHoverNormalColor
    property color switchKeyHoverOpenColor: themeWrapper.switchKeyHoverOpenColor
    property color switchKeyNormalDropShadowColor: themeWrapper.switchKeyNormalDropShadowColor
    property color switchKeyOpenDropShadowColor: themeWrapper.switchKeyOpenDropShadowColor
    property color switchKeyOpenPressedDropShadowColor: themeWrapper.switchKeyOpenPressedDropShadowColor
    property color currentIMColor: themeWrapper.currentIMColor
    property color candidateListBackgroundColor: themeWrapper.candidateListBackgroundColor
    property color candidateDefaultColor: themeWrapper.candidateDefaultColor
    property color candidateHighlightColor: themeWrapper.candidateHighlightColor
    property color preeditBottomColor: themeWrapper.preeditBottomColor
    property color dragBarIndicatorColor: themeWrapper.dragBarIndicatorColor
    property color hideButtonPressedColor: themeWrapper.hideButtonPressedColor
    property color hideButtonHoverColor: themeWrapper.hideButtonHoverColor
    property color placementButtonPressedColor: themeWrapper.placementButtonPressedColor
    property color placementButtonHoverColor: themeWrapper.placementButtonHoverColor
    property color fontPrimaryColor: themeWrapper.fontPrimaryColor
    property color fontSecondaryColor: themeWrapper.fontSecondaryColor
    // 圆角相关
    property int virtualKeyboardFloatPlacementRadius: themeWrapper.virtualKeyboardFloatPlacementRadius
    property int dragBarIndicatorRadius: themeWrapper.dragBarIndicatorRadius
    property int toolbarRadius: themeWrapper.toolbarRadius
    //状态相关
    property string letterState: "NORMAL"
    property string symbolState: "NORMAL"
    property string fnSymbolState: "NORMAL"
    property string capslockState: "NORMAL"
    property string shiftState: "NORMAL"
    property string altState: "NORMAL"
    property string ctrlState: "NORMAL"
    property string winState: "NORMAL"
    property string changeIMState: "NORMAL"
    property string switchLayoutButtonState: "NORMAL"
    property string placementMode: isFloatMode ? "FLOAT" : "EXPANSION"
    //可见性相关
    property bool isToolbarVisible: true
    property bool isToolAreaVisible: false
    property bool isKeyBoardLayoutVisible: true
    property bool isAllLayoutListVisible: false
    property string layout: "classic"
    property bool isCurrentIMListVisible: false
    property bool isShiftKeyLongPressed: shiftState === "LONG_PRESSED" || shiftState === "OPEN_LONG_PRESSED"
    property bool isTibetanKeyboardLayout: uniqueName.indexOf("keyboard-cn-tib") !== -1 || currentIM.indexOf("keyboard-cn-tib") !== -1

    //内部使用
    signal showToolbar()
    signal showCandidateList()
    signal charKeyClicked()
    signal shiftClicked()
    signal altClicked()
    signal ctrlClicked()
    signal winClicked()
    //后台发送给前台的信号
    signal qmlUpdateCandidateList(var candidateList, int globalCursorIndex)
    signal qmlImDeactivated()

    function processKeyEvent(key, keycode, modifierKeyStates, isRelease, time) {
        console.debug('key:', key, ',keycode:', keycode, ',modifierKeyStates:', modifierKeyStates, ',isRelease:', isRelease, ',time:', time);
        model.processKeyEvent(key, keycode, modifierKeyStates, isRelease, time);
    }

    function selectCandidate(index) {
        model.selectCandidate(index);
    }

    function setCurrentIM(imName) {
        model.setCurrentIM(imName);
    }

    function hideVirtualKeyboard() {
        manager.hide();
    }

    function flipPlacementMode() {
        manager.flipPlacementMode();
    }

    function tibetanKeyLabel(label) {
        if (!isTibetanKeyboardLayout)
            return label;

        var normalLabels = {
            "`": "ཨ", "1": "༡", "2": "༢", "3": "༣", "4": "༤", "5": "༥", "6": "༦", "7": "༧", "8": "༨", "9": "༩", "0": "༠", "-": "ཧ", "=": "ཝ",
            "q": "ཅ", "w": "ཆ", "e": "ེ", "r": "ར", "t": "ཏ", "y": "ཡ", "u": "ུ", "i": "ི", "o": "ོ", "p": "ཕ", "[": "ཙ", "]": "ཚ", "\\": "ཛ",
            "a": "འ", "s": "ས", "d": "ད", "f": "བ", "g": "ང", "h": "མ", "j": "་", "k": "ག", "l": "ལ", ";": "ཞ", "'": "།",
            "z": "ཟ", "x": "ཤ", "c": "ཀ", "v": "ཁ", "b": "པ", "n": "ན", "m": "-བཏགས་", ", ": "ཐ", ".": "ཇ", "/": "ཉ"
        };
        return normalLabels[label] || label;
    }

    function tibetanShiftedKeyLabel(label) {
        if (!isTibetanKeyboardLayout)
            return label;

        var shiftedLabels = {
            "~": "༁", "!": "༪", "@": "༫", "#": "༬", "$": "༭", "%": "༮", "^": "༯", "&": "༰", "*": "༱", "(": "༲", ")": "༳", "_": "༼", "+": "༽",
            "Q": "༕", "W": "༖", "E": "༗", "R": "ྼ", "T": "ཊ", "Y": "ྻ", "U": "༘", "I": "༙", "O": "༚", "P": "༛", "{": "༜", "}": "༝", "|": "༞",
            "A": "ཱ", "S": "༟", "D": "ཌ", "F": "༾", "G": "༿", "H": "࿏", "J": "༂", "K": "༃", "L": "༆", ":": "༇", "\"": "༸",
            "Z": "༴", "X": "ཥ", "C": "ཀྵ", "V": "྇", "B": "྆", "N": "ཎ", "M": "-བཏགས་", "<": "ཋ", ">": "༺", "?": "༻"
        };
        return shiftedLabels[label] || label;
    }

    function pressed() {
        manager.pressed()
    }

    function moveBy(offsetX, offsetY) {
        manager.moveBy(offsetX, offsetY);
    }

    function endDrag() {
        manager.endDrag();
    }

    // 根据当前主题获取图标路径
    // iconName: 图标文件名，例如 "backspace.svg"
    // 返回: 根据主题返回 "qrc:/img/light/backspace.svg" 或 "qrc:/img/dark/backspace.svg"
    function getIconPath(iconName) {
        return themeWrapper.getIconPath(iconName);
    }

    function onCharKeyClicked() {
        updateShiftKeyNormalState();
        ctrlState = "NORMAL";
        altState = "NORMAL";
        winState = "NORMAL";
    }

    function onShiftClicked() {
        ctrlState = "NORMAL";
        altState = "NORMAL";
        winState = "NORMAL";
    }

    function onCtrlClicked() {
        updateShiftKeyNormalState();
        altState = "NORMAL";
        winState = "NORMAL";
    }

    function onAltClicked() {
        updateShiftKeyNormalState();
        ctrlState = "NORMAL";
        winState = "NORMAL";
    }

    function onWinClicked() {
        updateShiftKeyNormalState();
        altState = "NORMAL";
        ctrlState = "NORMAL";
    }

    function updateShiftKeyNormalState() {
        if (!isShiftKeyLongPressed)
            shiftState = "NORMAL";

    }

    anchors.fill: parent
    color: themeWrapper.virtualKeyboardColor
    radius: isFloatMode ? themeWrapper.virtualKeyboardFloatPlacementRadius : 0
    Component.onCompleted: {
        charKeyClicked.connect(onCharKeyClicked);
        shiftClicked.connect(onShiftClicked);
        ctrlClicked.connect(onCtrlClicked);
        altClicked.connect(onAltClicked);
        winClicked.connect(onWinClicked);
    }
    onUniqueNameChanged: {
        console.info('onUniqueNameChanged');
        for (var i = 0; i < currentIMList.length; i++) {
            if (currentIMList[i].includes(uniqueName))
                currentIM = currentIMList[i];

        }
    }

    // 主题包装器优先初始化
    ThemeModule.ThemeWrapper {
        id: themeWrapper
    }

    Connections {
        target: virtualKeyboard
        onShowToolbar: {
            isToolbarVisible = true;
        }
        onShowCandidateList: {
            isToolbarVisible = false;
        }
        onQmlUpdateCandidateList: (candidateList, globalCursorIndex) => {
            if (candidateList.length === 0) {
                showToolbar();
            } else {
                virtualKeyboard.candidateList = candidateList;
                virtualKeyboard.globalCursorIndex = globalCursorIndex;
                showCandidateList();
            }
        }
    }

    Rectangle {
        id: virtualKeyboardContent

        color: "transparent"
        radius: virtualKeyboard.radius
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: parent.height

        DragBar {
            id: dragBar

            anchors.top: parent.top
        }

        Preedit {
            id: preedit

            anchors.top: dragBar.visible ? dragBar.bottom : parent.top
            cursorPosition: model.preeditCaret
            text: model.preeditText
        }

        Separator {
            id: separator

            anchors.top: preedit.bottom
        }

        ToolbarAndCandidateArea {
            id: toolbarAndCandidate

            anchors.top: separator.bottom
        }

        KeyboardLayoutArea {
            id: keyboardLayoutArea

            anchors.bottom: virtualKeyboardContent.bottom
        }

    }

}
