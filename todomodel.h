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
    Q_PROPERTY(FilterMode filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DueDateRole,
        CreatedDateRole,
        CompletedRole
    };
    enum FilterMode {
        AllTasks,
        TodayTasks
    };
    Q_ENUM(FilterMode)   // 使枚举在 QML 中可用

    explicit TodoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addItem(const QString &title, const QDate &dueDate);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void setCompleted(int id, bool completed);
    Q_INVOKABLE void setCurrentDate(const QDate &date);
    Q_INVOKABLE void updateTask(int id, const QString &newTitle, const QDate &newDueDate);

    QDate currentDate() const { return m_currentDate; }
    FilterMode filterMode() const { return m_filterMode; }
    void setFilterMode(FilterMode mode);

signals:
    void currentDateChanged();
    void filterModeChanged();

private:
    void saveToFile() const;
    void loadFromFile();
    int getNextId() const;
    QList<TodoItem> filterByDate() const;          // 原有按日期过滤（可保留）
    QList<TodoItem> filteredItems() const;         // 按当前过滤模式过滤
    void migrateTasks(const QDate &targetDate);

    QList<TodoItem> m_items;
    QDate m_currentDate;
    QString m_filePath;
    QDate m_lastDate;
    FilterMode m_filterMode = AllTasks;
};

#endif // TODOMODEL_H
