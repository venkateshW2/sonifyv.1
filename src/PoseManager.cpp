#include "PoseManager.h"
#include "LineManager.h"
#include "CommunicationManager.h"

PoseManager::PoseManager() {
    poseDetectionEnabled = false;
    poseDetectionInitialized = false;
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
    // Only setup pose detection if enabled - performance optimization
    if (poseDetectionEnabled) {
        setupPoseDetection();
        ofLogNotice() << "PoseManager: Pose detection initialized and enabled";
    } else {
        ofLogNotice() << "PoseManager: Initialized with pose detection disabled";
    }
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
    for (size_t poseIdx = 0; poseIdx < currentPoses.size(); poseIdx++) {
        const auto& pose = currentPoses[poseIdx];
        ofSetColor(0, 255, 0);
        // Simple pose visualization would go here
        // In stub mode, just draw a placeholder
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

void PoseManager::checkPoseLineCrossings() {
    if (!lineManager || !commManager || currentPoses.empty()) return;
    
    // Get all drawn lines from LineManager
    const auto& lines = lineManager->getLines();
    if (lines.empty()) return;
    
    // Check each pose for line crossings
    for (size_t poseIdx = 0; poseIdx < currentPoses.size(); poseIdx++) {
        const auto& pose = currentPoses[poseIdx];
        
        // Check each keypoint in the pose
        for (size_t jointIdx = 0; jointIdx < pose.keypoints.size(); jointIdx++) {
            const auto& keypoint = pose.keypoints[jointIdx];
            
            // Skip low-confidence keypoints
            if (keypoint.confidence < poseConfidenceThreshold) continue;
            
            // Check intersection with each line
            for (size_t lineIdx = 0; lineIdx < lines.size(); lineIdx++) {
                const auto& line = lines[lineIdx];
                
                // Calculate distance from keypoint to line using point-to-line distance
                ofPoint keypointPos(keypoint.position.x, keypoint.position.y);
                
                // Calculate distance from point to line segment using simple math
                // Based on the algorithm used in LineManager
                float A = keypointPos.x - line.startPoint.x;
                float B = keypointPos.y - line.startPoint.y;
                float C = line.endPoint.x - line.startPoint.x;
                float D = line.endPoint.y - line.startPoint.y;
                
                float dot = A * C + B * D;
                float lenSq = C * C + D * D;
                float distance = 999999.0f; // default large distance
                
                if (lenSq > 0) {
                    float param = dot / lenSq;
                    
                    ofPoint closestPoint;
                    if (param < 0) {
                        closestPoint = line.startPoint;
                    } else if (param > 1) {
                        closestPoint = line.endPoint;
                    } else {
                        closestPoint.x = line.startPoint.x + param * C;
                        closestPoint.y = line.startPoint.y + param * D;
                    }
                    
                    // Calculate distance from keypoint to closest point on line
                    distance = sqrt((keypointPos.x - closestPoint.x) * (keypointPos.x - closestPoint.x) + 
                                   (keypointPos.y - closestPoint.y) * (keypointPos.y - closestPoint.y));
                }
                
                // If keypoint is very close to line (within 10 pixels), consider it a crossing
                if (distance < 10.0f) {
                    // Create pose crossing event
                    poseCrossingEventsCount++;
                    
                    // Send OSC message for pose crossing
                    if (commManager) {
                        // Use joint names from pose detection system
                        string jointName = "joint_" + ofToString(jointIdx);
                        
                        commManager->sendOSCPoseCrossing(
                            static_cast<int>(lineIdx),           // line ID
                            static_cast<int>(poseIdx),           // person ID
                            jointName,                           // joint name
                            keypointPos,                         // crossing point
                            keypoint.confidence                  // confidence
                        );
                        
                        // Send MIDI event for pose crossing (using line's MIDI properties)
                        commManager->sendMIDILineCrossing(
                            static_cast<int>(lineIdx),           // line ID for MIDI properties
                            "person",                            // treat as person vehicle type
                            keypoint.confidence,                 // confidence
                            0.0f                                 // no speed for pose
                        );
                    }
                    
                    ofLogNotice() << "PoseManager: Pose crossing detected - Person:" << poseIdx 
                                  << " Joint:" << jointIdx << " Line:" << lineIdx
                                  << " Position:(" << keypointPos.x << "," << keypointPos.y << ")";
                }
            }
        }
    }
}

void PoseManager::setPoseDetectionEnabled(bool enabled) {
    poseDetectionEnabled = enabled;
    
    // Lazy initialization - only setup pose detection when first enabled
    if (enabled && !poseDetectionInitialized) {
        setupPoseDetection();
        poseDetectionInitialized = true;
        ofLogNotice() << "PoseManager: Pose detection system initialized and enabled";
    } else if (enabled) {
        ofLogNotice() << "PoseManager: Pose detection enabled";
    } else {
        ofLogNotice() << "PoseManager: Pose detection disabled";
    }
}

void PoseManager::setDefaults() {
    poseDetectionEnabled = false;
    poseDetectionInitialized = false;
    poseConfidenceThreshold = 0.5f;
    maxPeopleToDetect = 8;
    showSkeletonOverlay = true;
    showPoseLabels = true;
    showKeypointTrails = false;
}