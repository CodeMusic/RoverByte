/**
 * @brief Core cognitive system configuration and behavioral parameters
 * 
 * Defines fundamental neural processing parameters:
 * - System-wide log verbosity
 * - Cognitive state thresholds
 * - Behavioral response timing
 * - Sensory processing limits
 * - Neural pathway configurations
 * 
 * This configuration hub establishes:
 * - Default cognitive states
 * - Debug information flow
 * - System-wide constants
 * - Cross-cortex parameters
 * - Core behavioral settings
 */

#pragma once

#include "PrefrontalCortex/ProtoPerceptions.h"

// Global configuration settings
namespace RoverConfig 
{
    // Default log level for the system
    static const PrefrontalCortex::SystemTypes::LogLevel DEFAULT_LOG_LEVEL = 
        PrefrontalCortex::SystemTypes::LogLevel::SCOPE;
}
