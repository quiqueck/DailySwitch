#ifndef __SWITCH_UI_H__
#define __SWITCH_UI_H__
#include <TFT_eSPI.h>

#define TOUCH_BOX_SIZE 86

class SwitchUI{
    public:
        SwitchUI(bool force_calibration=false);
        
        void setBrightness(uint8_t val);
        void startTouchCalibration();
    protected:
        void prepareTouchCalibration(bool force_calibration=false);
    public:
        TFT_eSPI tft;
    private:
        uint16_t calibrationData[5];
};

#endif