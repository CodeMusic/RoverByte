#pragma once
#include <vector>
#include <functional>
#include <string>

struct MenuItem {
    std::string name;
    std::vector<MenuItem> subItems;
    std::function<void()> action;
    
    // Constructor for menu items with submenus
    MenuItem(const std::string& n, const std::vector<MenuItem>& items) 
        : name(n), subItems(items), action(nullptr) {}
    
    // Constructor for action items
    MenuItem(const std::string& n, std::function<void()> act) 
        : name(n), action(act) {}

    // Modified equality operator that doesn't compare actions
    bool operator==(const MenuItem& other) const {
        return (name == other.name && 
                subItems == other.subItems);
    }
};

class MenuManager {
public:
    static void init();
    static void show();
    static void hide();
    static bool isVisible() { return isMenuVisible; }
    static void handleRotaryTurn(int direction);
    static void handleRotaryPress();
    static void drawMenu();
    static void enterSubmenu(const std::vector<MenuItem>& submenu);
    static void handleIRBlastMenu();
    
private:
    static bool isMenuVisible;
    static std::vector<MenuItem> currentMenu;
    static std::vector<MenuItem> mainMenu;
    static std::vector<std::vector<MenuItem>*> menuStack;
    static int selectedIndex;
    
    static void goBack();
    static const unsigned long DEBOUNCE_DELAY = 50;
}; 