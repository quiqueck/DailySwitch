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

        bool readCalibrationFile(char* calibrationData);
        void writeCalibrationFile(const char* calibrationData);
    private:
        static FileSystem* _global;
        FileSystem();
};

#endif