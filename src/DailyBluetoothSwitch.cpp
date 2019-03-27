#include "DailyBluetoothSwitch.h"

DailyBluetoothSwitchServer::DailyBluetoothSwitchServer(std::string name){
    whenConnected = NULL;
    BLEClientConnected = false;
    characteristic = NULL;
    std::string baseName = "AmberSmart.Switch" + name;

    BLEDevice::init(baseName);
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);


    BLEService *pService = pServer->createService(SERVICE_UUID);
    characteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY       
                                       );
    BLEDescriptor* pSwitchPanelDescription = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
    pSwitchPanelDescription->setValue("Button Press Notification");
    characteristic->addDescriptor(pSwitchPanelDescription);

    BLE2902* p2902 = new BLE2902();
    p2902->setNotifications(true);
    p2902->setIndications(true);
    characteristic->addDescriptor(p2902);
    characteristic->setValue("{\"id\":-1, \"state\":0}");
    pService->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData myAdvertismentData;
    
    myAdvertismentData.setCompleteServices(pService->getUUID());
    myAdvertismentData.setName(baseName + "." + name);

    pAdvertising->setAdvertisementData(myAdvertismentData);
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
}

void DailyBluetoothSwitchServer::startAdvertising() {
    Serial.println("Started Advertising...");
    BLEDevice::startAdvertising();
}

void DailyBluetoothSwitchServer::sendNotification(uint16_t id, DBSNotificationStates state){
    char buffer[100];
    snprintf(buffer, 100, "{\"id\":%d, \"state\":%d}", id, state);
    characteristic->setValue(buffer);
    
    characteristic->notify();
}

void DailyBluetoothSwitchServer::onConnect(BLEServer* pServer) {
    Serial.printf("Connected %x\n", pServer);
    BLEClientConnected = true;
    characteristic->setValue("{\"id\":-1, \"state\":0}");
    characteristic->notify();
    if (whenConnected){
        whenConnected(true);
    }
};

void DailyBluetoothSwitchServer::onDisconnect(BLEServer* pServer) {
    Serial.printf("Disconnected %x\n", pServer);
    BLEClientConnected = false;
    if (whenConnected){
        whenConnected(false);
    }
}
