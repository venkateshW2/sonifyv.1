#pragma once

#include "ofMain.h"
#include "CoreMLDetector.h"
#include "ofxOsc.h"
#include <algorithm>

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;
		~ofApp();

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;

	private:
		ofVideoGrabber camera;
		ofVideoPlayer videoPlayer;
		bool cameraConnected;
		bool useVideoFile;
		bool videoLoaded;
		bool videoPaused;
		string currentVideoPath;
		
		// Line drawing system only
		vector<pair<ofPoint, ofPoint>> lines;  // Store lines as start/end point pairs
		vector<ofColor> lineColors;
		ofPoint lineStart;
		bool isDrawingLine;
		
		int currentColorIndex;
		
		// CoreML YOLO Detection system
		struct Detection {
			ofRectangle box;
			float confidence;
			int classId;
			string className;
		};
		
		vector<Detection> detections;
		vector<string> classNames;
		CoreMLDetector* detector;
		
		bool yoloLoaded;
		bool enableDetection;
		int frameSkipCounter;
		int detectionFrameSkip;
		float lastDetectionTime;
		int detectionErrorCount;
		
		// Window scaling for resizable display
		float displayScale;     // Scale factor for bounding boxes (currentWindow / 640x640)
		bool showUI;           // Toggle for UI visibility
		
		// Vehicle tracking and line crossing system
		struct TrackedVehicle {
			int id;
			ofRectangle currentBox;
			ofRectangle previousBox;
			ofPoint centerCurrent;
			ofPoint centerPrevious;
			int vehicleType;
			string className;
			float confidence;
			int framesSinceLastSeen;
			bool hasMovement;
			float speed;           // Pixels per frame
			float speedMph;        // Estimated MPH (rough approximation)
		};
		
		struct LineCrossEvent {
			int lineId;
			int vehicleId;
			int vehicleType;
			string className;
			float confidence;
			float speed;        // pixels per frame
			float speedMph;     // estimated MPH
			unsigned long timestamp;
			ofPoint crossingPoint;
			bool processed;
		};
		
		vector<TrackedVehicle> trackedVehicles;
		vector<LineCrossEvent> crossingEvents;
		int nextVehicleId;
		float vehicleTrackingThreshold;
		int maxFramesWithoutDetection;
		
		// OSC communication
		ofxOscSender oscSender;
		string oscHost;
		int oscPort;
		bool oscEnabled;
		
		void loadVideoFile(string path);
		void drawLines();
		ofColor getNextLineColor();
		void loadCoreMLModel();
		void processCoreMLDetection();
		void drawDetections();
		void applyNMS(const vector<Detection>& rawDetections, vector<Detection>& filteredDetections, float nmsThreshold);
		float calculateIoU(const ofRectangle& box1, const ofRectangle& box2);
		
		// Vehicle tracking and line crossing
		void updateVehicleTracking();
		void checkLineCrossings();
		void sendOSCLineCrossing(const LineCrossEvent& event);
		float calculateDistance(const ofPoint& p1, const ofPoint& p2);
		bool lineSegmentIntersection(const ofPoint& line1Start, const ofPoint& line1End, 
									const ofPoint& line2Start, const ofPoint& line2End, 
									ofPoint& intersection);
};
