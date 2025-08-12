#pragma once

#include "ofMain.h"
#include <string>
#include <vector>

// MediaPipe pose landmark structure (updated for JSON format)
struct PoseLandmark {
    int id;
    float x, y, z;          // Coordinates (x,y in pixels, z relative depth)
    float confidence;       // Detection confidence [0-1]
    std::string jointName;  // Human-readable joint name
    
    PoseLandmark() : id(0), x(0), y(0), z(0), confidence(0.0f), jointName("") {}
    
    PoseLandmark(int _id, float _x, float _y, float _z, float _conf, const std::string& _name)
        : id(_id), x(_x), y(_y), z(_z), confidence(_conf), jointName(_name) {}
    
    // Convert to ofPoint for existing line crossing code
    ofPoint toOfPoint() const {
        return ofPoint(x, y);
    }
    
    // Convert to normalized coordinates [0-1] for line crossing detection
    ofVec2f toNormalized(int frameWidth, int frameHeight) const {
        return ofVec2f(x / frameWidth, y / frameHeight);
    }
    
    // Convert to screen coordinates
    ofVec2f toScreen(int screenWidth, int screenHeight) const {
        return ofVec2f(x * screenWidth / 1920.0f, y * screenHeight / 1080.0f);
    }
};

// Legacy structure for compatibility (convert from PoseLandmark)
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
    
    // Constructor from MediaPipe landmark
    PoseKeypoint(const PoseLandmark& landmark) {
        jointName = landmark.jointName;
        position = landmark.toOfPoint();
        confidence = landmark.confidence;
        isVisible = (confidence > 0.5f);
        depth = landmark.z;
        position3D = ofPoint(landmark.x, landmark.y, landmark.z);
        velocity = ofPoint(0, 0); // Will be calculated in tracking
    }
};

struct PersonPose {
    int personID;
    std::vector<PoseKeypoint> keypoints;    // Legacy format for compatibility
    std::vector<PoseLandmark> landmarks;    // MediaPipe format
    float overallConfidence;
    unsigned long timestamp;
    bool isValid;
    
    // Future 3D support
    ofPoint center3D;
    std::vector<float> jointDepths;
    
    PersonPose() : personID(-1), overallConfidence(0.0f), timestamp(0), isValid(false) {}
    
    // Constructor from MediaPipe landmarks
    PersonPose(int id, const std::vector<PoseLandmark>& _landmarks, float confidence) {
        personID = id;
        landmarks = _landmarks;
        overallConfidence = confidence;
        timestamp = ofGetElapsedTimeMillis();
        isValid = true;
        
        // Convert to legacy keypoints for compatibility
        keypoints.clear();
        for (const auto& landmark : landmarks) {
            keypoints.push_back(PoseKeypoint(landmark));
        }
    }
    
    // Get specific landmark by joint name
    PoseLandmark* getLandmark(const std::string& jointName) {
        for (auto& landmark : landmarks) {
            if (landmark.jointName == jointName) {
                return &landmark;
            }
        }
        return nullptr;
    }
    
    // Get landmarks with confidence above threshold
    std::vector<PoseLandmark> getHighConfidenceLandmarks(float threshold = 0.5f) const {
        std::vector<PoseLandmark> result;
        for (const auto& landmark : landmarks) {
            if (landmark.confidence >= threshold) {
                result.push_back(landmark);
            }
        }
        return result;
    }
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

// Complete pose detection frame data from MediaPipe UDP
struct PoseFrameData {
    double timestamp;
    int frameWidth, frameHeight;
    std::vector<PersonPose> poses;
    
    PoseFrameData() : timestamp(0.0), frameWidth(0), frameHeight(0) {}
    
    // Check if we have any valid poses
    bool hasPoses() const {
        return !poses.empty();
    }
    
    // Get total number of detected landmarks across all people
    int getTotalLandmarks() const {
        int total = 0;
        for (const auto& pose : poses) {
            total += pose.landmarks.size();
        }
        return total;
    }
    
    // Get the most confident person
    PersonPose* getBestPose() {
        if (poses.empty()) return nullptr;
        
        PersonPose* best = &poses[0];
        for (auto& pose : poses) {
            if (pose.overallConfidence > best->overallConfidence) {
                best = &pose;
            }
        }
        return best;
    }
};