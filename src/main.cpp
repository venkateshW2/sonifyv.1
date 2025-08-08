#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLWindowSettings settings;
	settings.setSize(1050, 640);   // Fixed size: 640x640 video + 410px tabbed GUI
	// Video area: 640x640 (left), Tabbed GUI: 410px (right) with Main Controls + MIDI Settings tabs
	settings.windowMode = OF_WINDOW;
	// Window resizing will be handled in the app itself

	auto window = ofCreateWindow(settings);
	
	// Handle window resizing in the app's setup and windowResized functions
	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();
}
