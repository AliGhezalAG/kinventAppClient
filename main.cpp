#include <QCoreApplication>
#include "clientble.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ClientBLE *client = new ClientBLE();
    client->start();

    return a.exec();
}
