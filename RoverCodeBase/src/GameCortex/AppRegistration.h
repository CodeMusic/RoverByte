/**
 * @brief AppRegistration manages cognitive application callbacks and initialization
 * 
 * This class handles the registration and lifecycle callbacks for all entertainment
 * and engagement applications, including:
 * - Slots: Pattern recognition and reward systems
 * - IR Blast: External device interaction pathways
 * - NFC: Near-field cognitive communication
 * - Settings: System configuration and behavioral adjustments
 * 
 * Each application has three core neural pathways:
 * - onRun: Initial cognitive activation
 * - onUpdate: Continuous neural processing
 * - onExit: Graceful neural deactivation
 */

#ifndef APPREGISTRATION_H
#define APPREGISTRATION_H

#include "AppManager.h"
#include "SlotsManager.h"
#include "../PsychicCortex/NFCManager.h"
#include "../PsychicCortex/IRManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include <Arduino.h>

namespace GameCortex
{
    class AppRegistration {
    public:
        /**
         * @brief Slots application neural pathways
         * Manages pattern recognition and reward feedback loops
         */
        static void slotsOnRun();
        static void slotsOnUpdate();
        static void slotsOnExit();

        /**
         * @brief IR Blast application neural pathways
         * Handles external device communication patterns
         */
        static void irOnRun();
        static void irOnUpdate();
        static void irOnExit();

        /**
         * @brief NFC application neural pathways
         * Processes near-field cognitive interactions
         */
        static void nfcOnRun();
        static void nfcOnUpdate();
        static void nfcOnExit();

        /**
         * @brief Settings application neural pathways
         * Manages system configuration and behavioral adjustments
         */
        static void settingsOnRun();
        static void settingsOnUpdate();
        static void settingsOnExit();

        /**
         * @brief Registers all default applications with the cognitive system
         * Initializes core entertainment and engagement pathways
         */
        static void registerDefaultApps();
    };
}
#endif // APPREGISTRATION_H