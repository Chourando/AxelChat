import QtQuick 2.15
import QtQuick.Controls 2.15
import AxelChat.QMLUtils 1.0

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 640
    Item {
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

        Switch {
            id: switchEnabledHardwareGraphicsAccelerator
            x: 8
            y: 8
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
            y: 62
            text: qsTr("Enable Sound when New Message Received")

            Component.onCompleted: {
                checked = chatHandler.enabledSoundNewMessage;
            }

            onCheckedChanged: {
                chatHandler.enabledSoundNewMessage = checked;
            }
        }
    }
}
