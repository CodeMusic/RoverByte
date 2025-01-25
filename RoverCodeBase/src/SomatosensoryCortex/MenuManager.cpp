#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../SomatosensoryCortex/AppManager.h"

// Example: we still keep IR/LED references here as needed
// but major interactions launch their respective apps.

bool MenuManager::isMenuVisible = false;
std::vector<MenuItem> MenuManager::currentMenu;
std::vector<MenuItem> MenuManager::mainMenu;
std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
int MenuManager::selectedIndex = 0;

// Used elsewhere, so keep it
bool isRoverRadio = true;

void MenuManager::init() {
    // Main menu items. Each one starts an "app" by calling AppManager::startApp(...).
    // "Slots" is now a separate app.
    mainMenu.push_back({
        "Slots App", []() {
            AppManager::startApp("SlotsApp");
            hide();
        }
    });

    // "IR Blast" is another app (instead of the old IR submenu).
    mainMenu.push_back({
        "IR Blast App", []() {
            AppManager::startApp("IrBlastApp");
            hide();
        }
    });

    // "NFC" can also be an app if you'd like direct scanning within an app environment.
    mainMenu.push_back({
        "NFC App", []() {
            AppManager::startApp("NfcApp");
            hide();
        }
    });

    // "App Settings" could hold LED mode config
    mainMenu.push_back({
        "App Settings", []() {
            AppManager::startApp("AppSettings");
            hide();
        }
    });

    // Assign the main menu as our initial menu
    currentMenu = mainMenu;
}

void MenuManager::show() {
    isMenuVisible = true;
    RoverViewManager::drawMenuBackground();
    drawMenu();
}

void MenuManager::hide() {
    isMenuVisible = false;
    selectedIndex = 0;
    currentMenu = mainMenu;
    menuStack.clear();
    RoverViewManager::drawCurrentView();
}

void MenuManager::drawMenu() {
    const char* title = menuStack.empty() ? "MENU" : currentMenu[selectedIndex].name.c_str();
    RoverViewManager::drawFullScreenMenu(title, currentMenu, selectedIndex);
}

void MenuManager::handleRotaryTurn(int direction) {
    if (!isMenuVisible || currentMenu.empty()) {
        // If menu is not visible, show it (optional behavior)
        show();
        return;
    }

    if (direction > 0) {
        selectedIndex = (selectedIndex + 1) % currentMenu.size();
    } else {
        selectedIndex = (selectedIndex - 1 + currentMenu.size()) % currentMenu.size();
    }
    drawMenu();
}

void MenuManager::handleMenuSelect() {
    if (!isMenuVisible || currentMenu.empty()) {
        show();
        return;
    }

    if (selectedIndex >= (int)currentMenu.size()) {
        selectedIndex = 0;
        return;
    }

    MenuItem& selected = currentMenu[selectedIndex];

    // If there are subitems, descend into them
    if (!selected.subItems.empty()) {
        menuStack.push_back(&currentMenu);
        currentMenu = selected.subItems;
        selectedIndex = 0;
        drawMenu();
        return;
    }

    // Otherwise, call the action (which may start an app)
    if (selected.action) {
        selected.action();
        // Typically we might hide the menu after running an action
        // or re-draw it if we want to remain visible
        drawMenu();
    }
}

// If you still want a separate IR submenu for advanced IR control, you can place it here
void MenuManager::handleIRBlastMenu() {
    // Example placeholder. Could call AppManager::startApp("IrBlastApp") 
    // or handle advanced IR menus here.
}

// Helper for going back in the submenu stack
void MenuManager::goBack() {
    if (!menuStack.empty()) {
        currentMenu = *menuStack.back();
        menuStack.pop_back();
        selectedIndex = 0;
    }
    if (!currentMenu.empty()) {
        drawMenu();
    } else {
        isMenuVisible = false;
    }
}