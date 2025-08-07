#pragma once

#include "ofMain.h"
#include "CoreMLDetector.h"
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
		
		// Polygon drawing system
		vector<vector<ofPoint>> polygons;
		vector<ofPoint> currentPolygon;
		vector<ofColor> polygonColors;
		bool isDrawingPolygon;
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
		
		void loadVideoFile(string path);
		void drawPolygons();
		ofColor getNextPolygonColor();
		void loadCoreMLModel();
		void processCoreMLDetection();
		void drawDetections();
		void applyNMS(const vector<Detection>& rawDetections, vector<Detection>& filteredDetections, float nmsThreshold);
		float calculateIoU(const ofRectangle& box1, const ofRectangle& box2);
};
