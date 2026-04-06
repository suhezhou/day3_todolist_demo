import QtQuick 6.9
import QtQuick.Controls 6.9
import QtQuick.Layouts 6.9
import QtCore
import TodoModel 1.0

Window {
    id: root
    visible: true
    width: 540
    height: 780
    minimumWidth: 460
    minimumHeight: 700
    title: "待办清单"

    Settings {
        id: appSettings
        category: "ui"
        property int savedThemeIndex: 0
    }

    property int themeIndex: Math.max(0, Math.min(appSettings.savedThemeIndex, themes.length - 1))
    property var themes: [
        { name: "森系", window: "#eef7f1", ink: "#1d2d25", muted: "#66786d", header: "#173428", headerSoft: "#244638", accent: "#2f9a68", accentSoft: "#e3f5ea", card: "#ffffff", soft: "#f5faf6", line: "#d6e8dc", warm: "#efcf68", warmSoft: "#fff7df" },
        { name: "杏黄", window: "#fbf6ea", ink: "#34281f", muted: "#7f7368", header: "#4d3927", headerSoft: "#634a32", accent: "#d08a2f", accentSoft: "#fff0d9", card: "#fffdf9", soft: "#fff9f0", line: "#ecdcbf", warm: "#85b56f", warmSoft: "#edf8e7" },
        { name: "青岚", window: "#eef6f5", ink: "#1f2d30", muted: "#67777b", header: "#18363c", headerSoft: "#254a51", accent: "#2f8f9a", accentSoft: "#def3f5", card: "#ffffff", soft: "#f3fafb", line: "#d6e6e8", warm: "#e8c86a", warmSoft: "#fff7df" }
    ]
    property var theme: themes[themeIndex]
    property var weekNames: ["周日", "周一", "周二", "周三", "周四", "周五", "周六"]
    property date calendarMonth: new Date(todoModel.currentDate.getFullYear(), todoModel.currentDate.getMonth(), 1)

    color: theme.window

    onThemeIndexChanged: appSettings.savedThemeIndex = themeIndex

    function fmtDate(value) {
        return Qt.formatDate(value, "yyyy-MM-dd")
    }

    function fmtHeaderDate(value) {
        return Qt.formatDate(value, "yyyy年 M月d日")
    }

    function parseDateInput(text) {
        let parts = text.trim().split("-")
        if (parts.length !== 3)
            return null
        let date = new Date(parseInt(parts[0]), parseInt(parts[1]) - 1, parseInt(parts[2]))
        if (isNaN(date.getTime()))
            return null
        return date
    }

    function sameDay(a, b) {
        return a.getFullYear() === b.getFullYear()
                && a.getMonth() === b.getMonth()
                && a.getDate() === b.getDate()
    }

    function addDays(baseDate, offset) {
        let date = new Date(baseDate)
        date.setDate(date.getDate() + offset)
        return date
    }

    function weekStart(date) {
        let start = new Date(date)
        start.setDate(start.getDate() - start.getDay())
        return start
    }

    function changeWeek(offset) {
        applyDate(addDays(todoModel.currentDate, offset * 7))
    }

    function applyDate(date) {
        todoModel.setCurrentDate(date)
        calendarMonth = new Date(date.getFullYear(), date.getMonth(), 1)
    }

    Dialog {
        id: addDialog
        modal: true
        anchors.centerIn: parent
        width: 360
        padding: 0
        standardButtons: Dialog.NoButton

        background: Rectangle {
            radius: 22
            color: theme.card
            border.color: theme.line
        }

        contentItem: ColumnLayout {
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 70
                radius: 22
                color: theme.accentSoft

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Label { text: "新建任务"; font.pixelSize: 22; font.bold: true; color: theme.ink }
                    Label { text: "按当前选中的日期创建"; font.pixelSize: 13; color: theme.muted }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.bottomMargin: 18
                spacing: 12

                TextField {
                    id: addTitleField
                    Layout.fillWidth: true
                    placeholderText: "输入任务标题"
                    padding: 14
                    background: Rectangle {
                        radius: 14
                        color: theme.soft
                        border.color: addTitleField.activeFocus ? theme.accent : theme.line
                        border.width: addTitleField.activeFocus ? 2 : 1
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 10

                    Button { text: "取消"; onClicked: addDialog.close() }
                    Button {
                        text: "创建"
                        onClicked: {
                            let title = addTitleField.text.trim()
                            if (title) {
                                todoModel.addItem(title, todoModel.currentDate)
                                addTitleField.text = ""
                                addDialog.close()
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: editDialog
        modal: true
        anchors.centerIn: parent
        width: 380
        padding: 0
        standardButtons: Dialog.NoButton

        property int taskId: -1
        property string oldTitle: ""
        property date oldDueDate: new Date()

        background: Rectangle {
            radius: 22
            color: theme.card
            border.color: theme.line
        }

        contentItem: ColumnLayout {
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 70
                radius: 22
                color: theme.warmSoft

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Label { text: "编辑任务"; font.pixelSize: 22; font.bold: true; color: theme.ink }
                    Label { text: "修改标题和截止日期"; font.pixelSize: 13; color: theme.muted }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 18
                Layout.rightMargin: 18
                Layout.bottomMargin: 18
                spacing: 12

                TextField {
                    id: editTitleField
                    Layout.fillWidth: true
                    text: editDialog.oldTitle
                    padding: 14
                    background: Rectangle {
                        radius: 14
                        color: theme.soft
                        border.color: editTitleField.activeFocus ? theme.accent : theme.line
                        border.width: editTitleField.activeFocus ? 2 : 1
                    }
                }

                TextField {
                    id: editDateField
                    Layout.fillWidth: true
                    text: fmtDate(editDialog.oldDueDate)
                    padding: 14
                    background: Rectangle {
                        radius: 14
                        color: theme.soft
                        border.color: editDateField.activeFocus ? theme.accent : theme.line
                        border.width: editDateField.activeFocus ? 2 : 1
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 10

                    Button { text: "取消"; onClicked: editDialog.close() }
                    Button {
                        text: "保存"
                        onClicked: {
                            let title = editTitleField.text.trim()
                            let dueDate = parseDateInput(editDateField.text)
                            if (title && dueDate) {
                                todoModel.updateTask(editDialog.taskId, title, dueDate)
                                editDialog.close()
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: calendarPopup
        x: Math.max(16, root.width - width - 20)
        y: 150
        width: 360
        padding: 0
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 24
            color: theme.card
            border.color: theme.line
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            RowLayout {
                Layout.fillWidth: true

                Button { text: "<"; onClicked: calendarMonth = new Date(calendarMonth.getFullYear(), calendarMonth.getMonth() - 1, 1) }
                Label {
                    Layout.fillWidth: true
                    text: Qt.formatDate(calendarMonth, "yyyy年 M月")
                    color: theme.ink
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                Button { text: ">"; onClicked: calendarMonth = new Date(calendarMonth.getFullYear(), calendarMonth.getMonth() + 1, 1) }
            }

            DayOfWeekRow {
                Layout.fillWidth: true
                locale: Qt.locale("zh_CN")
                delegate: Label {
                    text: model.shortName
                    color: theme.muted
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            MonthGrid {
                id: monthGrid
                Layout.fillWidth: true
                month: calendarMonth.getMonth()
                year: calendarMonth.getFullYear()
                locale: Qt.locale("zh_CN")

                delegate: Rectangle {
                    required property var model
                    radius: 14
                    color: model.month !== monthGrid.month ? "transparent"
                           : sameDay(model.date, todoModel.currentDate) ? theme.accent
                           : model.today ? theme.warmSoft
                           : "transparent"
                    border.color: sameDay(model.date, todoModel.currentDate) ? theme.accent : "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 3

                        Label {
                            text: model.day
                            color: model.month !== monthGrid.month ? "#b6bfba"
                                   : sameDay(model.date, todoModel.currentDate) ? "white"
                                   : theme.ink
                            horizontalAlignment: Text.AlignHCenter
                            width: parent.width
                        }

                        Rectangle {
                            width: 6
                            height: 6
                            radius: 3
                            color: sameDay(model.date, todoModel.currentDate) ? "white" : theme.accent
                            visible: model.month === monthGrid.month && todoModel.hasTasksForDate(model.date)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: model.month === monthGrid.month
                        onClicked: {
                            applyDate(model.date)
                            calendarPopup.close()
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 16

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 250
            radius: 30
            color: theme.header

            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: -30
                anchors.rightMargin: -18
                width: 150
                height: 150
                radius: 75
                color: theme.accent
                opacity: 0.20
            }

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.leftMargin: -22
                anchors.bottomMargin: -26
                width: 120
                height: 120
                radius: 60
                color: theme.warm
                opacity: 0.16
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label { text: "日期安排"; color: "#d7ece0"; font.pixelSize: 12; font.bold: true }
                        Label { text: fmtHeaderDate(todoModel.currentDate); color: "white"; font.pixelSize: 28; font.bold: true }
                        Label { text: "点击下方日期可切换，圆点表示该天有任务"; color: "#b8d8c1"; font.pixelSize: 13 }
                    }

                    ComboBox {
                        id: skinBox
                        model: themes.map(function(item) { return item.name })
                        currentIndex: themeIndex
                        onActivated: themeIndex = currentIndex
                        implicitWidth: 110
                        implicitHeight: 40

                        background: Rectangle {
                            radius: 14
                            color: theme.headerSoft
                            border.color: "#3f6a58"
                        }

                        contentItem: Label {
                            leftPadding: 14
                            rightPadding: 26
                            text: skinBox.displayText
                            color: "white"
                            font.bold: true
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                        }
                        indicator: Canvas {
                            x: skinBox.width - width - 12
                            y: skinBox.topPadding + (skinBox.availableHeight - height) / 2
                            width: 12
                            height: 8
                            contextType: "2d"

                            onPaint: {
                                context.reset()
                                context.moveTo(0, 0)
                                context.lineTo(width, 0)
                                context.lineTo(width / 2, height)
                                context.closePath()
                                context.fillStyle = "#ffffff"
                                context.fill()
                            }
                        }

                        popup: Popup {
                            y: skinBox.height + 6
                            width: 124
                            padding: 6
                            background: Rectangle {
                                radius: 16
                                color: theme.card
                                border.color: theme.line
                                border.width: 1
                            }
                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: skinBox.popup.visible ? skinBox.delegateModel : null
                            }
                        }

                        delegate: ItemDelegate {
                            width: 112
                            height: 38
                            text: modelData
                            highlighted: skinBox.highlightedIndex === index
                            background: Rectangle {
                                radius: 12
                                color: highlighted || skinBox.currentIndex === index ? theme.accentSoft : "transparent"
                            }
                            contentItem: Label {
                                text: parent.text
                                color: theme.ink
                                font.bold: index === skinBox.currentIndex
                                font.pixelSize: 13
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 12
                            }
                        }
                    }
                }

                Item {
                    id: weekStrip
                    Layout.fillWidth: true
                    implicitHeight: 72
                    x: 0
                    opacity: 1

                    RowLayout {
                        anchors.fill: parent
                        spacing: 8

                        Button {
                            text: "<"
                            onClicked: changeWeek(-1)
                            background: Rectangle { radius: 14; color: theme.headerSoft }
                            contentItem: Label { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        }

                        Repeater {
                            model: 7

                            delegate: Rectangle {
                                required property int index
                                readonly property var cellDate: addDays(weekStart(todoModel.currentDate), index)
                            Layout.fillWidth: true
                            implicitHeight: 72
                            radius: 16
                            color: sameDay(cellDate, todoModel.currentDate) ? theme.warm : theme.headerSoft
                            border.width: sameDay(cellDate, new Date()) ? 2 : 0
                            border.color: sameDay(cellDate, new Date()) ? theme.accentSoft : "transparent"

                            Column {
                                anchors.centerIn: parent
                                spacing: 4

                                    Label {
                                        text: weekNames[index]
                                        color: sameDay(cellDate, todoModel.currentDate) ? "#5a4300" : "#c9e1d1"
                                        font.pixelSize: 10
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        width: 40
                                    }

                                    Label {
                                        text: Qt.formatDate(cellDate, "d")
                                        color: sameDay(cellDate, todoModel.currentDate) ? "#5a4300" : "white"
                                        font.pixelSize: 18
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        width: 40
                                    }

                                    Rectangle {
                                        width: 6
                                        height: 6
                                        radius: 3
                                        color: sameDay(cellDate, todoModel.currentDate) ? "#5a4300" : theme.warm
                                        visible: todoModel.hasTasksForDate(cellDate)
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: applyDate(parent.cellDate)
                                }
                            }
                        }

                        Button {
                            text: ">"
                            onClicked: changeWeek(1)
                            background: Rectangle { radius: 14; color: theme.headerSoft }
                            contentItem: Label { text: parent.text; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        }

                        Button {
                            text: "定位今天"
                            onClicked: applyDate(new Date())
                            background: Rectangle { radius: 14; color: "white" }
                            contentItem: Label { text: parent.text; color: theme.accent; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            radius: 24
            color: theme.card
            border.color: theme.line

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Label { text: "日期与筛选"; color: theme.ink; font.pixelSize: 18; font.bold: true }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TextField {
                        id: dateField
                        Layout.fillWidth: true
                        text: fmtDate(todoModel.currentDate)
                        padding: 14
                        onEditingFinished: {
                            let d = parseDateInput(text)
                            if (d)
                                applyDate(d)
                            else
                                text = fmtDate(todoModel.currentDate)
                        }
                        background: Rectangle {
                            radius: 14
                            color: theme.soft
                            border.color: dateField.activeFocus ? theme.accent : theme.line
                            border.width: dateField.activeFocus ? 2 : 1
                        }
                    }

                    Button {
                        text: "今天"
                        onClicked: applyDate(new Date())
                        background: Rectangle { radius: 14; color: theme.accentSoft }
                        contentItem: Label {
                            text: parent.text
                            color: theme.accent
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        Layout.fillWidth: true
                        text: "当日全部"
                        checkable: true
                        checked: todoModel.filterMode === TodoModel.AllTasks
                        onClicked: todoModel.filterMode = TodoModel.AllTasks
                    }

                    Button {
                        Layout.fillWidth: true
                        text: "当日待办"
                        checkable: true
                        checked: todoModel.filterMode === TodoModel.TodayTasks
                        onClicked: todoModel.filterMode = TodoModel.TodayTasks
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 28
            color: theme.card
            border.color: theme.line

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true

                    Label { text: "任务列表"; color: theme.ink; font.pixelSize: 20; font.bold: true }
                    Item { Layout.fillWidth: true }

                    Button {
                        text: "+ 新建"
                        onClicked: addDialog.open()
                        background: Rectangle {
                            radius: 14
                            color: theme.warm
                            border.color: theme.warm
                        }
                        contentItem: Label {
                            text: parent.text
                            color: "#5a4300"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                ListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: todoModel
                    spacing: 12
                    clip: true

                    delegate: Rectangle {
                        width: listView.width
                        height: 94
                        radius: 22
                        color: model.completed ? theme.soft : "white"
                        border.color: theme.line

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 12

                            CheckBox {
                                checked: model.completed
                                onClicked: todoModel.setCompleted(model.id, checked)
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Label {
                                    text: model.title
                                    color: theme.ink
                                    font.pixelSize: 17
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Row {
                                    spacing: 8

                                    Rectangle {
                                        radius: 999
                                        color: theme.accentSoft
                                        width: dueText.implicitWidth + 18
                                        height: 28

                                        Label {
                                            id: dueText
                                            anchors.centerIn: parent
                                            text: "截止 " + fmtDate(model.dueDate)
                                            color: theme.accent
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }

                                    Rectangle {
                                        radius: 999
                                        color: model.completed ? theme.accentSoft : theme.warmSoft
                                        width: stateText.implicitWidth + 18
                                        height: 28

                                        Label {
                                            id: stateText
                                            anchors.centerIn: parent
                                            text: model.completed ? "已完成" : "进行中"
                                            color: model.completed ? theme.accent : "#8b6700"
                                            font.pixelSize: 12
                                            font.bold: true
                                        }
                                    }
                                }
                            }

                            ColumnLayout {
                                spacing: 8

                                Button {
                                    text: "编辑"
                                    background: Rectangle {
                                        radius: 12
                                        color: theme.accentSoft
                                        border.color: theme.accentSoft
                                    }
                                    contentItem: Label {
                                        text: parent.text
                                        color: theme.accent
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        editDialog.taskId = model.id
                                        editDialog.oldTitle = model.title
                                        editDialog.oldDueDate = model.dueDate
                                        editTitleField.text = model.title
                                        editDateField.text = fmtDate(model.dueDate)
                                        editDialog.open()
                                    }
                                }

                                Button {
                                    text: "删除"
                                    background: Rectangle {
                                        radius: 12
                                        color: "#fff1f1"
                                        border.color: "#f2d3d3"
                                    }
                                    contentItem: Label {
                                        text: parent.text
                                        color: "#c44c4c"
                                        font.bold: true
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: todoModel.removeItem(model.id)
                                }
                            }
                        }
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 300
                        height: 200
                        radius: 24
                        color: theme.soft
                        border.color: theme.line
                        visible: listView.count === 0

                        Column {
                            anchors.centerIn: parent
                            spacing: 10

                            Label {
                                text: "还没有任务"
                                font.pixelSize: 20
                                font.bold: true
                                color: theme.ink
                                horizontalAlignment: Text.AlignHCenter
                                width: parent.width
                            }

                            Label {
                                text: "切换日期或新建任务开始安排。"
                                color: theme.muted
                                width: 220
                                wrapMode: Text.WordWrap
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: todoModel
        function onCurrentDateChanged() {
            dateField.text = fmtDate(todoModel.currentDate)
            calendarMonth = new Date(todoModel.currentDate.getFullYear(), todoModel.currentDate.getMonth(), 1)
        }
    }
}
