#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include "Logger.hpp"

enum class BypassMode {
    NONE,           // No bypass, only logging
    BASIC,          // Only bypass isHardwareDetected
    FULL,           // Full hardware detection bypass
    AUTHENTICATION, // Authentication bypass
    CUSTOM          // Custom bypass based on config
};

// Sensitivity level enum
enum class SensitivityLevel {
    MINIMAL = 0,  // Level 0: Minimal interception
    LOW = 1,      // Level 1: Low intervention
    MEDIUM = 2,   // Level 2: Balanced intervention (recommended)
    HIGH = 3,     // Level 3: Aggressive intervention
    MAXIMUM = 4   // Level 4: Maximum intervention
};

class Configuration {
public:
    static Configuration& getInstance() {
        static Configuration instance;
        return instance;
    }
    
    // Initialize configuration with path to config file
    bool initialize(const std::string& configPath);
    
    // Get the current bypass mode
    BypassMode getBypassMode() const;
    
    // Get the current sensitivity level
    SensitivityLevel getSensitivityLevel() const;
    
    // Check if a specific bypass is enabled
    bool isMethodBypassed(const std::string& methodName) const;
    
    // Check if a specific feature should be active based on sensitivity level
    bool isFeatureEnabled(const std::string& featureName) const;
    
    // Check if debug logging is enabled
    bool isDebugEnabled() const;
    
    // Get log file path
    std::string getLogFilePath() const;
    
    // Get custom parameters for specific methods
    std::string getCustomParameter(const std::string& methodName) const;
    
    // Save current configuration
    bool saveConfiguration() const;
    
private:
    Configuration() = default;
    ~Configuration() = default;
    
    // Disallow copy/move
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;
    Configuration(Configuration&&) = delete;
    Configuration& operator=(Configuration&&) = delete;
    
    std::string configPath;
    BypassMode mode = BypassMode::BASIC;
    SensitivityLevel sensitivity = SensitivityLevel::MEDIUM; // Default: Medium (2)
    bool debugEnabled = false;
    std::string logFilePath;
    std::unordered_map<std::string, bool> methodBypassMap;
    std::unordered_map<std::string, std::string> customParameters;
};
