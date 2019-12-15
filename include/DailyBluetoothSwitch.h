#ifndef __DAILY_BLUETOOTH_SWITCH_H__
#define __DAILY_BLUETOOTH_SWITCH_H__
#include <Arduino.h>
#include "ESP32Setup.h"

#if USE_BT==1
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <HardwareSerial.h>
#include <BLE2902.h>
#else
typedef void BLEServer;
typedef void BLECharacteristic;

#include <WiFi.h>
#endif

#include <functional>
#include "NopSerial.h"

#define SERVICE_UUID        "0818c2dd-1276-4105-9e65-0d10c8e27e35"
#define CHARACTERISTIC_UUID "3e989a75-3437-471e-a50a-20a838afcce7"


class DailyBluetoothSwitchServer 
#if USE_BT==1
: public BLEServerCallbacks 
#endif
{
    public:
        enum DBSNotificationStates :uint8_t {SINGLE_PRESS=1, DOUBLE_PRESS=0, LONG_PRESS=2, ON=1, OFF=0, ON_SECONDARY=2};
    
        DailyBluetoothSwitchServer(std::string name);
        void startAdvertising();
        void sendNotification(uint16_t id, DBSNotificationStates state);
        inline void setConnectionCallback(std::function<void(bool)> cb) {
            whenConnected = cb;
        }

        inline bool connectionState() { return BLEClientConnected; }
    private:
    #if USE_BT==1
        BLECharacteristic *characteristic;
    #else
        String baseName;
        uint16_t lastUpdateCall;
        uint8_t wifiRetries;
        enum SwitchUpdateState : uint8_t {IDLE=0, CONNECTING=1, CONNECTED=2, INIT=4};
        SwitchUpdateState state;

        void update();
    #endif 
    public: //BLEServerCallbacks
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);
        void tick();
    private: //BLEServerCallbacks
        bool BLEClientConnected;
        std::function<void(bool)> whenConnected;
};

#endif