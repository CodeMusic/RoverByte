#ifndef ROVER_VIEW_MANAGER_H
#define ROVER_VIEW_MANAGER_H

#include "TFT_eSPI.h"
#include "../PrefrontalCortex/utilities.h"
#include "ColorUtilities.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../AuditoryCortex/SoundFxManager.h"

// Forward declarations
extern TFT_eSprite spr;
extern TFT_eSPI tft;

class RoverViewManager {
public:
    enum ViewType {
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
    static void drawLoadingScreen();

private:
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
    static const int FRAME_Y = 195;
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
};

#endif 