#ifndef TODOMODEL_H
#define TODOMODEL_H

#include <QAbstractListModel>
#include <QDate>
#include <QList>
#include <QSqlDatabase>

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
    Q_ENUM(FilterMode)

    explicit TodoModel(QObject *parent = nullptr);
    ~TodoModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addItem(const QString &title, const QDate &dueDate);
    Q_INVOKABLE void removeItem(int id);
    Q_INVOKABLE void setCompleted(int id, bool completed);
    Q_INVOKABLE void setCurrentDate(const QDate &date);
    Q_INVOKABLE void updateTask(int id, const QString &newTitle, const QDate &newDueDate);
    Q_INVOKABLE bool hasTasksForDate(const QDate &date) const;

    QDate currentDate() const { return m_currentDate; }
    FilterMode filterMode() const { return m_filterMode; }
    void setFilterMode(FilterMode mode);

signals:
    void currentDateChanged();
    void filterModeChanged();

private:
    bool loadFromDatabase();
    bool importLegacyJsonIfNeeded();
    bool insertTask(const TodoItem &item, int *insertedId);
    bool updateTaskRecord(const TodoItem &item);
    bool deleteTaskRecord(int id);
    void migrateTasks(const QDate &targetDate);
    QDate loadLastDate() const;
    void saveLastDate(const QDate &date) const;

    QList<TodoItem> filteredItems() const;
    bool matchesCurrentFilter(const TodoItem &item) const;
    int indexOfItemById(int id) const;
    int visibleRowForSourceRow(int sourceRow) const;

    QSqlDatabase m_database;
    QList<TodoItem> m_items;
    QDate m_currentDate;
    QDate m_lastDate;
    FilterMode m_filterMode = AllTasks;
};

#endif // TODOMODEL_H
