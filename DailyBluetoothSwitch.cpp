#include "DailyBluetoothSwitch.h"

DailyBluetoothSwitchServer::DailyBluetoothSwitchServer(std::string name){
    BLEClientConnected = false;
    characteristic = NULL;
    std::string baseName = "AmberSmart.Switch";

    BLEDevice::init(baseName);
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);


    BLEService *pService = pServer->createService(SERVICE_UUID);
    characteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
    BLEDescriptor* pSwitchPanelDescription = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
    pSwitchPanelDescription->setValue("Button Press Notification");
    characteristic->addDescriptor(pSwitchPanelDescription);
    characteristic->addDescriptor(new BLE2902());
    characteristic->setValue("{}");
    pService->start();

    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData myAdvertismentData;
    
    myAdvertismentData.setName(baseName + "." + name);
    pAdvertising->setAdvertisementData(myAdvertismentData);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
}

void DailyBluetoothSwitchServer::startAdvertising() {
    BLEDevice::startAdvertising();
}

void DailyBluetoothSwitchServer::sendNotification(uint16_t id, DBSNotificationStates state){
    char buffer[100];
    sprintf(buffer, "{\"id\":%d, \"state\":%d}", id, state);
    characteristic->setValue(buffer);
    
    characteristic->notify();
}

void DailyBluetoothSwitchServer::onConnect(BLEServer* pServer) {
    Serial.println("Connected");
    BLEClientConnected = true;
};

void DailyBluetoothSwitchServer::onDisconnect(BLEServer* pServer) {
    Serial.println("Disconnected");
    BLEClientConnected = false;
}
