#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxMidi.h"
#include "ofxJSON.h"

class CommunicationManager {
public:
    CommunicationManager();
    ~CommunicationManager();
    
    void setup();
    void update();
    void draw();
    
    // OSC methods - EXACT COPY from working backup
    void setupOSC(const string& host, int port);
    void sendOSCLineCrossing(int lineId, int vehicleId, int vehicleType, 
                            const string& className, float confidence, float speed, 
                            float speedMph, const ofPoint& crossingPoint);
    void sendOSCPoseCrossing(int lineId, int personId, const string& jointName, 
                            const ofPoint& crossingPoint, float confidence);
    
    // MIDI methods - EXACT COPY from working backup  
    void setupMIDI();
    void refreshMIDIPorts();
    void connectMIDIPort(int portIndex);
    void disconnectMIDIPort(int portIndex);
    void setMIDIPortSelected(int portIndex, bool selected);
    void sendMIDINote(int note, int velocity, int channel);
    void sendMIDINoteOff(int note, int channel);
    void sendMIDILineCrossing(int lineId, const string& vehicleType, float confidence, float speed);
    void sendTestMIDINote();
    
    // Microtonal MIDI support
    void sendMIDIPitchBend(int pitchBend, int channel);
    void sendMIDIControlChange(int controller, int value, int channel);
    void sendMicrotonalNote(int baseNote, int pitchBend, int velocity, int channel);
    void sendMicrotonalNoteOff(int baseNote, int channel);
    void resetPitchBend(int channel);
    
    // Configuration methods
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // Manager connections
    void setManagers(class LineManager* lineMgr) { lineManager = lineMgr; }
    void setScaleManager(class ScaleManager* scaleMgr) { scaleManager = scaleMgr; }
    
    // UI Manager methods for MIDI port selection
    vector<string> getMidiPortNames() const { return midiPortNames; }
    vector<bool> getMidiPortSelected() const { return midiPortSelected; }
    vector<bool> getMidiPortConnected() const { return midiPortConnected; }
    
    // Live tracking data getters for UI Manager
    int getTotalMidiEvents() const { return totalMidiEvents; }
    
    // EXACT same communication variables as working backup
    ofxOscSender oscSender;
    string oscHost;
    int oscPort;
    bool oscEnabled;
    
    // MIDI system - EXACT COPY from working backup
    vector<ofxMidiOut> midiOuts;
    vector<string> midiPortNames;
    vector<bool> midiPortSelected;
    vector<bool> midiPortConnected;
    bool midiEnabled;
    bool anyMidiConnected;
    int midiNoteDuration;
    int midiActivityCounter;
    
    // MIDI timing system - EXACT COPY from working backup
    struct MidiNoteEvent {
        int note;
        int velocity;
        int channel;
        unsigned long timestamp;
        int duration;
        bool noteOffSent;
    };
    vector<MidiNoteEvent> activeMidiNotes;
    
    // MIDI tracking for UI
    int totalMidiEvents;
    
private:
    // Helper methods
    void sendMIDINoteToAllPorts(int note, int velocity, int channel);
    void sendMIDINoteOffToAllPorts(int note, int channel);
    void updateMIDIConnectionStatus();
    void updateMIDITiming();
    void processMIDINoteOffs();
    bool validateMidiPort(const string& portName);
    string findClosestMidiPort(const string& originalPort);
    
    class LineManager* lineManager;
    class ScaleManager* scaleManager;
};