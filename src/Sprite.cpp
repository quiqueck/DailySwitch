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
        else {
          inCol.value = inCol.value<<8 | inCol.value>>8;
          color.value = color.value<<8 | color.value>>8;        
        }
        const uint16_t rIn = (color.value >> 8) & 0x00F8;
        const uint16_t gIn = (color.value >> 3) & 0x00FC;
        const uint16_t bIn = (color.value << 3) & 0x00F8;
        
        const uint16_t bSrc = (inCol.value >> 8) & 0x00F8;
        const uint16_t gSrc = (inCol.value >> 3) & 0x00FC;
        const uint16_t rSrc = (inCol.value << 3) & 0x00F8;    
        const uint16_t aSrc = (0xff - color.alpha);

        const uint16_t rOut = (color.alpha * rIn + aSrc * rSrc) >> 8;
        const uint16_t gOut = (color.alpha * gIn + aSrc * gSrc) >> 8;
        const uint16_t bOut = (color.alpha * bIn + aSrc * bSrc) >> 8;

        color.value = ((rOut & 0xF8) << 8) | ((gOut & 0xFC) << 3) | (bOut >> 3);
        if(_iswapBytes) color.value = color.value<<8 | color.value>>8;
        _img[addr] = color.value;
        x++;
      }
      ys++;
    }
  
}