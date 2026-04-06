#include "todomodel.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#ifndef TODO_PROJECT_DIR
#define TODO_PROJECT_DIR "."
#endif

TodoModel::TodoModel(QObject *parent)
    : QAbstractListModel(parent),
      m_database(QSqlDatabase::database()),
      m_currentDate(QDate::currentDate())
{
    if (!m_database.isValid() || !m_database.isOpen()) {
        qWarning() << "Database connection is not available for TodoModel.";
        return;
    }

    loadFromDatabase();
    importLegacyJsonIfNeeded();

    m_lastDate = loadLastDate();
    if (!m_lastDate.isValid()) {
        m_lastDate = QDate::currentDate();
        saveLastDate(m_lastDate);
    }

    const QDate today = QDate::currentDate();
    if (today > m_lastDate) {
        migrateTasks(today);
        m_lastDate = today;
        saveLastDate(m_lastDate);
    }
}

int TodoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return filteredItems().size();
}

QVariant TodoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return {};

    const QList<TodoItem> visibleItems = filteredItems();
    const TodoItem &item = visibleItems.at(index.row());

    switch (role) {
    case IdRole:
        return item.id;
    case TitleRole:
        return item.title;
    case DueDateRole:
        return item.dueDate;
    case CreatedDateRole:
        return item.createdDate;
    case CompletedRole:
        return item.completed;
    default:
        return {};
    }
}

QHash<int, QByteArray> TodoModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {TitleRole, "title"},
        {DueDateRole, "dueDate"},
        {CreatedDateRole, "createdDate"},
        {CompletedRole, "completed"},
    };
}

void TodoModel::addItem(const QString &title, const QDate &dueDate)
{
    const QString trimmedTitle = title.trimmed();
    if (trimmedTitle.isEmpty())
        return;

    TodoItem newItem{0, trimmedTitle, dueDate, QDate::currentDate(), false};
    int insertedId = 0;
    if (!insertTask(newItem, &insertedId))
        return;

    newItem.id = insertedId;
    const int sourceRow = m_items.size();
    const bool visible = matchesCurrentFilter(newItem);
    const int visibleRow = visible ? visibleRowForSourceRow(sourceRow) : -1;

    if (visible)
        beginInsertRows({}, visibleRow, visibleRow);

    m_items.append(newItem);

    if (visible)
        endInsertRows();
}

void TodoModel::removeItem(int id)
{
    const int sourceRow = indexOfItemById(id);
    if (sourceRow < 0)
        return;

    const bool visible = matchesCurrentFilter(m_items.at(sourceRow));
    const int visibleRow = visible ? visibleRowForSourceRow(sourceRow) : -1;

    if (!deleteTaskRecord(id))
        return;

    if (visible)
        beginRemoveRows({}, visibleRow, visibleRow);

    m_items.removeAt(sourceRow);

    if (visible)
        endRemoveRows();
}

void TodoModel::setCompleted(int id, bool completed)
{
    const int sourceRow = indexOfItemById(id);
    if (sourceRow < 0)
        return;

    TodoItem updatedItem = m_items.at(sourceRow);
    if (updatedItem.completed == completed)
        return;

    const bool wasVisible = matchesCurrentFilter(updatedItem);
    const int previousVisibleRow = wasVisible ? visibleRowForSourceRow(sourceRow) : -1;

    updatedItem.completed = completed;
    const bool isVisible = matchesCurrentFilter(updatedItem);
    if (!updateTaskRecord(updatedItem))
        return;

    if (wasVisible && isVisible) {
        m_items[sourceRow] = updatedItem;
        const QModelIndex changedIndex = index(previousVisibleRow, 0);
        emit dataChanged(changedIndex, changedIndex, {CompletedRole});
    } else if (wasVisible && !isVisible) {
        beginRemoveRows({}, previousVisibleRow, previousVisibleRow);
        m_items[sourceRow] = updatedItem;
        endRemoveRows();
    } else if (!wasVisible && isVisible) {
        const int newVisibleRow = visibleRowForSourceRow(sourceRow);
        beginInsertRows({}, newVisibleRow, newVisibleRow);
        m_items[sourceRow] = updatedItem;
        endInsertRows();
    } else {
        m_items[sourceRow] = updatedItem;
    }
}

void TodoModel::setCurrentDate(const QDate &date)
{
    if (date == m_currentDate)
        return;

    m_currentDate = date;
    beginResetModel();
    endResetModel();
    emit currentDateChanged();
}

void TodoModel::updateTask(int id, const QString &newTitle, const QDate &newDueDate)
{
    const int sourceRow = indexOfItemById(id);
    if (sourceRow < 0)
        return;

    TodoItem updatedItem = m_items.at(sourceRow);
    const QString trimmedTitle = newTitle.trimmed();
    if (trimmedTitle.isEmpty())
        return;

    bool changed = false;
    if (updatedItem.title != trimmedTitle) {
        updatedItem.title = trimmedTitle;
        changed = true;
    }
    if (updatedItem.dueDate != newDueDate) {
        updatedItem.dueDate = newDueDate;
        changed = true;
    }

    if (!changed)
        return;

    const bool wasVisible = matchesCurrentFilter(m_items.at(sourceRow));
    const int previousVisibleRow = wasVisible ? visibleRowForSourceRow(sourceRow) : -1;

    const bool isVisible = matchesCurrentFilter(updatedItem);
    if (!updateTaskRecord(updatedItem))
        return;

    if (wasVisible && isVisible) {
        m_items[sourceRow] = updatedItem;
        const QModelIndex changedIndex = index(previousVisibleRow, 0);
        emit dataChanged(changedIndex, changedIndex, {TitleRole, DueDateRole});
    } else if (wasVisible && !isVisible) {
        beginRemoveRows({}, previousVisibleRow, previousVisibleRow);
        m_items[sourceRow] = updatedItem;
        endRemoveRows();
    } else if (!wasVisible && isVisible) {
        const int newVisibleRow = visibleRowForSourceRow(sourceRow);
        beginInsertRows({}, newVisibleRow, newVisibleRow);
        m_items[sourceRow] = updatedItem;
        endInsertRows();
    } else {
        m_items[sourceRow] = updatedItem;
    }
}

bool TodoModel::hasTasksForDate(const QDate &date) const
{
    for (const TodoItem &item : m_items) {
        if (item.dueDate == date)
            return true;
    }

    return false;
}

void TodoModel::setFilterMode(FilterMode mode)
{
    if (m_filterMode == mode)
        return;

    m_filterMode = mode;
    beginResetModel();
    endResetModel();
    emit filterModeChanged();
}

bool TodoModel::loadFromDatabase()
{
    QSqlQuery query(m_database);
    if (!query.exec("SELECT id, title, completed, dueDate, createdDate FROM tasks ORDER BY id ASC")) {
        qWarning() << "Failed to load tasks:" << query.lastError().text();
        return false;
    }

    QList<TodoItem> loadedItems;
    while (query.next()) {
        TodoItem item;
        item.id = query.value("id").toInt();
        item.title = query.value("title").toString();
        item.completed = query.value("completed").toInt() != 0;
        item.dueDate = QDate::fromString(query.value("dueDate").toString(), Qt::ISODate);
        item.createdDate = QDate::fromString(query.value("createdDate").toString(), Qt::ISODate);
        loadedItems.append(item);
    }

    if (!loadedItems.isEmpty()) {
        beginResetModel();
        m_items = loadedItems;
        endResetModel();
    } else {
        m_items.clear();
    }

    return true;
}

bool TodoModel::importLegacyJsonIfNeeded()
{
    if (!m_items.isEmpty())
        return false;

    const QString dataDir = QDir(QStringLiteral(TODO_PROJECT_DIR)).filePath("data");
    QFile legacyFile(QDir(dataDir).filePath("tasks.json"));
    if (!legacyFile.exists())
        return false;

    if (!legacyFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open legacy JSON file:" << legacyFile.fileName();
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(legacyFile.readAll());
    legacyFile.close();

    QJsonArray tasksArray;
    QDate importedLastDate;
    if (document.isArray()) {
        tasksArray = document.array();
    } else if (document.isObject()) {
        const QJsonObject rootObject = document.object();
        tasksArray = rootObject.value("tasks").toArray();
        importedLastDate = QDate::fromString(rootObject.value("lastDate").toString(), Qt::ISODate);
    } else {
        qWarning() << "Legacy JSON format is invalid:" << legacyFile.fileName();
        return false;
    }

    QList<TodoItem> importedItems;
    for (const QJsonValue &value : tasksArray) {
        const QJsonObject object = value.toObject();
        TodoItem item;
        item.title = object.value("title").toString().trimmed();
        item.completed = object.value("completed").toBool();
        item.dueDate = QDate::fromString(object.value("dueDate").toString(), Qt::ISODate);
        item.createdDate = QDate::fromString(object.value("createdDate").toString(), Qt::ISODate);

        if (item.title.isEmpty() || !item.dueDate.isValid() || !item.createdDate.isValid())
            continue;

        int insertedId = 0;
        if (!insertTask(item, &insertedId))
            return false;

        item.id = insertedId;
        importedItems.append(item);
    }

    if (!importedItems.isEmpty()) {
        beginResetModel();
        m_items = importedItems;
        endResetModel();
    }

    if (importedLastDate.isValid()) {
        m_lastDate = importedLastDate;
        saveLastDate(m_lastDate);
    }

    return !importedItems.isEmpty();
}

bool TodoModel::insertTask(const TodoItem &item, int *insertedId)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO tasks (title, completed, dueDate, createdDate) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(item.title);
    query.addBindValue(item.completed ? 1 : 0);
    query.addBindValue(item.dueDate.toString(Qt::ISODate));
    query.addBindValue(item.createdDate.toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "Failed to insert task:" << query.lastError().text();
        return false;
    }

    if (insertedId)
        *insertedId = query.lastInsertId().toInt();

    return true;
}

bool TodoModel::updateTaskRecord(const TodoItem &item)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE tasks SET title = ?, completed = ?, dueDate = ?, createdDate = ? "
                  "WHERE id = ?");
    query.addBindValue(item.title);
    query.addBindValue(item.completed ? 1 : 0);
    query.addBindValue(item.dueDate.toString(Qt::ISODate));
    query.addBindValue(item.createdDate.toString(Qt::ISODate));
    query.addBindValue(item.id);

    if (!query.exec()) {
        qWarning() << "Failed to update task:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool TodoModel::deleteTaskRecord(int id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "Failed to delete task:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

void TodoModel::migrateTasks(const QDate &targetDate)
{
    QList<TodoItem> pendingItems;
    for (const TodoItem &item : m_items) {
        if (!item.completed && item.dueDate <= m_lastDate)
            pendingItems.append(item);
    }

    for (const TodoItem &item : pendingItems) {
        addItem(item.title, targetDate);
    }
}

QDate TodoModel::loadLastDate() const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM app_meta WHERE key = ?");
    query.addBindValue("lastDate");
    if (!query.exec()) {
        qWarning() << "Failed to load lastDate:" << query.lastError().text();
        return {};
    }

    if (!query.next())
        return {};

    return QDate::fromString(query.value(0).toString(), Qt::ISODate);
}

void TodoModel::saveLastDate(const QDate &date) const
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO app_meta (key, value) VALUES (?, ?) "
                  "ON CONFLICT(key) DO UPDATE SET value = excluded.value");
    query.addBindValue("lastDate");
    query.addBindValue(date.toString(Qt::ISODate));

    if (!query.exec())
        qWarning() << "Failed to save lastDate:" << query.lastError().text();
}

QList<TodoItem> TodoModel::filteredItems() const
{
    QList<TodoItem> visibleItems;
    for (const TodoItem &item : m_items) {
        if (matchesCurrentFilter(item))
            visibleItems.append(item);
    }
    return visibleItems;
}

bool TodoModel::matchesCurrentFilter(const TodoItem &item) const
{
    if (m_filterMode == AllTasks)
        return item.dueDate == m_currentDate;

    return item.dueDate == m_currentDate && !item.completed;
}

int TodoModel::indexOfItemById(int id) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id)
            return i;
    }

    return -1;
}

int TodoModel::visibleRowForSourceRow(int sourceRow) const
{
    int visibleRow = 0;
    for (int i = 0; i < sourceRow; ++i) {
        if (matchesCurrentFilter(m_items.at(i)))
            ++visibleRow;
    }
    return visibleRow;
}
