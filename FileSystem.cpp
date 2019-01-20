#include "FileSystem.h"
#include <SPIFFS.h>
#include <FS.h>

#define CALIBRATION_FILE "/calibrationData"
FileSystem* FileSystem::_global = NULL;

FileSystem::FileSystem(){
    // check file system
    if (!SPIFFS.begin()) {
        Serial.println("Formating File System");

        SPIFFS.format();
        SPIFFS.begin();
    }
}

bool FileSystem::readCalibrationFile(char* calibrationData){
    bool result = false;
    if (SPIFFS.exists(CALIBRATION_FILE)) {
        File f = SPIFFS.open(CALIBRATION_FILE, "r");
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
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
        f.write((const unsigned char *)calibrationData, 14);
        f.close();
    }
}