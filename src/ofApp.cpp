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
    
    // Connect managers together - CRITICAL for modular system
    detectionManager.setVideoManagers(&videoManager);
    detectionManager.setVideoSource((int)videoManager.getCurrentVideoSource());
    
    // CRITICAL: Connect DetectionManager to LineManager and CommunicationManager for line crossing
    detectionManager.setLineManager(&lineManager);
    detectionManager.setCommunicationManager(&communicationManager);
    
    // Setup tempo manager
    tempoManager.setup();
    
    // Setup scale manager
    scaleManager.setup();
    
    // Connect TempoManager to LineManager
    lineManager.setTempoManager(&tempoManager);
    
    uiManager.setManagers(&videoManager, &lineManager, &detectionManager, 
                         &communicationManager, &configManager, &tempoManager, &scaleManager);
    
    communicationManager.setManagers(&lineManager);
    communicationManager.setScaleManager(&scaleManager);
    
    configManager.setManagers(&uiManager, &lineManager, &videoManager, 
                             &detectionManager, &communicationManager, &tempoManager, &scaleManager);
    
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