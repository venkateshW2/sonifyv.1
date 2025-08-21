#include "ConfigManager.h"
#include "TempoManager.h"
#include "UIManager.h"
#include "LineManager.h"
#include "VideoManager.h"
#include "DetectionManager.h"
#include "CommunicationManager.h"
#include "ScaleManager.h"


ConfigManager::ConfigManager() {
    uiManager = nullptr;
    lineManager = nullptr;
    videoManager = nullptr;
    detectionManager = nullptr;
    commManager = nullptr;
    
    configLoaded = false;
    configFilePath = "";
}

ConfigManager::~ConfigManager() {
}

void ConfigManager::setup() {
    configFilePath = getConfigPath();
    ensureConfigDirectory();
    ofLogNotice() << "ConfigManager: Setup complete - " << configFilePath;
}

void ConfigManager::setManagers(UIManager* uiMgr, LineManager* lineMgr, 
                                VideoManager* videoMgr, DetectionManager* detMgr,
                                CommunicationManager* commMgr, TempoManager* tempoMgr,
                                ScaleManager* scaleMgr) {
    uiManager = uiMgr;
    lineManager = lineMgr;
    videoManager = videoMgr;
    detectionManager = detMgr;
    commManager = commMgr;
    tempoManager = tempoMgr;
    scaleManager = scaleMgr;
}

void ConfigManager::saveConfig() {
    if (configFilePath.empty()) {
        ofLogNotice() << "ConfigManager: No config path set, cannot save";
        return;
    }
    
    ofxJSONElement json;
    
    // Save all manager configurations
    if (uiManager) {
        ofxJSONElement uiJson;
        // UI settings would be saved here
        json["ui"] = uiJson;
    }
    
    if (lineManager) {
        ofxJSONElement lineJson;
        lineManager->saveToJSON(lineJson);
        json["lines"] = lineJson;
    }
    
    if (videoManager) {
        ofxJSONElement videoJson;
        videoManager->saveToJSON(videoJson);
        json["video"] = videoJson;
    }
    
    if (detectionManager) {
        ofxJSONElement detectionJson;
        detectionManager->saveToJSON(detectionJson);
        json["detection"] = detectionJson;
    }
    
    if (commManager) {
        ofxJSONElement commJson;
        commManager->saveToJSON(commJson);
        json["communication"] = commJson;
    }
    
    if (tempoManager) {
        ofxJSONElement tempoJson;
        tempoManager->saveToJSON(tempoJson);
        json["tempo"] = tempoJson;
    }
    
    if (scaleManager) {
        ofxJSONElement scaleJson;
        scaleManager->saveToJSON(scaleJson);
        json["scales"] = scaleJson;
    }
    
    // Add metadata
    json["version"] = "1.0";
    json["timestamp"] = ofGetTimestampString();
    
    // Save to file
    if (json.save(configFilePath, true)) {
        ofLogNotice() << "ConfigManager: Configuration saved successfully";
    } else {
        ofLogNotice() << "ConfigManager: Failed to save configuration";
    }
}

void ConfigManager::loadConfig() {
    if (configFilePath.empty()) {
        ofLogNotice() << "ConfigManager: No config path set, using defaults";
        setDefaultConfig();
        return;
    }
    
    ofxJSONElement json;
    if (!json.open(configFilePath)) {
        ofLogNotice() << "ConfigManager: Config file not found, creating defaults";
        setDefaultConfig();
        saveConfig();  // Create default config file
        return;
    }
    
    if (!validateConfigFile(json)) {
        ofLogNotice() << "ConfigManager: Invalid config file, using defaults";
        setDefaultConfig();
        return;
    }
    
    // Load all manager configurations
    if (uiManager && json.isMember("ui")) {
        // UI settings would be loaded here
    }
    
    if (lineManager && json.isMember("lines")) {
        lineManager->loadFromJSON(json["lines"]);
    }
    
    if (videoManager && json.isMember("video")) {
        videoManager->loadFromJSON(json["video"]);
    }
    
    if (detectionManager && json.isMember("detection")) {
        detectionManager->loadFromJSON(json["detection"]);
    }
    
    if (commManager && json.isMember("communication")) {
        commManager->loadFromJSON(json["communication"]);
    }
    
    if (tempoManager && json.isMember("tempo")) {
        tempoManager->loadFromJSON(json["tempo"]);
    }
    
    if (scaleManager && json.isMember("scales")) {
        scaleManager->loadFromJSON(json["scales"]);
    }
    
    configLoaded = true;
    ofLogNotice() << "ConfigManager: Configuration loaded successfully";
}

void ConfigManager::setDefaultConfig() {
    // Set defaults for all managers
    if (uiManager) {
        // UI defaults would be set here
    }
    
    if (lineManager) {
        lineManager->setDefaults();
    }
    
    if (videoManager) {
        videoManager->setDefaults();
    }
    
    if (detectionManager) {
        detectionManager->setDefaults();
    }
    
    if (commManager) {
        commManager->setDefaults();
    }
    
    if (tempoManager) {
        tempoManager->setDefaults();
    }
    
    if (scaleManager) {
        scaleManager->setDefaults();
    }
    
    ofLogNotice() << "ConfigManager: Default configuration applied";
}

string ConfigManager::getConfigPath() const {
    return ofToDataPath("config.json", true);
}

void ConfigManager::saveOnExit() {
    saveConfig();
    ofLogNotice() << "ConfigManager: Configuration saved on exit";
}

void ConfigManager::resetToDefaults() {
    setDefaultConfig();
    ofLogNotice() << "ConfigManager: Reset to defaults";
}

void ConfigManager::ensureConfigDirectory() {
    string dataPath = ofToDataPath("", true);
    ofDirectory configDir(dataPath);
    if (!configDir.exists()) {
        configDir.create(true);
        ofLogNotice() << "ConfigManager: Created data directory";
    }
}

bool ConfigManager::validateConfigFile(const ofxJSONElement& json) {
    // Basic validation - check if it's a valid JSON object
    if (json.isNull() || !json.isObject()) {
        return false;
    }
    
    // Could add more specific validation here
    return true;
}