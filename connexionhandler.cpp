#include "connexionhandler.h"

ConnexionHandler::ConnexionHandler()
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    discoveryAgent->setLowEnergyDiscoveryTimeout(0);

    connect(discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(processDiscoveredDevice(const QBluetoothDeviceInfo&)));
    connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(deviceScanFinished()));
}

ConnexionHandler::~ConnexionHandler()
{
    delete discoveryAgent;
    qDeleteAll(devices);
    devices.clear();
}

void ConnexionHandler::startDeviceDiscovery()
{
    qDeleteAll(devices);
    devices.clear();

    qInfo() << "Scanning for KForce devices ..." << endl;
    //! [les-devicediscovery-2]
    discoveryAgent->start();
    //! [les-devicediscovery-2]

}

void ConnexionHandler::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qInfo() << "The Bluetooth adaptor is powered off, power it on before doing discovery." << endl;
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qInfo() << "Writing or reading from the device resulted in an error." << endl;
    else
        qInfo() << "An unknown error has occurred." << endl;
}

void ConnexionHandler::processDiscoveredDevice(const QBluetoothDeviceInfo& newDevice)
{
    qInfo() << "new device discovered" << endl;
    if (newDevice.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        if (newDevice.name().contains("KFORCE", Qt::CaseInsensitive)){
            ClientBLE *client = new ClientBLE(newDevice.address().toString());
            client->start();
        }
    }
    // TODO add device to devices list only if new
}
