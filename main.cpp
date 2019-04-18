#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "maze.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    Maze *maze = new Maze();
//    maze->readMazeFile();

//    maze->bfs();

    engine.rootContext()->setContextProperty("maze", &(*maze));
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
