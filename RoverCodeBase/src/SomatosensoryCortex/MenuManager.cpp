#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"

bool MenuManager::isMenuVisible = false;
std::vector<MenuItem> MenuManager::currentMenu;
std::vector<MenuItem> MenuManager::mainMenu;
std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
int MenuManager::selectedIndex = 0;

void MenuManager::init() {
    // Define LED submenu
    std::vector<MenuItem> ledSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Full Mode", []() { LEDManager::setMode(Mode::FULL_MODE); }),
        MenuItem("Week Mode", []() { LEDManager::setMode(Mode::WEEK_MODE); }),
        MenuItem("Timer Mode", []() { LEDManager::setMode(Mode::TIMER_MODE); })
    };
    
    // Define Feed submenu
    std::vector<MenuItem> feedSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Treat", []() { /* TODO */ }),
        MenuItem("Meal", []() { /* TODO */ }),
        MenuItem("Snack", []() { /* TODO */ })
    };

    // Define empty submenus with proper vector initialization
    std::vector<MenuItem> careSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); })
    };

    std::vector<MenuItem> cleanSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); })
    };
    
    // Define system submenu
    std::vector<MenuItem> systemSubmenu = {
        MenuItem("BACK", []() { MenuManager::goBack(); }),
        MenuItem("Reboot", []() { ESP.restart(); })
    };
    
    // Define main menu
    mainMenu = std::vector<MenuItem>{
        MenuItem("BACK", []() { MenuManager::hide(); }),
        MenuItem("LED Mode", ledSubmenu),
        MenuItem("Feed", feedSubmenu),
        MenuItem("Care", careSubmenu),
        MenuItem("Clean", cleanSubmenu),
        MenuItem("System", systemSubmenu)
    };
    
    currentMenu = mainMenu;
}

void MenuManager::show() {
    isMenuVisible = true;
    selectedIndex = 0;
    currentMenu = mainMenu;
    menuStack.clear();
    drawMenu();
}

void MenuManager::hide() {
    isMenuVisible = false;
    selectedIndex = 0;
    currentMenu = mainMenu;
    menuStack.clear();
}

void MenuManager::drawMenu() {
    RoverViewManager::drawFullScreenMenu(
        "MENU",
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
    if (!isMenuVisible || selectedIndex >= currentMenu.size()) return;
    
    MenuItem& selected = currentMenu[selectedIndex];
    
    if (!selected.subItems.empty()) {
        // Enter submenu without dynamic allocation
        menuStack.push_back(&currentMenu);
        currentMenu = selected.subItems;
        selectedIndex = 0;
        drawMenu();
    } else if (selected.action) {
        // Execute action and redraw
        selected.action();
        drawMenu();
    }
}

void MenuManager::goBack() {
    if (!menuStack.empty()) {
        currentMenu = *menuStack.back();
        menuStack.pop_back();
        selectedIndex = 0;
        drawMenu();
    } else {
        hide();
        RoverViewManager::drawCurrentView();
    }
}

void MenuManager::enterSubmenu(const std::vector<MenuItem>& submenu) {
    menuStack.push_back(&currentMenu);
    currentMenu = submenu;
    selectedIndex = 0;
    drawMenu();
}