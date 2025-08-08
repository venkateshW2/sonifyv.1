#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLWindowSettings settings;
	settings.setSize(1050, 640);   // Expanded for tabbed interface: 640x640 video + 410px tabbed GUI
	// Video area: 640x640 (left), Tabbed GUI: 410px (right) with Main Controls + MIDI Settings tabs
	settings.windowMode = OF_WINDOW;

	auto window = ofCreateWindow(settings);

	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();
}
