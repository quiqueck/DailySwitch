#ifndef __WEATHER_H__
#define __WEATHER_H__
#include <Arduino.h>
#include "WeatherKey.h"

#define WEATHER

class Weather {
    public:
        static inline bool begin(class SwitchUI* uiIn, std::string key=WEATHER_KEY){
            if (Weather::_global == NULL){
                Weather::_global = new Weather(key, uiIn);
            }
            return Weather::_global->ready();
        }
        static inline Weather* global() { return Weather::_global; }
        inline bool ready() const { return status==0; }

        void tick();
        void update();
    private:
        Weather(std::string key, class SwitchUI* ui);

        std::string key;
        class SwitchUI* ui;
        static Weather* _global;
        uint8_t status;
};
#endif