#ifndef __BUTTON_H__
#define __BUTTON_H__
#include <Arduino.h>
#include <cmath>
#include "DailyBluetoothSwitch.h"
#include "NopSerial.h"

enum ButtonType :uint8_t {SWITCH=0, PAGE=1, SELECT=2, SPECIAL_FUNCTION=3};

class Button {
    public: 
        inline uint16_t  l() const{
            return l_state >> 3;
        }
        inline uint16_t r() const{
            return r_altState >> 3;
        } 

        inline uint16_t  t() const{
            return ((l_state >> 2) & 1) | (tHi << 1);
        }
        inline uint16_t b() const{
            return ((r_altState >> 2) & 1) | (bHi << 1);
        }     

        inline DailyBluetoothSwitchServer::DBSNotificationStates state() const {
            return (DailyBluetoothSwitchServer::DBSNotificationStates) (l_state & 0x03);
        }

        inline DailyBluetoothSwitchServer::DBSNotificationStates altState() const {
            return (DailyBluetoothSwitchServer::DBSNotificationStates) (r_altState & 0x03);
        }
            
        const uint16_t col;
        const uint8_t id;
            //const DailyBluetoothSwitchServer::DBSNotificationStates state;
            //const DailyBluetoothSwitchServer::DBSNotificationStates altState;            

    private:
        const uint16_t l_state;
        const uint16_t r_altState;
        const uint8_t tHi;
        const uint8_t bHi; 
        /*union {
            struct{
                bool pressed         : 1; 
                bool hasAlt          : 1; 
                ButtonType type      : 2;
                uint8_t page         : 4;                
            };
            uint8_t val;
        } currentState;*/

        
        uint8_t currentState;

    public:

        inline ButtonType type() const{
            return (ButtonType)((currentState >> 2) & 0x03);
        }
        inline uint8_t page() const{
            return ((currentState >> 4) & 0x0F);
        }
        inline int16_t w() const { return r()-l();}
        inline int16_t h() const { return b()-t();}
        inline bool const inside(int16_t x, int16_t y){ return std::signbit((l()-x) & (x-r()) & (t()-y) & (y-b()));}

        inline void draw(class SwitchUI* ui) const {draw(ui, col);}
        void draw(class SwitchUI* ui, uint16_t oCol) const;
        inline bool isPressed() const { return (currentState & 0x01); }
        inline void toogle() { currentState ^= 0x01; }
        inline void down() {currentState |= 0x01;}
        inline void up() {currentState &= 0xFE;}
        inline bool hasAlternative() const { return (currentState & 0x02); }
        inline DailyBluetoothSwitchServer::DBSNotificationStates  activeState() const { 
            if (!hasAlternative()) return state();            
            return isPressed()?altState():state(); 
        }

        Button(
            uint8_t id, 
            DailyBluetoothSwitchServer::DBSNotificationStates state,  
            const char* name, 
            int16_t lIn, 
            int16_t tIn, 
            int16_t rIn, 
            int16_t bIn, 
            uint16_t colIn,
            uint8_t page=0,
            ButtonType type=ButtonType::SWITCH):
                col(colIn), 
                id(id),
                l_state((lIn<<3) | (state & 0x03) | ((tIn&1)<<2)),
                r_altState((rIn<<3) | (state & 0x03) | ((bIn&1)<<2)),
                tHi(tIn>>1),
                bHi(bIn>>1)                
        {
            this->ui = ui;
            currentState = 0x00 | ((type & 0x03) << 2) | ((page & 0x0F) << 4);
            /*currentState.hasAlt = false;
            currentState.pressed = false;
            currentState.type = type;
            currentState.page = page;*/
        }

        Button(
            uint8_t id, 
            DailyBluetoothSwitchServer::DBSNotificationStates state, 
            DailyBluetoothSwitchServer::DBSNotificationStates altState,            
            const char* name, 
            int16_t lIn, 
            int16_t tIn, 
            int16_t rIn, 
            int16_t bIn, 
            uint16_t colIn,
            uint8_t page=0,
            ButtonType type=ButtonType::SWITCH):
                col(colIn), 
                id(id),
                l_state((lIn <<3) | (state & 0x03) | ((tIn&1)<<2)),
                r_altState((rIn<<3) | (altState & 0x03) | ((bIn&1)<<2)),
                tHi(tIn >> 1),
                bHi(bIn >> 1)                
        {
            this->ui = ui;
            currentState = 0x02 | ((type & 0x03) << 0x02) | ((page & 0x0F) << 4);
            /*currentState.hasAlt = true;
            currentState.pressed = false;
            currentState.type = type;
            currentState.page = page;*/
        }
    private:
        class SwitchUI* ui;
        
};

#endif