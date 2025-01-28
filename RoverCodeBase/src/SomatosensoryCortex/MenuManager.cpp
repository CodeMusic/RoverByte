#include "MenuManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../GameCortex/AppManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../GameCortex/AppManager.h"

using namespace PrefrontalCortex;  // For Utilities
using namespace GameCortex;  // For AppManager

namespace SomatosensoryCortex 
    {
    // Example: we still keep IR/LED references here as needed
    // but major interactions launch their respective apps.

    bool MenuManager::isMenuVisible = false;
    std::vector<MenuItem> MenuManager::currentMenu;
    std::vector<MenuItem> MenuManager::mainMenu;
    std::vector<std::vector<MenuItem>*> MenuManager::menuStack;
    int MenuManager::selectedIndex = 0;

    // Used elsewhere, so keep it
    bool isRoverRadio = true;

    int MenuManager::getSelectedIndex() {
        return selectedIndex;
    }

    // Define LED Modes submenu
    std::vector<MenuItem> MenuManager::ledModesMenu = {
        MenuItem("Off", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::OFF_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("LED Mode set to Off");
        }),
        MenuItem("Encodings", []() 
        {
            PrefrontalCortex::Utilities::LOG_DEBUG("Entering Encodings menu");
        }),
        MenuItem("Festive", []() 
        {
            PrefrontalCortex::Utilities::LOG_DEBUG("Entering Festive Modes menu");
        }),
        MenuItem("Rover Emotions", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::ROVER_EMOTION_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("LED Mode set to Rover Emotions");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define Encoding Modes submenu
    std::vector<MenuItem> MenuManager::encodingModesMenu = {
        MenuItem("Full Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::FULL_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Full Mode");
        }),
        MenuItem("Week Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::WEEK_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Week Mode");
        }),
        MenuItem("Timer Mode", []() 
        {
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::TIMER_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Timer Mode");
        }),
        MenuItem("Custom Mode", []() 
        {
            VisualCortex::LEDManager::setMode(VisualCortex::Mode::ENCODING_MODE);
            VisualCortex::LEDManager::setEncodingMode(VisualCortex::EncodingModes::CUSTOM_MODE);
            PrefrontalCortex::Utilities::LOG_DEBUG("Encoding Mode set to Custom Mode");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define Festive Modes submenu with all options
    std::vector<MenuItem> MenuManager::festiveModesMenu = 
    {
        MenuItem("New Year", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::NEW_YEAR);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to New Year");
        }),
        MenuItem("Valentines", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::VALENTINES);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Valentines");
        }),
        MenuItem("St. Patrick", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::ST_PATRICK);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to St. Patrick");
        }),
        MenuItem("Easter", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::EASTER);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Easter");
        }),
        MenuItem("Canada Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CANADA_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Canada Day");
        }),
        MenuItem("Halloween", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::HALLOWEEN);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Halloween");
        }),
        MenuItem("Christmas", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CHRISTMAS);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Christmas");
        }),
        MenuItem("Thanksgiving", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::THANKSGIVING);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Thanksgiving");
        }),
        MenuItem("Independence Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::INDEPENDENCE_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Independence Day");
        }),
        MenuItem("Diwali", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::DIWALI);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Diwali");
        }),
        MenuItem("Ramadan", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::RAMADAN);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Ramadan");
        }),
        MenuItem("Chinese New Year", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::CHINESE_NEW_YEAR);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Chinese New Year");
        }),
        MenuItem("Mardi Gras", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::MARDI_GRAS);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Mardi Gras");
        }),
        MenuItem("Labor Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::LABOR_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Labor Day");
        }),
        MenuItem("Memorial Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::MEMORIAL_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Memorial Day");
        }),
        MenuItem("Flag Day", []() 
        {
            VisualCortex::LEDManager::setFestiveTheme(VisualCortex::FestiveTheme::FLAG_DAY);
            PrefrontalCortex::Utilities::LOG_DEBUG("Festive Theme set to Flag Day");
        }),
        MenuItem("Back", []() {
            MenuManager::goBack();
        })
    };

    // Define App Settings submenu
    std::vector<MenuItem> MenuManager::appSettingsMenu = {
        {"Change LED Mode", [&]() {
            MenuManager::enterSubmenu(MenuManager::ledModesMenu);
        }},
        {"Adjust Volume", []() {
            // Logic to adjust volume
            AuditoryCortex::SoundFxManager::adjustVolume(20);
            AuditoryCortex::SoundFxManager::playVoiceLine("volume_up", 0);
        }},
        
        {"Set Time", []() {
            // Logic to set time
            //AppManager::startApp("SetTimeApp");
            VisualCortex::RoverManager::setTemporaryExpression(
                VisualCortex::RoverManager::LOOKING_DOWN, 
                1000
            );
        }},
        {"Back", []() {
            MenuManager::goBack();
        }}
    };

    void MenuManager::init() {
        // Main menu items with explicit constructor calls
        mainMenu.push_back(MenuItem(
            "Slots App", []() {
                GameCortex::AppManager::startApp("SlotsApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "IR Blast App", []() {
                AppManager::startApp("IrBlastApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "NFC App", []() {
                AppManager::startApp("NfcApp");
                hide();
            }
        ));

        mainMenu.push_back(MenuItem(
            "App Settings", [&]() {
                MenuManager::enterSubmenu(MenuManager::ledModesMenu);
            }
        ));

        // Assign the main menu as our initial menu
        currentMenu = mainMenu;
    }

    void MenuManager::show() {
        isMenuVisible = true;
        VisualCortex::RoverViewManager::drawMenuBackground();
        drawMenu();
    }

    void MenuManager::hide() {
        isMenuVisible = false;
        selectedIndex = 0;
        currentMenu = mainMenu;
        menuStack.clear();
        VisualCortex::RoverViewManager::drawCurrentView();
    }

    void MenuManager::drawMenu() {
        const char* title = "Menu";
        VisualCortex::RoverViewManager::drawFullScreenMenu(title, currentMenu, selectedIndex);
    }

    void MenuManager::handleRotaryTurn(int direction) {
        if (!isMenuVisible || currentMenu.empty()) {
            // If menu is not visible, show it (optional behavior)
            show();
            return;
        }

        if (direction > 0) {
            selectedIndex = (selectedIndex - 1 + currentMenu.size()) % currentMenu.size();
        } else {
            selectedIndex = (selectedIndex + 1) % currentMenu.size();
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
            drawMenu();
        }
    }

    // If you still want a separate IR submenu for advanced IR control, you can place it here
    void MenuManager::handleIRBlastMenu() {
        Utilities::LOG_DEBUG("handleIRBlastMenu......");
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

    void MenuManager::enterSubmenu(const std::vector<MenuItem>& submenu) {
        menuStack.push_back(&currentMenu);
        currentMenu = submenu;
        selectedIndex = 0;
        drawMenu();
    }

    void MenuManager::selectMenuItem() {
        if (currentMenu.empty()) return; // No items to select

        // Execute the action associated with the selected menu item
        currentMenu[selectedIndex].action();
    }
}