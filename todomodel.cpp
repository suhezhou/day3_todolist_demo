#include "todomodel.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

TodoModel::TodoModel(QObject *parent)
    : QAbstractListModel(parent), m_currentDate(QDate::currentDate())
{
    // 确定保存文件路径
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists())
        dir.mkpath(".");
    m_filePath = dataDir + "/tasks.json";

    // 加载已有数据
    loadFromFile();
    // 迁移过期任务
    QDate today = QDate::currentDate();
    //QDate today = QDate::currentDate().addDays(1);  // 模拟明天(只用于测试）
    if (today > m_lastDate) {
        migrateTasks(today);
        m_lastDate = today;
        saveToFile();   // 保存迁移后的状态
    }
}
void TodoModel::migrateTasks(const QDate &targetDate)
{
    int nextId = getNextId();
    QList<TodoItem> newItems;

    for (const TodoItem &item : m_items) {
        if (!item.completed && item.dueDate <= m_lastDate) {
            TodoItem newItem;
            newItem.id = nextId++;
            newItem.title = item.title;
            newItem.dueDate = targetDate;
            newItem.createdDate = QDate::currentDate();
            newItem.completed = false;
            newItems.append(newItem);
        }
    }

    if (!newItems.isEmpty()) {
        m_items.append(newItems);
        // 迁移后刷新模型
        beginResetModel();
        endResetModel();
        saveToFile();
    }
}
int TodoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return filteredItems().size();
}

QVariant TodoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QList<TodoItem> filtered = filteredItems();
    const TodoItem &item = filtered.at(index.row());

    switch (role) {
    case IdRole: return item.id;
    case TitleRole: return item.title;
    case DueDateRole: return item.dueDate;
    case CreatedDateRole: return item.createdDate;
    case CompletedRole: return item.completed;
    default: return QVariant();
    }
}

QHash<int, QByteArray> TodoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[DueDateRole] = "dueDate";
    roles[CreatedDateRole] = "createdDate";
    roles[CompletedRole] = "completed";
    return roles;
}

void TodoModel::addItem(const QString &title, const QDate &dueDate)
{
    if (title.trimmed().isEmpty())
        return;

    TodoItem newItem{getNextId(), title, dueDate, QDate::currentDate(), false};
    m_items.append(newItem);

    // 重置模型，使视图根据当前过滤模式重新显示
    beginResetModel();
    endResetModel();

    saveToFile();
}
void TodoModel::removeItem(int id)
{
    int index = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    m_items.removeAt(index);

    beginResetModel();
    endResetModel();

    saveToFile();
}
void TodoModel::setCompleted(int id, bool completed)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (m_items[i].completed != completed) {
                m_items[i].completed = completed;

                beginResetModel();
                endResetModel();

                saveToFile();
            }
            break;
        }
    }
}
void TodoModel::setCurrentDate(const QDate &date)
{
    if (date == m_currentDate)
        return;
    m_currentDate = date;
    // 由于当前日期变化，模型的行数会改变，需要重置模型
    beginResetModel();
    endResetModel();
    emit currentDateChanged();
}

QList<TodoItem> TodoModel::filterByDate() const
{
    QList<TodoItem> result;
    for (const TodoItem &item : m_items) {
        if (item.dueDate == m_currentDate)
            result.append(item);
    }
    return result;
}

int TodoModel::getNextId() const
{
    int maxId = 0;
    for (const TodoItem &item : m_items) {
        if (item.id > maxId)
            maxId = item.id;
    }
    return maxId + 1;
}

void TodoModel::saveToFile() const
{
    QJsonObject rootObj;
    rootObj["lastDate"] = m_lastDate.toString(Qt::ISODate);

    QJsonArray tasksArray;
    for (const TodoItem &item : m_items) {
        QJsonObject obj;
        obj["id"] = item.id;
        obj["title"] = item.title;
        obj["dueDate"] = item.dueDate.toString(Qt::ISODate);
        obj["createdDate"] = item.createdDate.toString(Qt::ISODate);
        obj["completed"] = item.completed;
        tasksArray.append(obj);
    }
    rootObj["tasks"] = tasksArray;

    QJsonDocument doc(rootObj);
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法保存文件:" << m_filePath;
        return;
    }
    file.write(doc.toJson());
    file.close();
}

void TodoModel::loadFromFile()
{
    QFile file(m_filePath);
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件加载:" << m_filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject rootObj;
    QJsonArray tasksArray;

    // 兼容旧格式（纯数组）
    if (doc.isArray()) {
        tasksArray = doc.array();
        m_lastDate = QDate::currentDate();  // 旧格式没有 lastDate，默认当前日期
    } else if (doc.isObject()) {
        rootObj = doc.object();
        tasksArray = rootObj["tasks"].toArray();
        m_lastDate = QDate::fromString(rootObj["lastDate"].toString(), Qt::ISODate);
        if (!m_lastDate.isValid())
            m_lastDate = QDate::currentDate();
    } else {
        return;
    }

    // 解析任务
    QList<TodoItem> loadedItems;
    for (const QJsonValue &val : tasksArray) {
        QJsonObject obj = val.toObject();
        TodoItem item;
        item.id = obj["id"].toInt();
        item.title = obj["title"].toString();
        item.dueDate = QDate::fromString(obj["dueDate"].toString(), Qt::ISODate);
        item.createdDate = QDate::fromString(obj["createdDate"].toString(), Qt::ISODate);
        item.completed = obj["completed"].toBool();
        loadedItems.append(item);
    }

    if (!loadedItems.isEmpty()) {
        beginResetModel();
        m_items = loadedItems;
        endResetModel();
    }
}
void TodoModel::updateTask(int id, const QString &newTitle, const QDate &newDueDate)
{
    int index = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    TodoItem &item = m_items[index];
    bool changed = false;
    if (item.title != newTitle) {
        item.title = newTitle;
        changed = true;
    }
    if (item.dueDate != newDueDate) {
        item.dueDate = newDueDate;
        changed = true;
    }
    if (!changed) return;

    beginResetModel();
    endResetModel();

    saveToFile();
}
QList<TodoItem> TodoModel::filteredItems() const
{
    QList<TodoItem> result;
    if (m_filterMode == AllTasks) {
        result = m_items;
    } else if (m_filterMode == TodayTasks) {
        QDate today = QDate::currentDate();
        for (const TodoItem &item : m_items) {
            if (item.dueDate == today && !item.completed)
                result.append(item);
        }
    }
    return result;
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
