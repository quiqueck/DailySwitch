#ifndef __SWITCH_UI_H__
#define __SWITCH_UI_H__

#include <TFT_eSPI.h>
#include <functional>
#include "DailyBluetoothSwitch.h"

#define TOUCH_BOX_SIZE 86

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

        inline void addButton(class Button* b){ buttons.push_back(b); }
    protected:
        void prepareTouchCalibration(bool force_calibration=false);
        const class Button* buttonAt(uint16_t x, uint16_t y);
        void drawConnectionState();
    public:
        TFT_eSPI tft;
    private:
        const std::function<void(uint8_t, uint8_t)> pressRoutine;        
        const std::function<void(bool)> touchRoutine;        

        union{
            struct{
                bool blockUntilRelease  : 1; 
                bool touchDown          : 1; 
                bool wasConnected       : 1;
                uint32_t reserved       : 29;                
            };
            uint32_t val;
        } state;

        const class Button* pressedButton;
        std::vector<class Button*> buttons;        
        long lastDown;
        uint16_t calibrationData[10];
};

#endif