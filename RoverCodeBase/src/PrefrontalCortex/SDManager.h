#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

class SDManager {
public:
    static void init();
    static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
    static void createDir(fs::FS &fs, const char *path);
    static void removeDir(fs::FS &fs, const char *path);
    static void readFile(fs::FS &fs, const char *path);
    static void writeFile(fs::FS &fs, const char *path, const char *message);
    static void appendFile(fs::FS &fs, const char *path, const char *message);
    static void renameFile(fs::FS &fs, const char *path1, const char *path2);
    static void deleteFile(fs::FS &fs, const char *path);
    static void testFileIO(fs::FS &fs, const char *path);
    static bool isInitialized();

    static void ensureNFCFolderExists();
    static bool hasCardBeenScanned(uint32_t cardId);
    static void recordCardScan(uint32_t cardId);

private:
    static bool initialized;
    static const char* NFC_FOLDER;
    static const char* SCANNED_CARDS_FILE;
};

#endif // SDMANAGER_H