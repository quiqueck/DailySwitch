#include "Proximity.h"
#include <SD.h>
#include "SleepTimer.h"
#include "FileSystem.h"
#define PROXIMITY_FILE "/proximityData"

Proximity* Proximity::_global = NULL;

#if PROXIMITY
#include <Adafruit_APDS9960.h>
Adafruit_APDS9960 apds;
#endif

Proximity::Proximity(){
    state = 0;
    minP = 0;
#if HEADLESS 
    setMinProximity(MIN_PROX);
#else
    FileSystem::init();
    fs::File defFS;

    if (SD.exists(PROXIMITY_FILE)) {
        // Open requested file on SD card
        defFS = SD.open(PROXIMITY_FILE, "r");
        if (!defFS){
            Serial.println("Failed to open Proximity Settings.");
            return;
        }
        setMinProximity(defFS.read());
        defFS.close();
    } else {
        setMinProximity(MIN_PROX);
        storeSettings();
        return;
    }

    
#endif
}

void Proximity::storeSettings(){
    fs::File defFS;
    defFS = SD.open(PROXIMITY_FILE, "w");
    if (defFS){
        Console.println("Initializing Proximity Settings...");
        
        defFS.write(minProximity());
        defFS.close();
    } else {
        Serial.println("Failed to initialize Proximity Settings.");
    }
}

void Proximity::doBegin(){
#if PROXIMITY
    if(!apds.begin(10, APDS9960_AGAIN_64X, APDS9960_ADDRESS, &Wire, IC2_DAT, IC2_CLK, IC2_FREQUENCY)){
        Serial.println(F("Failed to initialize Proximity! Please check your wiring."));
    } else {
        pinMode(PROXIMITY_INT_PIN , INPUT_PULLUP);
        Serial.println(F("Proximity available"));
        apds.enableProximity(true);
        apds.setProxPulse(APDS9960_PPULSELEN_32US, 64);
        markReady();
        updateInterrupt();
        //apds.setLED(APDS9960_LEDDRIVE_100MA, APDS9960_LEDBOOST_300PCNT);
        apds.enableProximityInterrupt();        
    }
#endif
}

void Proximity::updateInterrupt(){
#if PROXIMITY
    if (isReady()){
        apds.setProximityInterruptThreshold(0, minProximity());
    }
#endif
}

uint8_t Proximity::currentValue(){
    #if PROXIMITY
    if (isReady()){
        return apds.readProximity();
    }
#endif
}

void Proximity::tick(){
#if PROXIMITY
    if(!digitalRead(PROXIMITY_INT_PIN)){
        uint8_t val = apds.readProximity();
        if (val > minProximity()){
            Console.println(val);
            SleepTimer::global()->invalidate();
            apds.clearInterrupt();
        }
    } else {
        //Console.printf("(%d)\n", apds.readProximity());
    }
#endif
}