#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>

SwitchUI::SwitchUI(bool force_calibration):tft(TFT_eSPI()){
    Serial.println("Initializing TFT...");
    
    tft.init();
    tft.setRotation(0);

    this->prepareTouchCalibration(force_calibration);

    Serial.println("Done Initializing TFT");
}

void SwitchUI::prepareTouchCalibration(bool force_calibration){
    Serial.println("Read Calibration Data");   
    // check if calibration file exists
    bool calDataOK = false;
    calDataOK = FileSystem::global()->readCalibrationFile((char *)calibrationData);

    if (calDataOK && !force_calibration) {
        Serial.println("Starting Touch");

        // calibration data valid
        tft.setTouch(calibrationData);
    } else {
        startTouchCalibration();
    }
}

void SwitchUI::startTouchCalibration(){
  Serial.println("Prepare Calibrating Touch");

  tft.fillScreen(TFT_WHITE);
  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
  tft.println("calibration run");
  
  Serial.println("Calibrating Touch");
  tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);

  Serial.println("Writing Calibration");
  FileSystem::global()->writeCalibrationFile((const char*)calibrationData);  
}

void SwitchUI::setBrightness(uint8_t val){
    analogWrite(TFT_BL, val);
}
