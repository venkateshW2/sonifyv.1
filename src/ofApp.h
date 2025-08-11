#pragma once

#include "ofMain.h"

// Include all managers - EXACT same functionality, just modularized
#include "VideoManager.h"
#include "LineManager.h"
#include "DetectionManager.h"
#include "UIManager.h"
#include "CommunicationManager.h"
#include "ConfigManager.h"
#include "PoseManager.h"

class ofApp : public ofBaseApp{
public:
    // EXACT same interface as working backup
    void setup() override;
    void update() override;
    void draw() override;
    void exit() override;
    ~ofApp();

    void keyPressed(int key) override;
    void keyReleased(int key) override;
    void mouseMoved(int x, int y ) override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
    void mouseEntered(int x, int y) override;
    void mouseExited(int x, int y) override;
    void windowResized(int w, int h) override;
    void dragEvent(ofDragInfo dragInfo) override;
    void gotMessage(ofMessage msg) override;

private:
    // All the managers - EXACT same functionality as working backup, just organized
    VideoManager videoManager;
    LineManager lineManager;
    DetectionManager detectionManager;
    UIManager uiManager;
    CommunicationManager communicationManager;
    ConfigManager configManager;
    PoseManager poseManager;
    
    // Window resize management - EXACT COPY from working backup
    int originalWindowWidth;
    int originalWindowHeight;
};