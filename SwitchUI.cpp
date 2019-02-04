#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>
#include "DailyBluetoothSwitch.h"
#include "SleepTimer.h"
#include "Button.h"
#include "ESP32Setup.h"
#include <SPIFFS.h>


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
    bmpFS = SPIFFS.open(filename, "rb");

    if (!bmpFS)
    {
        Serial.printf("File not found '%s'\n", filename);
        return;
    }

    uint16_t w, h, row, col;
    uint8_t  r, g, b;
    uint32_t startTime = millis();
    w = read16(bmpFS);
    h = read16(bmpFS);
    
    uint16_t y = 0;
    tft.setSwapBytes(false);  

    const uint16_t seekOffset = 4;    
    const uint16_t eSz = sizeof(uint16_t);
    bmpFS.seek(seekOffset);
    uint16_t lineBuffer[w];

    for (row = 0; row < h; row++) {    
        bmpFS.read((uint8_t*)lineBuffer, sizeof(lineBuffer));

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(0, y++, w, 1, (uint16_t*)lineBuffer);
    }
    Serial.print(F("Loaded in ")); Serial.print(millis() - startTime);
    Serial.println(" ms");
    
    bmpFS.close();
}

void SwitchUI::drawBmp(const class Button* bt){
    if (state.wasConnected) {
        if (bt->isPressed()){
            drawBmp("/MMD.istl", bt->l, bt->t, bt->w(), bt->h());
        } else {
            drawBmp("/MM.istl", bt->l, bt->t, bt->w(), bt->h());
        }
    } else {
        drawBmp("/MMX.istl", bt->l, bt->t, bt->w(), bt->h());
    }
}

void SwitchUI::drawBmp(const char *filename, const class Button* bt){
    drawBmp(filename, bt->l, bt->t, bt->w(), bt->h());
}

uint16_t* dataBuffer = NULL;

void SwitchUI::drawBmp(const char *filename, int16_t x, int16_t y, int16_t wd, int16_t hg, bool toSprite) {

    if ((x >= tft.width()) || (y >= tft.height())) return;

    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS)
    {
        Serial.printf("File not found '%s'\n", filename);
        return;
    }

    uint16_t w, h, row, col;
    uint8_t  r, g, b;

    uint32_t startTime = millis();
    w = read16(bmpFS);
    h = read16(bmpFS);

    tft.setSwapBytes(false);    
    const uint16_t seekOffset = 4;    
    const uint16_t eSz = sizeof(uint16_t);
    bmpFS.seek(seekOffset + y * (w*eSz) + x * eSz);    

    if (wd*hg*2<80000) {
        uint16_t* lineBuffer = (uint16_t*)malloc(eSz*wd*hg);
        uint8_t* tptr = (uint8_t*)lineBuffer;
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)tptr, wd*eSz);
            tptr += wd*eSz;
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);            
        }   
        if (toSprite) spr.pushImage(0, 0, wd, hg, (uint16_t*)lineBuffer);     
        else tft.pushImage(x, y, wd, hg, (uint16_t*)lineBuffer);
        free(lineBuffer);
    } else {
        uint16_t lineBuffer[wd];
        
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)lineBuffer, wd*eSz);
        
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            if (toSprite) spr.pushImage(0, row, wd, 1, (uint16_t*)lineBuffer);     
            else tft.pushImage(x, y++, wd, 1, (uint16_t*)lineBuffer);
        }
    }
    Serial.print(F("Loaded in ")); Serial.print(millis() - startTime);
    Serial.println(" ms");
    
    bmpFS.close();
}




inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((31*(r+4))/255)<<11) | 
               (((63*(g+2))/255)<<5) | 
               ((31*(b+4))/255);
}

SwitchUI::SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration):tft(TFT_eSPI()), pressRoutine(pressRoutine), touchRoutine(touchRoutine), spr(TFT_eSprite(&tft)){

    spr.setColorDepth(16);
    spr.createSprite(150, 70);

    temperature = NAN;
    humidity = NAN;
    state.wasConnected = false;
    state.drewConnected = true;
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

    this->addButton(new Button(4, DailyBluetoothSwitchServer::DBSNotificationStates::OFF, DailyBluetoothSwitchServer::DBSNotificationStates::ON, "",
        XP2(), 380, RIG(), 380+79, rgb(0, 0, 150)));
        
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

void SwitchUI::temperaturChanged(float tmp){
    Serial.printf("Update Temperature %f => %f (%f)\n", temperature, tmp, fabs(temperature-tmp));

    if (
        (isnan(temperature) && !isnan(tmp)) ||
        (!isnan(temperature) && isnan(tmp)) ||
        fabs(tmp-temperature) > 1.0f
       ) {
        temperature = tmp;
        drawTemperatureState();
    }    
}
void SwitchUI::drawTemperatureState(){
    spr.setColorDepth(16);
    spr.setSwapBytes(true);
    drawBmp("/MM.istl", 10, 00, spr.width(), spr.height(), true);

#ifdef SI7021_DRIVER
    spr.loadFont("RobotoCondensed-Light-42");
    spr.setTextSize(1);
    spr.setCursor(0, 20);  
    spr.setTextColor(TFT_BLACK, TFT_WHITE); 

    if (isnan(temperature)){
        spr.print(F("--°"));
    } else {        
        spr.print((String)((int)temperature)+"°");                
    }
    spr.unloadFont();
    
    if (!isnan(humidity)){
        spr.setCursor(spr.getCursorX() - 10, 37); 
        spr.loadFont("RobotoCondensed-Light-18");
        spr.print((String)((int)humidity)+"%");                
        spr.unloadFont();
    }
#endif

/*#ifdef BH1750_DRIVER
    spr.loadFont("RobotoCondensed-Light-18");
    spr.setCursor(spr.getCursorX() + 10, 37); 
    spr.print((String)((int)lux)+" lx");                
    spr.unloadFont();
#endif*/

    spr.pushSprite(10, 00);
    
}

void SwitchUI::internalTemperatureChanged(float tmp){
    Serial.printf("Update internal Temperature %f => %f (%f)\n", temperatureIntern, tmp, fabs(temperatureIntern-tmp));

    if (
        (isnan(temperatureIntern) && !isnan(tmp)) ||
        (!isnan(temperatureIntern) && isnan(tmp)) ||
        fabs(tmp-temperatureIntern) > 1.0f
       ) {
        temperatureIntern = tmp;
        drawInternalState();
    }    
}

void SwitchUI::humidityChanged(float hum){
    Serial.printf("Update Humidity %f => %f (%f)\n", humidity, hum, fabs(humidity-hum));
    if (
        (isnan(humidity) && !isnan(hum)) ||
        (!isnan(humidity) && isnan(hum)) ||
        fabs(hum-humidity) > 1.0f
       ){
        humidity = hum;
        drawTemperatureState();
    }
}

void SwitchUI::luxChanged(float l){
    Serial.printf("Update Lux %f => %f (%f)\n", lux, l, fabs(lux-l));
    if (
        (isnan(lux) && !isnan(l)) ||
        (!isnan(lux) && isnan(l)) ||
        fabs(l-lux) > 10.0f
       ){
        lux = l;
        drawInternalState();
    }
}

void SwitchUI::connectionStateChanged(bool stateIn){
    state.wasConnected = stateIn;
    drawConnectionState();
}

void SwitchUI::drawInternalState(){
    tft.fillRect(150, 461, 140, 20, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);

    tft.loadFont("RobotoCondensed-Light-12");
    tft.setTextDatum(TR_DATUM);
    tft.setCursor(310, 463, 2);
#ifdef BH1750_DRIVER
    tft.drawString((String)((int)lux)+" lx   " + (String)((int)temperatureIntern)+"°", 310, 464);                    
#else
    tft.drawString((String)((int)temperatureIntern)+"°", 310, 463); 
#endif
    tft.unloadFont();
}

void SwitchUI::drawConnectionState(){
    tft.loadFont("RobotoCondensed-Regular-12");
    tft.fillRect(66, 461, 110, 20, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(66, 463, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  tft.setTextSize(1);
    if (state.wasConnected) tft.println(F("verbunden"));
    else tft.println(F("NICHT verbunden"));

    tft.unloadFont();

    if (state.drewConnected != state.wasConnected){
        state.drewConnected = state.wasConnected;
        if (state.wasConnected == true)
            drawBmp("/MM.istl", 0, 70, 320, 480-70-20);
        else
            drawBmp("/MMX.istl", 0, 70, 320, 480-70-20);
    }
}

void SwitchUI::redrawAll(){
    state.dirty = false;
    Serial.printf("Redraw all %d\n", buttons.size());
    if (state.wasConnected == true)
        drawBmp("/MM.istl");
    else
        drawBmp("/MMX.istl");

    Serial.printf("Redraw state\n", buttons.size());
    drawConnectionState(); 
    drawInternalState();
    drawTemperatureState(); 
}

Button* SwitchUI::buttonAt(uint16_t x, uint16_t y){
    for (auto button : buttons) {
        if (button->inside(x, y)) return button;        
    } 

    return NULL;
}



void SwitchUI::scanTouch(){
    uint16_t x, y;
    unsigned long delta = micros() - lastDown;
    
    
    //wait a bit before we send the touch-up event
    if (delta > 100*1000) {
        if (state.touchDown) touchRoutine(false);
        state.touchDown = false;
        state.blockUntilRelease = false;

        if (pressedButton != NULL) {
            if (!pressedButton->hasAlternative()) {
                pressedButton->up();
            }
            if (state.wasConnected == true) {
                drawBmp(pressedButton);
            } else {
                drawBmp(pressedButton);
            }
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
            Button* nowButton = buttonAt(x, y);
            if (pressedButton != nowButton){
                if (pressedButton != NULL) {
                    if (!pressedButton->hasAlternative()) {
                        pressedButton->up();
                    }
                    if (state.wasConnected == true) {
                        drawBmp(pressedButton);
                    } else {
                        drawBmp(pressedButton);
                    }
                    //pressedButton->draw(this);
                }
                if (nowButton != NULL) {
                    state.dirty = true;
                    if (!nowButton->hasAlternative()) {
                        nowButton->down();
                    } else {
                        nowButton->toogle();
                    }
                    if (state.wasConnected == true) {
                        drawBmp(nowButton);
                    } else {
                        drawBmp(nowButton);
                    }
                    //nowButton->draw(this, TFT_YELLOW);
                    this->pressRoutine(nowButton->id, nowButton->activeState());
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