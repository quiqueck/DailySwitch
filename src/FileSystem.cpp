#include "FileSystem.h"
#include "ESP32Setup.h"
#include <SD.h>
#include <FS.h>

#define CALIBRATION_FILE "/calibrationData"
FileSystem* FileSystem::_global = NULL;

FileSystem::FileSystem(){
    // check file system
    if (!SD.begin(SDCARD_CS, SPI, SPI_FREQUENCY*2)){
        Serial.println("SD-Card Initialization failed.");
    }
    Serial.printf("SD-Info: %dMB/%dMB \n", SD.usedBytes()/(1024*1024), SD.totalBytes()/(1024*1024));    
}

bool FileSystem::readCalibrationFile(char* calibrationData){
    bool result = false;
    if (SD.exists(CALIBRATION_FILE)) {
        File f = SD.open(CALIBRATION_FILE, "r");
        if (f) {
            if (f.readBytes((char *)calibrationData, 14) == 14)
                result = true;
            f.close();
        }
    }

    return result;
}

void FileSystem::writeCalibrationFile(const char* calibrationData) {
    // store data
    File f = SD.open(CALIBRATION_FILE, "w");
    if (f) {
        f.write((const unsigned char *)calibrationData, 14);
        f.close();
    }
}