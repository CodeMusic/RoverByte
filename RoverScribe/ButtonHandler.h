#pragma once
#include <Arduino.h>

class ButtonHandler {
public:
    void init();
    void handle();
    bool wasPressed();
private:
    bool buttonPressed = false;
    static const int BUTTON_PIN = 14; // Use actual pin from T5S3 schematic
};