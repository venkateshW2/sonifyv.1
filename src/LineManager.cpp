#include "LineManager.h"
#include "TempoManager.h"

LineManager::LineManager() {
    // EXACT initialization from working backup
    isDrawingLine = false;
    currentColorIndex = 0;
    selectedLineIndex = -1;
    showLines = true;
    
    // Line editing system - EXACT COPY
    isDraggingEndpoint = false;
    draggingLineIndex = -1;
    isDraggingStartPoint = false;
    
    // NEW: Initialize TempoManager reference
    tempoManager = nullptr;
    
    // Master Musical System - EXACT COPY
    initializeMasterMusicalSystem();
}

LineManager::~LineManager() {}

void LineManager::setup() {
    ofLogNotice() << "LineManager: Initialized with master musical system";
}

void LineManager::update() {
    // Minimal update - most logic is in mouse handling
}

void LineManager::draw() {
    if (showLines) {
        drawLines();
    }
}

// EXACT COPY from working backup
void LineManager::drawLines() {
    // Task 3.1: Draw MidiLine structures with enhanced visual feedback
    for (int i = 0; i < lines.size(); i++) {
        bool isSelected = (i == selectedLineIndex);
        
        // Draw selection highlight first (underneath)
        if (isSelected) {
            ofSetColor(255, 255, 255, 150);  // White highlight
            ofSetLineWidth(8);  // Thicker outline
            ofDrawLine(lines[i].startPoint, lines[i].endPoint);
        }
        
        // Draw the main line using MidiLine color
        ofSetColor(lines[i].color, 255);
        ofSetLineWidth(isSelected ? 5 : 4);  // Thicker if selected
        ofDrawLine(lines[i].startPoint, lines[i].endPoint);
        
        // Draw endpoint squares (larger if selected)
        int squareSize = isSelected ? 10 : 8;
        ofSetColor(lines[i].color, 255);
        ofDrawRectangle(lines[i].startPoint.x - squareSize/2, lines[i].startPoint.y - squareSize/2, squareSize, squareSize);
        ofDrawRectangle(lines[i].endPoint.x - squareSize/2, lines[i].endPoint.y - squareSize/2, squareSize, squareSize);
        
        // Draw line number at midpoint (brighter if selected)
        ofPoint midpoint = (lines[i].startPoint + lines[i].endPoint) * 0.5f;
        ofSetColor(isSelected ? ofColor(255, 255, 0) : ofColor(255, 255, 255));  // Yellow if selected
        ofDrawBitmapString("L" + ofToString(i + 1), midpoint.x - 10, midpoint.y + 4);
    }
    
    // Draw current line being drawn
    if (isDrawingLine) {
        ofColor currentColor = getNextLineColor();
        ofSetColor(currentColor, 200);
        ofSetLineWidth(3);
        ofDrawLine(lineStart, ofPoint(ofGetMouseX(), ofGetMouseY()));
        
        // Draw start point square
        ofSetColor(currentColor, 255);
        ofDrawRectangle(lineStart.x - 4, lineStart.y - 4, 8, 8);
    }
    
    // Reset drawing state
    ofSetColor(255);
    ofSetLineWidth(1);
}

// EXACT COPY from working backup
void LineManager::handleMousePressed(int x, int y, int button) {
    // Only allow line drawing within the video area (640x640 on the left)
    if (x < 0 || x >= 640 || y < 0 || y >= 640) {
        return; // Outside video area, ignore mouse clicks
    }
    
    if (button == 0 && !isDrawingLine) { // Left click
        ofPoint clickPoint(x, y);
        
        // Priority 1: Check if we're clicking near an endpoint for dragging
        bool foundEndpoint = false;
        for (int i = 0; i < lines.size() && !foundEndpoint; i++) {
            bool isStartPoint;
            if (isNearEndpoint(clickPoint, i, isStartPoint, 15.0f)) {
                // Start dragging the endpoint
                isDraggingEndpoint = true;
                draggingLineIndex = i;
                isDraggingStartPoint = isStartPoint;
                selectedLineIndex = i; // Also select this line
                foundEndpoint = true;
                ofLogNotice() << "Dragging " << (isStartPoint ? "start" : "end") << " point of line " << (i + 1);
            }
        }
        
        // Priority 2: If not dragging, check for line selection
        if (!foundEndpoint) {
            int nearestLine = findClosestLine(clickPoint, 15.0f);
            
            if (nearestLine != -1) {
                // Select the line
                selectedLineIndex = nearestLine;
                ofLogNotice() << "Selected line " << (selectedLineIndex + 1) << " from (" 
                              << lines[selectedLineIndex].startPoint.x << "," << lines[selectedLineIndex].startPoint.y 
                              << ") to (" << lines[selectedLineIndex].endPoint.x << "," << lines[selectedLineIndex].endPoint.y << ")";
            } else {
                // Priority 3: Start drawing a new line
                lineStart = ofPoint(x, y);
                isDrawingLine = true;
                selectedLineIndex = -1; // Deselect when starting new line
                ofLogNotice() << "Started line at: (" << x << ", " << y << ")";
            }
        }
    } else if (button == 2 && isDrawingLine) { // Right click - finish line
        ofPoint lineEnd = ofPoint(x, y);
        
        // Task 3.1: Create new MidiLine with complete properties
        MidiLine newLine;
        newLine.startPoint = lineStart;
        newLine.endPoint = lineEnd;
        newLine.color = getNextLineColor();
        
        // Initialize with defaults and set first available MIDI port
        initializeNewLineDefaults(newLine);
        
        lines.push_back(newLine);
        isDrawingLine = false;
        
        ofLogNotice() << "Finished line " << lines.size() << " from (" 
                      << lineStart.x << "," << lineStart.y << ") to (" 
                      << lineEnd.x << "," << lineEnd.y << ")";
    }
}

// EXACT COPY from working backup
void LineManager::handleMouseReleased(int x, int y, int button) {
    // Only allow interactions within the video area (640x640 on the left)
    if (x < 0 || x >= 640 || y < 0 || y >= 640) {
        return; // Outside video area, ignore mouse events
    }
    
    // Stop endpoint dragging
    if (isDraggingEndpoint) {
        ofLogNotice() << "Finished dragging " << (isDraggingStartPoint ? "start" : "end") 
                      << " point of line " << (draggingLineIndex + 1);
        isDraggingEndpoint = false;
        draggingLineIndex = -1;
        isDraggingStartPoint = false;
    }
}

// EXACT COPY from working backup
void LineManager::handleMouseDragged(int x, int y, int button) {
    // Only allow dragging within the video area (640x640 on the left)
    if (x < 0 || x >= 640 || y < 0 || y >= 640) {
        return; // Outside video area, ignore mouse drags
    }
    
    // Handle endpoint dragging
    if (isDraggingEndpoint && draggingLineIndex >= 0 && draggingLineIndex < lines.size()) {
        ofPoint newPos(x, y);
        
        if (isDraggingStartPoint) {
            lines[draggingLineIndex].startPoint = newPos;
        } else {
            lines[draggingLineIndex].endPoint = newPos;
        }
    }
}

void LineManager::handleMouseMoved(int x, int y) {
    // Minimal - just for consistency
}

void LineManager::clearAllLines() {
    lines.clear();
    selectedLineIndex = -1;
    isDrawingLine = false;
    currentColorIndex = 0;
    isDraggingEndpoint = false;
    draggingLineIndex = -1;
    isDraggingStartPoint = false;
    ofLogNotice() << "LineManager: All lines cleared";
}

void LineManager::selectLine(int index) {
    if (index >= -1 && index < lines.size()) {
        selectedLineIndex = index;
    }
}

void LineManager::deleteSelectedLine() {
    if (selectedLineIndex >= 0 && selectedLineIndex < lines.size()) {
        ofLogNotice() << "LineManager: Deleted line " << (selectedLineIndex + 1);
        lines.erase(lines.begin() + selectedLineIndex);
        selectedLineIndex = -1;
    }
}

void LineManager::duplicateSelectedLine() {
    if (selectedLineIndex >= 0 && selectedLineIndex < lines.size()) {
        MidiLine duplicatedLine = lines[selectedLineIndex];
        // Offset the new line slightly
        duplicatedLine.startPoint += ofPoint(10, 10);
        duplicatedLine.endPoint += ofPoint(10, 10);
        duplicatedLine.color = getNextLineColor();
        lines.push_back(duplicatedLine);
        ofLogNotice() << "LineManager: Duplicated line " << (selectedLineIndex + 1);
    }
}

LineManager::MidiLine* LineManager::getSelectedLine() {
    if (selectedLineIndex >= 0 && selectedLineIndex < lines.size()) {
        return &lines[selectedLineIndex];
    }
    return nullptr;
}

// EXACT COPY from working backup
ofColor LineManager::getNextLineColor() {
    vector<ofColor> colors = {
        ofColor::red,
        ofColor::blue,
        ofColor::green,
        ofColor::orange,
        ofColor::purple,
        ofColor::cyan,
        ofColor::yellow,
        ofColor::magenta,
        ofColor(255, 0, 128),   // Pink
        ofColor(128, 255, 0),   // Lime green
        ofColor(255, 128, 0),   // Dark orange
        ofColor(0, 128, 255)    // Light blue
    };
    
    ofColor color = colors[currentColorIndex % colors.size()];
    currentColorIndex++; // Increment for each new line
    return color;
}

// EXACT COPY from working backup  
int LineManager::findClosestLine(const ofPoint& clickPoint, float threshold) {
    int closestLineIndex = -1;
    float minDistance = threshold;
    
    for (int i = 0; i < lines.size(); i++) {
        float distance = distanceToLineSegment(clickPoint, lines[i].startPoint, lines[i].endPoint);
        if (distance < minDistance) {
            minDistance = distance;
            closestLineIndex = i;
        }
    }
    
    return closestLineIndex;
}

// EXACT COPY from working backup
bool LineManager::isNearEndpoint(const ofPoint& clickPoint, int lineIndex, bool& isStartPoint, float threshold) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        return false;
    }
    
    float distToStart = sqrt((clickPoint.x - lines[lineIndex].startPoint.x) * (clickPoint.x - lines[lineIndex].startPoint.x) + 
                            (clickPoint.y - lines[lineIndex].startPoint.y) * (clickPoint.y - lines[lineIndex].startPoint.y));
    float distToEnd = sqrt((clickPoint.x - lines[lineIndex].endPoint.x) * (clickPoint.x - lines[lineIndex].endPoint.x) + 
                          (clickPoint.y - lines[lineIndex].endPoint.y) * (clickPoint.y - lines[lineIndex].endPoint.y));
    
    if (distToStart <= threshold && distToStart <= distToEnd) {
        isStartPoint = true;
        return true;
    } else if (distToEnd <= threshold) {
        isStartPoint = false;
        return true;
    }
    
    return false;
}

// EXACT COPY from working backup
float LineManager::distanceToLineSegment(const ofPoint& point, const ofPoint& lineStart, const ofPoint& lineEnd) {
    // Calculate distance from point to line segment
    float A = point.x - lineStart.x;
    float B = point.y - lineStart.y;
    float C = lineEnd.x - lineStart.x;
    float D = lineEnd.y - lineStart.y;
    
    float dot = A * C + B * D;
    float lenSq = C * C + D * D;
    
    if (lenSq == 0) {
        // Line segment is actually a point
        return sqrt((point.x - lineStart.x) * (point.x - lineStart.x) + 
                   (point.y - lineStart.y) * (point.y - lineStart.y));
    }
    
    float param = dot / lenSq;
    
    ofPoint closestPoint;
    if (param < 0) {
        closestPoint = lineStart;
    } else if (param > 1) {
        closestPoint = lineEnd;
    } else {
        closestPoint.x = lineStart.x + param * C;
        closestPoint.y = lineStart.y + param * D;
    }
    
    return sqrt((point.x - closestPoint.x) * (point.x - closestPoint.x) + 
               (point.y - closestPoint.y) * (point.y - closestPoint.y));
}

// EXACT COPY from working backup
void LineManager::initializeNewLineDefaults(MidiLine& line) {
    // Will need MIDI port names - for now use placeholder
    if (line.midiPortName.empty()) {
        line.midiPortName = "IAC Driver Bus 1"; // Default
    }
    
    // Task 3.4: Assign different scale note indices for variety
    if (lines.size() > 0) {
        vector<int> scaleIntervals = getScaleIntervals(masterScale);
        if (!scaleIntervals.empty()) {
            int scaleNoteOffset = (lines.size() - 1) % scaleIntervals.size();
            line.scaleNoteIndex = scaleNoteOffset;
        }
    }
}

void LineManager::initializeMasterMusicalSystem() {
    // EXACT copy from working backup
    masterRootNote = 0;  // C
    masterScale = "Major";
    ofLogNotice() << "LineManager: Master musical system initialized: C Major";
}

// Musical system methods - EXACT COPY from working backup
vector<string> LineManager::getAvailableScales() {
    return {"Major", "Minor", "Pentatonic", "Blues", "Chromatic"};
}

vector<string> LineManager::getScaleNoteNames() {
    vector<string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    vector<int> intervals = getScaleIntervals(masterScale);
    vector<string> scaleNotes;
    
    for (int interval : intervals) {
        int noteIndex = (masterRootNote + interval) % 12;
        scaleNotes.push_back(noteNames[noteIndex]);
    }
    
    return scaleNotes;
}

vector<int> LineManager::getScaleIntervals(const string& scaleName) {
    if (scaleName == "Major") {
        return {0, 2, 4, 5, 7, 9, 11};
    } else if (scaleName == "Minor") {
        return {0, 2, 3, 5, 7, 8, 10};
    } else if (scaleName == "Pentatonic") {
        return {0, 2, 4, 7, 9};
    } else if (scaleName == "Blues") {
        return {0, 3, 5, 6, 7, 10};
    } else if (scaleName == "Chromatic") {
        return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    }
    return {0, 2, 4, 5, 7, 9, 11}; // Default to Major
}

int LineManager::getMidiNoteFromMasterScale(int lineIndex) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        return 60; // Middle C
    }
    
    const MidiLine& line = lines[lineIndex];
    vector<int> scaleIntervals = getScaleIntervals(masterScale);
    
    if (scaleIntervals.empty()) {
        return 60; // Middle C
    }
    
    int scaleNoteIndex;
    if (line.randomizeNote) {
        // Always use immediate randomization for real-time responsiveness
        // Tempo sync doesn't make sense for vehicle detection events
        scaleNoteIndex = getImmediateRandomScaleIndex(lineIndex);
    } else {
        scaleNoteIndex = line.scaleNoteIndex;
    }
    if (scaleNoteIndex >= scaleIntervals.size()) {
        scaleNoteIndex = 0;
    }
    
    int baseNote = 12 + masterRootNote + scaleIntervals[scaleNoteIndex]; // Start from octave 1
    int finalNote = baseNote + (line.octave * 12);
    
    // Clamp to valid MIDI range
    return ofClamp(finalNote, 0, 127);
}

void LineManager::rescaleLines(int oldWidth, int oldHeight, int newWidth, int newHeight) {
    float scaleX = (float)newWidth / (float)oldWidth;
    float scaleY = (float)newHeight / (float)oldHeight;
    
    for (MidiLine& line : lines) {
        line.startPoint.x *= scaleX;
        line.startPoint.y *= scaleY;
        line.endPoint.x *= scaleX;
        line.endPoint.y *= scaleY;
    }
    
    ofLogNotice() << "LineManager: Rescaled " << lines.size() << " lines";
}

// Configuration methods - EXACT COPY from working backup
void LineManager::saveToJSON(ofxJSONElement& json) {
    // Save master musical system
    json["masterRootNote"] = masterRootNote;
    json["masterScale"] = masterScale;
    
    // Save all lines
    json["lines"] = ofxJSONElement();
    for (int i = 0; i < (int)lines.size(); i++) {
        const MidiLine& line = lines[i];
        ofxJSONElement lineJson;
        
        lineJson["startPoint"]["x"] = line.startPoint.x;
        lineJson["startPoint"]["y"] = line.startPoint.y;
        lineJson["endPoint"]["x"] = line.endPoint.x;
        lineJson["endPoint"]["y"] = line.endPoint.y;
        lineJson["color"]["r"] = (int)line.color.r;
        lineJson["color"]["g"] = (int)line.color.g;
        lineJson["color"]["b"] = (int)line.color.b;
        
        lineJson["scaleNoteIndex"] = line.scaleNoteIndex;
        lineJson["randomizeNote"] = line.randomizeNote;
        lineJson["octave"] = line.octave;
        lineJson["midiChannel"] = line.midiChannel;
        lineJson["midiPortName"] = line.midiPortName;
        lineJson["durationType"] = (int)line.durationType;
        lineJson["fixedDuration"] = line.fixedDuration;
        lineJson["velocityType"] = (int)line.velocityType;
        lineJson["fixedVelocity"] = line.fixedVelocity;
        
        // NEW: Tempo-based randomization properties
        lineJson["enableTempoSync"] = line.enableTempoSync;
        lineJson["quantizeMode"] = (int)line.quantizeMode;
        lineJson["quantizeStrength"] = line.quantizeStrength;
        lineJson["randomSeed"] = line.randomSeed;
        lineJson["lastBeatTime"] = line.lastBeatTime;
        lineJson["lastRandomNoteIndex"] = line.lastRandomNoteIndex;
        
        // Save scale degree weights
        ofxJSONElement weightsJson;
        for (int w = 0; w < line.scaleDegreeWeights.size(); w++) {
            weightsJson[w] = line.scaleDegreeWeights[w];
        }
        lineJson["scaleDegreeWeights"] = weightsJson;
        
        json["lines"][i] = lineJson;
    }
    
    json["showLines"] = showLines;
}

void LineManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("masterRootNote")) {
        masterRootNote = json["masterRootNote"].asInt();
    }
    if (json.isMember("masterScale")) {
        masterScale = json["masterScale"].asString();
    }
    
    lines.clear();
    if (json.isMember("lines") && json["lines"].isArray()) {
        for (int i = 0; i < json["lines"].size(); i++) {
            const ofxJSONElement& lineJson = json["lines"][i];
            MidiLine line;
            
            if (lineJson.isMember("startPoint")) {
                line.startPoint.x = lineJson["startPoint"]["x"].asFloat();
                line.startPoint.y = lineJson["startPoint"]["y"].asFloat();
            }
            if (lineJson.isMember("endPoint")) {
                line.endPoint.x = lineJson["endPoint"]["x"].asFloat();
                line.endPoint.y = lineJson["endPoint"]["y"].asFloat();
            }
            if (lineJson.isMember("color")) {
                line.color.r = lineJson["color"]["r"].asInt();
                line.color.g = lineJson["color"]["g"].asInt();
                line.color.b = lineJson["color"]["b"].asInt();
            }
            
            if (lineJson.isMember("scaleNoteIndex")) line.scaleNoteIndex = lineJson["scaleNoteIndex"].asInt();
            if (lineJson.isMember("randomizeNote")) line.randomizeNote = lineJson["randomizeNote"].asBool();
            if (lineJson.isMember("octave")) line.octave = lineJson["octave"].asInt();
            if (lineJson.isMember("midiChannel")) line.midiChannel = lineJson["midiChannel"].asInt();
            if (lineJson.isMember("midiPortName")) line.midiPortName = lineJson["midiPortName"].asString();
            if (lineJson.isMember("durationType")) line.durationType = (MidiLine::DurationType)lineJson["durationType"].asInt();
            if (lineJson.isMember("fixedDuration")) line.fixedDuration = lineJson["fixedDuration"].asInt();
            if (lineJson.isMember("velocityType")) line.velocityType = (MidiLine::VelocityType)lineJson["velocityType"].asInt();
            if (lineJson.isMember("fixedVelocity")) line.fixedVelocity = lineJson["fixedVelocity"].asInt();
            
            // NEW: Load tempo-based randomization properties
            if (lineJson.isMember("enableTempoSync")) line.enableTempoSync = lineJson["enableTempoSync"].asBool();
            if (lineJson.isMember("quantizeMode")) line.quantizeMode = (MidiLine::QuantizeMode)lineJson["quantizeMode"].asInt();
            if (lineJson.isMember("quantizeStrength")) line.quantizeStrength = lineJson["quantizeStrength"].asFloat();
            if (lineJson.isMember("randomSeed")) line.randomSeed = lineJson["randomSeed"].asInt();
            if (lineJson.isMember("lastBeatTime")) line.lastBeatTime = lineJson["lastBeatTime"].asFloat();
            if (lineJson.isMember("lastRandomNoteIndex")) line.lastRandomNoteIndex = lineJson["lastRandomNoteIndex"].asInt();
            
            // Load scale degree weights
            if (lineJson.isMember("scaleDegreeWeights")) {
                const ofxJSONElement& weightsJson = lineJson["scaleDegreeWeights"];
                line.scaleDegreeWeights.clear();
                if (weightsJson.isArray()) {
                    for (int w = 0; w < weightsJson.size(); w++) {
                        line.scaleDegreeWeights.push_back(weightsJson[w].asFloat());
                    }
                }
                // Ensure we have at least default weights
                if (line.scaleDegreeWeights.empty()) {
                    line.scaleDegreeWeights = {1.5f, 0.8f, 1.2f, 0.9f, 1.4f, 0.9f, 0.7f}; // Major scale defaults
                }
            }
            
            lines.push_back(line);
        }
    }
    
    if (json.isMember("showLines")) {
        showLines = json["showLines"].asBool();
    }
    
    ofLogNotice() << "LineManager: Loaded " << lines.size() << " lines from config";
}

void LineManager::setDefaults() {
    lines.clear();
    masterRootNote = 0;  // C
    masterScale = "Major";
    showLines = true;
    selectedLineIndex = -1;
    isDrawingLine = false;
    isDraggingEndpoint = false;
    draggingLineIndex = -1;
    currentColorIndex = 0;
    
    ofLogNotice() << "LineManager: Set to default values";
}

// NEW: Tempo-synchronized randomization methods
int LineManager::getTempoSyncedRandomNote(int lineIndex, float currentTime) {
    ofLogNotice() << "DEBUG: getTempoSyncedRandomNote called for line " << lineIndex;
    
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        return 60; // Middle C fallback
    }
    
    const MidiLine& line = lines[lineIndex];
    
    // If tempo sync is disabled, fall back to immediate randomization
    if (!line.enableTempoSync || !tempoManager) {
        return getImmediateRandomNote(lineIndex);
    }
    
    // Get quantized beat time from TempoManager
    float closestBeatTime = tempoManager->getClosestBeatTime(currentTime);
    int beatIndex = tempoManager->getBeatIndexForTime(closestBeatTime);
    
    // FIXED: Add microsecond precision and line index for true randomness
    // Combine multiple changing factors to ensure different results each time
    float currentTimeMicros = currentTime * 1000000.0f; // Convert to microseconds
    int randomSeed = (int)(currentTimeMicros) + beatIndex * 1000 + line.randomSeed + (lineIndex * 777);
    srand(randomSeed);
    
    // Apply scale degree weighting for musical randomness
    vector<int> scaleIntervals = getScaleIntervals(masterScale);
    if (scaleIntervals.empty()) {
        return 60; // Middle C fallback
    }
    
    // Adjust weights to match scale size
    vector<float> weights = line.scaleDegreeWeights;
    if (weights.size() != scaleIntervals.size()) {
        // Resize weights to match scale size
        weights.resize(scaleIntervals.size(), 1.0f);
        // If scale is smaller, just use the first N weights
        // If scale is larger, fill remaining with average weight
        if (line.scaleDegreeWeights.size() < scaleIntervals.size()) {
            float avgWeight = 1.0f;
            if (!line.scaleDegreeWeights.empty()) {
                avgWeight = 0.0f;
                for (float w : line.scaleDegreeWeights) avgWeight += w;
                avgWeight /= line.scaleDegreeWeights.size();
            }
            for (int i = line.scaleDegreeWeights.size(); i < scaleIntervals.size(); i++) {
                weights[i] = avgWeight;
            }
        }
    }
    
    // Weighted random selection
    int selectedIndex = weightedRandomSelection(weights);
    if (selectedIndex >= scaleIntervals.size()) {
        selectedIndex = 0; // Fallback to root
    }
    
    // Calculate final MIDI note
    int baseNote = 12 + masterRootNote + scaleIntervals[selectedIndex]; // Start from octave 1
    int finalNote = baseNote + (line.octave * 12);
    
    return ofClamp(finalNote, 0, 127); // Clamp to valid MIDI range
}

int LineManager::getImmediateRandomScaleIndex(int lineIndex) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        return 0; // Root note fallback
    }
    
    const MidiLine& line = lines[lineIndex];
    vector<int> scaleIntervals = getScaleIntervals(masterScale);
    
    if (scaleIntervals.empty()) {
        return 0; // Root note fallback
    }
    
    // Use microsecond precision for true randomness
    float currentTimeMicros = ofGetElapsedTimef() * 1000000.0f;
    int randomSeed = (int)(currentTimeMicros) + line.randomSeed + (lineIndex * 777);
    srand(randomSeed);
    
    // Apply weighted random selection for musical intelligence
    vector<float> weights = line.scaleDegreeWeights;
    if (weights.size() != scaleIntervals.size()) {
        // Resize weights to match scale size
        weights.resize(scaleIntervals.size(), 1.0f);
        if (line.scaleDegreeWeights.size() < scaleIntervals.size()) {
            float avgWeight = 1.0f;
            if (!line.scaleDegreeWeights.empty()) {
                avgWeight = 0.0f;
                for (float w : line.scaleDegreeWeights) avgWeight += w;
                avgWeight /= line.scaleDegreeWeights.size();
            }
            for (int i = line.scaleDegreeWeights.size(); i < scaleIntervals.size(); i++) {
                weights[i] = avgWeight;
            }
        }
    }
    
    // Use weighted selection and return the scale index
    int scaleNoteIndex = weightedRandomSelection(weights);
    if (scaleNoteIndex >= scaleIntervals.size()) {
        scaleNoteIndex = 0;
    }
    
    return scaleNoteIndex;
}

int LineManager::getImmediateRandomNote(int lineIndex) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        return 60; // Middle C fallback
    }
    
    const MidiLine& line = lines[lineIndex];
    vector<int> scaleIntervals = getScaleIntervals(masterScale);
    
    if (scaleIntervals.empty()) {
        return 60; // Middle C fallback
    }
    
    // Use microsecond precision for true randomness
    float currentTimeMicros = ofGetElapsedTimef() * 1000000.0f;
    int randomSeed = (int)(currentTimeMicros) + line.randomSeed + (lineIndex * 777);
    srand(randomSeed);
    
    // Apply weighted random selection for musical intelligence
    vector<float> weights = line.scaleDegreeWeights;
    if (weights.size() != scaleIntervals.size()) {
        // Resize weights to match scale size
        weights.resize(scaleIntervals.size(), 1.0f);
        if (line.scaleDegreeWeights.size() < scaleIntervals.size()) {
            float avgWeight = 1.0f;
            if (!line.scaleDegreeWeights.empty()) {
                avgWeight = 0.0f;
                for (float w : line.scaleDegreeWeights) avgWeight += w;
                avgWeight /= line.scaleDegreeWeights.size();
            }
            for (int i = line.scaleDegreeWeights.size(); i < scaleIntervals.size(); i++) {
                weights[i] = avgWeight;
            }
        }
    }
    
    // Use weighted selection instead of simple random
    int scaleNoteIndex = weightedRandomSelection(weights);
    if (scaleNoteIndex >= scaleIntervals.size()) {
        scaleNoteIndex = 0;
    }
    
    int baseNote = 12 + masterRootNote + scaleIntervals[scaleNoteIndex];
    int finalNote = baseNote + (line.octave * 12);
    
    return ofClamp(finalNote, 0, 127);
}

int LineManager::weightedRandomSelection(const vector<float>& weights) {
    if (weights.empty()) {
        return 0;
    }
    
    float totalWeight = 0.0f;
    for (float weight : weights) totalWeight += weight;
    
    if (totalWeight <= 0.0f) {
        return 0; // Fallback if all weights are 0
    }
    
    float randomValue = (rand() / (float)RAND_MAX) * totalWeight;
    float currentWeight = 0.0f;
    
    for (int i = 0; i < weights.size(); i++) {
        currentWeight += weights[i];
        if (randomValue <= currentWeight) {
            return i;
        }
    }
    
    return 0; // Fallback to first index
}