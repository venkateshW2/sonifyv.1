#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    void setup();
    void update();
    void draw();
    
    void loadConfig();
    void saveConfig();
    void saveOnExit();
    void resetToDefaults(); // Public method to reset all settings to defaults
    
    // Manager connections
    void setManagers(class UIManager* uiMgr, class LineManager* lineMgr, 
                     class VideoManager* videoMgr, class DetectionManager* detMgr,
                     class CommunicationManager* commMgr, class PoseManager* poseMgr);
    
private:
    string configPath;
    string configFilePath;
    bool configLoaded;
    
    // Helper methods
    string getConfigPath() const;
    void ensureConfigDirectory();
    bool validateConfigFile(const ofxJSONElement& json);
    void setDefaultConfig();
    
    // Manager references
    class UIManager* uiManager;
    class LineManager* lineManager;
    class VideoManager* videoManager;
    class DetectionManager* detectionManager;
    class CommunicationManager* commManager;
    class PoseManager* poseManager;
};