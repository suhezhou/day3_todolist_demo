#ifndef TODOITEM_H
#define TODOITEM_H

#include <QString>
#include <QDate>

struct TodoItem {
    int id;
    QString title;
    QDate dueDate;      // 任务截止日期（即显示日期）
    QDate createdDate;  // 创建日期
    bool completed;
};

#endif // TODOITEM_H
