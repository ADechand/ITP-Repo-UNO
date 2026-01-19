#include <iostream>

#include <qvector.h>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <qthread.h>
using namespace std;

class ClientConnection {

public:

    int socket;

    string playerName;

    ClientConnection(int sock) : socket(sock) {}

    void sendMessage(const string& msg) {

        send(socket, msg.c_str(), msg.size(), 0);

    }

    string receiveMessage() {

        char buffer[1024];

        int bytes = recv(socket, buffer, sizeof(buffer)-1, 0);

        if (bytes > 0) {

            buffer[bytes] = '\0';

            return string(buffer);

        }

        return "";

    }

};


int main() {

    Server server(12345);

    if (server.start()) {

        server.acceptConnections();

    }

    return 0;

}

