#include "VideoManager.h"

VideoManager::VideoManager() {
    // Initialize video mode flags - EXACT COPY from working backup
    useVideoFile = false;
    videoLoaded = false;
    videoPaused = false;
    currentVideoPath = "";
    currentVideoSource = CAMERA;  // Default to camera
    
    // Initialize USB camera device variables
    currentCameraDeviceID = 0;  // Default to first camera
    currentCameraName = "Default Camera";
    
    // Initialize IP camera configuration - EXACT COPY from working backup
    ipCameraUrl = "http://localhost:8080/video";  // USB forwarded IP Webcam URL
    ipCameraConnected = false;
    // Simple IP camera setup with performance optimization
    ipFrameReady = false;
    lastFrameRequest = 0.0f;
    frameRequestInterval = 0.5f;  // 2fps for much better performance
    ipFrameSkip = 1;              // Process every frame requested
    ipFrameCounter = 0;
}

VideoManager::~VideoManager() {
    if (camera.isInitialized()) {
        camera.close();
    }
    if (videoPlayer.isLoaded()) {
        videoPlayer.close();
    }
}

void VideoManager::setup() {
    // Enumerate available cameras first
    refreshCameraDevices();
    
    // Try to load test video file first (optional) - EXACT COPY from working backup
    videoPlayer.load("test_video.mp4");
    if (videoPlayer.isLoaded()) {
        videoLoaded = true;
        videoPlayer.setLoopState(OF_LOOP_NORMAL);
        videoPlayer.setVolume(0.0f);  // Mute audio
        videoPlayer.play();
        videoPaused = false;
        useVideoFile = true;  // Start with video if available
        currentVideoPath = "test_video.mp4";
        ofLogNotice() << "Test video loaded: test_video.mp4 (audio muted)";
    } else {
        ofLogNotice() << "No test video found, will use camera. Press 'o' to open video file.";
    }
    
    // Initialize camera at compatible resolution with device ID support
    camera.setDesiredFrameRate(30);
    camera.setDeviceID(currentCameraDeviceID);
    // Try different common resolutions for better compatibility
    bool cameraSetup = false;
    
    // Try HD 1280x720 first for high quality detection
    if (camera.setup(1280, 720)) {
        cameraSetup = true;
        ofLogNotice() << "Camera (" << currentCameraName << ") set to HD 1280x720";
    }
    // Fallback to 640x480 if HD not supported
    else if (camera.setup(640, 480)) {
        cameraSetup = true;
        ofLogNotice() << "Camera (" << currentCameraName << ") fallback to 640x480";
    } 
    // Lower resolution fallback
    else if (camera.setup(320, 240)) {
        cameraSetup = true;
        ofLogNotice() << "Camera (" << currentCameraName << ") fallback to 320x240";
    }
    // Last resort - let camera choose
    else if (camera.setup(0, 0)) {
        cameraSetup = true;
        ofLogNotice() << "Camera (" << currentCameraName << ") auto-resolution: " << camera.getWidth() << "x" << camera.getHeight();
    }
    
    // Check if camera connected (keep fixed window size)
    cameraConnected = camera.isInitialized();
    
    if (cameraConnected) {
        ofLogNotice() << "Camera initialized: " << camera.getWidth() << "x" << camera.getHeight();
    }
    
    if (!cameraConnected && !videoLoaded) {
        ofLogError() << "Neither camera nor test video available!";
    } else if (!cameraConnected) {
        ofLogNotice() << "Camera not available, using video file only";
    }
    
    // Final video source validation - ensure we start with a working source
    validateAndFixVideoSource();
}

void VideoManager::update() {
    // Update video sources based on current source - EXACT COPY from working backup
    switch (currentVideoSource) {
        case CAMERA:
            if (cameraConnected) {
                camera.update();
            }
            break;
        case VIDEO_FILE:
            if (videoLoaded) {
                videoPlayer.update();
            }
            break;
        case IP_CAMERA:
            // Request new frame with frame skipping for performance
            if (ipCameraConnected && ofGetElapsedTimef() - lastFrameRequest > frameRequestInterval) {
                ipFrameCounter++;
                if (ipFrameCounter >= ipFrameSkip) {
                    // Load new frame via HTTP with proper handling
                    ofBuffer imageBuffer = ofLoadURL(ipCameraSnapshotUrl).data;
                    if (imageBuffer.size() > 0) {
                        ofImage newFrame;
                        if (newFrame.loadImage(imageBuffer)) {
                            // Resize to 320x240 for much better performance  
                            newFrame.resize(320, 240);
                            currentIPFrame = newFrame;
                            ipFrameReady = true;
                        }
                    }
                    ipFrameCounter = 0; // Reset counter
                }
                lastFrameRequest = ofGetElapsedTimef();
            }
            break;
    }
    
    // Backward compatibility: also update based on useVideoFile flag - EXACT COPY
    if (useVideoFile && videoLoaded) {
        videoPlayer.update();
    } else if (cameraConnected && currentVideoSource == CAMERA) {
        camera.update();
    }
}

void VideoManager::draw() {
    // Draw video source in left 640x640 area only - EXACT COPY from working backup
    ofSetColor(255, 255, 255);  // White color for video
    
    bool videoDrawn = false;
    
    // Primary video source drawing - EXACT COPY
    switch (currentVideoSource) {
        case CAMERA:
            if (cameraConnected && camera.isInitialized()) {
                camera.draw(0, 0, 640, 640);
                videoDrawn = true;
            }
            break;
        case VIDEO_FILE:
            if (videoLoaded && videoPlayer.isLoaded()) {
                videoPlayer.draw(0, 0, 640, 640);
                videoDrawn = true;
            }
            break;
        case IP_CAMERA:
            if (ipCameraConnected && ipFrameReady && currentIPFrame.isAllocated()) {
                currentIPFrame.draw(0, 0, 640, 640);
                videoDrawn = true;
            } else if (ipCameraConnected) {
                // Show loading message
                ofSetColor(255, 255, 255);
                ofDrawBitmapString("Loading IP camera frame...", 20, 320);
            } else {
                // Show disconnected message
                ofSetColor(255, 255, 0);
                ofDrawBitmapString("IP Camera not connected. Press 'v' to switch source.", 20, 300);
                ofDrawBitmapString("Use GUI to connect to IP camera.", 20, 320);
            }
            break;
    }
    
    // Fallback: if no video drawn and we have other sources available - EXACT COPY
    if (!videoDrawn) {
        if (videoLoaded && videoPlayer.isLoaded()) {
            ofSetColor(255, 255, 255);
            videoPlayer.draw(0, 0, 640, 640);
            videoDrawn = true;
        } else if (cameraConnected && camera.isInitialized()) {
            ofSetColor(255, 255, 255);
            camera.draw(0, 0, 640, 640);
            videoDrawn = true;
        }
    }
    
    // If still no video, show helpful message - EXACT COPY
    if (!videoDrawn) {
        ofSetColor(255, 255, 0);
        ofDrawBitmapString("No video source available:", 20, 300);
        ofDrawBitmapString("Camera: " + string(cameraConnected ? "Connected" : "Not connected"), 20, 320);
        ofDrawBitmapString("Video: " + string(videoLoaded ? "Loaded" : "Not loaded"), 20, 340);
        ofDrawBitmapString("Press 'v' to switch sources or 'o' to load video", 20, 360);
    }
}

// EXACT COPY from working backup
void VideoManager::validateAndFixVideoSource() {
    bool currentSourceWorking = false;
    
    // Check if current video source is actually working
    switch (currentVideoSource) {
        case CAMERA:
            currentSourceWorking = (cameraConnected && camera.isInitialized());
            break;
        case VIDEO_FILE:
            currentSourceWorking = (videoLoaded && videoPlayer.isLoaded());
            break;
        case IP_CAMERA:
            currentSourceWorking = (ipCameraConnected && ipFrameReady);
            break;
    }
    
    // If current source isn't working, find a working one
    if (!currentSourceWorking) {
        ofLogNotice() << "Current video source not working, finding alternative...";
        
        // Try video file first (more reliable)
        if (videoLoaded && videoPlayer.isLoaded()) {
            currentVideoSource = VIDEO_FILE;
            useVideoFile = true;
            ofLogNotice() << "Switched to video file: " << currentVideoPath;
        }
        // Then try camera
        else if (cameraConnected && camera.isInitialized()) {
            currentVideoSource = CAMERA;
            useVideoFile = false;
            ofLogNotice() << "Switched to camera";
        }
        // Keep IP_CAMERA mode but user will see helpful message
        else {
            ofLogNotice() << "No working video sources available";
        }
    }
}

// Camera restart functionality - EXACT COPY from working backup key handler
void VideoManager::handleCameraRestart() {
    camera.close();
    camera.setDesiredFrameRate(30);
    
    // Try HD first, then fallback resolutions
    bool cameraSetup = false;
    if (camera.setup(1280, 720)) {
        cameraSetup = true;
        ofLogNotice() << "Camera restarted at HD 1280x720";
    } else if (camera.setup(640, 480)) {
        cameraSetup = true;
        ofLogNotice() << "Camera restarted at 640x480 (fallback)";
    } else if (camera.setup(320, 240)) {
        cameraSetup = true;
        ofLogNotice() << "Camera restarted at 320x240 (low res fallback)";
    }
    
    cameraConnected = camera.isInitialized();
    
    if (cameraConnected) {
        ofLogNotice() << "Camera restart successful: " << camera.getWidth() << "x" << camera.getHeight();
    } else {
        ofLogError() << "Camera restart failed";
    }
}

// EXACT COPY from working backup - get pixels for detection
ofPixels VideoManager::getCurrentPixels() {
    ofPixels pixels;
    
    switch (currentVideoSource) {
        case CAMERA:
            if (cameraConnected) {
                pixels = camera.getPixels();
            }
            break;
        case VIDEO_FILE:
            if (videoLoaded) {
                pixels = videoPlayer.getPixels();
            }
            break;
        case IP_CAMERA:
            if (ipCameraConnected && ipFrameReady && currentIPFrame.isAllocated()) {
                pixels = currentIPFrame.getPixels();
            }
            break;
    }
    
    // Backward compatibility fallback
    if (pixels.size() == 0) {
        if (useVideoFile && videoLoaded) {
            pixels = videoPlayer.getPixels();
        } else if (cameraConnected) {
            pixels = camera.getPixels();
        }
    }
    
    return pixels;
}

// Configuration methods - EXACT COPY from working backup
void VideoManager::saveToJSON(ofxJSONElement& json) {
    json["useVideoFile"] = useVideoFile;
    json["currentVideoPath"] = currentVideoPath;
    json["currentVideoSource"] = (int)currentVideoSource;
    json["ipCameraUrl"] = ipCameraUrl;
    json["frameRequestInterval"] = frameRequestInterval;
    json["ipFrameSkip"] = ipFrameSkip;
    json["currentCameraDeviceID"] = currentCameraDeviceID;
    json["currentCameraName"] = currentCameraName;
}

void VideoManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("useVideoFile")) {
        useVideoFile = json["useVideoFile"].asBool();
    }
    if (json.isMember("currentVideoPath")) {
        currentVideoPath = json["currentVideoPath"].asString();
    }
    if (json.isMember("currentVideoSource")) {
        currentVideoSource = (VideoSource)json["currentVideoSource"].asInt();
    }
    if (json.isMember("ipCameraUrl")) {
        ipCameraUrl = json["ipCameraUrl"].asString();
    }
    if (json.isMember("frameRequestInterval")) {
        frameRequestInterval = json["frameRequestInterval"].asFloat();
    }
    if (json.isMember("ipFrameSkip")) {
        ipFrameSkip = json["ipFrameSkip"].asInt();
    }
    if (json.isMember("currentCameraDeviceID")) {
        currentCameraDeviceID = json["currentCameraDeviceID"].asInt();
    }
    if (json.isMember("currentCameraName")) {
        currentCameraName = json["currentCameraName"].asString();
    }
    
    ofLogNotice() << "VideoManager: Configuration loaded";
}

void VideoManager::setDefaults() {
    useVideoFile = false;
    videoLoaded = false;
    videoPaused = false;
    cameraConnected = false;
    currentVideoPath = "";
    currentVideoSource = CAMERA;
    ipCameraUrl = "";
    ipCameraSnapshotUrl = "";
    ipCameraConnected = false;
    ipFrameReady = false;
    lastFrameRequest = 0;
    frameRequestInterval = 33.0f;
    ipFrameSkip = 1;
    ipFrameCounter = 0;
    currentCameraDeviceID = 0;
    currentCameraName = "Default Camera";
    
    ofLogNotice() << "VideoManager: Set to default values";
}

// Missing UI methods referenced from UIManager and ofApp
void VideoManager::openVideoFileDialog() {
    ofFileDialogResult result = ofSystemLoadDialog("Load video file");
    if (result.bSuccess) {
        currentVideoPath = result.getPath();
        
        if (videoPlayer.load(currentVideoPath)) {
            videoLoaded = true;
            useVideoFile = true;
            currentVideoSource = VIDEO_FILE;
            videoPlayer.setVolume(0.0f);  // Mute audio - CRITICAL FIX
            videoPlayer.play();
            videoPaused = false;
            ofLogNotice() << "VideoManager: Loaded video from dialog: " << result.getName() << " (audio muted)";
        } else {
            ofLogError() << "VideoManager: Failed to load video from dialog: " << result.getName();
        }
    }
}

void VideoManager::setupCamera() {
    initializeCamera();
}

void VideoManager::initializeCamera() {
    // EXACT COPY from working backup camera initialization
    cameraConnected = false;
    
    // Try to setup camera with fallback logic  
    if (trySetupCamera()) {
        cameraConnected = true;
        useVideoFile = false;
        currentVideoSource = CAMERA;
        ofLogNotice() << "VideoManager: Camera initialized successfully";
    } else {
        cameraConnected = false;
        ofLogError() << "VideoManager: Camera initialization failed";
        
        // Fall back to video file if available
        if (!currentVideoPath.empty()) {
            useVideoFile = true;
            currentVideoSource = VIDEO_FILE;
            ofLogNotice() << "VideoManager: Falling back to video file";
        }
    }
}

bool VideoManager::trySetupCamera() {
    // EXACT COPY from working backup camera setup logic
    try {
        // Set desired camera resolution  
        camera.setDesiredFrameRate(30);
        camera.setDeviceID(currentCameraDeviceID);  // Use selected camera device
        
        // Try HD resolution first - EXACT COPY from working backup
        // Try HD first, then fallback resolutions
        if (camera.setup(1280, 720)) {
            ofLogNotice() << "VideoManager: Camera setup successful at HD 1280x720";
            return true;
        } else if (camera.setup(640, 480)) {
            ofLogNotice() << "VideoManager: Camera setup successful at 640x480 (fallback)";
            return true;
        } else if (camera.setup(320, 240)) {
            ofLogNotice() << "VideoManager: Camera setup successful at 320x240 (low res fallback)";
            return true;
        }
    } catch (const std::exception& e) {
        ofLogError() << "VideoManager: Camera setup failed with exception: " << e.what();
    }
    
    ofLogError() << "VideoManager: Camera setup failed completely";
    return false;
}

void VideoManager::handleVideoKeyPress(int key) {
    switch(key) {
        case ' ':  // SPACE - play/pause
            if (currentVideoSource == VIDEO_FILE && videoLoaded) {
                if (videoPaused) {
                    videoPlayer.play();
                    videoPaused = false;
                    ofLogNotice() << "VideoManager: Video resumed";
                } else {
                    videoPlayer.setPaused(true);
                    videoPaused = true;
                    ofLogNotice() << "VideoManager: Video paused";
                }
            }
            break;
            
        case OF_KEY_LEFT:  // Seek backward
            if (currentVideoSource == VIDEO_FILE && videoLoaded) {
                float currentPos = videoPlayer.getPosition();
                videoPlayer.setPosition(std::max(0.0f, currentPos - 0.05f));
                ofLogNotice() << "VideoManager: Seeked backward";
            }
            break;
            
        case OF_KEY_RIGHT:  // Seek forward
            if (currentVideoSource == VIDEO_FILE && videoLoaded) {
                float currentPos = videoPlayer.getPosition();
                videoPlayer.setPosition(std::min(1.0f, currentPos + 0.05f));
                ofLogNotice() << "VideoManager: Seeked forward";
            }
            break;
            
        case 'l':  // Toggle loop
        case 'L':
            if (currentVideoSource == VIDEO_FILE && videoLoaded) {
                bool loopState = videoPlayer.getLoopState();
                videoPlayer.setLoopState(loopState ? OF_LOOP_NONE : OF_LOOP_NORMAL);
                ofLogNotice() << "VideoManager: Loop " << (loopState ? "disabled" : "enabled");
            }
            break;
    }
}

void VideoManager::handleVideoFileOpen() {
    openVideoFileDialog();
}

void VideoManager::handleVideoSourceSwitch() {
    validateAndFixVideoSource();
    
    switch(currentVideoSource) {
        case CAMERA:
            currentVideoSource = VIDEO_FILE;
            useVideoFile = true;
            ofLogNotice() << "VideoManager: Switched to video file";
            break;
        case VIDEO_FILE:
            currentVideoSource = IP_CAMERA;
            useVideoFile = false;
            ofLogNotice() << "VideoManager: Switched to IP camera";
            break;
        case IP_CAMERA:
            currentVideoSource = CAMERA;
            useVideoFile = false;
            ofLogNotice() << "VideoManager: Switched to camera";
            break;
    }
}

// IP Camera methods - EXACT COPY from working backup
void VideoManager::connectIPCamera() {
    if (ipCameraUrl.empty()) {
        ofLogError() << "VideoManager: IP Camera URL is empty";
        return;
    }
    
    ipCameraSnapshotUrl = ipCameraUrl;
    ipCameraConnected = true;
    ipFrameReady = false;
    currentVideoSource = IP_CAMERA;
    useVideoFile = false;
    
    ofLogNotice() << "VideoManager: IP Camera connected to " << ipCameraUrl;
}

void VideoManager::disconnectIPCamera() {
    ipCameraConnected = false;
    ipFrameReady = false;
    
    // Switch to camera or video file as fallback
    validateAndFixVideoSource();
    
    ofLogNotice() << "VideoManager: IP Camera disconnected";
}

// USB Camera device management methods
vector<ofVideoDevice> VideoManager::getAvailableCameras() {
    return availableCameras;
}

void VideoManager::refreshCameraDevices() {
    availableCameras = camera.listDevices();
    
    ofLogNotice() << "VideoManager: Found " << availableCameras.size() << " camera devices:";
    for (size_t i = 0; i < availableCameras.size(); i++) {
        ofLogNotice() << "  [" << availableCameras[i].id << "] " << availableCameras[i].deviceName;
        if (i == 0 && currentCameraDeviceID == 0) {
            currentCameraName = availableCameras[i].deviceName;
        }
    }
    
    // Update current camera name if device ID is valid
    if (currentCameraDeviceID < availableCameras.size()) {
        currentCameraName = availableCameras[currentCameraDeviceID].deviceName;
    }
}

void VideoManager::setCameraDevice(int deviceID) {
    if (deviceID < 0 || deviceID >= availableCameras.size()) {
        ofLogError() << "VideoManager: Invalid camera device ID: " << deviceID;
        return;
    }
    
    currentCameraDeviceID = deviceID;
    currentCameraName = availableCameras[deviceID].deviceName;
    
    ofLogNotice() << "VideoManager: Switching to camera device [" << deviceID << "] " << currentCameraName;
    
    // Restart camera with new device
    if (cameraConnected && currentVideoSource == CAMERA) {
        handleCameraRestart();
    }
}