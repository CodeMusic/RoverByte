 #include "RoverManager.h"

// Initialize static members
int RoverManager::currentMood = 0;
int RoverManager::roverYOffset = 0;
bool RoverManager::movingDown = true;
bool RoverManager::earsPerked = false;
unsigned long RoverManager::lastHoverUpdate = 0;
const char* RoverManager::moods[] = {"happy", "looking_left", "looking_right", "intense"};

void RoverManager::init() {
    LOG_PROD("Initializing RoverManager");
    currentMood = 0;
    roverYOffset = 0;
    movingDown = true;
    lastHoverUpdate = 0;
    LOG_DEBUG("RoverManager initialized with default mood: %s", moods[currentMood]);
}

void RoverManager::nextMood() {
    currentMood = (currentMood + 1) % numMoods;
    LOG_DEBUG("Mood changed to: %s", moods[currentMood]);
}

void RoverManager::previousMood() {
    currentMood = (currentMood - 1 + numMoods) % numMoods;
    LOG_DEBUG("Mood changed to: %s", moods[currentMood]);
}

const char* RoverManager::getCurrentMood() {
    return moods[currentMood];
}

void RoverManager::updateHoverAnimation() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastHoverUpdate >= HOVER_SPEED) {
        lastHoverUpdate = currentMillis;
        
        LOG_SCOPE("Updating hover animation: offset=%d, moving%s", 
                  roverYOffset, movingDown ? "Down" : "Up");
        
        if (movingDown) {
            roverYOffset++;
            if (roverYOffset >= MAX_OFFSET) {
                movingDown = false;
                LOG_DEBUG("Hover animation reached bottom, reversing");
            }
        } else {
            roverYOffset--;
            if (roverYOffset <= -MAX_OFFSET) {
                movingDown = true;
                LOG_DEBUG("Hover animation reached top, reversing");
            }
        }
    }
}

void RoverManager::drawRover(String mood, bool earsPerked) {
    LOG_SCOPE("Drawing Rover with mood: %s, ears: %s", 
              mood.c_str(), earsPerked ? "perked" : "down");

    // Get current month for eye colors
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    CRGB leftEye = MONTH_COLORS[timeInfo->tm_mon][0];
    CRGB rightEye = MONTH_COLORS[timeInfo->tm_mon][1];
    
    // Convert CRGB to uint16_t for TFT display
    uint16_t leftEyeColor = ((leftEye.r & 0xF8) << 8) | ((leftEye.g & 0xFC) << 3) | (leftEye.b >> 3);
    uint16_t rightEyeColor = ((rightEye.r & 0xF8) << 8) | ((rightEye.g & 0xFC) << 3) | (rightEye.b >> 3);
    
    int roverX = SCREEN_CENTER_X - 50;
    int baseY = 80;
    int currentY = baseY + roverYOffset;
    
    // Draw Rover's body
    spr.fillRect(roverX, currentY, 100, 70, TFT_WHITE);
    
    // Draw ears
    if (earsPerked) {
        LOG_SCOPE("Drawing perked ears");
        spr.fillTriangle(roverX + 10, currentY - 25, roverX + 25, currentY, roverX + 40, currentY - 25, TFT_WHITE);
        spr.fillTriangle(roverX + 60, currentY - 25, roverX + 75, currentY, roverX + 90, currentY - 25, TFT_WHITE);
    } else {
        LOG_SCOPE("Drawing relaxed ears");
        spr.fillTriangle(roverX + 10, currentY - 10, roverX + 25, currentY + 5, roverX + 40, currentY - 10, TFT_WHITE);
        spr.fillTriangle(roverX + 60, currentY - 10, roverX + 75, currentY + 5, roverX + 90, currentY - 10, TFT_WHITE);
    }
    
    // Draw eye panel
    spr.fillRect(roverX + 15, currentY + 5, 70, 30, 0xC618);
    
    // Draw eyes and face
    drawEyes(mood, roverX, currentY, leftEyeColor, rightEyeColor);
    drawNoseAndMouth(mood, roverX, currentY);
}

void RoverManager::drawEyes(String mood, int roverX, int currentY, uint16_t leftEyeColor, uint16_t rightEyeColor) {
    LOG_SCOPE("Drawing eyes for mood: %s", mood.c_str());
    
    if (mood == "cool") {
        spr.fillRect(roverX + 20, currentY + 15, 60, 15, TFT_BLACK);
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_BLACK);
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_BLACK);
        spr.drawLine(roverX + 25, currentY + 15, roverX + 30, currentY + 15, TFT_WHITE);
        spr.drawLine(roverX + 65, currentY + 15, roverX + 70, currentY + 15, TFT_WHITE);
    } else {
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_WHITE);
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_WHITE);
        
        if (mood == "sleeping") {
            spr.drawLine(roverX + 25, currentY + 20, roverX + 35, currentY + 20, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 20, roverX + 75, currentY + 20, TFT_BLACK);
        } else if (mood == "looking_left") {
            spr.fillCircle(roverX + 25, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 65, currentY + 20, 5, rightEyeColor);
        } else if (mood == "looking_right") {
            spr.fillCircle(roverX + 35, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 75, currentY + 20, 5, rightEyeColor);
        } else if (mood == "intense") {
            spr.fillCircle(roverX + 30, currentY + 20, 3, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 3, rightEyeColor);
        } else {
            spr.fillCircle(roverX + 30, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 5, rightEyeColor);
        }
    }
}

void RoverManager::drawNoseAndMouth(String mood, int roverX, int currentY) {
    LOG_SCOPE("Drawing nose and mouth for mood: %s", mood.c_str());
    
    // Draw nose
    spr.fillTriangle(roverX + 45, currentY + 35, roverX + 40, currentY + 45, 
                    roverX + 50, currentY + 45, TFT_BLACK);
    
    // Draw mouth based on mood
    if (mood == "happy") {
        spr.drawArc(roverX + 50, currentY + 55, 15, 10, 270, 450, TFT_BLACK, TFT_BLACK);
    } else if (mood == "sad") {
        spr.drawArc(roverX + 50, currentY + 70, 20, 15, 180, 360, TFT_BLACK, TFT_BLACK);
    } else if (mood == "intense") {
        spr.drawLine(roverX + 35, currentY + 60, roverX + 65, currentY + 60, TFT_BLACK);
    } else if (mood == "sleeping") {
        spr.drawArc(roverX + 50, currentY + 60, 15, 10, 0, 180, TFT_BLACK, TFT_BLACK);
    } else {
        spr.drawLine(roverX + 50, currentY + 45, roverX + 50, currentY + 55, TFT_BLACK);
    }
}