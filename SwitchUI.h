#ifndef __SWITCH_UI_H__
#define __SWITCH_UI_H__

#include <TFT_eSPI.h>
#include <functional>
#include "DailyBluetoothSwitch.h"

#define TOUCH_BOX_SIZE 86

struct ButtonRect {
    const int16_t l;
    const int16_t t;
    const int16_t b; 
    const int16_t r;
    const uint16_t col;
    const uint8_t id;
    const DailyBluetoothSwitchServer::DBSNotificationStates state;
    const char* name;
    inline int16_t w() { return r-l;}
    inline int16_t h() { return b-t;}

    ButtonRect(uint8_t id, DailyBluetoothSwitchServer::DBSNotificationStates state, const char* name, int16_t lIn, int16_t tIn, int16_t rIn, int16_t bIn, uint16_t colIn):
    l(lIn),r(rIn),t(tIn),b(bIn),col(colIn), id(id), state(state), name(name){}
};

class SwitchUI{
    public:
        SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration=false);
        
        void setBrightness(uint8_t val);
        void startTouchCalibration();
        void redrawAll();
        void scanTouch();
        void connectionStateChanged(bool state);

        inline void displayOff() { tft.writecommand(0x10); delay(6); }
        inline void displayOn() { tft.writecommand(0x11); delay(6); }
    protected:
        void prepareTouchCalibration(bool force_calibration=false);
        int8_t buttonAt(uint16_t x, uint16_t y);
        void drawConnectionState();
    public:
        TFT_eSPI tft;
    private:
        const std::function<void(uint8_t, uint8_t)> pressRoutine;        
        const std::function<void(bool)> touchRoutine;        

        union{
            struct{
                uint8_t buttonCount     : 8;
                int8_t pressedButton    : 8;
                bool blockUntilRelease  : 1; 
                bool touchDown          : 1; 
                bool wasConnected       : 1;
                uint32_t reserved       : 13;                
            };
            uint32_t val;
        } state;

        ButtonRect* buttons[9];        
        long lastDown;
        uint16_t calibrationData[10];
};

#endif