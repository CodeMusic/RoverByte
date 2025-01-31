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
    using PC::StorageTypes::StorageStats;
    using PC::StorageTypes::StorageDevice;
    using PC::StorageTypes::FileMetadata;

    // Initialize neural state variables
    bool SDManager::initialized = false;
    uint8_t SDManager::cardType = 0;
    const char* SDManager::NFC_FOLDER = "/nfc";
    const char* SDManager::SCANNED_CARDS_FILE = "/nfc/scanned_cards.txt";
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

    void SDManager::listDir(fs::FS &fs, const char *dirname, uint8_t levels) 
    {
        File root = fs.open(dirname);
        if (!root) 
        {
            Utilities::LOG_ERROR("Memory pathway access failed: %s", dirname);
            return;
        }
        if (!root.isDirectory()) 
        {
            Utilities::LOG_ERROR("Not a memory cluster: %s", dirname);
            return;
        }

        File file = root.openNextFile();
        while (file) 
        {
            if (file.isDirectory()) 
            {
                Utilities::LOG_DEBUG("Memory cluster: %s", file.name());
                if (levels) 
                {
                    listDir(fs, file.path(), levels - 1);
                }
            } 
            else 
            {
                Utilities::LOG_DEBUG("Memory fragment: %s, size: %d", file.name(), file.size());
            }
            file = root.openNextFile();
        }
    }

    void SDManager::createDir(fs::FS &fs, const char *path) 
    {
        if(fs.mkdir(path)){
            Utilities::LOG_DEBUG("Dir created");
        } else {
            Utilities::LOG_DEBUG("mkdir failed");
        }
    }

    void SDManager::removeDir(fs::FS &fs, const char *path) 
    {
        if(fs.rmdir(path)){
            Utilities::LOG_DEBUG("Dir removed");
        } else {
            Utilities::LOG_DEBUG("rmdir failed");
        }
    }

    void SDManager::readFile(fs::FS &fs, const char *path) 
    {
        File file = fs.open(path);
        if(!file){
            Utilities::LOG_ERROR("Failed to open file for reading");
            return;
        }

        Utilities::LOG_DEBUG("Read from file: ");
        while(file.available()){
            Utilities::LOG_DEBUG("%c", file.read());
        }
        file.close();
    }

    void SDManager::writeFile(fs::FS &fs, const char *path, const char *message) 
    {
        File file = fs.open(path, FILE_WRITE);
        if(!file){
            Utilities::LOG_ERROR("Failed to open file for writing");
            return;
        }
        if(file.print(message)){
            Utilities::LOG_DEBUG("File written");
        } else {
            Utilities::LOG_ERROR("Write failed");
        }
        file.close();
    }

    void SDManager::appendFile(fs::FS &fs, const char *path, const char *message) 
    {
        File file = fs.open(path, FILE_APPEND);
        if(!file){
            Utilities::LOG_ERROR("Failed to open file for appending");
            return;
        }
        if(file.print(message)){
            Utilities::LOG_DEBUG("Message appended");
        } else {
            Utilities::LOG_ERROR("Append failed");
        }
        file.close();
    }

    void SDManager::renameFile(fs::FS &fs, const char *path1, const char *path2) 
    {
        if (fs.rename(path1, path2)) {
            Utilities::LOG_DEBUG("File renamed");
        } else {
            Utilities::LOG_ERROR("Rename failed");
        }
    }

    void SDManager::deleteFile(fs::FS &fs, const char *path) 
    {
        if(fs.remove(path)){
            Utilities::LOG_DEBUG("File deleted");
        } else {
            Utilities::LOG_ERROR("Delete failed");
        }
    }

    void SDManager::testFileIO(fs::FS &fs, const char *path) 
    {
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
            Utilities::LOG_DEBUG("%u bytes read for %lu ms\n", flen, end);
            file.close();
        } else {
            Utilities::LOG_ERROR("Failed to open file for reading");
        }

        file = fs.open(path, FILE_WRITE);
        if(!file){
            Utilities::LOG_ERROR("Failed to open file for writing");
            return;
        }

        size_t i;
        start = millis();
        for(i=0; i<2048; i++){
            file.write(buf, 512);
        }
        end = millis() - start;
        Utilities::LOG_DEBUG("%u bytes written for %lu ms\n", 2048 * 512, end);
        file.close();
        delete[] buf;
    }

    void SDManager::init(uint8_t cs) 
    {
        if (!SD.begin(cs)) 
        {
            Utilities::LOG_ERROR("Memory pathway initialization failed");
            return;
        }

        cardType = SD.cardType();
        if (cardType == CARD_NONE) 
        {
            Utilities::LOG_ERROR("No memory storage detected");
            return;
        }

        // Initialize memory capacity metrics
        cardSize = SD.cardSize() / (1024 * 1024);
        totalSpace = SD.totalBytes() / (1024 * 1024);
        usedSpace = SD.usedBytes() / (1024 * 1024);

        const char* cardTypeStr = 
            cardType == CARD_MMC ? "MMC" :
            cardType == CARD_SD ? "SDSC" :
            cardType == CARD_SDHC ? "SDHC" : "UNKNOWN";
        
        Utilities::LOG_DEBUG("SD Card Type: %s", cardTypeStr);
        Utilities::LOG_DEBUG("SD Card Size: %llu MB", cardSize);
        Utilities::LOG_DEBUG("Total space: %llu MB", totalSpace);
        Utilities::LOG_DEBUG("Used space: %llu MB", usedSpace);
        
        initialized = true;
        Utilities::LOG_DEBUG("Memory pathways initialized successfully");
        
        // Ensure experiential memory structure exists
        ensureNFCFolderExists();
    }

    void SDManager::ensureNFCFolderExists() 
    {
        if (!SD.exists(NFC_FOLDER)) {
            SD.mkdir(NFC_FOLDER);
        }
    }

    bool SDManager::hasCardBeenScanned(uint32_t cardId) 
    {
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

    void SDManager::recordCardScan(uint32_t cardId) 
    {
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