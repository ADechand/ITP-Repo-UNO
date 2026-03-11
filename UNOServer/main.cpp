#include <QCoreApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //Instanziert den Server und Führt die App aus
    Server server;
    return a.exec();
}
