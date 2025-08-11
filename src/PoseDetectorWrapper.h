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
    
    // Helper method to convert Objective-C poses to C++ poses
    std::vector<PersonPose> convertToCppPoses(void* nsPoses);
};