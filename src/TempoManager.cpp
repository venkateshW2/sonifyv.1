#include "TempoManager.h"

TempoManager::TempoManager() {
    setDefaults();
}

TempoManager::~TempoManager() {
    // Cleanup if needed
}

void TempoManager::setup() {
    startTime = ofGetElapsedTimef();
    isRunning = true;
    ofLogNotice() << "TempoManager: Setup complete - BPM: " << globalBPM << ", Subdivision: " << subdivisionBeats << ", Swing: " << swingRatio;
}

void TempoManager::start() {
    if (!isRunning) {
        startTime = ofGetElapsedTimef();
        isRunning = true;
        ofLogNotice() << "TempoManager: Started";
    }
}

void TempoManager::stop() {
    isRunning = false;
    ofLogNotice() << "TempoManager: Stopped";
}

void TempoManager::reset() {
    startTime = ofGetElapsedTimef();
    ofLogNotice() << "TempoManager: Reset beat timing";
}

void TempoManager::setBPM(float bpm) {
    globalBPM = bpm;
    clampBPM();
    ofLogNotice() << "TempoManager: BPM set to " << globalBPM;
}

void TempoManager::setSubdivision(float beats) {
    subdivisionBeats = beats;
    clampSubdivision();
    ofLogNotice() << "TempoManager: Subdivision set to " << subdivisionBeats;
}

void TempoManager::setSwingRatio(float ratio) {
    swingRatio = ratio;
    clampSwingRatio();
    ofLogNotice() << "TempoManager: Swing ratio set to " << swingRatio;
}

float TempoManager::getCurrentBeat(float currentTime) {
    if (!isRunning) return 0.0f;
    
    float elapsedSeconds = currentTime - startTime;
    float secondsPerBeat = getSecondsPerBeat();
    return elapsedSeconds / secondsPerBeat;
}

float TempoManager::getClosestBeatTime(float currentTime) {
    if (!isRunning) return currentTime;
    
    float currentBeat = getCurrentBeat(currentTime);
    float subdivisionInterval = 4.0f / subdivisionBeats;  // Convert subdivision to beat fraction
    
    // Find the closest subdivision boundary
    float quantizedBeat = round(currentBeat / subdivisionInterval) * subdivisionInterval;
    
    return startTime + (quantizedBeat * getSecondsPerBeat());
}

bool TempoManager::isOnBeat(float currentTime, float tolerance) {
    float closestBeatTime = getClosestBeatTime(currentTime);
    return abs(currentTime - closestBeatTime) <= tolerance;
}

float TempoManager::getNextBeatTime(float currentTime) {
    float currentBeat = getCurrentBeat(currentTime);
    float subdivisionInterval = 4.0f / subdivisionBeats;
    float nextBeat = ceil(currentBeat / subdivisionInterval) * subdivisionInterval;
    
    return startTime + (nextBeat * getSecondsPerBeat());
}

int TempoManager::getBeatIndexForTime(float currentTime) {
    float currentBeat = getCurrentBeat(currentTime);
    float subdivisionInterval = 4.0f / subdivisionBeats;
    
    return (int)floor(currentBeat / subdivisionInterval);
}

float TempoManager::getSecondsPerBeat() const {
    return 60.0f / globalBPM;  // Convert BPM to seconds per quarter note
}

float TempoManager::applySwing(float beatPosition) {
    if (swingRatio == 0.5f) return beatPosition;  // No swing
    
    // Apply swing to eighth note subdivisions
    float beatFraction = beatPosition - floor(beatPosition);
    
    if (beatFraction < 0.5f) {
        // First half of beat - lengthen
        return floor(beatPosition) + (beatFraction * 2.0f * swingRatio);
    } else {
        // Second half of beat - compress
        float secondHalf = (beatFraction - 0.5f) * 2.0f;
        return floor(beatPosition) + swingRatio + (secondHalf * (1.0f - swingRatio));
    }
}

bool TempoManager::isOnSwingBeat(float currentTime, float tolerance) {
    float currentBeat = getCurrentBeat(currentTime);
    float swungBeat = applySwing(currentBeat);
    float closestSwungBeat = round(swungBeat);
    
    return abs(swungBeat - closestSwungBeat) <= tolerance;
}

// Configuration methods
void TempoManager::saveToJSON(ofxJSONElement& json) {
    json["globalBPM"] = globalBPM;
    json["subdivisionBeats"] = subdivisionBeats;
    json["swingRatio"] = swingRatio;
    json["isRunning"] = isRunning;
}

void TempoManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("globalBPM")) {
        globalBPM = json["globalBPM"].asFloat();
        clampBPM();
    }
    if (json.isMember("subdivisionBeats")) {
        subdivisionBeats = json["subdivisionBeats"].asFloat();
        clampSubdivision();
    }
    if (json.isMember("swingRatio")) {
        swingRatio = json["swingRatio"].asFloat();
        clampSwingRatio();
    }
    if (json.isMember("isRunning")) {
        isRunning = json["isRunning"].asBool();
    }
    
    ofLogNotice() << "TempoManager: Configuration loaded - BPM: " << globalBPM << ", Subdivision: " << subdivisionBeats << ", Swing: " << swingRatio;
}

void TempoManager::setDefaults() {
    globalBPM = 120.0f;
    subdivisionBeats = 4.0f;  // Quarter notes
    swingRatio = 0.5f;        // Straight timing
    startTime = 0.0f;
    isRunning = false;
    
    ofLogNotice() << "TempoManager: Set to default values";
}

// Helper methods
float TempoManager::getElapsedBeats(float currentTime) {
    if (!isRunning) return 0.0f;
    return (currentTime - startTime) / getSecondsPerBeat();
}

float TempoManager::beatTimeToSeconds(float beatTime) {
    return startTime + (beatTime * getSecondsPerBeat());
}

float TempoManager::secondsToBeatTime(float seconds) {
    return (seconds - startTime) / getSecondsPerBeat();
}

void TempoManager::clampBPM() {
    globalBPM = ofClamp(globalBPM, 40.0f, 200.0f);
}

void TempoManager::clampSubdivision() {
    // Allow common subdivisions: 1, 2, 4, 8, 16
    vector<float> validSubdivisions = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
    
    // Find closest valid subdivision
    float minDist = abs(subdivisionBeats - validSubdivisions[0]);
    float closest = validSubdivisions[0];
    
    for (float valid : validSubdivisions) {
        float dist = abs(subdivisionBeats - valid);
        if (dist < minDist) {
            minDist = dist;
            closest = valid;
        }
    }
    
    subdivisionBeats = closest;
}

void TempoManager::clampSwingRatio() {
    swingRatio = ofClamp(swingRatio, 0.5f, 0.75f);
}