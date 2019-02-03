#include "SI7021.h"
#include <Wire.h>
#include "ESP32Setup.h"

#define _TRANSACTION_TIMEOUT 10
#define REQUEST_TMP_DELAY 6
#define REQUEST_HUM_DELAY 8

#define CMD_HUMIDITY 0xF5
#define CMD_TEMPERATURE 0xF3

union{
    uint16_t v;
    struct{
        uint8_t lo;
        uint8_t hi;
    };
} value;

uint8_t const SI7021::readByte(uint8_t reg) {
  uint8_t value;
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();

  uint32_t start = millis(); // start timeout
  while(millis()-start < _TRANSACTION_TIMEOUT) {
    if (Wire.requestFrom(addr, 1) == 1) {
      value = Wire.read();
      return value;
    }
    delay(1);
  }

  return 0; // Error timeout
}

uint16_t const SI7021::readShort(uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();

  uint32_t start = millis(); // start timeout
  while(millis()-start < _TRANSACTION_TIMEOUT) {
    if (Wire.requestFrom(addr, 2) == 2) { 
      value.hi = Wire.read();
      value.lo = Wire.read();
      return value.v;
    }
    delay(1);
  }

  return 0; // Error timeout
}

bool SI7021::begin(){
    Wire.begin(IC2_DAT, IC2_CLK, IC2_FREQUENCY);
    Serial.printf("Temp Init: %02x\n", readByte(0xE7));
}

float SI7021::readHumidity(){
    lastHumidity =  SI7021::calcHumidity(readShort(CMD_HUMIDITY));
    return lastHumidity;
}

float SI7021::readTemperature() {  
    lastTemperature = SI7021::calcTemperature(readShort(CMD_TEMPERATURE));
    return lastTemperature;
    //float fahrTemp = celsTemp * 1.8 + 32;    
}

bool SI7021::update(unsigned long ms_interval){
    const unsigned long now = millis();
    const unsigned long delta = now - last;
    if ((readState & 0xF0) == 0) { 
        Serial.print(F("Environment Sensor Idle for"));
        Serial.printf("%dms (interv:%dms)\n", delta, ms_interval);
        if (delta > ms_interval) {
            readState = SI7021::ReadState::StartRequestTemp;                       
        }
    } else if (readState == SI7021::ReadState::StartRequestTemp) {
        Serial.println(F("Requesting Temperature"));
        Wire.beginTransmission(addr);
        Wire.write((uint8_t)CMD_TEMPERATURE);
        Wire.endTransmission();

        last = now; 
        readState = SI7021::ReadState::RequestedTemp;
    } else if (readState == SI7021::ReadState::RequestedTemp) {
        if (delta > REQUEST_TMP_DELAY + _TRANSACTION_TIMEOUT) {  
            Serial.println(F("Failed to read Temp."));
            Serial.printf("    %dms (interv:%dms)\n", delta, REQUEST_TMP_DELAY + _TRANSACTION_TIMEOUT);
            readState = SI7021::ReadState::StartRequestHum;
        } else if (delta > REQUEST_TMP_DELAY) { 
            Serial.println(F("Try to read Temp.")); 
            Serial.printf("    %dms (interv:%dms)\n", delta, REQUEST_TMP_DELAY);
            if (Wire.requestFrom(addr, 2) == 2) { 
                value.hi = Wire.read();
                value.lo = Wire.read();
                lastTemperature = value.v;

                readState = SI7021::ReadState::StartRequestHum;
                return true;
            }
        }
    } else if (readState == SI7021::ReadState::StartRequestHum) {
        Serial.println(F("Requesting Humidity"));
        Wire.beginTransmission(addr);
        Wire.write((uint8_t)CMD_HUMIDITY);
        Wire.endTransmission();

        last = now; 
        readState = SI7021::ReadState::RequestedHum;
    } else if (readState == SI7021::ReadState::RequestedHum) {
        if (delta > REQUEST_HUM_DELAY + _TRANSACTION_TIMEOUT) {  
            Serial.println(F("Failed to read Hum."));
            Serial.printf("    %dms (interv:%dms)\n", delta, REQUEST_TMP_DELAY + _TRANSACTION_TIMEOUT);
            readState = SI7021::ReadState::Waiting;            
        } else if (delta > REQUEST_HUM_DELAY) {  
            Serial.println(F("Try to read Hum."));  
            Serial.printf("    %dms (interv:%dms)\n", delta, REQUEST_TMP_DELAY);                   
            if (Wire.requestFrom(addr, 2) == 2) { 
                value.hi = Wire.read();
                value.lo = Wire.read();
                lastHumidity = value.v;

                readState = SI7021::ReadState::Waiting;
                return true;
            }
        }
    } 
    
    return false;
}