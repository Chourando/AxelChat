import QtQuick 2.15
import QtQuick.Controls 2.15
import AxelChat.QMLUtils 1.0

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640
    Item {
        id: element
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Dialog {
            id: restartDialog
            anchors.centerIn: parent
            modal: true
            footer: DialogButtonBox {
                Button {
                    flat: true
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                    onClicked: {
                        restartDialog.close();
                    }
                }
                Button {
                    flat: true
                    text: qsTr("Restart")
                    DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
                    onClicked: {
                        restartDialog.close();
                        Qt.callLater(qmlUtils.restartApplication);
                    }
                }
            }
        }

        ComboBox {
            id: comboBoxLanguage
            anchors.left: labelLanguage.right
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 8
            model: ListModel {
                id: model
                ListElement { text: "English"; }
                ListElement { text: "Русский"; }
            }
        }

        Image {
            id: imageLanguageFlag
            y: 12
            width: 40
            height: 40
            anchors.verticalCenter: comboBoxLanguage.verticalCenter
            anchors.left: comboBoxLanguage.right
            anchors.leftMargin: 6
            mipmap: true
            fillMode: Image.PreserveAspectFit
            source: {
                if (comboBoxLanguage.currentText.toLowerCase() === "english")
                    return "qrc:/resources/images/flags/usa.svg";
                if (comboBoxLanguage.currentText.toLowerCase() === "русский")
                    return "qrc:/resources/images/flags/russia.svg";
            }
        }

        Switch {
            id: switchEnabledHardwareGraphicsAccelerator
            x: 8
            y: 62
            text: qsTr("Enabled Hardware Graphics Accelerator")

            Component.onCompleted: {
                checked = qmlUtils.enabledHardwareGraphicsAccelerator;
            }

            onCheckedChanged: {
                qmlUtils.enabledHardwareGraphicsAccelerator = checked;
            }

            onClicked: {
                restartDialog.title = qsTr("Changes will take effect after restarting the program");
                restartDialog.open();
            }
        }

        Switch {
            id: switchEnableSoundNewMessage
            x: 8
            y: 116
            text: qsTr("Enable Sound when New Message Received")

            Component.onCompleted: {
                checked = chatHandler.enabledSoundNewMessage;
            }

            onCheckedChanged: {
                chatHandler.enabledSoundNewMessage = checked;
            }
        }

        Label {
            id: labelLanguage
            y: 24
            text: qsTr("Language")
            anchors.verticalCenter: comboBoxLanguage.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
        }

    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.25}D{i:8;anchors_x:8}D{i:9;anchors_x:206;anchors_y:8}D{i:13;anchors_x:202}
}
##^##*/
