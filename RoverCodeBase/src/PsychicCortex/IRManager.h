#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <string>
#include "../PrefrontalCortex/utilities.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../MotorCortex/PinDefinitions.h"

class IRManager {
public:
    static void init();
    static void startBlast();
    static void stopBlast();
    static void update();
    static bool isBlasting() { return blasting; }
    
private:
    static bool blasting;
    static unsigned long lastSendTime;
    static unsigned long lastLEDUpdate;
    static uint16_t currentCode;
    static uint8_t currentRegion;
    static uint8_t currentLEDPosition;
    static bool animationDirection;
    static const uint16_t SEND_DELAY_MS = 100;
    static const uint16_t LED_UPDATE_MS = 50;
    static IRsend irsend;
    
    static void sendCode(uint16_t code);
    static void setupIROutput();
    static void updateLEDAnimation();
};

#endif