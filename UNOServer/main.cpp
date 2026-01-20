#include <iostream>

#include <qvector.h>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <qthread.h>
#include <QtCore>
#include <server.h>
using namespace std;




int main() {

    Server server(12345);

    if (server.start()) {

        server.acceptConnections();
        qDebug() << "Hallo";

    }

    return 0;

}

