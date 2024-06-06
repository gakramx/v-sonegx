import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import Theme

GroupBox {
    id:groupBox
    property color textColor: Theme.colorText
    property color colorSelect: Theme.colorSelect
    property color colorBorder: Theme.colorBorder
    property string tmpTitle: title
    title: qsTr("GroupBox")
    label:Item{
        Label {
            x: groupBox.leftPadding
            text: groupBox.title
            color: groupBox.textColor
            elide: Text.ElideRight
            font.pixelSize: 14
           // font.bold: true
            padding: 2
            layer.enabled: true
                   layer.effect: Glow {
                       radius: 64
                       spread: 0.5
                       samples: 128
                       color: "#ff6127"
                       visible: true
                   }
            Rectangle{
                anchors.fill: parent
                border.width: 1
                color: 'transparent'
                border.color : 'transparent'
                MouseArea{
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.border.color = groupBox.colorSelect
                    onExited: parent.border.color = 'transparent'
                    onClicked: {
                        if(contentItem.visible){
                            contentItem.visible=false
                            contentHeight=7
                            title=""
                            title=tmpTitle+" " +String.fromCodePoint(0x25BC)
                            groupBox.Layout.fillHeight= false

                        }
                        else{
                            contentItem.visible=true
                            contentHeight=implicitContentHeight
                            title=tmpTitle + " " +String.fromCodePoint(0x25B2)
                            groupBox.Layout.fillHeight= true
                        }
                    }

                }
            }
        }
    }
    background: Rectangle {
        y: groupBox.topPadding - groupBox.bottomPadding
        width: parent.width
        height: parent.height - groupBox.topPadding + groupBox.bottomPadding
        color:"transparent" //Theme.colorBackgroundView
        border.color: groupBox.colorBorder
        radius: 4
        Behavior on height {
            NumberAnimation {
                id: animateContentHeight
                duration: 400
                easing.type: Easing.OutQuint
            }
        }

    }
    Component.onCompleted: {
        tmpTitle=title
        title=tmpTitle + " " +String.fromCodePoint(0x25B2)
    }
}
