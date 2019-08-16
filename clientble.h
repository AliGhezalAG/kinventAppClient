#ifndef ClientBLE_H
#define ClientBLE_H

#include <QLowEnergyController>

#define SERVICE_UUID                "{49535343-fe7d-4ae5-8fa9-9fafd205e455}"
#define CHARACTERISTIC_UUID         "{49535343-1e4d-4bd9-ba61-23c647249616}"

#define MAX_SIZE 20 // 20 octets de données max

class ClientBLE : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool etatConnexion MEMBER m_etatConnexion NOTIFY connecte)
    Q_PROPERTY(float compteur MEMBER m_compteur NOTIFY compteurChange)

public:
    ClientBLE(QString address);
    ~ClientBLE();
    Q_INVOKABLE void start();
//    Q_INVOKABLE void stop();
    Q_INVOKABLE void read();
    Q_INVOKABLE void write(const QByteArray &data);
    Q_INVOKABLE void gererNotification(bool notification);
    bool isConnected();
    bool isActive();
    QLowEnergyController::ControllerState getControllerState();

protected slots:
    void connecterAppareil();
    void connecterService(QLowEnergyService *service);
    void ajouterService(QBluetoothUuid serviceUuid);
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void appareilConnecte();
    void appareilDeconnecte();
    void processDevice();
    void stop();

private:
    QString deviceAddress;
    QList<QObject*>                  m_devices;
    QLowEnergyController            *m_controller;
    QLowEnergyService               *m_service;
    bool                             m_etatConnexion;
    int                              m_compteur;
    QLowEnergyCharacteristic         m_txCharacteristic;
    QLowEnergyCharacteristic         m_rxCharacteristic;
    bool                             clientIsActive;

signals:
    void connecte();
    void doneProcessing();
    void compteurChange();
    void processingFinished();
};

#endif // ClientBLE_H
