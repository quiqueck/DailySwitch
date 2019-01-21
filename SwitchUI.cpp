#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>
#include "DailyBluetoothSwitch.h"
#include <soc/rtc.h>


#define HG() TOUCH_BOX_SIZE
#define TOP(__nr) (__nr * HG())
#define BOT(__nr) (__nr * HG() + HG()-1) 
#define LEF() 0
#define RIG() 320
#define XP1() 170
#define XP2() (RIG() - XP1()) / 2 + XP1()


inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((31*(r+4))/255)<<11) | 
               (((63*(g+2))/255)<<5) | 
               ((31*(b+4))/255);
}

SwitchUI::SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, bool force_calibration):tft(TFT_eSPI()){
    this->pressRoutine = pressRoutine;
    wasConnected = false;
    buttonCount = 9;
    pressedButton = -1;
    lastDown = micros();
    touchStart = 0;
    didDim = 0;
    blockUntilRelease = false;
    
    buttons[0] = new ButtonRect(1, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Wohnzimmer",           
        LEF(), TOP(0), XP1(), BOT(0), rgb(220, 0, 0));
    buttons[1] = new ButtonRect(1, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(0), XP2(), BOT(0), rgb(255, 0, 0));
    buttons[2] = new ButtonRect(1, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(0), RIG(), BOT(0), rgb(150, 0, 0));

    buttons[3] = new ButtonRect(2, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Eszimmer",
        LEF(), TOP(1), XP1(), BOT(1), rgb(0, 220, 0));
    buttons[4] = new ButtonRect(2, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(1), XP2(), BOT(1), rgb(0, 255, 0));
    buttons[5] = new ButtonRect(2, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(1), RIG(), BOT(1), rgb(0, 150, 0));

    buttons[6] = new ButtonRect(3, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Küche",           
        LEF(), TOP(2), XP1(), BOT(2), rgb(0, 0, 220));
    buttons[7] = new ButtonRect(3, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(2), XP2(), BOT(2), rgb(0, 0, 255));
    buttons[8] = new ButtonRect(3, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(2), RIG(), BOT(2), rgb(0, 0, 150));
        
    Serial.printf("Initialized Buttons %x %d\n", buttons, buttons[0]->l);
    
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
        this->redrawAll();
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

  this->redrawAll();
}

void SwitchUI::setBrightness(uint8_t val){
    /*if (val==0) digitalWrite(TFT_BL, LOW);
    else if (val=0xff) digitalWrite(TFT_BL, HIGH);
    else*/ analogWrite(TFT_BL, val);
}

void SwitchUI::connectionStateChanged(bool state){
    wasConnected = state;
    drawConnectionState();
}

void SwitchUI::drawConnectionState(){
    tft.fillRect(20, 440, 100, 40, TFT_WHITE);
    tft.setCursor(20, 460, 2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
    if (wasConnected) tft.println("connected");
    else tft.println("NOT connected");
}

void SwitchUI::redrawAll(){
    Serial.printf("Redraw all %d\n", buttonCount);
    int32_t maxY = 0;
    for (uint8_t nr = 0; nr < buttonCount; nr++){
        //Serial.printf("Button %d %x\n", nr, buttons);
        //Serial.printf("    l: %d, t: %d, r:%d, b:%d, col: %x \n", buttons[nr]->l, buttons[nr]->t, buttons[nr]->r, buttons[nr]->b, buttons[nr]->col);
        tft.fillRect(buttons[nr]->l, buttons[nr]->t, buttons[nr]->w(), buttons[nr]->h(), buttons[nr]->col);
        if (buttons[nr]->b > maxY) maxY = buttons[nr]->b;
    };    

    Serial.println("Fill Rest");
    tft.fillRect(LEF(), maxY, RIG()-LEF(), 480-maxY, TFT_WHITE);
    drawConnectionState();
}

int8_t SwitchUI::buttonAt(uint16_t x, uint16_t y){
    for (uint8_t nr = 0; nr < buttonCount; nr++){
        if (x>buttons[nr]->l && y > buttons[nr]->t && x < buttons[nr]->r && y < buttons[nr]->b)
            return nr;        
    }; 

    return -1;
}

void SwitchUI::scanTouch(){
    uint16_t x, y;
    long delta = micros() - lastDown;

    if (delta > 200000) {
        if (pressedButton>=0) {
            tft.fillRect(buttons[pressedButton]->l, buttons[pressedButton]->t, buttons[pressedButton]->w(), buttons[pressedButton]->h(), buttons[pressedButton]->col);
            pressedButton = -1;
        }
    }

    if (didDim<1 && delta > 10000000) {
        Serial.println("dim=1");
        didDim = 1;
        this->setBrightness(0x10);
    } else if (didDim<2 && delta > 30000000) {
        Serial.println("dim=2");
        delay(120);
        didDim = 2;
        this->setBrightness(0x0);        
        rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
    } else if (didDim<3 && delta > 60000000) {
        Serial.println("dim=3");
        didDim = 3;
        tft.writecommand(0x10);
        delay(6);                
    }
  
    if (tft.getTouch(&x, &y)) {
        if (x>0 && y>0) {
            lastDown = micros();
            if (didDim != 0){ 
                bool discardTouch = false;
                Serial.println("dim=0");               
                if (didDim >= 3) {
                    discardTouch = true;
                    tft.writecommand(0x11);
                    delay(120);
                } else if (didDim >= 2) {
                    discardTouch = true;
                    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
                }
                didDim = 0;
                this->setBrightness(0xFF);
                if (discardTouch) {
                    blockUntilRelease = true;
                    return;
                }
            }

            if (!blockUntilRelease){
                int8_t nowButton = buttonAt(x, y);
                if (pressedButton != nowButton){
                    if (pressedButton>=0) {
                        tft.fillRect(buttons[pressedButton]->l, buttons[pressedButton]->t, buttons[pressedButton]->w(), buttons[pressedButton]->h(), buttons[pressedButton]->col);
                    }
                    if (nowButton>=0) {
                        //Serial.printf("Button %d %x\n", nowButton, buttons);
                        //Serial.printf("    l: %d, t: %d, r:%d, b:%d, col: %x \n", buttons[nowButton]->l, buttons[nowButton]->t, buttons[nowButton]->r, buttons[nowButton]->b,  buttons[nowButton]->col); 
                        tft.fillRect(buttons[nowButton]->l, buttons[nowButton]->t, buttons[nowButton]->w(), buttons[nowButton]->h(), TFT_YELLOW);
                        this->pressRoutine(buttons[nowButton]->id, buttons[nowButton]->state);
                    }
                    pressedButton = nowButton;           
                }                
            }
            lastDown = micros();
        } else {
            blockUntilRelease = false;
        }
    } else {
        blockUntilRelease = false;
        
    }
    if (didDim==2){
        delay(200);
    } else if (didDim>2){
        delay(300);
    }
}