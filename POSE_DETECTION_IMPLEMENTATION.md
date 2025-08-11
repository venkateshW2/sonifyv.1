# SonifyV1 Pose Detection Implementation Guide

## Overview
This document provides a complete implementation guide for adding 2D pose detection to the SonifyV1 highway sonification system using Apple's Vision framework. The implementation is designed to be modular and future-proof, with data structures that can accommodate 3D depth information for future OAK-D-Lite integration.

## Architecture

### Modular Design
The pose detection system is implemented as a separate module that integrates with the existing SonifyV1 codebase without breaking current functionality.

```
SonifyV1 Enhanced System:
├── Core Application (unchanged)
├── Object Detection (existing)
├── MIDI System (existing)
├── OSC Communication (existing)
└── NEW: Pose Detection Module
    ├── Vision Framework Integration
    ├── Pose Data Structures
    ├── Skeleton Visualization
    ├── Line Crossing Detection
    ├── Pose-MIDI Integration
    └── Configuration System
```

## File Structure

### New Files to Create
1. **PoseDetector.h** - Objective-C interface for Vision framework
2. **PoseDetector.mm** - Objective-C implementation
3. **PoseStructures.h** - C++ data structures for pose data
4. **PoseDetectorWrapper.h** - C++ wrapper for Objective-C classes
5. **PoseDetectorWrapper.mm** - C++ wrapper implementation

### Integration Points
- **ofApp.h** - Add pose detection members and methods
- **ofApp.cpp** - Integrate pose detection into main application flow

## Implementation Details

### 1. PoseDetector.h (Objective-C Interface)

```objc
// PoseDetector.h
#import <Foundation/Foundation.h>

@interface PoseKeypoint : NSObject
@property (nonatomic, assign) float x;
@property (nonatomic, assign) float y;
@property (nonatomic, assign) float confidence;
@property (nonatomic, strong) NSString* jointName;
@property (nonatomic, assign) BOOL isVisible;
@property (nonatomic, assign) float velocityX;
@property (nonatomic, assign) float velocityY;
@end

@interface PersonPose : NSObject
@property (nonatomic, assign) int personID;
@property (nonatomic, strong) NSArray<PoseKeypoint*>* keypoints;
@property (nonatomic, assign) float overallConfidence;
@property (nonatomic, assign) unsigned long timestamp;
@property (nonatomic, assign) BOOL isValid;
@end

@interface PoseDetector : NSObject
- (instancetype)init;
- (void)detectPosesInPixels:(unsigned char*)pixelData
                      width:(int)width
                     height:(int)height
                   channels:(int)channels
                 completion:(void(^)(NSArray<PersonPose*>* poses))completion;
- (void)setConfidenceThreshold:(float)threshold;
- (void)setMaxPeople:(int)maxPeople;
- (NSArray<NSString*>*)getJointNames;
@end
```

### 2. PoseStructures.h (C++ Data Structures)

```cpp
// PoseStructures.h
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
```

### 3. PoseDetectorWrapper.h (C++ Wrapper)

```cpp
// PoseDetectorWrapper.h
#pragma once

#include "ofMain.h"
#include "PoseStructures.h"

class PoseDetectorWrapper {
public:
    PoseDetectorWrapper();
    ~PoseDetectorWrapper();
    
    bool setup();
    std::vector<PersonPose> detectPoses(ofPixels& pixels);
    void detectPosesAsync(ofPixels& pixels, std::function<void(std::vector<PersonPose>)> callback);
    
    void setConfidenceThreshold(float threshold);
    void setMaxPeople(int maxPeople);
    std::vector<std::string> getJointNames();
    
private:
    class Impl;
    Impl* impl;
};
```

## Integration Steps

### Step 1: Add to ofApp.h

Add the following to your existing ofApp.h file:

```cpp
// Add new includes
#include "PoseStructures.h"
#include "PoseDetectorWrapper.h"

// Add to private section
private:
    // Pose detection system
    PoseDetectorWrapper poseDetector;
    bool poseDetectionEnabled;
    float poseConfidenceThreshold;
    int maxPeopleToDetect;
    
    vector<PersonPose> detectedPoses;
    vector<PoseCrossingEvent> poseCrossingEvents;
    
    // Pose visualization
    bool showSkeletonOverlay;
    bool showKeypointTrails;
    bool showPoseLabels;
    
    // Pose methods
    void setupPoseDetection();
    void updatePoseDetection();
    void drawPoseDetection();
    void checkPoseLineCrossings();
    void sendPoseOSCMessage(const PoseCrossingEvent& event);
    void sendPoseMIDIMessage(const PoseCrossingEvent& event);
    void drawPoseDetectionTab();
```

### Step 2: Update ofApp.cpp

#### Setup Method
```cpp
void ofApp::setup() {
    // Existing setup code...
    
    // Initialize pose detection
    setupPoseDetection();
}

void ofApp::setupPoseDetection() {
    poseDetector.setup();
    poseDetector.setConfidenceThreshold(poseConfidenceThreshold);
    poseDetector.setMaxPeople(maxPeopleToDetect);
    
    // Default visualization settings
    showSkeletonOverlay = true;
    showKeypointTrails = false;
    showPoseLabels = true;
}
```

#### Update Method
```cpp
void ofApp::update() {
    // Existing update code...
    
    if (poseDetectionEnabled) {
        updatePoseDetection();
    }
}

void ofApp::updatePoseDetection() {
    // Get current frame pixels
    ofPixels pixels;
    if (currentVideoSource == CAMERA) {
        pixels = camera.getPixels();
    } else if (currentVideoSource == VIDEO_FILE) {
        pixels = videoPlayer.getPixels();
    } else if (currentVideoSource == IP_CAMERA && ipFrameReady) {
        pixels = currentIPFrame.getPixels();
    } else {
        return;
    }
    
    // Detect poses asynchronously
    poseDetector.detectPosesAsync(pixels, [this](vector<PersonPose> poses) {
        this->detectedPoses = poses;
        this->checkPoseLineCrossings();
    });
}
```

#### Draw Method
```cpp
void ofApp::draw() {
    // Existing draw code...
    
    if (poseDetectionEnabled) {
        drawPoseDetection();
    }
}

void ofApp::drawPoseDetection() {
    if (detectedPoses.empty()) return;
    
    if (showSkeletonOverlay) {
        for (const auto& pose : detectedPoses) {
            if (!pose.isValid) continue;
            
            // Draw skeleton connections
            auto connections = PoseSkeletonDefinition::getConnections();
            for (const auto& connection : connections) {
                // Find and draw connections between visible joints
                // Implementation details...
            }
            
            // Draw keypoints
            for (const auto& keypoint : pose.keypoints) {
                if (!keypoint.isVisible) continue;
                
                ofColor color;
                if (keypoint.confidence > 0.8f) color = ofColor::green;
                else if (keypoint.confidence > 0.5f) color = ofColor::yellow;
                else color = ofColor::red;
                
                ofSetColor(color);
                ofDrawCircle(keypoint.position, 5);
            }
        }
    }
}
```

### Step 3: Line Crossing Detection

```cpp
void ofApp::checkPoseLineCrossings() {
    for (const auto& pose : detectedPoses) {
        if (!pose.isValid) continue;
        
        for (const auto& keypoint : pose.keypoints) {
            if (!keypoint.isVisible) continue;
            
            for (int i = 0; i < lines.size(); i++) {
                // Check if keypoint crosses line
                float distance = distanceToLineSegment(keypoint.position, 
                                                     lines[i].startPoint, 
                                                     lines[i].endPoint);
                
                if (distance < 10.0f) {
                    PoseCrossingEvent event;
                    event.personID = pose.personID;
                    event.jointName = keypoint.jointName;
                    event.lineID = i;
                    event.crossingPoint = keypoint.position;
                    event.confidence = keypoint.confidence;
                    event.timestamp = ofGetElapsedTimeMillis();
                    
                    poseCrossingEvents.push_back(event);
                    sendPoseOSCMessage(event);
                    
                    if (midiEnabled) {
                        sendPoseMIDIMessage(event);
                    }
                }
            }
        }
    }
}
```

### Step 4: OSC Integration

```cpp
void ofApp::sendPoseOSCMessage(const PoseCrossingEvent& event) {
    if (!oscEnabled) return;
    
    ofxOscMessage msg;
    msg.setAddress("/pose_cross");
    msg.addIntArg(event.personID);
    msg.addStringArg(event.jointName);
    msg.addIntArg(event.lineID);
    msg.addFloatArg(event.crossingPoint.x);
    msg.addFloatArg(event.crossingPoint.y);
    msg.addFloatArg(event.confidence);
    msg.addStringArg(event.direction);
    
    oscSender.sendMessage(msg);
}
```

### Step 5: GUI Integration

Add a new tab to the existing GUI:

```cpp
void ofApp::drawPoseDetectionTab() {
    if (ImGui::BeginTabItem("Pose Detection")) {
        ImGui::Checkbox("Enable Pose Detection", &poseDetectionEnabled);
        
        ImGui::Text("Visualization Options");
        ImGui::Checkbox("Show Skeleton Overlay", &showSkeletonOverlay);
        ImGui::Checkbox("Show Keypoint Trails", &showKeypointTrails);
        ImGui::Checkbox("Show Pose Labels", &showPoseLabels);
        
        ImGui::Text("Detection Settings");
        ImGui::SliderFloat("Confidence Threshold", &poseConfidenceThreshold, 0.1f, 0.9f);
        ImGui::SliderInt("Max People", &maxPeopleToDetect, 1, 8);
        
        ImGui::Text("Current Status");
        ImGui::Text("Detected People: %d", detectedPoses.size());
        
        ImGui::EndTabItem();
    }
}
```

### Step 6: Configuration System

```cpp
void ofApp::saveConfig() {
    ofJson config = ofLoadJson(getConfigPath());
    
    // Add pose detection settings
    config["poseDetection"]["enabled"] = poseDetectionEnabled;
    config["poseDetection"]["confidenceThreshold"] = poseConfidenceThreshold;
    config["poseDetection"]["maxPeople"] = maxPeopleToDetect;
    config["poseDetection"]["showSkeletonOverlay"] = showSkeletonOverlay;
    config["poseDetection"]["showKeypointTrails"] = showKeypointTrails;
    config["poseDetection"]["showPoseLabels"] = showPoseLabels;
    
    ofSavePrettyJson(getConfigPath(), config);
}

void ofApp::loadConfig() {
    ofJson config = ofLoadJson(getConfigPath());
    
    if (config.contains("poseDetection")) {
        poseDetectionEnabled = config["poseDetection"]["enabled"];
        poseConfidenceThreshold = config["poseDetection"]["confidenceThreshold"];
        maxPeopleToDetect = config["poseDetection"]["maxPeople"];
        showSkeletonOverlay = config["poseDetection"]["showSkeletonOverlay"];
        showKeypointTrails = config["poseDetection"]["showKeypointTrails"];
        showPoseLabels = config["poseDetection"]["showPoseLabels"];
    }
}
```

## Build Configuration


1. Add new files to the project
2. Link Vision framework: `Build Phases > Link Binary With Libraries > Vision.framework`
3. Ensure Objective-C++ compilation for .mm files

### Makefile Updates
Add the following to your Makefile:

```makefile
# Add new source files
SOURCES += src/PoseDetector.mm
SOURCES += src/PoseDetectorWrapper.mm

# Add Vision framework
ADDON_FRAMEWORKS += Vision
```

## Testing Checklist

### Functional Tests
- [ ] Pose detection initializes without errors
- [ ] Skeleton overlay displays correctly
- [ ] Line crossing detection works for keypoints
- [ ] OSC messages are sent on pose line crossings
- [ ] MIDI integration works with pose events
- [ ] Configuration save/load works correctly

### Performance Tests
- [ ] No frame drops with pose detection enabled
- [ ] Memory usage remains stable over time
- [ ] CPU usage increase is acceptable (<20%)
- [ ] Multi-person detection works (up to 8 people)

### Integration Tests
- [ ] Works with all video sources (camera, video file, IP camera)
- [ ] No conflicts with existing object detection
- [ ] GUI integration is smooth and responsive
- [ ] Configuration persistence works across restarts

## Future 3D Upgrade Path

The implementation is designed to easily accommodate 3D pose detection when upgrading to OAK-D-Lite:

1. **Data Structures**: Already include 3D fields (depth, position3D)
2. **Detection Pipeline**: Can be extended to use OAK-D-Lite instead of Vision framework
3. **Visualization**: Can be enhanced to show 3D skeletons
4. **Line Crossing**: Can be extended to 3D line crossing detection
5. **Audio**: Can be enhanced with spatial audio based on 3D positions

## Usage Instructions

### Basic Usage
1. Compile the application with the new pose detection files
2. Run the application
3. Press 'g' to open the GUI
4. Navigate to the "Pose Detection" tab
5. Enable pose detection
6. Draw lines as usual
7. Pose line crossings will trigger OSC/MIDI events

### Keyboard Shortcuts
- **'p'**: Toggle pose detection on/off
- **'g'**: Open GUI
- **'c'**: Clear all lines (affects both object and pose detection)

### OSC Message Format
```
/pose_cross personID jointName lineID x_pos y_pos confidence direction
Example: /pose_cross 1 "leftWrist" 0 245.5 123.7 0.89 "left_to_right"
```

## Troubleshooting

### Common Issues
1. **Vision framework not found**: Ensure Vision.framework is linked in Xcode
2. **Compilation errors**: Check that .mm files are compiled as Objective-C++
3. **No poses detected**: Check confidence threshold and ensure people are visible
4. **Performance issues**: Reduce maxPeopleToDetect or increase detectionFrameSkip

### Debug Information
Enable debug logging by adding:
```cpp
ofSetLogLevel(OF_LOG_VERBOSE);
```

## Conclusion

This implementation provides a complete 2D pose detection system that integrates seamlessly with the existing SonifyV1 application. The modular design ensures that the system can be easily upgraded to 3D pose detection with the OAK-D-Lite in the future without requiring major architectural changes.