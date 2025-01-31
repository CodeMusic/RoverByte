#ifndef NFC_MANAGER_H
#define NFC_MANAGER_H

#include <Adafruit_PN532.h>
#include "../PrefrontalCortex/utilities.h"
#include "../VisualCortex/LEDManager.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"

namespace PsychicCortex 
{
    using namespace CorpusCallosum;
    using VC::VisualPattern;
    using VC::VisualMessage;
    using PC::NFCTypes::InitState;

    class NFCManager {
    public:
        static constexpr size_t MAX_CARD_DATA = 256;
        static constexpr uint8_t MAX_INIT_RETRIES = 3;
        static constexpr unsigned long INIT_TIMEOUT_MS = 5000;
        
        static void init();
        static void update();
        static void checkForCard();
        static bool isCardPresent() { return cardPresent; }
        static uint32_t getTotalScans() { return totalScans; }
        static uint32_t getLastCardId() { return lastCardId; }
        static void handleNFCScan();
        static bool isCardEncrypted();
        static const char* getCardData() { return cardData; }
        static void readCardData();
        static void startBackgroundInit();
        static bool isInitializing() { return initInProgress; }
        static void stop();
        static bool isCardValid();
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();
        
    private:
        static Adafruit_PN532 nfc;
        static uint32_t lastCardId;
        static uint32_t totalScans;
        static bool cardPresent;
        static unsigned long lastInitAttempt;
        static bool initInProgress;
        static uint8_t initStage;
        static bool isProcessingScan;
        static char cardData[256];
        static bool isReadingData;
        static const uint8_t PAGE_COUNT = 135;
        static bool checkForEncryption();
        static InitState initState;
        static uint8_t initRetries;
        static unsigned long initStartTime;
    };

}

#endif 