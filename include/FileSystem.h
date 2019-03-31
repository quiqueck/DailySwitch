#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__
#include <Arduino.h>
#include "NopSerial.h"

class FileSystem {
    public:
        static inline FileSystem* global(){
            if (FileSystem::_global == NULL){
                FileSystem::_global = new FileSystem();
            }
            return FileSystem::_global;
        }

        static inline void init(){
            if (FileSystem::_global == NULL){
                FileSystem::_global = new FileSystem();
            }
        }

        bool readCalibrationFile(uint8_t* calibrationData, uint8_t sz);
        void writeCalibrationFile(const uint8_t* calibrationData, uint8_t sz);
    private:
        static FileSystem* _global;
        FileSystem();
};

#endif