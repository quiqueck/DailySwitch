#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include "ESP32Setup.h"

#include "TouchPin.h"
#include "SwitchUI.h"

#include "DailyBluetoothSwitch.h"

TouchPin* t1 = NULL;
TouchPin* t2 = NULL;
DailyBluetoothSwitchServer* dbss = NULL;
SwitchUI* ui = NULL;
void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Daily Switch");

    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);

    ui = new SwitchUI(buttonEvent, false);
    dbss = new DailyBluetoothSwitchServer("001");
    dbss->setConnectionCallback(stateChanged);

    t1 = new TouchPin(T0, forceCalib, 20);
    t2 = new TouchPin(T1, touchEvent, 20);
    
	pinMode(TFT_IRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TFT_IRQ), message, CHANGE);

    dbss->startAdvertising();

    //esp_sleep_enable_touchpad_wakeup();
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
    //esp_deep_sleep_start();
}

bool triggerStateUpdate = false;
bool triggerCalibration = false;
void stateChanged(bool state){
    triggerStateUpdate = true;    
}

void loop()
{
    if (triggerStateUpdate) {
        triggerStateUpdate = false;
        ui->connectionStateChanged(dbss->connectionState());
    }

    if (triggerCalibration) {
        ui->startTouchCalibration();
        triggerCalibration = false;
    }

	t1->read();
    //t2->read();
    ui->scanTouch();
}

void message(){
    //Serial.println("TOUCH");
}

void buttonEvent(uint8_t id, uint8_t state){
    Serial.printf("Sending %d, %d\n", id, state);
    dbss->sendNotification(id, (DailyBluetoothSwitchServer::DBSNotificationStates)state);
}


void forceCalib(uint8_t pin, bool pressed){
    triggerCalibration = true;
    Serial.println("Requested Recalibration");    
}

void touchEvent(uint8_t pin, bool pressed){
    dbss->sendNotification(
        0, 
        pressed?DailyBluetoothSwitchServer::DBSNotificationStates::ON : DailyBluetoothSwitchServer::DBSNotificationStates::OFF
    );
}
