/**
 * @brief MenuManager implementation
 * 
 * Handles:
 * - Menu system initialization
 * - Navigation processing
 * - Selection management
 * - Visual feedback coordination
 * - Application state transitions
 */

#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../GameCortex/AppManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/Utilities.h"

namespace SomatosensoryCortex 
{
    namespace SC = SomatosensoryCortex;  // Add namespace alias
    using namespace CorpusCallosum;
    
    // Import specific types we need
    using PC::Utilities;
    using GC::AppManager;
    using VC::LEDManager;
    using VC::RoverViewManager;
    using VC::RoverManager;
    using AC::SoundFxManager;
    using PC::MenuTypes::MenuItem;

    // Initialize static members
    bool MenuManager::isMenuVisible = false;
    std::vector<MenuItem> MenuManager::currentMenu;
    std::vector<MenuItem> MenuManager::mainMenu;
    std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
    int MenuManager::selectedIndex = 0;
    bool MenuManager::initialized = false;
    // Used elsewhere, so keep it
    bool isRoverRadio = true;


    // Define LED Modes submenu with explicit constructors
    std::vector<MenuItem> MenuManager::ledModesMenu = 
    {
        MenuItem("Off", []() 
        {
            LEDManager::setMode(PC::VisualTypes::VisualMode::OFF_MODE);
            Utilities::LOG_DEBUG("LED Mode set to Off");
        }),
        MenuItem("Encodings", []() 
        {
            MenuManager::enterSubmenu(MenuManager::encodingModesMenu);
            Utilities::LOG_DEBUG("Entering Encodings menu");
        }),
        MenuItem("Festive", []() 
        {
            MenuManager::enterSubmenu(MenuManager::festiveModesMenu);
            Utilities::LOG_DEBUG("Entering Festive Modes menu");
        }),
        MenuItem("Rover Emotions", []() 
        {
            LEDManager::setMode(PC::VisualTypes::VisualMode::ROVER_EMOTION_MODE);
            Utilities::LOG_DEBUG("LED Mode set to Rover Emotions");
        }),
        MenuItem("Back", []() 
        {
            MenuManager::goBack();
        })
    };

    // Define Encoding Modes submenu
    std::vector<MenuItem> MenuManager::encodingModesMenu = 
    {
        MenuItem("Full Mode", []() 
        {
            LEDManager::setEncodingMode(VC::EncodingModes::FULL_MODE);
            Utilities::LOG_DEBUG("Encoding Mode set to Full Mode");
        }),
        MenuItem("Week Mode", []() 
        {
            LEDManager::setEncodingMode(VC::EncodingModes::WEEK_MODE);
            Utilities::LOG_DEBUG("Encoding Mode set to Week Mode");
        }),
        MenuItem("Timer Mode", []() 
        {
            LEDManager::setEncodingMode(VC::EncodingModes::TIMER_MODE);
            Utilities::LOG_DEBUG("Encoding Mode set to Timer Mode");
        }),
        MenuItem("Custom Mode", []() 
        {
            LEDManager::setMode(PC::VisualTypes::VisualMode::ENCODING_MODE);
            LEDManager::setEncodingMode(VC::EncodingModes::CUSTOM_MODE);
            Utilities::LOG_DEBUG("Encoding Mode set to Custom Mode");
        }),
        MenuItem("Back", []() 
        {
            MenuManager::goBack();
        })
    };

    // Define Festive Modes submenu with all options
    std::vector<MenuItem> MenuManager::festiveModesMenu = 
    {
        MenuItem("New Year", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::NEW_YEAR);
            Utilities::LOG_DEBUG("Festive Theme set to New Year");
        }),
        MenuItem("Valentines", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::VALENTINES);
            Utilities::LOG_DEBUG("Festive Theme set to Valentines");
        }),
        MenuItem("St. Patrick", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::ST_PATRICK);
            Utilities::LOG_DEBUG("Festive Theme set to St. Patrick");
        }),
        MenuItem("Easter", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::EASTER);
            Utilities::LOG_DEBUG("Festive Theme set to Easter");
        }),
        MenuItem("Canada Day", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::CANADA_DAY);
            Utilities::LOG_DEBUG("Festive Theme set to Canada Day");
        }),
        MenuItem("Halloween", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::HALLOWEEN);
            Utilities::LOG_DEBUG("Festive Theme set to Halloween");
        }),
        MenuItem("Christmas", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::CHRISTMAS);
            Utilities::LOG_DEBUG("Festive Theme set to Christmas");
        }),
        MenuItem("Thanksgiving", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::THANKSGIVING);
            Utilities::LOG_DEBUG("Festive Theme set to Thanksgiving");
        }),
        MenuItem("Independence Day", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::INDEPENDENCE_DAY);
            Utilities::LOG_DEBUG("Festive Theme set to Independence Day");
        }),
        MenuItem("Diwali", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::DIWALI);
            Utilities::LOG_DEBUG("Festive Theme set to Diwali");
        }),
        MenuItem("Ramadan", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::RAMADAN);
            Utilities::LOG_DEBUG("Festive Theme set to Ramadan");
        }),
        MenuItem("Chinese New Year", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::CHINESE_NEW_YEAR);
            Utilities::LOG_DEBUG("Festive Theme set to Chinese New Year");
        }),
        MenuItem("Mardi Gras", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::MARDI_GRAS);
            Utilities::LOG_DEBUG("Festive Theme set to Mardi Gras");
        }),
        MenuItem("Labor Day", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::LABOR_DAY);
            Utilities::LOG_DEBUG("Festive Theme set to Labor Day");
        }),
        MenuItem("Memorial Day", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::MEMORIAL_DAY);
            Utilities::LOG_DEBUG("Festive Theme set to Memorial Day");
        }),
        MenuItem("Flag Day", []() 
        {
            LEDManager::setFestiveTheme(VC::FestiveTheme::FLAG_DAY);
            Utilities::LOG_DEBUG("Festive Theme set to Flag Day");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define App Settings submenu
    std::vector<MenuItem> MenuManager::appSettingsMenu = 
    {
        MenuItem("Change LED Mode", []() 
        {
            MenuManager::enterSubmenu(MenuManager::ledModesMenu);
        }),
        MenuItem("Adjust Volume", []() 
        {
            SoundFxManager::adjustVolume(20);
            SoundFxManager::playVoiceLine("volume_up", 0);
        }),
        MenuItem("Set Time", []() 
        {
            RoverManager::setTemporaryExpression(
                PC::Expression::LOOKING_DOWN, 
                1000
            );
        }),
        MenuItem("Back", []() 
        {
            MenuManager::goBack();
        })
    };

    /**
     * @brief Initialize menu system and default items
     * Sets up main menu structure and initial state
     */
    void MenuManager::init() 
    {
        mainMenu = 
        {
            MenuItem("Slots App", []() 
            {
                AppManager::startApp("SlotsApp");
                hide();
            }),
            MenuItem("IR Blast App", []() 
            {
                AppManager::startApp("IrBlastApp");
                hide();
            }),
            MenuItem("NFC App", []() 
            {
                AppManager::startApp("NfcApp");
                hide();
            }),
            MenuItem("App Settings", []() 
            {
                MenuManager::enterSubmenu(MenuManager::appSettingsMenu);
            })
        };

        currentMenu = mainMenu;
        initialized = true;
    }

    /**
     * @brief Update menu display and state
     * Handles visual updates and state maintenance
     */
    void MenuManager::show() 
    {
        if (!initialized) {
            return;
        }
        isMenuVisible = true;
        RoverViewManager::drawMenuBackground();
        drawMenu();
    }

    void MenuManager::hide() 
    {
        if (!initialized) {
            return;
        }
        isMenuVisible = false;
        selectedIndex = 0;
        currentMenu = mainMenu;
        menuStack.clear();
        RoverViewManager::drawCurrentView();
    }

    void MenuManager::drawMenu() 
    {
        const char* title = "Menu";
        RoverViewManager::drawFullScreenMenu(title, currentMenu, selectedIndex);
    }

    /**
     * @brief Process menu navigation input
     * @param direction Rotation direction (+/-)
     * Updates selection and provides feedback
     */
    void MenuManager::handleRotaryTurn(int direction) 
    {
        if (!initialized) {
            return;
        }
        if (!isMenuVisible) return;
        
        selectedIndex += direction;
        
        if (selectedIndex < 0) 
        {
            selectedIndex = currentMenu.size() - 1;
        }
        else if (selectedIndex >= static_cast<int>(currentMenu.size())) 
        {
            selectedIndex = 0;
        }
        
        drawMenu();
    }

    /**
     * @brief Handle menu item selection
     * Processes selection and triggers appropriate actions
     */
    void MenuManager::handleMenuSelect() 
    {
        if (!initialized) {
            return;
        }
        if (!isMenuVisible || selectedIndex >= static_cast<int>(currentMenu.size())) 
        {
            return;
        }

        if (currentMenu[selectedIndex].action) 
        {
            currentMenu[selectedIndex].action();
        }
        
        drawMenu();
    }

    // If you still want a separate IR submenu for advanced IR control, you can place it here
    void MenuManager::handleIRBlastMenu() {
        if (!initialized) {
            return;
        }
        Utilities::LOG_DEBUG("handleIRBlastMenu......");
        // Example placeholder. Could call AppManager::startApp("IrBlastApp") 
        // or handle advanced IR menus here.
    }

    /**
     * @brief Push new menu onto navigation stack
     * @param menu Pointer to menu item vector
     * Manages menu hierarchy for navigation
     */
    void MenuManager::enterSubmenu(const std::vector<MenuItem>& submenu) 
    {
        if (!initialized) {
            return;
        }
        menuStack.push_back(&currentMenu);
        currentMenu = submenu;
        selectedIndex = 0;
        drawMenu();
    }

    /**
     * @brief Pop current menu from navigation stack
     * Returns to previous menu level
     * @return True if menu was popped successfully
     */
    void MenuManager::goBack() 
    {
        if (!initialized) {
            return;
        }
        if (!menuStack.empty()) 
        {
            currentMenu = *menuStack.back();
            menuStack.pop_back();
            selectedIndex = 0;
            drawMenu();
        }
        else 
        {
            hide();
        }
    }

    void MenuManager::selectMenuItem() {
        if (!initialized) {
            return;
        }
        if (currentMenu.empty()) return; // No items to select

        // Execute the action associated with the selected menu item
        currentMenu[selectedIndex].action();
    }

    /**
     * @brief Get currently selected menu item
     * @return Reference to current MenuItem
     */
    const MenuItem& MenuManager::getCurrentItem() 
    {
        if (!initialized) {
            return currentMenu[0];
        }
        return currentMenu[selectedIndex];
    }

    int MenuManager::getSelectedIndex() 
    {
        if (!initialized) {
            return 0;
        }
        return selectedIndex;
    }

}