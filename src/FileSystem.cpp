#include "FileSystem.h"
#include "ESP32Setup.h"
#include <SD.h>
#include <FS.h>

#define CALIBRATION_FILE "/calibrationData"
FileSystem* FileSystem::_global = NULL;

FileSystem::FileSystem(){
    // check file system
    if (!SD.begin(SDCARD_CS, SPI, 80000000*2)){
        Console.println("SD-Card Initialization failed.");
    }
    Console.printf("SD-Info: %dMB/%dMB \n", SD.usedBytes()/(1024*1024), SD.totalBytes()/(1024*1024));    
}

bool FileSystem::readCalibrationFile(uint8_t* calibrationData, uint8_t sz){
#if HEADLESS
    for (int i=0; i<sz; i++) calibrationData[i] = 0;
    return true;
#else
    bool result = false;
    if (SD.exists(CALIBRATION_FILE)) {
        File f = SD.open(CALIBRATION_FILE, "r");
        if (f) {
            if (f.readBytes((char *)calibrationData, sz) == sz)
                result = true;
            f.close();
        }
    }

    return result;
#endif
}

void FileSystem::writeCalibrationFile(const uint8_t* calibrationData, uint8_t sz) {
#if !HEADLESS
    // store data
    File f = SD.open(CALIBRATION_FILE, "w");
    if (f) {
        f.write((const unsigned char *)calibrationData, sz);
        f.close();
    }
#endif
}