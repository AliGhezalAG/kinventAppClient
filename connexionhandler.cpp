#include "connexionhandler.h"

ConnexionHandler::ConnexionHandler()
{
    std::time_t result = std::time(nullptr);
    string logTitle = std::ctime(&result);
    logFile.open (logTitle + ".log", ios::out | ios::app);
    logFile << logTitle << endl;

    discoveredDevicesList = {};
    clientList = {};

    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &ConnexionHandler::addDevice);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &ConnexionHandler::processDevices);
    connect(m_deviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
}

ConnexionHandler::~ConnexionHandler()
{
    delete m_deviceDiscoveryAgent;
    discoveredDevicesList.clear();
    qDeleteAll(clientList);
    clientList.clear();
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

void ConnexionHandler::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        discoveredDevicesList.append(device.address().toString());
        qInfo() << device.address().toString();
        qInfo() << device.name();
    }
}

void ConnexionHandler::start()
{
    while(true){
        discoveredDevicesList.clear();
        qDeleteAll(clientList);
        clientList.clear();
        m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        {
            QEventLoop loop;
            qInfo() << "start of loop";
            loop.connect(this, SIGNAL(scanProcessingEnded()), SLOT(quit()));
            m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
            loop.exec();
        }
        qInfo() << "end of loop";
    }
}

void ConnexionHandler::processDevices()
{
    for(int i=0; i < discoveredDevicesList.size(); i++){
        if(devicesList.contains(discoveredDevicesList.at(i))){
            qInfo() << "processing " << discoveredDevicesList.at(i);
            ClientBLE *client = new ClientBLE(discoveredDevicesList.at(i));
            {
                QEventLoop loop;
                loop.connect(client, SIGNAL(doneProcessing()), SLOT(quit()));
                client->start();
                loop.exec();
            }
        }
    }
    qInfo() << "this is the end dadada!";
    emit scanProcessingEnded();
}
