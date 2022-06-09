import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import CustomComponents


Item
{
    id: root
    property bool activated : false
    
    implicitWidth: layout.width
    implicitHeight: layout.height    
    
    
    RowLayout
    {
        id: layout
        Layout.fillWidth: true
        spacing: 4
        
        MCheckBox
        {
            id: updatesCheckBox
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            
            onClicked: root.activated = !root.activated;            
        }
        
        Item
        {
            id: keepMeUpdatedText
            Layout.fillWidth: true
            Layout.preferredHeight: keepMeUpdatedTextFirst.implicitHeight
            Layout.leftMargin: 6
            
            Column
            {
                spacing: 2
                
                Label
                {
                    id: keepMeUpdatedTextFirst
                    text: "Keep me updated about the new features and"
                    wrapMode: Text.WordWrap
                    font.pointSize: 11
                    color: properties.colorMediumText
                }
                
                Label
                {
                    id: keepMeUpdatedTextSecond
                    text: "upcoming improvements."
                    wrapMode: Text.WordWrap
                    font.pointSize: 11
                    color: properties.colorMediumText
                }
            }
        }
    }
}