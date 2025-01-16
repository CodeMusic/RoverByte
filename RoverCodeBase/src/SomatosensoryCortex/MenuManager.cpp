#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"

bool MenuManager::isMenuVisible = false;
std::vector<MenuItem> MenuManager::currentMenu;
std::vector<MenuItem> MenuManager::mainMenu;
std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
int MenuManager::selectedIndex = 0;

bool isRoverRadio = true;

void MenuManager::init() {
    // Define LED submenu with Off as last option
    std::vector<MenuItem> ledSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Full Mode", []() { LEDManager::setMode(Mode::FULL_MODE); }),
        MenuItem("Week Mode", []() { LEDManager::setMode(Mode::WEEK_MODE); }),
        MenuItem("Timer Mode", []() { LEDManager::setMode(Mode::TIMER_MODE); }),
        MenuItem("Festive", []() { LEDManager::setMode(Mode::FESTIVE_MODE); }),
        MenuItem("Off", []() { LEDManager::setMode(Mode::OFF_MODE); })
    };
    
    // Define Festive submenu with all themes
    std::vector<MenuItem> festiveSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("New Year", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::NEW_YEAR); 
        }),
        MenuItem("Valentine's", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::VALENTINES); 
        }),
        MenuItem("St. Patrick's", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::ST_PATRICK); 
        }),
        MenuItem("Easter", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::EASTER); 
        }),
        MenuItem("Canada Day", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::CANADA_DAY); 
        }),
        MenuItem("Halloween", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::HALLOWEEN); 
        }),
        MenuItem("Christmas", []() { 
            LEDManager::setMode(Mode::FESTIVE_MODE);
            LEDManager::setFestiveTheme(FestiveTheme::CHRISTMAS); 
        })
    };
    
    // Add festive submenu to LED submenu
    ledSubmenu[4] = MenuItem("Festive", festiveSubmenu);
    
    // Define Feed submenu
    std::vector<MenuItem> feedSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Treat", []() { /* TODO */ }),
        MenuItem("Meal", []() { /* TODO */ }),
        MenuItem("Snack", []() { /* TODO */ })
    };

    std::vector<MenuItem> gamesSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Slots", []() { 
            LEDManager::setMode(Mode::OFF_MODE);
            FastLED.clear();
            FastLED.show();
            SlotsManager::startGame();
            MenuManager::hide();
        })
    };


    // Define empty submenus with proper vector initialization
    std::vector<MenuItem> careSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Pet", []() { MenuManager::goBack(); }),
        MenuItem("Games", gamesSubmenu),
        MenuItem("Walk", []() { MenuManager::goBack(); })
    };

    std::vector<MenuItem> cleanSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); })
    };
    
    // Define system submenu
    std::vector<MenuItem> systemSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Reboot", []() { ESP.restart(); })
    };
    
    // Define mischief submenu
    std::vector<MenuItem> mischiefSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("NFC Scan", []() { 
            LEDManager::setMode(Mode::OFF_MODE);
            FastLED.clear();
            FastLED.show();
            NFCManager::handleNFCScan();
            MenuManager::hide();
        }),
        MenuItem("IR Blast", []() { 
            LEDManager::setMode(Mode::OFF_MODE);
            FastLED.clear();
            FastLED.show();
            IRManager::startBlast();
            MenuManager::hide();
        }),
        MenuItem("SubGhz", []() { 
            MenuManager::goBack();
        })
    };
    
    // Define main menu
    if (isRoverRadio) {
        mainMenu = std::vector<MenuItem>{
            MenuItem("BACK", []() { MenuManager::hide(); }),
            MenuItem("LED Mode", ledSubmenu),
            MenuItem("Feed", feedSubmenu),
            MenuItem("Care", careSubmenu),
            MenuItem("Clean", cleanSubmenu),
            MenuItem("Mischief", mischiefSubmenu),
            MenuItem("System", systemSubmenu)
        };
    } else {
        mainMenu = std::vector<MenuItem>{
            MenuItem("BACK", []() { MenuManager::hide(); }),
            MenuItem("LED Mode", ledSubmenu),
            MenuItem("Feed", feedSubmenu),
            MenuItem("Care", careSubmenu),
            MenuItem("Clean", cleanSubmenu),
            MenuItem("System", systemSubmenu)
        };
    }
    currentMenu = mainMenu;
}

void MenuManager::show() {
    isMenuVisible = true;  // Set this first
    selectedIndex = 0;
    currentMenu = mainMenu;
    menuStack.clear();
    
    // Safety check
    if (!currentMenu.empty()) {
        drawMenu();
    } else {
        isMenuVisible = false;  // Only revert if menu is empty
    }
}

void MenuManager::hide() {
    isMenuVisible = false;
    selectedIndex = 0;
    currentMenu = mainMenu;
    menuStack.clear();
}

void MenuManager::drawMenu() {
    const char* title = menuStack.empty() ? "MENU" : currentMenu[selectedIndex].name.c_str();
    RoverViewManager::drawFullScreenMenu(
        title,
        currentMenu,
        selectedIndex
    );
}

void MenuManager::handleRotaryTurn(int direction) {
    if (direction > 0) {
        selectedIndex = (selectedIndex + 1) % currentMenu.size();
    } else {
        selectedIndex = (selectedIndex - 1 + currentMenu.size()) % currentMenu.size();
    }
    drawMenu();
}

void MenuManager::handleRotaryPress() {
    if (!isMenuVisible || currentMenu.empty()) {
        show();  // Show menu if it's not visible
        return;
    }
    
    if (selectedIndex >= currentMenu.size()) {
        selectedIndex = 0;
        return;
    }
    
    MenuItem& selected = currentMenu[selectedIndex];
    
    // Handle submenu navigation first
    if (!selected.subItems.empty()) {
        menuStack.push_back(&currentMenu);
        currentMenu = selected.subItems;
        selectedIndex = 0;
        drawMenu();
        return;  // Exit after handling submenu navigation
    }
    
    // Handle action if there's no submenu
    if (selected.action) {
        selected.action();
        drawMenu();
    }
}

void MenuManager::goBack() {
    if (!menuStack.empty()) {
        // We're in a submenu
        currentMenu = *menuStack.back();
        menuStack.pop_back();
        selectedIndex = 0;
        drawMenu();  // Redraw the parent menu
    } else if (currentMenu == mainMenu) {
        // We're in the main menu
        hide();
        RoverViewManager::drawCurrentView();
    } else {
        // Failsafe: if we somehow get in an invalid state
        currentMenu = mainMenu;
        selectedIndex = 0;
        menuStack.clear();
        drawMenu();
    }
}

void MenuManager::enterSubmenu(const std::vector<MenuItem>& submenu) {
    menuStack.push_back(&currentMenu);
    currentMenu = submenu;
    selectedIndex = 0;
    drawMenu();
}

void MenuManager::handleIRBlastMenu() {
    if (IRManager::isBlasting()) {
        IRManager::stopBlast();
    } else {
        IRManager::startBlast();
    }
}