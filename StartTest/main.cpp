#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "linkservice.h"
#include "gameclient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    //Erstellt eine Instanz von LinkService
    LinkService linkService;
    engine.rootContext()->setContextProperty("linkService", &linkService);

    //Erstellt eine Instanz von GameClient
    GameClient gameClient;
    engine.rootContext()->setContextProperty("gameClient", &gameClient);

    engine.loadFromModule("StartTest", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    //Führt die App aus
    return app.exec();
}
