#include "SDManager.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/SPIManager.h"

namespace PrefrontalCortex 
{
    bool SDManager::initialized = false;
    const char* SDManager::NFC_FOLDER = "/NFC";
    const char* SDManager::SCANNED_CARDS_FILE = "/NFC/scannedcards.inf";

    uint8_t SDManager::cardType;
    uint64_t SDManager::cardSize = 0;
    uint64_t SDManager::totalSpace = 0;
    uint64_t SDManager::usedSpace = 0;


    uint64_t SDManager::getTotalSpace() {
        return SDManager::totalSpace;
    }

    uint64_t SDManager::getUsedSpace() {
        return SDManager::usedSpace;
    }

    uint64_t SDManager::getCardSize() {
        return SDManager::cardSize;
    }

    #define REASSIGN_PINS
    int sck =  BOARD_SD_SCK;
    int miso =  BOARD_SD_MISO;
    int mosi =  BOARD_SD_MOSI;
    int cs =    BOARD_SD_CS;

    void SDManager::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
        Utilities::LOG_DEBUG("Listing directory: %s\n", dirname);

        File root = fs.open(dirname);
        if(!root){
            Utilities::LOG_ERROR("Failed to open directory");
            return;
        }
        if(!root.isDirectory()){
            Utilities::LOG_ERROR("Not a directory");
            return;
        }

        File file = root.openNextFile();
        while(file){
            if(file.isDirectory()){
                Utilities::LOG_DEBUG("  DIR : %s", file.name());
                if(levels){
                    listDir(fs, file.path(), levels -1);
                }
            } else {
                Utilities::LOG_DEBUG("  FILE: %s  SIZE: %llu", file.name(), file.size());
            }
            file = root.openNextFile();
        }
    }

    void SDManager::createDir(fs::FS &fs, const char * path){
        Serial.printf("Creating Dir: %s\n", path);
        if(fs.mkdir(path)){
            Serial.println("Dir created");
        } else {
            Serial.println("mkdir failed");
        }
    }

    void SDManager::removeDir(fs::FS &fs, const char * path){
        Serial.printf("Removing Dir: %s\n", path);
        if(fs.rmdir(path)){
            Serial.println("Dir removed");
        } else {
            Serial.println("rmdir failed");
        }
    }

    void SDManager::readFile(fs::FS &fs, const char * path){
        Serial.printf("Reading file: %s\n", path);

        File file = fs.open(path);
        if(!file){
            Serial.println("Failed to open file for reading");
            return;
        }

        Serial.print("Read from file: ");
        while(file.available()){
            Serial.write(file.read());
        }
        file.close();
    }

    void SDManager::writeFile(fs::FS &fs, const char * path, const char * message){
        Serial.printf("Writing file: %s\n", path);

        File file = fs.open(path, FILE_WRITE);
        if(!file){
            Serial.println("Failed to open file for writing");
            return;
        }
        if(file.print(message)){
            Serial.println("File written");
        } else {
            Serial.println("Write failed");
        }
        file.close();
    }

    void SDManager::appendFile(fs::FS &fs, const char * path, const char * message){
        Serial.printf("Appending to file: %s\n", path);

        File file = fs.open(path, FILE_APPEND);
        if(!file){
            Serial.println("Failed to open file for appending");
            return;
        }
        if(file.print(message)){
            Serial.println("Message appended");
        } else {
            Serial.println("Append failed");
        }
        file.close();
    }

    void SDManager::renameFile(fs::FS &fs, const char * path1, const char * path2){
        Serial.printf("Renaming file %s to %s\n", path1, path2);
        if (fs.rename(path1, path2)) {
            Serial.println("File renamed");
        } else {
            Serial.println("Rename failed");
        }
    }

    void SDManager::deleteFile(fs::FS &fs, const char * path){
        Serial.printf("Deleting file: %s\n", path);
        if(fs.remove(path)){
            Serial.println("File deleted");
        } else {
            Serial.println("Delete failed");
        }
    }

    void SDManager::testFileIO(fs::FS &fs, const char * path){
        File file = fs.open(path);
        static uint8_t buf[512];
        size_t len = 0;
        uint32_t start = millis();
        uint32_t end = start;
        if(file){
            len = file.size();
            size_t flen = len;
            start = millis();
            while(len){
                size_t toRead = len;
                if(toRead > 512){
                    toRead = 512;
                }
                file.read(buf, toRead);
                len -= toRead;
            }
            end = millis() - start;
            Serial.printf("%u bytes read for %lu ms\n", flen, end);
            file.close();
        } else {
            Serial.println("Failed to open file for reading");
        }


        file = fs.open(path, FILE_WRITE);
        if(!file){
            Serial.println("Failed to open file for writing");
            return;
        }

        size_t i;
        start = millis();
        for(i=0; i<2048; i++){
            file.write(buf, 512);
        }
        end = millis() - start;
        Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
        file.close();
        delete[] buf;
    }

    void SDManager::init(uint8_t cs) {
        if (initialized) return;

        // Initialize SD card
        if (!SD.begin(cs)) {
            Utilities::LOG_ERROR("SD Card initialization failed!");
            return;
        }

        // Check card type
        cardType = SD.cardType();
        if (cardType == CARD_NONE) {
            Utilities::LOG_ERROR("No SD Card detected!");
            return;
        }

        // Log card information
        cardSize = (uint64_t)SD.cardSize() / (1024 * 1024);
        totalSpace = (uint64_t)SD.totalBytes() / (1024 * 1024);
        usedSpace = (uint64_t)SD.usedBytes() / (1024 * 1024);

        const char* cardTypeStr = 
            cardType == CARD_MMC ? "MMC" :
            cardType == CARD_SD ? "SDSC" :
            cardType == CARD_SDHC ? "SDHC" : "UNKNOWN";
        
        Utilities::LOG_DEBUG("SD Card Type: %s", cardTypeStr);
        Utilities::LOG_DEBUG("SD Card Size: %llu MB", cardSize);
        Utilities::LOG_DEBUG("Total space: %llu MB", totalSpace);
        Utilities::LOG_DEBUG("Used space: %llu MB", usedSpace);
        
        initialized = true;
    }



    void SDManager::ensureNFCFolderExists() {
        if (!SD.exists(NFC_FOLDER)) {
            SD.mkdir(NFC_FOLDER);
        }
    }

    bool SDManager::hasCardBeenScanned(uint32_t cardId) {
        return false;
        /*
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
        */
    }

    void SDManager::recordCardScan(uint32_t cardId) {
        return;
        /*
        if (!initialized) return;
        
        ensureNFCFolderExists();
        
        File file = SD.open(SCANNED_CARDS_FILE, FILE_APPEND);
        if (!file) {
            Utilities::LOG_ERROR("Failed to open scanned cards file for writing");
            return;
        }
        
        file.write((uint8_t*)&cardId, sizeof(uint32_t));
        file.close();
        */
    } 

}