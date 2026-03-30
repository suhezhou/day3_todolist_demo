#ifndef TODOMODEL_H
#define TODOMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QDate>
#include "todoitem.h"

class TodoModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QDate currentDate READ currentDate WRITE setCurrentDate NOTIFY currentDateChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DueDateRole,
        CreatedDateRole,
        CompletedRole
    };

    explicit TodoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addItem(const QString &title, const QDate &dueDate);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void setCompleted(int id, bool completed);
    Q_INVOKABLE void setCurrentDate(const QDate &date);

    QDate currentDate() const { return m_currentDate; }
public slots:
    void updateTask(int id, const QString &newTitle, const QDate &newDueDate);

signals:
    void currentDateChanged();

private:
    void saveToFile() const;
    void loadFromFile();
    int getNextId() const;                 // 获取下一个可用的 id
    QList<TodoItem> filterByDate() const;  // 返回当前日期下的任务列表（用于 rowCount 和 data）

    QList<TodoItem> m_items;   // 存储所有任务
    QDate m_currentDate;       // 当前显示的日期
    QString m_filePath;
    QDate m_lastDate;      // 上次迁移的日期
    void migrateTasks(const QDate &targetDate);
};

#endif // TODOMODEL_H
