#include "SDManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
bool SDManager::initialized = false;
const char* SDManager::NFC_FOLDER = "/NFC";
const char* SDManager::SCANNED_CARDS_FILE = "/NFC/scannedcards.inf";

void SDManager::init() {
    if (!initialized) {
        // Configure SD card pins
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        
        // Initialize SPI for SD card with lower speed initially
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
        SPI.setFrequency(4000000);  // Start at 4MHz
        
        if (!SD.begin(SD_CS)) {
            LOG_ERROR("SD card initialization failed!");
            // Continue without SD - it's not critical for core functionality
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
    }
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

void SDManager::ensureNFCFolderExists() {
    if (!SD.exists(NFC_FOLDER)) {
        SD.mkdir(NFC_FOLDER);
    }
}

bool SDManager::hasCardBeenScanned(uint32_t cardId) {
    if (!initialized) return false;
    
    File file = SD.open(SCANNED_CARDS_FILE, FILE_READ);
    if (!file) return false;
    
    uint32_t storedId;
    bool found = false;
    
    while (file.available() >= sizeof(uint32_t)) {
        file.read((uint8_t*)&storedId, sizeof(uint32_t));
        if (storedId == cardId) {
            found = true;
            break;
        }
    }
    
    file.close();
    return found;
}

void SDManager::recordCardScan(uint32_t cardId) {
    if (!initialized) return;
    
    ensureNFCFolderExists();
    
    File file = SD.open(SCANNED_CARDS_FILE, FILE_APPEND);
    if (!file) {
        LOG_ERROR("Failed to open scanned cards file for writing");
        return;
    }
    
    file.write((uint8_t*)&cardId, sizeof(uint32_t));
    file.close();
} 

void SDManager::handleSDCardOperation() {
    if (!SD.begin()) {
        LOG_ERROR("SD Card initialization failed");
        LEDManager::setLED(0, CRGB::Red);
        LEDManager::showLEDs();
        RoverManager::setEarsDown();
        delay(1000);
        return;
    }
}