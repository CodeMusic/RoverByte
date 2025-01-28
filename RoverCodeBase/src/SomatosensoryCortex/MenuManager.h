#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <vector>
#include <functional>
#include <string>

namespace SomatosensoryCortex 
{
    struct MenuItem 
    {
        std::string name;
        std::function<void()> action;
        std::vector<MenuItem> subItems;

        MenuItem(const std::string& n, const std::vector<MenuItem>& items) 
            : name(n), 
              action(nullptr),
              subItems(items)
        {
        }

        MenuItem(const std::string& n, std::function<void()> act) 
            : name(n), 
              action(act),
              subItems()
        {
        }

        bool operator==(const MenuItem& other) const 
        {
            return (name == other.name && 
                    subItems == other.subItems);
        }
    };

    class MenuManager 
    {
    public:
        static void init();
        static void show();
        static void hide();
        static bool isVisible() { return isMenuVisible; }
        static void handleRotaryTurn(int direction);
        static void handleMenuSelect();
        static void drawMenu();
        static void enterSubmenu(const std::vector<MenuItem>& submenu);
        static void handleIRBlastMenu();
        static void selectMenuItem();
        static void goBack();
        static std::vector<MenuItem> appSettingsMenu;
        static std::vector<MenuItem> ledModesMenu;
        static std::vector<MenuItem> encodingModesMenu;
        static std::vector<MenuItem> festiveModesMenu;
        static int getSelectedIndex();

    private:
        static std::vector<MenuItem> currentMenu;
        static std::vector<MenuItem> mainMenu;
        static std::vector<std::vector<MenuItem>*> menuStack;
        static int selectedIndex;
        static bool isMenuVisible;
        static const unsigned long DEBOUNCE_DELAY = 50;
    }; 
}

#endif // MENU_MANAGER_H