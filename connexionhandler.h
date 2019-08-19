#ifndef CONNEXIONHANDLER_H
#define CONNEXIONHANDLER_H

#include <QObject>
#include <QDebug>
#include <qbluetoothdevicediscoveryagent.h>
#include "clientble.h"
#include <QEventLoop>

#include <chrono>
#include <ctime>

#define KFOREC_ADDRESS              "80:1F:12:B1:3C:D7"

class ConnexionHandler : public QObject
{
    Q_OBJECT

public:
    ConnexionHandler();
    ~ConnexionHandler();
    QList<QString> devicesList = {"80:1F:12:B1:3C:D7", "80:1F:12:B1:3A:C8", "80:1F:12:B1:3C:D2", "80:1F:12:B1:3C:B8"};
    QList<QString> discoveredDevicesList;
    void start();
    ofstream logFile;

protected slots:
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void addDevice(const QBluetoothDeviceInfo &device);
    void processDevices();

private:
    QList<QObject*> devices;
    QList<ClientBLE*> clientList;
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;

signals:
    void scanProcessingEnded();
};

#endif // CONNEXIONHANDLER_H
