#include <QCoreApplication>
#include "kinventclient.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    KinventClient *client = new KinventClient();
    client->startClient();

    return a.exec();
}
