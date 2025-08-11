#pragma once

#include "ofMain.h"
#include <string>
#include <vector>

struct PoseKeypoint {
    std::string jointName;
    ofPoint position;
    float confidence;
    ofPoint velocity;
    bool isVisible;
    
    // Future 3D support
    float depth;
    ofPoint position3D;
    
    PoseKeypoint() : confidence(0.0f), isVisible(false), depth(0.0f) {}
};

struct PersonPose {
    int personID;
    std::vector<PoseKeypoint> keypoints;
    float overallConfidence;
    unsigned long timestamp;
    bool isValid;
    
    // Future 3D support
    ofPoint center3D;
    std::vector<float> jointDepths;
    
    PersonPose() : personID(-1), overallConfidence(0.0f), timestamp(0), isValid(false) {}
};

struct PoseCrossingEvent {
    int personID;
    std::string jointName;
    int lineID;
    ofPoint crossingPoint;
    float confidence;
    unsigned long timestamp;
    std::string direction;
    
    // Future 3D support
    ofPoint position3D;
    float depth;
    
    PoseCrossingEvent() : personID(-1), lineID(-1), confidence(0.0f), timestamp(0) {}
};

// Define the connections between joints for skeleton visualization
struct JointConnection {
    std::string joint1;
    std::string joint2;
    
    JointConnection(const std::string& j1, const std::string& j2) : joint1(j1), joint2(j2) {}
};

// Helper class to define the skeleton structure
class PoseSkeletonDefinition {
public:
    static std::vector<JointConnection> getConnections() {
        return {
            // Head connections
            {"nose", "leftEye"}, {"nose", "rightEye"},
            {"leftEye", "leftEar"}, {"rightEye", "rightEar"},
            
            // Torso connections
            {"leftShoulder", "rightShoulder"}, {"leftShoulder", "leftHip"},
            {"rightShoulder", "rightHip"}, {"leftHip", "rightHip"},
            
            // Arms
            {"leftShoulder", "leftElbow"}, {"leftElbow", "leftWrist"},
            {"rightShoulder", "rightElbow"}, {"rightElbow", "rightWrist"},
            
            // Legs
            {"leftHip", "leftKnee"}, {"leftKnee", "leftAnkle"},
            {"rightHip", "rightKnee"}, {"rightKnee", "rightAnkle"}
        };
    }
    
    static std::vector<std::string> getJointNames() {
        return {
            "nose", "leftEye", "rightEye", "leftEar", "rightEar",
            "leftShoulder", "rightShoulder", "leftElbow", "rightElbow",
            "leftWrist", "rightWrist", "leftHip", "rightHip",
            "leftKnee", "rightKnee", "leftAnkle", "rightAnkle"
        };
    }
};