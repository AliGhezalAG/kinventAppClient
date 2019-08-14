#ifndef CONNEXIONHANDLER_H
#define CONNEXIONHANDLER_H

#include<QObject>
#include <QDebug>
#include <qbluetoothdevicediscoveryagent.h>
#include "clientble.h"

#define KFOREC_ADDRESS              "80:1F:12:B1:3C:D7"

class ConnexionHandler : public QObject
{
public:
    ConnexionHandler();
    ~ConnexionHandler();
    Q_INVOKABLE void startDeviceDiscovery();

protected slots:
    void lookForDevices();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void processDiscoveredDevice(const QBluetoothDeviceInfo& newDevice);

private:
    QList<QObject*> devices;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
};

#endif // CONNEXIONHANDLER_H
