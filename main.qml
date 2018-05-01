import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.3

Window {

    id: mainFrame
    visible: true
    width: 640
    height: 360
    color: "#2d2d2d"
    title: qsTr("Duplicity UI")

    Connections {
        target: handler
        // Add new status text on top
        onUpdateStatusText: statusText.s = newStatus + '\n' + statusText.s
        onUpdateKeys: {
            encryptKey.append({
                                  text: uid,
                                  value: key
                              })
            signKey.append({
                               text: uid,
                               value: key
                           })
        }
        onUpdateDir: {
            selectDir.append({
                                 text: dir,
                                 value: dir
                             })
        }
    }

    Rectangle {
        id: leftMain
        anchors.left: mainFrame.left
        anchors.top: mainFrame.top
        width: mainFrame.width * 0.4
        height: mainFrame.height
        color: "#f0f0f2"

        Column {
            anchors.centerIn: parent
            spacing: 20

            ComboBox {
                id: box0
                currentIndex: 0
                width: leftMain.width * 0.7
                model: ListModel {
                    id: selectDir
                    ListElement {
                        text: "Select directory:"
                        value: ""
                    }
                }
                onCurrentIndexChanged: {
                    handler.showLastStatus(selectDir.get(
                                               box0.currentIndex).value)
                }
            }

            ComboBox {
                id: box1
                currentIndex: 0
                width: leftMain.width * 0.7
                model: ListModel {
                    id: encryptKey
                    ListElement {
                        text: "Encrypt key:"
                        value: ""
                    }
                }
            }

            TextField {
                id: encryptKeyPass
                width: leftMain.width * 0.7
                placeholderText: qsTr("Encrypt key passphrase:")
                echoMode: TextInput.Password
            }

            ComboBox {
                id: box2
                currentIndex: 0
                width: leftMain.width * 0.7
                model: ListModel {
                    id: signKey
                    ListElement {
                        text: "Sign key:"
                        value: ""
                    }
                }
            }

            TextField {
                id: signKeyPass
                width: leftMain.width * 0.7
                placeholderText: qsTr("Sign key passphrase:")
                echoMode: TextInput.Password
            }

            TextField {
                id: sourceDir
                width: leftMain.width * 0.7
                placeholderText: qsTr("Source directory:")
                Button {
                    id: sourceDirButton
                    text: "..."
                    width: parent.width / 10
                    height: parent.height
                    anchors.right: parent.right
                    anchors.top: parent.top
                    onClicked: folderDialog.open()
                }
            }

            FileDialog {
                id: folderDialog
                title: "Choose a folder to backup"
                selectFolder: true
                onAccepted: {
                    sourceDir.remove(0, sourceDir.length)
                    sourceDir.insert(0, folder)
                }
                onRejected: {
                    console.log("Rejected")
                }
            }

            TextField {
                id: targetDir
                width: leftMain.width * 0.7
                placeholderText: qsTr("Target directory:")
            }
        }
    }

    // Status Text
    Rectangle {
        id: rightMain1
        anchors.left: leftMain.right
        anchors.top: mainFrame.top
        width: mainFrame.width - leftMain.width
        height: mainFrame.height * 0.7
        color: "#f0f0f2"

        Text {
            id: statusText
            property string s: ""
            anchors.fill: parent
            anchors.margins: 10
            text: qsTr(s)
            wrapMode: Text.Wrap
            //            font.family: "Mono"
        }
    }

    Rectangle {
        id: rightMain2
        anchors.left: leftMain.right
        anchors.top: rightMain1.bottom
        width: mainFrame.width - leftMain.width
        height: mainFrame.height - rightMain1.height
        color: "#f0f0f2"

        Row {
            anchors.centerIn: parent
            spacing: 20

            Button {
                id: fullButton
                text: "Full"
                onClicked: {
                    handler.updateHandleFromQML(
                                targetDir.getText(0,
                                                  targetDir.length), sourceDir.getText(
                                    0, sourceDir.length),
                                encryptKey.get(box1.currentIndex).value,
                                signKey.get(box2.currentIndex).value,
                                encryptKeyPass.getText(0,
                                                       encryptKeyPass.length), signKeyPass.getText(
                                    0, signKeyPass.length),
                                selectDir.get(box0.currentIndex).text)
                    handler.performBackup(1)
                }
            }

            Button {
                id: incrButton
                text: "Incr"
                onClicked: {
                    handler.updateHandleFromQML(
                                targetDir.getText(0,
                                                  targetDir.length), sourceDir.getText(
                                    0, sourceDir.length),
                                encryptKey.get(box1.currentIndex).value,
                                signKey.get(box2.currentIndex).value,
                                encryptKeyPass.getText(0,
                                                       encryptKeyPass.length), signKeyPass.getText(
                                    0, signKeyPass.length),
                                selectDir.get(box0.currentIndex).text)
                    handler.performBackup(0)
                }
            }
        }
    }
}
