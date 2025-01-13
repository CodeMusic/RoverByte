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
    pinMode(ENCODER_KEY, INPUT_PULLUP);
    lastEncoderPosition = encoder->getPosition();
    Serial.println("UIManager initialized");
}

void UIManager::update() {
    updateEncoder();
    updateSideButton();
}

void UIManager::handleRotaryPress() {
    if (RoverViewManager::hasActiveNotification()) {
        RoverViewManager::clearNotification();
        return;
    }

    // Play button press sound
    SoundFxManager::playRotaryPressSound(0);
    
    // Toggle between time and LED modes
    if (!RoverManager::showTime) {
        RoverManager::showTime = true;
        LEDManager::setMode(Mode::WEEK_MODE);
    } else {
        LEDManager::nextMode();
    }
    
    PowerManager::updateLastActivityTime();
    RoverViewManager::drawCurrentView();
}

void UIManager::updateEncoder() {
    encoder->tick();
    int newPos = encoder->getPosition();
    
    // Check for button press
    static bool lastButtonState = HIGH;
    bool currentButtonState = digitalRead(ENCODER_KEY);
    
    if (currentButtonState != lastButtonState) {
        Serial.println("Button state changed");
        if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
            lastDebounceTime = millis();
            if (currentButtonState == LOW) {  // Button pressed
                Serial.println("Button pressed");
                handleRotaryPress();
            }
        }
        lastButtonState = currentButtonState;
    }
    
    // Handle rotation
    if (newPos != lastEncoderPosition) {
        Serial.println("Encoder position changed");
        if (RoverViewManager::hasActiveNotification()) {
            RoverViewManager::clearNotification();
        } else {
            handleRotaryTurn(newPos > lastEncoderPosition ? 1 : -1);
            SoundFxManager::playRotaryTurnSound(newPos > lastEncoderPosition);
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
                    RoverManager::earsPerked = !RoverManager::earsPerked;
                    SoundFxManager::playSideButtonSound(RoverManager::earsPerked);
                    RoverViewManager::drawCurrentView();
                }
            }
            PowerManager::updateLastActivityTime();
        }
        lastState = currentState;
    }
}