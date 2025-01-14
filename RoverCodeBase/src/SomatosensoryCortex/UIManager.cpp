#include "UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../SomatosensoryCortex/MenuManager.h"

// Static member initialization
RotaryEncoder* UIManager::encoder = nullptr;
int UIManager::lastEncoderPosition = 0;
bool UIManager::rotaryPressed = false;
bool UIManager::sideButtonPressed = false;
unsigned long UIManager::lastDebounceTime = 0;

void UIManager::init() {
    encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
    pinMode(BOARD_USER_KEY, INPUT_PULLUP);
    pinMode(ENCODER_KEY, INPUT_PULLUP);
    lastEncoderPosition = encoder->getPosition();
    Serial.println("UIManager initialized");
}

void UIManager::update() {
    updateEncoder();
    updateSideButton();
}

void UIManager::handleRotaryPress() {
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(ENCODER_KEY);
    
    if (currentButtonState == LOW && lastButtonState == HIGH) {
        Serial.println("Rotary button pressed");
        
        if (RoverViewManager::hasActiveNotification()) {
            RoverViewManager::clearNotification();
        } else if (!MenuManager::isVisible()) {
            Serial.println("Opening menu");
            MenuManager::show();
        } else {
            MenuManager::handleRotaryPress();
        }
        PowerManager::updateLastActivityTime();
    }
    
    lastButtonState = currentButtonState;
}

void UIManager::updateEncoder() {
    encoder->tick();
    int newPos = encoder->getPosition();
    
    // Handle rotation
    if (newPos != lastEncoderPosition) {
        if (RoverViewManager::hasActiveNotification()) {
            RoverViewManager::clearNotification();
        } else {
            handleRotaryTurn(newPos > lastEncoderPosition ? 1 : -1);
            SoundFxManager::playRotaryTurnSound(newPos > lastEncoderPosition);
        }
        lastEncoderPosition = newPos;
        PowerManager::updateLastActivityTime();
    }
    
    // Handle button press
    handleRotaryPress();
}

void UIManager::handleRotaryTurn(int direction) {
    if (MenuManager::isVisible()) {
        MenuManager::handleRotaryTurn(direction);
    } else {
        // Control views instead of moods
        if (direction > 0) {
            RoverViewManager::nextView();
        } else {
            RoverViewManager::previousView();
        }
    }
}

void UIManager::updateSideButton() {
    static bool lastState = HIGH;
    bool currentState = digitalRead(BOARD_USER_KEY);
    
    if (currentState != lastState) {
        if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
            lastDebounceTime = millis();
            sideButtonPressed = (currentState == LOW);
            
            if (sideButtonPressed) {
                if (RoverViewManager::hasActiveNotification()) {
                    RoverViewManager::clearNotification();
                } else {
                    RoverBehaviorManager::handleSideButton();
                }
            }
            PowerManager::updateLastActivityTime();
        }
        lastState = currentState;
    }
    
    // Update ears based on button state
    bool shouldBeUp = (currentState == LOW);
    if (shouldBeUp != RoverManager::earsPerked) {
        RoverManager::earsPerked = shouldBeUp;
        if (shouldBeUp) {
            RoverManager::setEarsUp();
        } else {
            RoverManager::setEarsDown();
        }
        RoverViewManager::drawCurrentView();
    }
}