#ifndef __NOPSERIAL_H__
#define __NOPSERIAL_H__
#include <Arduino.h>
#include <HardwareSerial.h>
#include "ESP32Setup.h"


class NopSerial {
    public:
    NopSerial(uint16_t xx){  }
    inline void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL) const {}
    inline size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3))) { return 0; }
    inline size_t print(const __FlashStringHelper *) const { return 0; }
    inline size_t print(const String &) const { return 0; }
    inline size_t print(const char[]) const { return 0; }
    inline size_t print(char) const { return 0; }
    inline size_t print(unsigned char, int = DEC) const { return 0; }
    inline size_t print(int, int = DEC) const { return 0; }
    inline size_t print(unsigned int, int = DEC) const { return 0; }
    inline size_t print(long, int = DEC) const { return 0; }
    inline size_t print(unsigned long, int = DEC) const { return 0; }
    inline size_t print(double, int = 2) const { return 0; }
    inline size_t print(const Printable&) const { return 0; }
    inline size_t print(struct tm * timeinfo, const char * format = NULL) const { return 0; }
    inline size_t println(const __FlashStringHelper *) const { return 0; }
    inline size_t println(const String &s) const { return 0; }
    inline size_t println(const char[]) const { return 0; }
    inline size_t println(char) const { return 0; }
    inline size_t println(unsigned char, int = DEC) const { return 0; }
    inline size_t println(int, int = DEC) const { return 0; }
    inline size_t println(unsigned int, int = DEC) const { return 0; }
    inline size_t println(long, int = DEC) const { return 0; }
    inline size_t println(unsigned long, int = DEC) const { return 0; }
    inline size_t println(double, int = 2) const { return 0; }
    inline size_t println(const Printable&) const { return 0; }
    inline size_t println(struct tm * timeinfo, const char * format = NULL) const { return 0; }
    inline size_t println(void) const { return 0; }
};

#ifdef DEBUG_LOG
#define Console Serial
#else
extern NopSerial NullSerial;
#define Console NullSerial
#endif
#endif