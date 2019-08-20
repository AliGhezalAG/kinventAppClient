#include "clientble.h"
#include <QDebug>
#include <QtEndian>

ClientBLE::ClientBLE(QString address) : deviceAddress(address), m_service(nullptr), m_etatConnexion(false), m_compteur(0), clientIsActive(false), measurementMultiplierSet(false), baselineSet(false)
{
    qDebug() << Q_FUNC_INFO;
    m_controller =  new QLowEnergyController(QBluetoothAddress(address), this);
}

ClientBLE::~ClientBLE()
{
    if (m_controller)
        m_controller->disconnectFromDevice();
    delete m_controller;
    qDeleteAll(m_devices);
    qDebug() << Q_FUNC_INFO;
}

void ClientBLE::start()
{
    clientIsActive = true;
    //    qDebug() << Q_FUNC_INFO << deviceAddress;
    connecterAppareil();
}

void ClientBLE::stop()
{
    //    qDebug() << Q_FUNC_INFO << deviceAddress;
    if (m_controller)
        m_controller->disconnectFromDevice();
}

void ClientBLE::read()
{
    if(m_service && m_txCharacteristic.isValid())
    {
        if (m_txCharacteristic.properties() & QLowEnergyCharacteristic::Read)
        {
            m_service->readCharacteristic(m_txCharacteristic);
            bool ok;
            m_compteur = m_txCharacteristic.value().toHex().toInt(&ok, 16);
            //            qDebug() << Q_FUNC_INFO << m_txCharacteristic.value() << m_compteur;
            //qDebug() << (int)qFromLittleEndian<quint8>(m_txCharacteristic.value().constData());
            emit compteurChange();
        }
    }
}

void ClientBLE::write(const QByteArray &data)
{
    if(m_service && m_rxCharacteristic.isValid())
    {
        if (m_rxCharacteristic.properties() & QLowEnergyCharacteristic::Write)
        {
            //            qDebug() << Q_FUNC_INFO << data;
            if(data.length() <= MAX_SIZE)
                m_service->writeCharacteristic(m_rxCharacteristic, data, QLowEnergyService::WriteWithResponse);
            else
                m_service->writeCharacteristic(m_rxCharacteristic, data.mid(0, MAX_SIZE), QLowEnergyService::WriteWithResponse); // TODO : et les autres octets ?
        }
    }
}

void ClientBLE::gererNotification(bool notification)
{
    if(m_service && m_txCharacteristic.isValid())
    {
        if (m_txCharacteristic.properties() & QLowEnergyCharacteristic::Notify)
        {
            QLowEnergyDescriptor descripteurNotification = m_txCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (descripteurNotification.isValid())
            {
                // active la notification : 0100 ou désactive 0000
                qDebug() << Q_FUNC_INFO << "modification notification" << m_txCharacteristic.uuid().toString() << notification;
                if(notification)
                    m_service->writeDescriptor(descripteurNotification, QByteArray::fromHex("0100"));
                else
                    m_service->writeDescriptor(descripteurNotification, QByteArray::fromHex("0000"));
            }
        }
    }
}

void ClientBLE::connecterAppareil()
{

    // Slot pour la récupération des services
    connect(m_controller, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(ajouterService(QBluetoothUuid)));
    connect(m_controller, SIGNAL(connected()), this, SLOT(appareilConnecte()));
    connect(m_controller, SIGNAL(disconnected()), this, SLOT(appareilDeconnecte()));

    connect(this, SIGNAL(connecte()), this, SLOT(getMeasurementMultiplier()));
    connect(this, SIGNAL(processMeasurementMultiplierFinished()), this, SLOT(getBaseline()));
    connect(this, SIGNAL(processBaselineFinished()), this, SLOT(getData()));

    connect(this, SIGNAL(processingFinished()), this, SLOT(stop()));

    qDebug() << Q_FUNC_INFO << "demande de connexion";
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    m_controller->connectToDevice();
}

void ClientBLE::getMeasurementMultiplier()
{
    QByteArray getDeviceTypeRequest = QByteArray::fromHex("21");
    write(getDeviceTypeRequest);
}

void ClientBLE::getBaseline()
{
    QByteArray getDeviceTypeRequest = QByteArray::fromHex("43");
    write(getDeviceTypeRequest);
}

void ClientBLE::getData()
{
    QByteArray getDeviceTypeRequest = QByteArray::fromHex("67");
    write(getDeviceTypeRequest);
}

void ClientBLE::connecterService(QLowEnergyService *service)
{
    m_service = service;

    if (m_service->state() == QLowEnergyService::DiscoveryRequired)
    {
        // Slot pour le changement d'une caractéristique
        connect(m_service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        // Slot pour la récupération des caractéristiques
        connect(m_service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceDetailsDiscovered(QLowEnergyService::ServiceState)));

        //        qDebug() << Q_FUNC_INFO << "découverte des détails des services";
        m_service->discoverDetails();
    }
}

void ClientBLE::ajouterService(QBluetoothUuid serviceUuid)
{
    //    qDebug() << Q_FUNC_INFO << serviceUuid.toString();
    QLowEnergyService *service = m_controller->createServiceObject(serviceUuid);
    connecterService(service);
}

void ClientBLE::serviceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if (c.uuid().toString() == CHARACTERISTIC_UUID)
    {
        if(!measurementMultiplierSet){
            QByteArray val = QByteArray::fromHex(value.toHex(':'));
            measurementMultiplier = val.toInt()/2000000.0;

            logFile.open ("test.txt", ios::out | ios::app);
            logFile << "Measurement multiplier: " << measurementMultiplier << endl;
            logFile.close();

            measurementMultiplierSet = true;
            emit processMeasurementMultiplierFinished();
        } else if (!baselineSet){
            baseline1 = byteArrayToUint16(value.mid(0,2));
            baseline2 = byteArrayToUint16(value.mid(2,2));

            logFile.open ("test.txt", ios::out | ios::app);
            logFile << "baseline1: " << baseline1 << endl;
            logFile << "baseline2: " << baseline2 << endl;
            logFile.close();

            baselineSet = true;
            emit processBaselineFinished();
        } else {
            qInfo() << "received: " << value.toHex(':') << "global data size : " << receivedData.size();
            receivedData.append(value);
            if (receivedData.size() >= 128000){

                logFile.open ("test.txt", ios::out | ios::app);

                quint32 entry1DataNb = byteArrayToUint32(receivedData.mid(0,3));
                quint32 entry1TimeStamp = byteArrayToUint32(receivedData.mid(3,4));

                logFile << "Number of data bytes: " << entry1DataNb << endl;
                logFile << "Timestamp: " << entry1TimeStamp << endl;

                for (int j = 7; j < static_cast<int>(entry1DataNb) ; j+=4) {
                    double mes1 = (baseline1 - byteArrayToUint16(receivedData.mid(j,2))) * measurementMultiplier;
                    double mes2 = (baseline2 - byteArrayToUint16(receivedData.mid(j+2,2))) * measurementMultiplier;

                    logFile << "Measure1: " << mes1 << " Measure2: " << mes2 << endl;
                }

                logFile.close();
                emit processingFinished();
            }
        }
    }
}

quint32 ClientBLE::byteArrayToUint32(const QByteArray &bytes)
{
    auto count = bytes.size();
    if (count == 0 || count > 4) {
        return 0;
    }
    quint32 number = 0U;
    for (int i = 0; i < count; ++i) {
        auto b = static_cast<quint32>(bytes[count - 1 - i]);
        number += static_cast<quint32>(b << (8 * i));
    }
    return number;
}

quint16 ClientBLE::byteArrayToUint16(const QByteArray &bytes)
{
    auto count = bytes.size();
    if (count == 0 || count > 2) {
        return 0;
    }
    quint16 number = 0U;
    for (int i = 0; i < count; ++i) {
        auto b = static_cast<quint16>(bytes[count - 1 - i]);
        number += static_cast<quint16>(b << (8 * i));
    }
    return number;
}

void ClientBLE::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    Q_UNUSED(newState)

    // décourverte ?
    if (newState != QLowEnergyService::ServiceDiscovered)
    {
        return;
    }

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    //    qDebug() << Q_FUNC_INFO << "service" << service->serviceUuid().toString();

    if (service->serviceUuid().toString() == SERVICE_UUID)
    {
        foreach (QLowEnergyCharacteristic c, service->characteristics())
        {
            //            qDebug() << Q_FUNC_INFO << "characteristic" << c.uuid().toString();
            if (c.uuid().toString() == CHARACTERISTIC_UUID)
            {
                //                qDebug() << Q_FUNC_INFO << "my characteristic TX" << c.uuid().toString() << (c.properties() & QLowEnergyCharacteristic::Notify);
                m_txCharacteristic = c;

                QLowEnergyDescriptor descripteurNotification = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descripteurNotification.isValid())
                {
                    // active la notification : 0100 ou désactive 0000
                    //                    qDebug() << Q_FUNC_INFO << "modification notification" << c.uuid().toString();
                    service->writeDescriptor(descripteurNotification, QByteArray::fromHex("0100"));
                }

                //                qDebug() << Q_FUNC_INFO << "my characteristic RX" << c.uuid().toString() << (c.properties() & QLowEnergyCharacteristic::Write);
                m_service = service;
                m_rxCharacteristic = c;
            }
        }

        m_etatConnexion = true;
        emit connecte();
    }
}

bool ClientBLE::isConnected()
{
    return m_etatConnexion;
}

void ClientBLE::appareilConnecte()
{
    //    qDebug() << Q_FUNC_INFO;
    m_controller->discoverServices();
}

void ClientBLE::appareilDeconnecte()
{
    //    qDebug() << Q_FUNC_INFO;
    m_etatConnexion = false;

    clientIsActive = false;
    emit doneProcessing();
}

bool ClientBLE::isActive()
{
    return clientIsActive;
}

QLowEnergyController::ControllerState ClientBLE::getControllerState()
{
    return m_controller->state();
}
