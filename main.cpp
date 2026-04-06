#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "todomodel.h"

#ifndef TODO_PROJECT_DIR
#define TODO_PROJECT_DIR "."
#endif

int main(int argc, char *argv[])
{
    QGuiApplication::setOrganizationName("MyCompany");
    QGuiApplication::setApplicationName("TodoListDemo");
    QGuiApplication app(argc, argv);

    const QString dataDir = QDir(QStringLiteral(TODO_PROJECT_DIR)).filePath("data");
    QDir dir(dataDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        qCritical() << "Failed to create database directory:" << dataDir;
        return -1;
    }

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(dir.filePath("todo.db"));
    if (!database.open()) {
        qCritical() << "Failed to open database:" << database.lastError().text();
        return -1;
    }

    QSqlQuery query(database);
    if (!query.exec("CREATE TABLE IF NOT EXISTS tasks ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "title TEXT NOT NULL, "
                    "completed INTEGER NOT NULL DEFAULT 0, "
                    "dueDate TEXT NOT NULL, "
                    "createdDate TEXT NOT NULL)")) {
        qCritical() << "Failed to create tasks table:" << query.lastError().text();
        return -1;
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS app_meta ("
                    "key TEXT PRIMARY KEY, "
                    "value TEXT NOT NULL)")) {
        qCritical() << "Failed to create metadata table:" << query.lastError().text();
        return -1;
    }

    TodoModel todoModel;
    qmlRegisterUncreatableType<TodoModel>("TodoModel", 1, 0, "TodoModel", "Cannot create");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("todoModel", &todoModel);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("todo_list_demo", "Main");

    return app.exec();
}
