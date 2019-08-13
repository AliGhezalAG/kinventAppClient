#ifndef KINVENTCLIENT_H
#define KINVENTCLIENT_H

#include <QObject>
#include "device.h"

using namespace std;

class KinventClient : public QObject
{

private slots:
    void displayUpdate();

public:
    KinventClient();
    void startClient();

private:
    Device *deviceHandler;
};

#endif // KINVENTCLIENT_H
