#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "linkservice.h"
#include "gameclient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    LinkService linkService;
    engine.rootContext()->setContextProperty("linkService", &linkService);

    GameClient gameClient;
    engine.rootContext()->setContextProperty("gameClient", &gameClient);

    engine.loadFromModule("StartTest", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
