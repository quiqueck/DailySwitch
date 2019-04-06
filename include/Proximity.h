#ifndef __PROXIMITY_H__
#define __PROXIMITY_H__


#include "ESP32Setup.h"
#include "NopSerial.h"

#define MIN_PROX 95


class Proximity {
    public:
        
        void tick();
        void storeSettings();
        inline uint8_t minProximity() const { return minP; }
        inline uint8_t incProximity() { return changProximity(5); }
        inline uint8_t decProximity() { return changProximity(-5); }
        inline uint8_t isReady() const { return state & 0x01; }
        uint8_t currentValue();

        static inline void begin(){
            if (Proximity::_global == NULL){
                Proximity::_global = new Proximity();
                Proximity::_global->doBegin();
            }
        }
        static inline Proximity* global() { return Proximity::_global; }
    private:
        Proximity();
        void doBegin();
        void updateInterrupt();

        static Proximity* _global;
        uint8_t state;
        uint8_t minP;
        inline void markReady() { 
            state = state | 0x01;            
        }
        inline void setMinProximity(uint8_t minp) { 
            minP = minp;
            updateInterrupt();
        }
        uint8_t changProximity(int8_t delta){
            int v = minProximity();
            v += delta;
            if (v>0xff) v= 0xff;
            else if (v<0) v = 0;
            setMinProximity(v);
            return v;
        }
};


#endif