#include "NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"

Adafruit_PN532 NFCManager::nfc(BOARD_PN532_IRQ, BOARD_PN532_RF_REST);
uint32_t NFCManager::lastCardId = 0;
uint32_t NFCManager::totalScans = 0;
bool NFCManager::cardPresent = false;

void NFCManager::init() {
    delay(100);  // Give hardware time to stabilize
    nfc.begin();
    
    // Try multiple times to get firmware version
    for(int i = 0; i < 3; i++) {
        if (nfc.getFirmwareVersion()) {
            nfc.setPassiveActivationRetries(0xFF);
            LOG_PROD("NFC initialized successfully");
            return;
        }
        delay(100);
    }
    
    LOG_ERROR("Didn't find PN532 board after multiple attempts");
}

void NFCManager::update() {
    static bool wasCardPresent = false;
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    if (success) {
        if (uidLength == 4) {
            uint32_t cardid = uid[0];
            cardid <<= 8; cardid |= uid[1];
            cardid <<= 8; cardid |= uid[2];
            cardid <<= 8; cardid |= uid[3];
            
            if (!wasCardPresent) {  // Card just arrived
                if (cardid != lastCardId) {
                    lastCardId = cardid;
                    totalScans++;
                    // Play success sound and flash LED
                    SoundFxManager::playVoiceLine("card_detected");
                    LEDManager::flashSuccess();
                }
                cardPresent = true;
                wasCardPresent = true;
            }
        }
    } else {
        cardPresent = false;
        wasCardPresent = false;
    }
}

uint32_t NFCManager::getLastCardId() {
    return lastCardId;
}

uint32_t NFCManager::getTotalScans() {
    return totalScans;
}

bool NFCManager::isCardPresent() {
    return cardPresent;
}