#ifndef __SWITCH_UI_H__
#define __SWITCH_UI_H__

#include "ESP32Setup.h"
#include <TFT_eSPI.h>
#include <functional>
#include <cmath>
#include "Sprite.h"
#include "DailyBluetoothSwitch.h"

#define TOUCH_BOX_SIZE 86
#define LIGHT_LEVEL_PAGE 0xF

class SwitchUI{
    public:
        SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration=false);
        
        void setBrightness(uint8_t val);
        void startTouchCalibration();
        void redrawAll();
        void scanTouch();
        void connectionStateChanged(bool state);
        void temperaturChanged(float tmp);
        void internalTemperatureChanged(float tmp);
        void humidityChanged(float hum);
        void weatherChanged(class Weather* w);
        void luxChanged(float l);
        std::string  pageDefName() {
            if (pages.size() == 0) return "/MM.IST";
            return "/" + pages[state.currentPage >= pages.size() ? state.pushPage : state.currentPage] + ".IST";
        }
        std::string  pageDisName() {
            if (pages.size() == 0) return "/MMX.IST";
            return "/" + pages[state.currentPage >= pages.size() ? state.pushPage : state.currentPage] + "X.IST";
        }
        std::string  pageSelName() {
            if (pages.size() == 0) return "/MMD.IST";
            return "/" + pages[state.currentPage >= pages.size() ? state.pushPage : state.currentPage] + "D.IST";
        }

        inline void displayOff() { tft.writecommand(0x10); delay(6); }
        inline void displayOn() { tft.writecommand(0x11); delay(6); }
        void reloadMainPage();
        void returnToNormalState();

        inline void addButton(class Button* b){ buttons.push_back(b); }
    protected:
        void drawBmp(std::string filename);
        void drawBmp(std::string filename, int16_t x, int16_t y, int16_t w, int16_t h, bool toSprite=false, int16_t offX=0, int16_t offY=0);
        void drawBmpAlpha(std::string filename, int16_t x, int16_t y, int16_t w, int16_t h,  int16_t offX=0, int16_t offY=0);
        void drawBmp(std::string filename, const  class Button* bt);
        void drawBmp(const class Button* bt, bool toSprite=false, uint16_t offX=0, uint16_t offY=0);
        void prepareTouchCalibration(bool force_calibration=false);
        class Button* buttonAt(uint16_t x, uint16_t y);
        void handleButtonPress(class Button* btn);
        void handleLightLevelSelect(class Button* btn);

        void drawConnectionState();
        void drawInternalState();
        void drawTemperatureState();
        void drawLightLevelBack();

        void ReadDefinitions(const char *filename);
    public:
        TFT_eSPI tft;
    private:
        mySprite spr;
        const std::function<void(uint8_t, uint8_t)> pressRoutine;        
        const std::function<void(bool)> touchRoutine;        

        struct {
                bool blockUntilRelease  : 1; 
                bool touchDown          : 1; 
                bool wasConnected       : 1;
                bool drewConnected      : 1;
                bool dirty              : 1;
                uint8_t currentPage     : 4;
                uint8_t pushPage        : 4;
                uint32_t reserved       : 19;                
        } state;

        float temperature;
        float temperatureIntern;
        float humidity;
        float lux;
        class Button* pressedButton;
        class Button* selectButton;
        std::vector<class Button*> buttons;  
        std::vector<std::string> pages;      
        long lastDown;
        uint16_t calibrationData[10];
        uint16_t lightLevelX;
        uint16_t lightLevelY;
        uint16_t lightLevelW;
        uint16_t lightLevelH;
};

#endif