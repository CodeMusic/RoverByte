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

#include "src/PrefrontalCortex/ProtoPerceptions.h"
#include "src/CorpusCallosum/SynapticPathways.h"

using namespace CorpusCallosum;
namespace PC = PrefrontalCortex;

// Global configuration settings
namespace RoverConfig 
{
    // Default log level for the system
    static const PrefrontalCortex::SystemTypes::LogLevel DEFAULT_LOG_LEVEL = 
        PrefrontalCortex::SystemTypes::LogLevel::SCOPE;

    // WiFi Configuration
    #define ROVER_WIFI_MAX_ATTEMPTS 3  // Maximum number of complete network rotation attempts
}

// Network Configuration
namespace NetworkConfig 
{
    using PC::NetworkTypes::NetworkCredentials;
    
    constexpr NetworkCredentials AVAILABLE_NETWORKS[] = 
    {
        {"RevivalNetwork", ""},
        {"CodeMusicai", ""},
        {"Starlink", ""}
    };
    
    constexpr size_t NETWORK_COUNT = sizeof(AVAILABLE_NETWORKS) / sizeof(AVAILABLE_NETWORKS[0]);
}
