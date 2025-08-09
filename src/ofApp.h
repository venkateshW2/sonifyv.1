#pragma once

#include "ofMain.h"
#include "CoreMLDetector.h"
#include "ofxOsc.h"
#include "ofxImGui.h"
#include "ofxJSON.h"
#include "ofxMidi.h"
#include <algorithm>
#include <memory>

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
		// Class detection category system
		vector<bool> enabledClasses;       // Track enabled classes (80 elements for all YOLO classes)
		vector<int> selectedClassIds;      // Currently selected class IDs (max 8)
		string currentPreset;              // Current preset name ("Vehicles", "People & Animals", etc.)
		int maxSelectedClasses;            // Maximum number of classes that can be selected (8)

		// Category definitions
		enum DetectionCategory {
			CATEGORY_VEHICLES,
			CATEGORY_PEOPLE,
			CATEGORY_ANIMALS,
			CATEGORY_OBJECTS,
			CATEGORY_COUNT
		};
		vector<bool> categoryEnabled;      // Which main categories are enabled
		
		// Category helper methods
		void initializeCategories();
		void applyPreset(const string& presetName);
		string getClassNameById(int classId);
		DetectionCategory getCategoryForClass(int classId);
		vector<int> getClassesInCategory(DetectionCategory category);
		void updateEnabledClassesFromSelection();
		
		// Video Source Management
		enum VideoSource { CAMERA, VIDEO_FILE, IP_CAMERA };
		VideoSource currentVideoSource;
		
		ofVideoGrabber camera;
		ofVideoPlayer videoPlayer;
		ofVideoPlayer ipCameraPlayer;
		bool cameraConnected;
		bool useVideoFile;  // Kept for backward compatibility
		bool videoLoaded;
		bool videoPaused;
		string currentVideoPath;
		
		// IP Camera configuration - SIMPLE SNAPSHOT APPROACH
		string ipCameraUrl;
		string ipCameraSnapshotUrl;  // For /photo.jpg endpoint
		bool ipCameraConnected;
		ofImage currentIPFrame;       // Current frame from IP camera
		bool ipFrameReady;           // New frame available?
		float lastFrameRequest;      // When did we last request a frame?
		float frameRequestInterval;  // How often to request frames
		int ipFrameSkip;             // Skip frames for performance
		int ipFrameCounter;          // Counter for frame skipping
		
		// IP Camera debugging
		void debugIPCameraConnection();
		
		// Task 3.1: Enhanced Line system with musical properties
		struct MidiLine {
			// Visual properties
			ofPoint startPoint;
			ofPoint endPoint;
			ofColor color;
			
			// Task 3.4: Redesigned MIDI properties (uses master scale system)
			int scaleNoteIndex;     // Index into master scale (0 = first note of scale, 1 = second, etc.)
			bool randomizeNote;     // Per-line randomization toggle (true = random, false = use scaleNoteIndex)
			int octave;             // 0-10 (default 4 for middle octave)
			int midiChannel;        // 1-16 (default 1)
			string midiPortName;    // Selected MIDI port name (default to first available)
			
			// Duration settings
			enum DurationType { DURATION_FIXED, SPEED_BASED, VEHICLE_BASED };
			DurationType durationType;
			int fixedDuration;      // milliseconds (default 500)
			
			// Velocity settings  
			enum VelocityType { VELOCITY_FIXED, CONFIDENCE_BASED };
			VelocityType velocityType;
			int fixedVelocity;      // 0-127 (default 100)
			
			// Constructor with sensible defaults
			MidiLine() : scaleNoteIndex(0), randomizeNote(true), octave(4), midiChannel(1), 
						 midiPortName(""), durationType(DURATION_FIXED), fixedDuration(500),
						 velocityType(VELOCITY_FIXED), fixedVelocity(100) {}
		};
		
		vector<MidiLine> lines;     // Enhanced line structure with MIDI properties
		ofPoint lineStart;
		bool isDrawingLine;
		
		// Task 3.4: Master Musical System - CORRECTED LOGIC
		int masterRootNote;         // 0-11 (C=0, C#=1, etc.) - root key for all lines
		string masterScale;         // "Major", "Minor", "Pentatonic", "Blues", "Chromatic" - scale for all lines
		// NOTE: randomization is now per-line (randomizeNote field in MidiLine struct)
		
		// Line selection system
		int selectedLineIndex;  // -1 means no line selected
		
		// Line editing system
		bool isDraggingEndpoint;
		int draggingLineIndex;
		bool isDraggingStartPoint;  // true = start point, false = end point
		
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
		
		// Task 5.1: Window resize management
		int originalWindowWidth;
		int originalWindowHeight;
		bool showResizeWarning;
		void rescaleLines(int oldWidth, int oldHeight, int newWidth, int newHeight);
		
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
			
			// Task 2.1: Enhanced tracking features
			vector<ofPoint> trajectory;        // History of center positions
			vector<float> trajectoryTimes;     // Timestamps for each position
			ofPoint velocity;                  // Current velocity vector (vx, vy)
			float acceleration;                // Change in speed magnitude
			ofColor trailColor;               // Visual trail color
			bool isOccluded;                  // Currently not detected but still tracked
			float predictionConfidence;       // Confidence in trajectory prediction
			int maxTrajectoryLength;          // Maximum trajectory history to keep
			
			// Constructor for initialization
			TrackedVehicle() : acceleration(0.0f), isOccluded(false),
							   predictionConfidence(0.0f), maxTrajectoryLength(30) {}
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
		
		// MIDI communication - Multi-port system
		vector<ofxMidiOut> midiOuts;          // Multiple MIDI output ports
		vector<string> midiPortNames;         // Names of available ports
		vector<bool> midiPortSelected;        // Which ports are selected for output
		vector<bool> midiPortConnected;       // Connection status of each port
		bool midiEnabled;                     // Global MIDI enable/disable
		bool anyMidiConnected;                // True if any port is connected
		
		// Task 2.3: MIDI note timing system
		struct MidiNoteEvent {
			int note;
			int velocity;
			int channel;
			unsigned long timestamp;
			unsigned long duration;              // Duration in milliseconds
			bool noteOffSent;
		};
		vector<MidiNoteEvent> activeMidiNotes;   // Currently playing MIDI notes
		int midiNoteDuration;                    // Default note duration (500ms)
		int midiActivityCounter;                 // Counter for MIDI activity display
		
		// ImGui interface
		ofxImGui::Gui gui;
		bool showGUI;
		
		// GUI control parameters
		float confidenceThreshold;
		int frameSkipValue;
		bool showDetections;
		bool showLines;
		
		// Task 2.1: Enhanced tracking GUI controls
		bool showTrajectoryTrails;
		bool showVelocityVectors;
		float trailFadeTime;          // How long trails persist (seconds)
		int maxTrajectoryPoints;      // Maximum trajectory history
		bool enableOcclusionTracking; // Track vehicles during brief occlusions
		
		void loadVideoFile(string path);
		void validateAndFixVideoSource();  // Ensure we always have a working video source
		
		// IP Camera methods - simplified approach
		bool connectToIPCamera(const string& url);
		void disconnectIPCamera();
		bool isValidIPCameraURL(const string& url);
		
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
		
		// Line selection methods
		float distanceToLineSegment(const ofPoint& point, const ofPoint& lineStart, const ofPoint& lineEnd);
		int findClosestLine(const ofPoint& clickPoint, float threshold = 15.0f);
		
		// Line editing methods
		bool isNearEndpoint(const ofPoint& clickPoint, int lineIndex, bool& isStartPoint, float threshold = 15.0f);
		
		// Task 2.1: Enhanced tracking methods
		void updateTrajectoryHistory(TrackedVehicle& vehicle);
		void calculateVelocityAndAcceleration(TrackedVehicle& vehicle);
		void drawTrajectoryTrails();
		void drawVelocityVectors();
		void handleOccludedVehicles();
		void sendEnhancedOSCMessage(const TrackedVehicle& vehicle, int lineId = -1);
		ofColor getTrailColorForVehicle(const TrackedVehicle& vehicle);
		void cleanupOldTrajectoryPoints();
		
		// MIDI methods - Multi-port system
		void setupMIDI();
		void refreshMIDIPorts();
		void connectMIDIPort(int portIndex);
		void disconnectMIDIPort(int portIndex);
		void sendTestMIDINote();
		void sendMIDINote(int note, int velocity, int channel = 1);
		void sendMIDINoteOff(int note, int channel = 1);
		void sendMIDINoteToAllPorts(int note, int velocity, int channel = 1);
		void sendMIDINoteOffToAllPorts(int note, int channel = 1);
		void updateMIDIConnectionStatus();
		
		// Task 2.3: MIDI line crossing integration
		void sendMIDILineCrossing(int lineId, const string& vehicleType, float confidence, float speed);
		void updateMIDITiming();             // Handle note-off timing
		void processMIDINoteOffs();          // Process pending note-offs
		
		// Task 3.1: Musical properties methods
		void initializeNewLineDefaults(MidiLine& line);
		string getNoteNameFromIndex(int noteIndex);     // Convert 0-11 to "C", "C#", etc.
		int getNoteIndexFromName(const string& noteName); // Convert "C", "C#" to 0-11
		vector<string> getAvailableScales();            // Return list of musical scales
		void updateLineMIDIProperties(int lineIndex);   // Apply property changes
		
		// Task 3.2: Musical scale system methods
		vector<int> getScaleIntervals(const string& scaleName);  // Get interval array for scale
		int getMidiNoteFromScale(int lineIndex);                 // Get random note from line's scale
		
		// Task 3.4: Master musical system methods
		int getMidiNoteFromMasterScale(int lineIndex);           // Get note using master scale system
		vector<string> getScaleNoteNames();                      // Get note names for current master scale
		void initializeMasterMusicalSystem();                    // Initialize master scale defaults
		
		// GUI methods
		void setupGUI();
		void drawGUI();
		void drawMainControlsTab();
		void drawMIDISettingsTab();
		void drawDetectionClassesTab();  // New tab for class selection
		
		// Configuration methods
		void saveConfig();
		void loadConfig();
		void setDefaultConfig();
		string getConfigPath();
		
		// Task 5.1: Window resize dialog
		void showWindowResizeDialog();
		
		// Task 5.2: MIDI configuration helpers
		bool validateMidiPort(const string& portName);
		string findClosestMidiPort(const string& originalPort);
};
