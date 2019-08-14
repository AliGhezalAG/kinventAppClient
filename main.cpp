#include <QCoreApplication>
#include "clientble.h"
#include "connexionhandler.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ConnexionHandler *connexionHandler = new ConnexionHandler();
    connexionHandler->startDeviceDiscovery();

//    ClientBLE *client = new ClientBLE();
//    client->start();

    return a.exec();
}
