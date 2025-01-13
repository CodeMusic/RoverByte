#pragma once
#include <SD.h>
#include "../PrefrontalCortex/utilities.h"

class SDManager {
public:
    static void init();
    static bool isInitialized() { return initialized; }
    static bool hasCardBeenScanned(uint32_t cardId);
    static void recordCardScan(uint32_t cardId);
    static void handleSDCardOperation();
    
    // Add these file operation methods
    static File openFile(const char* path, const char* mode);
    static void closeFile(File& file);
    static bool deleteFile(const char* path);
    static bool exists(const char* path);
    static size_t writeToFile(File& file, const uint8_t* buffer, size_t size);
    static size_t readFromFile(File& file, uint8_t* buffer, size_t size);

private:
    static bool initialized;
    static const char* NFC_FOLDER;
    static const char* SCANNED_CARDS_FILE;
    static void ensureNFCFolderExists();
}; 