#ifndef APPREGISTRATION_H
#define APPREGISTRATION_H

#include "AppManager.h"
#include "../GameCortex/SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include <Arduino.h>

namespace GameCortex
{
    class AppRegistration {
    public:
        // SlotsApp callbacks
        static void slotsOnRun();
        static void slotsOnUpdate();
        static void slotsOnExit();

        // IrBlastApp callbacks
        static void irOnRun();
        static void irOnUpdate();
        static void irOnExit();

        // NfcApp callbacks
        static void nfcOnRun();
        static void nfcOnUpdate();
        static void nfcOnExit();

        // AppSettings callbacks
        static void settingsOnRun();
        static void settingsOnUpdate();
        static void settingsOnExit();

        // Function to register all default apps
        static void registerDefaultApps();


    };
}
#endif // APPREGISTRATION_H