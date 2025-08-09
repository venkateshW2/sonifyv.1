#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);  // Let app run at 60fps for responsiveness
	ofSetBackgroundColor(0, 0, 0);  // Black background instead of green
	
	// Initialize detection class selection system
	maxSelectedClasses = 8;  // Maximum number of classes that can be selected
	initializeCategories();
	
	// Initialize video mode flags
	useVideoFile = false;
	videoLoaded = false;
	videoPaused = false;
	currentVideoPath = "";
	currentVideoSource = CAMERA;  // Default to camera
	
	// Initialize IP camera configuration
	ipCameraUrl = "http://192.168.29.151:8080/video";  // Your IP Webcam URL
	ipCameraConnected = false;
	connectionRetries = 0;
	connectionTimeout = 5000;  // 5 seconds timeout
	maxConnectionRetries = 3;
	isConnecting = false;
	lastConnectionAttempt = 0.0f;
	connectionError = "";
	
	// Initialize line drawing system
	isDrawingLine = false;
	currentColorIndex = 0;
	selectedLineIndex = -1;  // No line selected initially
	
	// Initialize line editing system
	isDraggingEndpoint = false;
	draggingLineIndex = -1;
	isDraggingStartPoint = false;
	
	// Initialize CoreML detection system
	detector = [[CoreMLDetector alloc] init];
	yoloLoaded = false;
	enableDetection = false;
	frameSkipCounter = 0;
	detectionFrameSkip = 3;  // Back to every 3rd frame for better detection rate
	lastDetectionTime = 0.0f;
	detectionErrorCount = 0;
	
	// Initialize UI and scaling
	displayScale = 1.0f;  // Will be calculated based on window size vs 640x640
	
	// Initialize vehicle tracking system
	nextVehicleId = 1;
	vehicleTrackingThreshold = 50.0f;  // Max distance to consider same vehicle
	maxFramesWithoutDetection = 10;   // Remove vehicles after this many missed frames
	
	// Initialize OSC communication
	oscHost = "127.0.0.1";  // localhost
	oscPort = 12000;        // Default port for Max/MSP
	oscEnabled = true;
	oscSender.setup(oscHost, oscPort);
	ofLogNotice() << "OSC sender initialized: " << oscHost << ":" << oscPort;
	
	// Initialize MIDI system
	setupMIDI();
	
	// Load configuration (this will set all GUI parameters)
	loadConfig();
	setupGUI();
	
	// Load CoreML YOLO model
	loadCoreMLModel();
	
	// Try to load test video file first (optional)
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
	
	// Initialize camera at compatible resolution
	camera.setDesiredFrameRate(30);
	// Try different common resolutions for better compatibility
	bool cameraSetup = false;
	
	// Try 640x480 first
	if (camera.setup(640, 480)) {
		cameraSetup = true;
		ofLogNotice() << "Camera set to 640x480";
	} 
	// Fallback to lower resolution if needed
	else if (camera.setup(320, 240)) {
		cameraSetup = true;
		ofLogNotice() << "Camera fallback to 320x240";
	}
	// Last resort - let camera choose
	else if (camera.setup(0, 0)) {
		cameraSetup = true;
		ofLogNotice() << "Camera auto-resolution: " << camera.getWidth() << "x" << camera.getHeight();
	}
	
	// Check if camera connected (keep fixed window size)
	cameraConnected = camera.isInitialized();
	
	if (cameraConnected) {
		ofLogNotice() << "Camera initialized: " << camera.getWidth() << "x" << camera.getHeight();
	}
	
	// Calculate initial display scale (always 1.0 since video area is fixed at 640x640)
	displayScale = 1.0f;  // Video area is always 640x640 regardless of window size
	ofLogNotice() << "Window size: " << ofGetWidth() << "x" << ofGetHeight() 
				  << " (video area: 640x640, display scale: " << displayScale << ")";
	
	if (!cameraConnected && !videoLoaded) {
		ofLogError() << "Neither camera nor test video available!";
	} else if (!cameraConnected) {
		ofLogNotice() << "Camera not available, using video file only";
	}
	
	// Final video source validation - ensure we start with a working source
	validateAndFixVideoSource();
}

//--------------------------------------------------------------
void ofApp::validateAndFixVideoSource() {
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
			currentSourceWorking = (ipCameraConnected && ipCameraPlayer.isLoaded());
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

//--------------------------------------------------------------
void ofApp::update(){
	// Update video sources based on current source
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
			updateIPCameraStatus();
			break;
	}
	
	// Backward compatibility: also update based on useVideoFile flag
	if (useVideoFile && videoLoaded) {
		videoPlayer.update();
	} else if (cameraConnected && currentVideoSource == CAMERA) {
		camera.update();
	}
	
	// Process CoreML YOLO detection
	if (enableDetection && yoloLoaded) {
		processCoreMLDetection();
		
		// Update vehicle tracking and check for line crossings
		updateVehicleTracking();
		
		// Task 2.1: Enhanced tracking features
		for (auto& vehicle : trackedVehicles) {
			if (vehicle.framesSinceLastSeen == 0) { // Only for recently detected vehicles
				updateTrajectoryHistory(vehicle);
				calculateVelocityAndAcceleration(vehicle);
			}
		}
		
		// Handle occluded vehicles and cleanup
		handleOccludedVehicles();
		cleanupOldTrajectoryPoints();
		
		checkLineCrossings();
	}
	
	// Task 2.3: Update MIDI note timing (handle note-offs)
	updateMIDITiming();
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Clear background
	ofSetColor(40, 40, 40);  // Dark gray background
	ofFill();
	ofDrawRectangle(0, 0, 640, 640);
	
	// Draw video source in left 640x640 area only
	ofSetColor(255, 255, 255);  // White color for video
	
	bool videoDrawn = false;
	
	// Primary video source drawing
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
			if (ipCameraConnected && ipCameraPlayer.isLoaded()) {
				ipCameraPlayer.draw(0, 0, 640, 640);
				videoDrawn = true;
			} else if (isConnecting) {
				// Show connecting message
				ofSetColor(255, 255, 255);
				ofDrawBitmapString("Connecting to IP camera...", 20, 320);
			} else if (!connectionError.empty()) {
				// Show error message
				ofSetColor(255, 0, 0);
				ofDrawBitmapString("IP Camera Error: " + connectionError, 20, 320);
			} else {
				// Show IP camera not connected message
				ofSetColor(255, 255, 0);
				ofDrawBitmapString("IP Camera not connected. Press 'v' to switch to camera or video.", 20, 300);
				ofDrawBitmapString("Or use GUI to connect to IP camera.", 20, 320);
			}
			break;
	}
	
	// Fallback: if no video drawn and we have other sources available
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
	
	// If still no video, show helpful message
	if (!videoDrawn) {
		ofSetColor(255, 255, 0);
		ofDrawBitmapString("No video source available:", 20, 300);
		ofDrawBitmapString("Camera: " + string(cameraConnected ? "Connected" : "Not connected"), 20, 320);
		ofDrawBitmapString("Video: " + string(videoLoaded ? "Loaded" : "Not loaded"), 20, 340);
		ofDrawBitmapString("Press 'v' to switch sources or 'o' to load video", 20, 360);
	}
	
	// Draw lines over video (only if enabled)
	if (showLines) {
		drawLines();
	}
	
	// Draw object detections (only if enabled)
	if (enableDetection && yoloLoaded && showDetections) {
		drawDetections();
	}
	
	// Task 2.1: Draw enhanced tracking visualizations
	if (enableDetection && yoloLoaded) {
		drawTrajectoryTrails();
		drawVelocityVectors();
	}
	
	// Draw GUI
	drawGUI();
}

//--------------------------------------------------------------
void ofApp::exit(){
	// Save configuration before exiting
	saveConfig();
	
	// Clean up CoreML detector
	if (detector) {
		detector = nil;
	}
}

//--------------------------------------------------------------
ofApp::~ofApp() {
	// Clean up CoreML detector in destructor as well
	if (detector) {
		detector = nil;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// Check if GUI wants to capture keyboard input (e.g., typing in text fields)
	if (showGUI && ImGui::GetIO().WantCaptureKeyboard) {
		return; // GUI is handling this keyboard event
	}
	
	if (key == 'r' || key == 'R') {
		// Restart camera
		camera.close();
		camera.setDesiredFrameRate(30);
		camera.setup(640, 480);  // VGA resolution for compatibility
		cameraConnected = camera.isInitialized();
		
		// Window size remains fixed at 640x640 regardless of camera restart
		
		if (cameraConnected) {
			ofLogNotice() << "Camera restarted successfully";
		} else {
			ofLogError() << "Camera restart failed";
		}
	}
	
	if (key == 'v' || key == 'V') {
		// Cycle through video sources: Camera -> Video File -> IP Camera -> Camera...
		VideoSource nextSource;
		switch (currentVideoSource) {
			case CAMERA:
				if (videoLoaded) {
					nextSource = VIDEO_FILE;
				} else if (ipCameraConnected) {
					nextSource = IP_CAMERA;
				} else {
					nextSource = CAMERA;  // Stay on camera
				}
				break;
			case VIDEO_FILE:
				if (ipCameraConnected) {
					nextSource = IP_CAMERA;
				} else if (cameraConnected) {
					nextSource = CAMERA;
				} else {
					nextSource = VIDEO_FILE;  // Stay on video
				}
				break;
			case IP_CAMERA:
				if (cameraConnected) {
					nextSource = CAMERA;
				} else if (videoLoaded) {
					nextSource = VIDEO_FILE;
				} else {
					nextSource = IP_CAMERA;  // Stay on IP camera
				}
				break;
		}
		
		currentVideoSource = nextSource;
		
		// Update backward compatibility flag
		useVideoFile = (currentVideoSource == VIDEO_FILE);
		
		string sourceName;
		switch (currentVideoSource) {
			case CAMERA: sourceName = "camera"; break;
			case VIDEO_FILE: sourceName = "video file"; break;
			case IP_CAMERA: sourceName = "IP camera"; break;
		}
		ofLogNotice() << "Switched to " << sourceName;
	}
	
	if (key == 'i' || key == 'I') {
		// Toggle IP camera mode
		if (currentVideoSource == IP_CAMERA) {
			// Switch away from IP camera
			if (cameraConnected) {
				currentVideoSource = CAMERA;
				useVideoFile = false;
				ofLogNotice() << "Switched to camera";
			} else if (videoLoaded) {
				currentVideoSource = VIDEO_FILE;
				useVideoFile = true;
				ofLogNotice() << "Switched to video file";
			}
		} else {
			// Switch to IP camera
			currentVideoSource = IP_CAMERA;
			useVideoFile = false;
			if (!ipCameraConnected && !ipCameraUrl.empty()) {
				// Try to connect automatically
				connectToIPCamera(ipCameraUrl);
			}
			ofLogNotice() << "Switched to IP camera mode";
		}
	}
	
	if (key == 'o' || key == 'O') {
		// Open file dialog to choose video file
		ofFileDialogResult result = ofSystemLoadDialog("Select Video File", false, ofToDataPath("", true));
		if (result.bSuccess) {
			loadVideoFile(result.getPath());
		}
	}
	
	if (key == ' ') {
		// Toggle play/pause for video
		if (useVideoFile && videoLoaded) {
			if (videoPaused) {
				videoPlayer.play();
				videoPaused = false;
				ofLogNotice() << "Video resumed";
			} else {
				videoPlayer.stop();
				videoPaused = true;
				ofLogNotice() << "Video paused";
			}
		}
	}
	
	if (key == OF_KEY_LEFT) {
		// Seek backward
		if (useVideoFile && videoLoaded) {
			float currentPos = videoPlayer.getPosition();
			float newPos = MAX(0.0f, currentPos - 0.1f);  // Go back 10% for more noticeable movement
			
			// Always ensure video is playing when seeking
			if (videoPaused) {
				videoPlayer.play();
			}
			
			videoPlayer.setPosition(newPos);
			
			// Small delay to let seek complete
			ofSleepMillis(50);
			
			// Restore pause state
			if (videoPaused) {
				videoPlayer.stop();
			}
			
			ofLogNotice() << "Seeking backward: " << currentPos << " -> " << newPos;
		}
	}
	
	if (key == OF_KEY_RIGHT) {
		// Seek forward
		if (useVideoFile && videoLoaded) {
			float currentPos = videoPlayer.getPosition();
			float newPos = MIN(1.0f, currentPos + 0.1f);  // Go forward 10% for more noticeable movement
			
			// Always ensure video is playing when seeking
			if (videoPaused) {
				videoPlayer.play();
			}
			
			videoPlayer.setPosition(newPos);
			
			// Small delay to let seek complete
			ofSleepMillis(50);
			
			// Restore pause state
			if (videoPaused) {
				videoPlayer.stop();
			}
			
			ofLogNotice() << "Seeking forward: " << currentPos << " -> " << newPos;
		}
	}
	
	if (key == 'l' || key == 'L') {
		// Toggle loop mode
		if (useVideoFile && videoLoaded) {
			if (videoPlayer.getLoopState() == OF_LOOP_NORMAL) {
				videoPlayer.setLoopState(OF_LOOP_NONE);
				ofLogNotice() << "Loop mode: OFF";
			} else {
				videoPlayer.setLoopState(OF_LOOP_NORMAL);
				ofLogNotice() << "Loop mode: ON";
			}
		}
	}
	
	if (key == 'c' || key == 'C') {
		// Clear all lines
		lines.clear();
		// Task 3.1: Clear lines (colors now stored in MidiLine structure)
		isDrawingLine = false;
		currentColorIndex = 0;
		selectedLineIndex = -1; // Deselect when clearing lines
		isDraggingEndpoint = false; // Stop any dragging
		draggingLineIndex = -1;
		isDraggingStartPoint = false;
		ofLogNotice() << "All lines cleared";
	}
	
	if (key == 't' || key == 'T') {
		// Send test MIDI note
		sendTestMIDINote();
	}
	
	if (key == 'p' || key == 'P') {
		// Debug IP camera connection
		debugIPCameraConnection();
	}
	
	if (key == OF_KEY_ESC) {
		// Deselect current line
		if (selectedLineIndex != -1) {
			ofLogNotice() << "Line " << (selectedLineIndex + 1) << " deselected";
			selectedLineIndex = -1;
		}
	}
	
	if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE) {
		// Delete selected line
		if (selectedLineIndex != -1 && selectedLineIndex < lines.size()) {
			ofLogNotice() << "Deleted line " << (selectedLineIndex + 1);
			lines.erase(lines.begin() + selectedLineIndex);
			// Task 3.1: Color now stored in MidiLine structure, no separate lineColors vector
			selectedLineIndex = -1; // Deselect after deletion
		}
	}
	
	if (key == 'd' || key == 'D') {
		// Toggle YOLO detection
		if (yoloLoaded) {
			enableDetection = !enableDetection;
			string status = enableDetection ? "enabled" : "disabled";
			ofLogNotice() << "YOLO detection " << status;
		} else {
			ofLogWarning() << "YOLO model not loaded - cannot enable detection";
		}
	}
	
	if (key == 'g' || key == 'G') {
		// Toggle GUI visibility
		showGUI = !showGUI;
		string status = showGUI ? "visible" : "hidden";
		ofLogNotice() << "GUI " << status;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	// Check if mouse is over GUI - if so, don't process dragging
	if (showGUI && ImGui::GetIO().WantCaptureMouse) {
		return; // GUI is handling this mouse event
	}
	
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

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	// Check if mouse is over GUI - if so, don't process line drawing
	if (showGUI && ImGui::GetIO().WantCaptureMouse) {
		return; // GUI is handling this mouse event
	}
	
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
		
		// Task 3.4: Updated logging for master scale system
		vector<string> scaleNoteNames = getScaleNoteNames();
		string noteDisplay = (newLine.scaleNoteIndex < scaleNoteNames.size()) ? scaleNoteNames[newLine.scaleNoteIndex] : "?";
		ofLogNotice() << "Finished line " << lines.size() << " from (" << lineStart.x << "," << lineStart.y 
					  << ") to (" << lineEnd.x << "," << lineEnd.y << ") with scale note " << noteDisplay << " octave " << newLine.octave;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	// Check if mouse is over GUI - if so, don't process line drawing
	if (showGUI && ImGui::GetIO().WantCaptureMouse) {
		return; // GUI is handling this mouse event
	}
	
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

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
	// Check if mouse is over GUI - if so, don't process scrolling
	if (showGUI && ImGui::GetIO().WantCaptureMouse) {
		return; // GUI is handling this scroll event
	}
	
	// Future: Could add zoom functionality here
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	// Attempt to enforce fixed window size by resetting it
	if (w != 1050 || h != 640) {
		ofSetWindowShape(1050, 640);
		ofLogNotice() << "Enforcing fixed window size 1050x640";
		return;
	}
	
	// If we get here, window size is correct or couldn't be enforced
	ofLogNotice() << "Window size: " << w << "x" << h;
	
	// Display scale is always 1.0 since video area is fixed at 640x640
	displayScale = 1.0f;
	
	ofLogNotice() << "Video area remains 640x640 (left side), display scale: " << displayScale;
	
	// Clear current detections since coordinate system has changed
	detections.clear();
	
	// Force a detection update on next frame
	frameSkipCounter = 0;
}

//--------------------------------------------------------------
// Task 5.1: Window Resize Management
//--------------------------------------------------------------
void ofApp::showWindowResizeDialog() {
	if (!showResizeWarning) return;
	
	// Create a modal dialog using ImGui
	ImGui::OpenPopup("Window Size Changed");
	
	// Center the popup
	ImVec2 center = ImGui::GetIO().DisplaySize;
	center.x *= 0.5f;
	center.y *= 0.5f;
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	
	if (ImGui::BeginPopupModal("Window Size Changed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Window size changed. Line positions may be incorrect.");
		ImGui::Text("Original size: %dx%d, New size: %dx%d", 
			originalWindowWidth, originalWindowHeight, ofGetWidth(), ofGetHeight());
		ImGui::Separator();
		ImGui::Text("Would you like to rescale lines to fit the new window size?");
		ImGui::Spacing();
		
		if (ImGui::Button("Rescale Lines", ImVec2(120, 0))) {
			// Rescale lines to fit new window size
			rescaleLines(originalWindowWidth, originalWindowHeight, ofGetWidth(), ofGetHeight());
			
			// Update original window size to current
			originalWindowWidth = ofGetWidth();
			originalWindowHeight = ofGetHeight();
			
			showResizeWarning = false;
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Keep Original", ImVec2(120, 0))) {
			// Keep original coordinates
			showResizeWarning = false;
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Reset Lines", ImVec2(120, 0))) {
			// Clear all lines and start over
			lines.clear();
			currentColorIndex = 0;
			selectedLineIndex = -1;
			
			// Update original window size to current
			originalWindowWidth = ofGetWidth();
			originalWindowHeight = ofGetHeight();
			
			showResizeWarning = false;
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
}

void ofApp::rescaleLines(int oldWidth, int oldHeight, int newWidth, int newHeight) {
	ofLogNotice() << "Rescaling lines from " << oldWidth << "x" << oldHeight 
				  << " to " << newWidth << "x" << newHeight;
	
	// Calculate scaling factors
	float scaleX = (float)newWidth / oldWidth;
	float scaleY = (float)newHeight / oldHeight;
	
	// Only rescale the lines if they're outside the 640x640 video area
	// For lines within the video area, keep them fixed
	for (auto& line : lines) {
		// Check if line is outside video area (640x640)
		bool startOutside = line.startPoint.x > 640 || line.startPoint.y > 640;
		bool endOutside = line.endPoint.x > 640 || line.endPoint.y > 640;
		
		// Only rescale points that are outside the video area
		if (startOutside) {
			line.startPoint.x = line.startPoint.x * scaleX;
			line.startPoint.y = line.startPoint.y * scaleY;
		}
		
		if (endOutside) {
			line.endPoint.x = line.endPoint.x * scaleX;
			line.endPoint.y = line.endPoint.y * scaleY;
		}
	}
	
	ofLogNotice() << "Rescaled " << lines.size() << " lines to fit new window size";
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


//--------------------------------------------------------------
void ofApp::loadVideoFile(string path) {
	ofLogNotice() << "Attempting to load video: " << path;
	
	// Stop current video if playing
	if (videoLoaded) {
		videoPlayer.stop();
		videoPlayer.close();
	}
	
	// Try to load new video
	videoPlayer.load(path);
	
	if (videoPlayer.isLoaded()) {
		videoLoaded = true;
		currentVideoPath = path;
		videoPlayer.setLoopState(OF_LOOP_NORMAL);
		videoPlayer.setVolume(0.0f);  // Mute audio
		videoPlayer.play();
		videoPaused = false;
		useVideoFile = true;  // Switch to video mode
		
		string filename = ofFilePath::getFileName(path);
		ofLogNotice() << "Successfully loaded video: " << filename;
		ofLogNotice() << "Video size: " << videoPlayer.getWidth() << "x" << videoPlayer.getHeight();
		ofLogNotice() << "Video duration: " << videoPlayer.getDuration() << " seconds";
		
		// Window size remains fixed at 640x640 for consistent display
		ofLogNotice() << "Video loaded: " << videoPlayer.getWidth() << "x" << videoPlayer.getHeight() 
					  << " (will be scaled to fit 640x640 window)";
	} else {
		videoLoaded = false;
		currentVideoPath = "";
		ofLogError() << "Failed to load video: " << path;
		ofLogError() << "Make sure the file is a valid video format (mp4, mov, avi, etc.)";
		
		// Fall back to camera if available
		if (cameraConnected) {
			useVideoFile = false;
			ofLogNotice() << "Falling back to camera";
		}
	}
}

//--------------------------------------------------------------
bool ofApp::connectToIPCamera(const string& url) {
	if (isConnecting) {
		return false;  // Already attempting connection
	}
	
	ofLogNotice() << "=== IP CAMERA DEBUGGING ===";
	ofLogNotice() << "Attempting to connect to IP camera: " << url;
	
	// Basic URL validation
	if (!isValidIPCameraURL(url)) {
		connectionError = "Invalid URL format";
		ofLogError() << "Invalid IP camera URL: " << url;
		return false;
	}
	
	isConnecting = true;
	connectionRetries = 0;
	lastConnectionAttempt = ofGetElapsedTimef();
	ipCameraUrl = url;
	connectionError = "";
	
	// Stop current IP camera if running
	if (ipCameraConnected) {
		disconnectIPCamera();
	}
	
	// Check if this is an RTSP stream
	bool isRTSP = (url.find("rtsp://") == 0);
	bool isHTTP = (url.find("http://") == 0 || url.find("https://") == 0);
	
	// DEBUG: Test network connectivity first
	ofLogNotice() << "Testing network connectivity...";
	
	// Extract host and port from URL for connectivity test
	string host, path;
	int port = 80;
	
	if (isHTTP) {
		// Parse HTTP URL
		size_t protocol_end = url.find("://");
		if (protocol_end != string::npos) {
			string after_protocol = url.substr(protocol_end + 3);
			size_t slash_pos = after_protocol.find('/');
			string host_port = after_protocol.substr(0, slash_pos);
			path = after_protocol.substr(slash_pos);
			
			size_t colon_pos = host_port.find(':');
			if (colon_pos != string::npos) {
				host = host_port.substr(0, colon_pos);
				port = ofToInt(host_port.substr(colon_pos + 1));
			} else {
				host = host_port;
				port = (url.find("https://") == 0) ? 443 : 80;
			}
		}
	}
	
	ofLogNotice() << "Parsed URL - Host: " << host << ", Port: " << port << ", Path: " << path;
	
	// Test basic connectivity using system ping
	string ping_cmd = "ping -c 1 -W 2000 " + host + " 2>/dev/null";
	int ping_result = system(ping_cmd.c_str());
	ofLogNotice() << "Network ping test result: " << (ping_result == 0 ? "SUCCESS" : "FAILED");
	
	// Test HTTP connectivity using curl
	if (isHTTP) {
		string curl_cmd = "curl -I -s -m 5 " + url + " | head -1";
		FILE* curl_pipe = popen(curl_cmd.c_str(), "r");
		if (curl_pipe) {
			char buffer[256];
			string response;
			while (fgets(buffer, sizeof(buffer), curl_pipe) != NULL) {
				response += buffer;
			}
			pclose(curl_pipe);
			
			ofLogNotice() << "HTTP response: " << response;
			if (!response.empty()) {
				ofLogNotice() << "HTTP connectivity: SUCCESS";
			} else {
				ofLogNotice() << "HTTP connectivity: FAILED";
			}
		}
	}
	
	// DEBUG: Check if URL requires specific headers or user-agent
	ofLogNotice() << "Testing with different user agents...";
	
	if (isRTSP) {
		ofLogNotice() << "Detected RTSP stream, attempting connection...";
		// RTSP streams are supported by AVFoundation on macOS
		// ofVideoPlayer should handle RTSP URLs on macOS through AVFoundation
	} else {
		ofLogNotice() << "Detected HTTP/MJPEG stream, attempting connection...";
	}
	
	// DEBUG: Try alternative URL formats for IP Webcam
	vector<string> test_urls;
	test_urls.push_back(url); // Original
	
	// Try common IP Webcam variations
	if (url.find("/video") != string::npos) {
		test_urls.push_back(url + "?dummy=param.mjpg"); // Force MJPEG extension
		test_urls.push_back(url.substr(0, url.find("/video")) + "/video.mjpg");
		test_urls.push_back(url.substr(0, url.find("/video")) + "/mjpeg");
		test_urls.push_back(url.substr(0, url.find("/video")) + "/mjpeg.cgi");
	}
	
	// Add RTSP endpoints for IP Webcam (works without browser)
	if (url.find("http://") == 0) {
		// Convert HTTP to RTSP for IP Webcam - these work without browser
		test_urls.push_back("rtsp://" + host + ":" + ofToString(port) + "/h264_ulaw.sdp");
		test_urls.push_back("rtsp://" + host + ":" + ofToString(port) + "/h264_pcm.sdp");
		test_urls.push_back("rtsp://" + host + ":" + ofToString(port) + "/h264_opus.sdp");
	}
	
	// Always add HTTP endpoints
	if (url.find("http://") == 0 || url.find("rtsp://") == 0) {
		test_urls.push_back("http://" + host + ":" + ofToString(port) + "/video");
		test_urls.push_back("http://" + host + ":" + ofToString(port) + "/videostream.cgi");
	}
	
	bool success = false;
	string final_url;
	
	// Try each URL variant
	for (const auto& test_url : test_urls) {
		ofLogNotice() << "Testing URL: " << test_url;
		
		// Create temporary player for testing
		ofVideoPlayer temp_player;
		success = temp_player.load(test_url);
		
		if (success && temp_player.isLoaded()) {
			final_url = test_url;
			ofLogNotice() << "SUCCESS: Found working URL: " << final_url;
			break;
		} else {
			ofLogNotice() << "FAILED: URL " << test_url << " did not load";
			
			// Get more detailed error information
			ofLogNotice() << "Player loaded: " << (temp_player.isLoaded() ? "YES" : "NO");
			ofLogNotice() << "Player width: " << temp_player.getWidth();
			ofLogNotice() << "Player height: " << temp_player.getHeight();
		}
		
		// Small delay between attempts
		ofSleepMillis(500);
	}
	
	// If no variant worked, try the original URL with the main player
	if (!success) {
		final_url = url;
		ofLogNotice() << "No URL variants worked, trying original with main player...";
	}
	
	// Try to load the IP camera stream (works for both HTTP/MJPEG and RTSP on macOS)
	ofLogNotice() << "Final attempt with URL: " << final_url;
	success = ipCameraPlayer.load(final_url);
	
	if (success && ipCameraPlayer.isLoaded()) {
		ipCameraConnected = true;
		connectionError = "";
		ofLogNotice() << "Successfully connected to IP camera: " << final_url;
		
		// Get video dimensions
		int width = ipCameraPlayer.getWidth();
		int height = ipCameraPlayer.getHeight();
		ofLogNotice() << "Video dimensions: " << width << "x" << height;
		
		return true;
	} else {
		ipCameraConnected = false;
		
		// Enhanced debugging for MJPEG streams
		ofLogError() << "=== CONNECTION FAILED ===";
		ofLogError() << "Final URL: " << final_url;
		ofLogError() << "Player loaded: " << (ipCameraPlayer.isLoaded() ? "YES" : "NO");
		
		// Try alternative approaches for MJPEG streams
		ofLogNotice() << "Trying alternative MJPEG handling...";
		
		// Method 1: Try snapshot endpoint (single frame)
		string snapshot_url = final_url.substr(0, final_url.find("/video")) + "/shot.jpg";
		ofLogNotice() << "Testing snapshot URL: " << snapshot_url;
		
		ofVideoPlayer snapshot_player;
		bool snapshot_success = snapshot_player.load(snapshot_url);
		if (snapshot_success) {
			ofLogNotice() << "Snapshot URL works! Using snapshot mode";
			ipCameraUrl = snapshot_url;
			success = ipCameraPlayer.load(snapshot_url);
			if (success) {
				ipCameraConnected = true;
				connectionError = "Using snapshot mode (single frame)";
				return true;
			}
		}
		
		// Method 2: Try different IP Webcam endpoints
		vector<string> alternative_endpoints = {
			"/videofeed",
			"/video.mjpg",
			"/mjpeg",
			"/live",
			"/stream"
		};
		
		string base_url = "http://192.168.1.14:8080";
		for (const auto& endpoint : alternative_endpoints) {
			string alt_url = base_url + endpoint;
			ofLogNotice() << "Testing alternative endpoint: " << alt_url;
			
			ofVideoPlayer alt_player;
			if (alt_player.load(alt_url)) {
				ofLogNotice() << "Found working endpoint: " << alt_url;
				ipCameraUrl = alt_url;
				success = ipCameraPlayer.load(alt_url);
				if (success) {
					ipCameraConnected = true;
					connectionError = "";
					return true;
				}
			}
		}
		
		// Method 3: Use ofxHTTP for MJPEG streaming (fallback)
		ofLogError() << "Standard endpoints failed, trying HTTP-based approach";
		connectionError = "MJPEG format not supported by AVFoundation";
		
		// Provide specific guidance
		ofLogError() << "=== SOLUTIONS ===";
		ofLogError() << "1. Try DroidCam app instead (better compatibility)";
		ofLogError() << "2. Use VLC to test: vlc http://192.168.1.14:8080/video";
		ofLogError() << "3. Check IP Webcam settings for MJPEG format";
		ofLogError() << "4. Try different Android IP camera app";
		
		return false;
	}
}

//--------------------------------------------------------------
void ofApp::disconnectIPCamera() {
	if (ipCameraConnected || ipCameraPlayer.isLoaded()) {
		ipCameraPlayer.stop();
		ipCameraPlayer.close();
		ipCameraConnected = false;
		isConnecting = false;
		connectionError = "";
		ofLogNotice() << "Disconnected from IP camera";
	}
}

//--------------------------------------------------------------
void ofApp::updateIPCameraStatus() {
	// Handle connection timeouts and retries
	if (isConnecting) {
		float timeSinceAttempt = ofGetElapsedTimef() - lastConnectionAttempt;
		if (timeSinceAttempt > (connectionTimeout / 1000.0f)) {
			// Connection timeout
			isConnecting = false;
			connectionRetries++;
			
			if (connectionRetries < maxConnectionRetries) {
				// Retry connection
				retryIPCameraConnection();
			} else {
				connectionError = "Connection timeout after " + ofToString(maxConnectionRetries) + " attempts";
				ofLogError() << connectionError;
			}
		}
	}
	
	// Update IP camera player
	if (ipCameraConnected && ipCameraPlayer.isLoaded()) {
		ipCameraPlayer.update();
		
		// Check if stream is still active
		if (!ipCameraPlayer.isPlaying()) {
			// Stream may have disconnected
			ofLogWarning() << "IP camera stream appears to have stopped, attempting reconnection...";
			retryIPCameraConnection();
		}
	}
}

//--------------------------------------------------------------
bool ofApp::isValidIPCameraURL(const string& url) {
	// Basic URL validation - check for http://, https://, or rtsp:// and basic format
	if (url.empty()) return false;
	
	// Check for supported protocols
	bool hasProtocol = (url.find("http://") == 0 || url.find("https://") == 0 || url.find("rtsp://") == 0);
	if (!hasProtocol) return false;
	
	// Allow IP addresses (192.168.x.x) and domain names
	// Let AVFoundation handle the actual connection validation
	return true;
}

//--------------------------------------------------------------
void ofApp::retryIPCameraConnection() {
	if (!isConnecting && connectionRetries < maxConnectionRetries) {
		ofLogNotice() << "Retrying IP camera connection (" << (connectionRetries + 1) << "/" << maxConnectionRetries << ")";
		ofSleepMillis(2000);  // Wait 2 seconds between retries
		connectToIPCamera(ipCameraUrl);
	}
}

//--------------------------------------------------------------
void ofApp::debugIPCameraConnection() {
	ofLogNotice() << "=== IP CAMERA DEBUGGING TOOL ===";
	ofLogNotice() << "Current URL: " << ipCameraUrl;
	
	// Test with curl to get detailed HTTP response
	string curl_cmd = "curl -v -I --max-time 10 " + ipCameraUrl + " 2>&1";
	ofLogNotice() << "Running: " << curl_cmd;
	
	FILE* pipe = popen(curl_cmd.c_str(), "r");
	if (pipe) {
		char buffer[1024];
		string full_response;
		while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
			full_response += buffer;
		}
		pclose(pipe);
		
		ofLogNotice() << "=== CURL DEBUG OUTPUT ===";
		ofLogNotice() << full_response;
		
		// Parse for specific issues
		if (full_response.find("404") != string::npos) {
			ofLogError() << "HTTP 404: URL not found - try different endpoint";
		} else if (full_response.find("401") != string::npos) {
			ofLogError() << "HTTP 401: Authentication required";
		} else if (full_response.find("403") != string::npos) {
			ofLogError() << "HTTP 403: Access forbidden";
		} else if (full_response.find("Connection refused") != string::npos) {
			ofLogError() << "Connection refused - check IP and port";
		} else if (full_response.find("timeout") != string::npos) {
			ofLogError() << "Connection timeout - check network/firewall";
		} else if (full_response.find("Content-Type: image/jpeg") != string::npos) {
			ofLogNotice() << "MJPEG stream detected - should work with AVFoundation";
		} else if (full_response.find("Content-Type: multipart/x-mixed-replace") != string::npos) {
			ofLogNotice() << "MJPEG multipart stream detected - should work with AVFoundation";
		}
	}
	
	// Test with different user agents
	vector<string> user_agents = {
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36",
		"VLC/3.0.18 LibVLC/3.0.18",
		"QuickTime/7.7.9",
		"AVFoundation/1.0"
	};
	
	// Special handling for RTSP streams (no browser needed)
	if (ipCameraUrl.find("rtsp://") == 0) {
		ofLogNotice() << "RTSP stream detected - attempting direct AVFoundation connection";
		ofLogNotice() << "RTSP streams work without browser - AVFoundation handles RTSP natively";
		
		// RTSP doesn't need HTTP testing - skip to direct connection
		ofLogNotice() << "Skipping HTTP tests for RTSP - proceeding with AVFoundation";
	}
	
	for (const auto& ua : user_agents) {
		string ua_cmd = "curl -I -s -A \"" + ua + "\" --max-time 5 " + ipCameraUrl + " | head -1";
		ofLogNotice() << "Testing with User-Agent: " << ua;
		
		FILE* ua_pipe = popen(ua_cmd.c_str(), "r");
		if (ua_pipe) {
			char buffer[256];
			string response;
			while (fgets(buffer, sizeof(buffer), ua_pipe) != NULL) {
				response += buffer;
			}
			pclose(ua_pipe);
			
			ofLogNotice() << "Response: " << response;
		}
	}
	
	// Test common IP Webcam endpoints
	ofLogNotice() << "=== TESTING COMMON IP WEBCAM ENDPOINTS ===";
	vector<string> test_endpoints = {
		"/video",
		"/video.mjpg",
		"/mjpeg",
		"/mjpeg.cgi",
		"/videostream.cgi",
		"/axis-cgi/mjpg/video.cgi",
		"/cgi-bin/mjpg/video.cgi"
	};
	
	string base_url = ipCameraUrl;
	size_t last_slash = base_url.find_last_of('/');
	if (last_slash != string::npos) {
		base_url = base_url.substr(0, last_slash);
	}
	
	for (const auto& endpoint : test_endpoints) {
		string test_url = base_url + endpoint;
		string test_cmd = "curl -I -s --max-time 3 " + test_url + " | head -1";
		
		FILE* test_pipe = popen(test_cmd.c_str(), "r");
		if (test_pipe) {
			char buffer[256];
			string response;
			while (fgets(buffer, sizeof(buffer), test_pipe) != NULL) {
				response += buffer;
			}
			pclose(test_pipe);
			
			if (!response.empty() && response.find("200") != string::npos) {
				ofLogNotice() << "WORKING: " << test_url << " - " << response;
			} else {
				ofLogNotice() << "FAILED: " << test_url;
			}
		}
	}
	
	// Test with VLC (if available)
	ofLogNotice() << "=== TESTING WITH VLC ===";
	string vlc_test = "vlc --intf dummy --run-time=3 " + ipCameraUrl + " vlc://quit 2>&1 | grep -E '(error|failed|unable)' | head -5";
	FILE* vlc_pipe = popen(vlc_test.c_str(), "r");
	if (vlc_pipe) {
		char buffer[1024];
		string vlc_output;
		while (fgets(buffer, sizeof(buffer), vlc_pipe) != NULL) {
			vlc_output += buffer;
		}
		pclose(vlc_pipe);
		
		if (!vlc_output.empty()) {
			ofLogNotice() << "VLC errors: " << vlc_output;
		} else {
			ofLogNotice() << "VLC test completed without obvious errors";
		}
	}
	
	ofLogNotice() << "=== DEBUGGING COMPLETE ===";
}


//--------------------------------------------------------------
void ofApp::drawLines() {
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

//--------------------------------------------------------------
ofColor ofApp::getNextLineColor() {
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

//--------------------------------------------------------------
void ofApp::loadCoreMLModel() {
	ofLogNotice() << "Loading CoreML YOLO model...";
	
	// Initialize CoreML models to nullptr
	
	// Load class names from coco.names - use data path
	string cocoNamesPath = ofToDataPath("models/coco.names", true);
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
	modelPath = ofToDataPath("models/yolov8l.mlpackage", true);
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
		modelPath = ofToDataPath("models/yolov8m.mlpackage", true);
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
		modelPath = ofToDataPath("models/yolov8n.mlpackage", true);
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

//--------------------------------------------------------------
void ofApp::processCoreMLDetection() {
	// Run every frame for maximum responsiveness
	// frameSkipCounter++;
	// if (frameSkipCounter < detectionFrameSkip) {
	//	return;
	// }
	// frameSkipCounter = 0;
	
	// Get current frame (don't clear detections until we get new results)
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
			if (ipCameraConnected && ipCameraPlayer.isLoaded()) {
				pixels = ipCameraPlayer.getPixels();
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
	
	if (pixels.size() == 0) {
		return;
	}
	
	// Use CoreML detector - run every frame for maximum real-time tracking
	static int counter = 0;
	if (counter++ % 1 == 0) { // Every frame for maximum real-time tracking (~60 times per second)
		// Reduce logging frequency to avoid spam
		if (counter % 30 == 0) { // Log every 30 frames (~2 times per second)
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
								  
								  ofLogNotice() << "Found " << detections.size() << " objects";
							  }];
	}
}

//--------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::applyNMS(const vector<Detection>& rawDetections, vector<Detection>& filteredDetections, float nmsThreshold) {
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

//--------------------------------------------------------------
float ofApp::calculateIoU(const ofRectangle& box1, const ofRectangle& box2) {
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

//--------------------------------------------------------------
void ofApp::drawDetections() {
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

//--------------------------------------------------------------
void ofApp::updateVehicleTracking() {
	// Update existing tracked vehicles with new detections
	for (auto& vehicle : trackedVehicles) {
		vehicle.framesSinceLastSeen++;
		vehicle.hasMovement = false;
	}
	
	// Try to match current detections with tracked vehicles
	for (const auto& detection : detections) {
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
			// Update existing vehicle
			TrackedVehicle& vehicle = trackedVehicles[bestMatchIndex];
			vehicle.previousBox = vehicle.currentBox;
			vehicle.centerPrevious = vehicle.centerCurrent;
			vehicle.currentBox = detection.box;
			vehicle.centerCurrent = detectionCenter;
			vehicle.confidence = detection.confidence;
			vehicle.framesSinceLastSeen = 0;
			
			// Calculate movement and speed
			float distance = calculateDistance(vehicle.centerCurrent, vehicle.centerPrevious);
			vehicle.hasMovement = distance > 2.0f;
			vehicle.speed = distance; // pixels per frame
			
			// Rough speed estimation (assumes ~30fps, 1 pixel ≈ 0.5 feet at highway distance)
			// This is very rough - real world calibration would be needed
			vehicle.speedMph = vehicle.speed * 30.0f * 0.5f * 0.681818f; // pixels/frame → mph
		} else {
			// Create new tracked vehicle
			TrackedVehicle newVehicle;
			newVehicle.id = nextVehicleId++;
			newVehicle.currentBox = detection.box;
			newVehicle.previousBox = detection.box;
			newVehicle.centerCurrent = detectionCenter;
			newVehicle.centerPrevious = detectionCenter;
			newVehicle.vehicleType = detection.classId;
			newVehicle.className = detection.className;
			newVehicle.confidence = detection.confidence;
			newVehicle.framesSinceLastSeen = 0;
			newVehicle.hasMovement = false;
			newVehicle.speed = 0.0f;
			
			// Task 2.1: Initialize enhanced tracking features
			newVehicle.velocity = ofPoint(0, 0);
			newVehicle.acceleration = 0.0f;
			newVehicle.trailColor = getTrailColorForVehicle(newVehicle);
			newVehicle.isOccluded = false;
			newVehicle.predictionConfidence = 1.0f;
			newVehicle.maxTrajectoryLength = maxTrajectoryPoints;
			
			// Initialize trajectory with current position
			updateTrajectoryHistory(newVehicle);
			newVehicle.speedMph = 0.0f;
			
			trackedVehicles.push_back(newVehicle);
			
			ofLogNotice() << "New vehicle tracked: ID " << newVehicle.id 
						  << " (" << newVehicle.className << ")";
		}
	}
	
	// Remove vehicles that haven't been seen for too long
	trackedVehicles.erase(
		std::remove_if(trackedVehicles.begin(), trackedVehicles.end(),
			[this](const TrackedVehicle& v) {
				if (v.framesSinceLastSeen > maxFramesWithoutDetection) {
					ofLogNotice() << "Vehicle lost: ID " << v.id;
					return true;
				}
				return false;
			}),
		trackedVehicles.end()
	);
}

//--------------------------------------------------------------
void ofApp::checkLineCrossings() {
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
				LineCrossEvent crossEvent;
				crossEvent.lineId = lineIndex;
				crossEvent.vehicleId = vehicle.id;
				crossEvent.vehicleType = vehicle.vehicleType;
				crossEvent.className = vehicle.className;
				crossEvent.confidence = vehicle.confidence;
				crossEvent.speed = vehicle.speed;
				crossEvent.speedMph = vehicle.speedMph;
				crossEvent.timestamp = ofGetElapsedTimeMillis();
				crossEvent.crossingPoint = intersection;
				crossEvent.processed = false;
				
				crossingEvents.push_back(crossEvent);
				
				// Send OSC message immediately
				sendOSCLineCrossing(crossEvent);
				
				// Task 2.3: Send MIDI note alongside OSC
				sendMIDILineCrossing(lineIndex, vehicle.className, vehicle.confidence, vehicle.speed);
				
				ofLogNotice() << "Line crossing detected: Vehicle " << vehicle.id 
							  << " (" << vehicle.className << ") crossed line " << lineIndex
							  << " with confidence " << vehicle.confidence;
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::sendOSCLineCrossing(const LineCrossEvent& event) {
	if (!oscEnabled) return;
	
	ofxOscMessage msg;
	msg.setAddress("/line_cross");
	msg.addIntArg(event.lineId);           // Line ID (which line was crossed)
	msg.addIntArg(event.vehicleId);        // Vehicle ID  
	msg.addIntArg(event.vehicleType);      // Vehicle type (COCO class ID)
	msg.addStringArg(event.className);     // Vehicle class name
	msg.addFloatArg(event.confidence);     // Confidence (0.0-1.0)
	msg.addIntArg((int)(event.confidence * 127)); // MIDI velocity from confidence (0-127)
	msg.addIntArg(60 + event.lineId);      // MIDI note (C4 + line offset)
	msg.addIntArg(100);                    // Duration in milliseconds
	msg.addFloatArg(event.speed);          // Speed in pixels per frame
	msg.addFloatArg(event.speedMph);       // Estimated speed in MPH
	msg.addIntArg((int)ofClamp(event.speed * 4, 0, 127)); // Speed-based velocity (0-127)
	msg.addInt64Arg(event.timestamp);      // Timestamp
	msg.addFloatArg(event.crossingPoint.x); // Crossing X position
	msg.addFloatArg(event.crossingPoint.y); // Crossing Y position
	
	oscSender.sendMessage(msg, false);
	
	// Also send a simpler message format for basic Max patches
	ofxOscMessage simpleMsg;
	simpleMsg.setAddress("/note");
	simpleMsg.addIntArg(60 + event.lineId);        // MIDI note
	simpleMsg.addIntArg((int)ofClamp(event.speed * 4, 0, 127)); // Velocity (speed-based)
	simpleMsg.addIntArg(100);                      // Duration (100ms default)
	simpleMsg.addIntArg(event.vehicleType);        // Vehicle type (COCO class ID)
	
	oscSender.sendMessage(simpleMsg, false);
	
	ofLogNotice() << "OSC sent: /line_cross line=" << event.lineId 
				  << " vehicle=" << event.className 
				  << " velocity=" << (int)(event.confidence * 127)
				  << " note=" << (60 + event.lineId);
}

//--------------------------------------------------------------
float ofApp::calculateDistance(const ofPoint& p1, const ofPoint& p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

//--------------------------------------------------------------
bool ofApp::lineSegmentIntersection(const ofPoint& line1Start, const ofPoint& line1End, 
									const ofPoint& line2Start, const ofPoint& line2End, 
									ofPoint& intersection) {
	float x1 = line1Start.x, y1 = line1Start.y;
	float x2 = line1End.x, y2 = line1End.y;
	float x3 = line2Start.x, y3 = line2Start.y;
	float x4 = line2End.x, y4 = line2End.y;
	
	float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	
	if (abs(denom) < 0.001f) {
		return false; // Lines are parallel
	}
	
	float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
	float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
	
	if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
		intersection.x = x1 + t * (x2 - x1);
		intersection.y = y1 + t * (y2 - y1);
		return true;
	}
	
	return false;
}

//--------------------------------------------------------------
// Line Selection Helper Functions
//--------------------------------------------------------------
float ofApp::distanceToLineSegment(const ofPoint& point, const ofPoint& lineStart, const ofPoint& lineEnd) {
	// Calculate distance from point to line segment
	float A = point.x - lineStart.x;
	float B = point.y - lineStart.y;
	float C = lineEnd.x - lineStart.x;
	float D = lineEnd.y - lineStart.y;
	
	float dot = A * C + B * D;
	float lenSq = C * C + D * D;
	
	if (lenSq == 0) {
		// Line segment is actually a point
		return calculateDistance(point, lineStart);
	}
	
	float param = dot / lenSq;
	
	ofPoint closest;
	if (param < 0) {
		closest = lineStart;
	} else if (param > 1) {
		closest = lineEnd;
	} else {
		closest.x = lineStart.x + param * C;
		closest.y = lineStart.y + param * D;
	}
	
	return calculateDistance(point, closest);
}

int ofApp::findClosestLine(const ofPoint& clickPoint, float threshold) {
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

bool ofApp::isNearEndpoint(const ofPoint& clickPoint, int lineIndex, bool& isStartPoint, float threshold) {
	if (lineIndex < 0 || lineIndex >= lines.size()) {
		return false;
	}
	
	float distToStart = calculateDistance(clickPoint, lines[lineIndex].startPoint);
	float distToEnd = calculateDistance(clickPoint, lines[lineIndex].endPoint);
	
	if (distToStart <= threshold && distToStart <= distToEnd) {
		isStartPoint = true;
		return true;
	} else if (distToEnd <= threshold) {
		isStartPoint = false;
		return true;
	}
	
	return false;
}

//--------------------------------------------------------------
// Task 2.1: Enhanced Object Tracking Methods
//--------------------------------------------------------------

void ofApp::updateTrajectoryHistory(TrackedVehicle& vehicle) {
	float currentTime = ofGetElapsedTimef();
	
	// Add current position to trajectory
	vehicle.trajectory.push_back(vehicle.centerCurrent);
	vehicle.trajectoryTimes.push_back(currentTime);
	
	// Limit trajectory length to prevent memory bloat
	if (vehicle.trajectory.size() > vehicle.maxTrajectoryLength) {
		vehicle.trajectory.erase(vehicle.trajectory.begin());
		vehicle.trajectoryTimes.erase(vehicle.trajectoryTimes.begin());
	}
}

void ofApp::calculateVelocityAndAcceleration(TrackedVehicle& vehicle) {
	if (vehicle.trajectory.size() < 2) {
		vehicle.velocity = ofPoint(0, 0);
		vehicle.acceleration = 0.0f;
		return;
	}
	
	// Calculate velocity from last two positions
	int lastIndex = vehicle.trajectory.size() - 1;
	int prevIndex = lastIndex - 1;
	
	float deltaTime = vehicle.trajectoryTimes[lastIndex] - vehicle.trajectoryTimes[prevIndex];
	if (deltaTime > 0) {
		ofPoint deltaPos = vehicle.trajectory[lastIndex] - vehicle.trajectory[prevIndex];
		vehicle.velocity = deltaPos / deltaTime;
		
		// Calculate acceleration (change in speed)
		float currentSpeed = vehicle.velocity.length();
		float previousSpeed = vehicle.speed;
		vehicle.acceleration = (currentSpeed - previousSpeed) / deltaTime;
		
		// Update speed
		vehicle.speed = currentSpeed;
		
		// Estimate MPH (rough approximation: pixels per second to MPH)
		// Assume 640px width represents ~100 feet of highway width
		vehicle.speedMph = (currentSpeed * 3600) / (640.0f / 100.0f) * 0.000189394; // Convert to MPH
	}
}

void ofApp::drawTrajectoryTrails() {
	if (!showTrajectoryTrails) return;
	
	float currentTime = ofGetElapsedTimef();
	
	for (const auto& vehicle : trackedVehicles) {
		if (vehicle.trajectory.size() < 2) continue;
		
		ofSetLineWidth(2.0f);
		
		// Draw trajectory with fading effect
		for (int i = 1; i < vehicle.trajectory.size(); i++) {
			float age = currentTime - vehicle.trajectoryTimes[i];
			if (age > trailFadeTime) continue;
			
			// Calculate alpha based on age
			float alpha = 1.0f - (age / trailFadeTime);
			alpha = ofClamp(alpha, 0.0f, 1.0f);
			
			ofColor trailColor = vehicle.trailColor;
			trailColor.a = alpha * 255;
			ofSetColor(trailColor);
			
			// Draw line segment
			ofDrawLine(vehicle.trajectory[i-1], vehicle.trajectory[i]);
		}
	}
	
	ofSetLineWidth(1.0f);
}

void ofApp::drawVelocityVectors() {
	if (!showVelocityVectors) return;
	
	for (const auto& vehicle : trackedVehicles) {
		if (vehicle.velocity.length() < 1.0f) continue; // Skip very slow vehicles
		
		// Draw velocity vector as arrow
		ofPoint start = vehicle.centerCurrent;
		ofPoint end = start + (vehicle.velocity * 0.1f); // Scale for visibility
		
		// Color based on speed (blue=slow, red=fast)
		float speedNorm = ofClamp(vehicle.speed / 50.0f, 0.0f, 1.0f); // Normalize to 0-50 pixels/second
		ofColor velocityColor = ofColor::blue;
		velocityColor.lerp(ofColor::red, speedNorm);
		ofSetColor(velocityColor);
		
		ofSetLineWidth(3.0f);
		ofDrawLine(start, end);
		
		// Draw arrowhead
		ofPoint direction = (end - start).getNormalized();
		ofPoint perpendicular = ofPoint(-direction.y, direction.x);
		ofPoint arrowPoint1 = end - direction * 8 + perpendicular * 4;
		ofPoint arrowPoint2 = end - direction * 8 - perpendicular * 4;
		
		ofDrawLine(end, arrowPoint1);
		ofDrawLine(end, arrowPoint2);
	}
	
	ofSetLineWidth(1.0f);
}

void ofApp::handleOccludedVehicles() {
	if (!enableOcclusionTracking) return;
	
	// Handle vehicles that haven't been detected for a few frames
	for (auto& vehicle : trackedVehicles) {
		if (vehicle.framesSinceLastSeen > 0 && vehicle.framesSinceLastSeen <= 5) {
			// Vehicle is temporarily occluded - predict position
			vehicle.isOccluded = true;
			
			if (vehicle.trajectory.size() >= 2) {
				// Simple linear prediction based on velocity
				ofPoint predictedPosition = vehicle.centerCurrent + vehicle.velocity * (1.0f / 30.0f); // Assume 30fps
				
				// Update predicted position
				vehicle.centerCurrent = predictedPosition;
				vehicle.currentBox.setFromCenter(predictedPosition, vehicle.currentBox.width, vehicle.currentBox.height);
				
				// Add to trajectory with lower confidence
				vehicle.predictionConfidence = 0.5f;
				updateTrajectoryHistory(vehicle);
			}
		} else {
			vehicle.isOccluded = false;
			vehicle.predictionConfidence = 1.0f;
		}
	}
}

void ofApp::sendEnhancedOSCMessage(const TrackedVehicle& vehicle, int lineId) {
	if (!oscEnabled) return;
	
	ofxOscMessage msg;
	msg.setAddress("/vehicle_tracking");
	
	// Enhanced message with trajectory data
	msg.addIntArg(vehicle.id);
	msg.addStringArg(vehicle.className);
	msg.addFloatArg(vehicle.centerCurrent.x / 640.0f); // Normalize to 0-1
	msg.addFloatArg(vehicle.centerCurrent.y / 640.0f);
	msg.addFloatArg(vehicle.velocity.x);
	msg.addFloatArg(vehicle.velocity.y);
	msg.addFloatArg(vehicle.speed);
	msg.addFloatArg(vehicle.speedMph);
	msg.addFloatArg(vehicle.acceleration);
	msg.addIntArg(vehicle.trajectory.size()); // Trajectory length
	msg.addFloatArg(vehicle.confidence);
	msg.addBoolArg(vehicle.isOccluded);
	
	if (lineId >= 0) {
		msg.addIntArg(lineId); // Line crossing information
	}
	
	oscSender.sendMessage(msg);
}

ofColor ofApp::getTrailColorForVehicle(const TrackedVehicle& vehicle) {
	// Assign colors based on vehicle type
	switch (vehicle.vehicleType) {
		case 2: return ofColor::cyan;     // car
		case 3: return ofColor::orange;   // motorcycle  
		case 5: return ofColor::green;    // bus
		case 7: return ofColor::magenta;  // truck
		default: return ofColor::white;
	}
}

void ofApp::cleanupOldTrajectoryPoints() {
	float currentTime = ofGetElapsedTimef();
	
	for (auto& vehicle : trackedVehicles) {
		// Remove trajectory points older than fadeTime
		auto it = vehicle.trajectory.begin();
		auto timeIt = vehicle.trajectoryTimes.begin();
		
		while (it != vehicle.trajectory.end() && timeIt != vehicle.trajectoryTimes.end()) {
			if (currentTime - *timeIt > trailFadeTime) {
				it = vehicle.trajectory.erase(it);
				timeIt = vehicle.trajectoryTimes.erase(timeIt);
			} else {
				break; // Since times are chronological, we can stop here
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::setupGUI() {
	gui.setup();
	
	// Set a more subtle and calmer color scheme
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 2.0f;         // Slightly rounded corners
	style.FrameRounding = 3.0f;          // Rounded buttons and frames
	style.ItemSpacing = ImVec2(8, 6);    // Add more space between items
	style.ScrollbarSize = 14.0f;         // Wider scrollbars
	
	ImVec4* colors = style.Colors;
	
	// More muted, calmer color theme - lighter background
	colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.18f, 0.94f);  // Lighter background
	colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.58f, 0.31f);    // More muted header
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.40f, 0.65f, 0.50f); // Softer highlight
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.35f, 0.58f, 0.40f);    // More muted buttons
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.40f, 0.65f, 0.60f); // Softer highlight
	colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.40f, 0.65f, 0.50f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.45f, 0.70f, 0.90f); // Softer checkmarks
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);   // Match window background
	colors[ImGuiCol_Border] = ImVec4(0.25f, 0.45f, 0.70f, 0.30f);    // Subtle borders
	colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.22f, 0.54f);   // Subtle frame background
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 0.54f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.20f, 0.80f);   // Title background
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.35f, 0.58f, 0.80f); // Active title
}

//--------------------------------------------------------------
void ofApp::drawGUI() {
	if (!showGUI) return;
	
	gui.begin();
	
	// Task 5.1: Show window resize warning dialog if needed
	if (showResizeWarning) {
		showWindowResizeDialog();
	}
	
	// Main tabbed interface window - fixed position and size, non-floating
	// Position exactly at the end of video area (640px) with full height
	ImGui::SetNextWindowPos(ImVec2(640, 0));
	ImGui::SetNextWindowSize(ImVec2(410, 640));
	
	// Remove window borders and make it fixed
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | 
								   ImGuiWindowFlags_NoResize | 
								   ImGuiWindowFlags_NoCollapse | 
								   ImGuiWindowFlags_NoTitleBar;
	
	if (ImGui::Begin("SonifyV1 Controls", nullptr, window_flags)) {
		// Create tab bar
		if (ImGui::BeginTabBar("ControlTabs")) {
			
			// Main Controls Tab
			if (ImGui::BeginTabItem("Main Controls")) {
				drawMainControlsTab();
				ImGui::EndTabItem();
			}
			
			// MIDI Settings Tab  
			if (ImGui::BeginTabItem("MIDI Settings")) {
				drawMIDISettingsTab();
				ImGui::EndTabItem();
			}
			
			// Detection Classes Tab
			if (ImGui::BeginTabItem("Detection Classes")) {
				drawDetectionClassesTab();
				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	
	gui.end();
}
	//--------------------------------------------------------------
void ofApp::drawMainControlsTab() {
	// Detection Controls
	if (ImGui::CollapsingHeader("Detection Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SliderFloat("Confidence Threshold", &confidenceThreshold, 0.1f, 1.0f, "%.2f");
		ImGui::SliderInt("Frame Skip", &frameSkipValue, 1, 10, "%d");
		ImGui::Checkbox("Enable Detection", &enableDetection);
		ImGui::Checkbox("Show Detections", &showDetections);
		
		// Apply changes to detection system
		if (frameSkipValue != detectionFrameSkip) {
			detectionFrameSkip = frameSkipValue;
		}
	}
	
	// Enhanced Tracking Controls
	if (ImGui::CollapsingHeader("Enhanced Tracking")) {
		ImGui::Checkbox("Show Trajectory Trails", &showTrajectoryTrails);
		ImGui::Checkbox("Show Velocity Vectors", &showVelocityVectors);
		ImGui::Checkbox("Enable Occlusion Tracking", &enableOcclusionTracking);
		
		ImGui::SliderFloat("Trail Fade Time", &trailFadeTime, 0.5f, 10.0f, "%.1f sec");
		ImGui::SliderInt("Max Trajectory Points", &maxTrajectoryPoints, 10, 100, "%d");
	}
	
	// OSC Settings Section  
	if (ImGui::CollapsingHeader("OSC Settings")) {
		ImGui::Checkbox("OSC Enabled", &oscEnabled);
		
		// Host input (read-only for now)
		char hostBuffer[256];
		strcpy(hostBuffer, oscHost.c_str());
		if (ImGui::InputText("Host", hostBuffer, sizeof(hostBuffer), ImGuiInputTextFlags_ReadOnly)) {
			oscHost = string(hostBuffer);
		}
		
		ImGui::SliderInt("Port", &oscPort, 8000, 15000, "%d");
		
		// Reconnect button
		if (ImGui::Button("Reconnect OSC")) {
			oscSender.setup(oscHost, oscPort);
			ofLogNotice() << "OSC reconnected to " << oscHost << ":" << oscPort;
		}
	}
	
	// Line Drawing Section
	if (ImGui::CollapsingHeader("Line Drawing")) {
		ImGui::Checkbox("Show Lines", &showLines);
		ImGui::Text("Lines drawn: %d", (int)lines.size());
		
		if (ImGui::Button("Clear All Lines")) {
			lines.clear();
			// Task 3.1: Clear lines (colors now stored in MidiLine structure)
			currentColorIndex = 0;
			selectedLineIndex = -1; // Clear selection
		}
		
		// Instructions
		ImGui::TextWrapped("Left click to start line, right click to finish line");
		ImGui::TextWrapped("Click on line to select, Delete/Backspace to remove selected line");
		ImGui::TextWrapped("Drag line endpoints to move lines");
	}
	
	// IP Camera Controls Section
	if (ImGui::CollapsingHeader("IP Camera")) {
		// URL input field
		static char urlBuffer[512];
		if (strlen(urlBuffer) == 0) {
			strcpy(urlBuffer, ipCameraUrl.c_str());
		}
		
		ImGui::Text("IP Camera URL:");
		if (ImGui::InputText("##ipurl", urlBuffer, sizeof(urlBuffer))) {
			ipCameraUrl = string(urlBuffer);
		}
		
		// Connection status indicator (LED style)
		ImVec4 statusColor;
		string statusText;
		if (ipCameraConnected) {
			statusColor = ImVec4(0, 1, 0, 1);  // Green
			statusText = "CONNECTED";
		} else if (isConnecting) {
			statusColor = ImVec4(1, 1, 0, 1);  // Yellow
			statusText = "CONNECTING...";
		} else if (!connectionError.empty()) {
			statusColor = ImVec4(1, 0, 0, 1);  // Red
			statusText = "ERROR: " + connectionError;
		} else {
			statusColor = ImVec4(0.5, 0.5, 0.5, 1);  // Gray
			statusText = "DISCONNECTED";
		}
		
		ImGui::TextColored(statusColor, statusText.c_str());
		
		// Control buttons
		if (ImGui::Button("Test Connection")) {
			connectToIPCamera(ipCameraUrl);
		}
		ImGui::SameLine();
		if (ImGui::Button("Disconnect") && ipCameraConnected) {
			disconnectIPCamera();
		}
		ImGui::SameLine();
		if (ImGui::Button("Debug Connection")) {
			debugIPCameraConnection();
		}
		
		// Preset buttons for common IP camera apps
		ImGui::Text("HTTP/MJPEG Presets:");
		if (ImGui::Button("Your IP Webcam")) {
			strcpy(urlBuffer, "http://192.168.29.151:8080/video");
			ipCameraUrl = string(urlBuffer);
		}
		ImGui::SameLine();
		if (ImGui::Button("Generic (8080)")) {
			strcpy(urlBuffer, "http://192.168.1.100:8080/video");
			ipCameraUrl = string(urlBuffer);
		}
		
		ImGui::Text("RTSP Presets:");
		if (ImGui::Button("Your IP Webcam RTSP")) {
			strcpy(urlBuffer, "rtsp://192.168.29.151:8080/h264_ulaw.sdp");
			ipCameraUrl = string(urlBuffer);
		}
		ImGui::SameLine();
		if (ImGui::Button("IP Webcam PCM")) {
			strcpy(urlBuffer, "rtsp://192.168.29.151:8080/h264_pcm.sdp");
			ipCameraUrl = string(urlBuffer);
		}
		ImGui::SameLine();
		if (ImGui::Button("Generic RTSP")) {
			strcpy(urlBuffer, "rtsp://192.168.1.100:554/live");
			ipCameraUrl = string(urlBuffer);
		}
		
		// Connection settings
		ImGui::Text("Settings:");
		int timeoutSeconds = connectionTimeout / 1000;
		if (ImGui::SliderInt("Timeout (sec)", &timeoutSeconds, 1, 30)) {
			connectionTimeout = timeoutSeconds * 1000;  // Convert to milliseconds
		}
		ImGui::SliderInt("Max Retries", &maxConnectionRetries, 1, 10);
		
		// Instructions
		ImGui::TextWrapped("Enter IP camera URL and click Test Connection. Use presets for common apps like IP Webcam.");
		
		// RTSP Quick Setup
		ImGui::Separator();
		ImGui::Text("RTSP Quick Setup:");
		static char ipBuffer[32] = "192.168.29.151";
		if (ImGui::InputText("IP Address", ipBuffer, sizeof(ipBuffer))) {
			// Update RTSP URLs dynamically
		}
		if (ImGui::Button("Set RTSP URLs")) {
			string ip = string(ipBuffer);
			string rtsp_ulaw = "rtsp://" + ip + ":8080/h264_ulaw.sdp";
			string rtsp_pcm = "rtsp://" + ip + ":8080/h264_pcm.sdp";
			
			// Show in console for copy/paste
			ofLogNotice() << "RTSP URLs for " << ip << ":";
			ofLogNotice() << "  Primary: " << rtsp_ulaw;
			ofLogNotice() << "  Backup:  " << rtsp_pcm;
			
			// Auto-set the primary RTSP URL
			strcpy(urlBuffer, rtsp_ulaw.c_str());
			ipCameraUrl = rtsp_ulaw;
		}
	}
	
	// Video Controls Section
	if (ImGui::CollapsingHeader("Video Controls")) {
		if (useVideoFile && videoLoaded) {
			ImGui::Text("Video: %s", currentVideoPath.c_str());
			
			// Play/Pause button
			if (ImGui::Button(videoPaused ? "Play" : "Pause")) {
				if (videoPaused) {
					videoPlayer.play();
					videoPaused = false;
				} else {
					videoPlayer.setPaused(true);
					videoPaused = true;
				}
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				videoPlayer.stop();
				videoPaused = true;
			}
			
			// Seek bar
			float position = videoPlayer.getPosition();
			if (ImGui::SliderFloat("Position", &position, 0.0f, 1.0f, "%.2f")) {
				videoPlayer.setPosition(position);
			}
			
			// Loop toggle
			bool loopState = videoPlayer.getLoopState() == OF_LOOP_NORMAL;
			if (ImGui::Checkbox("Loop", &loopState)) {
				videoPlayer.setLoopState(loopState ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}
			
		} else if (cameraConnected) {
			ImGui::Text("Camera connected");
			if (ImGui::Button("Restart Camera")) {
				keyPressed('r');
			}
		} else {
			ImGui::Text("No video source");
		}
		
		if (ImGui::Button("Load Video File")) {
			// Trigger file dialog (existing 'o' key functionality)
			ofLogNotice() << "GUI: Load Video File button pressed";
			keyPressed('o');
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Toggle Source")) {
			// Cycle through video sources
			ofLogNotice() << "GUI: Toggle Source button pressed";
			keyPressed('v');
		}
		
		// Additional direct controls
		ImGui::Text("Direct Controls:");
		if (ImGui::Button("Switch to Camera")) {
			ofLogNotice() << "GUI: Switch to Camera button pressed";
			if (cameraConnected) {
				currentVideoSource = CAMERA;
				useVideoFile = false;
				ofLogNotice() << "Switched to camera";
			} else {
				ofLogNotice() << "Camera not connected";
			}
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Switch to Video") && videoLoaded) {
			ofLogNotice() << "GUI: Switch to Video button pressed";
			currentVideoSource = VIDEO_FILE;
			useVideoFile = true;
			ofLogNotice() << "Switched to video file";
		}
	}
	
	// Status Section
	if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("FPS: %.1f", ofGetFrameRate());
		ImGui::Text("Detections: %d", (int)detections.size());
		ImGui::Text("Tracked Vehicles: %d", (int)trackedVehicles.size());
		ImGui::Text("Crossing Events: %d", (int)crossingEvents.size());
		
		if (enableDetection && yoloLoaded) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Detection: ACTIVE");
		} else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Detection: INACTIVE");
		}
		
		if (oscEnabled) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "OSC: CONNECTED");
		} else {
			ImGui::TextColored(ImVec4(1, 0.5, 0, 1), "OSC: DISABLED");
		}
		
		// Video source status
		string sourceName;
		ImVec4 sourceColor;
		switch (currentVideoSource) {
			case CAMERA:
				sourceName = cameraConnected ? "Camera: CONNECTED" : "Camera: NOT AVAILABLE";
				sourceColor = cameraConnected ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
				break;
			case VIDEO_FILE:
				sourceName = videoLoaded ? "Video: LOADED" : "Video: NOT LOADED";
				sourceColor = videoLoaded ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
				break;
			case IP_CAMERA:
				if (ipCameraConnected) {
					sourceName = "IP Camera: CONNECTED";
					sourceColor = ImVec4(0, 1, 0, 1);
				} else if (isConnecting) {
					sourceName = "IP Camera: CONNECTING...";
					sourceColor = ImVec4(1, 1, 0, 1);
				} else {
					sourceName = "IP Camera: DISCONNECTED";
					sourceColor = ImVec4(1, 0, 0, 1);
				}
				break;
		}
		ImGui::TextColored(sourceColor, sourceName.c_str());
	}
	
	// Configuration Section
	if (ImGui::CollapsingHeader("Configuration")) {
		ImGui::Text("Save/Load Settings");
		
		if (ImGui::Button("Save Config")) {
			saveConfig();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Load Config")) {
			loadConfig();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Reset to Defaults")) {
			setDefaultConfig();
		}
		
		ImGui::Separator();
		
		ImGui::Text("Config Path:");
		ImGui::TextWrapped("%s", getConfigPath().c_str());
		
		ImGui::Text("Auto-save: On application exit");
	}
	
	// Keyboard Shortcuts Section
	if (ImGui::CollapsingHeader("Keyboard Shortcuts")) {
		ImGui::Text("Video Sources:");
		ImGui::Text("'o' - Open video file");
		ImGui::Text("'v' - Cycle video sources (Camera->Video->IP Camera)");
		ImGui::Text("'i' - Toggle IP camera mode");
		ImGui::Text("'r' - Restart camera");
		ImGui::Text("");
		ImGui::Text("Detection & Lines:");
		ImGui::Text("'d' - Toggle detection");
		ImGui::Text("'c' - Clear all lines");
		ImGui::Text("ESC - Deselect all lines");
		ImGui::Text("Delete/Backspace - Delete selected line");
		ImGui::Text("");
		ImGui::Text("Video Playback:");
		ImGui::Text("SPACE - Play/pause video");
		ImGui::Text("LEFT/RIGHT - Seek video");
		ImGui::Text("'l' - Toggle video loop");
		ImGui::Text("");
		ImGui::Text("System:");
		ImGui::Text("'t' - Send test MIDI note");
		ImGui::Text("'p' - Debug IP camera connection");
		ImGui::Text("'g' - Toggle this GUI");
	}
	
	// Live Tracking Data (at bottom to avoid disrupting other controls)
	if (ImGui::CollapsingHeader("Live Tracking Data")) {
		ImGui::Text("Tracking Statistics:");
		ImGui::Text("Active Vehicles: %d", (int)trackedVehicles.size());
		
		int visibleVehicles = 0;
		int occludedVehicles = 0;
		for (const auto& vehicle : trackedVehicles) {
			if (vehicle.isOccluded) occludedVehicles++;
			else visibleVehicles++;
		}
		ImGui::Text("Visible: %d, Occluded: %d", visibleVehicles, occludedVehicles);
		
		// Vehicle details (show first 3 vehicles)
		if (!trackedVehicles.empty()) {
			ImGui::Separator();
			ImGui::Text("Vehicle Details (first 3):");
			for (int i = 0; i < std::min(3, (int)trackedVehicles.size()); i++) {
				const auto& vehicle = trackedVehicles[i];
				ImGui::Text("ID %d: %s (%.1f mph, %.0f px/s)", 
					vehicle.id, vehicle.className.c_str(), 
					vehicle.speedMph, vehicle.speed);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::drawMIDISettingsTab() {
	// Task 3.3: Enhanced Lines List Panel with Musical Properties
	ImGui::Text("Lines List (%d)", (int)lines.size());
	ImGui::Separator();
	
	// List all lines with musical properties display
	for (int i = 0; i < lines.size(); i++) {
		bool isSelected = (i == selectedLineIndex);
		const MidiLine& line = lines[i];
		
		ImGui::PushID(i);
		
		// Task 3.4: Create display text with master scale system info
		vector<string> scaleNoteNames = getScaleNoteNames();
		string noteName = "?";
		if (line.scaleNoteIndex >= 0 && line.scaleNoteIndex < scaleNoteNames.size()) {
			noteName = scaleNoteNames[line.scaleNoteIndex];
		}
		
		string lineLabel = "L" + ofToString(i + 1) + ": " + 
						   noteName + ofToString(line.octave) + 
						   " (" + masterScale + " #" + ofToString(line.scaleNoteIndex + 1) + ")" +
						   " Ch" + ofToString(line.midiChannel);
		
		if (ImGui::Selectable(lineLabel.c_str(), isSelected)) {
			selectedLineIndex = i;
			ofLogNotice() << "Selected line " << (i + 1) << " from Line List panel: " 
						  << noteName << line.octave << " from " << masterScale << " scale";
		}
		
		// Show line color indicator
		ImGui::SameLine();
		ImVec2 colorPos = ImGui::GetCursorScreenPos();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec4 lineColor = ImVec4(line.color.r/255.0f, line.color.g/255.0f, line.color.b/255.0f, 1.0f);
		drawList->AddRectFilled(colorPos, ImVec2(colorPos.x + 12, colorPos.y + 12), ImGui::ColorConvertFloat4ToU32(lineColor));
		
		// Add detailed properties on separate line when selected
		if (isSelected) {
			ImGui::Indent();
			// Calculate MIDI note for display
			int midiNote = getMidiNoteFromMasterScale(i);
			ImGui::TextDisabled("MIDI %d, %s mode, %dms", 
				midiNote,
				line.durationType == MidiLine::DURATION_FIXED ? "Fixed" : 
				line.durationType == MidiLine::SPEED_BASED ? "Speed" : "Vehicle",
				line.fixedDuration);
			ImGui::Unindent();
		}
		
		ImGui::PopID();
	}
		
	
	if (lines.empty()) {
		ImGui::TextDisabled("No lines drawn");
	}
	
	// Task 5.1: Add Rescale Lines button to Lines List panel
	ImGui::Separator();
	if (ImGui::Button("Rescale Lines")) {
		rescaleLines(originalWindowWidth, originalWindowHeight, ofGetWidth(), ofGetHeight());
		originalWindowWidth = ofGetWidth();
		originalWindowHeight = ofGetHeight();
		ofLogNotice() << "Lines rescaled manually";
	}
	
	ImGui::SameLine();
	if (ImGui::Button("Reset Window Size")) {
		originalWindowWidth = ofGetWidth();
		originalWindowHeight = ofGetHeight();
		ofLogNotice() << "Window size reference reset to current: " << originalWindowWidth << "x" << originalWindowHeight;
	}
	
	ImGui::Separator();
	
	// Task 3.4: Master Scale System + Selected Line MIDI Properties Panel
	ImGui::Text("MIDI Properties");
	
	// Master Musical System Controls (always visible)
	if (ImGui::CollapsingHeader("Master Musical System", ImGuiTreeNodeFlags_DefaultOpen)) {
		const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
		const char* scaleNames[] = {"Major", "Minor", "Pentatonic", "Blues", "Chromatic"};
		
		// Root Key selector
		int currentRootIndex = masterRootNote;
		if (ImGui::Combo("Root Key", &currentRootIndex, noteNames, 12)) {
			masterRootNote = currentRootIndex;
			ofLogNotice() << "Master root key changed to: " << getNoteNameFromIndex(masterRootNote);
		}
		
		// Master Scale selector  
		int currentScaleIndex = 0;
		for (int i = 0; i < 5; i++) {
			if (masterScale == scaleNames[i]) {
				currentScaleIndex = i;
				break;
			}
		}
		if (ImGui::Combo("Master Scale", &currentScaleIndex, scaleNames, 5)) {
			masterScale = scaleNames[currentScaleIndex];
			ofLogNotice() << "Master scale changed to: " << masterScale;
		}
		
		// Note: Randomization is now controlled per-line in MIDI Properties section
		
		// Show current scale notes
		vector<string> scaleNoteNames = getScaleNoteNames();
		ImGui::Text("Scale Notes: ");
		ImGui::SameLine();
		for (int i = 0; i < scaleNoteNames.size(); i++) {
			ImGui::Text("%s", scaleNoteNames[i].c_str());
			if (i < scaleNoteNames.size() - 1) {
				ImGui::SameLine();
				ImGui::Text(",");
				ImGui::SameLine();
			}
		}
	}
	
	ImGui::Separator();
	
	// Individual Line Properties (only when line selected)
	if (selectedLineIndex >= 0 && selectedLineIndex < lines.size()) {
		MidiLine& currentLine = lines[selectedLineIndex];
		ImGui::Text("Line %d Settings", selectedLineIndex + 1);
		ImGui::Separator();
		
		// Musical Note Settings - Organized with better spacing
		if (ImGui::CollapsingHeader("🎵 Musical Note", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Spacing();
			
			// Per-line randomization toggle with better layout
			if (ImGui::Checkbox("Randomize Note", &currentLine.randomizeNote)) {
				updateLineMIDIProperties(selectedLineIndex);
				ofLogNotice() << "Line " << (selectedLineIndex + 1) << " randomization " 
							  << (currentLine.randomizeNote ? "enabled" : "disabled");
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("ON: Random notes from master scale per crossing\nOFF: Fixed note selected below");
			}
			
			ImGui::Spacing();
			
			// Visual Note Selection System
			vector<string> scaleNoteNames = getScaleNoteNames();
			
			ImGui::Text("Scale Notes:");
			ImGui::Separator();
			
			// Show scale notes as clickable boxes
			for (int i = 0; i < scaleNoteNames.size(); i++) {
				bool isSelected = false;
				bool isCurrentlyPlaying = false;
				
				if (currentLine.randomizeNote) {
					// In random mode, highlight the note that would be played if triggered now
					// (This is just visual feedback, actual randomization happens at crossing time)
					isCurrentlyPlaying = (i == (ofGetElapsedTimeMillis() / 500) % scaleNoteNames.size());
				} else {
					// In fixed mode, show which note is selected
					isSelected = (i == currentLine.scaleNoteIndex);
				}
				
				// Set colors based on state
				if (isSelected) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));        // Green for selected
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.9f, 0.0f, 1.0f));
				} else if (isCurrentlyPlaying) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.0f, 1.0f));        // Yellow for random highlight
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.9f, 0.0f, 1.0f));
				} else {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));        // Gray for unselected
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				}
				
				// Create button for each note
				string buttonLabel = scaleNoteNames[i];
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(50, 30))) {
					// Only allow selection when not in random mode
					if (!currentLine.randomizeNote) {
						currentLine.scaleNoteIndex = i;
						updateLineMIDIProperties(selectedLineIndex);
						ofLogNotice() << "Line " << (selectedLineIndex + 1) << " note changed to: " << scaleNoteNames[i];
					}
				}
				
				ImGui::PopStyleColor(3); // Pop all 3 color styles
				
				// Show tooltip
				if (ImGui::IsItemHovered()) {
					if (currentLine.randomizeNote) {
						ImGui::SetTooltip("Random mode: Any note from scale can play");
					} else if (isSelected) {
						ImGui::SetTooltip("Selected note: %s", scaleNoteNames[i].c_str());
					} else {
						ImGui::SetTooltip("Click to select: %s", scaleNoteNames[i].c_str());
					}
				}
				
				// Layout notes in rows (4 notes per row for better layout)
				if ((i + 1) % 4 != 0 && i < scaleNoteNames.size() - 1) {
					ImGui::SameLine();
				}
			}
			
			// Status text
			ImGui::Separator();
			if (currentLine.randomizeNote) {
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Mode: Random note from scale");
				ImGui::TextDisabled("(Yellow highlight shows random preview)");
			} else {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mode: Fixed note - %s", 
					(currentLine.scaleNoteIndex < scaleNoteNames.size()) ? scaleNoteNames[currentLine.scaleNoteIndex].c_str() : "Unknown");
				ImGui::TextDisabled("(Green box shows selected note)");
			}
		}
		
		// Octave Selection with better organization
		if (ImGui::CollapsingHeader("🎹 Octave & Preview")) {
			ImGui::Spacing();
			
			int currentOctave = currentLine.octave;
			if (ImGui::SliderInt("Octave", &currentOctave, 0, 10)) {
				currentLine.octave = currentOctave;
				updateLineMIDIProperties(selectedLineIndex);
			}
			
			ImGui::Spacing();
			// Show preview MIDI note with better formatting
			int previewNote = getMidiNoteFromMasterScale(selectedLineIndex);
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Preview MIDI Note: %d", previewNote);
			
			ImGui::Spacing();
		}
		
		// MIDI Settings with better organization  
		if (ImGui::CollapsingHeader("⚙️ MIDI Settings")) {
			ImGui::Spacing();
			// MIDI Channel
			int currentChannel = currentLine.midiChannel;
			if (ImGui::SliderInt("MIDI Channel", &currentChannel, 1, 16)) {
				currentLine.midiChannel = currentChannel;
				updateLineMIDIProperties(selectedLineIndex);
			}
			
			// MIDI Port Selection
			if (!midiPortNames.empty()) {
				int currentPortIndex = -1;
				for (int i = 0; i < midiPortNames.size(); i++) {
					if (midiPortNames[i] == currentLine.midiPortName) {
						currentPortIndex = i;
						break;
					}
				}
				
				// Create port names for combo
				vector<const char*> portNamePtrs;
				for (const auto& name : midiPortNames) {
					portNamePtrs.push_back(name.c_str());
				}
				
				if (ImGui::Combo("MIDI Port", &currentPortIndex, portNamePtrs.data(), (int)portNamePtrs.size())) {
					if (currentPortIndex >= 0 && currentPortIndex < midiPortNames.size()) {
						currentLine.midiPortName = midiPortNames[currentPortIndex];
						updateLineMIDIProperties(selectedLineIndex);
					}
				}
			}
			
			// Duration Settings with better spacing
			ImGui::Text("🕐 Duration:");
			const char* durationTypes[] = {"Fixed", "Speed Based", "Vehicle Based"};
			int currentDurationType = (int)currentLine.durationType;
			if (ImGui::Combo("Duration Mode", &currentDurationType, durationTypes, 3)) {
				currentLine.durationType = (MidiLine::DurationType)currentDurationType;
				updateLineMIDIProperties(selectedLineIndex);
			}
			
			if (currentLine.durationType == MidiLine::DURATION_FIXED) {
				int currentDuration = currentLine.fixedDuration;
				if (ImGui::SliderInt("Duration (ms)", &currentDuration, 50, 2000)) {
					currentLine.fixedDuration = currentDuration;
					updateLineMIDIProperties(selectedLineIndex);
				}
			} else {
				ImGui::TextDisabled("Duration: Variable based on %s", 
					currentLine.durationType == MidiLine::SPEED_BASED ? "vehicle speed" : "vehicle type");
			}
			
			ImGui::Spacing();
			
			// Velocity Settings with better spacing
			ImGui::Text("🔊 Velocity:");
			const char* velocityTypes[] = {"Fixed", "Confidence Based"};
			int currentVelocityType = (int)currentLine.velocityType;
			if (ImGui::Combo("Velocity Mode", &currentVelocityType, velocityTypes, 2)) {
				currentLine.velocityType = (MidiLine::VelocityType)currentVelocityType;
				updateLineMIDIProperties(selectedLineIndex);
			}
			
			if (currentLine.velocityType == MidiLine::VELOCITY_FIXED) {
				int currentVelocity = currentLine.fixedVelocity;
				if (ImGui::SliderInt("Velocity", &currentVelocity, 1, 127)) {
					currentLine.fixedVelocity = currentVelocity;
					updateLineMIDIProperties(selectedLineIndex);
				}
			} else {
				ImGui::TextDisabled("Velocity: Variable based on detection confidence");
			}
			
			ImGui::Spacing();
		}
		
		// Test Button with better styling
		ImGui::Separator();
		ImGui::Spacing();
		
		// Center the test button
		float buttonWidth = 100.0f;
		float availableWidth = ImGui::GetContentRegionAvail().x;
		if (availableWidth > buttonWidth) {
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - buttonWidth) * 0.5f);
		}
		
		if (ImGui::Button("🎵 Test Note", ImVec2(buttonWidth, 35))) {
			// Play a test note from master scale system
			int testNote = getMidiNoteFromMasterScale(selectedLineIndex);
			sendMIDINoteToAllPorts(testNote, currentLine.fixedVelocity, currentLine.midiChannel);
			ofLogNotice() << "Test note: " << testNote << " from " << masterScale << " scale " 
						  << (currentLine.randomizeNote ? "(random)" : "(fixed)");
		}
		
	} else {
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "🎵 No Line Selected");
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextWrapped("Click on a line in the video area or select from the Lines List above to edit MIDI properties.");
		ImGui::Spacing();
		ImGui::TextDisabled("💡 Tip: Draw a line by left-clicking to start, right-clicking to finish.");
	}
	
	ImGui::Separator();
	
	// MIDI System Panel
	ImGui::Text("MIDI System");
	
	// MIDI Port Selection - Multi-port system with checkboxes
	if (ImGui::CollapsingHeader("MIDI Ports", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!midiPortNames.empty()) {
			ImGui::Text("Select ports for MIDI output:");
			ImGui::Separator();
			
			// Show each port with checkbox and connection status
			for (int i = 0; i < midiPortNames.size(); i++) {
				ImGui::PushID(i);
				
				// Port selection checkbox (need temp bool due to vector<bool> specialization)
				bool wasSelected = midiPortSelected[i];
				bool currentSelected = midiPortSelected[i];
				if (ImGui::Checkbox("", &currentSelected)) {
					midiPortSelected[i] = currentSelected;
					// Selection changed
					if (midiPortSelected[i] && !wasSelected) {
						// Port was selected, try to connect
						connectMIDIPort(i);
					} else if (!midiPortSelected[i] && wasSelected) {
						// Port was deselected, disconnect
						disconnectMIDIPort(i);
					}
				}
				
				// Port name and connection status
				ImGui::SameLine();
				ImGui::Text("%s", midiPortNames[i].c_str());
				
				// Connection indicator
				if (midiPortSelected[i]) {
					ImGui::SameLine();
					if (midiPortConnected[i]) {
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "●");
					} else {
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "●");
					}
				}
				
				ImGui::PopID();
			}
			
			ImGui::Separator();
			
			// Summary
			int selectedCount = 0;
			int connectedCount = 0;
			for (int i = 0; i < midiPortSelected.size(); i++) {
				if (midiPortSelected[i]) {
					selectedCount++;
					if (midiPortConnected[i]) {
						connectedCount++;
					}
				}
			}
			
			ImGui::Text("Selected: %d | Connected: %d", selectedCount, connectedCount);
		} else {
			ImGui::Text("No MIDI ports available");
		}
		
		if (ImGui::Button("Refresh Ports")) {
			refreshMIDIPorts();
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Test MIDI")) {
			sendTestMIDINote();
		}
	}
	
	// Global Settings
	if (ImGui::CollapsingHeader("Global Settings")) {
		ImGui::Checkbox("Enable MIDI Output", &midiEnabled);
		
		static bool showMidiActivity = true;
		ImGui::Checkbox("Show MIDI Activity", &showMidiActivity);
		
		// Task 2.3: Enhanced MIDI Activity Monitor
		ImGui::Text("Connection: %s", anyMidiConnected ? "Connected" : "Not Connected");
		ImGui::Text("Active Notes: %d", (int)activeMidiNotes.size());
		ImGui::Text("Total MIDI Events: %d", midiActivityCounter);
		
		// Activity indicator bar
		float activityLevel = (activeMidiNotes.size() > 0) ? 1.0f : 0.0f;
		string activityText = (activeMidiNotes.size() > 0) ? 
			ofToString(activeMidiNotes.size()) + " notes playing" :
			(anyMidiConnected ? "Ready" : "Disconnected");
		ImGui::ProgressBar(activityLevel, ImVec2(-1, 0), activityText.c_str());
	}
}

//--------------------------------------------------------------
string ofApp::getConfigPath() {
	return ofToDataPath("config.json", true);
}

//--------------------------------------------------------------
void ofApp::setDefaultConfig() {
	// Detection settings
	confidenceThreshold = 0.3f;
	frameSkipValue = 3;
	enableDetection = false;
	showDetections = true;
	showLines = true;
	
	// Task 2.1: Enhanced tracking settings
	showTrajectoryTrails = true;
	showVelocityVectors = true;
	trailFadeTime = 2.0f;         // 2 seconds trail fade
	maxTrajectoryPoints = 30;     // 30 point history (~1 second at 30fps)
	enableOcclusionTracking = true;
	
	// OSC settings
	oscHost = "127.0.0.1";
	oscPort = 12000;
	oscEnabled = true;
	
	// GUI settings
	showGUI = true;
	
	// Vehicle tracking settings
	vehicleTrackingThreshold = 50.0f;
	maxFramesWithoutDetection = 10;
	
	ofLogNotice() << "Default configuration applied";
}

//--------------------------------------------------------------
void ofApp::saveConfig() {
	ofxJSONElement config;
	
	// Task 5.1: Save original window size
	config["window"]["originalWidth"] = originalWindowWidth;
	config["window"]["originalHeight"] = originalWindowHeight;
	
	// Detection settings
	config["detection"]["confidenceThreshold"] = confidenceThreshold;
	config["detection"]["frameSkip"] = frameSkipValue;
	config["detection"]["enabled"] = enableDetection;
	config["detection"]["showDetections"] = showDetections;
	config["detection"]["showLines"] = showLines;
	
	// Save detection class selection
	config["detection"]["preset"] = currentPreset;
	config["detection"]["maxSelectedClasses"] = maxSelectedClasses;
	
	// Save selected class IDs
	ofxJSONElement selectedClasses;
	for (int i = 0; i < selectedClassIds.size(); i++) {
		selectedClasses[i] = selectedClassIds[i];
	}
	config["detection"]["selectedClasses"] = selectedClasses;
	
	// Save category enabled flags
	ofxJSONElement categories;
	for (int i = 0; i < CATEGORY_COUNT; i++) {
		// Convert bool to int/bool for JSON compatibility
		bool isEnabled = categoryEnabled[i];
		categories[i] = isEnabled;
	}
	config["detection"]["categories"] = categories;
	
	// Task 2.1: Enhanced tracking settings
	config["tracking"]["showTrajectoryTrails"] = showTrajectoryTrails;
	config["tracking"]["showVelocityVectors"] = showVelocityVectors;
	config["tracking"]["trailFadeTime"] = trailFadeTime;
	config["tracking"]["maxTrajectoryPoints"] = maxTrajectoryPoints;
	config["tracking"]["enableOcclusionTracking"] = enableOcclusionTracking;
	
	// OSC settings
	config["osc"]["host"] = oscHost;
	config["osc"]["port"] = oscPort;
	config["osc"]["enabled"] = oscEnabled;
	
	// GUI settings
	config["gui"]["show"] = showGUI;
	
	// Vehicle tracking settings
	config["tracking"]["threshold"] = vehicleTrackingThreshold;
	config["tracking"]["maxFramesWithoutDetection"] = maxFramesWithoutDetection;
	
	// Video settings
	config["video"]["useVideoFile"] = useVideoFile;
	config["video"]["currentVideoPath"] = currentVideoPath;
	config["video"]["currentVideoSource"] = (int)currentVideoSource;
	
	// IP Camera settings
	config["ipCamera"]["url"] = ipCameraUrl;
	config["ipCamera"]["connectionTimeout"] = connectionTimeout;
	config["ipCamera"]["maxConnectionRetries"] = maxConnectionRetries;
	
	// Save lines
	config["lines"]["count"] = (int)lines.size();
	for (int i = 0; i < lines.size(); i++) {
		config["lines"]["line_" + ofToString(i)]["startX"] = lines[i].startPoint.x;
		config["lines"]["line_" + ofToString(i)]["startY"] = lines[i].startPoint.y;
		config["lines"]["line_" + ofToString(i)]["endX"] = lines[i].endPoint.x;
		config["lines"]["line_" + ofToString(i)]["endY"] = lines[i].endPoint.y;
		
		// Task 3.4: Save MIDI properties (updated for master scale system)
		config["lines"]["line_" + ofToString(i)]["scaleNoteIndex"] = lines[i].scaleNoteIndex;
		config["lines"]["line_" + ofToString(i)]["randomizeNote"] = lines[i].randomizeNote;
		config["lines"]["line_" + ofToString(i)]["octave"] = lines[i].octave;
		config["lines"]["line_" + ofToString(i)]["midiChannel"] = lines[i].midiChannel;
		config["lines"]["line_" + ofToString(i)]["midiPortName"] = lines[i].midiPortName;
		config["lines"]["line_" + ofToString(i)]["durationType"] = (int)lines[i].durationType;
		config["lines"]["line_" + ofToString(i)]["fixedDuration"] = lines[i].fixedDuration;
		config["lines"]["line_" + ofToString(i)]["velocityType"] = (int)lines[i].velocityType;
		config["lines"]["line_" + ofToString(i)]["fixedVelocity"] = lines[i].fixedVelocity;
		// Save line color from MidiLine structure
		config["lines"]["line_" + ofToString(i)]["colorR"] = lines[i].color.r;
		config["lines"]["line_" + ofToString(i)]["colorG"] = lines[i].color.g;
		config["lines"]["line_" + ofToString(i)]["colorB"] = lines[i].color.b;
	}
	
	// Task 3.4: Save master musical system (randomization is now per-line)
	config["masterMusicalSystem"]["rootNote"] = masterRootNote;
	config["masterMusicalSystem"]["scale"] = masterScale;
	
	// Metadata
	config["metadata"]["version"] = "1.0";
	config["metadata"]["timestamp"] = ofGetTimestampString();
	config["metadata"]["application"] = "SonifyV1";
	
	string configPath = getConfigPath();
	bool saved = config.save(configPath, true);
	
	if (saved) {
		ofLogNotice() << "Configuration saved to: " << configPath;
	} else {
		ofLogError() << "Failed to save configuration to: " << configPath;
	}
}

//--------------------------------------------------------------
void ofApp::loadConfig() {
	string configPath = getConfigPath();
	
	if (!ofFile::doesFileExist(configPath)) {
		ofLogNotice() << "Configuration file not found, using defaults: " << configPath;
		setDefaultConfig();
		return;
	}
	
	ofxJSONElement config;
	bool loaded = config.open(configPath);
	
	if (!loaded) {
		ofLogError() << "Failed to load configuration from: " << configPath;
		setDefaultConfig();
		return;
	}
	
	// Task 5.1: Load original window size from config
	if (config["window"].isObject()) {
		originalWindowWidth = config["window"]["originalWidth"].isNumeric() ? 
			config["window"]["originalWidth"].asInt() : ofGetWidth();
		originalWindowHeight = config["window"]["originalHeight"].isNumeric() ? 
			config["window"]["originalHeight"].asInt() : ofGetHeight();
		
		ofLogNotice() << "Loaded original window size: " << originalWindowWidth << "x" << originalWindowHeight;
		
		// Check if current window size differs significantly from original
		if (abs(ofGetWidth() - originalWindowWidth) > 50 || abs(ofGetHeight() - originalWindowHeight) > 50) {
			showResizeWarning = true;
			ofLogNotice() << "Current window size differs from original, will show resize warning";
		}
	} else {
		// If no window size in config, use current as original
		originalWindowWidth = ofGetWidth();
		originalWindowHeight = ofGetHeight();
	}
	
	try {
		// Detection settings
		if (config["detection"].isObject()) {
			confidenceThreshold = config["detection"]["confidenceThreshold"].asFloat();
			frameSkipValue = config["detection"]["frameSkip"].asInt();
			enableDetection = config["detection"]["enabled"].asBool();
			showDetections = config["detection"]["showDetections"].asBool();
			showLines = config["detection"]["showLines"].asBool();
			
			// Apply frame skip setting
			detectionFrameSkip = frameSkipValue;
			
			// Load detection class selection
			if (config["detection"]["preset"].isString()) {
				currentPreset = config["detection"]["preset"].asString();
			}
			
			if (config["detection"]["maxSelectedClasses"].isNumeric()) {
				maxSelectedClasses = config["detection"]["maxSelectedClasses"].asInt();
			}
			
			// Load selected class IDs
			selectedClassIds.clear();
			if (config["detection"]["selectedClasses"].isArray()) {
				for (int i = 0; i < config["detection"]["selectedClasses"].size(); i++) {
					if (config["detection"]["selectedClasses"][i].isNumeric()) {
						selectedClassIds.push_back(config["detection"]["selectedClasses"][i].asInt());
					}
				}
			}
			
			// Load category enabled flags
			if (config["detection"]["categories"].isArray()) {
				int arraySize = (int)config["detection"]["categories"].size();
				int elementsToProcess = (arraySize < CATEGORY_COUNT) ? arraySize : CATEGORY_COUNT;
				
				for (int i = 0; i < elementsToProcess; i++) {
					if (config["detection"]["categories"][i].isBool()) {
						categoryEnabled[i] = config["detection"]["categories"][i].asBool();
					}
				}
			}
			
			// If no classes loaded, apply default preset
			if (selectedClassIds.empty()) {
				currentPreset = "Vehicles Only";
				applyPreset(currentPreset);
			} else {
				// Update enabled classes from loaded selection
				updateEnabledClassesFromSelection();
			}
		}
		
		// Task 2.1: Enhanced tracking settings
		if (config["tracking"].isObject()) {
			showTrajectoryTrails = config["tracking"]["showTrajectoryTrails"].asBool();
			showVelocityVectors = config["tracking"]["showVelocityVectors"].asBool();
			trailFadeTime = config["tracking"]["trailFadeTime"].asFloat();
			maxTrajectoryPoints = config["tracking"]["maxTrajectoryPoints"].asInt();
			enableOcclusionTracking = config["tracking"]["enableOcclusionTracking"].asBool();
		}
		
		// OSC settings
		if (config["osc"].isObject()) {
			oscHost = config["osc"]["host"].asString();
			oscPort = config["osc"]["port"].asInt();
			oscEnabled = config["osc"]["enabled"].asBool();
			
			// Reconnect OSC with loaded settings
			if (oscEnabled) {
				oscSender.setup(oscHost, oscPort);
			}
		}
		
		// GUI settings
		if (config["gui"].isObject()) {
			showGUI = config["gui"]["show"].asBool();
		}
		
		// Vehicle tracking settings
		if (config["tracking"].isObject()) {
			vehicleTrackingThreshold = config["tracking"]["threshold"].asFloat();
			maxFramesWithoutDetection = config["tracking"]["maxFramesWithoutDetection"].asInt();
		}
		
		// Video settings
		if (config["video"].isObject()) {
			useVideoFile = config["video"]["useVideoFile"].asBool();
			currentVideoPath = config["video"]["currentVideoPath"].asString();
			
			// Load current video source (with fallback for older configs)
			if (config["video"]["currentVideoSource"].isNumeric()) {
				int sourceValue = config["video"]["currentVideoSource"].asInt();
				currentVideoSource = static_cast<VideoSource>(sourceValue);
			} else {
				// Fallback for older configs without currentVideoSource
				currentVideoSource = useVideoFile ? VIDEO_FILE : CAMERA;
			}
			
			// Try to load the saved video file
			if (useVideoFile && !currentVideoPath.empty() && ofFile::doesFileExist(currentVideoPath)) {
				loadVideoFile(currentVideoPath);
			}
		}
		
		// Load IP Camera settings
		if (config["ipCamera"].isObject()) {
			ipCameraUrl = config["ipCamera"]["url"].asString();
			connectionTimeout = config["ipCamera"]["connectionTimeout"].asInt();
			maxConnectionRetries = config["ipCamera"]["maxConnectionRetries"].asInt();
			
			// If IP camera was the active source, try to reconnect (but don't block startup)
			if (currentVideoSource == IP_CAMERA && !ipCameraUrl.empty()) {
				ofLogNotice() << "IP camera was active source, but not connected.";
				// Auto-fallback to available video sources instead of staying on IP camera
				if (videoLoaded && videoPlayer.isLoaded()) {
					currentVideoSource = VIDEO_FILE;
					useVideoFile = true;
					ofLogNotice() << "Auto-switched to video file: " << currentVideoPath;
				} else if (cameraConnected && camera.isInitialized()) {
					currentVideoSource = CAMERA;
					useVideoFile = false;
					ofLogNotice() << "Auto-switched to camera";
				} else {
					ofLogNotice() << "No video sources available - staying on IP camera mode";
				}
			}
		}
		
		// Load lines
		if (config["lines"].isObject()) {
			lines.clear();
			// Task 3.1: Clear lines (colors now stored in MidiLine structure)
			
			int lineCount = config["lines"]["count"].asInt();
			for (int i = 0; i < lineCount; i++) {
				string lineKey = "line_" + ofToString(i);
				if (config["lines"][lineKey].isObject()) {
					float startX = config["lines"][lineKey]["startX"].asFloat();
					float startY = config["lines"][lineKey]["startY"].asFloat();
					float endX = config["lines"][lineKey]["endX"].asFloat();
					float endY = config["lines"][lineKey]["endY"].asFloat();
					
					// Task 3.1: Create MidiLine with complete properties
					MidiLine line;
					line.startPoint = ofPoint(startX, startY);
					line.endPoint = ofPoint(endX, endY);
					
					// Load color if available
					if (config["lines"][lineKey]["colorR"].isNumeric()) {
						line.color.r = config["lines"][lineKey]["colorR"].asInt();
						line.color.g = config["lines"][lineKey]["colorG"].asInt();
						line.color.b = config["lines"][lineKey]["colorB"].asInt();
					} else {
						line.color = getNextLineColor();
					}
					
					// Task 3.4: Load MIDI properties with defaults (updated for master scale system)
					line.scaleNoteIndex = config["lines"][lineKey]["scaleNoteIndex"].isNumeric() ? 
						config["lines"][lineKey]["scaleNoteIndex"].asInt() : 0;  // Default first note of scale
					line.randomizeNote = config["lines"][lineKey]["randomizeNote"].isBool() ? 
						config["lines"][lineKey]["randomizeNote"].asBool() : true; // Default to randomization ON
					line.octave = config["lines"][lineKey]["octave"].isNumeric() ? 
						config["lines"][lineKey]["octave"].asInt() : 4;    // Default middle octave
					line.midiChannel = config["lines"][lineKey]["midiChannel"].isNumeric() ? 
						config["lines"][lineKey]["midiChannel"].asInt() : 1;
					
					// Task 5.2: Validate MIDI port name and find closest match if not available
					string savedPortName = config["lines"][lineKey]["midiPortName"].isString() ? 
						config["lines"][lineKey]["midiPortName"].asString() : "";
					
					if (!savedPortName.empty()) {
						// Check if the saved port is still available
						if (!validateMidiPort(savedPortName)) {
							// Port not available, find closest match
							string closestPort = findClosestMidiPort(savedPortName);
							if (!closestPort.empty()) {
								ofLogNotice() << "MIDI port '" << savedPortName << "' not available, using '" << closestPort << "' instead";
								line.midiPortName = closestPort;
							} else {
								ofLogWarning() << "MIDI port '" << savedPortName << "' not available and no alternative found";
								line.midiPortName = "";
							}
						} else {
							line.midiPortName = savedPortName;
						}
					} else {
						line.midiPortName = "";
					}
					line.durationType = config["lines"][lineKey]["durationType"].isNumeric() ? 
						(MidiLine::DurationType)config["lines"][lineKey]["durationType"].asInt() : MidiLine::DURATION_FIXED;
					line.fixedDuration = config["lines"][lineKey]["fixedDuration"].isNumeric() ? 
						config["lines"][lineKey]["fixedDuration"].asInt() : 500;
					line.velocityType = config["lines"][lineKey]["velocityType"].isNumeric() ? 
						(MidiLine::VelocityType)config["lines"][lineKey]["velocityType"].asInt() : MidiLine::VELOCITY_FIXED;
					line.fixedVelocity = config["lines"][lineKey]["fixedVelocity"].isNumeric() ? 
						config["lines"][lineKey]["fixedVelocity"].asInt() : 100;
					
					// Initialize defaults if needed
					initializeNewLineDefaults(line);
					
					lines.push_back(line);
				}
			}
			
			// Update color index
			currentColorIndex = lines.size() % 12;
		}
		
		// Task 3.4: Load master musical system (randomization is now per-line)
		if (config["masterMusicalSystem"].isObject()) {
			masterRootNote = config["masterMusicalSystem"]["rootNote"].isNumeric() ? 
				config["masterMusicalSystem"]["rootNote"].asInt() : 0;  // Default C
			masterScale = config["masterMusicalSystem"]["scale"].isString() ? 
				config["masterMusicalSystem"]["scale"].asString() : "Major";  // Default Major
			// Note: randomizeNotes is now per-line property, not global
			
			ofLogNotice() << "Master musical system loaded: " << getNoteNameFromIndex(masterRootNote) 
						  << " " << masterScale << " scale (randomization is per-line)";
		}
		
		ofLogNotice() << "Configuration loaded successfully from: " << configPath;
		
	} catch (const std::exception& e) {
		ofLogError() << "Error parsing configuration file: " << e.what();
		setDefaultConfig();
	}
}

//--------------------------------------------------------------

//--------------------------------------------------------------
void ofApp::setupMIDI() {
	// Initialize MIDI variables
	midiEnabled = true;
	anyMidiConnected = false;
	
	// Task 2.3: Initialize MIDI timing system
	midiNoteDuration = 500;              // 500ms default note duration
	midiActivityCounter = 0;
	activeMidiNotes.clear();
	
	// Task 3.4: Initialize master musical system
	initializeMasterMusicalSystem();
	
	// Clear any existing data
	midiOuts.clear();
	midiPortNames.clear();
	midiPortSelected.clear();
	midiPortConnected.clear();
	
	// Refresh MIDI ports and initialize vectors
	refreshMIDIPorts();
	
	// Auto-select IAC Driver if available, otherwise first port
	bool iacFound = false;
	for (int i = 0; i < midiPortNames.size(); i++) {
		if (midiPortNames[i].find("IAC") != string::npos) {
			midiPortSelected[i] = true;
			connectMIDIPort(i);
			iacFound = true;
			ofLogNotice() << "Auto-selected IAC Driver: " << midiPortNames[i];
			break;
		}
	}
	
	// If no IAC driver found, select first available port
	if (!iacFound && !midiPortNames.empty()) {
		midiPortSelected[0] = true;
		connectMIDIPort(0);
		ofLogNotice() << "Auto-selected first port: " << midiPortNames[0];
	}
	
	if (midiPortNames.empty()) {
		ofLogWarning() << "No MIDI ports available";
	}
}

//--------------------------------------------------------------
void ofApp::refreshMIDIPorts() {
	// Clear existing data
	midiPortNames.clear();
	midiPortSelected.clear();
	midiPortConnected.clear();
	
	// Disconnect all existing ports
	for (auto& midiOut : midiOuts) {
		if (midiOut.isOpen()) {
			midiOut.closePort();
		}
	}
	midiOuts.clear();
	
	// Create a temporary MIDI out to get port information
	ofxMidiOut tempMidiOut;
	int numPorts = tempMidiOut.getNumOutPorts();
	
	// Initialize vectors for each port
	for (int i = 0; i < numPorts; i++) {
		string portName = tempMidiOut.getOutPortName(i);
		midiPortNames.push_back(portName);
		midiPortSelected.push_back(false);
		midiPortConnected.push_back(false);
		
		// Create a MIDI out object for each port
		midiOuts.push_back(ofxMidiOut());
		
		ofLogNotice() << "MIDI port " << i << ": " << portName;
	}
	
	if (numPorts == 0) {
		ofLogWarning() << "No MIDI output ports found";
	} else {
		ofLogNotice() << "Found " << numPorts << " MIDI output ports";
	}
	
	// Update connection status
	updateMIDIConnectionStatus();
}

//--------------------------------------------------------------
void ofApp::sendTestMIDINote() {
	if (midiEnabled && anyMidiConnected) {
		// Send a test C4 note (note 60) to all selected ports
		sendMIDINoteToAllPorts(60, 100, 1);
		
		// Count how many ports received the note
		int portCount = 0;
		for (int i = 0; i < midiPortSelected.size(); i++) {
			if (midiPortSelected[i] && midiPortConnected[i]) {
				portCount++;
			}
		}
		
		ofLogNotice() << "Test MIDI note sent to " << portCount << " port(s): C4 (60) velocity 100 channel 1";
	} else {
		ofLogWarning() << "Cannot send MIDI: " << 
			(midiEnabled ? "no ports connected" : "MIDI disabled");
	}
}

//--------------------------------------------------------------
void ofApp::sendMIDINote(int note, int velocity, int channel) {
	// Forward to multi-port function for consistency
	sendMIDINoteToAllPorts(note, velocity, channel);
}

//--------------------------------------------------------------
void ofApp::sendMIDINoteOff(int note, int channel) {
	// Forward to multi-port function for consistency
	sendMIDINoteOffToAllPorts(note, channel);
}

//--------------------------------------------------------------
void ofApp::connectMIDIPort(int portIndex) {
	if (portIndex < 0 || portIndex >= midiOuts.size()) {
		return;
	}
	
	if (!midiOuts[portIndex].isOpen()) {
		midiOuts[portIndex].openPort(portIndex);
		midiPortConnected[portIndex] = midiOuts[portIndex].isOpen();
		
		if (midiPortConnected[portIndex]) {
			ofLogNotice() << "Connected to MIDI port " << portIndex << ": " << midiPortNames[portIndex];
		} else {
			ofLogWarning() << "Failed to connect to MIDI port " << portIndex << ": " << midiPortNames[portIndex];
		}
	}
	
	updateMIDIConnectionStatus();
}

//--------------------------------------------------------------
void ofApp::disconnectMIDIPort(int portIndex) {
	if (portIndex < 0 || portIndex >= midiOuts.size()) {
		return;
	}
	
	if (midiOuts[portIndex].isOpen()) {
		midiOuts[portIndex].closePort();
		midiPortConnected[portIndex] = false;
		ofLogNotice() << "Disconnected from MIDI port " << portIndex << ": " << midiPortNames[portIndex];
	}
	
	updateMIDIConnectionStatus();
}

//--------------------------------------------------------------
void ofApp::updateMIDIConnectionStatus() {
	// Check if any MIDI port is connected
	anyMidiConnected = false;
	
	for (int i = 0; i < midiPortConnected.size(); i++) {
		if (midiPortConnected[i]) {
			anyMidiConnected = true;
			break;
		}
	}
	
	// Update connection status for each port
	for (int i = 0; i < midiOuts.size(); i++) {
		midiPortConnected[i] = midiOuts[i].isOpen();
	}
	
	ofLogNotice() << "MIDI connection status updated: " 
				  << (anyMidiConnected ? "Connected" : "Not connected");
}

//--------------------------------------------------------------
void ofApp::sendMIDINoteToAllPorts(int note, int velocity, int channel) {
	if (!midiEnabled || !anyMidiConnected) {
		return;
	}
	
	for (int i = 0; i < midiOuts.size(); i++) {
		if (midiPortSelected[i] && midiPortConnected[i] && midiOuts[i].isOpen()) {
			midiOuts[i].sendNoteOn(channel, note, velocity);
			ofLogVerbose() << "MIDI Note ON sent to port " << i << ": note=" << note << " vel=" << velocity << " ch=" << channel;
		}
	}
}

//--------------------------------------------------------------
void ofApp::sendMIDINoteOffToAllPorts(int note, int channel) {
	if (!midiEnabled || !anyMidiConnected) {
		return;
	}
	
	for (int i = 0; i < midiOuts.size(); i++) {
		if (midiPortSelected[i] && midiPortConnected[i] && midiOuts[i].isOpen()) {
			midiOuts[i].sendNoteOff(channel, note, 0);
			ofLogVerbose() << "MIDI Note OFF sent to port " << i << ": note=" << note << " ch=" << channel;
		}
	}
}

//--------------------------------------------------------------
// This section is removed to fix the redefinition error

//--------------------------------------------------------------
// Task 5.2: MIDI Configuration Helpers
//--------------------------------------------------------------
bool ofApp::validateMidiPort(const string& portName) {
	// Check if the port name exists in the current list of available ports
	for (const string& availablePort : midiPortNames) {
		if (availablePort == portName) {
			return true;
		}
	}
	return false;
}

string ofApp::findClosestMidiPort(const string& originalPort) {
	// If the original port is valid, return it
	if (validateMidiPort(originalPort)) {
		return originalPort;
	}
	
	// If no ports available, return empty string
	if (midiPortNames.empty()) {
		return "";
	}
	
	// Try to find a port with similar name (partial match)
	for (const string& availablePort : midiPortNames) {
		// Check if the available port contains part of the original port name
		// or if the original port contains part of the available port name
		if (availablePort.find(originalPort) != string::npos || 
			originalPort.find(availablePort) != string::npos) {
			return availablePort;
		}
	}
	
	// Try to find a port with similar type (IAC, Network, etc.)
	if (originalPort.find("IAC") != string::npos) {
		// Original was IAC, look for any IAC
		for (const string& availablePort : midiPortNames) {
			if (availablePort.find("IAC") != string::npos) {
				return availablePort;
			}
		}
	} else if (originalPort.find("Network") != string::npos) {
		// Original was Network, look for any Network
		for (const string& availablePort : midiPortNames) {
			if (availablePort.find("Network") != string::npos) {
				return availablePort;
			}
		}
	}
	
	// Fallback to first available port
	return midiPortNames[0];
}

//--------------------------------------------------------------
// Task 2.3: MIDI Line Crossing Integration
//--------------------------------------------------------------

void ofApp::sendMIDILineCrossing(int lineId, const string& vehicleType, float confidence, float speed) {
	if (!midiEnabled || !anyMidiConnected) return;
	
	// Task 3.4: Use master musical scale system for note selection
	int midiNote = getMidiNoteFromMasterScale(lineId);
	
	// Fixed velocity 100 for all notes initially (will be enhanced in later tasks)
	int velocity = 100;
	
	// Send MIDI note-on to all selected ports
	sendMIDINoteToAllPorts(midiNote, velocity, 1);
	
	// Create MIDI note event for timing management
	MidiNoteEvent noteEvent;
	noteEvent.note = midiNote;
	noteEvent.velocity = velocity;
	noteEvent.channel = 1;
	noteEvent.timestamp = ofGetElapsedTimeMillis();
	noteEvent.duration = midiNoteDuration;  // 500ms default duration
	noteEvent.noteOffSent = false;
	
	activeMidiNotes.push_back(noteEvent);
	
	// Increment activity counter for display
	midiActivityCounter++;
	
	// Task 3.4: Enhanced logging with master scale system information
	string noteName = "Unknown";
	bool isRandomized = false;
	if (lineId >= 0 && lineId < lines.size()) {
		// Calculate which note name this MIDI note represents
		int noteIndex = midiNote % 12;
		int octaveNumber = midiNote / 12;
		noteName = getNoteNameFromIndex(noteIndex) + ofToString(octaveNumber);
		isRandomized = lines[lineId].randomizeNote;
	}
	
	ofLogNotice() << "MIDI note sent: Line " << lineId << " Vehicle " << vehicleType 
				  << " Note " << midiNote << " (" << noteName << ") from " << getNoteNameFromIndex(masterRootNote) 
				  << " " << masterScale << " scale" << (isRandomized ? " (random)" : " (fixed)")
				  << " Velocity " << velocity << " Duration " << midiNoteDuration << "ms";
}

void ofApp::updateMIDITiming() {
	if (!midiEnabled || activeMidiNotes.empty()) return;
	
	unsigned long currentTime = ofGetElapsedTimeMillis();
	
	// Process note-offs for expired notes
	for (auto& noteEvent : activeMidiNotes) {
		if (!noteEvent.noteOffSent && (currentTime >= noteEvent.timestamp + noteEvent.duration)) {
			// Send note-off to all selected ports
			sendMIDINoteOffToAllPorts(noteEvent.note, noteEvent.channel);
			noteEvent.noteOffSent = true;
		}
	}
	
	// Remove notes that have been turned off
	activeMidiNotes.erase(
		remove_if(activeMidiNotes.begin(), activeMidiNotes.end(),
			[](const MidiNoteEvent& note) { return note.noteOffSent; }),
		activeMidiNotes.end()
	);
}

void ofApp::processMIDINoteOffs() {
	// This function is called by updateMIDITiming() - kept for future expansion
	// Currently all note-off processing is handled in updateMIDITiming()
}

//--------------------------------------------------------------
// Task 3.1: Musical Properties Helper Methods
//--------------------------------------------------------------

void ofApp::initializeNewLineDefaults(MidiLine& line) {
	// Set first available MIDI port if not already set
	if (line.midiPortName.empty() && !midiPortNames.empty()) {
		// Try to find IAC Driver first, otherwise use first available
		for (const string& portName : midiPortNames) {
			if (portName.find("IAC") != string::npos) {
				line.midiPortName = portName;
				break;
			}
		}
		if (line.midiPortName.empty()) {
			line.midiPortName = midiPortNames[0];
		}
	}
	
	// Task 3.4: Assign different scale note indices for variety
	if (lines.size() > 0) {
		vector<int> scaleIntervals = getScaleIntervals(masterScale);
		if (!scaleIntervals.empty()) {
			int scaleNoteOffset = (lines.size() - 1) % scaleIntervals.size();
			line.scaleNoteIndex = scaleNoteOffset;
		} else {
			line.scaleNoteIndex = 0;
		}
	}
}

string ofApp::getNoteNameFromIndex(int noteIndex) {
	static const vector<string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	if (noteIndex >= 0 && noteIndex < 12) {
		return noteNames[noteIndex];
	}
	return "C";  // Default fallback
}

int ofApp::getNoteIndexFromName(const string& noteName) {
	static const vector<string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	for (int i = 0; i < noteNames.size(); i++) {
		if (noteNames[i] == noteName) {
			return i;
		}
	}
	return 0;  // Default to C
}

vector<string> ofApp::getAvailableScales() {
	return {"Major", "Minor", "Pentatonic", "Blues", "Chromatic"};
}

// Task 3.2: Musical scale system implementation
vector<int> ofApp::getScaleIntervals(const string& scaleName) {
	if (scaleName == "Major") {
		return {0, 2, 4, 5, 7, 9, 11};  // C, D, E, F, G, A, B
	} else if (scaleName == "Minor") {
		return {0, 2, 3, 5, 7, 8, 10};  // C, D, Eb, F, G, Ab, Bb
	} else if (scaleName == "Pentatonic") {
		return {0, 2, 4, 7, 9};  // C, D, E, G, A
	} else if (scaleName == "Blues") {
		return {0, 3, 5, 6, 7, 10};  // C, Eb, F, F#, G, Bb
	} else if (scaleName == "Chromatic") {
		return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};  // All 12 notes
	} else {
		// Default to major scale if unknown scale
		return {0, 2, 4, 5, 7, 9, 11};
	}
}

int ofApp::getMidiNoteFromScale(int lineIndex) {
	if (lineIndex < 0 || lineIndex >= lines.size()) {
		return 60;  // Default C4 if invalid line index
	}
	
	const MidiLine& line = lines[lineIndex];
	
	// Task 3.4: Use master scale system instead of line's individual scale
	vector<int> scaleIntervals = getScaleIntervals(masterScale);
	
	// Pick a random note from the scale
	int randomIndex = ofRandom(scaleIntervals.size());
	int scaleNote = scaleIntervals[randomIndex];
	
	// Task 3.4: Calculate final MIDI note using master scale system
	int finalMidiNote = masterRootNote + scaleNote + (line.octave * 12);
	
	// Ensure MIDI note is in valid range (0-127)
	if (finalMidiNote < 0) finalMidiNote = 0;
	if (finalMidiNote > 127) finalMidiNote = 127;
	
	return finalMidiNote;
}

void ofApp::updateLineMIDIProperties(int lineIndex) {
	if (lineIndex >= 0 && lineIndex < lines.size()) {
		const MidiLine& line = lines[lineIndex];
		
		// Task 3.4: Updated property logging for master scale system
		vector<int> scaleIntervals = getScaleIntervals(masterScale);
		string noteName = "Unknown";
		if (line.scaleNoteIndex >= 0 && line.scaleNoteIndex < scaleIntervals.size()) {
			int noteInScale = (masterRootNote + scaleIntervals[line.scaleNoteIndex]) % 12;
			noteName = getNoteNameFromIndex(noteInScale);
		}
		
		ofLogNotice() << "Updated MIDI properties for line " << (lineIndex + 1) << ": "
					  << noteName << line.octave << " (" << masterScale << " scale note " 
					  << (line.scaleNoteIndex + 1) << "), "
					  << "Channel " << line.midiChannel << ", "
					  << "Duration: " << (line.durationType == MidiLine::DURATION_FIXED ? 
					  	ofToString(line.fixedDuration) + "ms" : "Variable") << ", "
					  << "Velocity: " << (line.velocityType == MidiLine::VELOCITY_FIXED ? 
					  	ofToString(line.fixedVelocity) : "Variable");
	}
}

// Task 3.4: Master Musical System Implementation
void ofApp::initializeMasterMusicalSystem() {
	masterRootNote = 0;          // C
	masterScale = "Major";       // Major scale
	// NOTE: Randomization is now per-line (each line defaults to randomizeNote = true)
	
	ofLogNotice() << "Master Musical System initialized: " << getNoteNameFromIndex(masterRootNote) 
				  << " " << masterScale << " scale (randomization is per-line)";
}

int ofApp::getMidiNoteFromMasterScale(int lineIndex) {
	if (lineIndex < 0 || lineIndex >= lines.size()) {
		return 60;  // Default C4 if invalid line index
	}
	
	const MidiLine& line = lines[lineIndex];
	vector<int> scaleIntervals = getScaleIntervals(masterScale);
	
	if (scaleIntervals.empty()) {
		return 60;  // Fallback to C4
	}
	
	int scaleNoteToUse;
	if (line.randomizeNote) {
		// Pick random note from master scale (per-line randomization)
		scaleNoteToUse = ofRandom(scaleIntervals.size());
	} else {
		// Use line's specific scale note index (user-selected note from master scale)
		scaleNoteToUse = line.scaleNoteIndex % scaleIntervals.size();
	}
	
	// Calculate final MIDI note: root + scale interval + octave offset
	int finalMidiNote = masterRootNote + scaleIntervals[scaleNoteToUse] + (line.octave * 12);
	
	// Ensure MIDI note is in valid range (0-127)
	if (finalMidiNote < 0) finalMidiNote = 0;
	if (finalMidiNote > 127) finalMidiNote = 127;
	
	return finalMidiNote;
}

vector<string> ofApp::getScaleNoteNames() {
	vector<string> noteNames;
	vector<int> scaleIntervals = getScaleIntervals(masterScale);
	const char* baseNotes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	
	for (int interval : scaleIntervals) {
		int noteIndex = (masterRootNote + interval) % 12;
		noteNames.push_back(baseNotes[noteIndex]);
	}
	
	return noteNames;
}
