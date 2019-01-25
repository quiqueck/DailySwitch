#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include "ESP32Setup.h"

#include "TouchPin.h"
#include "SwitchUI.h"
#include "SleepTimer.h"

#include "DailyBluetoothSwitch.h"

TouchPin* t1 = NULL;
DailyBluetoothSwitchServer* dbss = NULL;
SwitchUI* ui = NULL;
SleepTimer* sleepTimer = NULL;

portMUX_TYPE calibMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE btMux = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Daily Switch");

    pinMode(SDCARD_CS, OUTPUT);
    digitalWrite(SDCARD_CS, HIGH);

    ui = new SwitchUI(buttonEvent, touchPanelEvent, false);
    dbss = new DailyBluetoothSwitchServer("001");
    dbss->setConnectionCallback(stateChanged);

    t1 = new TouchPin(T0, forceCalib, 20);
    
	//pinMode(TFT_IRQ, INPUT);
    //attachInterrupt(digitalPinToInterrupt(TFT_IRQ), touchEvent, FALLING);

    SleepTimer::begin(ui);    

    dbss->startAdvertising();

    //esp_sleep_enable_touchpad_wakeup();
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
    //esp_deep_sleep_start();
}

volatile bool triggerStateUpdate = true;
volatile bool triggerCalibration = false;
void stateChanged(bool state){
    //portENTER_CRITICAL_ISR(&btMux);
    triggerStateUpdate = false;    
    //portEXIT_CRITICAL_ISR(&btMux);
}

void loop()
{
    if (!triggerStateUpdate) {
        triggerStateUpdate = true;
        ui->connectionStateChanged(dbss->connectionState());
    }

    if (triggerCalibration) {
        Serial.println("Starting Calibration...");
        SleepTimer::global()->invalidate();
        SleepTimer::global()->stop();
        ui->startTouchCalibration();
        triggerCalibration = false;
        SleepTimer::global()->start();
    }

	t1->read();
    ui->scanTouch();
    SleepTimer::global()->tick();
}

void touchEvent(){
    static int ct = 0;
    Serial.printf("TOUCH event %d\n", ct++);
}

void buttonEvent(uint8_t id, uint8_t state){
    Serial.printf("Sending %d, %d\n", id, state);
    dbss->sendNotification(id, (DailyBluetoothSwitchServer::DBSNotificationStates)state);
}

void touchPanelEvent(bool down){
    if (down) SleepTimer::global()->stop();
    else SleepTimer::global()->start();
}


void forceCalib(uint8_t pin, bool pressed){
    if (pressed=false){
        triggerCalibration = true;        
    }
}
