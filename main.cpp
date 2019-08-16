#include <QCoreApplication>
#include "clientble.h"
#include "connexionhandler.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ConnexionHandler *connexionHandler = new ConnexionHandler();
    connexionHandler->start();

    return a.exec();
}
