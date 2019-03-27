#ifndef __NOPSERIAL_H__
#define __NOPSERIAL_H__
#include <Arduino.h>
#include <HardwareSerial.h>
#include "ESP32Setup.h"


class NopSerial {
    public:
    NopSerial(uint16_t xx){  }
    inline void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL) const {}
    inline void printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3))) { }
    inline void print(const __FlashStringHelper *) const { }
    inline void print(const String &) const { }
    inline void print(const char[]) const { }
    inline void print(char) const { }
    inline void print(unsigned char, int = DEC) const { }
    inline void print(int, int = DEC) const { }
    inline void print(unsigned int, int = DEC) const { }
    inline void print(long, int = DEC) const { }
    inline void print(unsigned long, int = DEC) const { }
    inline void print(double, int = 2) const { }
    inline void print(const Printable&) const { }
    inline void print(struct tm * timeinfo, const char * format = NULL) const { }
    inline void println(const __FlashStringHelper *) const { }
    inline void println(const String &s) const { }
    inline void println(const char[]) const { }
    inline void println(char) const { }
    inline void println(unsigned char, int = DEC) const { }
    inline void println(int, int = DEC) const { }
    inline void println(unsigned int, int = DEC) const { }
    inline void println(long, int = DEC) const { }
    inline void println(unsigned long, int = DEC) const { }
    inline void println(double, int = 2) const { }
    inline void println(const Printable&) const { }
    inline void println(struct tm * timeinfo, const char * format = NULL) const { }
    inline void println(void) const { }
};

#ifdef DEBUG_LOG
#define Console Serial
#else
extern NopSerial NullSerial;
#define Console NullSerial
#endif
#endif