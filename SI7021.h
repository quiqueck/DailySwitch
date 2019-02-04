#ifndef __SI7021_H__
#define __SI7021_H__
#include <Arduino.h>
#include <cmath>


class SI7021 {
    public: 
        SI7021(int addr=0x40) : addr(addr){
            readState = ReadState::Initialize;
            last = millis();
            lastTemperature = NAN;
            lastHumidity = NAN;
        }  

        bool begin();
        //read directly from the bus (will delay until info is available)
        float readHumidity();
        float readTemperature();

    public:
        //read periodically 
        bool update(unsigned long ms_interval = 1000 * 30 * 1);

        inline bool const hasValidTemperature() { return !isnan(SI7021::lastTemperature); }
        inline bool const hasValidHumidity() { return !isnan(SI7021::lastHumidity); }

        inline float const temperature() { return lastTemperature; }
        inline float const humidity() { return lastHumidity; }

        const int addr;        
    private:
        enum ReadState:uint8_t {
            Waiting             =0x00,
            Initialize          =0x10,
            StartRequestTemp    =0x10,
            RequestedTemp       =0x11,
            StartRequestHum     =0x20,
            RequestedHum        =0x21
        };

        unsigned long last;
        float lastTemperature;
        float lastHumidity;
        ReadState readState;

        inline float const calcHumidity(uint16_t raw) {
            return  raw==0 ? NAN : ((125 * raw) / 65536.0) - 6;    
        }

        inline float const calcTemperature(uint16_t raw) {
            return  raw==0 ? NAN :  ((175.72 * raw) / 65536.0) - 46.85;    
        }

        uint8_t const readByte(uint8_t reg);
        uint16_t const readShort(uint8_t reg);
};


#endif