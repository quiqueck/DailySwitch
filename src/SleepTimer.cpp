#include "SleepTimer.h"
#include "SwitchUI.h"
#include <Arduino.h>
#include <soc/rtc.h>

#define SECOND(__s__) (__s__*1000*1000)

volatile int interruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}


SleepTimer* SleepTimer::_global = NULL;
SleepTimer::SleepTimer(class SwitchUI* uiIn):ui(uiIn){
    interruptCounter = 0;
    state = 0;
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, SECOND(10), true);
    timerAlarmEnable(timer);
}

void SleepTimer::stop(){
    Serial.println(F("Stopping SleepTimer"));
    timerAlarmDisable(timer);
}

void SleepTimer::start(){
    Serial.println(F("Starting SleepTimer"));
    timerWrite(timer, 0);
    timerAlarmEnable(timer);
}

void SleepTimer::restart(){
    timerRestart(timer);
}

const uint8_t SleepTimer::reduceBrightnessAt = 3;
const uint8_t SleepTimer::noBacklightAt = 6;
const uint8_t SleepTimer::displayOffAt = 9;

//unsigned long lastStateChange = 0;
void SleepTimer::setState(uint8_t s) {
    
    if (s==0){       
        if (state >= displayOffAt){
            ui->displayOn();
        } 
        if (state >= noBacklightAt){
            rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
            //timerSetDivider(timer, 240);
        } 
        if (state >= reduceBrightnessAt) {
            ui->setBrightness(0xFF);
        }
    } else if (s>=reduceBrightnessAt && state<reduceBrightnessAt) {
        Serial.print(F("Reducing Brightness \n"));
        ui->setBrightness(0x10);
        ui->returnToNormalState();
    } else if (s>=noBacklightAt && state<noBacklightAt) {
        Serial.print(F("Turn off Backlight \n"));
        ui->setBrightness(0x00);
        rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
        //timerSetDivider(timer, 80);
        ui->returnToNormalState();
    } else if (s>=displayOffAt && state<displayOffAt) {
        Serial.print(F("Turn off Display \n"));
        ui->reloadMainPage();
        ui->displayOff();        
    } else if (s>=6*5 && state<6*5) {
        /*esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
        Serial.println(F("Going to sleep now...\n"));
        esp_deep_sleep_start();*/
    }

    //Serial.printf("state: %d => %d (%dms)\n", state, s, millis()-lastStateChange);    
    //lastStateChange = millis();
    state = s;
}

void SleepTimer::tick(){
    portENTER_CRITICAL_ISR(&timerMux); 
    uint8_t newState = min(0xff, state+interruptCounter);
    interruptCounter = 0;
    portEXIT_CRITICAL_ISR(&timerMux);
    
    if (state!=newState)
        setState(newState);        
}
