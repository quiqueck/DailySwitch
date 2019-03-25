// See SetupX_Template.h for all options available

#include <analogWrite.h>

//#define HEADLESS
//#define SI7021_DRIVER  //Temperature/Humidity
//#define BH1750_DRIVER  //LightMeter

//#define RPI_ILI9486_DRIVER // 20MHz maximum SPI
#define ILI9486_DRIVER
#define TFT_BACKLIGHT_ON HIGH

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_BL    17  // Data Command control pin
#define TFT_RST   21  // Reset pin (could connect to RST pin)
#define TFT_IRQ   GPIO_NUM_32
//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TOUCH_CS 22     // Chip select pin (T_CS) of touch screen
#define SDCARD_CS 15

#define IC2_CLK GPIO_NUM_14
#define IC2_DAT GPIO_NUM_26

#define LED GPIO_NUM_2

#define AUDIO_PIN GPIO_NUM_34

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define SMOOTH_FONT


#define SPI_FREQUENCY  80000000 
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define IC2_FREQUENCY 100000

#define USER_SETUP_LOADED 