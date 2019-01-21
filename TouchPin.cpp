#include "TouchPin.h"

TouchPin::TouchPin(uint8_t pin, std::function<void(uint8_t, bool)> pressRoutine, uint8_t samples)
:pin(pin), samples(samples){
    this->pressRoutine = pressRoutine;
    hadTouch = false;
    lastEvent = 0;
}

void TouchPin::read(){
  int touchVal = 0;
  for(int i=0; i< samples; i++){
    touchVal += touchRead(pin);    
  }
  touchVal /= samples;
  //Serial.printf("%d: %d\n", pin, touchVal);
  if (touchVal<40){
      lastEvent = micros();
      if (!hadTouch){
          hadTouch = true;
          Serial.printf("Button down on pin %d\n", pin); 
          pressRoutine(pin, true);        
      }
  } else {
      if (hadTouch) {
        long now = micros();
        if (now - lastEvent > 2000) {
            hadTouch = false;
            Serial.printf("Button up on pin %d\n", pin);         
            pressRoutine(pin, false);
        }
        //Serial.printf("%d\n", now - lastEvent);
      }
  }
}