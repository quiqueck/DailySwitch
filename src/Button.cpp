#include "Button.h"
#include "SwitchUI.h"


void Button::draw(class SwitchUI* ui, uint16_t oCol) const{
    ui->tft.fillRect(l, t, w(),h(), oCol);
}