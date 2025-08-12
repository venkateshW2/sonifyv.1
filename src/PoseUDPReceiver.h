#pragma once

#include "ofMain.h"
#include "ofxUDPManager.h"
#include "ofxJSON.h"
#include "PoseStructures.h"
#include <thread>
#include <mutex>
#include <atomic>

class PoseUDPReceiver {
public:
    PoseUDPReceiver();
    ~PoseUDPReceiver();
    
    // Setup and connection management
    bool setup(int port = 8080, const std::string& host = "localhost");
    void close();
    bool isConnected() const { return connected; }
    
    // Data access (thread-safe)
    bool hasNewData() const { return hasNewPoseData; }
    PoseFrameData getLatestPoseData();
    std::vector<PersonPose> getCurrentPoses();
    
    // Statistics and monitoring
    int getFramesReceived() const { return framesReceived; }
    float getFPS() const { return currentFPS; }
    double getLastUpdateTime() const { return lastUpdateTime; }
    std::string getStatusMessage() const { return statusMessage; }
    
    // Update (call from main thread)
    void update();
    
    // Configuration
    void setConfidenceThreshold(float threshold) { confidenceThreshold = threshold; }
    float getConfidenceThreshold() const { return confidenceThreshold; }
    
private:
    // Network
    ofxUDPManager udpReceiver;
    std::string hostAddress;
    int udpPort;
    std::atomic<bool> connected;
    
    // Threading
    std::thread receiverThread;
    std::atomic<bool> shouldStop;
    std::mutex dataMutex;
    
    // Data storage
    PoseFrameData currentPoseData;
    PoseFrameData latestPoseData;
    std::atomic<bool> hasNewPoseData;
    
    // Statistics
    std::atomic<int> framesReceived;
    std::atomic<double> lastUpdateTime;
    float currentFPS;
    double lastFPSUpdate;
    int framesSinceLastFPS;
    
    // Configuration
    float confidenceThreshold;
    
    // Status
    std::string statusMessage;
    
    // Methods
    void receiverThreadFunction();
    bool parseJSONPoseData(const std::string& jsonString, PoseFrameData& frameData);
    PoseLandmark parseLandmark(const ofxJSONElement& landmarkJson);
    void updateStatistics();
    void setStatus(const std::string& message);
};