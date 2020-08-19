import QtQuick 2.0
import QtQuick.Controls 2.15
import ClipboardQml.Clipboard 1.0

TextField {
    id: root
    selectByMouse: true

    property int selectStart
    property int selectEnd
    property int curPos
    property bool autoTrim: false

    persistentSelection: true
    background.visible: !readOnly
    onTextChanged: {
        if (autoTrim)
        {
            var preLength = text.length;
            var atStart = cursorPosition <= 1;
            text = text.trim();
            if (atStart && text.length < preLength)
            {
                cursorPosition = 0;
            }
        }
    }

    onFocusChanged: {
        if (!focus)
        {
            deselect();
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true
        cursorShape: Qt.IBeamCursor

        onClicked: {
            contextMenu.x = mouse.x;
            contextMenu.y = mouse.y;
            contextMenu.open();
        }

        Menu {
            id: contextMenu
            MenuItem {
                enabled: !readOnly && selectedText.length != 0
                text: qsTr("Cut")
                icon.source: "qrc:/resources/images/cut-content-button.svg"
                onTriggered: {
                    root.cut();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuItem {
                enabled: selectedText.length != 0
                text: qsTr("Copy")
                icon.source: "qrc:/resources/images/copy-content.svg"
                onTriggered: {
                    root.copy();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuItem {
                enabled: !readOnly && clipboard.text.length != 0
                text: qsTr("Paste")
                icon.source: "qrc:/resources/images/clipboard-paste-button.svg"
                onTriggered: {
                    root.paste();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
            MenuSeparator {}
            MenuItem {
                enabled: root.selectedText.length != root.text.length
                text: qsTr("Select All")
                onTriggered: {
                    root.selectAll();
                    Qt.callLater(root.forceActiveFocus);
                }
            }
        }
    }
}
