#pragma once
#include <FastLED.h>
#include "TFT_eSPI.h"

// Forward declare the sprite
extern TFT_eSprite spr;

class RoverManager {
public:
    static bool earsPerked;
    static const char* getCurrentMood();
    static void nextMood();
    static void previousMood();
    static void updateHoverAnimation();
    static void drawRover(const char* mood, bool earsPerked = false, bool large = false, int x = 85, int y = 120);

private:
    static void drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale);
    static void drawNoseAndMouth(String mood, int roverX, int currentY, float scale);
    
    // Static member variables
    static int currentMood;
    static int hoverOffset;
    static bool movingDown;
    static unsigned long lastHoverUpdate;
    static const char* moods[];
    static const int NUM_MOODS = 6;
    
    // Color definitions
    static const uint16_t monthColors[12][2];
    static const uint16_t color1;
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
