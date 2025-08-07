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
	// Skip frames for performance
	frameSkipCounter++;
	if (frameSkipCounter < detectionFrameSkip) {
		return;
	}
	frameSkipCounter = 0;
	
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
	
	// Use CoreML detector - run more frequently for real-time tracking
	static int counter = 0;
	if (counter++ % 5 == 0) { // Every 5 frames for real-time tracking (~12 times per second)
		ofLogNotice() << "Running CoreML object detection...";
		
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
		
		// Draw bounding box with vehicle-specific color
		ofSetColor(boxColor);
		ofSetLineWidth(3);
		ofNoFill();
		ofDrawRectangle(x, y, w, h);
		ofFill();
		
		// Draw confidence bar (small bar next to detection) - scaled
		float confBarWidth = 60 * displayScale;
		float confBarHeight = 8 * displayScale;
		float confBarX = x + w + 5 * displayScale;
		float confBarY = y;
		
		// Background of confidence bar
		ofSetColor(50, 50, 50, 180);
		ofDrawRectangle(confBarX, confBarY, confBarWidth, confBarHeight);
		
		// Filled portion based on confidence
		ofSetColor(boxColor);
		ofDrawRectangle(confBarX, confBarY, confBarWidth * detection.confidence, confBarHeight);
		
		// Draw class label and confidence - scaled
		string label = detection.className + " " + ofToString(detection.confidence, 2);
		float labelWidth = label.length() * 8 * displayScale;
		float labelHeight = 20 * displayScale;
		
		// Ensure label background fits on screen
		float labelX = ofClamp(x, 0, ofGetWidth() - labelWidth);
		float labelY = ofClamp(y - labelHeight, labelHeight, ofGetHeight());
		
		// Draw label background
		ofSetColor(boxColor.r, boxColor.g, boxColor.b, 180);
		ofDrawRectangle(labelX, labelY - labelHeight, labelWidth, labelHeight);
		
		// Draw label text
		ofSetColor(0, 0, 0); // Black text
		ofDrawBitmapString(label, labelX + 2, labelY - 5);
	}
	
	// Reset drawing state
	ofSetColor(255);
	ofSetLineWidth(1);
	ofFill();
}

//--------------------------------------------------------------
