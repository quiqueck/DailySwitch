#ifndef __SPRITE_H__
#define __SPRITE_H__
#include <TFT_eSPI.h>


struct AlphaCol{
    uint16_t value : 16;  
    uint8_t alpha : 8;
    uint8_t zero : 8;   
};

class mySprite : public TFT_eSprite {
public:
  mySprite(TFT_eSPI *tft) : TFT_eSprite(tft) {}
  void     pushImageAlpha(int32_t x0, int32_t y0, uint32_t w, uint32_t h, const AlphaCol *data);
};

#endif