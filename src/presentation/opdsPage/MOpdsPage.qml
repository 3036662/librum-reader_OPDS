import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Librum.controllers
import Librum.style
import Librum.fonts
import Librum.icons
import CustomComponents

Page {
    id: root
    background: Rectangle {
        anchors.fill: parent
        color: Style.colorPageBackground
    }

  ColumnLayout {
      id: layout
      anchors.fill: parent
      anchors.rightMargin: internal.windowRightMargin
      anchors.leftMargin: internal.windowLeftMargin
      spacing: 0

      RowLayout {
          id: headerRow
          Layout.fillWidth: true
          spacing: 0


          MTitle {
              id: pageTitle
              Layout.topMargin: 44
              titleText: "OPDS  catalogs"
              descriptionText: "Find books in public OPDS libraries"
          }

          Item {
              Layout.fillWidth: true
          }

          MButton {
              id: addOpdsButton
              Layout.preferredWidth: 140
              Layout.preferredHeight: 40
              Layout.topMargin: 22
              Layout.alignment: Qt.AlignBottom
              borderWidth: 0
              backgroundColor: Style.colorBasePurple
              text: "Add OPDS"
              textColor: Style.colorFocusedButtonText
              fontWeight: Font.Bold
              fontSize: Fonts.size13
              imagePath: Icons.addWhite

              onClicked: ;// FIXME importFilesDialog.open()
          }



      }

      Pane {
              id: opdsGridContainer
              Layout.fillWidth: true
              Layout.fillHeight: true
              Layout.topMargin: 30
              visible: !root.empty
              padding: 0
              background: Rectangle {
                  color: "transparent"
              }

              GridView {
                  id: opdsGrid
                  anchors.fill: parent
                  cellWidth: internal.folderWidth + internal.horizontalFolderSpacing
                  cellHeight: internal.folderHeight + internal.verticalFolderSpacing
                  delegate: contactDelegate
                  clip: true

                  cacheBuffer: 1000
                  model: OpdsController.opdsModel
              }
      }

      Component {
              id: contactDelegate
              Item {
                  id: delegateItem
                  width: opdsGrid.cellWidth; height: opdsGrid.cellHeight
                  Column {
                      anchors.fill: parent
                      Image {
                          width: parent.width
                          height: parent.width
                          fillMode :Image.Stretch
                          source: Icons.opdsFolder;
                          anchors.horizontalCenter: parent.horizontalCenter }
                      Text {
                          text: model.title;
                          anchors.horizontalCenter: parent.horizontalCenter
                          color: Style.colorText
                      }
                  }
                  MouseArea{
                      id:clickArea
                      anchors.fill: parent
                      hoverEnabled: true
                      onEntered:{
                           delegateItem.scale =1.1
                      }
                      onExited: {
                           delegateItem.scale =1.0
                      }
                      onClicked: {
                          if (model.id  === ""){
                            OpdsController.loadRootLib(model.url);
                           }
                          // TODO go by id
                      }
                  }
              }
        }

  }

  ListModel {
     id: contactModel
      ListElement {
          name: "Jim Williams"
      }
      ListElement {
          name: "John Brown"
      }
      ListElement {
          name: "Bill Smyth"
      }
      ListElement {
          name: "Sam Wise"
      }
  }


      QtObject {
          id: internal
          property int windowLeftMargin: 64
          property int windowRightMargin: 70

          property int folderWidth: 80
          property int folderHeight: 160
          property int horizontalFolderSpacing: 64
          property int verticalFolderSpacing: 48
      }


}
