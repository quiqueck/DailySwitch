// See SetupX_Template.h for all options available

#include <analogWrite.h>

//#define DEBUG_LOG
//#define HEADLESS
#define PROXIMITY 1
//#define TEST_TOUCH
//#define DISABLE_BUTTON_PRESS
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


#define PROXIMITY_INT_PIN GPIO_NUM_33

/*#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts*/

#define SMOOTH_FONT


#define SPI_FREQUENCY  80000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define IC2_FREQUENCY 100000

#define USER_SETUP_LOADED 