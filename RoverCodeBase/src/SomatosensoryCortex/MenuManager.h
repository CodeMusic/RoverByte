#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <vector>
#include <functional>
#include <string>
#include "PrefrontalCortex/ProtoPerceptions.h"
#include "CorpusCallosum/SynapticPathways.h"

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
        /**
         * @brief Initialize the menu
         */
        static void init();
        /**
         * @brief Show the menu
         */
        static void show();
        /**
         * @brief Hide the menu
         */
        static void hide();
        /**
         * @brief Check if menu system is currently visible
         * @return True if menu is being displayed
         */
        static bool isVisible() { return isMenuVisible; }
        
        // Input handling methods
        /**
         * @brief Handle the rotary turn
         * @param direction The direction of the turn
         */
        static void handleRotaryTurn(int direction);
        /**
         * @brief Handle the menu select
         */
        static void handleMenuSelect();
        
        // Menu navigation methods
        /**
         * @brief Enter the submenu
         * @param submenu The submenu to enter
         */
        static void enterSubmenu(const std::vector<MenuItem>& submenu);
        /**
         * @brief Go back
         */
        static void goBack();
        /**
         * @brief Get the selected index
         * @return The selected index
         */
        static int getSelectedIndex();
        /**
         * @brief Get the current item
         */
        static const MenuItem& getCurrentItem();

        // Menu rendering methods   
        /**
         * @brief Draw the menu
         */
        static void drawMenu();
        /**
         * @brief Handle the IR blast menu
         */
        // Specialized menu handlers
        static void handleIRBlastMenu();
        /**
         * @brief Select the menu item
         */
        static void selectMenuItem();

        // Menu structure definitions
        /**
         * @brief The app settings menu
         */
        static std::vector<MenuItem> appSettingsMenu;
        /**
         * @brief The led modes menu
         */
        static std::vector<MenuItem> ledModesMenu;
        /**
         * @brief The encoding modes menu
         */
        static std::vector<MenuItem> encodingModesMenu;
        /**
         * @brief The festive modes menu
         */
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