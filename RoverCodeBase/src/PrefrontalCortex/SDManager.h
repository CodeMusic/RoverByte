#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../MotorCortex/PinDefinitions.h"

namespace PrefrontalCortex 
{
    using PC::StorageTypes::StorageStats;

    /**
     * @brief Manages long-term memory storage and retrieval pathways
     * 
     * Controls:
     * - Memory formation and storage
     * - Information retrieval patterns
     * - Memory consolidation processes
     * - Neural pathway persistence
     * - Experiential data recording
     */
    class SDManager 
    {
    public:
        // Core memory functions
        static void init(uint8_t cs);
        static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
        static void createDir(fs::FS &fs, const char *path);
        static void removeDir(fs::FS &fs, const char *path);
        static void readFile(fs::FS &fs, const char *path);
        static void writeFile(fs::FS &fs, const char *path, const char *message);
        static void appendFile(fs::FS &fs, const char *path, const char *message);
        static void renameFile(fs::FS &fs, const char *path1, const char *path2);
        static void deleteFile(fs::FS &fs, const char *path);
        static void testFileIO(fs::FS &fs, const char *path);
        static bool isInitialized() { return initialized; }

        // Experiential memory management
        static void ensureNFCFolderExists();
        static bool hasCardBeenScanned(uint32_t cardId);
        static void recordCardScan(uint32_t cardId);

        // Memory capacity assessment
        static uint64_t getCardSize();
        static uint64_t getTotalSpace();
        static uint64_t getUsedSpace();

    private:
        // Neural state variables
        static bool initialized;
        static uint8_t cardType;
        static const char* NFC_FOLDER;
        static const char* SCANNED_CARDS_FILE;
        static uint64_t cardSize;
        static uint64_t totalSpace;
        static uint64_t usedSpace;
    };
}

#endif // SDMANAGER_H