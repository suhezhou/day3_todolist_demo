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

    // 如果没有任务，添加几条示例（可选）
    if (m_items.isEmpty()) {
        QDate today = QDate::currentDate();
        beginInsertRows(QModelIndex(), 0, 4);
        m_items.append({getNextId(), "学习 QML", today, today, false});
        m_items.append({getNextId(), "完成项目报告", today.addDays(1), today, true});
        m_items.append({getNextId(), "买菜", today, today, false});
        m_items.append({getNextId(), "锻炼身体", today.addDays(2), today, false});
        m_items.append({getNextId(), "阅读技术文章", today, today, true});
        endInsertRows();
        saveToFile();
    }
}

int TodoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // 返回当前日期下的任务数
    return filterByDate().size();
}

QVariant TodoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QList<TodoItem> filtered = filterByDate();
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

    // 生成新任务
    TodoItem newItem{getNextId(), title, dueDate, QDate::currentDate(), false};

    // 如果新任务属于当前日期，需要插入到过滤后的视图中
    if (dueDate == m_currentDate) {
        // 获取当前过滤后的任务列表（用于确定插入位置）
        QList<TodoItem> filtered = filterByDate();
        int insertPosInFilter = filtered.size();  // 插入到末尾（也可按需排序）
        // 通知视图：在过滤后列表的第 insertPosInFilter 行插入
        beginInsertRows(QModelIndex(), insertPosInFilter, insertPosInFilter);
        // 实际插入到内部数据列表（末尾）
        m_items.append(newItem);
        endInsertRows();
    } else {
        // 不属于当前日期，直接追加到内部数据列表，视图无变化
        m_items.append(newItem);
    }

    saveToFile();
}

void TodoModel::removeItem(int id)
{
    // 1. 先在内部数据列表中查找要删除的任务的索引
    int rowInItems = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            rowInItems = i;
            break;
        }
    }
    if (rowInItems == -1)
        return;

    // 2. 确定该任务是否属于当前显示日期，如果是，还需要知道它在过滤后列表中的索引
    bool belongsToCurrent = (m_items[rowInItems].dueDate == m_currentDate);
    int rowInFilter = -1;
    if (belongsToCurrent) {
        QList<TodoItem> filtered = filterByDate();
        for (int j = 0; j < filtered.size(); ++j) {
            if (filtered[j].id == id) {
                rowInFilter = j;
                break;
            }
        }
        // 理论上一定找到，但以防万一
        if (rowInFilter == -1)
            return;
    }

    // 3. 根据是否属于当前日期，选择正确的视图删除方式
    if (belongsToCurrent) {
        // 从视图中删除该行
        beginRemoveRows(QModelIndex(), rowInFilter, rowInFilter);
        // 从内部数据列表中删除
        m_items.removeAt(rowInItems);
        endRemoveRows();
    } else {
        // 不属于当前日期，直接删除内部数据，视图无变化
        m_items.removeAt(rowInItems);
    }

    saveToFile();
}

void TodoModel::setCompleted(int id, bool completed)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (m_items[i].completed != completed) {
                m_items[i].completed = completed;
                // 如果该任务属于当前显示日期，更新视图
                if (m_items[i].dueDate == m_currentDate) {
                    // 找到该任务在当前过滤列表中的位置
                    QList<TodoItem> filtered = filterByDate();
                    int rowInFilter = -1;
                    for (int j = 0; j < filtered.size(); ++j) {
                        if (filtered[j].id == id) {
                            rowInFilter = j;
                            break;
                        }
                    }
                    if (rowInFilter != -1) {
                        QModelIndex idx = index(rowInFilter);
                        emit dataChanged(idx, idx, {CompletedRole});
                    }
                }
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

    QJsonDocument doc(tasksArray);
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
    if (!doc.isArray())
        return;

    QJsonArray tasksArray = doc.array();
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
