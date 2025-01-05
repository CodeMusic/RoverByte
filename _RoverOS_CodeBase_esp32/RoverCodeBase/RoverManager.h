#ifndef ROVER_MANAGER_H
#define ROVER_MANAGER_H

#include "TFT_eSPI.h"
#include "utilities.h"
#include <time.h>
#include <FastLED.h>

// Forward declarations
extern TFT_eSprite spr;
extern const CRGB MONTH_COLORS[][2];

class RoverManager {
public:
    static bool earsPerked;
    
    // Static initialization
    static void init();

    // Mood management
    static void nextMood();
    static void previousMood();
    static const char* getCurrentMood();

    // Animation and drawing
    static void updateHoverAnimation();
    static void drawRover(String mood = "happy", bool earsPerked = false);

private:
    // Constants
    
    static const int MAX_OFFSET = 5;
    static const int HOVER_SPEED = 90;
    static const int numMoods = 4;
    
    // Static member variables
    static int currentMood;
    static int roverYOffset;
    static bool movingDown;
    static unsigned long lastHoverUpdate;
    static const char* moods[];

    // Helper methods
    static void drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor);
    static void drawNoseAndMouth(String mood, int roverX, int currentY);
};

#endif // ROVER_MANAGER_H