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
            return true;
        }
        static inline Weather* global() { return Weather::_global; }        

        void tick();
        void update();

        inline bool hasValidData() const { return hasData; }
        float temperature() const;
        float pressure() const;
        float humidity() const;
        std::string icon() const;
    protected:
        void startWiFi();
        void stopWiFi();
        void readData();
    private:
        enum WeatherUpdateState : uint8_t {IDLE=0, CONNECTING=1, CONNECTED=2, LOADING=3, INIT=4};
        Weather(std::string key, class SwitchUI* ui);

        std::string key;
        class SwitchUI* ui;
        static Weather* _global;
        WeatherUpdateState state;
        uint16_t lastUpdateCall;
        uint8_t wifiRetries;
        bool hasData;
};
#endif