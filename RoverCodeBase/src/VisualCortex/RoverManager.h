#pragma once
#include <FastLED.h>
#include "TFT_eSPI.h"

// Forward declare the sprite
extern TFT_eSprite spr;

class RoverManager {
public:
    // Enum must be declared before it's used as a type
    enum Expression {
        HAPPY,
        LOOKING_LEFT,
        LOOKING_RIGHT,
        INTENSE,
        LOOKING_UP,      // New - not idle
        LOOKING_DOWN,    // New - not idle
        BIG_SMILE,      // New - not idle
        EXCITED,         // New expression with star eyes
        NUM_EXPRESSIONS
    };

    static const char* expressionToMood(Expression exp);
    static bool showTime;  // Added here as it's related to rover display state

private:
    // Now we can use Expression as a type
    static Expression currentExpression;
    static Expression previousExpression;

public:
    static bool isIdleExpression(Expression exp) {
        return exp <= INTENSE;  // Only first 4 are idle expressions
    }

    static void setTemporaryExpression(Expression exp, int duration = 1000, uint16_t starColor = TFT_WHITE);
    static void showThinking() { setTemporaryExpression(LOOKING_UP); }
    static void showError() { setTemporaryExpression(LOOKING_DOWN, 1000); }
    static void showSuccess() { setTemporaryExpression(BIG_SMILE, 1000); }
    
    static bool earsPerked;
    static const char* getCurrentMood();
    static void nextMood();
    static void previousMood();
    static void updateHoverAnimation();
    static void drawRover(const char* mood, bool earsPerked = false, bool large = false, int x = 85, int y = 120);
    static void setRandomMood();
    static void setEarsDown();
    static void setEarsUp();
    static void setEarsState(bool up);

private:
    static void drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale);
    static void drawNoseAndMouth(String mood, int roverX, int currentY, float scale);
    
    // Static member variables
    static int currentMood;
    static int hoverOffset;
    static bool movingDown;
    static unsigned long lastHoverUpdate;
    static const char* moods[];
    static const int NUM_MOODS = 5;
    
    // Color definitions
    static const uint16_t monthColors[12][2];
    static const uint16_t color1;

    // Add these new members
    static unsigned long expressionStartTime;
    static int expressionDuration;
    static uint16_t starColor;
    static void drawExpression(Expression exp);
};

// Define the static const arrays outside the class
inline const uint16_t RoverManager::monthColors[12][2] = {
    {0xF800, 0xF800},  // January   - Red/Red
    {0xF800, 0xFD20},  // February  - Red/Orange
    {0xFD20, 0xFD20},  // March     - Orange/Orange
    {0xFD20, 0xFFE0},  // April     - Orange/Yellow
    {0xFFE0, 0xFFE0},  // May       - Yellow/Yellow
    {0x07E0, 0x07E0},  // June      - Green/Green
    {0x07E0, 0x001F},  // July      - Green/Blue
    {0x001F, 0x001F},  // August    - Blue/Blue
    {0x001F, 0x4810},  // September - Blue/Indigo
    {0x4810, 0x4810},  // October   - Indigo/Indigo
    {0x4810, 0x780F},  // November  - Indigo/Violet
    {0x780F, 0x780F}   // December  - Violet/Violet
};
inline const uint16_t RoverManager::color1 = 0xC638;  // Silver color for eye plate
