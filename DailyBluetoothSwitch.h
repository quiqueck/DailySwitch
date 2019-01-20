#ifndef __DAILY_BLUETOOTH_SWITCH_H__
#define __DAILY_BLUETOOTH_SWITCH_H__
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <HardwareSerial.h>
#include <BLE2902.h>

#define SERVICE_UUID        "0818c2dd-1276-4105-9e65-0d10c8e27e35"
#define CHARACTERISTIC_UUID "3e989a75-3437-471e-a50a-20a838afcce7"
typedef enum:uint8_t {SINGLE_PRESS=1, DOUBLE_PRESS=0, LONG_PRESS=2, ON=1, OFF=0, ON_SECONDARY=2} DBSNotificationStates;

class DailyBluetoothSwitchServer : public BLEServerCallbacks {
    public:
        DailyBluetoothSwitchServer(std::string name);
        void startAdvertising();
        void sendNotification(uint16_t id, DBSNotificationStates state);
    private:
        BLECharacteristic *characteristic;
    public: //BLEServerCallbacks
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);
    private: //BLEServerCallbacks
        bool BLEClientConnected;
};

#endif