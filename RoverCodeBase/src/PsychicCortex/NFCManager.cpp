#include "NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../PrefrontalCortex/SDManager.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <string.h> // For strcmp

// Define your I2C or SPI connection to the PN532 module
#define SDA_PIN 4
#define SCL_PIN 5
Adafruit_PN532 NFCManager::nfc(SDA_PIN, SCL_PIN);
uint32_t NFCManager::lastCardId = 0;
uint32_t NFCManager::totalScans = 0;
bool NFCManager::cardPresent = false;
unsigned long NFCManager::lastInitAttempt = 0;
bool NFCManager::initInProgress = false;
uint8_t NFCManager::initStage = 0;
bool NFCManager::isProcessingScan = false;
char NFCManager::cardData[256] = {0};

// Example valid card IDs
const char* validCardIDs[] = {
    "ROVER123",
    "BYTE456",
    "ROVERBYTE789"
};

void NFCManager::init() {
    LOG_PROD("Starting NFC initialization...");
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    
    nfc.begin();
    delay(1000);
    
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        LOG_ERROR("Didn't find PN532 board");
        return;
    }
    
    LOG_PROD("Found chip PN5%x", (versiondata >> 24) & 0xFF);
    LOG_PROD("Firmware ver. %d.%d", (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);
    
    nfc.SAMConfig();
    LOG_PROD("NFC initialization complete");
    
    SoundFxManager::playStartupSound();
}

void NFCManager::startBackgroundInit() {
    initInProgress = true;
    initStage = 0;
    lastInitAttempt = millis();
}

void NFCManager::update() {
    if (initInProgress && !SoundFxManager::isPlaying()) {
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
                    LOG_PROD("NFC initialization complete");
                } else {
                    LOG_DEBUG("NFC init retry...");
                }
                break;
        }
        lastInitAttempt = millis();
        return;
    }
    
    if (isProcessingScan || SoundFxManager::isPlaying()) return;
    
    static unsigned long lastScanTime = 0;
    const unsigned long SCAN_COOLDOWN = 5000;
    
    if (millis() - lastScanTime < SCAN_COOLDOWN) return;
    
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        if (!cardPresent) {
            cardPresent = true;
            LEDManager::setPattern(Pattern::NFC_SCAN);
            
            if (isCardValid()) {
                LEDManager::handleMessage(LEDMessage::NFC_DETECTED);
                
                // Calculate experience based on card data
                uint16_t expGain = (uid[0] + uid[1] + uid[2] + uid[3]) % 50 + 10;
                RoverViewManager::incrementExperience(expGain);
                
                // Start entertainment pattern using card data
                LEDManager::displayCardPattern(uid, uidLength);
                
                // Read card data and generate song
                readCardData();
                VisualSynesthesia::playNFCCardData(cardData);
            } else { 
                LEDManager::handleMessage(LEDMessage::NFC_ERROR);
            }
        }
    } else {
        cardPresent = false;
        LEDManager::setPattern(Pattern::NONE);
    }
}

void NFCManager::readCardData() {
    memset(cardData, 0, sizeof(cardData));
    
    if (isCardEncrypted()) {
        strcpy(cardData, "CARD ENCRYPTED");
        return;
    }
    
    uint8_t data[4];
    int dataIndex = 0;
    
    for (uint8_t page = 4; page < PAGE_COUNT && dataIndex < sizeof(cardData) - 1; page++) {
        if (nfc.ntag2xx_ReadPage(page, data)) {
            for (int i = 0; i < 4 && dataIndex < sizeof(cardData) - 1; i++) {
                if (data[i] >= 32 && data[i] <= 126) {  // Printable ASCII only
                    cardData[dataIndex++] = data[i];
                }
            }
        } else {
            break;
        }
    }
    cardData[dataIndex] = '\0';
}

void NFCManager::handleNFCScan() {
    if (isCardPresent()) {
        // Card present - start NFC scan flow
        SoundFxManager::playVoiceLine("card_detected");
        checkForCard();  // Process the card
    } else {
        Serial.println("No card present");
    }
}

bool NFCManager::isCardEncrypted() {
    return checkForEncryption();
}

bool NFCManager::checkForEncryption() {
    uint8_t data[4];
    // Check authentication block
    if (!nfc.ntag2xx_ReadPage(3, data)) {
        return true; // If we can't read the auth block, assume encrypted
    }
    // Check if any authentication bits are set
    return (data[3] & 0x0F) != 0;
}

void NFCManager::checkForCard() {
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    if (success) {
        lastCardId = (uid[0] << 24) | (uid[1] << 16) | (uid[2] << 8) | uid[3];
        cardPresent = true;
        LEDManager::displayCardPattern(uid, uidLength);
        readCardData();
    } else {
        cardPresent = false;
    }
}

void NFCManager::stop() {
    isProcessingScan = false;
    initInProgress = false;
    cardPresent = false;
}

bool NFCManager::isCardValid() {
    // Assuming cardData holds the scanned card ID
    for (const char* validID : validCardIDs) {
        if (strcmp(cardData, validID) == 0) {
            Serial.println("Card is valid: " + String(cardData));
            return true; // Card is valid
        }
    }
    Serial.println("Card is not valid: " + String(cardData));
    return true; // Card is not valid (FOR NOW WE ARE JUST GOING TO ASSUME IT IS VALID)
}