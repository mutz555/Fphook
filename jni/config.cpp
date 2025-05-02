#include "config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

bool Configuration::initialize(const std::string& configFilePath) {
    configPath = configFilePath;
    
    // Check if configuration file exists
    if (!std::filesystem::exists(configFilePath)) {
        Logger::error("Configuration file not found: " + configFilePath);
        return false;
    }
    
    try {
        // Read configuration file
        std::ifstream configFile(configFilePath);
        json config;
        configFile >> config;
        
        // Parse bypass mode
        std::string modeStr = config.value("bypass_mode", "basic");
        if (modeStr == "none") {
            mode = BypassMode::NONE;
        } else if (modeStr == "basic") {
            mode = BypassMode::BASIC;
        } else if (modeStr == "full") {
            mode = BypassMode::FULL;
        } else if (modeStr == "authentication") {
            mode = BypassMode::AUTHENTICATION;
        } else if (modeStr == "custom") {
            mode = BypassMode::CUSTOM;
        } else if (modeStr == "hyperos") {
            mode = BypassMode::FULL; // Use FULL mode but with HyperOS specific tweaks
            customParameters["hyperos_mode"] = "true";
            Logger::info("HyperOS specific mode enabled by mutz");
        } else {
            Logger::warn("Unknown bypass mode: " + modeStr + ", using BASIC");
            mode = BypassMode::BASIC;
        }
        
        // Parse sensitivity level
        int sensitivityValue = config.value("sensitivity_level", 2); // Default: Medium (2)
        switch (sensitivityValue) {
            case 0:
                sensitivity = SensitivityLevel::MINIMAL;
                Logger::info("Sensitivity level set to MINIMAL (0)");
                break;
            case 1:
                sensitivity = SensitivityLevel::LOW;
                Logger::info("Sensitivity level set to LOW (1)");
                break;
            case 2:
                sensitivity = SensitivityLevel::MEDIUM;
                Logger::info("Sensitivity level set to MEDIUM (2)");
                break;
            case 3:
                sensitivity = SensitivityLevel::HIGH;
                Logger::info("Sensitivity level set to HIGH (3)");
                break;
            case 4:
                sensitivity = SensitivityLevel::MAXIMUM;
                Logger::info("Sensitivity level set to MAXIMUM (4)");
                break;
            default:
                sensitivity = SensitivityLevel::MEDIUM;
                Logger::warn("Invalid sensitivity level: " + std::to_string(sensitivityValue) + ", using MEDIUM (2)");
                break;
        }
        
        // Parse debug setting
        debugEnabled = config.value("debug_enabled", false);
        
        // Configure logger
        Logger::setDebugEnabled(debugEnabled);
        
        // Parse log file path
        logFilePath = config.value("log_file", "/data/local/tmp/fpbypass.log");
        if (!logFilePath.empty()) {
            Logger::setLogFile(logFilePath);
        }
        
        // Parse method bypass settings for custom mode
        if (mode == BypassMode::CUSTOM && config.contains("methods")) {
            auto methods = config["methods"];
            for (auto& [key, value] : methods.items()) {
                methodBypassMap[key] = value.value("enabled", false);
                
                // Store custom parameters if present
                if (value.contains("params")) {
                    customParameters[key] = value["params"].dump();
                }
            }
        }
        
        Logger::info("Configuration loaded successfully");
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to parse configuration: " + std::string(e.what()));
        return false;
    }
}

BypassMode Configuration::getBypassMode() const {
    return mode;
}

bool Configuration::isMethodBypassed(const std::string& methodName) const {
    // In non-custom modes, determine based on mode
    if (mode == BypassMode::NONE) {
        return false;
    } else if (mode == BypassMode::BASIC) {
        // Only bypass isHardwareDetected in basic mode
        return methodName == "isHardwareDetected";
    } else if (mode == BypassMode::FULL) {
        // Bypass all hardware detection methods in full mode
        return methodName == "isHardwareDetected" || 
               methodName == "canAuthenticate" || 
               methodName == "hasEnrolledFingerprints" ||
               methodName == "getAuthenticatorId";
    } else if (mode == BypassMode::AUTHENTICATION) {
        // Bypass authentication methods
        return methodName == "authenticate" ||
               methodName == "isHardwareDetected" ||
               methodName == "canAuthenticate" ||
               methodName == "hasEnrolledFingerprints" ||
               methodName == "getAuthenticatorId";
    } else if (mode == BypassMode::CUSTOM) {
        // For custom mode, check the map
        auto it = methodBypassMap.find(methodName);
        if (it != methodBypassMap.end()) {
            return it->second;
        }
    }
    
    return false;
}

bool Configuration::isDebugEnabled() const {
    return debugEnabled;
}

std::string Configuration::getLogFilePath() const {
    return logFilePath;
}

SensitivityLevel Configuration::getSensitivityLevel() const {
    return sensitivity;
}

bool Configuration::isFeatureEnabled(const std::string& featureName) const {
    // Base functionality is always enabled at all sensitivity levels
    if (featureName == "basic_bypass") {
        return true;
    }
    
    // Basic fingerprint hardware detection features
    if (featureName == "hardware_detection") {
        // Enabled at all levels except MINIMAL
        return sensitivity != SensitivityLevel::MINIMAL;
    }
    
    // Enhanced hardware spoofing
    if (featureName == "enhanced_spoofing") {
        // Only enabled at MEDIUM and above
        return sensitivity >= SensitivityLevel::MEDIUM;
    }
    
    // Advanced detection bypass
    if (featureName == "advanced_bypass") {
        // Only enabled at HIGH and above
        return sensitivity >= SensitivityLevel::HIGH;
    }
    
    // Experimental features and authentication bypasses
    if (featureName == "experimental_features" || 
        featureName == "authentication_bypass") {
        // Only enabled at MAXIMUM level
        return sensitivity == SensitivityLevel::MAXIMUM;
    }
    
    // HyperOS specific features
    if (featureName == "hyperos_bypass") {
        // Enabled at MEDIUM and above if hyperosMode is active
        auto it = customParameters.find("hyperos_mode");
        bool hyperosMode = (it != customParameters.end() && it->second == "true");
        return hyperosMode && sensitivity >= SensitivityLevel::MEDIUM;
    }
    
    // Feature not recognized, return false by default
    return false;
}

std::string Configuration::getCustomParameter(const std::string& methodName) const {
    auto it = customParameters.find(methodName);
    if (it != customParameters.end()) {
        return it->second;
    }
    return "{}"; // Return empty JSON object as string
}

bool Configuration::saveConfiguration() const {
    try {
        json config;
        
        // Convert enum to string
        std::string modeStr;
        
        // Check if we're using HyperOS specific mode
        auto hyperosIt = customParameters.find("hyperos_mode");
        bool isHyperOSMode = (hyperosIt != customParameters.end() && hyperosIt->second == "true");
        
        if (isHyperOSMode) {
            modeStr = "hyperos";
        } else {
            switch (mode) {
                case BypassMode::NONE: modeStr = "none"; break;
                case BypassMode::BASIC: modeStr = "basic"; break;
                case BypassMode::FULL: modeStr = "full"; break;
                case BypassMode::AUTHENTICATION: modeStr = "authentication"; break;
                case BypassMode::CUSTOM: modeStr = "custom"; break;
            }
        }
        
        config["bypass_mode"] = modeStr;
        config["sensitivity_level"] = static_cast<int>(sensitivity);
        config["debug_enabled"] = debugEnabled;
        config["log_file"] = logFilePath;
        
        // Save device specific settings
        json deviceSpecific = json::object();
        deviceSpecific["infinix_hot40_pro"] = true;
        deviceSpecific["hyperos_2"] = true;
        config["device_specific"] = deviceSpecific;
        
        // Save HyperOS specific settings
        json hyperosSpecific = json::object();
        hyperosSpecific["FingerprintServiceStub"] = true;
        hyperosSpecific["FingerprintServiceStubImpl"] = true;
        hyperosSpecific["FingerprintAuthenticator"] = true;
        config["hyperos_specific"] = hyperosSpecific;
        
        // Save bypass modes
        json bypassModes = json::array();
        
        json noneMode = json::object();
        noneMode["name"] = "No Bypass";
        noneMode["id"] = "none";
        noneMode["description"] = "Tidak melakukan bypass, hanya logging";
        bypassModes.push_back(noneMode);
        
        json basicMode = json::object();
        basicMode["name"] = "Basic Mode";
        basicMode["id"] = "basic";
        basicMode["description"] = "Hanya bypass isHardwareDetected";
        bypassModes.push_back(basicMode);
        
        json fullMode = json::object();
        fullMode["name"] = "Full Hardware";
        fullMode["id"] = "full";
        fullMode["description"] = "Bypass penuh untuk deteksi hardware";
        bypassModes.push_back(fullMode);
        
        json authMode = json::object();
        authMode["name"] = "Authentication";
        authMode["id"] = "authentication";
        authMode["description"] = "Bypass autentikasi fingerprint (Berbahaya)";
        bypassModes.push_back(authMode);
        
        json hyperosMode = json::object();
        hyperosMode["name"] = "HyperOS Mode";
        hyperosMode["id"] = "hyperos";
        hyperosMode["description"] = "Mode khusus untuk HyperOS by mutz";
        bypassModes.push_back(hyperosMode);
        
        config["bypass_modes"] = bypassModes;
        
        // Save method settings for custom mode
        json methods = json::object();
        for (const auto& [method, enabled] : methodBypassMap) {
            methods[method]["enabled"] = enabled;
            
            auto paramIt = customParameters.find(method);
            if (paramIt != customParameters.end()) {
                // Parse stored JSON string back to JSON object
                methods[method]["params"] = json::parse(paramIt->second);
            }
        }
        
        config["methods"] = methods;
        
        // Write to file
        std::ofstream file(configPath);
        file << config.dump(4); // Pretty print with 4 spaces
        
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to save configuration: " + std::string(e.what()));
        return false;
    }
}
