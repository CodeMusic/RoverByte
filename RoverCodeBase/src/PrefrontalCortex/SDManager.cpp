#include "SDManager.h"

bool SDManager::initialized = false;

void SDManager::init() {
    if (!initialized) {
        // Configure SD card pins
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        delay(100);  // Give SD card time to stabilize
        
        // Initialize SPI for SD card with lower speed initially
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
        SPI.setFrequency(4000000);  // Start at 4MHz
        
        unsigned long startTime = millis();
        bool initSuccess = false;
        
        // Try initialization with timeout
        while (millis() - startTime < 3000) {  // 3 second timeout
            if (SD.begin(SD_CS)) {
                initSuccess = true;
                break;
            }
            delay(100);
        }
        
        if (!initSuccess) {
            LOG_ERROR("SD card initialization failed after timeout!");
            return;
        }
        
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            LOG_ERROR("No SD card detected!");
            return;
        }
        
        // Increase SPI speed after successful init
        SPI.setFrequency(20000000);  // Increase to 20MHz
        
        initialized = true;
        LOG_PROD("SD card initialized successfully");
        LOG_PROD("Card Type: %s", 
            cardType == CARD_MMC ? "MMC" :
            cardType == CARD_SD ? "SDSC" :
            cardType == CARD_SDHC ? "SDHC" : "UNKNOWN");
    }
}

bool SDManager::isInitialized() {
    return initialized;
}

File SDManager::openFile(const char* path, const char* mode) {
    if (!initialized) {
        LOG_ERROR("SD card not initialized!");
        return File();
    }
    
    if (strcmp(mode, "r") == 0) {
        return SD.open(path, FILE_READ);
    } else if (strcmp(mode, "w") == 0) {
        return SD.open(path, FILE_WRITE);
    }
    return File();
}

void SDManager::closeFile(File& file) {
    if (file) {
        file.close();
    }
}

bool SDManager::deleteFile(const char* path) {
    if (!initialized) return false;
    return SD.remove(path);
}

bool SDManager::exists(const char* path) {
    if (!initialized) return false;
    return SD.exists(path);
}

size_t SDManager::writeToFile(File& file, const uint8_t* buffer, size_t size) {
    if (!file) return 0;
    return file.write(buffer, size);
}

size_t SDManager::readFromFile(File& file, uint8_t* buffer, size_t size) {
    if (!file) return 0;
    return file.read(buffer, size);
} 