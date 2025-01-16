#include "ButtonHandler.h"

void ButtonHandler::init() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    buttonPressed = false;
}

void ButtonHandler::handle() {
    static bool lastState = HIGH;
    bool currentState = digitalRead(BUTTON_PIN);
    
    if (lastState == HIGH && currentState == LOW) {
        buttonPressed = true;
    } else {
        buttonPressed = false;
    }
    
    lastState = currentState;
}

bool ButtonHandler::wasPressed() {
    bool pressed = buttonPressed;
    buttonPressed = false;  // Clear the flag after reading
    return pressed;
} 