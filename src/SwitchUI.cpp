#include "SwitchUI.h"
#include "FileSystem.h"
#include <analogWrite.h>
#include "DailyBluetoothSwitch.h"
#include "SleepTimer.h"
#include "Button.h"
#include "ESP32Setup.h"
#include "Sprite.h"
#include "Weather.h"
#include "Proximity.h"
#include <SD.h>


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

void SwitchUI::drawBmp(std::string filename) {
#if !HEADLESS
    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SD.open(filename.c_str(), "rb");

    if (!bmpFS)
    {
        Console.printf("File not found '%s'\n", filename.c_str());
        return;
    }

    uint16_t w, h, row;
    uint32_t startTime = millis();
    w = read16(bmpFS);
    h = read16(bmpFS);
    //Console.printf("%dx%d\n", w, h);
    uint16_t y = 0;
    tft.setSwapBytes(false);  

    const uint16_t seekOffset = 4;    
    //const uint16_t eSz = sizeof(uint16_t);
    bmpFS.seek(seekOffset);
    uint16_t lineBuffer[w];

    for (row = 0; row < h; row++) {    
        bmpFS.read((uint8_t*)lineBuffer, sizeof(lineBuffer));

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(0, y++, w, 1, (uint16_t*)lineBuffer);
    }
    Console.printf("Loaded %s in ", filename.c_str()); Console.print(millis() - startTime);
    Console.println(" ms");
    
    bmpFS.close();
#endif
}

void SwitchUI::drawLightLevelBack(){
    drawBmp(pageDefName(), lightLevelX, lightLevelY, lightLevelW, lightLevelH, true);
    if (selectButton)
        drawBmp(selectButton, true, selectButton->l() - lightLevelX, selectButton->t() - lightLevelY); 

    drawBmpAlpha("/LL.IST", 0, 0, lightLevelW, lightLevelH, 0, 0);
}

void SwitchUI::drawBmp(const class Button* bt, bool toSprite, uint16_t offX, uint16_t offY){
    //discard draw event if this button is not visible
    if (!toSprite && bt->page() != state.currentPage)
        return;

    if (bt->page() == LIGHT_LEVEL_PAGE){        
        //drawLightLevelBack();
        if (bt->isPressed())
            spr.createSprite(bt->w(), bt->h());
            
            drawBmpAlpha(
                    "/LLD.IST", 
                    bt->l() - lightLevelX, 
                    bt->t() - lightLevelY, 
                    bt->w(), 
                    bt->h(),
                    0, 
                    0
            );         
            spr.pushSprite(bt->l(), bt->t());
            spr.deleteSprite();
    } else if (state.wasConnected) {
        std::string file = bt->isPressed() ? pageSelName() : pageDefName();
        drawBmp(file, bt->l(), bt->t(), bt->w(), bt->h(), toSprite, offX, offY);        
    } else {
        drawBmp(pageDisName(), bt->l(), bt->t(), bt->w(), bt->h(), toSprite, offX, offY);
    }
}

void SwitchUI::drawBmp(std::string filename, const class Button* bt){
    drawBmp(filename, bt->l(), bt->t(), bt->w(), bt->h());
}

void SwitchUI::drawBmp(std::string filename, int16_t x, int16_t y, int16_t wd, int16_t hg, bool toSprite, int16_t offX, int16_t offY) {
#if !HEADLESS
    //Console.print("freeMemory()="); Console.println(ESP.getFreeHeap());
    if (toSprite){
        if (offX < 0) {
            wd += offX;
            x -= offX;
            offX = 0;
        }
        if (wd<=0) return;

        if (offY < 0) {
            hg += offY;
            y -= offY;
            offY = 0;
        }
        if (hg<=0) return;
    }


    if ((x >= tft.width()) || (y >= tft.height())) return;

    fs::File bmpFS;
    // Open requested file on SD card
    bmpFS = SD.open(filename.c_str(), "r");

    if (!bmpFS)
    {
        Console.printf("File not found '%s'\n", filename.c_str());
        return;
    }

    uint16_t w, row;

    uint32_t startTime = millis();
    w = read16(bmpFS);
    read16(bmpFS); //height

    tft.setSwapBytes(false);    
    const uint16_t seekOffset = 4;    
    const uint16_t eSz = sizeof(uint16_t);
    bmpFS.seek(seekOffset + y * (w*eSz) + x * eSz);    

    
    uint16_t* buffer = (uint16_t*)malloc(eSz*wd*hg);
    if (buffer) {
        uint8_t* tptr = (uint8_t*)buffer;
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)tptr, wd*eSz);
            tptr += wd*eSz;
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);            
        }   
        if (toSprite) {
            spr.pushImage(offX, offY, wd, hg, (uint16_t*)buffer);     
        } else tft.pushImage(x+offX, y+offY, wd, hg, (uint16_t*)buffer);
        free(buffer);
    } else {
        uint16_t lineBuffer[wd];
        
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)lineBuffer, wd*eSz);
        
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            if (toSprite) spr.pushImage(offX, row + offY, wd, 1, (uint16_t*)lineBuffer);     
            else tft.pushImage(x+offX, (y++) + offY, wd, 1, (uint16_t*)lineBuffer);
        }
    }
    Console.printf("Loaded %s in ", filename.c_str());  Console.print(millis() - startTime);
    Console.println(" ms");
    
    bmpFS.close();
    //Console.print("freeMemory()="); Console.println(ESP.getFreeHeap());
#endif
}


void SwitchUI::drawBmpAlpha(std::string filename, int16_t x, int16_t y, int16_t wd, int16_t hg, int16_t offX, int16_t offY) {
#if !HEADLESS
    //Console.print("freeMemory()="); Console.println(ESP.getFreeHeap());
    if (offX < 0) {
        wd += offX;
        x -= offX;
        offX = 0;
    }

    if (offY < 0) {
        hg += offY;
        y -= offY;
        offY = 0;
    }
    
    if ((x >= tft.width()) || (y >= tft.height())) return;

    fs::File bmpFS;
    // Open requested file on SD card
    bmpFS = SD.open(filename.c_str(), "r");

    if (!bmpFS)
    {
        Console.printf("File not found '%s'\n", filename.c_str());
        return;
    }

    uint16_t w, row;

    uint32_t startTime = millis();
    w = read16(bmpFS);
    read16(bmpFS); //height

    tft.setSwapBytes(false);    
    const uint16_t seekOffset = 4;    
    const uint16_t eSz = sizeof(AlphaCol);
    bmpFS.seek(seekOffset + y * (w*eSz) + x * eSz);    

    AlphaCol* buffer = (AlphaCol*)malloc(eSz*wd*hg);
    if (buffer) {
        uint8_t* tptr = (uint8_t*)buffer;
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)tptr, wd*eSz);
            tptr += wd*eSz;
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);            
        }   
        spr.pushImageAlpha(offX, offY, wd, hg, (AlphaCol*)buffer);             
        free(buffer);
    } else {
        AlphaCol lineBuffer[wd];
        for (row = 0; row < hg; row++) {
            bmpFS.read((uint8_t*)lineBuffer, wd*eSz);
            bmpFS.seek((w-wd)*eSz, fs::SeekMode::SeekCur);
            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            spr.pushImageAlpha(offX, row + offY, wd, 1, (AlphaCol*)lineBuffer);
        }
    }
    Console.printf("Loaded %s in ", filename.c_str());  Console.print(millis() - startTime);
    Console.println(" ms");
    
    bmpFS.close();
    //Console.print("freeMemory()="); Console.println(ESP.getFreeHeap());
#endif
}




inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b){
    return (((31*(r+4))/255)<<11) | 
               (((63*(g+2))/255)<<5) | 
               ((31*(b+4))/255);
}

struct DefInput {
    uint16_t l;
    uint16_t t;
    uint16_t r;
    uint16_t b;

    uint8_t type;
    uint8_t id;
    uint8_t state;
    uint8_t altState;
    uint8_t page;

    char name[9];
};

void SwitchUI::ReadDefinitions(const char *filename) {
    fs::File defFS;
#if !HEADLESS
    // Open requested file on SD card
    defFS = SD.open(filename, "r");

    if (!defFS)
    {
        Console.printf("File not found '%s'\n", filename);
        return;
    }

    uint16_t buttonCount, pageCount;
    

    uint32_t startTime = millis();
    buttonCount = defFS.read();
    pageCount = defFS.read();
    lightLevelX = read16(defFS);
    lightLevelY = read16(defFS);
    lightLevelW = read16(defFS); //lightLevel Width
    lightLevelH = read16(defFS); //lightLevel Height
    Console.printf("Counts: %d, %d\n", buttonCount, pageCount);

    char pageName[3];
    pageName[2] = 0;
    for (int i=0; i<pageCount; i++){
        for (int k =0; k<2; k++) {
            pageName[k] = defFS.read();
            
        }
        std::string s(pageName);
        this->pages.push_back(s);
        Console.printf("Page %d = %s\n", i, pageName);
    }

    DefInput def;
    for (int i=0; i<buttonCount; i++){
        defFS.read((uint8_t*)&def, sizeof(DefInput));
        Button* b;
        if (def.state == def.altState) {
            b = new Button(
                    def.id, 
                    (DailyBluetoothSwitchServer::DBSNotificationStates)def.state, 
                    def.name,           
                    def.l, 
                    def.t, 
                    def.r, 
                    def.b, 
                    rgb(220, 0, 0),
                    def.page,
                    (ButtonType)def.type
                );
        } else {
            b = new Button(
                    def.id, 
                    (DailyBluetoothSwitchServer::DBSNotificationStates)def.state, 
                    (DailyBluetoothSwitchServer::DBSNotificationStates)def.altState, 
                    def.name,           
                    def.l, 
                    def.t, 
                    def.r, 
                    def.b, 
                    rgb(0, 0, 220),
                    def.page,
                    (ButtonType)def.type
                );
        }
        this->addButton(b);
        Console.printf("Button %d = %d - %d, %s\n", i, def.page, def.id, def.name);
        Console.printf("       type=%d, %d; page=%d, %d; state=%d, %d\n", def.type, b->type(), def.page, b->page(), def.state, b->state());
        Console.printf("       l=%d, %d; r=%d, %d\n", def.l, b->l(), def.r, b->r()); 
        Console.printf("       t=%d, %d; b=%d, %d\n", def.t, b->t(), def.b, b->b()); 
    }
    Console.printf("Loaded %s in ", filename);  Console.print(millis() - startTime);
    Console.println(" ms");
    
    defFS.close();
#endif
}

SwitchUI::SwitchUI(std::function<void(uint8_t, uint8_t)> pressRoutine, std::function<void(bool)> touchRoutine, bool force_calibration):tft(TFT_eSPI()), pressRoutine(pressRoutine), touchRoutine(touchRoutine), spr(mySprite(&tft)){
    Console.printf("Resolution: %dx%d\n", tft.width(), tft.height());

    temperature = NAN;
    humidity = NAN;
    lightLevelX = 0;
    lightLevelY = 0;
    lightLevelW = 1;
    lightLevelH = 1;
    state.currentPage = 0;
    state.pushPage = 0;
    state.wasConnected = false;
    state.drewConnected = true;
    state.dirty = true;
    pressedButton = NULL;
    selectButton = NULL;
    lastDown = micros();
    state.touchDown = false;
    state.blockUntilRelease = false;
    
    ReadDefinitions("/DEF.BTS");
    
    //Console.printf("Initialized Buttons %d\n", buttons.size());
    
    Console.println("Initializing TFT...");
    
    tft.init();
    tft.setRotation(3);
    calibrationData = (uint8_t*)malloc(tft.calibrationDataSize());

    this->prepareTouchCalibration(force_calibration);

    Console.println("Done Initializing TFT");    
}

void SwitchUI::prepareTouchCalibration(bool force_calibration){
#if !HEADLESS
    Console.println("Read Calibration Data");   
    // check if calibration file exists
    bool calDataOK = false;
    calDataOK = FileSystem::global()->readCalibrationFile(calibrationData, tft.calibrationDataSize());

    if (calDataOK && !force_calibration) {
        Console.println("Starting Touch");

        // calibration data valid
        tft.setTouch(calibrationData);
        this->redrawAll();
    } else {
        startTouchCalibration();
    }
#else
    //FileSystem::init();
#endif
}

void SwitchUI::startTouchCalibration(){
#if !HEADLESS
  Console.println("Prepare Calibrating Touch");

  tft.fillScreen(TFT_WHITE);
  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
  tft.loadFont("RCR12");
  tft.println("calibration run");
  
  Console.println("Calibrating Touch");
  tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);

  Console.println("Writing Calibration");
  FileSystem::global()->writeCalibrationFile(calibrationData, tft.calibrationDataSize());  

  this->redrawAll();
#endif
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
    //Console.printf("Update Temperature %f => %f (%f)\n", temperature, tmp, fabs(temperature-tmp));

    if (
        (isnan(temperature) && !isnan(tmp)) ||
        (!isnan(temperature) && isnan(tmp)) ||
        fabs(tmp-temperature) > 1.0f
       ) {
        temperature = tmp;
        drawTemperatureState();
    }    
}

void SwitchUI::weatherChanged(Weather* w){
    drawTemperatureState();
}

void SwitchUI::drawTemperatureState(){
    const int16_t barWidth = 74;
    const Weather* w = Weather::global();
    spr.setColorDepth(16);
    spr.createSprite(74, 80);
    spr.setSwapBytes(true);
    drawBmp(pageDefName(), 406, 15, 74, 80, true);

    if (w && w->hasValidData()){
        drawBmpAlpha(w->icon(), 0, 0, 45, 45, (barWidth - 45) / 2, 35);

        spr.loadFont("RCL42", -4);
        spr.setTextSize(1);
        spr.setTextDatum(TC_DATUM);
        
        spr.setTextColor(TFT_BLACK, TFT_WHITE);

        String txt = (String)((int) w->temperature())+"°";
        int16_t wd = spr.textWidth(txt);
        spr.setCursor((barWidth - wd)/2 + 2, 0);  
        spr.print(txt);
        spr.unloadFont();
    }

#ifdef SI7021_DRIVER
    spr.loadFont("RCL42");
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
        spr.loadFont("RCL18");
        spr.print((String)((int)humidity)+"%");                
        spr.unloadFont();
    }
#endif

/*#ifdef BH1750_DRIVER
    spr.loadFont("RCL18");
    spr.setCursor(spr.getCursorX() + 10, 37); 
    spr.print((String)((int)lux)+" lx");                
    spr.unloadFont();
#endif*/

    spr.pushSprite(406, 15); 
    spr.deleteSprite();    
}

void SwitchUI::internalTemperatureChanged(float tmp){
    //Console.printf("Update internal Temperature %f => %f (%f)\n", temperatureIntern, tmp, fabs(temperatureIntern-tmp));

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
    //Console.printf("Update Humidity %f => %f (%f)\n", humidity, hum, fabs(humidity-hum));
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
    //Console.printf("Update Lux %f => %f (%f)\n", lux, l, fabs(lux-l));
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
    tft.loadFont("RCL12");
    tft.fillRect(380, 305, 25, 15, TFT_WHITE);
    tft.setTextDatum(TR_DATUM);
    tft.setCursor(310, 463, 2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
#ifdef BH1750_DRIVER
    tft.drawString((String)((int)lux)+" lx   " + (String)((int)temperatureIntern)+"°", , 400, 306, 2);                    
#else
    if (temperatureIntern > 0 && temperatureIntern<500)
        tft.drawString((String)((int)temperatureIntern)+"°", 400, 307, 2); 
    else
        tft.drawString("--", 400, 306, 2); 
#endif
    tft.unloadFont();
}

void SwitchUI::drawConnectionState(){
    tft.loadFont("RCR12");
    tft.fillRect(406, 305, 74, 15, TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    tft.setCursor(443, 305, 2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
    
    if (state.wasConnected) tft.drawCentreString("verbunden", 443, 306, 2);
    else tft.drawCentreString("suche...", 443, 306, 2);
    
    tft.unloadFont();

    if (state.drewConnected != state.wasConnected){
        state.drewConnected = state.wasConnected;
        std::string f;
        if (state.wasConnected == true)
            f = pageDefName();            
        else
            f = pageDisName();
        drawBmp(f, 0, 0, 406, 320);
    }
}

void SwitchUI::redrawAll(){
    state.dirty = false;
    Console.printf("Redraw all %d\n", buttons.size());
    if (state.wasConnected == true) {
        drawBmp(pageDefName());
        for (auto button : buttons) {
            if (button->isPressed()){
                drawBmp(button);
            }
        }
    } else
        drawBmp(pageDisName());

    //Console.printf("Redraw state\n", buttons.size());
    drawConnectionState();
    drawInternalState();
    drawTemperatureState(); 
}

Button* SwitchUI::buttonAt(uint16_t x, uint16_t y){
#ifndef DISABLE_BUTTON_PRESS
    for (auto button : buttons) {
        if (button->inside(x, y)){
            if (button->page() == state.currentPage) {            
                return button;        
            } else {
                Console.printf("Button: for %d, on %d\n", button->page(), state.currentPage);
            }
        }
    } 

    if (state.currentPage == LIGHT_LEVEL_PAGE){
        handleLightLevelSelect(NULL);
    }
#endif
    return NULL;
}

void SwitchUI::returnToNormalState(){
    handleLightLevelSelect(NULL);
}

void SwitchUI::reloadMainPage() {
    if (selectButton){
        selectButton->up();
        drawBmp(selectButton);
        selectButton = NULL;
    }

    if (state.currentPage == SETTINGS_PAGE || state.currentPage == LIGHT_LEVEL_PAGE){        
        state.currentPage = MAIN_PAGE;
        state.pushPage = MAIN_PAGE;
        redrawAll();
    }
}

void SwitchUI::handleLightLevelSelect(Button* btn){
    if (state.currentPage == LIGHT_LEVEL_PAGE) {
        state.currentPage = state.pushPage;        
        drawBmp(pageDefName(), lightLevelX, lightLevelY, lightLevelW, lightLevelH); 
    }

    if (selectButton) {
        Console.println("Finish LightLevelSelect...");
        if (btn!=NULL) {
            this->pressRoutine(selectButton->id, btn->activeState());
        }
        selectButton->up();
        drawBmp(selectButton);
        selectButton = NULL;
    }
}

void SwitchUI::handleButtonPress(Button* btn){
    //Serial.printf("btn: %d, %d, %d, %d", btn->id, btn->type(), btn->page(), btn->hasAlternative());
    if (btn->page() == LIGHT_LEVEL_PAGE){
        handleLightLevelSelect(btn);
    } else if (btn->type() == ButtonType::PAGE){
        state.currentPage = btn->id;
        state.pushPage = btn->id;
        redrawAll();
    } else if (btn->type() == ButtonType::SELECT){
        Console.printf("Undo Button %X\n", (unsigned int)pressedButton);
        //makes sure the button does NOT trigger yet  
        if (pressedButton)
            drawBmp(pressedButton);      
        pressedButton = NULL;  

        selectButton = btn;
        state.pushPage = state.currentPage;
        state.currentPage = LIGHT_LEVEL_PAGE;

        Console.printf("Draw Alpha %d\n", state.currentPage);

        spr.setColorDepth(16);
        spr.createSprite(lightLevelW, lightLevelH);
        spr.setSwapBytes(true);
        drawLightLevelBack();
        spr.pushSprite(lightLevelX, lightLevelY);
        spr.deleteSprite();
    } else if (btn->type() == ButtonType::SPECIAL_FUNCTION){
        switch (btn->id )  {
            case SPECIAL_BUTTON_PROXIMITY_INC:
                Proximity::global()->incProximity();
                Proximity::global()->storeSettings();
            break;
            case SPECIAL_BUTTON_PROXIMITY_DEC:
                Proximity::global()->decProximity();
                Proximity::global()->storeSettings();
            break;
            case SPECIAL_BUTTON_SCREEN_OFF:
                SleepTimer::global()->displayOff();
                SleepTimer::global()->start();
                delay(2000);
            break;
            case SPECIAL_BUTTON_UPDATE_WEATHER:
                Weather::global()->update();
            break;
            case SPECIAL_BUTTON_SETTINGS:
                
            break;
            case SPECIAL_BUTTON_CALIBRATE_TOUCH:
                SleepTimer::global()->stop();
                startTouchCalibration();        
                SleepTimer::global()->start();
            break;
        }
    } else {
        this->pressRoutine(btn->id, btn->activeState());
    }
}

void SwitchUI::scanTouch(){
    /*tft.loadFont("RCL12");
    tft.setTextColor(TFT_BLACK, TFT_WHITE);*/
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
#ifdef TEST_TOUCH
        tft.drawRect(x-2, y-2, 5, 5, TFT_RED);
#endif
        if (!state.touchDown) touchRoutine(true);
        state.touchDown = true;
        lastDown = micros();
        Console.printf("touch %d, %d\n", x, y);
        
        //ignore touches when we reactivate
        if (SleepTimer::global()->noBacklight()) {
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
                    //this->pressRoutine(nowButton->id, nowButton->activeState());#
                    handleButtonPress(nowButton);
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