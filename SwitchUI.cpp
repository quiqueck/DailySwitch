#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>
#include "DailyBluetoothSwitch.h"
#include "SleepTimer.h"
#include "Button.h"
#include <soc/rtc.h>


#define HG() 85
#define TOP(__nr) (85 + __nr * HG())
#define BOT(__nr) (85 + __nr * HG() + HG()-1) 
#define LEF() 0
#define RIG() tft.width()
#define XP1() 155
#define XP2() 238

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void SwitchUI::drawBmp(const char *filename) {
  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();
  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      int16_t y = h - 1;

      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(0, y--, w, 1, (uint16_t*)lineBuffer);
      }
      Serial.print("Loaded in "); Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

void SwitchUI::drawBmp(const char *filename, const class Button* bt){
    drawBmp(filename, bt->l, bt->t, bt->w(), bt->h());
}

void SwitchUI::drawBmp(const char *filename, int16_t x, int16_t y, int16_t wd, int16_t hg) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();
  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];
      bmpFS.seek(seekOffset + (h-y-hg) * sizeof(lineBuffer) + x * 3);
      y += hg - 1;

      
      for (row = 0; row < hg; row++) {
        
        bmpFS.read(lineBuffer, wd*3);
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < wd; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
        bmpFS.seek((w-wd)*3 + padding, fs::SeekMode::SeekCur);
        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, wd, 1, (uint16_t*)lineBuffer);
      }
      Serial.print("Loaded in "); Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}




inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((31*(r+4))/255)<<11) | 
               (((63*(g+2))/255)<<5) | 
               ((31*(b+4))/255);
}

SwitchUI::SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration):tft(TFT_eSPI()), pressRoutine(pressRoutine), touchRoutine(touchRoutine){
    state.wasConnected = false;
    state.dirty = true;
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
    tft.setRotation(2);

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

    if (val==0 && state.dirty){
           redrawAll();
    }
}

void SwitchUI::connectionStateChanged(bool stateIn){
    state.wasConnected = stateIn;
    drawConnectionState();
}

void SwitchUI::drawConnectionState(){
    tft.fillRect(66, 461, 110, 20, TFT_BLACK);
    tft.setCursor(66, 462, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
    if (state.wasConnected) tft.println("verbunden");
    else tft.println("NICHT verbunden");
}

void SwitchUI::redrawAll(){
    state.dirty = false;
    Serial.printf("Redraw all %d\n", buttons.size());
    /*int32_t maxY = 0;
    for (auto&& button : buttons) {
        button->draw(this);
        if (button->b > maxY) maxY = button->b;
    }    

    Serial.println("Fill Rest");
    tft.fillRect(LEF(), maxY, RIG()-LEF(), 480-maxY, TFT_WHITE);*/
    drawBmp("/MainMenu.bmp");
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
            drawBmp("/MainMenu.bmp", pressedButton);
            //pressedButton->draw(this);
            pressedButton = NULL;
        }
    }
    
    if (tft.getTouch(&x, &y) && x>0 && y>0 && x<tft.width() && y<tft.height()) {
        if (!state.touchDown) touchRoutine(true);
        state.touchDown = true;
        lastDown = micros();
        //Serial.printf("touch %d, %d\n", x, y);
        
        //ignore touches when we reactivate
        if (SleepTimer::global()->currentState() >= 6) {
            state.blockUntilRelease = true;
            SleepTimer::global()->invalidate();                     
            return;
        }
        SleepTimer::global()->invalidate();      

        if (!state.blockUntilRelease){
            const Button* nowButton = buttonAt(x, y);
            if (pressedButton != nowButton){
                if (pressedButton != NULL) {
                    drawBmp("/MainMenu.bmp", pressedButton);
                    //pressedButton->draw(this);
                }
                if (nowButton != NULL) {
                    state.dirty = true;
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