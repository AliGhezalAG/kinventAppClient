#include "kinventclient.h"

KinventClient::KinventClient()
{
    deviceHandler = new Device();
    connect(deviceHandler, &Device::updateChanged, this, &KinventClient::displayUpdate);
    connect(deviceHandler, &Device::scanFinished, this, &KinventClient::displayUpdate);
    connect(deviceHandler, &Device::servicesScanFinished, this, &KinventClient::displayUpdate);
}

void KinventClient::displayUpdate()
{
    qInfo() << deviceHandler->getUpdate() << endl;
}

void KinventClient::startClient()
{
    deviceHandler->startDeviceDiscovery();
}
