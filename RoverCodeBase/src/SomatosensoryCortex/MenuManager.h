#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <vector>
#include <functional>
#include <string>
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace SomatosensoryCortex 
{
    using namespace CorpusCallosum;
    using PC::MenuTypes::MenuItem;  // Add this after creating it in ProtoPerceptions

    class MenuManager 
    {
    public:
        // Core menu perception methods
        static void init();
        static void show();
        static void hide();
        static bool isVisible() { return isMenuVisible; }
        
        // Input handling methods
        static void handleRotaryTurn(int direction);
        static void handleMenuSelect();
        
        // Menu navigation methods
        static void enterSubmenu(const std::vector<MenuItem>& submenu);
        static void goBack();
        static int getSelectedIndex();

        // Menu rendering methods
        static void drawMenu();
        
        // Specialized menu handlers
        static void handleIRBlastMenu();
        static void selectMenuItem();

        // Menu structure definitions
        static std::vector<MenuItem> appSettingsMenu;
        static std::vector<MenuItem> ledModesMenu;
        static std::vector<MenuItem> encodingModesMenu;
        static std::vector<MenuItem> festiveModesMenu;

    private:
        // Menu state tracking
        static std::vector<MenuItem> currentMenu;
        static std::vector<MenuItem> mainMenu;
        static std::vector<std::vector<MenuItem>*> menuStack;
        static int selectedIndex;
        static bool isMenuVisible;
        
        // Timing constants
        static constexpr unsigned long DEBOUNCE_DELAY = 50;
    }; 
}

#endif // MENU_MANAGER_H