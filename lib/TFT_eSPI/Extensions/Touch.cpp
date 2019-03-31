// The following touch screen support code by maxpautsch was merged 1/10/17
// https://github.com/maxpautsch

// Define TOUCH_CS is the user setup file to enable this code

// A demo is provided in examples Generic folder

// Additions by Bodmer to double sample, use Z value to improve detection reliability
// and to correct rotation handling

// See license in root directory.




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
/*getTouchRaw(&x_tmp,&y_tmp); 
String s(String("") + x_tmp + " x " + y_tmp + " x " + z1 + " (" + threshold + ")");
drawString(s, 50, 50, 2);*/
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

//Serial.printf("<------ %04dx%04d\n", x_tmp, y_tmp);
  convertRawXY(&x_tmp, &y_tmp);
//Serial.printf("------> %04dx%04d\n", x_tmp, y_tmp);
  if (x_tmp >= _width || y_tmp >= _height) return valid;

  _pressX = x_tmp;
  _pressY = y_tmp;
  *x = _pressX;
  *y = _pressY;
  return valid;
}

#define PX(__n) (((_width - size - 1) * (__n)) / (CAPTURE_POINTS_X-1))
#define PY(__n) (((_height - size - 1) *(__n)) / (CAPTURE_POINTS_Y-1))
#define SX(__n) (((_width - 1) * (__n)) / (CAPTURE_POINTS_X-1))
#define SY(__n) (((_height - 1) *(__n)) / (CAPTURE_POINTS_Y-1))
#define FILL_RECT(___nx, ___ny, ___cl) fillRect(PX(___nx), PY(___ny), size+1, size+1, ___cl);
#define DRAW_RECT(___nx, ___ny, ___cl) drawRect(PX(___nx), PY(___ny), size+1, size+1, ___cl);
#define DRAW_LINE(___nx, ___ny, ___cl) drawLine(PX(___nx), PY(___ny)+size, PX(___nx)+size, PY(___ny), ___cl);

#define IDXX(___xx, ___yy) (((___xx)*CAPTURE_POINTS_Y + (___yy))*2)
#define IDXY(___xx, ___yy) ((((___xx)*CAPTURE_POINTS_Y + (___yy))*2)+1)
#define VX(___xx, ___yy) values[IDXX(___xx, ___yy)]
#define VY(___xx, ___yy) values[IDXY(___xx, ___yy)]


#define DEBUG_DRAW 0

class Vector {
  public:
    const float x;
    const float y;
    Vector(float xIn, float yIn) : x(xIn), y(yIn){
      
    }

    inline float dot(Vector v) const {
      return x*v.x + y*v.y;
    }

    inline float cross(Vector v) const {
      return x*v.y - y*v.x;
    }

    inline Vector add(Vector v){
      return Vector(x + v.x, y + v.y);
    }

    inline Vector sub(Vector v){
      return Vector(x - v.x, y - v.y);
    }

    inline Vector mul(float s){
      return Vector(x * s, y * s);
    }
};

void barycentric(Vector p, Vector a, Vector b, Vector c, float &u, float &v, float &w){
  const Vector vbc = b.sub(c);
  const Vector vac = a.sub(c);
  const Vector vpc = p.sub(c);
  const float det = vac.cross(vbc);
  
  u = vpc.cross(vbc) / det;
  v = vac.cross(vpc) / det;
  w = 1 - u - v;  
}
/***************************************************************************************
** Function name:           convertRawXY
** Description:             convert raw touch x,y values to screen coordinates 
***************************************************************************************/
void TFT_eSPI::convertRawXY(uint16_t *x, uint16_t *y)
{
  const uint16_t size = 15;
  const uint8_t CAPTURE_POINTS_X = calibrationXCaptureCount();
  const uint8_t CAPTURE_POINTS_Y = calibrationYCaptureCount();
  const uint16_t X = -1*(*x);
  const uint16_t Y = *y;
  
 
#if DEBUG_DRAW
  fillRect(0, 0, 480, 320, TFT_BLACK);
#endif
  for (int xi=1; xi<CAPTURE_POINTS_X; xi++){
    for (int yi=1; yi<CAPTURE_POINTS_Y; yi++){
      uint16_t Ax = -VX(xi-1, yi-1);
      uint16_t Ay = VY(xi-1, yi-1);
      uint16_t Bx = -VX(xi-1, yi);
      uint16_t By = VY(xi-1, yi);
      //Serial.printf("  %dx%d:  %dx%d -- %dx%d      %dx%d %dx%d\n", X, Y, Ax, Ay, Bx, By, IDXX(xi-1, yi-1), IDXY(xi-1, yi-1), xi-1, yi-1);
      if ((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax)< 0) continue;
        

      uint16_t Cx = -VX(xi, yi);
      uint16_t Cy = VY(xi, yi);
      if ((Cx - Bx) * (Y - By) - (Cy - By) * (X - Bx) < 0) continue;

      uint16_t Dx = -VX(xi, yi-1);
      uint16_t Dy = VY(xi, yi-1);
      if ((Dx - Cx) * (Y - Cy) - (Dy - Cy) * (X - Cx) < 0) continue;      
      if ((Ax - Dx) * (Y - Dy) - (Ay - Dy) * (X - Dx) < 0) continue;

      Vector A(SX(xi-1), SY(yi-1));
      Vector B(SX(xi-1), SY(yi));
      Vector C(SX(xi), SY(yi));
      Vector D(SX(xi), SY(yi-1));


#if DEBUG_DRAW
      drawLine(A.x, A.y, B.x, B.y, TFT_WHITE);
      drawLine(B.x, B.y, C.x, C.y, TFT_WHITE);
      drawLine(C.x, C.y, D.x, D.y, TFT_WHITE);
      drawLine(D.x, D.y, A.x, A.y, TFT_WHITE);
#endif


      Serial.printf("  %d, %d    %dx%d   %dx%d\n", xi, yi, SX(xi), SY(yi), SX(xi-1), PY(yi-1));
      float u, v, w;
      Vector p(X, Y);
      if ((Dx - Bx) * (Y - By) - (Dy - By) * (X - Bx) < 0) {
#if DEBUG_DRAW
        drawLine(B.x, B.y, C.x, C.y, TFT_GREEN);
        drawLine(C.x, C.y, D.x, D.y, TFT_GREEN);
        drawLine(D.x, D.y, B.x, B.y, TFT_GREEN);
#endif        
        //triangle BCD
        barycentric(p, Vector(Bx, By), Vector(Cx, Cy), Vector(Dx, Dy), u, v, w);
        
        Vector res = B.mul(u).add(C.mul(v)).add(D.mul(w));
        Serial.printf("--: u=%f, v=%f, w=%f --> %fx%f\n", u, v, w, res.x, res.y);
        *x = (uint16_t)res.x;
        *y = (uint16_t)res.y;

#if DEBUG_DRAW
        fillRect(res.x-size/2, res.y-size/2, size, size, TFT_BLUE);
#endif        
      } else {
#if DEBUG_DRAW        
        drawLine(A.x, A.y, B.x, B.y, TFT_GREEN);
        drawLine(B.x, B.y, D.x, D.y, TFT_GREEN);
        drawLine(D.x, D.y, A.x, A.y, TFT_GREEN);
#endif        
        //triangle ABD
        barycentric(p, Vector(Ax, Ay), Vector(Bx, By), Vector(Dx, Dy), u, v, w);
       
        Vector res = A.mul(u).add(B.mul(v)).add(D.mul(w));
        Serial.printf("++: u=%f, v=%f, w=%f --> %fx%f\n", u, v, w, res.x, res.y);
        *x = (uint16_t)res.x;
        *y = (uint16_t)res.y;

#if DEBUG_DRAW
        fillRect(res.x-size/2, res.y-size/2, size, size, TFT_BLUE);
#endif
      }

      return;
    }
  }
  
   *x = 10000;
  *y = 10000;
}

void TFT_eSPI::calibrateTouch(uint8_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size){
  const uint8_t SAMPLES = 64;
  const uint8_t CAPTURE_POINTS_X = calibrationXCaptureCount();
  const uint8_t CAPTURE_POINTS_Y = calibrationYCaptureCount();
  ;

  uint16_t x_tmp, y_tmp, z_tmp;  
  int32_t zInit = 0;
  int32_t zSampleAcc = 0;
  values.resize(CAPTURE_POINTS_X*CAPTURE_POINTS_Y*2); 

  //calibrate the initial z-value
  for (int i=0; i<SAMPLES; i++){
    zInit += getTouchRawZ();
    delay(50);
  }
  zInit = (zInit * 1.5) / SAMPLES;
  Serial.printf("MinZ: %d\n", zInit);

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


      uint32_t samplesX = 0;
      uint32_t samplesY = 0;

      //take calibration samples
      for(uint8_t j= 0; j<SAMPLES; j++){
        // Use a lower detect threshold as corners tend to be less sensitive
        while(!validTouch(&x_tmp, &y_tmp, &z_tmp, zInit)){
          //nop just busy wait
        }

        samplesX += x_tmp;
        samplesY += y_tmp;
        zSampleAcc += z_tmp;

        delay(5);
      }
      Serial.printf("%dx%d: %d, %d\n", IDXX(xi, yi), IDXY(xi, yi), samplesX / SAMPLES, samplesY / SAMPLES);
      VX(xi, yi) = samplesX / SAMPLES;
      VY(xi, yi) = samplesY / SAMPLES;
        
        
    }
  }

Serial.print("Values: ");
for (int i=0; i<values.size(); i++){
  Serial.printf("%d ", values[i]);
}
Serial.println();
Serial.print("ZMin: ");
  touchCalibration_zMin = (((zSampleAcc/SAMPLES) / (CAPTURE_POINTS_X*CAPTURE_POINTS_Y)) + zInit) >> 1; 
  Serial.println(touchCalibration_zMin);

  if (parameters != NULL) {
    *((uint16_t*)(&parameters[0])) = values.size();
    *((uint16_t*)(&parameters[2])) = touchCalibration_zMin;
    uint8_t pos = 4;
    for (int i=0; i<values.size(); i++){
      *((uint16_t*)(&parameters[pos])) = values[i];
      pos += sizeof(uint16_t);
    }  
  }
  //return calibrateTouchLinear(parameters, color_fg, color_bg, size);
}


/***************************************************************************************
** Function name:           setTouch
** Description:             imports calibration parameters for touchscreen. 
***************************************************************************************/
void TFT_eSPI::setTouch(uint8_t *parameters){
  uint16_t count = *((uint16_t*)(&parameters[0]));
  touchCalibration_zMin = *((uint16_t*)(&parameters[2]));

  values.clear();
  uint8_t pos =4;
  for (int i=0; i<count; i++){
    values.push_back(*((uint16_t*)(&parameters[pos])));
    pos += sizeof(uint16_t);
  }  

  Serial.print("Values: ");  
  for (int i=0; i<values.size(); i++){
    Serial.printf("%d ", values[i]);
  }
  Serial.println();
}
