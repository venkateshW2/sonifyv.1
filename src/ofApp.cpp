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
	
	// Initialize polygon drawing system
	isDrawingPolygon = false;
	currentColorIndex = 0;
	
	// Initialize CoreML detection system
	detector = [[CoreMLDetector alloc] init];
	yoloLoaded = false;
	enableDetection = false;
	frameSkipCounter = 0;
	detectionFrameSkip = 3;  // Back to every 3rd frame for better detection rate
	lastDetectionTime = 0.0f;
	detectionErrorCount = 0;
	
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
		ofLogNotice() << "Window size: 640x640 (fixed for consistent detection display)";
	}
	
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
	
	// Draw polygons over video
	drawPolygons();
	
	// Draw object detections
	if (enableDetection && yoloLoaded) {
		drawDetections();
	}
	
	// Draw FPS counter
	ofDrawBitmapStringHighlight("App FPS: " + ofToString(ofGetFrameRate(), 1), 10, 20, ofColor::black, ofColor::white);
	
	// Draw source status
	string source = useVideoFile ? "Source: Video File" : "Source: Camera";
	ofColor sourceColor = (useVideoFile && videoLoaded) || (!useVideoFile && cameraConnected) ? ofColor::green : ofColor::red;
	ofDrawBitmapStringHighlight(source, 10, 40, ofColor::black, sourceColor);
	
	// Draw camera status
	string cameraStatus = cameraConnected ? "Camera: Available" : "Camera: Not Available";
	ofColor cameraColor = cameraConnected ? ofColor::green : ofColor::yellow;
	ofDrawBitmapStringHighlight(cameraStatus, 10, 60, ofColor::black, cameraColor);
	
	// Draw video status
	string videoStatus = videoLoaded ? "Video: Available" : "Video: Not Available";
	ofColor videoColor = videoLoaded ? ofColor::green : ofColor::yellow;
	ofDrawBitmapStringHighlight(videoStatus, 10, 80, ofColor::black, videoColor);
	
	// Draw current source info
	if (useVideoFile && videoLoaded) {
		ofDrawBitmapStringHighlight("Video Size: " + ofToString(videoPlayer.getWidth()) + "x" + ofToString(videoPlayer.getHeight()), 10, 100, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Video Position: " + ofToString(videoPlayer.getPosition(), 2), 10, 120, ofColor::black, ofColor::white);
	} else if (cameraConnected) {
		ofDrawBitmapStringHighlight("Camera Size: " + ofToString(camera.getWidth()) + "x" + ofToString(camera.getHeight()), 10, 100, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Frame New: " + string(camera.isFrameNew() ? "YES" : "NO"), 10, 120, ofColor::black, ofColor::white);
	}
	
	// Draw current file info
	if (useVideoFile && videoLoaded && !currentVideoPath.empty()) {
		string filename = ofFilePath::getFileName(currentVideoPath);
		ofDrawBitmapStringHighlight("Current Video: " + filename, 10, 140, ofColor::black, ofColor::white);
		string playStatus = videoPaused ? "PAUSED" : "PLAYING";
		ofDrawBitmapStringHighlight("Status: " + playStatus, 10, 160, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Position: " + ofToString(videoPlayer.getPosition(), 2) + " / 1.0", 10, 180, ofColor::black, ofColor::white);
	}
	
	// Draw instructions
	ofDrawBitmapStringHighlight("Press 'o' to open video file", 10, 200, ofColor::black, ofColor::white);
	ofDrawBitmapStringHighlight("Press 'v' to toggle video/camera", 10, 220, ofColor::black, ofColor::white);
	ofDrawBitmapStringHighlight("Press 'r' to restart camera", 10, 240, ofColor::black, ofColor::white);
	if (useVideoFile && videoLoaded) {
		ofDrawBitmapStringHighlight("Press SPACE to play/pause video", 10, 260, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Press LEFT/RIGHT to seek video", 10, 280, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Press 'l' to toggle loop mode", 10, 300, ofColor::black, ofColor::white);
	}
	
	// Draw YOLO detection info
	string yoloStatus = yoloLoaded ? (enableDetection ? "YOLO: ON" : "YOLO: Loaded (OFF)") : "YOLO: Failed to load";
	ofColor yoloColor = yoloLoaded ? (enableDetection ? ofColor::green : ofColor::yellow) : ofColor::red;
	ofDrawBitmapStringHighlight(yoloStatus, 10, 400, ofColor::black, yoloColor);
	
	if (yoloLoaded) {
		ofDrawBitmapStringHighlight("Detections: " + ofToString(detections.size()), 10, 420, ofColor::black, ofColor::white);
		ofDrawBitmapStringHighlight("Frame skip: " + ofToString(frameSkipCounter) + "/" + ofToString(detectionFrameSkip), 10, 440, ofColor::black, ofColor::white);
		
		// Detection rate info already shown in status
	}
	
	// Draw polygon controls and info
	ofDrawBitmapStringHighlight("Left-click: Add point | Right-click: Finish polygon", 10, 320, ofColor::black, ofColor::white);
	ofDrawBitmapStringHighlight("Press 'c' to clear all polygons", 10, 340, ofColor::black, ofColor::white);
	ofDrawBitmapStringHighlight("Press 'd' to toggle YOLO detection", 10, 440, ofColor::black, ofColor::white);
	
	// Draw polygon stats
	string polygonInfo = "Polygons: " + ofToString(polygons.size()) + " | Current points: " + ofToString(currentPolygon.size());
	ofDrawBitmapStringHighlight(polygonInfo, 10, 360, ofColor::black, ofColor::white);
	
	if (isDrawingPolygon && currentPolygon.size() > 0) {
		ofDrawBitmapStringHighlight("Drawing polygon " + ofToString(polygons.size() + 1), 10, 380, ofColor::black, ofColor::yellow);
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
		// Clear all polygons
		polygons.clear();
		polygonColors.clear();
		currentPolygon.clear();
		isDrawingPolygon = false;
		currentColorIndex = 0;
		ofLogNotice() << "All polygons cleared";
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
	if (button == 0) { // Left click
		// Add point to current polygon
		currentPolygon.push_back(ofPoint(x, y));
		isDrawingPolygon = true;
		ofLogNotice() << "Added point: (" << x << ", " << y << ") to polygon " << (polygons.size() + 1);
	} else if (button == 2) { // Right click
		// Finish current polygon and start new one
		if (currentPolygon.size() >= 3) {
			polygons.push_back(currentPolygon);
			polygonColors.push_back(getNextPolygonColor());
			ofLogNotice() << "Finished polygon " << polygons.size() << " with " << currentPolygon.size() << " points";
			
			currentPolygon.clear();
			isDrawingPolygon = false;
		} else if (currentPolygon.size() > 0) {
			ofLogWarning() << "Polygon needs at least 3 points to finish";
		}
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
	
	// Clear current detections since coordinate system has changed
	detections.clear();
	
	// Optionally scale existing polygons to new window size
	// For now, we'll leave them as-is since user can redraw if needed
	// In future versions, we could implement polygon coordinate scaling
	
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
void ofApp::drawPolygons() {
	// Draw completed polygons
	for (int i = 0; i < polygons.size(); i++) {
		if (polygons[i].size() >= 3) {
			// Set polygon color with transparency
			ofSetColor(polygonColors[i], 100);
			
			// Draw filled polygon
			ofBeginShape();
			for (auto& point : polygons[i]) {
				ofVertex(point.x, point.y);
			}
			ofEndShape(true);
			
			// Draw polygon outline
			ofSetColor(polygonColors[i], 255);
			ofSetLineWidth(2);
			ofNoFill();
			ofBeginShape();
			for (auto& point : polygons[i]) {
				ofVertex(point.x, point.y);
			}
			ofEndShape(true);
			ofFill();
			
			// Draw polygon number
			if (polygons[i].size() > 0) {
				ofPoint center = polygons[i][0];
				for (int j = 1; j < polygons[i].size(); j++) {
					center += polygons[i][j];
				}
				center /= polygons[i].size();
				ofSetColor(255, 255, 255);
				ofDrawBitmapString("Z" + ofToString(i + 1), center.x - 10, center.y + 4);
			}
		}
	}
	
	// Draw current polygon being drawn
	if (currentPolygon.size() > 0) {
		ofColor currentColor = getNextPolygonColor();
		
		// Draw points of current polygon
		ofSetColor(currentColor, 200);
		ofSetLineWidth(3);
		if (currentPolygon.size() > 1) {
			for (int i = 1; i < currentPolygon.size(); i++) {
				ofDrawLine(currentPolygon[i-1], currentPolygon[i]);
			}
		}
		
		// Draw all points as circles
		for (auto& point : currentPolygon) {
			ofDrawCircle(point, 4);
		}
		
		// Draw line from last point to mouse if drawing
		if (isDrawingPolygon) {
			ofSetColor(currentColor, 150);
			ofSetLineWidth(1);
			ofDrawLine(currentPolygon.back(), ofPoint(ofGetMouseX(), ofGetMouseY()));
		}
		
		// Highlight first point if we have enough points to close
		if (currentPolygon.size() >= 3) {
			ofSetColor(255, 255, 0);
			ofDrawCircle(currentPolygon[0], 6);
		}
	}
	
	// Reset drawing state
	ofSetColor(255);
	ofSetLineWidth(1);
	ofFill();
}

//--------------------------------------------------------------
ofColor ofApp::getNextPolygonColor() {
	vector<ofColor> colors = {
		ofColor::red,
		ofColor::blue,
		ofColor::green,
		ofColor::orange,
		ofColor::purple,
		ofColor::cyan,
		ofColor::yellow,
		ofColor::magenta
	};
	
	ofColor color = colors[currentColorIndex % colors.size()];
	if (currentPolygon.size() == 0) { // Only increment when starting new polygon
		currentColorIndex++;
	}
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
		
		// Clamp coordinates to screen bounds
		float x = ofClamp(detection.box.x, 0, ofGetWidth() - detection.box.width);
		float y = ofClamp(detection.box.y, 0, ofGetHeight() - detection.box.height);
		float w = ofClamp(detection.box.width, 1, ofGetWidth() - x);
		float h = ofClamp(detection.box.height, 1, ofGetHeight() - y);
		
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
		
		// Draw confidence bar (small bar next to detection)
		float confBarWidth = 60;
		float confBarHeight = 8;
		float confBarX = x + w + 5;
		float confBarY = y;
		
		// Background of confidence bar
		ofSetColor(50, 50, 50, 180);
		ofDrawRectangle(confBarX, confBarY, confBarWidth, confBarHeight);
		
		// Filled portion based on confidence
		ofSetColor(boxColor);
		ofDrawRectangle(confBarX, confBarY, confBarWidth * detection.confidence, confBarHeight);
		
		// Draw class label and confidence
		string label = detection.className + " " + ofToString(detection.confidence, 2);
		float labelWidth = label.length() * 8;
		float labelHeight = 20;
		
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
