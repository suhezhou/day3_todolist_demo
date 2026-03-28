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

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("todoModel", &todoModel);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("todo_list_demo", "Main");

    return app.exec();
}
