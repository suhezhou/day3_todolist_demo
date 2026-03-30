#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "todomodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setOrganizationName("MyCompany");
    QGuiApplication::setApplicationName("TodoListDemo");
    QGuiApplication app(argc, argv);

    TodoModel todoModel;
    // 在 main 函数中，创建引擎前
    qmlRegisterUncreatableType<TodoModel>("TodoModel", 1, 0, "TodoModel", "Cannot create");
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("todoModel", &todoModel);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("todo_list_demo", "Main");

    return app.exec();
}
