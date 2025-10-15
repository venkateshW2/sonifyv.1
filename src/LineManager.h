#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class LineManager {
public:
    // EXACT COPY from working backup - MidiLine struct
    struct MidiLine {
        // Visual properties
        ofPoint startPoint;
        ofPoint endPoint;
        ofColor color;
        
        // MIDI properties (uses master scale system) - EXACT COPY
        int scaleNoteIndex;     // Index into master scale (0 = first note of scale, 1 = second, etc.)
        bool randomizeNote;     // Per-line randomization toggle (true = random, false = use scaleNoteIndex)
        int octave;             // 0-10 (default 4 for middle octave)
        int midiChannel;        // 1-16 (default 1)
        string midiPortName;    // Selected MIDI port name (default to first available)
        
        // Duration settings
        enum DurationType { DURATION_FIXED, SPEED_BASED, VEHICLE_BASED };
        DurationType durationType;
        int fixedDuration;      // milliseconds (default 500)
        
        // Velocity settings  
        enum VelocityType { VELOCITY_FIXED, CONFIDENCE_BASED };
        VelocityType velocityType;
        int fixedVelocity;      // 0-127 (default 100)
        
        // NEW: Tempo-based randomization settings
        bool enableTempoSync = true;                    // Enable/disable tempo sync for this line
        enum QuantizeMode { HARD_SNAP, GRADUAL_TRANSITION };
        QuantizeMode quantizeMode = HARD_SNAP;          // Quantization behavior
        float quantizeStrength = 1.0f;                  // 0.0 = no quantization, 1.0 = full quantization
        int randomSeed = 0;                             // Per-line consistent randomization seed
        float lastBeatTime = 0.0f;                      // Last quantized beat for this line
        int lastRandomNoteIndex = 0;                    // Last selected random note
        
        // Scale degree weighting for musical randomness
        vector<float> scaleDegreeWeights = {1.5f, 0.8f, 1.2f, 0.9f, 1.4f, 0.9f, 0.7f}; // Major scale weights
        
        // Constructor with sensible defaults
        MidiLine() : scaleNoteIndex(0), randomizeNote(true), octave(4), midiChannel(1), 
                     midiPortName(""), durationType(DURATION_FIXED), fixedDuration(500),
                     velocityType(VELOCITY_FIXED), fixedVelocity(100) {
            // Initialize scale degree weights for Major scale (root and fifth emphasized)
            scaleDegreeWeights = {1.5f, 0.8f, 1.2f, 0.9f, 1.4f, 0.9f, 0.7f};
            randomSeed = rand() % 1000; // Random seed for this line
        }
    };

    LineManager();
    ~LineManager();
    
    void setup();
    void update();
    void draw();
    
    // Line drawing methods - EXACT same as working backup
    void drawLines();
    
    // Mouse handling - EXACT same as working backup
    void handleMousePressed(int x, int y, int button);
    void handleMouseReleased(int x, int y, int button);
    void handleMouseDragged(int x, int y, int button);
    void handleMouseMoved(int x, int y);
    
    // Line management - EXACT same as working backup
    void clearAllLines();
    void selectLine(int index);
    void deleteSelectedLine();
    void duplicateSelectedLine();
    
    // Access methods - EXACT same as working backup
    int getLineCount() const { return lines.size(); }
    int getSelectedLineIndex() const { return selectedLineIndex; }
    const vector<MidiLine>& getLines() const { return lines; }
    vector<MidiLine>& getLines() { return lines; }
    MidiLine* getSelectedLine();
    
    // Master musical system - EXACT same as working backup
    int getMasterRootNote() const { return masterRootNote; }
    string getMasterScale() const { return masterScale; }
    void setMasterRootNote(int note) { masterRootNote = note; }
    void setMasterScale(const string& scale) { masterScale = scale; }
    
    // Musical methods - EXACT same as working backup
    vector<string> getAvailableScales();
    vector<string> getScaleNoteNames();
    vector<int> getScaleIntervals(const string& scaleName);
    int getMidiNoteFromMasterScale(int lineIndex);
    void initializeNewLineDefaults(MidiLine& line);
    
    // NEW: Tempo-synchronized randomization methods
    int getTempoSyncedRandomNote(int lineIndex, float currentTime);
    int getImmediateRandomNote(int lineIndex);  // Fallback for non-tempo mode
    int getImmediateRandomScaleIndex(int lineIndex);  // Returns scale index only
    int weightedRandomSelection(const vector<float>& weights);
    void setTempoManager(class TempoManager* tempoMgr) { tempoManager = tempoMgr; }
    
    // Window resize - EXACT same as working backup
    void rescaleLines(int oldWidth, int oldHeight, int newWidth, int newHeight);
    
    // Configuration methods  
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // State variables - EXACT COPY from working backup
    vector<MidiLine> lines;     // Enhanced line structure with MIDI properties
    ofPoint lineStart;
    bool isDrawingLine;
    bool showLines;  // Add this for GUI control
    
    // Master Musical System - EXACT COPY from working backup
    int masterRootNote;         // 0-11 (C=0, C#=1, etc.) - root key for all lines
    string masterScale;         // "Major", "Minor", "Pentatonic", "Blues", "Chromatic" - scale for all lines
    
    // Line selection system - EXACT COPY from working backup
    int selectedLineIndex;  // -1 means no line selected
    
    // Line editing system - EXACT COPY from working backup
    bool isDraggingEndpoint;
    int draggingLineIndex;
    bool isDraggingStartPoint;  // true = start point, false = end point
    
    int currentColorIndex;
    
    // NEW: TempoManager reference for tempo-synchronized randomization
    class TempoManager* tempoManager;

private:
    // Helper methods from working backup
    ofColor getNextLineColor();
    float distanceToLineSegment(const ofPoint& point, const ofPoint& lineStart, const ofPoint& lineEnd);
    int findClosestLine(const ofPoint& clickPoint, float threshold = 15.0f);
    bool isNearEndpoint(const ofPoint& clickPoint, int lineIndex, bool& isStartPoint, float threshold = 15.0f);
    void initializeMasterMusicalSystem();
};