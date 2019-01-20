#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__
#include <Arduino.h>

class FileSystem {
    public:
        static inline FileSystem* global(){
            if (FileSystem::_global == NULL){
                FileSystem::_global = new FileSystem();
            }
            return FileSystem::_global;
        }

        bool readCalibrationFile(char* calibrationData);
        void writeCalibrationFile(const char* calibrationData);
    private:
        static FileSystem* _global;
        FileSystem();
};

#endif