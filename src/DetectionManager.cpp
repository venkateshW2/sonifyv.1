#include "DetectionManager.h"
#include "VideoManager.h"
#include "LineManager.h"
#include "CommunicationManager.h"
#include <algorithm>

DetectionManager::DetectionManager() {
    // EXACT COPY from working backup
    detector = [[CoreMLDetector alloc] init];
    yoloLoaded = false;
    enableDetection = false;
    frameSkipCounter = 0;
    detectionFrameSkip = 3;  
    lastDetectionTime = 0.0f;
    detectionErrorCount = 0;
    displayScale = 1.0f;
    showDetections = true;
    
    // Initialize category system - EXACT COPY from working backup
    categoryEnabled.resize(CATEGORY_COUNT, false);
    enabledClasses.resize(80, false);
    currentPreset = "Vehicles Only";
    maxSelectedClasses = 15;
    currentVideoSource = 0; // CAMERA = 0
    
    // Initialize vehicle tracking variables - EXACT COPY from working backup
    nextVehicleId = 1;
    vehicleTrackingThreshold = 50.0f;  // Pixels - how close centers must be to match
    maxFramesWithoutDetection = 15;    // Frames - how long to keep tracking without detection
    
    videoManager = nullptr;
    lineManager = nullptr;
    communicationManager = nullptr;
    confidenceThreshold = 0.25f;  // Lower threshold to allow more detections through
}

DetectionManager::~DetectionManager() {
    if (detector) {
        detector = nil;
    }
}

void DetectionManager::setup() {
    initializeCategories();
    loadCoreMLModel();
}

void DetectionManager::update() {
    if (enableDetection && yoloLoaded) {
        processCoreMLDetection();
        
        // SAFE VERSION: Vehicle tracking with thread safety and null checks
        if (!detections.empty()) {
            updateVehicleTrackingSafe();
            checkLineCrossingsSafe();
        }
        
        // Cleanup old vehicles periodically
        static int cleanupCounter = 0;
        if (cleanupCounter++ % 60 == 0) { // Every 60 frames (~2 seconds)
            cleanupOldVehicles();
        }
    }
}

void DetectionManager::draw() {
    if (enableDetection && yoloLoaded && showDetections) {
        drawDetections();
    }
}

// EXACT COPY from working backup
void DetectionManager::loadCoreMLModel() {
    ofLogNotice() << "Loading CoreML YOLO model...";
    
    // Initialize CoreML models to nullptr
    
    // Load class names from coco.names - use data path
    string cocoNamesPath = ofToDataPath("models/coco.names");
    ofLogNotice() << "Looking for coco.names at: " << cocoNamesPath;
    ofBuffer buffer = ofBufferFromFile(cocoNamesPath);
    if (buffer.size() > 0) {
        for (auto& line : buffer.getLines()) {
            if (!line.empty()) {
                classNames.push_back(line);
            }
        }
        ofLogNotice() << "Loaded " << classNames.size() << " class names";
    } else {
        ofLogError() << "Failed to load coco.names from: " << cocoNamesPath;
        // Continue without class names - we can still use class IDs
        ofLogWarning() << "Continuing without class names file";
    }
    
    // Load CoreML model - try larger models first for better resolution support
    string modelPath;
    
    // Try yolov8l (large) first for potential higher resolution support
    modelPath = ofToDataPath("models/yolov8l.mlpackage");
    ofLogNotice() << "Looking for YOLOv8L model at: " << modelPath;
    
    if (ofFile::doesFileExist(modelPath, false)) {
        ofLogNotice() << "YOLOv8L CoreML model found: " << modelPath;
        NSString* nsModelPath = [NSString stringWithUTF8String:modelPath.c_str()];
        
        if ([detector loadModelAtPath:nsModelPath]) {
            ofLogNotice() << "YOLOv8L CoreML model loaded successfully";
            yoloLoaded = true;
        }
    }
    
    // Fallback to yolov8m (medium) if large model not available
    if (!yoloLoaded) {
        modelPath = ofToDataPath("models/yolov8m.mlpackage");
        ofLogNotice() << "Looking for YOLOv8M model at: " << modelPath;
        
        if (ofFile::doesFileExist(modelPath, false)) {
            ofLogNotice() << "YOLOv8M CoreML model found: " << modelPath;
            NSString* nsModelPath = [NSString stringWithUTF8String:modelPath.c_str()];
            
            if ([detector loadModelAtPath:nsModelPath]) {
                ofLogNotice() << "YOLOv8M CoreML model loaded successfully";
                yoloLoaded = true;
            }
        }
    }
    
    // Final fallback to yolov8n (nano)
    if (!yoloLoaded) {
        modelPath = ofToDataPath("models/yolov8n.mlpackage");
        ofLogNotice() << "Looking for YOLOv8N model at: " << modelPath;
        
        if (ofFile::doesFileExist(modelPath, false)) {
            ofLogNotice() << "YOLOv8N CoreML model found: " << modelPath;
            NSString* nsModelPath = [NSString stringWithUTF8String:modelPath.c_str()];
            
            if ([detector loadModelAtPath:nsModelPath]) {
                ofLogNotice() << "YOLOv8N CoreML model loaded successfully";
                yoloLoaded = true;
            }
        }
    }
    
    if (!yoloLoaded) {
        ofLogError() << "Failed to load any CoreML model";
    }
}

// EXACT COPY from working backup
void DetectionManager::processCoreMLDetection() {
    // RESTORED: Frame skip logic from working backup for performance control
    frameSkipCounter++;
    if (frameSkipCounter < detectionFrameSkip) {
        return;
    }
    frameSkipCounter = 0;
    
    // Get current frame (don't clear detections until we get new results)
    ofPixels pixels;
    
    if (videoManager) {
        pixels = videoManager->getCurrentPixels();
    }
    
    if (pixels.size() == 0) {
        return;
    }
    
    // Use CoreML detector - respects frame skip for performance
    static int counter = 0;
    counter++;  // Increment counter for logging purposes
    
    // Reduce logging frequency to avoid spam
    if (counter % 30 == 0) { // Log every 30 frames (~1 second at detectionFrameSkip=3)
        ofLogNotice() << "Running CoreML object detection...";
    }
    
    [detector detectObjectsInPixels:pixels.getData()
                                  width:pixels.getWidth()
                                 height:pixels.getHeight()
                               channels:pixels.getNumChannels()
                             completion:^(NSArray<CoreMLDetection*>* coremlDetections) {
                                 
                                 // Clear previous detections and convert CoreML detections to our format
                                 detections.clear();
                                 
                                 for (CoreMLDetection* coremlDet in coremlDetections) {
                                     // Filter by class - only include selected classes
                                     int classId = coremlDet.classId;
                                     if (classId >= 0 && classId < enabledClasses.size() && enabledClasses[classId]) {
                                         // Additional confidence filtering against DetectionManager threshold
                                         if (coremlDet.confidence >= confidenceThreshold) {
                                             Detection detection;
                                             
                                             // CoreML detector returns coordinates in 640x640 display space
                                             // Direct mapping to our fixed 640x640 window
                                             detection.box.x = coremlDet.x;
                                             detection.box.y = coremlDet.y;
                                             detection.box.width = coremlDet.width;
                                             detection.box.height = coremlDet.height;
                                             
                                             detection.confidence = coremlDet.confidence;
                                             detection.classId = coremlDet.classId;
                                             detection.className = [coremlDet.className UTF8String];
                                             
                                             detections.push_back(detection);
                                         }
                                     }
                                 }
                                 
                                 ofLogNotice() << "Found " << detections.size() << " objects";
                             }];
}

// EXACT COPY from working backup
void DetectionManager::drawDetections() {
    if (detections.empty()) {
        return;
    }
    
    // Debug: Log drawing info occasionally
    static int drawCounter = 0;
    if (drawCounter++ % 60 == 0) { // Every 2 seconds
        ofLogNotice() << "DRAWING " << detections.size() << " detections";
        if (detections.size() > 0) {
            ofLogNotice() << "First detection box: " << detections[0].box.x << "," << detections[0].box.y 
                << " size:" << detections[0].box.width << "x" << detections[0].box.height;
        }
    }
    
    for (const auto& detection : detections) {
        // Validate detection box coordinates
        if (detection.box.width <= 0 || detection.box.height <= 0) {
            continue;
        }
        
        // Apply display scaling to detection coordinates
        float x = detection.box.x * displayScale;
        float y = detection.box.y * displayScale;
        float w = detection.box.width * displayScale;
        float h = detection.box.height * displayScale;
        
        // Clamp scaled coordinates to screen bounds
        x = ofClamp(x, 0, ofGetWidth() - w);
        y = ofClamp(y, 0, ofGetHeight() - h);
        w = ofClamp(w, 1, ofGetWidth() - x);
        h = ofClamp(h, 1, ofGetHeight() - y);
        
        // Choose color based on vehicle type - with reduced opacity for calmer look
        ofColor boxColor;
        switch (detection.classId) {
            case 2: // car
                boxColor = ofColor(0, 200, 0, 150); // Softer Green
                break;
            case 3: // motorcycle  
                boxColor = ofColor(200, 200, 0, 150); // Softer Yellow
                break;
            case 5: // bus
                boxColor = ofColor(200, 0, 0, 150); // Softer Red
                break;
            case 7: // truck
                boxColor = ofColor(0, 0, 200, 150); // Softer Blue
                break;
            default:
                boxColor = ofColor(180, 180, 180, 150); // Softer White
                break;
        }
        
        // Draw more subtle bounding box
        ofSetColor(boxColor);
        ofSetLineWidth(1.5);  // Thinner line
        ofNoFill();
        
        // Draw main rectangle with subtle style
        ofDrawRectangle(x, y, w, h);
        
        // Add corner accents with reduced size and line width
        ofSetLineWidth(2);
        float cornerSize = 8 * displayScale; // Smaller corners
        // Top-left corner
        ofDrawLine(x, y, x + cornerSize, y);
        ofDrawLine(x, y, x, y + cornerSize);
        // Top-right corner
        ofDrawLine(x + w - cornerSize, y, x + w, y);
        ofDrawLine(x + w, y, x + w, y + cornerSize);
        // Bottom-left corner
        ofDrawLine(x, y + h - cornerSize, x, y + h);
        ofDrawLine(x, y + h, x + cornerSize, y + h);
        // Bottom-right corner
        ofDrawLine(x + w - cornerSize, y + h, x + w, y + h);
        ofDrawLine(x + w, y + h - cornerSize, x + w, y + h);
        
        ofFill();
        
        // Draw thinner confidence indicator
        float confBarWidth = w * 0.7f;  // 70% of box width
        float confBarHeight = 3 * displayScale;  // Even thinner bar
        float confBarX = x + (w - confBarWidth) / 2;  // Center horizontally
        float confBarY = y + h - confBarHeight - 3 * displayScale;  // Bottom of box with padding
        
        // Background of confidence bar with subtle transparency
        ofSetColor(0, 0, 0, 90); // More transparent
        ofDrawRectangle(confBarX, confBarY, confBarWidth, confBarHeight);
        
        // Filled portion based on confidence with reduced opacity
        ofSetColor(boxColor.r, boxColor.g, boxColor.b, 150);
        ofDrawRectangle(confBarX, confBarY, confBarWidth * detection.confidence, confBarHeight);
        
        // Draw compact class label with more subtle styling
        string label = detection.className + " " + ofToString(detection.confidence, 2);
        float labelWidth = label.length() * 6.5 * displayScale;  // Slightly smaller
        float labelHeight = 12 * displayScale;  // More compact height
        
        // Position label at top-left of box with small padding
        float labelX = x + 2 * displayScale;
        float labelY = y;
        
        // Ensure label fits on screen
        labelX = ofClamp(labelX, 0, ofGetWidth() - labelWidth);
        labelY = ofClamp(labelY, labelHeight, ofGetHeight());
        
        // Draw label background with reduced opacity
        ofSetColor(boxColor.r, boxColor.g, boxColor.b, 130);
        ofDrawRectangle(labelX - 2, labelY - labelHeight + 2, labelWidth + 4, labelHeight);
        
        // Draw label text with better contrast
        ofSetColor(255, 255, 255, 220); // White text with slight transparency
        ofDrawBitmapString(label, labelX, labelY - 3);
    }
    
    // Reset drawing state
    ofSetColor(255);
    ofSetLineWidth(1);
    ofFill();
}

// EXACT COPY from working backup
void DetectionManager::initializeCategories() {
    // Initialize category enabled flags
    categoryEnabled.resize(CATEGORY_COUNT, false);
    categoryEnabled[CATEGORY_VEHICLES] = true;  // Vehicles enabled by default
    
    // Initialize enabled classes vector (80 elements for all YOLO classes)
    enabledClasses.resize(80, false);
    
    // CRITICAL FIX: Enable vehicle classes directly to match working backup
    // Vehicle classes: bicycle(1), car(2), motorcycle(3), airplane(4), bus(5), train(6), truck(7), boat(8)
    vector<int> vehicleClasses = {1, 2, 3, 4, 5, 6, 7, 8};
    selectedClassIds = vehicleClasses;
    
    // Enable these classes in the enabledClasses array
    for (int classId : vehicleClasses) {
        if (classId < enabledClasses.size()) {
            enabledClasses[classId] = true;
        }
    }
    
    // Apply the default "Vehicles Only" preset
    currentPreset = "Vehicles Only";
    
    ofLogNotice() << "DetectionManager: Initialized with " << selectedClassIds.size() << " vehicle classes enabled";
}

// EXACT COPY from working backup
void DetectionManager::applyPreset(const string& presetName) {
    // Clear current selection
    selectedClassIds.clear();
    
    // Reset categories
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        categoryEnabled[i] = false;
    }
    
    // Apply the selected preset
    if (presetName == "Vehicles Only") {
        // Enable Vehicles category
        categoryEnabled[CATEGORY_VEHICLES] = true;
        
        // Select specific vehicle classes
        vector<int> vehicleIds = {1, 2, 3, 5, 7};  // bicycle, car, motorcycle, bus, truck
        selectedClassIds.insert(selectedClassIds.end(), vehicleIds.begin(), vehicleIds.end());
        
    } else if (presetName == "People & Animals") {
        // Enable People and Animals categories
        categoryEnabled[CATEGORY_PEOPLE] = true;
        categoryEnabled[CATEGORY_ANIMALS] = true;
        
        // Add person (0)
        selectedClassIds.push_back(0);
        
        // Add common animals (bird, cat, dog, horse, sheep, cow, etc.)
        vector<int> animalIds = {14, 15, 16, 17, 18, 19, 20, 21};
        selectedClassIds.insert(selectedClassIds.end(), animalIds.begin(), animalIds.end());
        
    } else if (presetName == "Everything Moving") {
        // Enable all categories
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            categoryEnabled[i] = true;
        }
        
        // Add all potentially moving objects
        vector<int> movingObjects = {
            0,  // person
            1, 2, 3, 4, 5, 6, 7, 8,  // vehicles  
            14, 15, 16, 17, 18, 19, 20, 21, 22, 23,  // animals
            32, 33, 37  // sports ball, kite, surfboard (movable objects)
        };
        selectedClassIds.insert(selectedClassIds.end(), movingObjects.begin(), movingObjects.end());
        
    } else if (presetName == "Custom") {
        // Custom preset keeps the current selection
        // Just update the UI to reflect custom mode
    }
    
    // Enforce the max number of selected classes
    if (selectedClassIds.size() > maxSelectedClasses) {
        selectedClassIds.resize(maxSelectedClasses);
    }
    
    // Update enabled classes vector based on selection
    updateEnabledClassesFromSelection();
    
    ofLogNotice() << "Applied preset: " << presetName 
                 << " with " << selectedClassIds.size() << " classes selected";
}

// EXACT COPY from working backup
void DetectionManager::updateEnabledClassesFromSelection() {
    // Reset all classes to disabled
    std::fill(enabledClasses.begin(), enabledClasses.end(), false);
    
    // Enable only the selected class IDs
    for (int classId : selectedClassIds) {
        if (classId >= 0 && classId < enabledClasses.size()) {
            enabledClasses[classId] = true;
        }
    }
}

// EXACT COPY from working backup
string DetectionManager::getClassNameById(int classId) {
    // Return class name from classNames vector if available
    if (classId >= 0 && classId < classNames.size()) {
        return classNames[classId];
    }
    
    // Fallback to basic class ID if not found
    return "Class " + ofToString(classId);
}

// EXACT COPY from working backup
DetectionManager::DetectionCategory DetectionManager::getCategoryForClass(int classId) {
    // Map YOLO COCO class IDs to our categories
    if (classId == 0) {
        return CATEGORY_PEOPLE;  // person
    } else if (classId >= 1 && classId <= 8) {
        return CATEGORY_VEHICLES;  // bicycle to boat
    } else if (classId >= 14 && classId <= 23) {
        return CATEGORY_ANIMALS;  // bird to zebra
    } else {
        return CATEGORY_OBJECTS;  // everything else
    }
}

// EXACT COPY from working backup
vector<int> DetectionManager::getClassesInCategory(DetectionManager::DetectionCategory category) {
    vector<int> result;
    
    switch (category) {
        case CATEGORY_VEHICLES:
            // Vehicles: bicycle(1), car(2), motorcycle(3), airplane(4), bus(5), train(6), truck(7), boat(8)
            result = {1, 2, 3, 4, 5, 6, 7, 8};
            break;
            
        case CATEGORY_PEOPLE:
            // Just person(0)
            result = {0};
            break;
            
        case CATEGORY_ANIMALS:
            // Animals: bird(14) through zebra(23)
            for (int i = 14; i <= 23; i++) {
                result.push_back(i);
            }
            break;
            
        case CATEGORY_OBJECTS:
            // Everything else
            for (int i = 0; i < 80; i++) {
                if (i != 0 && (i < 1 || i > 8) && (i < 14 || i > 23)) {
                    result.push_back(i);
                }
            }
            break;
            
        default:
            break;
    }
    
    return result;
}

// EXACT COPY from working backup
void DetectionManager::applyNMS(const vector<Detection>& rawDetections, vector<Detection>& filteredDetections, float nmsThreshold) {
    filteredDetections.clear();
    
    if (rawDetections.empty()) {
        return;
    }
    
    // Create indices sorted by confidence (highest first)
    vector<int> indices;
    for (int i = 0; i < rawDetections.size(); i++) {
        indices.push_back(i);
    }
    
    // Sort by confidence descending
    std::sort(indices.begin(), indices.end(), [&rawDetections](int a, int b) {
        return rawDetections[a].confidence > rawDetections[b].confidence;
    });
    
    vector<bool> suppressed(rawDetections.size(), false);
    
    for (int i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        if (suppressed[idx]) continue;
        
        // This detection is not suppressed, so add it
        filteredDetections.push_back(rawDetections[idx]);
        
        // Calculate IoU with remaining detections and suppress overlapping ones
        for (int j = i + 1; j < indices.size(); j++) {
            int other_idx = indices[j];
            if (suppressed[other_idx]) continue;
            
            // Calculate Intersection over Union (IoU)
            float iou = calculateIoU(rawDetections[idx].box, rawDetections[other_idx].box);
            
            // Suppress if IoU is above threshold AND same class
            if (iou > nmsThreshold && rawDetections[idx].classId == rawDetections[other_idx].classId) {
                suppressed[other_idx] = true;
            }
        }
    }
}

// EXACT COPY from working backup
float DetectionManager::calculateIoU(const ofRectangle& box1, const ofRectangle& box2) {
    // Calculate intersection rectangle
    float x1 = std::max(box1.x, box2.x);
    float y1 = std::max(box1.y, box2.y);
    float x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    float y2 = std::min(box1.y + box1.height, box2.y + box2.height);
    
    // No intersection
    if (x2 <= x1 || y2 <= y1) {
        return 0.0f;
    }
    
    // Calculate areas
    float intersectionArea = (x2 - x1) * (y2 - y1);
    float box1Area = box1.width * box1.height;
    float box2Area = box2.width * box2.height;
    float unionArea = box1Area + box2Area - intersectionArea;
    
    // Avoid division by zero
    if (unionArea <= 0.0f) {
        return 0.0f;
    }
    
    return intersectionArea / unionArea;
}

// Configuration methods - EXACT COPY from working backup
void DetectionManager::saveToJSON(ofxJSONElement& json) {
    json["enableDetection"] = enableDetection;
    json["showDetections"] = showDetections;
    json["confidenceThreshold"] = confidenceThreshold;
    json["detectionFrameSkip"] = detectionFrameSkip;
    json["currentPreset"] = currentPreset;
    json["maxSelectedClasses"] = maxSelectedClasses;
    json["displayScale"] = displayScale;
    
    // Save enabled classes
    json["enabledClasses"] = ofxJSONElement();
    for (int i = 0; i < (int)enabledClasses.size(); i++) {
        json["enabledClasses"][i] = (bool)enabledClasses[i];
    }
    
    // Save category enabled states
    json["categoryEnabled"] = ofxJSONElement();
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        json["categoryEnabled"][i] = (bool)categoryEnabled[i];
    }
    
    // Save selected class IDs
    json["selectedClassIds"] = ofxJSONElement();
    for (int i = 0; i < (int)selectedClassIds.size(); i++) {
        json["selectedClassIds"][i] = selectedClassIds[i];
    }
}

void DetectionManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("enableDetection")) {
        enableDetection = json["enableDetection"].asBool();
    }
    if (json.isMember("showDetections")) {
        showDetections = json["showDetections"].asBool();
    }
    if (json.isMember("confidenceThreshold")) {
        confidenceThreshold = json["confidenceThreshold"].asFloat();
    }
    if (json.isMember("detectionFrameSkip")) {
        detectionFrameSkip = json["detectionFrameSkip"].asInt();
    }
    if (json.isMember("currentPreset")) {
        currentPreset = json["currentPreset"].asString();
    }
    if (json.isMember("maxSelectedClasses")) {
        maxSelectedClasses = json["maxSelectedClasses"].asInt();
    }
    if (json.isMember("displayScale")) {
        displayScale = json["displayScale"].asFloat();
    }
    
    // Load enabled classes
    if (json.isMember("enabledClasses") && json["enabledClasses"].isArray()) {
        enabledClasses.clear();
        for (int i = 0; i < json["enabledClasses"].size(); i++) {
            enabledClasses.push_back(json["enabledClasses"][i].asBool());
        }
        // Ensure we have 80 classes
        while (enabledClasses.size() < 80) {
            enabledClasses.push_back(false);
        }
    }
    
    // Load category enabled states
    if (json.isMember("categoryEnabled") && json["categoryEnabled"].isArray()) {
        for (int i = 0; i < std::min((int)json["categoryEnabled"].size(), (int)CATEGORY_COUNT); i++) {
            categoryEnabled[i] = json["categoryEnabled"][i].asBool();
        }
    }
    
    // Load selected class IDs
    selectedClassIds.clear();
    if (json.isMember("selectedClassIds") && json["selectedClassIds"].isArray()) {
        for (int i = 0; i < json["selectedClassIds"].size(); i++) {
            selectedClassIds.push_back(json["selectedClassIds"][i].asInt());
        }
    }
    
    ofLogNotice() << "DetectionManager: Configuration loaded";
}

// Live tracking data getter implementations
int DetectionManager::getVisibleVehiclesCount() const {
    int count = 0;
    for (const auto& vehicle : trackedVehicles) {
        if (!vehicle.isOccluded) {
            count++;
        }
    }
    return count;
}

int DetectionManager::getOccludedVehiclesCount() const {
    int count = 0;
    for (const auto& vehicle : trackedVehicles) {
        if (vehicle.isOccluded) {
            count++;
        }
    }
    return count;
}

void DetectionManager::setDefaults() {
    enableDetection = true;
    showDetections = true;
    confidenceThreshold = 0.25f;  // Lower threshold to allow more detections through
    detectionFrameSkip = 3;
    frameSkipCounter = 0;
    lastDetectionTime = 0;
    detectionErrorCount = 0;
    displayScale = 1.0f;
    yoloLoaded = false;
    currentPreset = "Vehicles Only";
    maxSelectedClasses = 10;
    currentVideoSource = 0;
    videoManager = nullptr;
    
    // Initialize enabled classes (80 COCO classes)
    enabledClasses.clear();
    enabledClasses.resize(80, false);
    
    // Initialize category states
    categoryEnabled.resize(CATEGORY_COUNT, false);
    categoryEnabled[CATEGORY_VEHICLES] = true;  // Default to vehicles only
    
    // Initialize with vehicle classes
    selectedClassIds = {1, 2, 3, 4, 5, 6, 7, 8};  // bicycle through boat
    updateEnabledClassesFromSelection();
    
    // Initialize class names (COCO 80 classes)
    initializeCategories();
    
    ofLogNotice() << "DetectionManager: Set to default values";
}

// UI Manager methods for Detection Classes tab - EXACT COPY from working backup
void DetectionManager::addSelectedClass(int classId) {
    if (selectedClassIds.size() < maxSelectedClasses && 
        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
        selectedClassIds.push_back(classId);
    }
}

void DetectionManager::removeSelectedClass(int classId) {
    selectedClassIds.erase(
        remove(selectedClassIds.begin(), selectedClassIds.end(), classId),
        selectedClassIds.end()
    );
}

void DetectionManager::removeClassesByCategory(DetectionCategory category) {
    selectedClassIds.erase(
        remove_if(selectedClassIds.begin(), selectedClassIds.end(),
            [this, category](int classId) {
                return getCategoryForClass(classId) == category;
            }),
        selectedClassIds.end()
    );
}

// Vehicle tracking and line crossing methods - EXACT COPY from working backup
void DetectionManager::updateVehicleTracking() {
    // Safety check: ensure we have valid data structures
    if (!enableDetection || !yoloLoaded) {
        return;
    }
    
    if (detections.empty()) {
        // Increment frame counter for all tracked vehicles if no new detections
        for (auto& vehicle : trackedVehicles) {
            vehicle.framesSinceLastSeen++;
        }
    } else {
        // Try to match each new detection to existing tracked vehicles
        vector<bool> detectionMatched(detections.size(), false);
        vector<bool> vehicleMatched(trackedVehicles.size(), false);
        
        // Match detections to existing vehicles
        for (int i = 0; i < detections.size(); i++) {
            const auto& detection = detections[i];
            ofPoint detectionCenter = ofPoint(
                detection.box.x + detection.box.width / 2,
                detection.box.y + detection.box.height / 2
            );
            
            float closestDistance = vehicleTrackingThreshold;
            int closestVehicleIndex = -1;
            
            // Find closest existing vehicle
            for (int j = 0; j < trackedVehicles.size(); j++) {
                if (vehicleMatched[j]) continue;
                
                auto& vehicle = trackedVehicles[j];
                float distance = calculateDistance(detectionCenter, vehicle.centerCurrent);
                
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestVehicleIndex = j;
                }
            }
            
            if (closestVehicleIndex >= 0) {
                // Update existing vehicle
                auto& vehicle = trackedVehicles[closestVehicleIndex];
                vehicle.previousBox = vehicle.currentBox;
                vehicle.centerPrevious = vehicle.centerCurrent;
                vehicle.currentBox = detection.box;
                vehicle.centerCurrent = detectionCenter;
                vehicle.vehicleType = detection.classId;
                vehicle.className = detection.className;
                vehicle.confidence = detection.confidence;
                vehicle.framesSinceLastSeen = 0;
                vehicle.isOccluded = false;
                
                // Calculate movement
                float movementDistance = calculateDistance(vehicle.centerPrevious, vehicle.centerCurrent);
                vehicle.hasMovement = movementDistance > 2.0f; // Minimum movement threshold
                vehicle.speed = movementDistance; // pixels per frame
                vehicle.speedMph = movementDistance * 0.1f; // rough approximation
                
                detectionMatched[i] = true;
                vehicleMatched[closestVehicleIndex] = true;
            }
        }
        
        // Create new tracked vehicles for unmatched detections
        for (int i = 0; i < detections.size(); i++) {
            if (!detectionMatched[i]) {
                const auto& detection = detections[i];
                
                TrackedVehicle newVehicle;
                newVehicle.id = nextVehicleId++;
                newVehicle.currentBox = detection.box;
                newVehicle.previousBox = detection.box;
                newVehicle.centerCurrent = ofPoint(
                    detection.box.x + detection.box.width / 2,
                    detection.box.y + detection.box.height / 2
                );
                newVehicle.centerPrevious = newVehicle.centerCurrent;
                newVehicle.vehicleType = detection.classId;
                newVehicle.className = detection.className;
                newVehicle.confidence = detection.confidence;
                newVehicle.framesSinceLastSeen = 0;
                newVehicle.hasMovement = false;
                newVehicle.speed = 0.0f;
                newVehicle.speedMph = 0.0f;
                newVehicle.isOccluded = false;
                
                trackedVehicles.push_back(newVehicle);
            }
        }
        
        // Increment frame counter for unmatched existing vehicles
        for (int i = 0; i < trackedVehicles.size(); i++) {
            if (!vehicleMatched[i]) {
                trackedVehicles[i].framesSinceLastSeen++;
            }
        }
    }
    
    // Remove vehicles that haven't been seen for too long
    trackedVehicles.erase(
        remove_if(trackedVehicles.begin(), trackedVehicles.end(),
            [this](const TrackedVehicle& vehicle) {
                return vehicle.framesSinceLastSeen > maxFramesWithoutDetection;
            }),
        trackedVehicles.end()
    );
}

void DetectionManager::checkLineCrossings() {
    if (!lineManager || !communicationManager) {
        // ofLogNotice() << "Line crossing check: managers not initialized";
        return;
    }
    
    const vector<LineManager::MidiLine>& lines = lineManager->getLines();
    if (lines.empty()) {
        // ofLogNotice() << "Line crossing check: no lines drawn";
        return;
    }
    
    if (trackedVehicles.empty()) {
        // ofLogNotice() << "Line crossing check: no tracked vehicles";
        return;
    }
    
    // Check each tracked vehicle against each line
    for (const auto& vehicle : trackedVehicles) {
        if (!vehicle.hasMovement) continue;  // Skip stationary vehicles
        
        for (int lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
            const auto& line = lines[lineIndex];
            ofPoint intersection;
            
            // Check if vehicle's movement path crosses this line
            if (lineSegmentIntersection(
                vehicle.centerPrevious, vehicle.centerCurrent,
                line.startPoint, line.endPoint,
                intersection)) {
                
                // Create crossing event
                LineCrossEvent event;
                event.lineId = lineIndex;
                event.vehicleId = vehicle.id;
                event.vehicleType = vehicle.vehicleType;
                event.className = vehicle.className;
                event.confidence = vehicle.confidence;
                event.speed = vehicle.speed;
                event.speedMph = vehicle.speedMph;
                event.timestamp = ofGetElapsedTimeMillis();
                event.crossingPoint = intersection;
                event.processed = false;
                
                // Send OSC message
                communicationManager->sendOSCLineCrossing(event.lineId, event.vehicleId, 
                    event.vehicleType, event.className, event.confidence, 
                    event.speed, event.speedMph, event.crossingPoint);
                
                // Send MIDI message
                communicationManager->sendMIDILineCrossing(event.lineId, event.className, 
                    event.confidence, event.speed);
                
                crossingEvents.push_back(event);
                
                ofLogNotice() << "DetectionManager: Line crossing - Vehicle " << event.vehicleId 
                             << " (" << event.className << ") crossed line " << event.lineId;
            }
        }
    }
}

float DetectionManager::calculateDistance(const ofPoint& p1, const ofPoint& p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

bool DetectionManager::lineSegmentIntersection(const ofPoint& line1Start, const ofPoint& line1End,
                                              const ofPoint& line2Start, const ofPoint& line2End,
                                              ofPoint& intersection) {
    float x1 = line1Start.x, y1 = line1Start.y;
    float x2 = line1End.x, y2 = line1End.y;
    float x3 = line2Start.x, y3 = line2Start.y;
    float x4 = line2End.x, y4 = line2End.y;
    
    float denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    
    if (abs(denominator) < 0.0001f) {
        return false; // Lines are parallel
    }
    
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denominator;
    
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        intersection.x = x1 + t * (x2 - x1);
        intersection.y = y1 + t * (y2 - y1);
        return true;
    }
    
    return false;
}

void DetectionManager::updateTrajectoryHistory(TrackedVehicle& vehicle) {
    // Add current position to trajectory
    vehicle.trajectory.push_back(vehicle.centerCurrent);
    vehicle.trajectoryTimes.push_back(ofGetElapsedTimef());
    
    // Limit trajectory length
    if (vehicle.trajectory.size() > vehicle.maxTrajectoryLength) {
        vehicle.trajectory.erase(vehicle.trajectory.begin());
        vehicle.trajectoryTimes.erase(vehicle.trajectoryTimes.begin());
    }
}

void DetectionManager::calculateVelocityAndAcceleration(TrackedVehicle& vehicle) {
    if (vehicle.trajectory.size() < 2) return;
    
    // Calculate velocity (direction and magnitude)
    ofPoint current = vehicle.trajectory.back();
    ofPoint previous = vehicle.trajectory[vehicle.trajectory.size() - 2];
    
    vehicle.velocity = ofPoint(current.x - previous.x, current.y - previous.y);
    
    float currentSpeed = vehicle.velocity.length();
    vehicle.acceleration = currentSpeed - vehicle.speed;
    vehicle.speed = currentSpeed;
}

void DetectionManager::handleOccludedVehicles() {
    for (auto& vehicle : trackedVehicles) {
        if (vehicle.framesSinceLastSeen > 3 && vehicle.framesSinceLastSeen <= maxFramesWithoutDetection) {
            vehicle.isOccluded = true;
            vehicle.predictionConfidence = 1.0f - (vehicle.framesSinceLastSeen / float(maxFramesWithoutDetection));
        }
    }
}

void DetectionManager::cleanupOldTrajectoryPoints() {
    float currentTime = ofGetElapsedTimef();
    float maxAge = 5.0f; // Keep trajectory points for 5 seconds
    
    for (auto& vehicle : trackedVehicles) {
        // Remove old trajectory points
        while (!vehicle.trajectoryTimes.empty() && 
               (currentTime - vehicle.trajectoryTimes.front()) > maxAge) {
            vehicle.trajectory.erase(vehicle.trajectory.begin());
            vehicle.trajectoryTimes.erase(vehicle.trajectoryTimes.begin());
        }
    }
}

// Safe versions of vehicle tracking methods with better error handling
void DetectionManager::updateVehicleTrackingSafe() {
    try {
        // Basic safety checks
        if (!enableDetection || !yoloLoaded) {
            return;
        }
        
        // RESTORED: Proper vehicle tracking algorithm from working backup
        
        // Update existing tracked vehicles with new detections
        for (auto& vehicle : trackedVehicles) {
            vehicle.framesSinceLastSeen++;
            vehicle.hasMovement = false;
        }
        
        // Try to match current detections with tracked vehicles
        for (const auto& detection : detections) {
            // Track any class that is currently selected for detection
            bool isTrackable = std::find(selectedClassIds.begin(), selectedClassIds.end(), detection.classId) != selectedClassIds.end();
            if (!isTrackable) continue;
            
            ofPoint detectionCenter = ofPoint(
                detection.box.x + detection.box.width / 2,
                detection.box.y + detection.box.height / 2
            );
            
            // Find closest tracked vehicle
            int bestMatchIndex = -1;
            float bestDistance = vehicleTrackingThreshold;
            
            for (int i = 0; i < trackedVehicles.size(); i++) {
                float distance = calculateDistance(detectionCenter, trackedVehicles[i].centerCurrent);
                if (distance < bestDistance && trackedVehicles[i].vehicleType == detection.classId) {
                    bestDistance = distance;
                    bestMatchIndex = i;
                }
            }
            
            if (bestMatchIndex >= 0) {
                // Update existing vehicle - CRITICAL FOR LINE CROSSING
                TrackedVehicle& vehicle = trackedVehicles[bestMatchIndex];
                vehicle.previousBox = vehicle.currentBox;
                vehicle.centerPrevious = vehicle.centerCurrent;  // CRITICAL: Save previous position
                vehicle.currentBox = detection.box;
                vehicle.centerCurrent = detectionCenter;
                vehicle.confidence = detection.confidence;
                vehicle.framesSinceLastSeen = 0;
                
                // Calculate movement and speed - CRITICAL FOR LINE CROSSING
                float distance = calculateDistance(vehicle.centerCurrent, vehicle.centerPrevious);
                vehicle.hasMovement = distance > 2.0f;  // CRITICAL: Only vehicles with movement can cross lines
                vehicle.speed = distance; // pixels per frame
                
                // Rough speed estimation (assumes ~30fps, 1 pixel ≈ 0.5 feet at highway distance)
                vehicle.speedMph = vehicle.speed * 30.0f * 0.5f * 0.681818f; // pixels/frame → mph
                
                // Update trajectory for enhanced tracking
                updateTrajectoryHistory(vehicle);
                calculateVelocityAndAcceleration(vehicle);
                
            } else {
                // Create new tracked object (any COCO class)
                TrackedVehicle newVehicle;  // Note: structure name kept for compatibility
                newVehicle.id = nextVehicleId++;
                newVehicle.currentBox = detection.box;
                newVehicle.previousBox = detection.box;
                newVehicle.centerCurrent = detectionCenter;
                newVehicle.centerPrevious = detectionCenter;  // Initially same position
                newVehicle.vehicleType = detection.classId;  // Note: stores any object type
                newVehicle.className = detection.className;
                newVehicle.confidence = detection.confidence;
                newVehicle.framesSinceLastSeen = 0;
                newVehicle.hasMovement = false;  // New objects start with no movement
                newVehicle.speed = 0.0f;
                newVehicle.speedMph = 0.0f;
                
                // Enhanced tracking initialization
                newVehicle.velocity = ofPoint(0, 0);
                newVehicle.acceleration = 0.0f;
                newVehicle.trailColor = ofColor::blue;
                newVehicle.isOccluded = false;
                newVehicle.predictionConfidence = 1.0f;
                newVehicle.maxTrajectoryLength = 30;
                
                // Initialize trajectory with current position
                updateTrajectoryHistory(newVehicle);
                
                trackedVehicles.push_back(newVehicle);
                
                ofLogNotice() << "New object tracked: ID " << newVehicle.id 
                             << " (" << newVehicle.className << ") - class " << detection.classId;
            }
        }
        
        // Remove objects that haven't been seen for too long
        trackedVehicles.erase(
            std::remove_if(trackedVehicles.begin(), trackedVehicles.end(),
                [this](const TrackedVehicle& v) {
                    if (v.framesSinceLastSeen > maxFramesWithoutDetection) {
                        ofLogNotice() << "Object lost: ID " << v.id << " (" << v.className << ")";
                        return true;
                    }
                    return false;
                }),
            trackedVehicles.end()
        );
        
    } catch (const std::exception& e) {
        ofLogError() << "DetectionManager: Exception in updateVehicleTrackingSafe: " << e.what();
    }
}

void DetectionManager::checkLineCrossingsSafe() {
    try {
        // Safety checks
        if (!lineManager || !communicationManager || trackedVehicles.empty()) {
            return;
        }
        
        const vector<LineManager::MidiLine>& lines = lineManager->getLines();
        if (lines.empty()) {
            return;
        }
        
        // Check each vehicle against each line with safe iteration
        for (size_t vehicleIndex = 0; vehicleIndex < trackedVehicles.size(); vehicleIndex++) {
            const auto& vehicle = trackedVehicles[vehicleIndex];
            
            if (!vehicle.hasMovement) continue;
            
            for (size_t lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
                const auto& line = lines[lineIndex];
                ofPoint intersection;
                
                // Simple line crossing check using vehicle center movement
                if (lineSegmentIntersection(
                    vehicle.centerPrevious, vehicle.centerCurrent,
                    line.startPoint, line.endPoint,
                    intersection)) {
                    
                    // Send OSC message safely
                    communicationManager->sendOSCLineCrossing(lineIndex, vehicle.id, 
                        vehicle.vehicleType, vehicle.className, vehicle.confidence, 
                        vehicle.speed, vehicle.speedMph, intersection);
                    
                    // Send MIDI message safely
                    ofLogNotice() << "DEBUG: About to send MIDI for line crossing - Line:" << lineIndex;
                    communicationManager->sendMIDILineCrossing(lineIndex, vehicle.className, 
                        vehicle.confidence, vehicle.speed);
                    
                    ofLogNotice() << "DetectionManager: Line crossing - Vehicle " << vehicle.id 
                                  << " (" << vehicle.className << ") crossed line " << lineIndex;
                    
                    // Only process one crossing per vehicle per frame
                    break;
                }
            }
        }
        
    } catch (const std::exception& e) {
        ofLogError() << "DetectionManager: Exception in checkLineCrossingsSafe: " << e.what();
    }
}

void DetectionManager::cleanupOldVehicles() {
    try {
        // Remove vehicles that haven't been seen for too long
        trackedVehicles.erase(
            std::remove_if(trackedVehicles.begin(), trackedVehicles.end(),
                [this](const TrackedVehicle& v) {
                    return v.framesSinceLastSeen > maxFramesWithoutDetection;
                }),
            trackedVehicles.end()
        );
        
    } catch (const std::exception& e) {
        ofLogError() << "DetectionManager: Exception in cleanupOldVehicles: " << e.what();
    }
}