#include <time.h>
#include "ColorUtilities.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "LEDManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "DisplayConfig.h"
#include "../MotorCortex/PinDefinitions.h"

// Forward declarations
extern TFT_eSprite spr;

// Initialize static members
bool RoverManager::earsPerked = false;
int RoverManager::currentMood = 0;
int RoverManager::hoverOffset = 0;
bool RoverManager::movingDown = true;
unsigned long RoverManager::lastHoverUpdate = 0;
unsigned long RoverManager::expressionStartTime = 0;
int RoverManager::expressionDuration = 0;
uint16_t RoverManager::starColor = TFT_WHITE;
const char* RoverManager::moods[] = {"happy", "looking_left", "looking_right", "intense"};
bool RoverManager::showTime = false;

extern bool isLowBrightness;

RoverManager::Expression RoverManager::currentExpression = RoverManager::HAPPY;
RoverManager::Expression RoverManager::previousExpression = RoverManager::HAPPY;

void RoverManager::drawRover(const char* mood, bool earsPerked, bool large, int x, int y) {
    // Use currentExpression instead of mood parameter if it's set
    const char* actualMood = currentExpression != previousExpression ? 
                            expressionToMood(currentExpression) : 
                            mood;
    
    String moodStr(actualMood);
    float scale = large ? 1.5 : 1.0;
    
    // Get current time and colors
    time_t now = time(nullptr);
    struct tm timeInfo;
    localtime_r(&now, &timeInfo);  // Remove timezone adjustment since it's handled by configTime
    
    // Get eye colors from current month
    uint16_t leftEyeColor = monthColors[timeInfo.tm_mon][0];
    uint16_t rightEyeColor = monthColors[timeInfo.tm_mon][1];
    
    // Small rover always shows time, large rover never shows time
    if (!large && showTime) {
        // Convert to 12-hour format
        int hours = timeInfo.tm_hour % 12;
        if (hours == 0) hours = 12;
        
        char timeStr[6];
        sprintf(timeStr, "%2d:%02d", hours, timeInfo.tm_min);
        spr.setTextFont(7);
        
        // Center time and rover using TFT_WIDTH
        int16_t timeWidth = spr.textWidth(timeStr);
        int centerX = TFT_WIDTH / 2;  // Screen center
        x = centerX - (100 * scale) / 2;  // Center rover (100 is rover width)
        
        spr.fillRect(centerX - (timeWidth/2) - 5, 25, timeWidth + 10, 40, TFT_BLACK);
        
        // Get day color for time display
        CRGB dayColor = ColorUtilities::getDayColor(timeInfo.tm_wday + 1);
        uint16_t timeColor = ColorUtilities::convertToRGB565(dayColor);
        spr.setTextColor(timeColor, TFT_BLACK);
        spr.drawString(timeStr, centerX, 30);
        y = 80;
    } else {
        y = 40;
    }
    
    // Draw rover starting from x position
    int roverX = x;  // Remove the offset calculation
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
    
    if (mood == "excited") {
        // Left star eye
        for (int i = -5; i <= 5; i++) {
            spr.drawLine(roverX + (30-5)*scale, currentY + 20*scale, 
                        roverX + (30+5)*scale, currentY + 20*scale, leftEyeColor);
            spr.drawLine(roverX + 30*scale, currentY + (20-5)*scale, 
                        roverX + 30*scale, currentY + (20+5)*scale, leftEyeColor);
            // Diagonal lines for star points
            spr.drawLine(roverX + (30-3)*scale, currentY + (20-3)*scale,
                        roverX + (30+3)*scale, currentY + (20+3)*scale, leftEyeColor);
            spr.drawLine(roverX + (30-3)*scale, currentY + (20+3)*scale,
                        roverX + (30+3)*scale, currentY + (20-3)*scale, leftEyeColor);
        }
        
        // Right star eye (same pattern, different position)
        for (int i = -5; i <= 5; i++) {
            spr.drawLine(roverX + (70-5)*scale, currentY + 20*scale, 
                        roverX + (70+5)*scale, currentY + 20*scale, rightEyeColor);
            spr.drawLine(roverX + 70*scale, currentY + (20-5)*scale, 
                        roverX + 70*scale, currentY + (20+5)*scale, rightEyeColor);
            // Diagonal lines for star points
            spr.drawLine(roverX + (70-3)*scale, currentY + (20-3)*scale,
                        roverX + (70+3)*scale, currentY + (20+3)*scale, rightEyeColor);
            spr.drawLine(roverX + (70-3)*scale, currentY + (20+3)*scale,
                        roverX + (70+3)*scale, currentY + (20-3)*scale, rightEyeColor);
        }
    } else if (mood == "looking_up") {
        spr.fillCircle(roverX + 30*scale, currentY + 15*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 70*scale, currentY + 15*scale, 5*scale, rightEyeColor);
    } else if (mood == "looking_down") {
        spr.fillCircle(roverX + 30*scale, currentY + 25*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 70*scale, currentY + 25*scale, 5*scale, rightEyeColor);
    } else if (mood == "big_smile") {
        // Normal eye position with bigger smile
        spr.fillCircle(roverX + 30*scale, currentY + 20*scale, 5*scale, leftEyeColor);
        spr.fillCircle(roverX + 70*scale, currentY + 20*scale, 5*scale, rightEyeColor);
    } else if (mood == "sleeping") {
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
    } else if (mood == "big_smile") {
        // Wider, bigger smile
        spr.drawArc(roverX + 50*scale, currentY + 55*scale, 20*scale, 15*scale, 270, 450, TFT_BLACK, TFT_BLACK);
    } else {
        spr.drawLine(roverX + 50*scale, currentY + 45*scale, 
                    roverX + 50*scale, currentY + 55*scale, TFT_BLACK);
    }
}

void RoverManager::updateHoverAnimation() {
    // Only update hover animation when device is awake
    if (PowerManager::getCurrentSleepState() != PowerManager::AWAKE) return;
    
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

void RoverManager::setRandomMood() {
    currentMood = random(0, NUM_MOODS);
    drawSprite();  // Now we can call it directly
}

// New function to handle temporary expressions
void RoverManager::setTemporaryExpression(Expression exp, int duration, uint16_t color) {
    currentExpression = exp;
    starColor = color;  // Store the star color
    expressionStartTime = millis();
    expressionDuration = duration;
    
    // Update display with new expression
    drawExpression(currentExpression);
}

const char* RoverManager::expressionToMood(Expression exp) {
    switch(exp) {
        case HAPPY: return "happy";
        case LOOKING_UP: return "looking_up";
        case LOOKING_DOWN: return "looking_down";
        case LOOKING_LEFT: return "looking_left";
        case LOOKING_RIGHT: return "looking_right";
        case INTENSE: return "intense";
        case BIG_SMILE: return "big_smile";
        case EXCITED: return "excited";
        default: return "happy";
    }
}

void RoverManager::setEarsUp() {
    earsPerked = true;
    setTemporaryExpression(HAPPY);  // Show happy expression when ears go up
    drawRover(moods[currentMood], true);  // Redraw with ears up
}

void RoverManager::setEarsDown() {
    earsPerked = false;
    setTemporaryExpression(LOOKING_DOWN, 1000);  // Show looking down expression briefly
    drawRover(moods[currentMood], false);  // Redraw with ears down
}

void RoverManager::drawExpression(Expression exp) {
    const char* mood = expressionToMood(exp);
    drawRover(mood, earsPerked);
}

void RoverManager::setEarsState(bool up) {
    earsPerked = up;
    setTemporaryExpression(HAPPY);
    drawRover(moods[currentMood], up);
}