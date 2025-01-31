#pragma once
#include <FastLED.h>
#include "TFT_eSPI.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"

namespace SomatosensoryCortex { class MenuManager; }  // Forward declaration

namespace VisualCortex 
{
        // Forward declare the sprite
        extern TFT_eSprite spr;

    // Use the Expression type from ProtoPerceptions
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using PC::RoverTypes::Expression;  // Changed from VisualTypes to RoverTypes

    class RoverManager 
    {

        public:

            static const char* expressionToMood(Expression exp);
            static bool showTime;  // Added here as it's related to rover display state

            static bool isIdleExpression(Expression exp) {
                return exp <= Expression::INTENSE;  // Only first 4 are idle expressions
            }

            static void setTemporaryExpression(PC::RoverTypes::Expression exp, int duration = 1000, uint16_t starColor = TFT_WHITE);
            static void showThinking() { setTemporaryExpression(PC::RoverTypes::Expression::LOOKING_UP); }
            static void showError() { setTemporaryExpression(PC::RoverTypes::Expression::LOOKING_DOWN, 1000); }
            static void showSuccess() { setTemporaryExpression(PC::RoverTypes::Expression::BIG_SMILE, 1000); }
            static void setShowTime(bool show);

            static bool earsPerked;
            static const char* getCurrentMood();
            static void nextMood();
            static void previousMood();
            static void updateHoverAnimation();
            static void drawRover(const char* mood, bool earsPerked = false, bool large = false, int x = 85, int y = 120);
            static void setRandomMood();
            static void setEarsPerked(bool up);

        private:
            static void drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale);
            static void drawNoseAndMouth(String mood, int roverX, int currentY, float scale);
            static Expression currentExpression;
            static Expression previousExpression;
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
            static void drawExpression(PC::RoverTypes::Expression exp);
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
}