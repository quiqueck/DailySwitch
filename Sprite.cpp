#include "Sprite.h"

/***************************************************************************************
** Function name:           pushImage
** Description:             push 565 colour FLASH (PROGMEM) image into a defined area
*************************************************************************************x*/
void  mySprite::pushImageAlpha(int32_t x, int32_t y, uint32_t w, uint32_t h, const AlphaCol *data)
{
  if ((x >= _iwidth) || (y >= _iheight) || (w == 0) || (h == 0) || !_created) return;
  if ((x + (int32_t)w < 0) || (y + (int32_t)h < 0)) return;

  int32_t  xo = 0;
  int32_t  yo = 0;

  int32_t  xs = x;
  int32_t  ys = y;

  uint32_t ws = w;
  uint32_t hs = h;

  if (x < 0) { xo = -x; xs = 0; }
  if (y < 0) { yo = -y; ys = 0; }

  if (xs + w >= _iwidth)  ws = _iwidth  - xs;
  if (ys + h >= _iheight) hs = _iheight - ys;

  AlphaCol inCol;
  for (uint32_t yp = yo; yp < yo + hs; yp++)
    {
      x = xs;
      for (uint32_t xp = xo; xp < xo + ws; xp++)
      {
        AlphaCol color = data[xp + yp * w];
        const uint16_t addr = x + ys * _iwidth;
        inCol.value = _img[addr];
        if(!_iswapBytes) color.value = color.value<<8 | color.value>>8;
        //color.alpha = 0xff;
        //const word = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        uint16_t rIn = (color.value >> 8) & 0xF8;
        uint16_t gIn = (color.value >> 3) & 0xFC;
        uint16_t bIn = (color.value) & 0xF8;
        

        const uint16_t rSrc = (inCol.value >> 8) & 0xF8;
        const uint16_t gSrc = (inCol.value >> 3) & 0xFC;
        const uint16_t bSrc = (inCol.value) & 0xF8;
        const uint16_t aSrc = (0xff - color.alpha);

        rIn = (color.alpha * rIn - aSrc * rSrc) >> 8;
        gIn = (color.alpha * gIn - aSrc * gSrc) >> 8;
        bIn = (color.alpha * bIn - aSrc * bSrc) >> 8;
        //Serial.printf("%d, %d, %X: %X %X %X = %X (%X) -- %X \n", xp, yp, &data[xp + yp * w], rIn, gIn, bIn, color.value, color.alpha, *((uint32_t*)&color));
        //float a = color.alpha / (float)0xff;
        //_img[addr] = color.value;
        //_img[addr] = ((rIn & 0xF8) << 8) | ((gIn & 0xFC) << 3) | (bIn >> 3);
        _img[addr] = alphaBlend(color.alpha, color.value, inCol.value);
        x++;
      }
      ys++;
    }
  
}