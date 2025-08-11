#pragma once

#include "ofMain.h"
#include "CoreMLDetector.h"
#include "ofxJSON.h"

class DetectionManager {
public:
    DetectionManager();
    ~DetectionManager();
    
    void setup();
    void update();
    void draw();
    
    // Forward declaration 
    struct Detection;
    
    // Core detection methods - EXACT COPY from working backup
    void loadCoreMLModel();
    void processCoreMLDetection();
    void drawDetections();
    void initializeCategories();
    void applyNMS(const vector<Detection>& rawDetections, vector<Detection>& filteredDetections, float nmsThreshold);
    float calculateIoU(const ofRectangle& box1, const ofRectangle& box2);
    
    // Category methods - EXACT COPY from working backup
    void applyPreset(const string& presetName);
    void updateEnabledClassesFromSelection();
    string getClassNameById(int classId);
    
    bool shouldProcess() const { return enableDetection && yoloLoaded; }
    void toggleDetection() { enableDetection = !enableDetection; }
    
    // EXACT same detection data structures as working backup
    struct Detection {
        ofRectangle box;
        float confidence;
        int classId;
        string className;
    };
    
    // Vehicle tracking and line crossing system - EXACT COPY from working backup
    struct TrackedVehicle {
        int id;
        ofRectangle currentBox;
        ofRectangle previousBox;
        ofPoint centerCurrent;
        ofPoint centerPrevious;
        int vehicleType;
        string className;
        float confidence;
        int framesSinceLastSeen;
        bool hasMovement;
        float speed;           // Pixels per frame
        float speedMph;        // Estimated MPH (rough approximation)
        
        // Enhanced tracking features
        vector<ofPoint> trajectory;        // History of center positions
        vector<float> trajectoryTimes;     // Timestamps for each position
        ofPoint velocity;                  // Current velocity vector (vx, vy)
        float acceleration;                // Change in speed magnitude
        ofColor trailColor;               // Visual trail color
        bool isOccluded;                  // Currently not detected but still tracked
        float predictionConfidence;       // Confidence in trajectory prediction
        int maxTrajectoryLength;          // Maximum trajectory history to keep
        
        // Constructor for initialization
        TrackedVehicle() : acceleration(0.0f), isOccluded(false),
                          predictionConfidence(0.0f), maxTrajectoryLength(30) {}
    };
    
    struct LineCrossEvent {
        int lineId;
        int vehicleId;
        int vehicleType;
        string className;
        float confidence;
        float speed;        // pixels per frame
        float speedMph;     // estimated MPH
        unsigned long timestamp;
        ofPoint crossingPoint;
        bool processed;
    };
    
    // Detection categories - EXACT COPY from working backup
    enum DetectionCategory {
        CATEGORY_VEHICLES = 0,
        CATEGORY_PEOPLE = 1,
        CATEGORY_ANIMALS = 2,
        CATEGORY_OBJECTS = 3,
        CATEGORY_COUNT = 4
    };
    
    DetectionCategory getCategoryForClass(int classId);
    vector<int> getClassesInCategory(DetectionCategory category);
    
    // EXACT same detection variables as working backup
    vector<Detection> detections;
    vector<string> classNames;
    vector<bool> enabledClasses;
    vector<bool> categoryEnabled;
    vector<int> selectedClassIds;
    CoreMLDetector* detector;
    
    // Vehicle tracking and line crossing variables - EXACT COPY from working backup
    vector<TrackedVehicle> trackedVehicles;
    vector<LineCrossEvent> crossingEvents;
    int nextVehicleId;
    float vehicleTrackingThreshold;
    int maxFramesWithoutDetection;
    
    bool yoloLoaded;
    bool enableDetection;
    bool showDetections;
    int frameSkipCounter;
    int detectionFrameSkip;
    float lastDetectionTime;
    int detectionErrorCount;
    float displayScale;
    
    // Category system - EXACT COPY from working backup
    string currentPreset;
    int maxSelectedClasses;
    
    // Need access to video source for detection processing  
    int currentVideoSource; // Use int instead of enum to avoid conflicts
    
    // VideoManager access methods (called from ofApp)
    void setVideoSource(int source) { currentVideoSource = source; }
    
    // Need access to video managers for pixel extraction
    void setVideoManagers(class VideoManager* videoMgr) { videoManager = videoMgr; }
    
    // Need access to line and communication managers for line crossing
    void setLineManager(class LineManager* lineMgr) { lineManager = lineMgr; }
    void setCommunicationManager(class CommunicationManager* commMgr) { communicationManager = commMgr; }
    
    // Configuration methods
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // Getters/setters for UI Manager
    bool isDetectionEnabled() const { return enableDetection; }
    void setDetectionEnabled(bool enabled) { enableDetection = enabled; }
    float getConfidenceThreshold() const { return confidenceThreshold; }
    void setConfidenceThreshold(float threshold) { confidenceThreshold = threshold; }
    int getDetectionFrameSkip() const { return detectionFrameSkip; }
    void setDetectionFrameSkip(int frameSkip) { detectionFrameSkip = frameSkip; }
    
    // UI Manager methods for Detection Classes tab
    string getCurrentPreset() const { return currentPreset; }
    void setCurrentPreset(const string& preset) { currentPreset = preset; }
    vector<int> getSelectedClassIds() const { return selectedClassIds; }
    int getMaxSelectedClasses() const { return maxSelectedClasses; }
    bool getCategoryEnabled(DetectionCategory category) const { return categoryEnabled[category]; }
    void setCategoryEnabled(DetectionCategory category, bool enabled) { categoryEnabled[category] = enabled; }
    void addSelectedClass(int classId);
    void removeSelectedClass(int classId);
    void removeClassesByCategory(DetectionCategory category);
    
    // Live tracking data getters for UI Manager
    int getTrackedVehiclesCount() const { return trackedVehicles.size(); }
    int getVisibleVehiclesCount() const;
    int getOccludedVehiclesCount() const;
    const vector<TrackedVehicle>& getTrackedVehicles() const { return trackedVehicles; }
    int getCrossingEventsCount() const { return crossingEvents.size(); }
    
    // Vehicle tracking and line crossing methods - EXACT COPY from working backup
    void updateVehicleTracking();
    void checkLineCrossings();
    
    // Safe versions of vehicle tracking methods
    void updateVehicleTrackingSafe();
    void checkLineCrossingsSafe();
    void cleanupOldVehicles();
    float calculateDistance(const ofPoint& p1, const ofPoint& p2);
    bool lineSegmentIntersection(const ofPoint& line1Start, const ofPoint& line1End, 
                                const ofPoint& line2Start, const ofPoint& line2End, 
                                ofPoint& intersection);
    void updateTrajectoryHistory(TrackedVehicle& vehicle);
    void calculateVelocityAndAcceleration(TrackedVehicle& vehicle);
    void handleOccludedVehicles();
    void cleanupOldTrajectoryPoints();
    
private:
    class VideoManager* videoManager;
    class LineManager* lineManager;
    class CommunicationManager* communicationManager;
    float confidenceThreshold;  // Add missing member variable
};