#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);  // Let app run at 60fps for responsiveness
	ofSetBackgroundColor(0, 255, 0);
	
	// Initialize video mode flags
	useVideoFile = false;
	videoLoaded = false;
	videoPaused = false;
	currentVideoPath = "";
	
	// Initialize line drawing system
	isDrawingLine = false;
	currentColorIndex = 0;
	
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
	showUI = true;        // Start with UI visible
	
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
	
	// Initialize camera at higher resolution for better detection
	camera.setDesiredFrameRate(30);
	camera.setup(1280, 720);  // HD resolution for better detection
	
	// Check if camera connected (keep fixed window size)
	cameraConnected = camera.isInitialized();
	
	if (cameraConnected) {
		ofLogNotice() << "Camera initialized: " << camera.getWidth() << "x" << camera.getHeight();
	}
	
	// Calculate initial display scale (should be 1.0 for 640x640 window)
	displayScale = (float)ofGetWidth() / 640.0f;  // Assumes square window
	ofLogNotice() << "Window size: " << ofGetWidth() << "x" << ofGetHeight() 
				  << " (display scale: " << displayScale << ")";
	
	if (!cameraConnected && !videoLoaded) {
		ofLogError() << "Neither camera nor test video available!";
	} else if (!cameraConnected) {
		ofLogNotice() << "Camera not available, using video file only";
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	if (useVideoFile && videoLoaded) {
		videoPlayer.update();
	} else if (cameraConnected) {
		camera.update();
	}
	
	// Process CoreML YOLO detection
	if (enableDetection && yoloLoaded) {
		processCoreMLDetection();
		
		// Update vehicle tracking and check for line crossings
		updateVehicleTracking();
		checkLineCrossings();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// Draw video source
	if (useVideoFile && videoLoaded) {
		videoPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
	} else if (cameraConnected) {
		camera.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	
	// Draw lines over video
	drawLines();
	
	// Draw object detections
	if (enableDetection && yoloLoaded) {
		drawDetections();
	}
	
	// Only draw UI if enabled (toggle with 'u' key)
	if (showUI) {
		// Draw FPS counter
		ofDrawBitmapStringHighlight("App FPS: " + ofToString(ofGetFrameRate(), 1), 10, 20, ofColor::black, ofColor::white);
		
		// Draw source status
		string source = useVideoFile ? "Source: Video File" : "Source: Camera";
		ofColor sourceColor = (useVideoFile && videoLoaded) || (!useVideoFile && cameraConnected) ? ofColor::green : ofColor::red;
		ofDrawBitmapStringHighlight(source, 10, 40, ofColor::black, sourceColor);
		
		// Show essential controls and YOLO status
		string yoloStatus = yoloLoaded ? (enableDetection ? "YOLO: ON" : "YOLO: OFF") : "YOLO: Failed";
		ofColor yoloColor = yoloLoaded ? (enableDetection ? ofColor::green : ofColor::yellow) : ofColor::red;
		ofDrawBitmapStringHighlight(yoloStatus, 10, 60, ofColor::black, yoloColor);
		
		if (yoloLoaded && enableDetection) {
			ofDrawBitmapStringHighlight("Detections: " + ofToString(detections.size()), 10, 80, ofColor::black, ofColor::white);
		}
		
		// Show current file info if video loaded
		if (useVideoFile && videoLoaded && !currentVideoPath.empty()) {
			string filename = ofFilePath::getFileName(currentVideoPath);
			ofDrawBitmapStringHighlight("Video: " + filename, 10, 100, ofColor::black, ofColor::white);
		}
		
		// Essential controls
		ofDrawBitmapStringHighlight("Controls: 'o' open video | 'd' toggle detect | 'u' toggle UI", 10, ofGetHeight() - 60, ofColor::black, ofColor(50, 50, 50, 200));
		
		// Show line drawing instructions
		ofDrawBitmapStringHighlight("Line Drawing: Left-click start | Right-click end | 'c' clear", 10, ofGetHeight() - 40, ofColor::black, ofColor(50, 50, 50, 200));
		
		// Window info
		ofDrawBitmapStringHighlight("Window: " + ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight()) + " (scale: " + ofToString(displayScale, 1) + ")", 10, ofGetHeight() - 20, ofColor::black, ofColor(50, 50, 50, 200));
		
		// Draw line stats
		if (lines.size() > 0 || isDrawingLine) {
			string lineInfo = "Lines: " + ofToString(lines.size());
			ofDrawBitmapStringHighlight(lineInfo, 10, 120, ofColor::black, ofColor::white);
		}
		
		if (isDrawingLine) {
			ofDrawBitmapStringHighlight("Drawing line " + ofToString(lines.size() + 1) + " - right-click to finish", 10, 140, ofColor::black, ofColor::yellow);
		}
	}
}

//--------------------------------------------------------------
void ofApp::exit(){
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
	if (key == 'r' || key == 'R') {
		// Restart camera
		camera.close();
		camera.setDesiredFrameRate(30);
		camera.setup(1280, 720);  // HD resolution
		cameraConnected = camera.isInitialized();
		
		// Window size remains fixed at 640x640 regardless of camera restart
		
		if (cameraConnected) {
			ofLogNotice() << "Camera restarted successfully";
		} else {
			ofLogError() << "Camera restart failed";
		}
	}
	
	if (key == 'v' || key == 'V') {
		// Toggle between video file and camera
		if (videoLoaded && cameraConnected) {
			useVideoFile = !useVideoFile;
			string newSource = useVideoFile ? "video file" : "camera";
			ofLogNotice() << "Switched to " << newSource;
		} else if (videoLoaded && !useVideoFile) {
			useVideoFile = true;
			ofLogNotice() << "Switched to video file (camera not available)";
		} else if (cameraConnected && useVideoFile) {
			useVideoFile = false;
			ofLogNotice() << "Switched to camera (video not available)";
		} else {
			ofLogWarning() << "Cannot toggle - only one source available";
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
		lineColors.clear();
		isDrawingLine = false;
		currentColorIndex = 0;
		ofLogNotice() << "All lines cleared";
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
	
	if (key == 'u' || key == 'U') {
		// Toggle UI visibility
		showUI = !showUI;
		string status = showUI ? "visible" : "hidden";
		ofLogNotice() << "UI " << status;
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

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (button == 0 && !isDrawingLine) { // Left click - start line
		lineStart = ofPoint(x, y);
		isDrawingLine = true;
		ofLogNotice() << "Started line at: (" << x << ", " << y << ")";
	} else if (button == 2 && isDrawingLine) { // Right click - finish line
		ofPoint lineEnd = ofPoint(x, y);
		lines.push_back(std::make_pair(lineStart, lineEnd));
		lineColors.push_back(getNextLineColor());
		isDrawingLine = false;
		ofLogNotice() << "Finished line " << lines.size() << " from (" << lineStart.x << "," << lineStart.y 
					  << ") to (" << lineEnd.x << "," << lineEnd.y << ")";
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	// Handle window resize events
	ofLogNotice() << "Window resized to: " << w << "x" << h;
	
	// Enforce square aspect ratio to maintain proper coordinate mapping
	int squareSize = std::min(w, h);
	if (w != squareSize || h != squareSize) {
		ofSetWindowShape(squareSize, squareSize);
		ofLogNotice() << "Corrected to square: " << squareSize << "x" << squareSize;
		w = h = squareSize;
	}
	
	// Calculate display scale factor (square window vs detection resolution 640x640)
	displayScale = (float)squareSize / 640.0f;
	
	ofLogNotice() << "Display scale factor: " << displayScale;
	
	// Clear current detections since coordinate system has changed
	detections.clear();
	
	// Force a detection update on next frame
	frameSkipCounter = 0;
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
void ofApp::drawLines() {
	// Draw completed lines with different colors and endpoint squares
	for (int i = 0; i < lines.size(); i++) {
		ofSetColor(lineColors[i], 255);
		ofSetLineWidth(4);
		ofDrawLine(lines[i].first, lines[i].second);
		
		// Draw endpoint squares
		ofSetColor(lineColors[i], 255);
		ofDrawRectangle(lines[i].first.x - 4, lines[i].first.y - 4, 8, 8);  // Start point square
		ofDrawRectangle(lines[i].second.x - 4, lines[i].second.y - 4, 8, 8); // End point square
		
		// Draw line number at midpoint
		ofPoint midpoint = (lines[i].first + lines[i].second) * 0.5f;
		ofSetColor(255, 255, 255);
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
	if (useVideoFile && videoLoaded) {
		pixels = videoPlayer.getPixels();
	} else if (cameraConnected) {
		pixels = camera.getPixels();
	} else {
		return;
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
		
		// Choose color based on vehicle type
		ofColor boxColor;
		switch (detection.classId) {
			case 2: // car
				boxColor = ofColor(0, 255, 0, 220); // Green
				break;
			case 3: // motorcycle  
				boxColor = ofColor(255, 255, 0, 220); // Yellow
				break;
			case 5: // bus
				boxColor = ofColor(255, 0, 0, 220); // Red
				break;
			case 7: // truck
				boxColor = ofColor(0, 0, 255, 220); // Blue
				break;
			default:
				boxColor = ofColor(255, 255, 255, 220); // White
				break;
		}
		
		// Draw elegant bounding box with rounded corners effect
		ofSetColor(boxColor);
		ofSetLineWidth(2);  // Thinner, more elegant line
		ofNoFill();
		
		// Draw main rectangle with subtle style
		ofDrawRectangle(x, y, w, h);
		
		// Add corner accents for a more professional look
		ofSetLineWidth(3);
		float cornerSize = 12 * displayScale;
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
		
		// Draw sleeker confidence indicator inside the box
		float confBarWidth = w * 0.8f;  // 80% of box width
		float confBarHeight = 4 * displayScale;  // Thinner bar
		float confBarX = x + (w - confBarWidth) / 2;  // Center horizontally
		float confBarY = y + h - confBarHeight - 4 * displayScale;  // Bottom of box with padding
		
		// Background of confidence bar with subtle transparency
		ofSetColor(0, 0, 0, 120);
		ofDrawRectangle(confBarX, confBarY, confBarWidth, confBarHeight);
		
		// Filled portion based on confidence with glow effect
		ofSetColor(boxColor.r, boxColor.g, boxColor.b, 200);
		ofDrawRectangle(confBarX, confBarY, confBarWidth * detection.confidence, confBarHeight);
		
		// Draw compact class label with modern styling
		string label = detection.className + " " + ofToString(detection.confidence, 2);
		float labelWidth = label.length() * 7 * displayScale;  // Slightly smaller font spacing
		float labelHeight = 14 * displayScale;  // More compact height
		
		// Position label at top-left of box with small padding
		float labelX = x + 2 * displayScale;
		float labelY = y;
		
		// Ensure label fits on screen
		labelX = ofClamp(labelX, 0, ofGetWidth() - labelWidth);
		labelY = ofClamp(labelY, labelHeight, ofGetHeight());
		
		// Draw modern label background with rounded appearance
		ofSetColor(boxColor.r, boxColor.g, boxColor.b, 200);
		ofDrawRectangle(labelX - 2, labelY - labelHeight + 2, labelWidth + 4, labelHeight);
		
		// Add subtle border for definition
		ofNoFill();
		ofSetLineWidth(1);
		ofSetColor(boxColor.r, boxColor.g, boxColor.b, 150);
		ofDrawRectangle(labelX - 2, labelY - labelHeight + 2, labelWidth + 4, labelHeight);
		ofFill();
		
		// Draw label text with better contrast
		ofSetColor(255, 255, 255); // White text for better readability
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
					line.first, line.second,
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
