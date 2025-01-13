#include "UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"

// Static member initialization
RotaryEncoder* UIManager::encoder = nullptr;
int UIManager::lastEncoderPosition = 0;
bool UIManager::rotaryPressed = false;
bool UIManager::sideButtonPressed = false;
unsigned long UIManager::lastDebounceTime = 0;

void UIManager::init() {
    encoder = new RotaryEncoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);
    pinMode(BOARD_USER_KEY, INPUT_PULLUP);
    lastEncoderPosition = encoder->getPosition();
}

void UIManager::update() {
    updateEncoder();
    updateSideButton();
}

void UIManager::handleRotaryPress() {
    // If we just woke from sleep, first press should show time
    if (PowerManager::getCurrentSleepState() == PowerManager::SleepState::AWAKE && 
        !PowerManager::getShowTime()) {
        PowerManager::setShowTime(true);
    } else {
        // Normal operation - toggle between time and LED modes
        if (!PowerManager::getShowTime()) {
            PowerManager::setShowTime(true);
        } else {
            LEDManager::nextMode();
            SoundFxManager::playRotaryPressSound(static_cast<int>(LEDManager::getMode()));
        }
    }
    PowerManager::updateLastActivityTime();
}

void UIManager::updateEncoder() {
    encoder->tick();
    int newPos = encoder->getPosition();
    
    if (newPos != lastEncoderPosition) {
        if (RoverViewManager::hasActiveNotification()) {
            RoverViewManager::clearNotification();
        } else {
            handleRotaryTurn(newPos > lastEncoderPosition ? 1 : -1);
        }
        lastEncoderPosition = newPos;
        PowerManager::updateLastActivityTime();
    }
}

void UIManager::handleRotaryTurn(int direction) {
    if (direction > 0) {
        RoverViewManager::handleInput(RoverViewManager::InputType::INPUT_RIGHT);
    } else {
        RoverViewManager::handleInput(RoverViewManager::InputType::INPUT_LEFT);
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
                    RoverManager::setEarsUp();
                    NFCManager::handleSideButtonPress();
                }
            } else {
                RoverManager::setEarsDown();
            }
            PowerManager::updateLastActivityTime();
        }
    }
    lastState = currentState;
}