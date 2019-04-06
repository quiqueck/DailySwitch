#include "Proximity.h"
#include <SD.h>
#include "SleepTimer.h"

Proximity* Proximity::_global = NULL;

#if PROXIMITY
#include <Adafruit_APDS9960.h>
Adafruit_APDS9960 apds;
#endif

Proximity::Proximity(){
#if HEADLESS 
    setMinProximity(MIN_PROX);
#else
    fs::File defFS;
    // Open requested file on SD card
    defFS = SD.open("Proximity.set", "r");

    if (!defFS)
    {
        setMinProximity(MIN_PROX);
        storeSettings();
        return;
    }

    setMinProximity(defFS.read());
    defFS.close();
#endif
}

void Proximity::storeSettings(){
    fs::File defFS;
    defFS = SD.open("Proximity.set", "w");
    if (defFS){
        Console.println("Initializing Proximity Settings...");
        
        defFS.write(minProximity());
        defFS.close();
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
        apds.setProximityInterruptThreshold(0, MIN_PROX);
        //apds.setLED(APDS9960_LEDDRIVE_100MA, APDS9960_LEDBOOST_300PCNT);
        apds.enableProximityInterrupt();
    }
#endif
}

void Proximity::tick(){
#if PROXIMITY
    if(!digitalRead(PROXIMITY_INT_PIN)){
        uint8_t val = apds.readProximity();
        if (val > MIN_PROX){
            Console.println(val);
            SleepTimer::global()->invalidate();
            apds.clearInterrupt();
        }
    } else {
        //Console.printf("(%d)\n", apds.readProximity());
    }
#endif
}