#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <RotaryEncoder.h>

class UIManager {
public:
    static void init();
    static void update();
    
    // Input states
    static bool isRotaryPressed();
    static bool isSideButtonPressed();
    static int getEncoderPosition();
    
private:
    static RotaryEncoder* encoder;
    static int lastEncoderPosition;
    static bool rotaryPressed;
    static bool sideButtonPressed;
    static unsigned long lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50;
    
    static void handleRotaryTurn(int direction);
    static void handleRotaryPress();
    static void handleSideButton(bool pressed);
    static void updateEncoder();
    static void updateSideButton();
};

#endif