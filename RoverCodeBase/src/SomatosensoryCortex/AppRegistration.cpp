#include "AppManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include <Arduino.h>

// Example "SlotsApp" callbacks
static void slotsOnRun() {
    Serial.println("[SlotsApp] onRun: Starting Slots game...");
    SlotsManager::init();
}
static void slotsOnUpdate() {
    // Called every cycle while SlotsApp is RUNNING
    if (!SlotsManager::isGameActive()) {
        Serial.println("[SlotsApp] No active Slots game, exiting app...");
        AppManager::exitApp();
    } else {
        SlotsManager::update();
    }
}
static void slotsOnExit() {
    Serial.println("[SlotsApp] onExit: Cleaning up Slots game...");
    SlotsManager::reset();
}

// Example "IrBlastApp" callbacks
static void irOnRun() {
    Serial.println("[IrBlastApp] onRun: Setting up IR blasting...");
    IRManager::init();
}
static void irOnUpdate() {
    // Could handle IR blasting logic, e.g. sending signals
    IRManager::update();
}
static void irOnExit() {
    Serial.println("[IrBlastApp] onExit: IR blasting done.");
    IRManager::stopBlast();
}

// Example "NfcApp" callbacks
static void nfcOnRun() {
    Serial.println("[NfcApp] onRun: Initializing NFC scan...");
    NFCManager::init();
}
static void nfcOnUpdate() {
    // Ask NFCManager to handle repeated scans
    NFCManager::update();
}
static void nfcOnExit() {
    Serial.println("[NfcApp] onExit: Stopping NFC logic...");
    // NFCManager may not have a teardown, but you can add if needed
    NFCManager::stop();
}

// Example "AppSettings" callbacks (LED config, etc.)
static void settingsOnRun() {
    Serial.println("[AppSettings] onRun: Opening LED Settings...");
    // e.g., store or retrieve LED state
}
static void settingsOnUpdate() {
    // e.g., let user pick a mode
}
static void settingsOnExit() {
    Serial.println("[AppSettings] onExit: Closing LED Settings...");
}

// Registering the apps
void registerDefaultApps() {
    AppManager::registerApp({
        "SlotsApp", 
        "A fun slot-machine mini-game.",
        slotsOnRun,
        slotsOnUpdate,
        slotsOnExit
    });
    
    AppManager::registerApp({
        "IrBlastApp", 
        "IR blasting and remote control.",
        irOnRun,
        irOnUpdate,
        irOnExit
    });
    
    AppManager::registerApp({
        "NfcApp", 
        "NFC scanning and reading.",
        nfcOnRun,
        nfcOnUpdate,
        nfcOnExit
    });
    
    AppManager::registerApp({
        "AppSettings", 
        "Configure LED modes, etc.",
        settingsOnRun,
        settingsOnUpdate,
        settingsOnExit
    });
}

// You can call this from setup() or a similar initialization sequence
//   AppManager::init();
//   registerDefaultApps(); 