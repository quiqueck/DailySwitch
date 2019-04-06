#ifndef __SLEEP_TIMER_H__
#define __SLEEP_TIMER_H__
#include <Arduino.h>
#include "NopSerial.h"

class SleepTimer {
    public:
        static inline void begin(class SwitchUI* uiIn){
            if (SleepTimer::_global == NULL){
                SleepTimer::_global = new SleepTimer(uiIn);
            }
        }
        static inline SleepTimer* global() { return SleepTimer::_global; }
        inline void invalidate() { if (state!=0) setState(0); }
        inline const uint8_t currentState() { return this->state; }
        inline const bool reduceBrightness() { return this->state>=reduceBrightnessAt; }
        inline const bool noBacklight() { return this->state>=noBacklightAt; }
        inline const bool displayOff() { return this->state>=displayOffAt; }
        void tick();
        void restart();
        void stop();
        void start();
        void sleepNow();

    private:
        static const uint8_t reduceBrightnessAt;
        static const uint8_t noBacklightAt;
        static const uint8_t displayOffAt;

        SleepTimer(class SwitchUI* uiIn);
        void setState(uint8_t s);

        static SleepTimer* _global;
        class SwitchUI* ui;
        uint8_t state;
};

#endif