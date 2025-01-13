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
    static const int SD_CS_PIN = 4;  // Adjust based on your hardware
};

#endif 