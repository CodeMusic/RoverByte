#include "LEDManager.h"
#include <time.h>
#include "ColorUtilities.h"
#include "utilities.h"
#include "FastLED.h"
// Initialize static members
LEDManager::Mode LEDManager::currentMode = Mode::FULL_MODE;
CRGB LEDManager::leds[WS2812_NUM_LEDS];
uint8_t LEDManager::currentColorIndex = 0;
uint8_t LEDManager::currentPosition = 0;
uint8_t LEDManager::completedCycles = 0;
uint8_t LEDManager::activeTrails = 0;
unsigned long LEDManager::lastStepTime = 0;
uint8_t LEDManager::filledPositions = 0;
bool LEDManager::isLoading = false;

void LEDManager::init() {
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    FastLED.clear();
    FastLED.show();
    
    // Initialize timer animation state
    currentColorIndex = 0;
    currentPosition = 0;
    completedCycles = 0;
    activeTrails = 0;
    lastStepTime = 0;
}

void LEDManager::updateLEDs() {
    switch (currentMode) {
        case Mode::FULL_MODE:
            updateFullMode();
            break;
        case Mode::WEEK_MODE:
            updateWeekMode();
            break;
        case Mode::TIMER_MODE:
            updateTimerMode();
            break;
    }
}

void LEDManager::setMode(Mode newMode) {
    currentMode = newMode;
    FastLED.clear();
    updateLEDs();
}

void LEDManager::nextMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % 3);
    FastLED.clear();
    updateLEDs();
}

void LEDManager::updateFullMode() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Create a 3-state blink cycle (0, 1, 2) using integer division of seconds
    int blinkState = (timeInfo->tm_sec % 3);
    
    // LED 0: Current day color (1-7, where 1 is Sunday)
    CRGB dayColor = ColorUtilities::getDayColor(timeInfo->tm_wday + 1);
    leds[0] = dayColor;
    
    // Rest of the LEDs based on hour colors
    // LED 1: Week number (base 5)
    int weekNum = (timeInfo->tm_mday + 6) / 7;  // Calculate week of month (1-5)
    switch(weekNum) {
        case 1: leds[1] = CRGB::Red; break;
        case 2: leds[1] = CRGB::Orange; break;
        case 3: leds[1] = CRGB::Yellow; break;
        case 4: leds[1] = CRGB::Green; break;
        default: leds[1] = CRGB::Blue; break;  // 5th week if exists
    }
    
    // LED 2: Month (base 12)
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    if (monthColor1 == monthColor2) {
        leds[2] = monthColor1;  // Solid color if both are the same
    } else {
        switch(blinkState) {
            case 0: leds[2] = monthColor1; break;
            case 1: leds[2] = monthColor2; break;
            case 2: leds[2] = CRGB::Black; break;
        }
    }
    
    // LED 3: Hours (base 12)
    int hour12 = timeInfo->tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    CRGB hourColor1, hourColor2;
    ColorUtilities::getHourColors(hour12, hourColor1, hourColor2);
    if (hourColor1 == hourColor2) {
        leds[3] = hourColor1;  // Solid color if both are the same
    } else {
        switch(blinkState) {
            case 0: leds[3] = hourColor1; break;
            case 1: leds[3] = hourColor2; break;
            case 2: leds[3] = CRGB::Black; break;
        }
    }
    
    // LED 4-5: Minutes (base 8)
    int minutes = timeInfo->tm_min;
    int minTens = minutes / 8;
    int minOnes = minutes % 8;
    leds[4] = ColorUtilities::getBase8Color(minTens);
    leds[5] = ColorUtilities::getBase8Color(minOnes);
    
    // LED 6-7: Day of month (base 8)
    int day = timeInfo->tm_mday;
    int dayTens = day / 8;
    int dayOnes = day % 8;
    leds[6] = ColorUtilities::getBase8Color(dayOnes);
    leds[7] = ColorUtilities::getBase8Color(dayTens);
    
    FastLED.show();
}

void LEDManager::updateWeekMode() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // LED 0: Month color with alternating pattern
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    
    // Create a 3-state blink cycle (0, 1, 2) using integer division of seconds
    int blinkState = (timeInfo->tm_sec % 3);
    
    if (monthColor1 == monthColor2) {
        // Single color month - blink between color and off
        if (timeInfo->tm_sec % 2 == 0) {
            monthColor1.nscale8(MONTH_DIM);
            leds[0] = monthColor1;
        } else {
            leds[0] = CRGB::Black;
        }
    } else {
        // Two-color month - cycle between colors and off
        switch(blinkState) {
            case 0:
                monthColor1.nscale8(MONTH_DIM);
                leds[0] = monthColor1;
                break;
            case 1:
                monthColor2.nscale8(MONTH_DIM);
                leds[0] = monthColor2;
                break;
            case 2:
                leds[0] = CRGB::Black;
                break;
        }
    }
    
    // Days of week
    for (int i = 1; i <= 7; i++) {
        CRGB dayColor = ColorUtilities::getDayColor(i);
        
        if (i - 1 < timeInfo->tm_wday) {
            leds[i] = CRGB::Black;  // Past days off
        } else if (i - 1 == timeInfo->tm_wday) {
            // Current day blinks at 72% brightness
            if (timeInfo->tm_sec % 2 == 0) {
                dayColor.nscale8(184);  // 72% brightness
                leds[i] = dayColor;
            } else {
                leds[i] = CRGB::Black;
            }
        } else {
            dayColor.nscale8(77);  // Future days dimmed to 30%
            leds[i] = dayColor;
        }
    }
    
    FastLED.show();
}

void LEDManager::updateTimerMode() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastStepTime >= STEP_DELAY) {
        lastStepTime = currentTime;
        
        // Clear all LEDs first
        FastLED.clear();
        
        // Draw completed cycles (white LEDs)
        for (int i = 0; i < completedCycles && i < WS2812_NUM_LEDS; i++) {
            // Blink white LED if it's in the current animation path
            if (i >= filledPositions && i < WS2812_NUM_LEDS) {
                if ((currentTime / 100) % 2 == 0) {  // Fast blink every 100ms
                    leds[i] = CRGB::White;
                }
            } else {
                leds[i] = CRGB::White;
            }
        }
        
        // If we haven't completed all cycles
        if (completedCycles < WS2812_NUM_LEDS) {
            CRGB currentColor = getRainbowColor(currentColorIndex);
            
            // Fill background with current color for all filled positions
            for (int i = filledPositions; i < WS2812_NUM_LEDS; i++) {
                if (i >= completedCycles) {  // Don't overwrite white LEDs
                    leds[i] = currentColor;
                }
            }
            
            // Update position counter
            if (currentPosition >= STEP_DELAY * 8) {  // Slow down the filling
                if (filledPositions > 0) {
                    filledPositions--;
                } else {
                    // When we've filled all positions with current color
                    if (currentColorIndex == NUM_RAINBOW_COLORS - 1) {  // If we're at violet
                        completedCycles++;  // Add a white LED
                        currentColorIndex = 0;  // Start over with red
                    } else {
                        currentColorIndex++;  // Move to next color
                    }
                    filledPositions = WS2812_NUM_LEDS - 1;  // Start filling from right again
                }
                currentPosition = 0;
            }
            currentPosition++;
            
            // Add blinking effect for the current filling position
            if ((millis() / 100) % 2 == 0) {  // Blink every 100ms
                leds[filledPositions] = CRGB::White;
            }
        }
        
        FastLED.show();
    }
}

CRGB LEDManager::getRainbowColor(uint8_t index) {
    static const CRGB rainbowColors[] = {
        CRGB::Red,
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Green,
        CRGB::Blue,
        CRGB(75, 0, 130),   // Indigo
        CRGB(148, 0, 211)   // Violet
    };
    return rainbowColors[index % NUM_RAINBOW_COLORS];
}

void LEDManager::startLoadingAnimation() {
    currentColorIndex = 0;
    currentPosition = 0;
    filledPositions = 0;
    completedCycles = 0;
    lastStepTime = 0;
    isLoading = true;
    FastLED.clear();
    FastLED.show();
}

void LEDManager::updateLoadingAnimation() {
    if (!isLoading) return;
    
    // No delay needed since it's a static display
    
    // Clear all LEDs first
    FastLED.clear();
    
    // Set Canadian flag pattern
    leds[0] = CRGB::Red;  // Left red stripe
    leds[4] = CRGB::Red;  // Right red stripe
    
    // White center
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    leds[3] = CRGB::White;
    
    FastLED.show();
}

bool LEDManager::isLoadingComplete() {
    return completedCycles >= WS2812_NUM_LEDS;
}

void LEDManager::stopLoadingAnimation() {
    isLoading = false;
    setMode(Mode::FULL_MODE);
} 