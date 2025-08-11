#include "PoseManager.h"

PoseManager::PoseManager() {
    poseDetectionEnabled = false;
    poseConfidenceThreshold = 0.5f;
    maxPeopleToDetect = 8;
    
    showSkeletonOverlay = true;
    showPoseLabels = true;
    showKeypointTrails = false;
    
    lineManager = nullptr;
    commManager = nullptr;
    poseCrossingEventsCount = 0;
}

PoseManager::~PoseManager() {
}

void PoseManager::setup() {
    setupPoseDetection();
    ofLogNotice() << "PoseManager: Initialized";
}

void PoseManager::update(ofPixels& videoFrame) {
    if (!poseDetectionEnabled) return;
    
    // Real pose detection using Vision framework
    try {
        currentPoses = poseDetector.detectPoses(videoFrame);
        
        // Process pose events for line crossing
        if (lineManager && commManager && !currentPoses.empty()) {
            checkPoseLineCrossings();
        }
    } catch (const std::exception& e) {
        ofLogError() << "PoseManager: Error in pose detection: " << e.what();
        currentPoses.clear();
    }
}

void PoseManager::draw() {
    if (!poseDetectionEnabled || currentPoses.empty()) return;
    
    // Draw pose visualizations (stub)
    for (const auto& pose : currentPoses) {
        ofSetColor(0, 255, 0);
        // Simple pose visualization would go here
    }
    
    ofSetColor(255);  // Reset color
}

void PoseManager::setupPoseDetection() {
    poseDetector.setConfidenceThreshold(poseConfidenceThreshold);
    poseDetector.setMaxPeople(maxPeopleToDetect);
    
    bool setupSuccess = poseDetector.setup();
    if (setupSuccess) {
        ofLogNotice() << "PoseManager: Apple Vision Framework pose detection system initialized successfully";
    } else {
        ofLogError() << "PoseManager: Failed to initialize pose detection system";
    }
}

void PoseManager::saveToJSON(ofxJSONElement& json) {
    json["poseDetectionEnabled"] = poseDetectionEnabled;
    json["poseConfidenceThreshold"] = poseConfidenceThreshold;
    json["maxPeopleToDetect"] = maxPeopleToDetect;
    json["showSkeletonOverlay"] = showSkeletonOverlay;
    json["showPoseLabels"] = showPoseLabels;
    json["showKeypointTrails"] = showKeypointTrails;
}

void PoseManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("poseDetectionEnabled")) {
        poseDetectionEnabled = json["poseDetectionEnabled"].asBool();
    }
    if (json.isMember("poseConfidenceThreshold")) {
        poseConfidenceThreshold = json["poseConfidenceThreshold"].asFloat();
    }
    if (json.isMember("maxPeopleToDetect")) {
        maxPeopleToDetect = json["maxPeopleToDetect"].asInt();
    }
    if (json.isMember("showSkeletonOverlay")) {
        showSkeletonOverlay = json["showSkeletonOverlay"].asBool();
    }
    if (json.isMember("showPoseLabels")) {
        showPoseLabels = json["showPoseLabels"].asBool();
    }
    if (json.isMember("showKeypointTrails")) {
        showKeypointTrails = json["showKeypointTrails"].asBool();
    }
}

void PoseManager::setDefaults() {
    poseDetectionEnabled = false;
    poseConfidenceThreshold = 0.5f;
    maxPeopleToDetect = 8;
    showSkeletonOverlay = true;
    showPoseLabels = true;
    showKeypointTrails = false;
}