#ifndef __TOUCH_PIN_H__
#define __TOUCH_PIN_H__
#include <Arduino.h>
#include <HardwareSerial.h>
#include <functional>
#include "NopSerial.h"

class TouchPin {
    public:
        TouchPin(uint8_t pin, std::function<void(uint8_t, bool)> pressRoutine, uint8_t samples=100);
        void read();
    private:
        bool hadTouch;
        long lastEvent;
        const uint8_t pin;
        std::function<void(uint8_t, bool)> pressRoutine;
        const uint8_t samples;
};
#endif