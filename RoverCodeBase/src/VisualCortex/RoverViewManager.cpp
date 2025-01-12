#include "RoverViewManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "DisplayConfig.h"
#include "RoverManager.h"

// Add at the top with other includes
extern bool showTime;  // Declare the external variable

// Initialize static members
RoverViewManager::ViewType RoverViewManager::currentView = RoverViewManager::VIRTUES;
unsigned long RoverViewManager::lastStatusUpdate = 0;
int RoverViewManager::statusRotation = 0;

// Forward declare all drawing functions
void drawRootChakra(int x, int y, int size);
void drawSacralChakra(int x, int y, int size);
void drawSolarChakra(int x, int y, int size);
void drawHeartChakra(int x, int y, int size);
void drawThroatChakra(int x, int y, int size);
void drawThirdEyeChakra(int x, int y, int size);
void drawCrownChakra(int x, int y, int size);
void drawChastitySymbol(int x, int y, int size);
void drawTemperanceSymbol(int x, int y, int size);
void drawCharitySymbol(int x, int y, int size);
void drawDiligenceSymbol(int x, int y, int size);
void drawForgivenessSymbol(int x, int y, int size);
void drawKindnessSymbol(int x, int y, int size);
void drawHumilitySymbol(int x, int y, int size);
void drawBatteryCharging(int x, int y, int size);
void drawBattery(int x, int y, int size);

// Update array names to match header
const RoverViewManager::ChakraInfo RoverViewManager::CHAKRA_DATA[] = {
    {"Root Chakra", "Survival, Grounding, \nStability, Comfort, Safety", TFT_RED, drawRootChakra},
    {"Sacral Chakra", "Sensuality, Sexuality, \nPleasure, Creativity, Emotions", 0xFDA0, drawSacralChakra},
    {"Solar Plexus Chakra", "Strength, Ego, Power, \nSelf-esteem, Digestion", 0xFFE0, drawSolarChakra},
    {"Heart Chakra", "Love, Acceptance, Compassion, \nKindness, Peace", 0x07E0, drawHeartChakra},
    {"Throat Chakra", "Communication, Expression, \nHonesty, Purification", 0x001F, drawThroatChakra},
    {"Third Eye Chakra", "Intuition, Visualization, \nImagination, Clairvoyance", 0x180E, drawThirdEyeChakra},
    {"Crown Chakra", "Knowledge, Fulfillment, \nSpirituality, Reality", 0x780F, drawCrownChakra}
};

const RoverViewManager::VirtueInfo RoverViewManager::VIRTUE_DATA[] = {
    {"Chastity cures Lust", "Purity \nquells \nexcessive sexual appetites", TFT_RED, drawChastitySymbol},
    {"Temperance cures Gluttony", "Self-restraint \nquells \nover-indulgence", 0xFDA0, drawTemperanceSymbol},
    {"Charity cures Greed", "Giving \nquells \navarice", 0xFFE0, drawCharitySymbol},
    {"Diligence cures Sloth", "Integrity and effort \nquells \nlaziness", 0x07E0, drawDiligenceSymbol},
    {"Forgiveness cures Wrath", "Keep composure \nto quell \nanger", 0x001F, drawForgivenessSymbol},
    {"Kindness cures Envy", "Admiration \nquells \njealousy", 0x180E, drawKindnessSymbol},
    {"Humility cures Pride", "Humbleness \nquells \nvanity", 0x780F, drawHumilitySymbol}
};

void RoverViewManager::init() {
    LOG_PROD("Initializing RoverViewManager");
    
    // Create sprite with screen dimensions
    spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
    spr.fillSprite(TFT_BLACK);
    spr.pushSprite(0, 0);
    
    // Draw initial frame
    drawFrame();
    drawCurrentView();
}

void RoverViewManager::setCurrentView(ViewType view) {
    LOG_DEBUG("Changing view to: %d", view);
    currentView = view;
}

void RoverViewManager::nextView() {
    LOG_DEBUG("Changing from view %d to next view", currentView);
    currentView = static_cast<ViewType>((currentView + 1) % ViewType::NUM_VIEWS);
    LOG_DEBUG("New view is %d", currentView);
    drawCurrentView();
}

void RoverViewManager::previousView() {
    LOG_DEBUG("Changing from view %d to previous view", currentView);
    currentView = static_cast<ViewType>((currentView - 1 + ViewType::NUM_VIEWS) % ViewType::NUM_VIEWS);
    LOG_DEBUG("New view is %d", currentView);
    drawCurrentView();
}

void RoverViewManager::drawCurrentView() {
    static bool isRecovering = false;
    
    try {
        if (isRecovering) {
            spr.fillSprite(TFT_BLACK);
            spr.setTextFont(2);
            spr.setTextColor(TFT_WHITE, TFT_BLACK);
            spr.drawString("Display Error", SCREEN_CENTER_X, SCREEN_HEIGHT/2);
            spr.pushSprite(0, 0);
            delay(1000);
            isRecovering = false;
            return;
        }
        
        spr.fillSprite(TFT_BLACK);
        
        RoverManager::drawRover(
            RoverManager::getCurrentMood(),
            RoverManager::earsPerked,
            !showTime,
            10,
            showTime ? 50 : 80
        );
        
        drawFrame();
        drawStatusBar();
        
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.setTextDatum(MC_DATUM);
        
        switch(currentView) {
            case TODO_LIST:
                drawTodoList();
                break;
            case CHAKRAS:
                drawChakras();
                break;
            case VIRTUES:
                drawVirtues();
                break;
            case QUOTES:
                drawQuotes();
                break;
            case WEATHER:
                drawWeather();
                break;
            case STATS:
                drawStats();
                break;
            case NEXTMEAL:
                drawNextMeal();
                break;
            default:
                break;
        }
        
        spr.pushSprite(0, 0);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error in drawCurrentView: %s", e.what());
        isRecovering = true;
        drawCurrentView();  // Try again with error recovery
    }
}


// Add this new function
void RoverViewManager::drawLoadingScreen() {
    spr.fillSprite(TFT_BLACK);
    
    // Center bone vertically and horizontally
    int boneX = tft.width() / 2;
    int boneY = tft.height() / 2 - 20;  // Slightly above center
    int boneWidth = 60;
    int boneHeight = 20;
    int circleRadius = 12;
    
    // Main bone rectangle
    spr.fillRect(boneX - boneWidth/2, boneY - boneHeight/2, 
                 boneWidth, boneHeight, TFT_WHITE);
    
    // Left circles
    spr.fillCircle(boneX - boneWidth/2, boneY - boneHeight/2, 
                   circleRadius, TFT_WHITE);
    spr.fillCircle(boneX - boneWidth/2, boneY + boneHeight/2, 
                   circleRadius, TFT_WHITE);
    
    // Right circles
    spr.fillCircle(boneX + boneWidth/2, boneY - boneHeight/2, 
                   circleRadius, TFT_WHITE);
    spr.fillCircle(boneX + boneWidth/2, boneY + boneHeight/2, 
                   circleRadius, TFT_WHITE);
    
    // Loading text centered below bone
    spr.setTextFont(4);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("...Loading", boneX, boneY + 60);
    
    spr.pushSprite(0, 0);
}

void RoverViewManager::drawFrame() {
    LOG_SCOPE("Drawing view frame");
    
    try {
        // Center the frame on screen
        int frameX = (SCREEN_WIDTH - FRAME_WIDTH) / 2;
        spr.fillRect(frameX, FRAME_Y + 5, FRAME_WIDTH, FRAME_HEIGHT - 5, FRAME_COLOR);
        spr.drawRect(frameX + 2, FRAME_Y + 3, FRAME_WIDTH + 4, FRAME_HEIGHT - 1, FRAME_BORDER_COLOR);
    } catch (const std::exception& e) {
        LOG_PROD("Error in drawFrame: %s", e.what());
    }
}

void RoverViewManager::drawTodoList() {
    LOG_SCOPE("Drawing TODO list view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Today's Tasks:", SCREEN_CENTER_X - 10, FRAME_Y + TITLE_Y_OFFSET);  // Moved left and up
    
    spr.setTextFont(2);
    spr.drawString("1. Service Canada", SCREEN_CENTER_X - 10, FRAME_Y + 35);     // Adjusted Y positions
    spr.drawString("2. Call Doctor", SCREEN_CENTER_X - 10, FRAME_Y + 55);        // and moved left
    spr.drawString("3. Call Therapist", SCREEN_CENTER_X - 10, FRAME_Y + 75);
}

void RoverViewManager::drawQuotes() {
    LOG_SCOPE("Drawing quotes view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Quote of the Day", SCREEN_CENTER_X - 10, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    spr.drawString("\"The best way to predict", SCREEN_CENTER_X - 10, FRAME_Y + 45);
    spr.drawString("the future is to create it.\"", SCREEN_CENTER_X - 10, FRAME_Y + 65);
    spr.drawString("- Peter Drucker", SCREEN_CENTER_X - 10, FRAME_Y + 85);
}

void RoverViewManager::drawWeather() {
    LOG_SCOPE("Drawing weather view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Weather", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET + 15);
    
    spr.setTextFont(2);
    spr.drawString("Sunny", SCREEN_CENTER_X, FRAME_Y + 55);
    spr.drawString("72°F / 22°C", SCREEN_CENTER_X, FRAME_Y + 75);
    spr.drawString("Humidity: 45%", SCREEN_CENTER_X, FRAME_Y + 95);
}

void RoverViewManager::drawStats() {
    LOG_SCOPE("Drawing stats view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("System Stats", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET + 15);
    
    spr.setTextFont(2);
    spr.drawString("Uptime: 3d 12h", SCREEN_CENTER_X, FRAME_Y + 55);
    spr.drawString("Memory: 65% free", SCREEN_CENTER_X, FRAME_Y + 75);
    spr.drawString("WiFi: Connected", SCREEN_CENTER_X, FRAME_Y + 95);
}

// Define symbols for each virtue
void drawChastitySymbol(int x, int y, int size) {
    // Pure white lily
    spr.drawCircle(x, y, size/2, TFT_RED);
    spr.drawLine(x, y - size/2, x, y + size/2, TFT_RED);
    spr.drawLine(x - size/2, y, x + size/2, y, TFT_RED);
}

void drawTemperanceSymbol(int x, int y, int size) {
    // Balanced scales
    spr.drawLine(x - size/2, y, x + size/2, y, 0xFDA0);
    spr.fillCircle(x - size/3, y - size/4, size/6, 0xFDA0);
    spr.fillCircle(x + size/3, y - size/4, size/6, 0xFDA0);
}

void drawCharitySymbol(int x, int y, int size) {
    // Heart with hands
    spr.fillCircle(x - size/4, y - size/4, size/4, 0xFFE0);
    spr.fillCircle(x + size/4, y - size/4, size/4, 0xFFE0);
    spr.fillTriangle(x, y + size/3, x - size/2, y - size/6, x + size/2, y - size/6, 0xFFE0);
}

void drawDiligenceSymbol(int x, int y, int size) {
    // Bee or honeycomb pattern
    for(int i = 0; i < 6; i++) {
        float angle = i * PI / 3;
        int x1 = x + cos(angle) * size/2;
        int y1 = y + sin(angle) * size/2;
        spr.drawLine(x, y, x1, y1, 0x07E0);
    }
}

void drawForgivenessSymbol(int x, int y, int size) {
    // Dove or peaceful wave
    spr.drawCircle(x, y, size/2, 0x001F);
    spr.drawCircle(x, y, size/3, 0x001F);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
}

void drawKindnessSymbol(int x, int y, int size) {
    // Open hands or gift
    spr.drawRect(x - size/2, y - size/2, size, size, 0x180E);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x180E);
    spr.drawLine(x, y - size/2, x, y + size/2, 0x180E);
}

void drawHumilitySymbol(int x, int y, int size) {
    // Bowed head or kneeling figure - using simpler shape since drawArc needs more parameters
    spr.drawCircle(x, y, size/2, 0x780F);
    spr.drawLine(x, y, x, y + size/2, 0x780F);
}

void drawBatteryCharging(int x, int y, int size) {
    // Battery outline
    spr.drawRect(x - size/2, y - size/4, size, size/2, TFT_WHITE);
    spr.drawRect(x + size/2, y - size/8, size/8, size/4, TFT_WHITE);
    
    // Lightning bolt
    spr.fillTriangle(x, y - size/8, x - size/4, y, x, y, TFT_YELLOW);
    spr.fillTriangle(x, y, x + size/4, y, x, y + size/8, TFT_YELLOW);
}

void drawBattery(int x, int y, int size) {

    int batteryWidth = size;
    int sizeY = size;
    int batteryX = x;
    int batteryY = y;
    
    spr.drawRect(batteryX, batteryY, size, sizeY, TFT_WHITE);
    spr.fillRect(batteryX + size, batteryY + 3, 2, 6, TFT_WHITE);
    
    int fillWidth = (size - 4) * PowerManager::getBatteryPercentage() / 100;
    spr.fillRect(batteryX + 2, batteryY + 2, fillWidth, sizeY - 4, TFT_WHITE);

}

void RoverViewManager::drawChakras() {
    LOG_SCOPE("Drawing chakras view");
    
    // Get current day of week (0 = Sunday)
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayIndex = timeInfo->tm_wday;
    
    const ChakraInfo& todayChakra = CHAKRA_DATA[dayIndex];
    
    // Draw title in black with smaller font
    spr.setTextFont(2);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString(todayChakra.name, SCREEN_CENTER_X - 10, FRAME_Y + 15);  // Adjusted position
    
    // Draw chakra symbol
    todayChakra.drawSymbol(SCREEN_CENTER_X - 10, FRAME_Y + 45, 40);  // Adjusted position
    
    // Draw attributes with line breaks
    String description = todayChakra.attributes;
    int y = FRAME_Y + 75;  // Start attributes lower
    int startPos = 0;
    int nextLine;
    
    while ((nextLine = description.indexOf('\n', startPos)) != -1) {
        spr.drawString(description.substring(startPos, nextLine), SCREEN_CENTER_X - 10, y);
        startPos = nextLine + 1;
        y += 20;
    }
    if (startPos < description.length()) {
        spr.drawString(description.substring(startPos), SCREEN_CENTER_X - 10, y);
    }
}

void RoverViewManager::drawVirtues() {
    LOG_SCOPE("Drawing virtues view");
    
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayIndex = timeInfo->tm_wday;
    
    const VirtueInfo& todayVirtue = VIRTUE_DATA[dayIndex];
    
    // Draw virtue title higher and more left
    spr.setTextFont(2);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString(todayVirtue.virtue, 
                  SCREEN_CENTER_X - CONTENT_LEFT_OFFSET, 
                  FRAME_Y + 10);  // Moved up 5 pixels
    
    // Draw virtue symbol
    todayVirtue.drawSymbol(SCREEN_CENTER_X - CONTENT_LEFT_OFFSET, 
                          FRAME_Y + 40, 
                          40);
    
    // Draw description with line breaks
    String description = todayVirtue.description;
    int y = FRAME_Y + 70;  // Start higher
    int startPos = 0;
    int nextLine;
    
    while ((nextLine = description.indexOf('\n', startPos)) != -1) {
        spr.drawString(description.substring(startPos, nextLine), 
                      SCREEN_CENTER_X - CONTENT_LEFT_OFFSET, 
                      y);
        startPos = nextLine + 1;
        y += 20;
    }
    if (startPos < description.length()) {
        spr.drawString(description.substring(startPos), 
                      SCREEN_CENTER_X - CONTENT_LEFT_OFFSET, 
                      y);
    }
}

void RoverViewManager::drawNextMeal() {
    LOG_SCOPE("Drawing recipe view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Garlic Pasta", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET + 15);
    
    spr.setTextFont(2);
    
    const char* ingredients[] = {
        "8 oz Spaghetti",
        "4 Garlic cloves",
        "3 tbsp Olive oil",
        "Salt & Pepper",
    };
    
    int y = FRAME_Y + 55;
    for(int i = 0; i < 4; i++) {
        spr.drawString(ingredients[i], SCREEN_CENTER_X, y);
        y += 20;
    }
}

// Implement all the chakra drawing functions
void drawRootChakra(int x, int y, int size) {
    // Basic square with downward triangle for root chakra
    spr.drawRect(x - size/2, y - size/2, size, size, TFT_RED);
    spr.fillTriangle(x, y + size/2, x - size/2, y - size/2, x + size/2, y - size/2, TFT_RED);
}

void drawSacralChakra(int x, int y, int size) {
    // Crescent moon shape for sacral chakra
    spr.drawCircle(x, y, size/2, 0xFDA0);
    spr.drawCircle(x + size/4, y, size/2, 0xFDA0);
}

void drawSolarChakra(int x, int y, int size) {
    // Sun-like pattern for solar plexus
    spr.drawCircle(x, y, size/2, 0xFFE0);
    for(int i = 0; i < 8; i++) {
        float angle = i * PI / 4;
        int x1 = x + cos(angle) * size/2;
        int y1 = y + sin(angle) * size/2;
        spr.drawLine(x, y, x1, y1, 0xFFE0);
    }
}

void drawHeartChakra(int x, int y, int size) {
    // Heart shape for heart chakra
    spr.fillCircle(x - size/4, y, size/4, 0x07E0);
    spr.fillCircle(x + size/4, y, size/4, 0x07E0);
    spr.fillTriangle(x - size/2, y, x + size/2, y, x, y + size/2, 0x07E0);
}

void drawThroatChakra(int x, int y, int size) {
    // Circle with wings for throat chakra
    spr.drawCircle(x, y, size/3, 0x001F);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
    spr.drawLine(x - size/2, y - size/4, x + size/2, y - size/4, 0x001F);
}

void drawThirdEyeChakra(int x, int y, int size) {
    // Eye shape for third eye chakra
    spr.drawEllipse(x, y, size/2, size/3, 0x180E);
    spr.fillCircle(x, y, size/6, 0x180E);
}

void drawCrownChakra(int x, int y, int size) {
    // Crown-like pattern
    for(int i = 0; i < 7; i++) {
        int x1 = x - size/2 + (i * size/6);
        spr.drawLine(x1, y + size/2, x1, y - size/2, 0x780F);
    }
    spr.drawLine(x - size/2, y - size/2, x + size/2, y - size/2, 0x780F);
}

void RoverViewManager::drawStatusBar() {
    try {
        time_t now = time(nullptr);
        if (now == -1) {
            LOG_PROD("Error getting time in drawStatusBar");
            return;
        }
        
        struct tm* timeInfo = localtime(&now);
        if (!timeInfo) {
            LOG_PROD("Error converting time in drawStatusBar");
            return;
        }
        
        // Status bar positioning
        int dateWidth = 40;
        int dateHeight = 30;
        int dateX = 2;
        
        // Get month colors
        CRGB monthColor1, monthColor2;
        ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
        
        // Draw month color square
        uint32_t monthTftColor = spr.color565(monthColor1.r, monthColor1.g, monthColor1.b);
        if (monthColor1.r == monthColor2.r && 
            monthColor1.g == monthColor2.g && 
            monthColor1.b == monthColor2.b) {
            spr.fillRect(dateX, STATUS_BAR_Y, dateWidth, dateHeight, monthTftColor);
        } else {
            for (int i = 0; i < dateWidth; i++) {
                float ratio = (float)i / dateWidth;
                uint8_t r = monthColor1.r + (monthColor2.r - monthColor1.r) * ratio;
                uint8_t g = monthColor1.g + (monthColor2.g - monthColor1.g) * ratio;
                uint8_t b = monthColor1.b + (monthColor2.b - monthColor1.b) * ratio;
                uint32_t tftColor = spr.color565(r, g, b);
                spr.drawFastVLine(dateX + i, STATUS_BAR_Y, dateHeight, tftColor);
            }
        }
        
        // Draw day number
        char dayStr[3];
        sprintf(dayStr, "%d", timeInfo->tm_mday);
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE, monthTftColor);
        spr.drawString(dayStr, dateX + dateWidth/2, STATUS_BAR_Y + dateHeight/2);
        
        // Status text section
        int statusX = dateX + dateWidth + 30;
        
        if (millis() - lastStatusUpdate >= STATUS_CHANGE_INTERVAL) {
            statusRotation = (statusRotation + 1) % 2;
            lastStatusUpdate = millis();
        }
        
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        
        switch (statusRotation) 
        {
            case 0:
                spr.drawString("Lvl:11 Exp:1,537", 
                              statusX + 35, STATUS_BAR_Y + dateHeight/2);
                break;
                   
            case 1:
                if (PowerManager::isCharging()) 
                {
                    drawBatteryCharging(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                    char batteryStr[5];
                    sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                    spr.drawString(batteryStr, statusX + 27, STATUS_BAR_Y + dateHeight/2);
                } 
                else 
                {
                    drawBattery(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                    char batteryStr[5];
                    sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                    spr.drawString(batteryStr, statusX + 27, STATUS_BAR_Y + dateHeight/2);
                }
                break;  
        }
        
    } catch (const std::exception& e) {
        LOG_PROD("Error in drawStatusBar: %s", e.what());
    }
}   
