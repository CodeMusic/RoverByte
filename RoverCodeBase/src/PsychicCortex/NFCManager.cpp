#include "NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../PrefrontalCortex/SDManager.h"

Adafruit_PN532 NFCManager::nfc(BOARD_PN532_IRQ, BOARD_PN532_RF_REST);
uint32_t NFCManager::lastCardId = 0;
uint32_t NFCManager::totalScans = 0;
bool NFCManager::cardPresent = false;
unsigned long NFCManager::lastInitAttempt = 0;
bool NFCManager::initInProgress = false;
uint8_t NFCManager::initStage = 0;
bool NFCManager::isProcessingScan = false;
char NFCManager::cardData[256] = {0};

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
    
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    if (success) {
        if (uidLength >= 4) {
            uint32_t cardid = uid[0];
            cardid <<= 8; cardid |= uid[1];
            cardid <<= 8; cardid |= uid[2];
            cardid <<= 8; cardid |= uid[3];
            
            if (cardid != lastCardId || (millis() - lastScanTime >= SCAN_COOLDOWN)) {
                isProcessingScan = true;
                lastCardId = cardid;
                lastScanTime = millis();
                totalScans++;
                cardPresent = true;
                
                uint16_t expGain = (cardid % 20) + 1;
                bool isNewCard = !SDManager::hasCardBeenScanned(cardid);
                
                if (isNewCard) {
                    SDManager::recordCardScan(cardid);
                    RoverManager::setTemporaryExpression(RoverManager::EXCITED, 2000, TFT_GOLD);  // Golden stars
                } else {
                    RoverManager::setTemporaryExpression(RoverManager::EXCITED, 2000, TFT_SILVER);  // Silver stars
                }
                
                SoundFxManager::playCardMelody(cardid);
                LEDManager::displayCardPattern(uid, uidLength);
                RoverViewManager::incrementExperience(expGain);
                isProcessingScan = false;
                
                RoverViewManager::showNotification("NFC Card", "Hold still, reading data...", "NFC", 1000);
                readCardData();
                
                char uidStr[32];
                sprintf(uidStr, "ID: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
                RoverViewManager::showNotification("NFC Scan", uidStr, cardData, 3000);
            }
        }
    } else {
        cardPresent = false;
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