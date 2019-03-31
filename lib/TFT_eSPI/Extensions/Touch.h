 // Coded by Bodmer 10/2/18, see license in root directory.
 // This is part of the TFT_eSPI class and is associated with the Touch Screen handlers

 public:
           // Get raw x,y ADC values from touch controller
  uint8_t  getTouchRaw(uint16_t *x, uint16_t *y);
           // Get raw z (i.e. pressure) ADC value from touch controller
  uint16_t getTouchRawZ(void);
           // Convert raw x,y values to calibrated and correctly rotated screen coordinates
  void     convertRawXY(uint16_t *x, uint16_t *y);
           // Get the screen touch coordinates, returns true if screen has been touched
           // if the touch cordinates are off screen then x and y are not updated
  uint8_t  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold);
  
  inline uint8_t getTouch(uint16_t *x, uint16_t *y){
      return getTouch(x, y, touchCalibration_zMin);
  }

           // Run screen calibration and test, report calibration values to the serial port
  void     calibrateTouchLinear(uint16_t *data, uint32_t color_fg, uint32_t color_bg, uint8_t size);
   inline uint16_t calibrationXCaptureCount() const { return coeffX.size(); }
   inline uint16_t calibrationYCaptureCount() const { return coeffY.size(); }
   inline uint16_t calibrationDataSize() const { return calibrationXCaptureCount() * calibrationYCaptureCount() * sizeof(float) + 2*sizeof(uint8_t) + sizeof(uint16_t); }
   void     calibrateTouch(uint8_t *data, uint32_t color_fg, uint32_t color_bg, uint8_t size);
           // Set the screen calibration values
  void     setTouch(uint8_t *data);

 private:
           // Handlers for the SPI settings and clock speed change
  inline void spi_begin_touch() __attribute__((always_inline));
  inline void spi_end_touch()   __attribute__((always_inline));

           // Private function to validate a touch, allow settle time and reduce spurious coordinates
  uint8_t  validTouch(uint16_t *x, uint16_t *y, uint16_t threshold = 600);
  uint8_t  validTouch(uint16_t *x, uint16_t *y, uint16_t *z, uint16_t threshold);

  // Initialise with example calibration values so processor does not crash if setTouch() not called in setup()
  uint16_t touchCalibration_zMin = 600;
  std::vector<float> coeffX, coeffY;

  uint32_t _pressTime;        // Press and hold time-out
  uint16_t _pressX, _pressY;  // For future use (last sampled calibrated coordinates)
