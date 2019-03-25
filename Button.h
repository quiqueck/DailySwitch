#ifndef __BUTTON_H__
#define __BUTTON_H__
#include <Arduino.h>
#include <cmath>
#include "DailyBluetoothSwitch.h"

enum ButtonType :uint8_t {SWITCH=0, PAGE=1, SELECT=2};

class Button {
    public:        
        const int16_t l;
        const int16_t t;
        const int16_t b; 
        const int16_t r;
        const uint16_t col;
        const uint8_t id;
        const DailyBluetoothSwitchServer::DBSNotificationStates state;
        const DailyBluetoothSwitchServer::DBSNotificationStates altState;
        const char* name;

    private:
        union {
            struct{
                bool pressed         : 1; 
                bool hasAlt          : 1; 
                ButtonType type      : 2;
                uint8_t page         : 2;
                uint8_t reserved     : 2;                
            };
            uint8_t val;
        } currentState;

    public:
        inline uint8_t page() const { return currentState.page;}
        inline ButtonType type() const { return currentState.type;}
        inline int16_t w() const { return r-l;}
        inline int16_t h() const { return b-t;}
        inline bool const inside(int16_t x, int16_t y){ return std::signbit((l-x) & (x-r) & (t-y) & (y-b));}

        inline void draw(class SwitchUI* ui) const {draw(ui, col);}
        void draw(class SwitchUI* ui, uint16_t oCol) const;
        inline bool isPressed() const { return currentState.pressed; }
        inline void toogle() {currentState.pressed = !currentState.pressed;}
        inline void down() {currentState.pressed = true;}
        inline void up() {currentState.pressed = false;}
        inline bool hasAlternative() const { return currentState.hasAlt; }
        inline DailyBluetoothSwitchServer::DBSNotificationStates  activeState() const { 
            if (!currentState.hasAlt) return state;            
            return currentState.pressed?altState:state; 
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
                l(lIn),
                r(rIn),
                t(tIn),
                b(bIn),
                col(colIn), 
                id(id), 
                state(state),
                altState(state),
                name(name)
        {
            this->ui = ui;
            currentState.hasAlt = false;
            currentState.pressed = false;
            currentState.type = type;
            currentState.page = page;
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
                l(lIn),
                r(rIn),
                t(tIn),
                b(bIn),
                col(colIn), 
                id(id), 
                state(state), 
                altState(altState),
                name(name)
        {
            this->ui = ui;
            currentState.hasAlt = true;
            currentState.pressed = false;
            currentState.type = type;
            currentState.page = page;
        }
    private:
        class SwitchUI* ui;
        
};

#endif