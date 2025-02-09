#include "NFCManager.h"
#include "AuditoryCortex/SoundFxManager.h"
#include "VisualCortex/LEDManager.h"
#include "VisualCortex/RoverViewManager.h"
#include "VisualCortex/VisualSynesthesia.h"
#include "PrefrontalCortex/SDManager.h"
#include "PrefrontalCortex/Utilities.h"
#include "PrefrontalCortex/ProtoPerceptions.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <string.h> // For strcmp

#define DEBUG_MODE 

using namespace CorpusCallosum;

namespace PsychicCortex 
{
    // Initialize static members with default values
    Adafruit_PN532 NFCManager::nfc(SDA_PIN, SCL_PIN);
    uint32_t NFCManager::lastCardId = 0;
    uint32_t NFCManager::totalScans = 0;
    bool NFCManager::cardPresent = false;
    unsigned long NFCManager::lastInitAttempt = 0;
    bool NFCManager::initInProgress = false;
    uint8_t NFCManager::initStage = 0;
    bool NFCManager::isProcessingScan = false;
    char NFCManager::cardData[256] = {0};
    bool NFCManager::isReadingData = false;
    InitState NFCManager::initState = InitState::NOT_STARTED;
    uint8_t NFCManager::initRetries = 0;
    unsigned long NFCManager::initStartTime = 0;

    // Example valid card IDs for testing and validation
    const char* validCardIDs[] = {
        "ROVER123",
        "BYTE456",
        "ROVERBYTE789"
    };

    /**
     * @brief Initialize NFC hardware and configure for operation
     * Sets up I2C communication, verifies hardware presence,
     * and configures the PN532 chip for card reading
     */
    void NFCManager::init() {
        PrefrontalCortex::Utilities::LOG_PROD("Starting NFC initialization...");
        Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
        
        nfc.begin();
        delay(1000);
        
        uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata) {
            PrefrontalCortex::Utilities::LOG_ERROR("Didn't find PN532 board");
            return;
        }
        
        PrefrontalCortex::Utilities::LOG_PROD("Found chip PN5%x", (versiondata >> 24) & 0xFF);
        PrefrontalCortex::Utilities::LOG_PROD("Firmware ver. %d.%d", (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);
        
        nfc.SAMConfig();
        PrefrontalCortex::Utilities::LOG_PROD("NFC initialization complete");
        
        AuditoryCortex::SoundFxManager::playStartupSound();
    }

    /**
     * @brief Handle rotary encoder rotation events
     * Updates NFC state and provides audio feedback
     * @param direction Rotation direction (+/-)
     */
    void NFCManager::handleRotaryTurn(int direction) {
    update();
    AC::SoundFxManager::playVoiceLine("waiting_for_card");
    }

    /**
     * @brief Process rotary encoder button press
     * Initiates card scanning and provides feedback
     */
    void NFCManager::handleRotaryPress() {
        checkForCard();
        AC::SoundFxManager::playVoiceLine("card_detected");
    }

    /**
     * @brief Begin background initialization sequence
     * Starts multi-stage initialization process
     */
    void NFCManager::startBackgroundInit() {
        initInProgress = true;
        initStage = 0;
        lastInitAttempt = millis();
    }

    /**
     * @brief Main update loop for NFC operations
     * Handles initialization, card detection, and data processing
     */
    void NFCManager::update() {
        if (initInProgress && !AC::SoundFxManager::isPlaying()) {
            if (millis() - lastInitAttempt < 1000) return; // Don't try too frequently
            
            switch (initStage) {
                case 0:
                    nfc.begin();
                    initStage++;
                    break;
                    
                case 1:
                    if (nfc.getFirmwareVersion()) {
                        nfc.SAMConfig();
                        initInProgress = false;
                        PC::Utilities::LOG_PROD("NFC initialization complete");
                    } else {
                        PC::Utilities::LOG_DEBUG("NFC init retry...");
                    }
                    break;
            }
            lastInitAttempt = millis();
            return;
        }
        
        if (isProcessingScan || AC::SoundFxManager::isPlaying()) return;
        
        static unsigned long lastScanTime = 0;
        const unsigned long SCAN_COOLDOWN = 5000;
        
        if (millis() - lastScanTime < SCAN_COOLDOWN) return;
        
        uint8_t success;
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;
        
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
            if (!cardPresent) {
                cardPresent = true;
                VC::LEDManager::setPattern(PC::VisualPattern::NFC_SCAN);
                
                if (isCardValid()) {
                    VC::LEDManager::handleMessage(PC::VisualMessage::NFC_DETECTED);
                    
                    // Calculate experience based on card data
                    uint16_t expGain = (uid[0] + uid[1] + uid[2] + uid[3]) % 50 + 10;
                    VC::RoverViewManager::incrementExperience(expGain);
                    
                    // Start entertainment pattern using card data
                    VC::LEDManager::displayCardPattern(uid, uidLength);
                    
                    // Read card data and generate song
                    readCardData();
                    VC::VisualSynesthesia::playNFCCardData(cardData);
                } else { 
                    VC::LEDManager::handleMessage(PC::VisualMessage::NFC_ERROR);
                }
            }
        } else {
            cardPresent = false;
            VC::LEDManager::setPattern(PC::VisualPattern::NONE);
        }
    }

    /**
     * @brief Read data from detected NFC card
     * Processes card pages and handles encryption
     */
    void NFCManager::readCardData() {
        memset(cardData, 0, sizeof(cardData));
        
        if (isCardEncrypted()) {
            strcpy(cardData, "CARD ENCRYPTED");
            return;
        }
        
        uint8_t data[4];
        int dataIndex = 0;
        const uint8_t MAX_PAGES = 32; // Protect against buffer overflow
        
        for (uint8_t page = 4; page < MAX_PAGES && dataIndex < sizeof(cardData) - 1; page++) {
            if (nfc.ntag2xx_ReadPage(page, data)) {
                for (int i = 0; i < 4 && dataIndex < sizeof(cardData) - 1; i++) {
                    if (data[i] >= 32 && data[i] <= 126)  // Printable ASCII only
                    {
                        cardData[dataIndex++] = data[i];
                    }
                }
            } else {
                break; // Stop reading if page read fails
            }
        }
        
        cardData[dataIndex] = '\0'; // Ensure null termination
    }

    /**
     * @brief Scan for presence of NFC card
     * Reads card UID and updates system state
     */
    void NFCManager::handleNFCScan() {
        if (isCardPresent()) {
            // Card present - start NFC scan flow
            AC::SoundFxManager::playVoiceLine("card_detected");
            checkForCard();  // Process the card
        } else {
            Serial.println("No card present");
        }
    }

    /**
     * @brief Check if card data is encrypted
     * Examines authentication block for encryption flags
     */
    bool NFCManager::isCardEncrypted() {
        return checkForEncryption();
    }

    /**
     * @brief Verify encryption status of card
     * Reads authentication block and checks security bits
     */
    bool NFCManager::checkForEncryption() {
        uint8_t data[4];
        // Check authentication block
        if (!nfc.ntag2xx_ReadPage(3, data)) {
            return true; // If we can't read the auth block, assume encrypted
        }
        // Check if any authentication bits are set
        return (data[3] & 0x0F) != 0;
    }

    /**
     * @brief Scan for presence of NFC card
     * Reads card UID and updates system state
     */
    void NFCManager::checkForCard() {
        uint8_t success;
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;
        
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        
        if (success) {
            lastCardId = (uid[0] << 24) | (uid[1] << 16) | (uid[2] << 8) | uid[3];
            cardPresent = true;
            VC::LEDManager::displayCardPattern(uid, uidLength);
            readCardData();
        } else {
            cardPresent = false;
        }
    }

    /**
     * @brief Stop all NFC operations
     * Resets scanning and initialization states
     */
    void NFCManager::stop() {
        isProcessingScan = false;
        initInProgress = false;
        cardPresent = false;
    }

    /**
     * @brief Validate detected card
     * Checks card data against known valid patterns
     */
    bool NFCManager::isCardValid() 
    {
        if (strlen(cardData) == 0) 
        {
            PC::Utilities::LOG_DEBUG("Card data is empty");
            return false;
        }

        for (const char* validID : validCardIDs) 
        {
            if (strcmp(cardData, validID) == 0) 
            {
                char logBuffer[64];
                snprintf(logBuffer, sizeof(logBuffer), "Valid card detected: %s", cardData);
                PC::Utilities::LOG_PROD(logBuffer);
                return true;
            }
        }
        
        char logBuffer[64];
        snprintf(logBuffer, sizeof(logBuffer), "Invalid card detected: %s", cardData);
        PC::Utilities::LOG_DEBUG(logBuffer);
        #ifdef DEBUG_MODE
            return true; // In debug mode, accept all cards
        #else
            return false; // In production, only accept valid cards
        #endif
    }

}