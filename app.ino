#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include "ESP32Setup.h"
#include "DailyBluetoothSwitch.h"
#include "TouchPin.h"
#include "SwitchUI.h"

TouchPin* t1 = NULL;
DailyBluetoothSwitchServer* dbss = NULL;
SwitchUI* ui = NULL;
void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Daily Switch");

    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);

    ui = new SwitchUI(true);
    dbss = new DailyBluetoothSwitchServer("001");
    t1 = new TouchPin(T0, touchEvent, 20);
    
	pinMode(TFT_IRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TFT_IRQ), message, CHANGE);

    dbss->startAdvertising();
}

void loop()
{
	t1->read();
}

void message(){
    Serial.println("TOUCH");
}

void touchEvent(uint8_t pin, bool pressed){
    dbss->sendNotification(
        0, 
        pressed?DBSNotificationStates::ON : DBSNotificationStates::OFF
    );
}
