import QtQuick 6.9
import QtQuick.Controls 6.9
import QtQuick.Layouts 6.9
import QtQuick.Dialogs

Window {
    visible: true
    width: 500
    height: 600
    title: "待办事项"

    Dialog {
        id: addTaskDialog
        title: "添加任务"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        width: 320

        ColumnLayout {
            TextField {
                id: taskTitleInput
                Layout.fillWidth: true
                placeholderText: "输入任务标题"
            }
        }

        onAccepted: {
            let title = taskTitleInput.text.trim()
            if (title) {
                todoModel.addItem(title, todoModel.currentDate)
                taskTitleInput.text = ""
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        RowLayout {
            Text { text: "当前日期："; font.bold: true }

            // 直接在这里改日期，回车就生效！
            TextField {
                id: dateEdit
                text: Qt.formatDate(todoModel.currentDate, "yyyy-MM-dd")
                Layout.fillWidth: true
                onEditingFinished: {
                    let s = text.trim()
                    let y = parseInt(s.slice(0,4))
                    let m = parseInt(s.slice(5,7)) - 1
                    let d = parseInt(s.slice(8,10))
                    let nd = new Date(y, m, d)
                    if (nd.getFullYear() === y) {
                        todoModel.setCurrentDate(nd)
                    }
                }
            }
        }

        Button {
            text: "＋ 添加任务"
            Layout.alignment: Qt.AlignRight
            onClicked: addTaskDialog.open()
        }

        ListView {
            id: listView
            keyNavigationWraps: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: todoModel
            spacing: 8
            clip: true

            delegate: Rectangle {
                width: listView.width
                height: 60
                color: completed ? "#e0e0e0" : "#ffffff"
                radius: 6
                border.color: "#ddd"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    CheckBox {
                        checked: completed
                        onClicked: todoModel.setCompleted(model.id, checked)
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Text { text: model.title; font.bold: true }
                        Text {
                            text: "截止：" + Qt.formatDate(model.dueDate, "yyyy-MM-dd")
                            font.pixelSize: 12
                        }
                    }

                    Button {
                        text: "删除"
                        onClicked: {
                            todoModel.removeItem(model.id)
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: todoModel
        function onCurrentDateChanged() {
            dateEdit.text = Qt.formatDate(todoModel.currentDate, "yyyy-MM-dd")
        }
    }
}
