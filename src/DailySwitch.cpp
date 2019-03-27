//#define MIC //enable microfone

#include <Arduino.h>
#include <FunctionalInterrupt.h>
#include <pins_arduino.h>
#include <SPI.h>
#include <SD.h>
#include "ESP32Setup.h"
#include "NopSerial.h"
#include "esp_pm.h"

#include "TouchPin.h"
#include "SwitchUI.h"
#include "SleepTimer.h"
#include "FileSystem.h"

#include "Weather.h"


#include "DailyBluetoothSwitch.h"

void stateChanged(bool state);
void touchEvent();
void buttonEvent(uint8_t id, uint8_t state);
void touchPanelEvent(bool down);
void forceCalib(uint8_t pin, bool pressed);

#ifdef __cplusplus
extern "C" {
#endif 
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

#ifdef SI7021_DRIVER
    #include "SI7021.h"
    SI7021 envSensor;
#endif

#ifdef MIC 

#include <arduinoFFT.h> // include the library
arduinoFFT FFT = arduinoFFT();

/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 10000; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
#endif 

#ifdef BH1750_DRIVER
    #include <Wire.h>
    #include <BH1750.h>

    BH1750 lightMeter;
#endif

TouchPin* t1 = NULL;
DailyBluetoothSwitchServer* dbss = NULL;
SwitchUI* ui = NULL;
SleepTimer* sleepTimer = NULL;

portMUX_TYPE calibMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE btMux = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
    Serial.begin(115200);
    Console.println(F("Starting Daily Switch"));

    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_esp32_t pm_config;
     
    pm_config.max_freq_mhz = 240;
    pm_config.min_freq_mhz = 80;
    pm_config.light_sleep_enable = true;

    esp_pm_configure(&pm_config);


    //pinMode(LED,OUTPUT);
    //pinMode(SDCARD_CS, OUTPUT);
    //digitalWrite(SDCARD_CS, HIGH);

    FileSystem::init();
    

    ui = new SwitchUI(buttonEvent, touchPanelEvent, false);
#ifdef HEADLESS
    dbss = new DailyBluetoothSwitchServer("HL.001");
#else
    dbss = new DailyBluetoothSwitchServer("001");
#endif
    dbss->setConnectionCallback(stateChanged);

    t1 = new TouchPin(T0, forceCalib, 100);
    
	//pinMode(TFT_IRQ, INPUT);
    //attachInterrupt(digitalPinToInterrupt(TFT_IRQ), touchEvent, FALLING);
    SleepTimer::begin(ui);    

    dbss->startAdvertising();

    //esp_sleep_enable_touchpad_wakeup();
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
    //esp_deep_sleep_start();

    #ifdef SI7021_DRIVER
    if (!envSensor.begin()){
        Console.println(F("Failed to init Environment Sensor"));
    }
    #endif

    #ifdef BH1750_DRIVER    
        #ifndef SI7021_DRIVER
            Console.println(F("Init IC2"));
            Wire.begin(IC2_DAT, IC2_CLK, IC2_FREQUENCY);
        #endif
        Console.println(F("Light Sensor"));
        lightMeter.begin();
    #endif

    #ifdef MIC
    pinMode(AUDIO_PIN, INPUT);
    sampling_period_us = round(1000000*(1.0/samplingFrequency));
    #endif

    #ifdef WEATHER
        if (!Weather::begin(ui)){        
            Console.println("Could not initialize Weather API.");
        }
    #endif
}

volatile bool triggerStateUpdate = true;
volatile bool triggerCalibration = false;
void stateChanged(bool state){
    //portENTER_CRITICAL_ISR(&btMux);
    triggerStateUpdate = false;    
    //portEXIT_CRITICAL_ISR(&btMux);
}

void loop()
{
    //turn off LED
    /*digitalWrite(LED, LOW); 
    delay(500);
    digitalWrite(LED, HIGH); 
    delay(1500);*/

    static uint32_t count = 1000;
    count++;
    if (!triggerStateUpdate) {
        triggerStateUpdate = true;
        ui->connectionStateChanged(dbss->connectionState());
    }

    if (triggerCalibration) {
        Console.println(F("Starting Calibration..."));
        SleepTimer::global()->invalidate();
        SleepTimer::global()->stop();
        ui->startTouchCalibration();
        triggerCalibration = false;
        SleepTimer::global()->start();
    }

	t1->read();
    ui->scanTouch();
#ifdef WEATHER
    Weather::global()->tick();    
#endif
    SleepTimer::global()->tick();

    if (!SleepTimer::global()->noBacklight()) {
        if (count >= 25){
            count = 0;
            ui->internalTemperatureChanged((temprature_sens_read() - 32) / 1.8);
        #ifdef BH1750_DRIVER        
            //long start = micros();
            const float lux = lightMeter.readLightLevel();
            //Console.print("Light: ");
            //Console.print(lux);
            //Console.printf(" lx in %dms\n", (micros()-start)/1000);
            ui->luxChanged(lux);
        #endif
        }

        #ifdef SI7021_DRIVER
            if (envSensor.update()){
                ui->temperaturChanged(envSensor.temperature());
                ui->humidityChanged(envSensor.humidity());
            }
        #endif
    }

    #ifdef MIC
    /*SAMPLING*/
    microseconds = micros();
    for(int i=0; i<samples; i++)
    {
        vReal[i] = analogRead(AUDIO_PIN);
        vImag[i] = 0;
        /*while(micros() - microseconds < sampling_period_us){
            //empty loop
        }*/
        microseconds += sampling_period_us;
    }
    /* Print the results of the sampling according to time */
    //Console.println("Data:");
    //PrintVector(vReal, samples, SCL_TIME);
    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
    //Console.println("Weighed data:");
    //PrintVector(vReal, samples, SCL_TIME);
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
    //Console.println("Computed Real values:");
    //PrintVector(vReal, samples, SCL_INDEX);
    //Console.println("Computed Imaginary values:");
    //PrintVector(vImag, samples, SCL_INDEX);
    FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
    double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
    Console.println("Computed magnitudes:");
    
    PrintVector(vReal, (samples >> 1), SCL_FREQUENCY, x);

    
    Console.println(x, 6); //Print out what frequency is the most dominant.
    #endif
}

#ifdef MIC
    void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType, double mp)
    {
    for (uint16_t i = 0; i < bufferSize; i++)
    {
        double abscissa;
        /* Print abscissa value */
        switch (scaleType)
        {
        case SCL_INDEX:
            abscissa = (i * 1.0);
        break;
        case SCL_TIME:
            abscissa = ((i * 1.0) / samplingFrequency);
        break;
        case SCL_FREQUENCY:
            abscissa = ((i * 1.0 * samplingFrequency) / samples);
        break;
        }
        Console.print(abscissa, 6);
        if(scaleType==SCL_FREQUENCY)
        Console.print("Hz");
        Console.print(" ");
        Console.print(vData[i], 4);

        if (scaleType==SCL_FREQUENCY){
            int v = (int)((vData[i]/100)*480);
            ui->tft.fillRect(20 + i*7, 10, 7, v, TFT_WHITE);
            ui->tft.fillRect(20 + i*7, 10 + v, 7, 480, TFT_BLACK);
            
            Console.print(" ");
            Console.print(v, 4);
        }
        Console.println();
    }
    Console.println();
    }
#endif

void touchEvent(){
    static int ct = 0;
    Console.printf("TOUCH event %d\n", ct++);
}

void buttonEvent(uint8_t id, uint8_t state){
    Console.printf("Sending %d, %d\n", id, state);
    dbss->sendNotification(id, (DailyBluetoothSwitchServer::DBSNotificationStates)state);
}

void touchPanelEvent(bool down){
    if (down) SleepTimer::global()->stop();
    else SleepTimer::global()->start();
}

void forceCalib(uint8_t pin, bool pressed){
    if (pressed==false){
        triggerCalibration = true;        
    }
}