#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#include <Adafruit_PN532.h>
#include "../PrefrontalCortex/utilities.h"

class NFCManager {
public:
    static void init();
    static void update();
    static uint32_t getLastCardId();
    static uint32_t getTotalScans();
    static bool isCardPresent();
    
private:
    static Adafruit_PN532 nfc;
    static uint32_t lastCardId;
    static uint32_t totalScans;
    static bool cardPresent;
};

#endif 