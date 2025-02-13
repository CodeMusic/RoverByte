/**
 * @brief Visual interface system implementation
 * 
 * Implements core display management functions:
 * - Display buffer handling
 * - View state transitions
 * - Error visualization
 * - Loading sequence coordination
 * - Cross-modal display feedback
 * 
 * The implementation manages:
 * - Sprite buffer updates
 * - View rendering cycles
 * - Error state handling
 * - Loading animations
 * - Visual feedback loops
 * 
 * @note Coordinates with RoverManager for facial expressions
 * @note Integrates with PowerManager for state transitions
 * @note Synchronizes with MenuManager for UI overlays
 */

#include "RoverViewManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/DisplayConfig.h"
#include "../VisualCortex/RoverManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include <TFT_eSPI.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../CorpusCallosum/SynapticPathways.h"


namespace VisualCortex 
{
    // Add namespace aliases for cleaner code
    namespace PC = PrefrontalCortex;
    namespace SC = SomatosensoryCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    
    using namespace CorpusCallosum;
    using PC::Utilities;
    using PC::PowerManager;
    using SC::MenuManager;
    using PC::SDManager;
    using PSY::WiFiManager;
    using PC::RoverTypes::Expression;

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite spr = TFT_eSprite(&tft);

    // Initialize static members
    RoverViewManager::ViewType RoverViewManager::currentView = ViewType::VIRTUES;
    unsigned long RoverViewManager::lastStatusUpdate = 0;
    int RoverViewManager::statusRotation = 0;
    int RoverViewManager::currentFrameX = 0;
    int RoverViewManager::currentFrameY = 0;
    uint32_t RoverViewManager::experience = 0;
    uint16_t RoverViewManager::experienceToNextLevel = 100;
    uint8_t RoverViewManager::level = 1;
    RoverViewManager::Notification RoverViewManager::currentNotification = {"", "", "", 0, 0};
    bool RoverViewManager::notificationActive = false;
    unsigned long RoverViewManager::lastCounterUpdate = 0;
    unsigned long RoverViewManager::lastAnimationStep = 0;
    bool RoverViewManager::isAnimating = false;
    int RoverViewManager::animationStep = 0;
    unsigned long RoverViewManager::lastExpressionChange = 0;
    unsigned long RoverViewManager::nextExpressionInterval = DEFAULT_EXPRESSION_INTERVAL;
    uint32_t RoverViewManager::roverExperience = 0;
    uint32_t RoverViewManager::roverExperienceToNextLevel = 327;
    uint8_t RoverViewManager::roverLevel = 1;
    uint32_t RoverViewManager::errorCode = 0;
    const char* RoverViewManager::genericErrorMessage = nullptr;
    const char* RoverViewManager::detailedErrorMessage = nullptr;
    bool RoverViewManager::isError = false;
    bool RoverViewManager::isFatalError = false;
    unsigned long RoverViewManager::warningStartTime = 0;
    bool RoverViewManager::initialized = false;

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

    uint16_t getCurrentDayColor() {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        CRGB dayColor = VisualSynesthesia::getDayColor(timeInfo->tm_wday + 1);
        return VisualSynesthesia::convertToRGB565(dayColor);
    }

    void RoverViewManager::init() 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::init()");
        
        try 
        {
            // Initialize TFT display first
            tft.init();
            tft.setRotation(0);  // Landscape mode
            tft.fillScreen(TFT_BLACK);
            
            // Create sprite directly without assignment
            spr.createSprite(DisplayConfig::SCREEN_WIDTH, DisplayConfig::SCREEN_HEIGHT);
            spr.fillSprite(TFT_BLACK);
            
            // Set default text properties
            spr.setTextDatum(MC_DATUM);
            spr.setTextColor(TFT_WHITE, TFT_BLACK);
            
            // Initialize error state
            errorCode = 0;
            genericErrorMessage = "System Error Detected";
            detailedErrorMessage = "";
            isError = false;
            isFatalError = false;
            
            // Draw initial frame
            drawFrame();
            
            // Push initial frame to display
            spr.pushSprite(0, 0);
            
            initialized = true;
            
            Utilities::LOG_DEBUG("RoverViewManager initialized successfully");
        }
        catch (const std::exception& e) 
        {
            PC::Utilities::LOG_ERROR("RoverViewManager initialization failed: %s", e.what());
            initialized = false;
            throw;
        }
    }

    void RoverViewManager::setCurrentView(ViewType view) {
        PC::Utilities::LOG_DEBUG("Changing view to: %d", static_cast<int>(view));
        currentView = view;
    }

    void RoverViewManager::nextView() {
        PC::Utilities::LOG_DEBUG("Changing from view %d to next view", static_cast<int>(currentView));
        currentView = static_cast<ViewType>((static_cast<int>(currentView) + 1) % static_cast<int>(ViewType::NUM_VIEWS));
        PC::Utilities::LOG_DEBUG("New view is %d", static_cast<int>(currentView));
        drawCurrentView();
    }

    void RoverViewManager::previousView() {
        PC::Utilities::LOG_DEBUG("Changing from view %d to previous view", static_cast<int>(currentView));
        currentView = static_cast<ViewType>((static_cast<int>(currentView) - 1 + static_cast<int>(ViewType::NUM_VIEWS)) % static_cast<int>(ViewType::NUM_VIEWS));
        PC::Utilities::LOG_DEBUG("New view is %d", static_cast<int>(currentView));
        drawCurrentView();
    }

    void RoverViewManager::drawCurrentView() 
    {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawCurrentView()");
        if (!isInitialized || !spr.created()) {
            Utilities::LOG_ERROR("VisualCortex::RoverViewManager not properly initialized");
            return;
        }

        if (MenuManager::isVisible()) {
            return;
        }
        
        static bool isRecovering = false;
        
        try {
            if (isRecovering) {
                if (!spr.created()) return;
                spr.fillSprite(TFT_BLACK);
                spr.setTextFont(2);
                spr.setTextColor(TFT_WHITE, TFT_BLACK);
                spr.drawString("Display Error", DisplayConfig::SCREEN_CENTER_X, DisplayConfig::SCREEN_HEIGHT/2);
                spr.pushSprite(0, 0);
                delay(1000);
                isRecovering = false;
                return;
            }
            
            if (!spr.created()) return;
            spr.fillSprite(TFT_BLACK);
            
            if (isError) {
                drawErrorScreen(errorCode, genericErrorMessage, detailedErrorMessage, isFatalError);
                return;
            }
            
            RoverViewManager::drawFrame();
            RoverViewManager::drawStatusBar();
            
            spr.setTextColor(TFT_BLACK, FRAME_COLOR);
            spr.setTextDatum(MC_DATUM);
            
            switch(currentView) {
                case ViewType::TODO_LIST:
                    RoverViewManager::drawTodoList();
                    break;
                case ViewType::CHAKRAS:
                    RoverViewManager::drawChakras();
                    break;
                case ViewType::VIRTUES:
                    RoverViewManager::drawVirtues();
                    break;
                case ViewType::QUOTES:
                    RoverViewManager::drawQuotes();
                    break;
                case ViewType::WEATHER:
                    RoverViewManager::drawWeather();
                    break;
                case ViewType::STATS:
                    RoverViewManager::drawStats();
                    break;
                case ViewType::NEXTMEAL:
                    RoverViewManager::drawNextMeal();
                    break;
                default:
                    break;
            }
            
            spr.pushSprite(0, 0);
            
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("Error in drawCurrentView: %s", e.what());
            isRecovering = true;
            RoverViewManager::drawCurrentView();  // Try again with error recovery
        }
    }

    void RoverViewManager::drawLoadingScreen(const char* statusText) {
        static unsigned long lastBoneRotation = 0;
        static int rotationAngle = 0;
        
        // Create temporary sprite for bone
        TFT_eSprite boneSpr = TFT_eSprite(&tft);
        boneSpr.createSprite(80, 80);
        boneSpr.fillSprite(TFT_BLACK);
        
        // Draw bone centered in sprite
        int tempX = 20;  // Center of sprite
        int tempY = 20;  // Center of sprite
        int boneWidth = 40;
        int boneHeight = 15;
        int circleRadius = 8;
        
        // Draw bone components
        boneSpr.fillRect(tempX - boneWidth/2, tempY - boneHeight/2, boneWidth, boneHeight, TFT_WHITE);
        boneSpr.fillCircle(tempX - boneWidth/2, tempY - boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX - boneWidth/2, tempY + boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX + boneWidth/2, tempY - boneHeight/2, circleRadius, TFT_WHITE);
        boneSpr.fillCircle(tempX + boneWidth/2, tempY + boneHeight/2, circleRadius, TFT_WHITE);
        
        spr.fillSprite(TFT_BLACK);
        
        // Keep text centered
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE);
        spr.drawCentreString("Loading...", DisplayConfig::SCREEN_CENTER_X - 25, 75, 2);
        
        if (statusText) {
            spr.setTextFont(1);
            spr.drawCentreString(statusText, DisplayConfig::SCREEN_CENTER_X - 25, 100, 1);
        }
        
        // Push rotated bone sprite to main sprite
        boneSpr.pushRotated(&spr, rotationAngle);
        
        if (millis() - lastBoneRotation > 50) {
            rotationAngle = (rotationAngle + 45) % 360;
            lastBoneRotation = millis();
        }
        
        boneSpr.deleteSprite();
        
        // Push main sprite to display
        spr.pushSprite(0, 0);
    }

    void RoverViewManager::drawFrame() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawFrame()");
        int frameX = (TFT_WIDTH - FRAME_WIDTH) / 2;
        int frameY = FRAME_Y;
        
        // Draw the frame
        spr.fillRect(frameX, frameY, FRAME_WIDTH, FRAME_HEIGHT + 10, FRAME_COLOR);
        spr.drawRect(frameX - 1, frameY - 1, FRAME_WIDTH + 2, FRAME_HEIGHT + 10, FRAME_BORDER_COLOR);
        
        // Store frame position for content alignment
        currentFrameX = frameX;
        currentFrameY = frameY;
    }

    void RoverViewManager::drawTodoList() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawTodoList()");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Today's Tasks:", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET);

        spr.setTextFont(2); // Regular font for content
        String task1 = RoverViewManager::wordWrap("1. Service Canada", DisplayConfig::SCREEN_WIDTH - 50);
        String task2 = RoverViewManager::wordWrap("2. Call Doctor", DisplayConfig::SCREEN_WIDTH - 50);
        String task3 = RoverViewManager::wordWrap("3. Call Therapist", DisplayConfig::SCREEN_WIDTH - 50);

        spr.drawString(task1, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 35);
        spr.drawString(task2, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 55);
        spr.drawString(task3, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 75);
    }

    void RoverViewManager::drawQuotes() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawQuotes()");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Quote of the Day", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET);

        spr.setTextFont(2); // Regular font for content
        String quote1 = RoverViewManager::wordWrap("\"The best way to predict", DisplayConfig::SCREEN_WIDTH - 50);
        String quote2 = RoverViewManager::wordWrap("the future is to create it.\"", DisplayConfig::SCREEN_WIDTH - 50);
        String author = RoverViewManager::wordWrap("- Peter Drucker", DisplayConfig::SCREEN_WIDTH - 50);

        spr.drawString(quote1, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 45);
        spr.drawString(quote2, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 65);
        spr.drawString(author, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 85);
    }

    void RoverViewManager::drawWeather() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawWeather()");

        spr.setTextFont(4); // Larger font for header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Weather", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET + 15);

        spr.setTextFont(2); // Regular font for content
        String weather = RoverViewManager::wordWrap("Sunny", DisplayConfig::SCREEN_WIDTH - 50);
        String temperature = RoverViewManager::wordWrap("72°F / 22°C", DisplayConfig::SCREEN_WIDTH - 50);
        String humidity = RoverViewManager::wordWrap("Humidity: 45%", DisplayConfig::SCREEN_WIDTH - 50);

        spr.drawString(weather, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 55);
        spr.drawString(temperature, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 75);
        spr.drawString(humidity, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 95);
    }

    void RoverViewManager::drawStats() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawStats()");
        
        spr.setTextFont(3);
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("System Stats", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + TITLE_Y_OFFSET + 15);
        
        spr.setTextFont(2);
        unsigned long uptime = millis(); // Get the uptime in milliseconds
        String uptimeString = RoverViewManager::formatUptime(uptime);
        
        spr.drawString("Uptime: " + uptimeString, DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 15);
        spr.drawString("SD Card Size: " + String(SDManager::getCardSize()) + "MB", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 75);
        spr.drawString("SD Card Used: " + String(SDManager::getUsedSpace()) + "MB", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 95);
        spr.drawString("SD Card Total: " + String(SDManager::getTotalSpace()) + "MB", DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 115);
        spr.drawString("WiFi: " + String(WiFiManager::isConnected() ? "Connected" : "Disconnected"), DisplayConfig::SCREEN_CENTER_X - DisplayConfig::FRAME_OFFSET_X, FRAME_Y + 135);
    }

    String RoverViewManager::formatUptime(unsigned long uptimeMillis) {
        unsigned long seconds = uptimeMillis / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;

        seconds %= 60;
        minutes %= 60;

        String formattedUptime = String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
        return formattedUptime;
    }

    // Define symbols for each virtue
    void RoverViewManager::drawChastitySymbol(int x, int y, int size) {
        // Pure white lily
        spr.drawCircle(x, y, size/2, TFT_RED);
        spr.drawLine(x, y - size/2, x, y + size/2, TFT_RED);
        spr.drawLine(x - size/2, y, x + size/2, y, TFT_RED);
    }

    void RoverViewManager::drawTemperanceSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
        spr.fillTriangle(x, y + size/6, x - size/6, y + size/6, x + size/6, y + size/6, symbolColor);
        spr.drawLine(x, y + size/6, x, y + size/2, symbolColor);
        spr.drawLine(x - size/4, y + size/2, x + size/4, y + size/2, symbolColor);
        spr.fillCircle(x - size/2, y, size/8, symbolColor);
        spr.fillCircle(x + size/2, y, size/8, symbolColor);
    }

    void RoverViewManager::drawCharitySymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.fillCircle(x - size/4, y - size/4, size/4, symbolColor);
        spr.fillCircle(x + size/4, y - size/4, size/4, symbolColor);
        spr.fillTriangle(x, y + size/3, x - size/2, y - size/6, x + size/2, y - size/6, symbolColor);
    }

    void RoverViewManager::drawDiligenceSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        for(int i = 0; i < 6; i++) {
            float angle = i * PI / 3;
            int x1 = x + cos(angle) * size/2;
            int y1 = y + sin(angle) * size/2;
            spr.drawLine(x, y, x1, y1, symbolColor);
        }
    }

    void RoverViewManager::drawForgivenessSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawCircle(x, y, size/2, symbolColor);
        spr.drawCircle(x, y, size/3, symbolColor);
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
    }

    void RoverViewManager::drawKindnessSymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawRect(x - size/2, y - size/2, size, size, symbolColor);
        spr.drawLine(x - size/2, y, x + size/2, y, symbolColor);
        spr.drawLine(x, y - size/2, x, y + size/2, symbolColor);
    }

    void RoverViewManager::drawHumilitySymbol(int x, int y, int size) {
        uint16_t symbolColor = getCurrentDayColor();
        spr.drawCircle(x, y, size/2, symbolColor);
        spr.drawLine(x, y, x, y + size/2, symbolColor);
    }

    void RoverViewManager::drawBatteryCharging(int x, int y, int size) {
        // Draw base battery first
        drawBattery(x, y, size);
        
        // Add lightning bolt overlay
        int batteryX = x - size/2;
        int batteryY = y - size/4;
        
        // Lightning bolt
        spr.fillTriangle(x - 2, batteryY + 2, x - size/4, y, x + 2, y, TFT_YELLOW);
        spr.fillTriangle(x - 2, y, x + size/4, y, x + 2, batteryY + size/2 - 2, TFT_YELLOW);
    }

    void RoverViewManager::drawBattery(int x, int y, int size) {
        int batteryWidth = size;
        int batteryHeight = size/2;
        int batteryX = x - size/2;
        int batteryY = y - size/4;
        
        // Main battery body
        spr.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
        
        // Battery terminal
        int terminalWidth = size/8;
        int terminalHeight = batteryHeight/2;
        spr.fillRect(batteryX + batteryWidth, batteryY + (batteryHeight - terminalHeight)/2, 
                    terminalWidth, terminalHeight, TFT_WHITE);
        
        // Fill level
        int fillWidth = (batteryWidth - 4) * PowerManager::getBatteryPercentage() / 100;
        spr.fillRect(batteryX + 2, batteryY + 2, fillWidth, batteryHeight - 4, TFT_WHITE);
    }

    void RoverViewManager::drawChakras() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawChakras()");

        // Set unique font for the header
        spr.setTextFont(5); // Unique font for chakras header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Chakras", DisplayConfig::SCREEN_CENTER_X - 40, FRAME_Y + TITLE_Y_OFFSET); // Centered header

        // Set regular font for content
        spr.setTextFont(2);
        int y = FRAME_Y + 45; // Start drawing content below the header

        for (int i = 0; i < sizeof(CHAKRA_DATA) / sizeof(CHAKRA_DATA[0]); i++) {
            const ChakraInfo& chakra = CHAKRA_DATA[i];
            
            // Draw chakra symbol
            chakra.drawSymbol(DisplayConfig::SCREEN_CENTER_X - 40, y, 20); // Adjust position for symbol
            y += 25; // Move down for the next chakra

            // Draw attributes with word wrap
            String wrappedAttributes = wordWrap(chakra.attributes, DisplayConfig::SCREEN_WIDTH - 50);
            spr.drawString(wrappedAttributes, DisplayConfig::SCREEN_CENTER_X - 40, y);
            y += 40; // Add space for the next chakra
        }
    }

    void RoverViewManager::drawVirtues() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawVirtues()");

        // Set unique font for the header
        spr.setTextFont(5); // Unique font for virtues header
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Virtues", DisplayConfig::SCREEN_CENTER_X - 40, FRAME_Y + TITLE_Y_OFFSET); // Centered header

        // Set regular font for content
        spr.setTextFont(2);
        int y = FRAME_Y + 45; // Start drawing content below the header

        for (int i = 0; i < sizeof(VIRTUE_DATA) / sizeof(VIRTUE_DATA[0]); i++) {
            const VirtueInfo& virtue = VIRTUE_DATA[i];

            // Draw attributes with word wrap
            String wrappedDescription = wordWrap(virtue.description, DisplayConfig::SCREEN_WIDTH - 50);
            spr.drawString(wrappedDescription, DisplayConfig::SCREEN_CENTER_X - 40, y);
            y += 40; // Add space for the next virtue
        }
    }

    void RoverViewManager::drawNextMeal() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawNextMeal()");
        
        spr.setTextFont(4);
        spr.setTextColor(TFT_BLACK, FRAME_COLOR);
        spr.drawString("Garlic Pasta", DisplayConfig::SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET + 15);
        
        spr.setTextFont(2);
        
        const char* ingredients[] = {
            "8 oz Spaghetti",
            "4 Garlic cloves",
            "3 tbsp Olive oil",
            "Salt & Pepper",
        };
        
        int y = FRAME_Y + 55;
        for(int i = 0; i < 4; i++) {
            spr.drawString(ingredients[i], DisplayConfig::SCREEN_CENTER_X, y);
            y += 20;
        }
    }

    // Implement all the chakra drawing functions
    void RoverViewManager::drawRootChakra(int x, int y, int size) {
        // Basic square with downward triangle for root chakra
        spr.drawRect(x - size/2, y - size/2, size, size, TFT_RED);
        spr.fillTriangle(x, y + size/2, x - size/2, y - size/2, x + size/2, y - size/2, TFT_RED);
    }

    void RoverViewManager::drawSacralChakra(int x, int y, int size) {
        // Crescent moon shape for sacral chakra
        spr.drawCircle(x, y, size/2, 0xFDA0);
        spr.drawCircle(x + size/4, y, size/2, 0xFDA0);
    }

    void RoverViewManager::drawSolarChakra(int x, int y, int size) {
        // Sun-like pattern for solar plexus
        spr.drawCircle(x, y, size/2, 0xFFE0);
        for(int i = 0; i < 8; i++) {
            float angle = i * PI / 4;
            int x1 = x + cos(angle) * size/2;
            int y1 = y + sin(angle) * size/2;
            spr.drawLine(x, y, x1, y1, 0xFFE0);
        }
    }

    void RoverViewManager::drawHeartChakra(int x, int y, int size) {
        // Heart shape for heart chakra
        spr.fillCircle(x - size/4, y, size/4, 0x07E0);
        spr.fillCircle(x + size/4, y, size/4, 0x07E0);
        spr.fillTriangle(x - size/2, y, x + size/2, y, x, y + size/2, 0x07E0);
    }

    void RoverViewManager::drawThroatChakra(int x, int y, int size) {
        // Circle with wings for throat chakra
        spr.drawCircle(x, y, size/3, 0x001F);
        spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
        spr.drawLine(x - size/2, y - size/4, x + size/2, y - size/4, 0x001F);
    }

    void RoverViewManager::drawThirdEyeChakra(int x, int y, int size) {
        // Eye shape for third eye chakra
        spr.drawEllipse(x, y, size/2, size/3, 0x180E);
        spr.fillCircle(x, y, size/6, 0x180E);
    }

    void RoverViewManager::drawCrownChakra(int x, int y, int size) {
        // Crown-like pattern
        for(int i = 0; i < 7; i++) {
            int x1 = x - size/2 + (i * size/6);
            spr.drawLine(x1, y + size/2, x1, y - size/2, 0x780F);
        }
        spr.drawLine(x - size/2, y - size/2, x + size/2, y - size/2, 0x780F);
    }

    void RoverViewManager::drawStatusBar() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawStatusBar()");
        try {
            time_t now = time(nullptr);
            if (now == -1) {
                Utilities::LOG_PROD("Error getting time in drawStatusBar");
                return;
            }
            
            struct tm* timeInfo = localtime(&now);
            if (!timeInfo) {
                Utilities::LOG_PROD("Error converting time in drawStatusBar");
                return;
            }
            
            // Status bar positioning
            int dateWidth = 40;
            int dateHeight = 30;
            int dateX = 0;  // Changed from 2 to 0
            
            // Get month colors
            CRGB monthColor1, monthColor2;
            VisualSynesthesia::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
            
            // Draw month color square
            uint32_t monthTftColor = spr.color565(monthColor1.r, monthColor1.g, monthColor1.b);
            if (monthColor1.r == monthColor2.r && 
                monthColor1.g == monthColor2.g && 
                monthColor1.b == monthColor2.b) {
                spr.fillRect(dateX, STATUS_BAR_Y - 5, dateWidth, dateHeight, monthTftColor);  // Added -2 to Y
            } else {
                for (int i = 0; i < dateWidth; i++) {
                    float ratio = (float)i / dateWidth;
                    uint8_t r = monthColor1.r + (monthColor2.r - monthColor1.r) * ratio;
                    uint8_t g = monthColor1.g + (monthColor2.g - monthColor1.g) * ratio;
                    uint8_t b = monthColor1.b + (monthColor2.b - monthColor1.b) * ratio;
                    uint32_t tftColor = spr.color565(r, g, b);
                    spr.drawFastVLine(dateX + i, STATUS_BAR_Y - 5, dateHeight, tftColor);  // Added -2 to Y
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
            
            switch (statusRotation) {
                case 0:
                    char statsStr[20];
                    sprintf(statsStr, "Lvl:%d Exp:%d", 
                            (roverLevel / 10) + 1,  // Level increases every 10 scans
                            roverExperience);
                    spr.drawString(statsStr, statusX + 35, STATUS_BAR_Y + dateHeight/2);
                    break;
                case 1:
                    if (PowerManager::isCharging()) {
                        drawBatteryCharging(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                        char batteryStr[5];
                        sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                        spr.drawString(batteryStr, statusX + 45, STATUS_BAR_Y + dateHeight/2);
                    } else {
                        drawBattery(statusX, STATUS_BAR_Y + dateHeight/2, 19);
                        char batteryStr[5];
                        sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                        spr.drawString(batteryStr, statusX + 45, STATUS_BAR_Y + dateHeight/2);
                    }
                    break;
            }
            
        } catch (const std::exception& e) {
            Utilities::LOG_PROD("Error in drawStatusBar: %s", e.what());
        }
    }   

    void RoverViewManager::incrementExperience(uint16_t amount) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::incrementExperience(uint16_t)");
        experience += amount;
        
        while (experience >= 327) {
            level++;
            experience -= 327;
            LEDManager::flashLevelUp();
            
            char levelStr[32];
            snprintf(levelStr, sizeof(levelStr), "Level %d!", level);
            showNotification("LEVEL UP", levelStr, "XP", 2000);
        }
        
        // Update experience display
        char expStr[32];
        snprintf(expStr, sizeof(expStr), "XP: %d/327", experience);
        updateExperienceBar(expStr);
    }

    uint16_t RoverViewManager::calculateNextLevelExperience(uint8_t currentLevel) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::calculateNextLevelExperience(uint8_t)");
        // Simple exponential growth formula
        return 100 * (currentLevel + 1);
    }       

    void RoverViewManager::showNotification(const char* header, const char* content, const char* symbol, int duration) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::showNotification(const char*, const char*, const char*, int)");
        currentNotification = {header, content, symbol, millis(), duration};
        notificationActive = true;
        drawNotification();
    }

    void RoverViewManager::drawNotification() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawNotification()");
        if (!notificationActive) return;
        
        // Fill entire screen with dark background
        spr.fillSprite(TFT_BLACK);
        
        // Draw full-height notification box
        int boxWidth = 160;
        int boxHeight = DisplayConfig::SCREEN_HEIGHT - 20; // Full height minus margins
        int boxX = 10;
        int boxY = 10;
        
        spr.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 8, TFT_DARKGREY);
        spr.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 8, TFT_WHITE);
        
        // Draw header
        spr.setTextFont(2);
        spr.setTextColor(TFT_WHITE);
        spr.drawCentreString(currentNotification.header, boxX + boxWidth/2, boxY + 10, 2);
        
        // If this is an NFC notification, try to read card data
        if (strcmp(currentNotification.symbol, "NFC") == 0) {
            uint32_t cardId =PSY::NFCManager::getLastCardId();
            char idStr[32];
            sprintf(idStr, "Card ID: %08X", cardId);
            spr.drawCentreString(idStr, boxX + boxWidth/2, boxY + 40, 2);
            
            // Try to read card data
            if (PSY::NFCManager::isCardEncrypted()) {
                drawSymbol("PADLOCK", boxX + boxWidth/2, boxY + boxHeight/2, 40);
                spr.drawCentreString("Encrypted Card", boxX + boxWidth/2, boxY + boxHeight - 60, 2);
            } else {
                // Show card data if available
                const char* cardData = PSY::NFCManager::getCardData();
                if (cardData) {
                    RoverViewManager::drawWordWrappedText(cardData, boxX + 10, boxY + 80, boxWidth - 20);
                }
            }
        } else {
            // Regular notification display
            RoverViewManager::drawSymbol(currentNotification.symbol, boxX + boxWidth/2, boxY + boxHeight/2, 40);
            RoverViewManager::drawWordWrappedText(currentNotification.content, boxX + 100, boxY + boxHeight - 60, boxWidth - 20);
        }
    }

    void RoverViewManager::drawSymbol(const char* symbol, int x, int y, int size) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawSymbol(const char*, int, int, int)");
        if (strcmp(symbol, "PADLOCK") == 0) {
            // Draw padlock body
            spr.fillRoundRect(x - size/3, y, size*2/3, size/2, size/8, TFT_WHITE);
            // Draw shackle
            spr.drawRoundRect(x - size/2, y - size/3, size, size/2, size/8, TFT_WHITE);
        } else if (strcmp(symbol, "NFC") == 0) {
            // Draw magnifying glass
            int glassSize = size * 0.8;
            // Draw circle
            spr.drawCircle(x, y, glassSize/2, TFT_WHITE);
            // Draw handle
            spr.drawLine(x + (glassSize/2 * 0.7), y + (glassSize/2 * 0.7), 
                        x + glassSize, y + glassSize, TFT_WHITE);
            // Fill circle with thinner border
            spr.fillCircle(x, y, (glassSize/2) - 2, TFT_BLACK);
            spr.drawCircle(x, y, (glassSize/2) - 2, TFT_WHITE);
            // Make handle thicker
            spr.drawLine(x + (glassSize/2 * 0.7) - 1, y + (glassSize/2 * 0.7), 
                        x + glassSize - 1, y + glassSize, TFT_WHITE);
            spr.drawLine(x + (glassSize/2 * 0.7) + 1, y + (glassSize/2 * 0.7), 
                        x + glassSize + 1, y + glassSize, TFT_WHITE);
        }
    }

    void RoverViewManager::clearNotification() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::clearNotification()");
        notificationActive = false;
        currentNotification = {"", "", "", 0, 0};
    }

    bool RoverViewManager::hasActiveNotification() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::hasActiveNotification()");
        return notificationActive;
    }   

    void RoverViewManager::handleInput(InputType input) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::handleInput(InputType)");
        if (hasActiveNotification()) {
            clearNotification();
            return;
        }
        
        switch (input) {
            case InputType::INPUT_LEFT:
                previousView();
                break;
                
            case InputType::INPUT_RIGHT:
                nextView();
                break;
        }
    }

    void RoverViewManager::drawWordWrappedText(const char* text, int x, int y, int maxWidth) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawWordWrappedText(const char*, int, int, int)");
        if (!text) return;
        
        const int lineHeight = 20;  // Adjust based on your font size
        char buffer[256];
        int currentLine = 0;
        int bufferIndex = 0;
        int lastSpace = -1;
        
        for (int i = 0; text[i] != '\0'; i++) {
            buffer[bufferIndex++] = text[i];
            buffer[bufferIndex] = '\0';
            
            if (text[i] == ' ') {
                lastSpace = bufferIndex - 1;
            }
            
            // Check if current line is too long
            if (spr.textWidth(buffer) > maxWidth) {
                if (lastSpace != -1) {
                    // Break at last space
                    buffer[lastSpace] = '\0';
                    spr.drawString(buffer, x, y + (currentLine * lineHeight));
                    
                    // Start new line from word after space
                    bufferIndex = 0;
                    for (int j = lastSpace + 1; j < i; j++) {
                        buffer[bufferIndex++] = text[j];
                    }
                    buffer[bufferIndex] = '\0';
                    lastSpace = -1;
                } else {
                    // Force break if no space found
                    buffer[bufferIndex-1] = '\0';
                    spr.drawString(buffer, x, y + (currentLine * lineHeight));
                    bufferIndex = 0;
                    buffer[bufferIndex++] = text[i];
                    buffer[bufferIndex] = '\0';
                }
                currentLine++;
            }
        }
        
        // Draw remaining text
        if (bufferIndex > 0) {
            spr.drawString(buffer, x, y + (currentLine * lineHeight));
        }
    }

    void RoverViewManager::drawFullScreenMenu(const char* title, const std::vector<SomatosensoryCortex::MenuItem>& items, int selectedIndex) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawFullScreenMenu(const char*, const std::vector<SomatosensoryCortex::MenuItem>&, int)");
        spr.fillSprite(TFT_BLACK);
        
        // Draw title - adjust position to be more centered
        spr.setTextFont(4);
        spr.setTextColor(TFT_WHITE);
        spr.drawString(title, DisplayConfig::SCREEN_CENTER_X - 30, 30);
        
        // Draw menu items
        spr.setTextFont(2);
        int y = 70;
        int menuX = DisplayConfig::SCREEN_CENTER_X - 30;  // Align with title
        
        for (size_t i = 0; i < items.size(); i++) {
            if (i == selectedIndex) {
                spr.setTextColor(TFT_BLACK, TFT_WHITE);
                spr.fillRect(15, y - 10, DisplayConfig::SCREEN_WIDTH - 30, 20, TFT_WHITE);
            } else {
                spr.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            spr.drawString(items[i].name.c_str(), menuX, y);
            y += 25;
        }
        
        spr.pushSprite(0, 0);
    }

    void RoverViewManager::drawAppSplash(const char* title, const char* description) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawAppSplash(const char*, const char*)");
        tft.fillScreen(TFT_BLACK);

        // Draw title near center
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2); // scale up text
        tft.setTextDatum(MC_DATUM); // middle-center
        tft.drawString(title, tft.width() / 2, tft.height() / 2 - 20);

        // Draw description a bit lower
        tft.setTextSize(1);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(description, tft.width() / 2, tft.height() / 2 + 10);

        // Draw instructions at the bottom
        tft.setTextSize(1);
        tft.setTextDatum(BC_DATUM); // bottom-center
        tft.drawString("Press Rotary to Start  |  Side Button = Exit", tft.width() / 2, tft.height() - 5);
    }

    void RoverViewManager::drawMenuBackground() {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawMenuBackground()");
        tft.fillScreen(TFT_BLACK);
    }

    void RoverViewManager::setTextColor(uint16_t color) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::setTextColor(uint16_t)");
        tft.setTextColor(color);
    }

    void RoverViewManager::drawString(const char* str, int x, int y) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawString(const char*, int, int)");
        tft.drawString(str, x, y);
    }

    void RoverViewManager::updateExperienceBar(const String& expStr) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::updateExperienceBar(const String&)");
        roverExperience += expStr.toInt();
        if (roverExperience >= roverExperienceToNextLevel) {
            roverLevel++;
            roverExperience -= roverExperienceToNextLevel;
            roverExperienceToNextLevel = calculateNextLevelExperience(roverLevel);
        }
    }

    void RoverViewManager::drawErrorScreen(uint32_t errorCode, const char* genericMessage, const char* detailedMessage, bool isFatal)
    {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::drawErrorScreen(uint32_t, const char*, const char*, bool)",
            String(errorCode),
            genericMessage,
            detailedMessage,
            String(isFatal)
        );
        
        spr.fillSprite(TFT_BLACK);
        
        // Move everything right 25px and  down 20px
        const int X_OFFSET = 45;
        const int Y_OFFSET = 20;
        
        // Draw ERRORBYTE text and code centered
        spr.setTextFont(2);
        spr.setTextColor(TFT_RED);  // ERRORBYTE always red
        spr.drawCentreString("ERRORBYTE", DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 20 + Y_OFFSET, 2);
        
        char errorCodeStr[32];
        sprintf(errorCodeStr, "0x%08X", (uint8_t)errorCode);
        spr.setTextColor(isFatal ? TFT_RED : TFT_YELLOW);
        spr.drawCentreString(errorCodeStr, DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 40 + Y_OFFSET, 2);
        
        // Define scale first
        static const float scale = 0.8f;
        
        // Center all rover graphics with new offset
        const int roverY = 80 + Y_OFFSET;
        const int roverX = DisplayConfig::SCREEN_CENTER_X - (int)(90*scale/2) - 35 + X_OFFSET;
        
        // Body and ears - wider body
        spr.fillRect(roverX, roverY, 90*scale, 76*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 10*scale, roverY - 10*scale,
                        roverX + 25*scale, roverY + 5*scale,
                        roverX + 40*scale, roverY - 10*scale, TFT_WHITE);
        spr.fillTriangle(roverX + 60*scale, roverY - 10*scale,
                        roverX + 75*scale, roverY + 5*scale,
                        roverX + 90*scale, roverY - 10*scale, TFT_WHITE);
        
        // Eye panel - shorter width
        spr.fillRect(roverX + 10*scale, roverY + 10*scale, 75*scale, 30*scale, 0x7BEF);
        
        // Draw X eyes with thicker lines
        uint16_t eyeColor = isFatal ? TFT_RED : TFT_YELLOW;
        // Left X - thicker
        for(int i = 0; i < 2; i++) {
            spr.drawLine(roverX + (20+i)*scale, roverY + 15*scale, 
                        roverX + (30+i)*scale, roverY + 25*scale, eyeColor);
            spr.drawLine(roverX + (30+i)*scale, roverY + 15*scale,
                        roverX + (20+i)*scale, roverY + 25*scale, eyeColor);
        }
        // Right X - thicker
        for(int i = 0; i < 2; i++) {
            spr.drawLine(roverX + (60+i)*scale, roverY + 15*scale,
                        roverX + (70+i)*scale, roverY + 25*scale, eyeColor);
            spr.drawLine(roverX + (70+i)*scale, roverY + 15*scale,
                        roverX + (60+i)*scale, roverY + 25*scale, eyeColor);
        }
        
        // Draw triangular nose
        spr.fillTriangle(roverX + 45*scale, roverY + 35*scale, 
                        roverX + 40*scale, roverY + 45*scale, 
                        roverX + 50*scale, roverY + 45*scale, TFT_BLACK);
        
        // Vertical line from nose to mouth
        spr.drawLine(roverX + 45*scale, roverY + 45*scale,
                    roverX + 45*scale, roverY + 55*scale, TFT_BLACK);
        
        // Draw full frown with ends
        spr.drawArc(roverX + 45*scale, roverY + 70*scale, 20*scale, 15*scale, 180, 360, TFT_BLACK, TFT_BLACK);
        // Left end
        spr.fillRect(roverX + 25*scale, roverY + 65*scale, 2*scale, 4*scale, TFT_BLACK);
        // Right end
        spr.fillRect(roverX + 63*scale, roverY + 65*scale, 2*scale, 4*scale, TFT_BLACK);
        
        // Draw generic error message
        spr.setTextFont(2);
        spr.setTextColor(isFatal ? TFT_RED : TFT_YELLOW);
        spr.drawCentreString(genericMessage, DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 160 + Y_OFFSET, 2);
        
        // Draw detailed message in smaller font below
        spr.setTextFont(1);
        spr.setTextColor(TFT_WHITE);
        spr.drawCentreString(detailedMessage, DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 180 + Y_OFFSET, 1);
        
        // For warnings, show countdown on separate line in white (moved down 20px)
        if (!isFatal && PC::RoverBehaviorManager::isWarningCountdownActive()) 
        {
            spr.setTextColor(TFT_WHITE);
            char countdownStr[32];
            int remainingSeconds = PC::RoverBehaviorManager::getRemainingWarningSeconds();
            sprintf(countdownStr, "Clearing in %d...", remainingSeconds);
            spr.drawCentreString(countdownStr, DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 220 + Y_OFFSET, 2);
        }
        
        // Show reboot instruction ONLY for fatal errors
        if (isFatal) 
        {
            spr.setTextFont(1);
            spr.setTextColor(TFT_YELLOW);
            spr.drawCentreString("Press Rotary to REBOOT", DisplayConfig::SCREEN_CENTER_X - 40 + X_OFFSET, 225 + Y_OFFSET, 1);
        }
        
        spr.pushSprite(0, 0);
        LEDManager::setErrorPattern(errorCode, isFatal);
    }

    String RoverViewManager::wordWrap(String text, int maxWidth) {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::wordWrap(String, int)");
        String wrappedText;
        int lastSpace = -1;
        int lineLength = 0;

        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            lineLength++;

            if (c == ' ') {
                lastSpace = i; // Record the last space
            }

            if (lineLength > maxWidth) {
                if (lastSpace != -1) {
                    wrappedText += text.substring(0, lastSpace) + "\n"; // Add line break
                    text = text.substring(lastSpace + 1); // Update text
                    i = -1; // Reset index for new line
                    lineLength = 0; // Reset line length
                    lastSpace = -1; // Reset last space
                }
            }
        }

        wrappedText += text; // Add any remaining text
        return wrappedText;
    }

    void RoverViewManager::clearSprite() 
    {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::clearSprite()");
        spr.fillSprite(TFT_BLACK);
        spr.pushSprite(0, 0);
    }

    void RoverViewManager::pushSprite() 
    {
        Utilities::LOG_SCOPE("VisualCortex::RoverViewManager::pushSprite()");
        spr.pushSprite(0, 0);
    }

}