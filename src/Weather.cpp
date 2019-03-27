#include "Weather.h"
#include "SwitchUI.h"
#include <WiFi.h>
#include <soc/rtc.h>

const char* ssid     = WIFI_NAME;
const char* password = WIFI_ACC;

#define SECOND(__s__) (__s__*1000*1000)
#define MINUTE(__m__) (__m__*1000*1000*60)

bool trippedTimer = false;
hw_timer_t * wtimer = NULL;
portMUX_TYPE wtimerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onWeatherTimer() {
  portENTER_CRITICAL_ISR(&wtimerMux);
  trippedTimer = true;  
  portEXIT_CRITICAL_ISR(&wtimerMux);
}

Weather* Weather::_global = NULL;
Weather::Weather(std::string keyIn, SwitchUI* uiIn){
    key = keyIn;
    ui = uiIn;
    wtimer = timerBegin(1, 80, true);
    
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    uint8_t repeat = 100;
    while (WiFi.status() != WL_CONNECTED && repeat>0) {
        delay(500);        
        Serial.print(".");
        repeat--;
    }
    if (WiFi.status() != WL_CONNECTED){
        status = 100;
        Serial.println("");
        Serial.println("WiFi connection failed");
    } else {
        status = 0;
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        update();
    }

    timerAttachInterrupt(wtimer, &onWeatherTimer, true);
    timerAlarmWrite(wtimer, MINUTE(3), true);
    timerAlarmEnable(wtimer);
}

void Weather::update(){
    if (!ready()) return;
    Serial.printf("Updating Weather Info...\n");
}

void Weather::tick(){
    portENTER_CRITICAL_ISR(&wtimerMux);
    bool didTrip = trippedTimer;
    trippedTimer = false;  
    portEXIT_CRITICAL_ISR(&wtimerMux);
    
    if (didTrip){
        update();
    }
}