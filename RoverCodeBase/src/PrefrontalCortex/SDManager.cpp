#include "SDManager.h"

bool SDManager::initialized = false;

void SDManager::init() {
    if (!initialized) {
        if (!SD.begin(SD_CS_PIN)) {
            LOG_ERROR("SD card initialization failed!");
            return;
        }
        initialized = true;
        LOG_PROD("SD card initialized successfully");
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