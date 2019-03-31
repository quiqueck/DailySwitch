// The following touch screen support code by maxpautsch was merged 1/10/17
// https://github.com/maxpautsch

// Define TOUCH_CS is the user setup file to enable this code

// A demo is provided in examples Generic folder

// Additions by Bodmer to double sample, use Z value to improve detection reliability
// and to correct rotation handling

// See license in root directory.


void polyfit(const std::vector<double> &tIn, const std::vector<double> &yIn, std::vector<double> &coeff, int order)
{
  assert(tIn.size() == yIn.size());
	assert(tIn.size() >= order+1);

	// Initialize Vandermonde Matrix
  Eigen::MatrixXd A = Eigen::MatrixXd::Ones(tIn.size(), order + 1);

  // Convert Datatypes
	Eigen::VectorXd t = Eigen::VectorXd::Map(&tIn.front(), tIn.size());
  Eigen::VectorXd y = Eigen::VectorXd::Map(&yIn.front(), yIn.size());

	for (uint8_t j = 1; j < order + 1; ++j) 
    	A.col(j) = A.col(j - 1).cwiseProduct(t);
  
  Eigen::VectorXd coeffs = A.householderQr().solve(y);
  	
	coeff.resize(order+1);
	for (size_t i = 0; i < order+1; i++)
		coeff[i] = (float)coeffs[i];  
}

/***************************************************************************************
** Function name:           getTouchRaw
** Description:             read raw touch position.  Always returns true.
***************************************************************************************/
uint8_t TFT_eSPI::getTouchRaw(uint16_t *x, uint16_t *y){
  uint16_t tmp;
  CS_H;

  spi_begin_touch();

  T_CS_L;
  
  // Start YP sample request for x position
  SPI.transfer(0xd0);                    // Start new YP conversion
  tmp = SPI.transfer(0);                 // Read first 8 bits
  tmp = tmp <<5;
  tmp |= 0x1f & (SPI.transfer(0x90)>>3); // Read last 8 bits and start new XP conversion

  *x = tmp;

  // Start XP sample request for y position
  tmp = SPI.transfer(0);                 // Read first 8 bits
  tmp = tmp <<5;
  tmp |= 0x1f & (SPI.transfer(0)>>3);    // Read last 8 bits

  *y = tmp;

  T_CS_H;

  spi_end_touch();

  return true;
}

/***************************************************************************************
** Function name:           getTouchRawZ
** Description:             read raw pressure on touchpad and return Z value. 
***************************************************************************************/
uint16_t TFT_eSPI::getTouchRawZ(void){
  CS_H;

  spi_begin_touch();

  T_CS_L;

  // Calculate Z
  int16_t tz = 0xFFF;
  SPI.transfer(0xb0);               // Start new Z1 conversion
  tz += SPI.transfer16(0xc0) >> 3;  // Read Z1 and start Z2 conversion
  tz -= SPI.transfer16(0x00) >> 3;  // Read Z2

  T_CS_H;

  spi_end_touch();

  return (uint16_t)tz;
}

/***************************************************************************************
** Function name:           validTouch
** Description:             read validated position. Return false if not pressed. 
***************************************************************************************/
#define _RAWERR 10 // Deadband error allowed in successive position samples
uint8_t TFT_eSPI::validTouch(uint16_t *x, uint16_t *y, uint16_t threshold){
  uint16_t zDontCare = 0;
  return validTouch(x, y, &zDontCare, threshold);
}
uint8_t TFT_eSPI::validTouch(uint16_t *x, uint16_t *y, uint16_t *z, uint16_t threshold){
  uint16_t x_tmp, y_tmp, x_tmp2, y_tmp2;

  // Wait until pressure stops increasing to debounce pressure
  uint16_t z1 = 1;
  uint16_t z2 = 0;
  while (z1 > z2)
  {
    z2 = z1;
    z1 = getTouchRawZ();
    delay(1);
  }

  
  //Serial.print("Z = ");Serial.print(z1);
  //   Serial.print(" "); Serial.println(threshold);

  if (z1 <= threshold) {
    return false;
  }
    
  getTouchRaw(&x_tmp,&y_tmp); 
    //Serial.print("Sample 1 x,y = "); Serial.print(x_tmp);Serial.print(",");Serial.print(y_tmp);
    //Serial.print(", Z = ");Serial.println(z1);

  delay(1); // Small delay to the next sample
  if (getTouchRawZ() <= threshold) return false;

  delay(2); // Small delay to the next sample
  getTouchRaw(&x_tmp2,&y_tmp2);
  
    //Serial.print("Sample 2 x,y = "); Serial.print(x_tmp2);Serial.print(",");Serial.println(y_tmp2);
    //Serial.print("Sample difference = ");Serial.print(abs(x_tmp - x_tmp2));Serial.print(",");Serial.println(abs(y_tmp - y_tmp2));

  if (abs(x_tmp - x_tmp2) > _RAWERR) return false;
  if (abs(y_tmp - y_tmp2) > _RAWERR) return false;
  
  *y = x_tmp;
  *x = y_tmp;
  *z = z1;
  return true;
}
  
/***************************************************************************************
** Function name:           getTouch
** Description:             read callibrated position. Return false if not pressed. 
***************************************************************************************/
#define Z_THRESHOLD 900 // Touch pressure threshold for validating touches
uint8_t TFT_eSPI::getTouch(uint16_t *x, uint16_t *y, uint16_t threshold){
  uint16_t x_tmp, y_tmp;
  
  if (threshold<20) threshold = 20;
  if (_pressTime > millis()) threshold=20;

  uint8_t n = 5;
  uint8_t valid = 0;
  while (n--)
  {
    if (validTouch(&x_tmp, &y_tmp, threshold)) valid++;;
  }

  if (valid<1) { _pressTime = 0; return false; }
  
  _pressTime = millis() + 50;

Serial.printf("<------ %04dx%04d\n", x_tmp, y_tmp);
  convertRawXY(&x_tmp, &y_tmp);
Serial.printf("------> %04dx%04d\n", x_tmp, y_tmp);
  if (x_tmp >= _width || y_tmp >= _height) return valid;

  _pressX = x_tmp;
  _pressY = y_tmp;
  *x = _pressX;
  *y = _pressY;
  return valid;
}

/***************************************************************************************
** Function name:           convertRawXY
** Description:             convert raw touch x,y values to screen coordinates 
***************************************************************************************/
void TFT_eSPI::convertRawXY(uint16_t *x, uint16_t *y)
{
  uint16_t x_tmp = *x, y_tmp = *y;
  float xx, yy;

  xx=0;
  for (int i=0; i<calibrationXCaptureCount(); i++){
    xx += coeffX[i] * powf(x_tmp, i);
  }
  yy=0;
  for (int i=0; i<calibrationYCaptureCount(); i++){
    yy += coeffY[i] * powf(y_tmp, i);
  }
  
  *x = xx;
  *y = yy;
}

/***************************************************************************************
** Function name:           calibrateTouch
** Description:             generates calibration parameters for touchscreen. 
***************************************************************************************/
void TFT_eSPI::calibrateTouchLinear(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size){
  const uint8_t SAMPLES = 64;
  int32_t values[] = {0,0, 0,0, 0,0, 0,0};
  int32_t zSamples = 0;
  uint16_t x_tmp, y_tmp, z_tmp;  
  int32_t zInit = 0;

  //calibrate the initial z-value
  for (int i=0; i<SAMPLES; i++){
    zInit += getTouchRawZ();
  }
  zInit = (zInit * 1.5) / SAMPLES;

  for(uint8_t i = 0; i<4; i++){
    fillRect(0, 0, size+1, size+1, color_fg);
    fillRect(0, _height-size-1, size+1, size+1, color_fg);
    fillRect(_width-size-1, 0, size+1, size+1, color_fg);
    fillRect(_width-size-1, _height-size-1, size+1, size+1, color_fg);

    
    if (i == 5) break; // used to clear the arrows

    // user has to get the chance to release
    // do before we mark the next rect to give feedback that we are in delay state
    //if(i>0) 
    {
      //wait for release
      while(validTouch(&x_tmp, &y_tmp, &z_tmp, zInit)){
        delay(50);
      }

      drawRect(0, 0, size+1, size+1, color_bg);
      drawRect(0, _height-size-1, size+1, size+1, color_bg);
      drawRect(_width-size-1, 0, size+1, size+1, color_bg);
      drawRect(_width-size-1, _height-size-1, size+1, size+1, color_bg);

      delay(2000);
    }

    fillRect(0, 0, size+1, size+1, color_bg);
    fillRect(0, _height-size-1, size+1, size+1, color_bg);
    fillRect(_width-size-1, 0, size+1, size+1, color_bg);
    fillRect(_width-size-1, _height-size-1, size+1, size+1, color_bg);

    switch (i) {
      case 0: // up left
        drawLine(0, 0, 0, size, color_fg);
        drawLine(0, 0, size, 0, color_fg);
        drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // bot left
        drawLine(0, _height-size-1, 0, _height-1, color_fg);
        drawLine(0, _height-1, size, _height-1, color_fg);
        drawLine(size, _height-size-1, 0, _height-1 , color_fg);
        break;
      case 2: // up right
        drawLine(_width-size-1, 0, _width-1, 0, color_fg);
        drawLine(_width-size-1, size, _width-1, 0, color_fg);
        drawLine(_width-1, size, _width-1, 0, color_fg);
        break;
      case 3: // bot right
        drawLine(_width-size-1, _height-size-1, _width-1, _height-1, color_fg);
        drawLine(_width-1, _height-1-size, _width-1, _height-1, color_fg);
        drawLine(_width-1-size, _height-1, _width-1, _height-1, color_fg);
        break;
      }    

    for(uint8_t j= 0; j<SAMPLES; j++){
      // Use a lower detect threshold as corners tend to be less sensitive
      while(!validTouch(&x_tmp, &y_tmp, &z_tmp, zInit));
        values[i*2  ] += x_tmp;
        values[i*2+1] += y_tmp;
        zSamples += z_tmp;
      }
      values[i*2  ] /= SAMPLES;
      values[i*2+1] /= SAMPLES;
      delay(50);
  }
 touchCalibration_zMin = (((zSamples/SAMPLES) >> 2) + zInit) >> 1;  
 
 float touchCalibration_x0, touchCalibration_x1, touchCalibration_y0, touchCalibration_y1;
  // from case 0 to case 1, the y value changed. 
  // If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
  bool touchCalibration_rotate = false;
  if(abs(values[0]-values[2]) > abs(values[1]-values[3])){
    touchCalibration_rotate = true;
    touchCalibration_x0 = (values[1] + values[3])/2; // calc min x
    touchCalibration_x1 = (values[5] + values[7])/2; // calc max x
    touchCalibration_y0 = (values[0] + values[4])/2; // calc min y
    touchCalibration_y1 = (values[2] + values[6])/2; // calc max y
  } else {
    touchCalibration_x0 = (values[0] + values[2])/2; // calc min x
    touchCalibration_x1 = (values[4] + values[6])/2; // calc max x
    touchCalibration_y0 = (values[1] + values[5])/2; // calc min y
    touchCalibration_y1 = (values[3] + values[7])/2; // calc max y
  }

  // in addition, the touch screen axis could be in the opposite direction of the TFT axis
  bool touchCalibration_invert_x = false;
  if(touchCalibration_x0 > touchCalibration_x1){
    values[0]=touchCalibration_x0;
    touchCalibration_x0 = touchCalibration_x1;
    touchCalibration_x1 = values[0];
    touchCalibration_invert_x = true;
  }
  bool touchCalibration_invert_y = false;
  if(touchCalibration_y0 > touchCalibration_y1){
    values[0]=touchCalibration_y0;
    touchCalibration_y0 = touchCalibration_y1;
    touchCalibration_y1 = values[0];
    touchCalibration_invert_y = true;
  }

  // pre calculate
  touchCalibration_x1 -= touchCalibration_x0;
  touchCalibration_y1 -= touchCalibration_y0;

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  // export parameters, if pointer valid
  if (parameters != NULL) {
    parameters[0] = 2;
    parameters[1] = 2;
    *((uint16_t*)(&parameters[2])) = touchCalibration_zMin;
    uint8_t pos = 4;
    *((float*)(&parameters[pos])) = touchCalibration_x0;
    pos += sizeof(float);
    *((float*)(&parameters[pos])) = touchCalibration_x1;
    pos += sizeof(float);
    *((float*)(&parameters[pos])) = touchCalibration_y0;
    pos += sizeof(float);
    *((float*)(&parameters[pos])) = touchCalibration_y1;
    pos += sizeof(float);
  }
}

#define PX(__n) (((_width - size - 1) * __n) / (CAPTURE_POINTS_X-1))
#define PY(__n) (((_height - size - 1) * __n) / (CAPTURE_POINTS_Y-1))
#define FILL_RECT(___nx, ___ny, ___cl) fillRect(PX(___nx), PY(___ny), size+1, size+1, ___cl);
#define DRAW_RECT(___nx, ___ny, ___cl) drawRect(PX(___nx), PY(___ny), size+1, size+1, ___cl);
#define DRAW_LINE(___nx, ___ny, ___cl) drawLine(PX(___nx), PY(___ny)+size, PX(___nx)+size, PY(___ny), ___cl);

void TFT_eSPI::calibrateTouch(uint8_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size){
  const uint8_t SAMPLES = 64;
  const uint8_t CAPTURE_POINTS_X = 4;
  const uint8_t CAPTURE_POINTS_Y = 4;
  ;

  uint16_t x_tmp, y_tmp, z_tmp;  

  int32_t zInit = 0;
  int32_t zSampleAcc = 0;

  std::vector<double> valuesX, valuesY, posX, posY;
  valuesX.resize(CAPTURE_POINTS_X); posX.resize(CAPTURE_POINTS_X);
  for (int i=0; i<CAPTURE_POINTS_X; i++){ 
    valuesX[i] = 0;
    posX[i] = PX(i) + size/2;
  }

  valuesY.resize(CAPTURE_POINTS_Y); posY.resize(CAPTURE_POINTS_Y);
  for (int i=0; i<CAPTURE_POINTS_Y; i++){ 
    valuesY[i] = 0;
    posY[i] = PY(i) + size/2;
  }
  
  //calibrate the initial z-value
  for (int i=0; i<SAMPLES; i++){
    zInit += getTouchRawZ();
  }
  zInit = (zInit * 1.5) / SAMPLES;

  for (uint8_t xi=0; xi<CAPTURE_POINTS_X; xi++){ 
    for (uint8_t yi=0; yi<CAPTURE_POINTS_Y; yi++){ 
      //remove all boxes (indicates holding state)
      for (uint8_t RRX=0; RRX<CAPTURE_POINTS_X; RRX++){ for (uint8_t RRY=0; RRY<CAPTURE_POINTS_Y; RRY++){ 
        FILL_RECT(RRX, RRY, color_fg)
      }}

      //wait for release
      while(validTouch(&x_tmp, &y_tmp, &z_tmp, zInit)){
        delay(50);
      }

      //draw outlines (indicates prepare state)
      for (uint8_t RRX=0; RRX<CAPTURE_POINTS_X; RRX++){ for (uint8_t RRY=0; RRY<CAPTURE_POINTS_Y; RRY++){ 
        DRAW_RECT(RRX, RRY, color_bg)
      }}
      delay(1000);

      //fill in all boxes (indicates ready state)
      for (uint8_t RRX=0; RRX<CAPTURE_POINTS_X; RRX++){ for (uint8_t RRY=0; RRY<CAPTURE_POINTS_Y; RRY++){ 
        FILL_RECT(RRX, RRY, color_bg)
      }}
      //mark the active one
      DRAW_LINE(xi, yi, color_fg)


      double samplesX = 0;
      double samplesY = 0;
      //take calibration samples
      for(uint8_t j= 0; j<SAMPLES; j++){
        // Use a lower detect threshold as corners tend to be less sensitive
        while(!validTouch(&x_tmp, &y_tmp, &z_tmp, zInit)){
          //nop just busy wait
        }

        samplesX += (double)x_tmp / (SAMPLES*CAPTURE_POINTS_Y);
        samplesY += (double)y_tmp / (SAMPLES*CAPTURE_POINTS_X);
        zSampleAcc += z_tmp;
      }
      Serial.printf("%f, %f\n", samplesX, samplesY);
      valuesX[xi] += samplesX;
      valuesY[yi] += samplesY;
        
      delay(50);  
    }
  }

  Serial.printf("%d, %d    %d, %d\n", valuesX.size(), posX.size(), valuesY.size(), posY.size());
  for (int i = 0; i<CAPTURE_POINTS_X; i++){
    Serial.printf("%f/%f ", valuesX[i], posX[i]);
  }
  Serial.println();

  for (int i = 0; i<CAPTURE_POINTS_Y; i++){
    Serial.printf("%f/%f ", valuesY[i], posY[i]);
  }
  Serial.println();

  coeffX.clear();
  polyfit(valuesX, posX, coeffX, CAPTURE_POINTS_X-1);
  Serial.print("y=");
  for (int i = 0; i<CAPTURE_POINTS_X; i++){
    Serial.printf("%s%f * x^%d", coeffX[i]<0?"":(i>0?"+":""), coeffX[i], i);
  }
  Serial.println();

  coeffY.clear();
  polyfit(valuesY, posY, coeffY, CAPTURE_POINTS_Y-1);
  Serial.print("y=");
  for (int i = 0; i<CAPTURE_POINTS_Y; i++){
    Serial.printf("%s%f * x^%d", coeffY[i]<0?"":(i>0?"+":""), coeffY[i], i);
  }
  Serial.println();

  touchCalibration_zMin = (((zSampleAcc/SAMPLES) / (CAPTURE_POINTS_X*CAPTURE_POINTS_Y)) + zInit) >> 1; 
  Serial.println(touchCalibration_zMin);

  if (parameters != NULL) {
    parameters[0] = calibrationXCaptureCount();
    parameters[1] = calibrationYCaptureCount();
    *((uint16_t*)(&parameters[2])) = touchCalibration_zMin;
    uint8_t pos = 4;
    for (int i=0; i<CAPTURE_POINTS_X; i++){
      *((float*)(&parameters[pos])) = coeffX[i];
      pos += sizeof(float);
    }
    for (int i=0; i<CAPTURE_POINTS_Y; i++){
      *((float*)(&parameters[pos])) = coeffY[i];
      pos += sizeof(float);
    }
  }
  //return calibrateTouchLinear(parameters, color_fg, color_bg, size);
}


/***************************************************************************************
** Function name:           setTouch
** Description:             imports calibration parameters for touchscreen. 
***************************************************************************************/
void TFT_eSPI::setTouch(uint8_t *parameters){
  uint8_t xct = parameters[0];
  uint8_t yct = parameters[1];

  touchCalibration_zMin = *((uint16_t*)(&parameters[2]));
  uint8_t pos = 4;
  coeffX.clear();
  for (int i=0; i<xct; i++){
    coeffX.push_back(*((float*)(&parameters[pos])));
    pos += sizeof(float);
  }
  coeffY.clear();
  for (int i=0; i<yct; i++){
    coeffY.push_back(*((float*)(&parameters[pos])));
    pos += sizeof(float);
  }
}
