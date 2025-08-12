#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // EXACT same setup logic as working backup, just organized into managers
    ofSetFrameRate(60);  // Let app run at 60fps for responsiveness
    ofSetBackgroundColor(0, 0, 0);  // Black background instead of green
    
    // Initialize window resize tracking - EXACT COPY from working backup
    originalWindowWidth = 0;
    originalWindowHeight = 0;
    
    // Initialize managers with EXACT same logic from working backup
    videoManager.setup();
    lineManager.setup();
    detectionManager.setup();
    uiManager.setup();
    communicationManager.setup();
    configManager.setup();
    
    // Initialize pose detection (new creative instrument)
    poseDetectionEnabled = false;  // Disabled by default
    // Note: poseReceiver.setup() will be called when user enables pose detection
    
    // Connect managers together - CRITICAL for modular system
    detectionManager.setVideoManagers(&videoManager);
    detectionManager.setVideoSource((int)videoManager.getCurrentVideoSource());
    
    // CRITICAL: Connect DetectionManager to LineManager and CommunicationManager for line crossing
    detectionManager.setLineManager(&lineManager);
    detectionManager.setCommunicationManager(&communicationManager);
    
    uiManager.setManagers(&videoManager, &lineManager, &detectionManager, 
                         &communicationManager, &configManager, 
                         &poseReceiver, &poseDetectionEnabled);
    
    communicationManager.setManagers(&lineManager);
    
    configManager.setManagers(&uiManager, &lineManager, &videoManager, 
                             &detectionManager, &communicationManager);
    
    // Load configuration - EXACT same as working backup
    configManager.loadConfig();
}

//--------------------------------------------------------------
void ofApp::update(){
    // EXACT same update logic as working backup, just organized into managers
    videoManager.update();
    
    // Process detection only if video has new frame - EXACT same logic
    if (detectionManager.shouldProcess()) {
        detectionManager.update();
    }
    
    // Update pose detection (new creative instrument)
    if (poseDetectionEnabled && poseReceiver.isConnected()) {
        poseReceiver.update();
        
        // Process pose line crossings if we have new data
        if (poseReceiver.hasNewData()) {
            // TODO: Implement pose line crossing detection
            // This will be similar to existing vehicle line crossing but for pose landmarks
        }
    }
    
    lineManager.update();
    communicationManager.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // EXACT same draw logic as working backup, just organized into managers
    
    // Clear background - EXACT same
    ofSetColor(40, 40, 40);  // Dark gray background
    ofFill();
    ofDrawRectangle(0, 0, 640, 640);
    
    // Draw video - EXACT same
    videoManager.draw();
    
    // Draw lines - EXACT same
    lineManager.draw();
    
    // Draw detections - EXACT same
    detectionManager.draw();
    
    // Draw pose skeletons (new creative instrument visual feedback)
    drawPoseSkeletons();
    
    // Draw GUI - EXACT same
    uiManager.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
    // EXACT same exit logic
    configManager.saveConfig();
}

//--------------------------------------------------------------
ofApp::~ofApp() {
    // EXACT same cleanup
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // EXACT same keyboard handling - just delegate to managers
    
    // Check GUI first - EXACT same logic
    if (uiManager.wantsKeyboard()) {
        return;
    }
    
    // Video controls - EXACT same
    if (key == 'r' || key == 'R') {
        videoManager.handleCameraRestart();
        return;
    }
    
    if (key == 'v' || key == 'V') {
        videoManager.handleVideoSourceSwitch();
        return;
    }
    
    if (key == 'o' || key == 'O') {
        videoManager.handleVideoFileOpen();
        return;
    }
    
    // Detection controls - EXACT same
    if (key == 'd' || key == 'D') {
        detectionManager.toggleDetection();
        return;
    }
    
    // Line controls - EXACT same
    if (key == 'c' || key == 'C') {
        lineManager.clearAllLines();
        return;
    }
    
    // GUI controls - EXACT same
    if (key == 'g' || key == 'G') {
        uiManager.toggleGUI();
        return;
    }
    
    // MIDI controls - EXACT same
    if (key == 't' || key == 'T') {
        communicationManager.sendTestMIDINote();
        return;
    }
    
    // Line editing - EXACT same
    if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE) {
        lineManager.deleteSelectedLine();
        return;
    }
    
    if (key == OF_KEY_ESC) {
        lineManager.selectLine(-1);  // Deselect
        return;
    }
    
    // Video playback - EXACT same (delegated to videoManager.handleVideoKeyPress)
    videoManager.handleVideoKeyPress(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    // EXACT same - minimal
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    lineManager.handleMouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    lineManager.handleMouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    lineManager.handleMousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    lineManager.handleMouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    // EXACT same - minimal
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    // EXACT same - minimal
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    // EXACT same - minimal
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    // Attempt to enforce fixed window size by resetting it - EXACT COPY from working backup
    if (w != 1050 || h != 640) {
        ofSetWindowShape(1050, 640);
        ofLogNotice() << "Enforcing fixed window size 1050x640";
        return;
    }
    
    // Store original window dimensions for resize detection - EXACT COPY from working backup
    if (originalWindowWidth == 0 && originalWindowHeight == 0) {
        originalWindowWidth = w;
        originalWindowHeight = h;
    }
    
    ofLogNotice() << "Window dimensions confirmed: " << w << "x" << h;
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    // EXACT same drag handling
    if (dragInfo.files.size() > 0) {
        string filePath = dragInfo.files[0];
        ofLogNotice() << "File dropped: " << filePath;
        // Delegate to videoManager
        videoManager.handleVideoFileOpen();
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    // EXACT same - minimal
}

//--------------------------------------------------------------
void ofApp::drawPoseSkeletons(){
    // Only draw if pose detection is enabled and we have data
    if (!poseDetectionEnabled || !poseReceiver.isConnected()) {
        return;
    }
    
    // Get current poses
    auto poses = poseReceiver.getCurrentPoses();
    if (poses.empty()) {
        return;
    }
    
    // Draw each person with a different color
    ofColor colors[] = {
        ofColor(255, 100, 100),  // Red
        ofColor(100, 255, 100),  // Green  
        ofColor(100, 100, 255),  // Blue
        ofColor(255, 255, 100),  // Yellow
        ofColor(255, 100, 255),  // Magenta
        ofColor(100, 255, 255),  // Cyan
        ofColor(255, 200, 100),  // Orange
        ofColor(200, 100, 255)   // Purple
    };
    
    for (size_t i = 0; i < poses.size() && i < 8; i++) {
        ofColor poseColor = colors[i % 8];
        drawPoseSkeleton(poses[i], poseColor);
    }
}

//--------------------------------------------------------------
void ofApp::drawPoseSkeleton(const PersonPose& pose, ofColor color){
    if (!pose.isValid || pose.landmarks.empty()) {
        return;
    }
    
    // MediaPipe sends coordinates in 640x640 space, openFrameworks displays in 640x640 space
    // Perfect 1:1 mapping - no scaling needed for YOLOv8 compatibility  
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    
    // MediaPipe 33-point pose connections for drawing skeleton
    // Each pair represents a connection between two landmark indices
    vector<pair<int, int>> connections = {
        // Face connections
        {0, 1}, {1, 2}, {2, 3}, {3, 7},     // Face outline
        {0, 4}, {4, 5}, {5, 6}, {6, 8},     // Face outline
        
        // Body connections
        {11, 12},  // Shoulders
        {11, 13}, {13, 15},  // Left arm
        {12, 14}, {14, 16},  // Right arm
        {11, 23}, {12, 24},  // Shoulder to hip
        {23, 24},  // Hips
        {23, 25}, {25, 27},  // Left leg
        {24, 26}, {26, 28},  // Right leg
        
        // Hand connections (simplified)
        {15, 17}, {15, 19}, {15, 21},  // Left hand
        {16, 18}, {16, 20}, {16, 22},  // Right hand
        
        // Foot connections
        {27, 29}, {27, 31},  // Left foot
        {28, 30}, {28, 32}   // Right foot
    };
    
    // Draw skeleton connections
    ofSetColor(color, 180);  // Semi-transparent
    ofSetLineWidth(3);
    
    for (const auto& connection : connections) {
        int idx1 = connection.first;
        int idx2 = connection.second;
        
        // Check if both landmarks exist and are confident enough
        if (idx1 < pose.landmarks.size() && idx2 < pose.landmarks.size()) {
            const auto& landmark1 = pose.landmarks[idx1];
            const auto& landmark2 = pose.landmarks[idx2];
            
            // Only draw if both landmarks are confident
            if (landmark1.confidence > 0.5f && landmark2.confidence > 0.5f) {
                float x1 = landmark1.x * scaleX;
                float y1 = landmark1.y * scaleY;
                float x2 = landmark2.x * scaleX;
                float y2 = landmark2.y * scaleY;
                
                ofDrawLine(x1, y1, x2, y2);
            }
        }
    }
    
    // Draw joint points
    ofSetColor(color);
    ofFill();
    
    for (const auto& landmark : pose.landmarks) {
        if (landmark.confidence > 0.5f) {  // Only draw confident landmarks
            float x = landmark.x * scaleX;
            float y = landmark.y * scaleY;
            
            // Different sizes for different joint types
            float radius = 4;
            if (landmark.id == 0) radius = 6;  // Nose - larger
            if (landmark.id >= 11 && landmark.id <= 16) radius = 5;  // Arms - medium
            if (landmark.id >= 23 && landmark.id <= 28) radius = 5;  // Legs - medium
            
            ofDrawCircle(x, y, radius);
        }
    }
    
    // Draw person ID label
    if (!pose.landmarks.empty()) {
        const auto& noseLandmark = pose.landmarks[0];  // Use nose position
        if (noseLandmark.confidence > 0.3f) {
            float labelX = noseLandmark.x * scaleX;
            float labelY = noseLandmark.y * scaleY - 20;  // Above head
            
            ofSetColor(color);
            string label = "Person " + ofToString(pose.personID) + 
                          " (" + ofToString(pose.overallConfidence * 100, 0) + "%)";
            ofDrawBitmapString(label, labelX, labelY);
        }
    }
}