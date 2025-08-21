#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class TempoManager {
private:
    float globalBPM = 120.0f;                    // Master tempo (40-200 BPM)
    float subdivisionBeats = 4.0f;               // 4=quarter, 8=eighth, 16=sixteenth notes  
    float swingRatio = 0.5f;                     // 0.5=straight, 0.67=swing feel
    float startTime = 0.0f;                      // Reference start time for beat calculations
    bool isRunning = false;                      // Tempo system active state
    
public:
    TempoManager();
    ~TempoManager();
    
    void setup();
    void start();
    void stop();
    void reset();
    
    // Core tempo methods
    float getBPM() const { return globalBPM; }
    void setBPM(float bpm);
    float getSubdivision() const { return subdivisionBeats; }
    void setSubdivision(float beats);
    float getSwingRatio() const { return swingRatio; }
    void setSwingRatio(float ratio);
    bool getIsRunning() const { return isRunning; }
    
    // Beat calculation methods
    float getCurrentBeat(float currentTime);
    float getClosestBeatTime(float currentTime);
    bool isOnBeat(float currentTime, float tolerance = 0.05f);
    float getNextBeatTime(float currentTime);
    int getBeatIndexForTime(float currentTime);
    float getSecondsPerBeat() const;
    
    // Swing timing support
    float applySwing(float beatPosition);
    bool isOnSwingBeat(float currentTime, float tolerance = 0.05f);
    
    // Configuration methods
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
private:
    // Helper methods
    float getElapsedBeats(float currentTime);
    float beatTimeToSeconds(float beatTime);
    float secondsToBeatTime(float seconds);
    void clampBPM();
    void clampSubdivision();
    void clampSwingRatio();
};