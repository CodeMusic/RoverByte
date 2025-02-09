/**
 * @brief Manages rover's visual interface and display coordination
 * 
 * Controls the rover's cognitive display systems:
 * - View state management
 * - Display buffer coordination
 * - Visual feedback processing
 * - Error state visualization
 * - Cross-modal display integration
 * 
 * Coordinates display of:
 * - Todo lists and reminders
 * - Chakra and virtue systems
 * - Weather and environmental data
 * - Statistical processing
 * - Temporal meal planning
 */

#ifndef ROVER_VIEW_MANAGER_H
#define ROVER_VIEW_MANAGER_H

#include "TFT_eSPI.h"
#include "PrefrontalCortex/Utilities.h"
#include "VisualSynesthesia.h"
#include "PrefrontalCortex/PowerManager.h"
#include "AuditoryCortex/SoundFxManager.h"
#include <vector>
#include "SomatosensoryCortex/MenuManager.h"
#include "MotorCortex/PinDefinitions.h"

namespace VisualCortex 
{
    // Add namespace aliases for cleaner code
    namespace PC = PrefrontalCortex;
    namespace SC = SomatosensoryCortex;
    
    extern TFT_eSPI tft;
    extern TFT_eSprite spr;

    class RoverViewManager {
        
    public:
        enum class ViewType { 
            TODO_LIST,
            CHAKRAS,
            VIRTUES,
            QUOTES,
            WEATHER,
            STATS,
            NEXTMEAL,
            NUM_VIEWS
        };

        static ViewType currentView;
        
        static void init();
        static void setCurrentView(ViewType view);
        static void nextView();
        static void previousView();
        static void drawStatusBar();
        static void drawCurrentView();
        static ViewType getCurrentView() { return currentView; }
        static void drawLoadingScreen(const char* statusText);  
        static void incrementExperience(uint16_t amount);
        static void drawAppSplash(const char* title, const char* description);

        static void setTextColor(uint16_t color);
        static void drawString(const char* str, int x, int y);

        struct Notification {
            const char* header;
            const char* content;
            const char* symbol;
            unsigned long startTime;
            int duration;
        };

        static void showNotification(const char* header, const char* content, const char* symbol, int duration = 3000);
        static void drawNotification();
        static void clearNotification();
        static bool hasActiveNotification();

        enum class InputType {
            INPUT_LEFT,
            INPUT_RIGHT
        };
        static void handleInput(InputType input);

        static void drawFullScreenMenu(const char* title, const std::vector<SC::MenuItem>& items, int selectedIndex);

        static void drawMenuBackground();
        static String formatUptime(unsigned long uptimeMillis);
        static void drawErrorScreen(uint32_t errorCode, const char* errorMessage, bool isFatal);
        static uint32_t errorCode;
        static const char* errorMessage;
        static bool isError;
        static bool isFatalError;

        static String wordWrap(String text, int maxWidth);
        static void drawWordWrappedText(const char* text, int x, int y, int maxWidth);

        static void clearSprite();
        static void pushSprite();

        static bool isInitialized() { return initialized; }
        static bool isValid() { 
            return isInitialized() && !isFatalError; 
        }

    private:
        static bool initialized;
        static int currentFrameX;
        static int currentFrameY;
        
        struct ChakraInfo {
            const char* name;
            const char* attributes;
            uint16_t color;
            void (*drawSymbol)(int x, int y, int size);
        };

        static const int STATUS_BAR_Y = 170;
        static const int STATUS_BAR_HEIGHT = 30;
        static unsigned long lastStatusUpdate;
        static const unsigned long STATUS_CHANGE_INTERVAL = 3000;
        static int statusRotation;

        struct VirtueInfo {
            const char* virtue;
            const char* description;
            uint16_t color;
            void (*drawSymbol)(int x, int y, int size);
        };

        static const ChakraInfo CHAKRA_DATA[];
        static const VirtueInfo VIRTUE_DATA[];


        static const int FRAME_X = 2;
        static const int FRAME_Y = 190;
        static const int FRAME_WIDTH = 280;
        static const int FRAME_HEIGHT = 120;
        static const int TITLE_Y_OFFSET = 15;
        static const int CONTENT_LEFT_OFFSET = 35;
        static const uint16_t FRAME_COLOR = 0xC618;
        static const uint16_t FRAME_BORDER_COLOR = TFT_DARKGREY;


        
        // Drawing methods for different views
        static void drawFrame();
        static void drawTodoList();
        static void drawQuotes();
        static void drawWeather();
        static void drawStats();
        static void drawNextMeal();
        static void drawChakras();
        static void drawVirtues();
        static void drawRootChakra(int x, int y, int size);
        static void drawSacralChakra(int x, int y, int size);
        static void drawSolarChakra(int x, int y, int size);
        static void drawHeartChakra(int x, int y, int size);
        static void drawThroatChakra(int x, int y, int size);
        static void drawThirdEyeChakra(int x, int y, int size);
        static void drawCrownChakra(int x, int y, int size);
        static void drawChastitySymbol(int x, int y, int size);
        static void drawTemperanceSymbol(int x, int y, int size);
        static void drawCharitySymbol(int x, int y, int size);
        static void drawDiligenceSymbol(int x, int y, int size);
        static void drawForgivenessSymbol(int x, int y, int size);
        static void drawKindnessSymbol(int x, int y, int size);
        static void drawHumilitySymbol(int x, int y, int size);
        static void drawBatteryCharging(int x, int y, int size);
        static void drawBattery(int x, int y, int size);
        

        static void updateExperienceBar(const String& expStr);


        static uint32_t experience;
        static uint16_t experienceToNextLevel;
        static uint8_t level;
        static uint16_t calculateNextLevelExperience(uint8_t currentLevel);

        static void drawSymbol(const char* symbol, int x, int y, int size);
        static Notification currentNotification;
        static bool notificationActive;

        // Animation timing
        static unsigned long lastCounterUpdate;
        static const unsigned long COUNTER_SPEED = 1000;  // 1 second interval
        static unsigned long lastAnimationStep;
        static const unsigned long ANIMATION_DELAY = 250; // 250ms between steps
        static bool isAnimating;
        static int animationStep;
        static const int TOTAL_ANIMATION_STEPS = 14;

        static uint32_t roverExperience;
        static uint32_t roverExperienceToNextLevel;
        static uint8_t roverLevel;

        // Expression timing
        static unsigned long lastExpressionChange;
        static unsigned long nextExpressionInterval;
        static const unsigned long DEFAULT_EXPRESSION_INTERVAL = 60000; // 1 minute default

        static constexpr unsigned long WARNING_DURATION = 3000; // 3 seconds
        static unsigned long warningStartTime;
    };
}

#endif 
