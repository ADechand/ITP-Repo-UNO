#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDirIterator>
#include <QDebug>
#include <QQmlContext>
#include "linkservice.h"

int main(int argc, char *argv[])
{    

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;


    LinkService linkService;
    engine.rootContext()->setContextProperty("linkService", &linkService);

    // Lädt Main.qml aus dem QML-Modul "StartTest"
    engine.loadFromModule("StartTest", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
