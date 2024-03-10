import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Librum.style
import Librum.fonts
import Librum.icons
import Librum.controllers

Pane {
    id: root
    property alias selectedItems: popup.selectedItems
    property alias list: popup.list
    signal countChanged

    implicitWidth: 520
    implicitHeight: 46
    padding: 0
    horizontalPadding: 16
    background: Rectangle {
        color: Style.colorControlBackground
        border.color: Style.colorContainerBorder
        radius: 5
        border.width: 2
    }

    RowLayout {
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1

        Image {
            id: searchBarIcon
            source: Icons.search
            fillMode: Image.PreserveAspectFit
            sourceSize.height: 18
        }

        TextField {
            id: inputField
            Layout.fillWidth: true
            selectByMouse: true
            color: Style.colorBaseInputText
            font.pointSize: Fonts.size12dot5
            placeholderText: "Search Books..."
            placeholderTextColor: Style.colorPlaceholderText
            selectionColor: Style.colorTextSelection
            background: Rectangle {
                anchors.fill: parent
                radius: 5
                color: "transparent"
            }

            Keys.onEscapePressed: {
                if (popup.opened)
                    popup.close()
            }

            onActiveFocusChanged: {
                if (activeFocus && !popup.opened) {
                    popup.open()
                } else if (!activeFocus) {
                    popup.close()
                }
            }

            onPressed: {
                if (!popup.opened)
                    popup.open()
            }

            onTextEdited: {
                if (!popup.opened)
                    popup.open()

                LibraryController.bookTitleModel.sortString = text
            }
        }
    }

    MBookSelectorPopup {
        id: popup
        width: root.width
        y: root.height + 8
        x: -root.horizontalPadding

        onItemsChanged: root.countChanged()
    }

    function giveFocus() {
        inputField.forceActiveFocus()
    }
}
