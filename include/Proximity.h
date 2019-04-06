#ifndef __PROXIMITY_H__
#define __PROXIMITY_H__


#include "ESP32Setup.h"
#include "NopSerial.h"

#define MIN_PROX 95


class Proximity {
    public:
        
        void tick();
        void storeSettings();
        inline uint8_t minProximity() const { return state & 0x0F; }
        inline uint8_t incProximity() { return changProximity(10); }
        inline uint8_t decProximity() { return changProximity(-10); }

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

        static Proximity* _global;
        uint16_t state;
        inline void setMinProximity(uint8_t minp) { state = (state & 0xF0) | minp;}
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