import QtQuick 2.15
import QtQuick.Controls 2.15
import "../my_components" as MyComponents

ScrollView {
    id: root
    clip: true
    contentHeight: 480
    contentWidth: 480

    Item {
        id: element1
        width:  Math.max(root.width, root.contentWidth)
        height: Math.max(root.height, root.contentHeight)

        Dial {
            id: dialBotVolume
            x: 8
            y: 87
            from: 1
            stepSize: 5
            to: 100

            Component.onCompleted: {
                value = chatBot.volume
            }

            onMoved: {
                chatBot.volume = value
            }
        }

        Label {
            id: element
            x: 8
            y: 62
            text: qsTr("Volume: %1 %").arg(chatBot.volume)
        }

        Switch {
            id: switchEnableSoundCommands
            y: 8
            height: 48
            text: qsTr("Enable Sound Commands")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8

            Component.onCompleted: {
                checked = chatBot.enabledSound;
            }

            onCheckedChanged: {
                chatBot.enabledSound = checked;
            }
        }

        MyComponents.MyTextField {
            id: textFieldTestMessage
            y: 239
            height: 43
            anchors.leftMargin: 8
            anchors.left: parent.left
            anchors.right: buttonSendTestMessage.left
            anchors.rightMargin: 8
            placeholderText: qsTr("Test message...")

            onAccepted: {
                sendTestMessage();
            }
        }

        Label {
            id: label
            y: 24
            height: 17
            text: qsTr("Send test message:")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: textFieldTestMessage.left
            anchors.leftMargin: 0
            anchors.bottom: textFieldTestMessage.top
            anchors.bottomMargin: 8
        }

        Button {
            id: buttonSendTestMessage
            x: 463
            y: 312
            width: 169
            height: 48
            text: qsTr("Send")
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenterOffset: 0
            anchors.verticalCenter: textFieldTestMessage.verticalCenter
            highlighted: true
            icon.source: "qrc:/resources/images/flask.svg"

            onClicked: {
                sendTestMessage();
            }
        }
    }

    function sendTestMessage(){
        if (textFieldTestMessage.text.length > 0)
        {
            chatHandler.sendTestMessage(textFieldTestMessage.text);
            textFieldTestMessage.text = "";
        }
    }
}

/*##^##
Designer {
    D{i:4;anchors_width:624;anchors_x:8}D{i:5;anchors_width:449;anchors_x:8}D{i:6;anchors_x:437}
}
##^##*/
