import QtQuick 2.0
import QtQuick.Controls 2.15

Item {
    id: root


    property int avatarType: root.AvatarType.Rounded

    enum AvatarType {
        None,
        Rounded,
        Original
    }

    onAvatarTypeChanged: {
        console.log("avatarType: ", avatarType)
    }

    Switch {
        id: switchShowTime
        x: 8
        y: 8
        text: qsTr("Show Time")
        onCheckedChanged: {
            //Main.showTime = checked
        }
    }

    GroupBox {
        id: groupBox
        x: 8
        y: 62
        width: 248
        height: 214
        title: qsTr("Avatar")

        RadioButton {
            id: radioButtonAvatarNone
            x: 0
            y: 0
            text: qsTr("None")
            onClicked: {
                avatarType = root.AvatarType.None
            }
        }

        RadioButton {
            id: radioButtonAvatarRounded
            x: 0
            y: 54
            text: qsTr("Rounded")
            checked: true
            onClicked: {
                avatarType = root.AvatarType.Rounded
            }
        }

        RadioButton {
            id: radioButtonAvatarOriginal
            x: 0
            y: 108
            text: qsTr("Original")
            onClicked: {
                avatarType = root.AvatarType.Original
            }
        }
    }

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
