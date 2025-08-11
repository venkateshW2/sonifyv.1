#pragma once

#include "ofMain.h"
#include "PoseStructures.h"
#include "PoseDetectorWrapper.h"
#include "ofxJSON.h"

class LineManager;
class CommunicationManager;

class PoseManager {
public:
    PoseManager();
    ~PoseManager();
    
    // Core pose detection methods
    void setup();
    void update(ofPixels& videoFrame);
    void draw();
    
    // Pose detection control
    void setPoseDetectionEnabled(bool enabled) { poseDetectionEnabled = enabled; }
    bool isPoseDetectionEnabled() const { return poseDetectionEnabled; }
    void setPoseConfidenceThreshold(float threshold) { poseConfidenceThreshold = threshold; }
    float getPoseConfidenceThreshold() const { return poseConfidenceThreshold; }
    void setMaxPeopleToDetect(int maxPeople) { maxPeopleToDetect = maxPeople; }
    int getMaxPeopleToDetect() const { return maxPeopleToDetect; }
    
    // Visual display controls
    void setShowSkeletonOverlay(bool enabled) { showSkeletonOverlay = enabled; }
    void setShowPoseLabels(bool enabled) { showPoseLabels = enabled; }
    void setShowKeypointTrails(bool enabled) { showKeypointTrails = enabled; }
    
    bool getShowSkeletonOverlay() const { return showSkeletonOverlay; }
    bool getShowPoseLabels() const { return showPoseLabels; }
    bool getShowKeypointTrails() const { return showKeypointTrails; }
    
    // Access to detected poses
    const std::vector<PersonPose>& getCurrentPoses() const { return currentPoses; }
    int getDetectedPeopleCount() const { return currentPoses.size(); }
    int getPoseCrossingEventsCount() const { return poseCrossingEventsCount; }
    
    // Configuration
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // Manager references
    void setLineManager(LineManager* manager) { lineManager = manager; }
    void setCommunicationManager(CommunicationManager* manager) { commManager = manager; }

private:
    // Pose detection system
    PoseDetectorWrapper poseDetector;
    bool poseDetectionEnabled;
    float poseConfidenceThreshold;
    int maxPeopleToDetect;
    std::vector<PersonPose> currentPoses;  // Store current detected poses
    
    // Pose detection GUI controls
    bool showSkeletonOverlay;
    bool showPoseLabels;
    bool showKeypointTrails;
    
    // Manager references
    LineManager* lineManager;
    CommunicationManager* commManager;
    
    // Event tracking
    int poseCrossingEventsCount;
    
    // Helper methods
    void setupPoseDetection();
    void processPoseDetection(ofPixels& videoFrame);
    void drawPoseDetections();
    void checkPoseLineCrossings();
    ofColor getPoseColorForPerson(int personId);
};