#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    void setup();
    void update();
    void draw();
    
    bool wantsKeyboard() const;
    void toggleGUI();
    void showResizeWarningDialog();
    
    // Core GUI methods - EXACT COPY from working backup
    void drawGUI();
    void drawMainControlsTab();
    void drawMIDISettingsTab();
    void drawDetectionClassesTab();
    
    // EXACT same GUI variables as working backup
    ofxImGui::Gui gui;
    bool showGUI;
    bool showResizeWarning_;
    
    // Need access to all managers for GUI controls
    void setManagers(class VideoManager* videoMgr, 
                     class LineManager* lineMgr, 
                     class DetectionManager* detMgr,
                     class CommunicationManager* commMgr,
                     class ConfigManager* confMgr,
                     class PoseManager* poseMgr);
    
    // GUI state variables - EXACT COPY from working backup
    float confidenceThreshold;
    int frameSkipValue;
    bool showTrajectoryTrails;
    bool showVelocityVectors;
    bool enableOcclusionTracking;
    float trailFadeTime;
    int maxTrajectoryPoints;
    bool showDetections;
    bool showLines;
    
    // Add pose detection tab method
    void drawPoseDetectionTab();
    void handleWindowResize(int width, int height);
    
private:
    // Manager references for GUI controls
    class VideoManager* videoManager;
    class LineManager* lineManager; 
    class DetectionManager* detectionManager;
    class CommunicationManager* communicationManager;
    class CommunicationManager* commManager;  // Alias used in cpp file
    class ConfigManager* configManager;
    class PoseManager* poseManager;  // Referenced in cpp file
};