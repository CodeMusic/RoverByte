/**
 * @file AppRegistration.cpp
 * @brief Implementation of the AppRegistration class for cognitive application management
 * 
 * Handles the implementation of application callbacks and registration processes,
 * coordinating cross-cortex communication for entertainment and engagement systems.
 */

#include "AppRegistration.h"
#include "AppManager.h"
#include "GameCortex/SlotsManager.h"
#include "PsychicCortex/NFCManager.h"
#include "PsychicCortex/IRManager.h"
#include "VisualCortex/LEDManager.h"
#include "SomatosensoryCortex/MenuManager.h"
#include "PrefrontalCortex/Utilities.h"
#include <Arduino.h>

using namespace PsychicCortex;      // For IRManager, NFCManager
using namespace SomatosensoryCortex; // For MenuManager
using namespace PrefrontalCortex;    // For Utilities

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::Utilities;
    using PC::GameTypes::AppInfo;
    using VC::LEDManager;
    using SC::MenuManager;
    using PSY::NFCManager;
    using PSY::IRManager;

    void AppRegistration::slotsOnRun() {
        Serial.println("[SlotsApp] onRun: Starting Slots game...");
        SlotsManager::init();
    }
    void AppRegistration::slotsOnUpdate() {
        // Called every cycle while SlotsApp is RUNNING
        if (!SlotsManager::isGameActive()) {
            Serial.println("[SlotsApp] No active Slots game, exiting app...");
            AppManager::exitApp();
        } else {
            SlotsManager::update();
        }
    }
    void AppRegistration::slotsOnExit() {
        Serial.println("[SlotsApp] onExit: Cleaning up Slots game...");
        SlotsManager::reset();
    }

    // Example "IrBlastApp" callbacks
    void AppRegistration::irOnRun() {
        Serial.println("[IrBlastApp] onRun: Setting up IR blasting...");
        IRManager::init();
    }
    void AppRegistration::irOnUpdate() {
        // Could handle IR blasting logic, e.g. sending signals
        IRManager::update();
    }
    void AppRegistration::irOnExit() {
        Serial.println("[IrBlastApp] onExit: IR blasting done.");
        IRManager::stopBlast();
    }

    // Example "NfcApp" callbacks
    void AppRegistration::nfcOnRun() {
        Serial.println("[NfcApp] onRun: Initializing NFC scan...");
        NFCManager::init();
    }
    void AppRegistration::nfcOnUpdate() {
        // Ask NFCManager to handle repeated scans
        NFCManager::update();
    }
    void AppRegistration::nfcOnExit() {
        Serial.println("[NfcApp] onExit: Stopping NFC logic...");
        // NFCManager may not have a teardown, but you can add if needed
        NFCManager::stop();
    }

    // Example "AppSettings" callbacks (LED config, etc.)
    void AppRegistration::settingsOnRun() {
        Serial.println("[AppSettings] onRun: Opening LED Settings...");
        MenuManager::enterSubmenu(MenuManager::appSettingsMenu);
        // e.g., store or retrieve LED state
    }
    void AppRegistration::settingsOnUpdate() {

        // e.g., let user pick a mode
    }

    void AppRegistration::settingsOnExit() {
        Serial.println("[AppSettings] onExit: Closing LED Settings...");
    }

    // Registering the apps
    void AppRegistration::registerDefaultApps() {
        if (!AppManager::isInitialized()) {
            Utilities::LOG_ERROR("Cannot register apps - AppManager not initialized");
            return;
        }

        try {
            bool success = true;
            
            // Register SlotsApp
            if (success) {
                AppInfo slotsApp;
                slotsApp.name = "SlotsApp";
                slotsApp.description = "A fun slot-machine mini-game.";
                slotsApp.onRun = &slotsOnRun;
                slotsApp.onUpdate = &slotsOnUpdate;
                slotsApp.onExit = &slotsOnExit;
                success = AppManager::registerApp(slotsApp);
                Utilities::LOG_DEBUG(success ? "SlotsApp registered" : "SlotsApp registration failed");
            }

            // Only continue registering if previous was successful
            if (success) {
                AppInfo irApp;
                irApp.name = "IrBlastApp";
                irApp.description = "IR blasting and remote control.";
                irApp.onRun = &irOnRun;
                irApp.onUpdate = &irOnUpdate;
                irApp.onExit = &irOnExit;
                success = AppManager::registerApp(irApp);
                Utilities::LOG_DEBUG(success ? "IrBlastApp registered" : "IrBlastApp registration failed");
            }

            if (success) {
                AppInfo nfcApp;
                nfcApp.name = "NfcApp";
                nfcApp.description = "NFC scanning and reading.";
                nfcApp.onRun = &nfcOnRun;
                nfcApp.onUpdate = &nfcOnUpdate;
                nfcApp.onExit = &nfcOnExit;
                success = AppManager::registerApp(nfcApp);
            }
            
            if (success) {
                AppInfo settingsApp;
                settingsApp.name = "AppSettings";
                settingsApp.description = "Configure LED modes, etc.";
                settingsApp.onRun = &settingsOnRun;
                settingsApp.onUpdate = &settingsOnUpdate;
                settingsApp.onExit = &settingsOnExit;
                success = AppManager::registerApp(settingsApp);
            }

            if (!success) {
                Utilities::LOG_ERROR("Failed to register one or more apps");
            }

        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("App registration error: %s", e.what());
            // Don't throw - allow boot to continue without apps
        }
    }
}