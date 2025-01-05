#ifndef ROVER_MANAGER_H
#define ROVER_MANAGER_H

#include "TFT_eSPI.h"
#include <time.h>

// Forward declarations
extern TFT_eSprite spr;
extern const uint16_t monthColors[][2];

// Screen layout
const int SCREEN_CENTER_X = 85;

// Hover animation constants and variables
const int MAX_OFFSET = 5;  // Maximum pixels to move up/down
int roverYOffset = 0;
bool movingDown = true;
unsigned long lastHoverUpdate = 0;
const int HOVER_SPEED = 90;  // Update every 90ms

// Rover mood management
const char* moods[] = {"happy", "looking_left", "looking_right", "intense"};
int currentMood = 0;
int numMoods = 4;
bool earsPerked = false;

// Function to update hover animation
void updateHoverAnimation() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastHoverUpdate >= HOVER_SPEED) {
        lastHoverUpdate = currentMillis;
        
        if (movingDown) {
            roverYOffset++;
            if (roverYOffset >= MAX_OFFSET) {
                movingDown = false;
            }
        } else {
            roverYOffset--;
            if (roverYOffset <= -MAX_OFFSET) {
                movingDown = true;
            }
        }
    }
}


void drawRover(String mood, bool earsPerked) {
    // Get current month for eye colors
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    uint16_t leftEyeColor = monthColors[timeInfo->tm_mon][0];
    uint16_t rightEyeColor = monthColors[timeInfo->tm_mon][1];
    
    int roverX = SCREEN_CENTER_X - 50;  // Center point for Rover
    int baseY = 80;  // Base Y position
    
    // Apply hover animation offset
    int currentY = baseY + roverYOffset;
    
    // Draw Rover's body (white)
    spr.fillRect(roverX, currentY, 100, 70, TFT_WHITE);
    
    // Draw ears (triangles) with adjusted Y position
    if (earsPerked) {
        // Perked ears - higher position and more upright angle
        spr.fillTriangle(roverX + 10, currentY - 25, roverX + 25, currentY, roverX + 40, currentY - 25, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, currentY - 25, roverX + 75, currentY, roverX + 90, currentY - 25, TFT_WHITE);  // Right ear
    } else {
        // Normal ears - lower position and more relaxed angle
        spr.fillTriangle(roverX + 10, currentY - 10, roverX + 25, currentY + 5, roverX + 40, currentY - 10, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, currentY - 10, roverX + 75, currentY + 5, roverX + 90, currentY - 10, TFT_WHITE);  // Right ear
    }
    
    // Draw eye panel (silver rectangle)
    spr.fillRect(roverX + 15, currentY + 5, 70, 30, 0xC618);
    
    if (mood == "cool") {
        // Draw sunglasses with adjusted Y
        spr.fillRect(roverX + 20, currentY + 15, 60, 15, TFT_BLACK);  // Sunglasses bar
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_BLACK);   // Left lens
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_BLACK);   // Right lens
        spr.drawLine(roverX + 25, currentY + 15, roverX + 30, currentY + 15, TFT_WHITE);
        spr.drawLine(roverX + 65, currentY + 15, roverX + 70, currentY + 15, TFT_WHITE);
    } else {
        // Normal eyes with adjusted Y
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_WHITE);  // Left eye white
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_WHITE);  // Right eye white
        
        if (mood == "sleeping") {
            // Closed eyes
            spr.drawLine(roverX + 25, currentY + 20, roverX + 35, currentY + 20, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 20, roverX + 75, currentY + 20, TFT_BLACK);
        } else if (mood == "looking_left") {
            // Eyes looking left
            spr.fillCircle(roverX + 25, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 65, currentY + 20, 5, rightEyeColor);
        } else if (mood == "looking_right") {
            // Eyes looking right
            spr.fillCircle(roverX + 35, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 75, currentY + 20, 5, rightEyeColor);
        } else if (mood == "intense") {
            // Intense eyes (smaller)
            spr.fillCircle(roverX + 30, currentY + 20, 3, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 3, rightEyeColor);
        } else if (mood == "broken") {
            // X eyes
            spr.drawLine(roverX + 25, currentY + 15, roverX + 35, currentY + 25, TFT_BLACK);
            spr.drawLine(roverX + 25, currentY + 25, roverX + 35, currentY + 15, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 15, roverX + 75, currentY + 25, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 25, roverX + 75, currentY + 15, TFT_BLACK);
        } else {
            // Default eyes (happy/neutral)
            spr.fillCircle(roverX + 30, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 5, rightEyeColor);
        }
    }
    
    // Draw nose with adjusted Y
    spr.fillTriangle(roverX + 45, currentY + 35, roverX + 40, currentY + 45, roverX + 50, currentY + 45, TFT_BLACK);
    
    // Draw mouth with adjusted Y
    spr.drawLine(roverX + 50, currentY + 45, roverX + 50, currentY + 55, TFT_BLACK);
    
    if (mood == "happy") {
        spr.drawArc(roverX + 50, currentY + 55, 15, 10, 270, 450, TFT_BLACK, TFT_BLACK);
    } else if (mood == "sad") {
        spr.drawArc(roverX + 50, currentY + 70, 20, 15, 180, 360, TFT_BLACK, TFT_BLACK);
    } else if (mood == "intense") {
        spr.drawLine(roverX + 35, currentY + 60, roverX + 65, currentY + 60, TFT_BLACK);
    } else if (mood == "sleeping") {
        spr.drawArc(roverX + 50, currentY + 60, 15, 10, 0, 180, TFT_BLACK, TFT_BLACK);
    }
}

void nextMood() {
    currentMood = (currentMood + 1) % numMoods;
}

void previousMood() {
    currentMood = (currentMood - 1 + numMoods) % numMoods;
}

static unsigned long lastMoodChange = 0;
const unsigned long MOOD_CHANGE_INTERVAL = 30000;  // Change mood every 30 seconds

// Function to calculate and update the Rover's mood
void calculateMood() {

    unsigned long currentMillis = millis();
    
    // Update mood randomly
    if (currentMillis - lastMoodChange >= MOOD_CHANGE_INTERVAL) {
        lastMoodChange = currentMillis;
        
        // Generate random mood different from current
        int newMood;
        do {
            newMood = random(numMoods);
        } while (newMood == currentMood);
        
        currentMood = newMood;
        Serial.printf("New mood: %s\n", moods[currentMood]);
    }
}

#endif // ROVER_MANAGER_H