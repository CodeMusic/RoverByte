#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <SD.h>
#include <FS.h>
#include "utilities.h"

class SDManager {
public:
    static void init();
    static bool isInitialized();
    static File openFile(const char* path, const char* mode);
    static void closeFile(File& file);
    static bool deleteFile(const char* path);
    static bool exists(const char* path);
    static size_t writeToFile(File& file, const uint8_t* buffer, size_t size);
    static size_t readFromFile(File& file, uint8_t* buffer, size_t size);
    
private:
    static bool initialized;
    static const int SD_SCK = BOARD_SD_SCK;   // Pin 11
    static const int SD_MISO = BOARD_SD_MISO; // Pin 10
    static const int SD_MOSI = BOARD_SD_MOSI; // Pin 9
    static const int SD_CS = BOARD_SD_CS;     // Pin 13
};

#endif 