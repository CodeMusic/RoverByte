#include "IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"

bool IRManager::blasting = false;
unsigned long IRManager::lastSendTime = 0;
unsigned long IRManager::lastLEDUpdate = 0;
uint16_t IRManager::currentCode = 0;
uint8_t IRManager::currentRegion = 0;
uint8_t IRManager::currentLEDPosition = 0;
bool IRManager::animationDirection = true;
IRsend IRManager::irsend(BOARD_IR_RX);  // Initialize with IR pin

void IRManager::init() {
    pinMode(BOARD_IR_EN, OUTPUT);
    pinMode(BOARD_IR_RX, OUTPUT);
    digitalWrite(BOARD_IR_EN, LOW); // Initially disabled
    irsend.begin();
}

void IRManager::startBlast() {
    blasting = true;
    digitalWrite(BOARD_IR_EN, HIGH);
    lastSendTime = millis();
    lastLEDUpdate = millis();
    currentLEDPosition = 4;  // Start at center
    currentCode = 0;
    currentRegion = 0;
    animationDirection = true;
    
    LEDManager::setMode(Mode::OFF_MODE);  // Prevent LED pattern interference
    FastLED.clear();
    LEDManager::setLED(4, CRGB::Red);  // Center LED
    LEDManager::showLEDs();
    
    MenuManager::hide();
    RoverViewManager::showNotification("IR", "Starting blast...", "BLAST", 1000);
}

void IRManager::stopBlast() {
    blasting = false;
    digitalWrite(BOARD_IR_EN, LOW); // Disable IR
    FastLED.clear();
    FastLED.show();
    RoverViewManager::showNotification("IR Blast", "Stopped!", "IR", 2000);
}
void IRManager::update() {
    if (!blasting) return;
    
    // Update LED animation
    if (millis() - lastLEDUpdate >= LED_UPDATE_MS) {
        updateLEDAnimation();
        lastLEDUpdate = millis();
    }
    
    // Update IR codes
    if (millis() - lastSendTime >= SEND_DELAY_MS) {
        sendCode(currentCode++);
        lastSendTime = millis();
        
        if (currentCode >= 100) {
            currentCode = 0;
            currentRegion++;
            if (currentRegion >= 4) {
                stopBlast();
                MenuManager::show();
                return;
            }
        }
        
        // Calculate and show progress
        int totalCodes = 100 * 4;
        int currentTotal = (currentRegion * 100) + currentCode;
        int progressPercent = (currentTotal * 100) / totalCodes;
        
        char progressStr[32];
        snprintf(progressStr, sizeof(progressStr), "                %d%% [%d:%d]", 
                progressPercent, currentRegion, currentCode);
        
        RoverViewManager::showNotification("IR", progressStr, "BLAST", 500);
    }
}

void IRManager::updateLEDAnimation() {
    if (!blasting) return;
    
    FastLED.clear();
    
    if (animationDirection) {
        // Moving outward from center
        LEDManager::setLED(4, CRGB::Red);  // Always show center
        if (currentLEDPosition < 4) {
            LEDManager::setLED(4 - currentLEDPosition, CRGB::Red);
            LEDManager::setLED(4 + currentLEDPosition, CRGB::Red);
        }
        
        currentLEDPosition++;
        if (currentLEDPosition >= 4) {
            animationDirection = false;
            currentLEDPosition = 4;
            SoundFxManager::playTone(1000, 200);
        }
    } else {
        // Moving inward to center
        LEDManager::setLED(4, CRGB::Red);  // Always show center
        if (currentLEDPosition > 0) {
            LEDManager::setLED(4 - currentLEDPosition, CRGB::Red);
            LEDManager::setLED(4 + currentLEDPosition, CRGB::Red);
        }
        
        currentLEDPosition--;
        if (currentLEDPosition == 0) {
            animationDirection = true;
        }
    }
    
    LEDManager::showLEDs();
}

void IRManager::sendCode(uint16_t code) {
    // Send different protocols based on region, following TV-B-Gone approach
    switch(currentRegion) {
        case 0:  // Sony (SIRC) - Most common for Bravia
            digitalWrite(BOARD_IR_EN, HIGH);
            // Power codes: 21/0x15, 0xA90, 0x290
            // Input codes: 25/0x19, 0xA50, 0x250
            // Vol codes: 18/0x12 (up), 19/0x13 (down), 20/0x14 (mute)
            if (code < 20) {
                irsend.sendSony(code + 0x10, 12);  // Basic commands (0x10-0x20)
            } else if (code < 40) {
                irsend.sendSony(code + 0xA80, 12); // Extended set A
            } else if (code < 60) {
                irsend.sendSony(code + 0x240, 12); // Extended set B
            } else {
                irsend.sendSony(code, 15);         // 15-bit codes
            }
            digitalWrite(BOARD_IR_EN, LOW);
            break;
            
        case 1:  // NEC - Common for many TVs
            digitalWrite(BOARD_IR_EN, HIGH);
            if (code < 50) {
                irsend.sendNEC(0x04FB0000UL + code); // Sony TV NEC variants
            } else {
                irsend.sendNEC(0x10000000UL + code); // Other NEC codes
            }
            digitalWrite(BOARD_IR_EN, LOW);
            break;
            
        case 2:  // RC5/RC6 - Common for European TVs
            digitalWrite(BOARD_IR_EN, HIGH);
            if (code % 2) {
                irsend.sendRC5(code, 12);
            } else {
                irsend.sendRC6(code, 20);
            }
            digitalWrite(BOARD_IR_EN, LOW);
            break;
            
        case 3:  // Samsung - Common for newer TVs
            digitalWrite(BOARD_IR_EN, HIGH);
            irsend.sendSAMSUNG(0xE0E0 + code);
            digitalWrite(BOARD_IR_EN, LOW);
            break;
    }
} 