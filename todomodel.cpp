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

    // 找到插入位置（按 dueDate 排序，可选，这里简单追加）
    // 但为了保持模型一致性，我们直接追加到末尾
    int insertPos = m_items.size();
    beginInsertRows(QModelIndex(), insertPos, insertPos);
    m_items.append({getNextId(), title, dueDate, QDate::currentDate(), false});
    endInsertRows();

    // 如果新任务的日期正好是当前显示的日期，需要刷新列表显示
    if (dueDate == m_currentDate) {
        // 由于我们追加了行，但 rowCount 基于过滤后的结果，需要通知视图数据变化
        // 更好的方式是使用 dataChanged，但 rowCount 已经变化，简单重置模型
        beginResetModel();
        endResetModel();
    } else {
        // 不是当前日期，不需要刷新视图，但保存文件仍然需要
    }
    saveToFile();
}

void TodoModel::removeItem(int id)
{
    int row = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            row = i;
            break;
        }
    }
    if (row == -1)
        return;

    // 标准删除（底层数据删除）
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();

    // 🔥 关键：强制刷新当前日期的显示列表
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    // 🔥 终极保险：强制整个模型重新刷新
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
