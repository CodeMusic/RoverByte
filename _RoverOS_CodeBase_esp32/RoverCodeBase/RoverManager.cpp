#include "RoverManager.h"
#include "utilities.h"
#include <time.h>
#include "ColorUtilities.h"

// Initialize static members
bool RoverManager::earsPerked = false;
int RoverManager::currentMood = 0;
int RoverManager::hoverOffset = 0;
bool RoverManager::movingDown = true;
unsigned long RoverManager::lastHoverUpdate = 0;
const char* RoverManager::moods[] = {"happy", "looking_left", "looking_right", "intense"};

extern bool isLowBrightness;

void RoverManager::drawRover(const char* mood, bool earsPerked, bool large, int x, int y) {
    String moodStr(mood);
    float scale = large ? 1.5 : 1.0;
    
    // Get current month for eye colors and day color
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    uint16_t leftEyeColor = monthColors[timeInfo->tm_mon][0];
    uint16_t rightEyeColor = monthColors[timeInfo->tm_mon][1];
    
    // Get day color from ColorUtilities
    CRGB dayColor = ColorUtilities::getDayColor(timeInfo->tm_wday + 1);
    uint16_t timeColor = ColorUtilities::convertToRGB565(dayColor);
    
    // Small rover always shows time, large rover never shows time
    if (!large) {  // If small rover
        // Convert to 12-hour format
        int hours = timeInfo->tm_hour % 12;
        if (hours == 0) hours = 12;  // Convert 0 to 12 for midnight
        
        char timeStr[6];
        sprintf(timeStr, "%2d:%02d", hours, timeInfo->tm_min);
        spr.setTextFont(7);
        spr.fillRect(x - 50, 25, 100, 40, TFT_BLACK);  // Black background for time
        spr.setTextColor(timeColor, TFT_BLACK);  // Use day color with black background
        spr.drawString(timeStr, x, 30);  // Moved time down to y=30
        y = 80;  // Moved small rover down to y=80
    } else {
        y = 40;  // Large rover position unchanged
    }
    
    int roverX = x - (50 * scale);
    int currentY = y + hoverOffset;
    
    // Draw Rover's body
    spr.fillRect(roverX, currentY, 100 * scale, 70 * scale, TFT_WHITE);
    
    // Draw ears
    if (earsPerked) {
        spr.fillTriangle(roverX + 10*scale, currentY - 25*scale, 
                        roverX + 25*scale, currentY, 
                        roverX + 40*scale, currentY - 25*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 60*scale, currentY - 25*scale, 
                        roverX + 75*scale, currentY, 
                        roverX + 90*scale, currentY - 25*scale, TFT_WHITE);
    } else {
        spr.fillTriangle(roverX + 10*scale, currentY - 10*scale, 
                        roverX + 25*scale, currentY + 5*scale, 
                        roverX + 40*scale, currentY - 10*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 60*scale, currentY - 10*scale, 
                        roverX + 75*scale, currentY + 5*scale, 
                        roverX + 90*scale, currentY - 10*scale, TFT_WHITE);
    }
    
    // Draw eye panel
    spr.fillRect(roverX + 15*scale, currentY + 5*scale, 70*scale, 30*scale, color1);
    
    drawEyes(moodStr, roverX, currentY, leftEyeColor, rightEyeColor, scale);
    drawNoseAndMouth(moodStr, roverX, currentY, scale);
}

void RoverManager::drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor, float scale) {
    // Draw white background circles for eyes
    spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 10*scale, TFT_WHITE);
    spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 10*scale, TFT_WHITE);
    
    if (mood == "sleeping") {
        spr.drawLine(roverX + 25*scale, currentY + 20*scale, 
                    roverX + 35*scale, currentY + 20*scale, TFT_BLACK);
        spr.drawLine(roverX + 65*scale, currentY + 20*scale, 
                    roverX + 75*scale, currentY + 20*scale, TFT_BLACK);
    } else if (mood == "looking_left") {
        spr.fillCircle(roverX + 25*scale, currentY + 20*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 65*scale, currentY + 20*scale, 5*scale, rightEyeColor);
    } else if (mood == "looking_right") {
        spr.fillCircle(roverX + 35*scale, currentY + 20*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 75*scale, currentY + 20*scale, 5*scale, rightEyeColor);
    } else if (mood == "intense") {
        spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 3*scale, leftEyeColor);
        spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 3*scale, rightEyeColor);
    } else {
        spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 5*scale, rightEyeColor);
    }
}

void RoverManager::drawNoseAndMouth(String mood, int roverX, int currentY, float scale) {
    // Draw triangular nose
    spr.fillTriangle(roverX + 45*scale, currentY + 35*scale, 
                    roverX + 40*scale, currentY + 45*scale, 
                    roverX + 50*scale, currentY + 45*scale, TFT_BLACK);
    
    // Draw mouth based on mood
    if (mood == "happy") {
        spr.drawArc(roverX + 50*scale, currentY + 55*scale, 15*scale, 10*scale, 270, 450, TFT_BLACK, TFT_BLACK);
    } else if (mood == "sad") {
        spr.drawArc(roverX + 50*scale, currentY + 70*scale, 20*scale, 15*scale, 180, 360, TFT_BLACK, TFT_BLACK);
    } else if (mood == "intense") {
        spr.drawLine(roverX + 35*scale, currentY + 60*scale, 
                    roverX + 65*scale, currentY + 60*scale, TFT_BLACK);
    } else if (mood == "sleeping") {
        spr.drawArc(roverX + 50*scale, currentY + 60*scale, 15*scale, 10*scale, 0, 180, TFT_BLACK, TFT_BLACK);
    } else {
        spr.drawLine(roverX + 50*scale, currentY + 45*scale, 
                    roverX + 50*scale, currentY + 55*scale, TFT_BLACK);
    }
}

void RoverManager::updateHoverAnimation() {
    if (millis() - lastHoverUpdate >= 100) {  // Update every 100ms
        if (movingDown) {
            hoverOffset++;
            if (hoverOffset >= 3) {  // Reduced from 5 to 3
                movingDown = false;
            }
        } else {
            hoverOffset--;
            if (hoverOffset <= -3) {  // Reduced from -5 to -3
                movingDown = true;
            }
        }
        lastHoverUpdate = millis();
    }
}

const char* RoverManager::getCurrentMood() {
    return moods[currentMood];
}

void RoverManager::nextMood() {
    currentMood = (currentMood + 1) % NUM_MOODS;
}

void RoverManager::previousMood() {
    currentMood = (currentMood - 1 + NUM_MOODS) % NUM_MOODS;
}