#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>
#include "DailyBluetoothSwitch.h"
#include "SleepTimer.h"
#include "Button.h"
#include <soc/rtc.h>


#define HG() TOUCH_BOX_SIZE
#define TOP(__nr) (__nr * HG())
#define BOT(__nr) (__nr * HG() + HG()-1) 
#define LEF() 0
#define RIG() tft.width()
#define XP1() 170
#define XP2() (RIG() - XP1()) / 2 + XP1()


inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((31*(r+4))/255)<<11) | 
               (((63*(g+2))/255)<<5) | 
               ((31*(b+4))/255);
}

SwitchUI::SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration):tft(TFT_eSPI()), pressRoutine(pressRoutine), touchRoutine(touchRoutine){
    state.wasConnected = false;
    pressedButton = NULL;
    lastDown = micros();
    state.touchDown = false;
    state.blockUntilRelease = false;
    
    this->addButton(new Button(1, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Wohnzimmer",           
        LEF(), TOP(0), XP1(), BOT(0), rgb(220, 0, 0)));
    this->addButton(new Button(1, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(0), XP2(), BOT(0), rgb(255, 0, 0)));
    this->addButton(new Button(1, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(0), RIG(), BOT(0), rgb(150, 0, 0)));

    this->addButton(new Button(2, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Eszimmer",
        LEF(), TOP(1), XP1(), BOT(1), rgb(0, 220, 0)));
    this->addButton(new Button(2, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(1), XP2(), BOT(1), rgb(0, 255, 0)));
    this->addButton(new Button(2, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(1), RIG(), BOT(1), rgb(0, 150, 0)));

    this->addButton(new Button(3, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "Küche",           
        LEF(), TOP(2), XP1(), BOT(2), rgb(0, 0, 220)));
    this->addButton(new Button(3, DailyBluetoothSwitchServer::DBSNotificationStates::ON_SECONDARY, "",
        XP1(), TOP(2), XP2(), BOT(2), rgb(0, 0, 255)));
    this->addButton(new Button(3, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, "",
        XP2(), TOP(2), RIG(), BOT(2), rgb(0, 0, 150)));
        
    Serial.printf("Initialized Buttons %d\n", buttons.size());
    
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

void SwitchUI::connectionStateChanged(bool stateIn){
    state.wasConnected = stateIn;
    drawConnectionState();
}

void SwitchUI::drawConnectionState(){
    tft.fillRect(20, 440, 100, 40, TFT_WHITE);
    tft.setCursor(20, 460, 2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
    if (state.wasConnected) tft.println("connected");
    else tft.println("NOT connected");
}

void SwitchUI::redrawAll(){
    Serial.printf("Redraw all %d\n", buttons.size());
    int32_t maxY = 0;
    for (auto&& button : buttons) {
        button->draw(this);
        if (button->b > maxY) maxY = button->b;
    }    

    Serial.println("Fill Rest");
    tft.fillRect(LEF(), maxY, RIG()-LEF(), 480-maxY, TFT_WHITE);
    drawConnectionState();
}

const Button* SwitchUI::buttonAt(uint16_t x, uint16_t y){
    for (auto&& button : buttons) {
        if (button->inside(x, y)) return button;        
    } 

    return NULL;
}



void SwitchUI::scanTouch(){
    uint16_t x, y;
    long delta = micros() - lastDown;
    //timer wrapped around
    if (delta<0) { lastDown = micros(); }
    
    //wait a bit before we send the touch-up event
    if (delta > 200*1000) {
        if (state.touchDown) touchRoutine(false);
        state.touchDown = false;
        state.blockUntilRelease = false;

        if (pressedButton != NULL) {
            pressedButton->draw(this);
            pressedButton = NULL;
        }
    }
    
    if (tft.getTouch(&x, &y) && x>0 && y>0 && x<tft.width() && y<tft.height()) {
        if (!state.touchDown) touchRoutine(true);
        state.touchDown = true;
        lastDown = micros();
        //Serial.printf("touch %d, %d\n", x, y);
        
        //ignore touches when we reactivate
        if (SleepTimer::global()->currentState() >= 2) {
            state.blockUntilRelease = true;   
            SleepTimer::global()->invalidate();         
            return;
        }
        SleepTimer::global()->invalidate();      

        if (!state.blockUntilRelease){
            const Button* nowButton = buttonAt(x, y);
            if (pressedButton != nowButton){
                if (pressedButton != NULL) pressedButton->draw(this);
                if (nowButton != NULL) {
                    nowButton->draw(this, TFT_YELLOW);
                    this->pressRoutine(nowButton->id, nowButton->state);
                }
                pressedButton = nowButton;           
            }                
        }
        lastDown = micros();        
    } else {
        /*if (touchDown) touchRoutine(false);
        touchDown = false;
        blockUntilRelease = false;*/
    }
}