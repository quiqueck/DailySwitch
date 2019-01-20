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
    t1 = new TouchPin(T0, touchEvent, 20);
    t2 = new TouchPin(T1, forceCalib, 20);
    
	pinMode(TFT_IRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TFT_IRQ), message, CHANGE);

    dbss->startAdvertising();
}

void loop()
{
	t1->read();
    t2->read();
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
    Serial.println("Requested Recalibration");
    ui->startTouchCalibration();
}

void touchEvent(uint8_t pin, bool pressed){
    dbss->sendNotification(
        0, 
        pressed?DailyBluetoothSwitchServer::DBSNotificationStates::ON : DailyBluetoothSwitchServer::DBSNotificationStates::OFF
    );
}
