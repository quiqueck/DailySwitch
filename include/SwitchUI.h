#ifndef __SWITCH_UI_H__
#define __SWITCH_UI_H__

#include "ESP32Setup.h"
#include <TFT_eSPI.h>
#include <functional>
#include <cmath>
#include "Sprite.h"
#include "DailyBluetoothSwitch.h"
#include "NopSerial.h"

#define TOUCH_BOX_SIZE 86
#define LIGHT_LEVEL_PAGE 0xF
#define MAIN_PAGE 0x01
#define SETTINGS_PAGE 0x00

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
            return "/" + pages[currentPage() >= pages.size() ? pushPage() : currentPage()] + ".IST";
        }
        std::string  pageDisName() {
            if (pages.size() == 0) return "/MMX.IST";
            return "/" + pages[currentPage() >= pages.size() ? pushPage() : currentPage()] + "X.IST";
        }
        std::string  pageSelName() {
            if (pages.size() == 0) return "/MMD.IST";
            return "/" + pages[currentPage() >= pages.size() ? pushPage() : currentPage()] + "D.IST";
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
        void drawProximityState();

        void ReadDefinitions(const char *filename);
    public:
        TFT_eSPI tft;
    private:
        const std::function<void(uint8_t, uint8_t)> pressRoutine;        
        const std::function<void(bool)> touchRoutine; 
        mySprite spr;       

        /*struct {
                bool blockUntilRelease  : 1; 
                bool touchDown          : 1; 
                bool wasConnected       : 1;
                bool drewConnected      : 1;
                bool dirty              : 1;
                uint8_t currentPage     : 4;
                uint8_t pushPage        : 4;
                uint32_t reserved       : 19;                
        } state;*/
        uint8_t state;
        uint8_t pageState;
        uint8_t settingsPressCount;
        inline boolean blockUntilRelease() const { return state & 0x01;}
        inline void blockUntilRelease(boolean v) { if (v) state |= 0x01; else state &= 0xFE; }
        inline boolean touchDown() const { return state & 0x02;}
        inline void touchDown(boolean v) { if (v) state |= 0x02; else state &= 0xFD; }
        inline boolean wasConnected() const { return state & 0x04;}
        inline void wasConnected(boolean v) { if (v) state |= 0x04; else state &= 0xFB; }
        inline boolean drewConnected() const { return state & 0x08;}
        inline void drewConnected(boolean v) { if (v) state |= 0x08; else state &= 0xF7; }
        inline boolean dirty() const { return state & 0x10;}
        inline void dirty(boolean v) { if (v) state |= 0x10; else state &= 0xEF; }

        inline uint8_t currentPage() const { return (pageState & 0x0F);}
        inline void currentPage(uint8_t v) { pageState = (pageState & 0xF0) | (v & 0x0F); }

        inline uint8_t pushPage() const { return (pageState & 0xF0) >> 4;}
        inline void pushPage(uint8_t v) { pageState = (pageState & 0x0F) | ((v << 4) & 0xF0); }

        float temperature;
        float temperatureIntern;
        float humidity;
        float lux;
        class Button* pressedButton;
        class Button* selectButton;
        std::vector<class Button*> buttons;  
        std::vector<std::string> pages;      
        long lastDown;
        uint8_t* calibrationData;
        uint16_t lightLevelX;
        uint16_t lightLevelY;
        uint16_t lightLevelW;
        uint16_t lightLevelH;
};

#endif