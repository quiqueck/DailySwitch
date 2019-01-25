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
    Serial.println("Stopping SleepTimer");
    timerAlarmDisable(timer);
}

void SleepTimer::start(){
    Serial.println("Starting SleepTimer");
    timerWrite(timer, 0);
    timerAlarmEnable(timer);
}

void SleepTimer::restart(){
    timerRestart(timer);
}

long lastStateChange = 0;
void SleepTimer::setState(uint8_t s) {
    if (s==0){       
        if (state >= 3){
            ui->displayOn();
        } 
        if (state >= 2){
            rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
            //timerSetDivider(timer, 240);
        } 
        if (state >= 1) {
            ui->setBrightness(0xFF);
        }
    } else if (s>=1 && state<1) {
        Serial.print("Reducing Brightness ");
        ui->setBrightness(0x10);
    } else if (s>=2 && state<2) {
        Serial.print("Turn off Backlight ");
        ui->setBrightness(0x00);
        rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
        //timerSetDivider(timer, 80);
    } else if (s>=3 && state<3) {
        Serial.print("Turn off Display ");
        ui->displayOff();
    } else if (s>=6*5 && state<6*5) {
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, LOW);
        Serial.println("Going to sleep now...");
        esp_deep_sleep_start();
    }

    Serial.printf("state: %d => %d (%d)\n", state, s, micros()-lastStateChange);    
    lastStateChange = micros();
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
