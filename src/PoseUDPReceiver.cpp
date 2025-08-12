#include "PoseUDPReceiver.h"

PoseUDPReceiver::PoseUDPReceiver() {
    connected = false;
    shouldStop = false;
    hasNewPoseData = false;
    framesReceived = 0;
    lastUpdateTime = 0.0;
    currentFPS = 0.0f;
    lastFPSUpdate = 0.0;
    framesSinceLastFPS = 0;
    confidenceThreshold = 0.5f;
    udpPort = 8080;
    hostAddress = "localhost";
    statusMessage = "Not connected";
}

PoseUDPReceiver::~PoseUDPReceiver() {
    close();
}

bool PoseUDPReceiver::setup(int port, const std::string& host) {
    udpPort = port;
    hostAddress = host;
    
    // Setup UDP receiver
    bool success = udpReceiver.Create();
    if (!success) {
        setStatus("Failed to create UDP socket");
        return false;
    }
    
    success = udpReceiver.Bind(port);
    if (!success) {
        setStatus("Failed to bind to port " + ofToString(port));
        return false;
    }
    
    udpReceiver.SetNonBlocking(true);
    
    // Start receiver thread
    shouldStop = false;
    connected = true;
    receiverThread = std::thread(&PoseUDPReceiver::receiverThreadFunction, this);
    
    setStatus("Connected on port " + ofToString(port));
    ofLogNotice("PoseUDPReceiver") << "Setup complete on port " << port;
    
    return true;
}

void PoseUDPReceiver::close() {
    if (connected) {
        shouldStop = true;
        connected = false;
        
        if (receiverThread.joinable()) {
            receiverThread.join();
        }
        
        udpReceiver.Close();
        setStatus("Disconnected");
        ofLogNotice("PoseUDPReceiver") << "Connection closed";
    }
}

PoseFrameData PoseUDPReceiver::getLatestPoseData() {
    std::lock_guard<std::mutex> lock(dataMutex);
    hasNewPoseData = false;
    return latestPoseData;
}

std::vector<PersonPose> PoseUDPReceiver::getCurrentPoses() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return latestPoseData.poses;
}

void PoseUDPReceiver::update() {
    updateStatistics();
}

void PoseUDPReceiver::receiverThreadFunction() {
    char buffer[65536]; // 64KB buffer
    
    while (!shouldStop) {
        // Try to receive data
        int bytesReceived = udpReceiver.Receive(buffer, sizeof(buffer) - 1);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null terminate
            std::string jsonString(buffer);
            
            PoseFrameData frameData;
            if (parseJSONPoseData(jsonString, frameData)) {
                // Update shared data
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    latestPoseData = frameData;
                    hasNewPoseData = true;
                }
                
                framesReceived++;
                lastUpdateTime = ofGetElapsedTimef();
                setStatus("Receiving pose data (" + ofToString(frameData.poses.size()) + " people)");
            } else {
                ofLogWarning("PoseUDPReceiver") << "Failed to parse JSON data";
            }
        } else {
            // No data available, sleep briefly to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    ofLogNotice("PoseUDPReceiver") << "Receiver thread stopped";
}

bool PoseUDPReceiver::parseJSONPoseData(const std::string& jsonString, PoseFrameData& frameData) {
    try {
        ofxJSONElement json;
        if (!json.parse(jsonString)) {
            return false;
        }
        
        // Parse timestamp
        if (json.isMember("timestamp")) {
            frameData.timestamp = json["timestamp"].asDouble();
        }
        
        // Parse frame size
        if (json.isMember("frame_size")) {
            frameData.frameWidth = json["frame_size"]["width"].asInt();
            frameData.frameHeight = json["frame_size"]["height"].asInt();
        }
        
        // Parse poses
        frameData.poses.clear();
        if (json.isMember("poses") && json["poses"].isArray()) {
            for (int i = 0; i < json["poses"].size(); i++) {
                const auto& poseJson = json["poses"][i];
                
                // Get person info
                int personId = poseJson["person_id"].asInt();
                float confidence = poseJson["confidence"].asFloat();
                
                // Parse landmarks
                std::vector<PoseLandmark> landmarks;
                if (poseJson.isMember("landmarks") && poseJson["landmarks"].isArray()) {
                    for (int j = 0; j < poseJson["landmarks"].size(); j++) {
                        PoseLandmark landmark = parseLandmark(poseJson["landmarks"][j]);
                        
                        // Only include landmarks above confidence threshold
                        if (landmark.confidence >= confidenceThreshold) {
                            landmarks.push_back(landmark);
                        }
                    }
                }
                
                // Create person pose
                if (!landmarks.empty()) {
                    PersonPose pose(personId, landmarks, confidence);
                    frameData.poses.push_back(pose);
                }
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        ofLogError("PoseUDPReceiver") << "JSON parsing error: " << e.what();
        return false;
    }
}

PoseLandmark PoseUDPReceiver::parseLandmark(const ofxJSONElement& landmarkJson) {
    PoseLandmark landmark;
    
    if (landmarkJson.isMember("id")) {
        landmark.id = landmarkJson["id"].asInt();
    }
    if (landmarkJson.isMember("x")) {
        landmark.x = landmarkJson["x"].asFloat();
    }
    if (landmarkJson.isMember("y")) {
        landmark.y = landmarkJson["y"].asFloat();
    }
    if (landmarkJson.isMember("z")) {
        landmark.z = landmarkJson["z"].asFloat();
    }
    if (landmarkJson.isMember("confidence")) {
        landmark.confidence = landmarkJson["confidence"].asFloat();
    }
    if (landmarkJson.isMember("joint_name")) {
        landmark.jointName = landmarkJson["joint_name"].asString();
    }
    
    return landmark;
}

void PoseUDPReceiver::updateStatistics() {
    double currentTime = ofGetElapsedTimef();
    framesSinceLastFPS++;
    
    // Update FPS every second
    if (currentTime - lastFPSUpdate >= 1.0) {
        currentFPS = framesSinceLastFPS / (currentTime - lastFPSUpdate);
        framesSinceLastFPS = 0;
        lastFPSUpdate = currentTime;
    }
}

void PoseUDPReceiver::setStatus(const std::string& message) {
    statusMessage = message;
}