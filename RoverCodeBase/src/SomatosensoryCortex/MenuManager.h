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

    /**
     * @brief Manages menu system and user interface interactions
     * 
     * Provides:
     * - Menu navigation and selection handling
     * - Dynamic menu structure management
     * - Visual feedback coordination
     * - Application state transitions
     * - User input processing
     * - Menu stack management for navigation history
     * 
     * The MenuManager serves as the cognitive interface between
     * user intentions and system responses, coordinating:
     * - Menu item organization and display
     * - Navigation state tracking
     * - Selection feedback
     * - Application switching
     * - Visual and audio responses
     */
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
        static const MenuItem& getCurrentItem();


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