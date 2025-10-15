#include "CommunicationManager.h"
#include "LineManager.h"
#include "ScaleManager.h"

CommunicationManager::CommunicationManager() {
    // OSC initialization
    oscHost = "127.0.0.1";
    oscPort = 12000;
    oscEnabled = true;
    
    // MIDI initialization
    midiEnabled = true;
    anyMidiConnected = false;
    midiNoteDuration = 500;
    midiActivityCounter = 0;
    totalMidiEvents = 0;  // Initialize MIDI event counter
    
    lineManager = nullptr;
    scaleManager = nullptr;
}

CommunicationManager::~CommunicationManager() {
    // Close all MIDI ports
    for (auto& midiOut : midiOuts) {
        if (midiOut.isOpen()) {
            midiOut.closePort();
        }
    }
}

void CommunicationManager::setup() {
    // Setup OSC
    setupOSC(oscHost, oscPort);
    
    // Setup MIDI
    setupMIDI();
    
    ofLogNotice() << "CommunicationManager: Initialized";
}

void CommunicationManager::update() {
    // Update MIDI timing for note-offs
    updateMIDITiming();
    processMIDINoteOffs();
    
    // Update MIDI connection status
    updateMIDIConnectionStatus();
    
    // Decay MIDI activity counter
    if (midiActivityCounter > 0) {
        midiActivityCounter--;
    }
}

void CommunicationManager::setupOSC(const string& host, int port) {
    oscHost = host;
    oscPort = port;
    oscSender.setup(oscHost, oscPort);
    ofLogNotice() << "CommunicationManager: OSC setup - " << oscHost << ":" << oscPort;
}

void CommunicationManager::sendOSCLineCrossing(int lineId, int vehicleId, int vehicleType, 
                                              const string& className, float confidence, float speed, 
                                              float speedMph, const ofPoint& crossingPoint) {
    if (!oscEnabled) return;
    
    // Send detailed OSC message
    ofxOscMessage message;
    message.setAddress("/line_cross");
    message.addIntArg(lineId);
    message.addIntArg(vehicleId);
    message.addIntArg(vehicleType);
    message.addStringArg(className);
    message.addFloatArg(confidence);
    message.addFloatArg(speed);
    message.addFloatArg(speedMph);
    message.addFloatArg(crossingPoint.x);
    message.addFloatArg(crossingPoint.y);
    message.addInt64Arg(ofGetElapsedTimeMillis());
    
    oscSender.sendMessage(message, false);
    
    // Send simple note message for musical applications
    ofxOscMessage noteMessage;
    noteMessage.setAddress("/note");
    noteMessage.addIntArg(lineId);
    noteMessage.addIntArg(60 + lineId);  // Simple note mapping
    noteMessage.addIntArg((int)(confidence * 127));  // Velocity from confidence
    
    oscSender.sendMessage(noteMessage, false);
    
    ofLogNotice() << "CommunicationManager: OSC line crossing sent - Line:" << lineId 
                 << " Vehicle:" << vehicleId << " Type:" << className;
}

void CommunicationManager::sendOSCPoseCrossing(int lineId, int personId, const string& jointName, 
                                              const ofPoint& crossingPoint, float confidence) {
    if (!oscEnabled) return;
    
    ofxOscMessage message;
    message.setAddress("/pose_cross");
    message.addIntArg(lineId);
    message.addIntArg(personId);
    message.addStringArg(jointName);
    message.addFloatArg(crossingPoint.x);
    message.addFloatArg(crossingPoint.y);
    message.addFloatArg(confidence);
    message.addInt64Arg(ofGetElapsedTimeMillis());
    
    oscSender.sendMessage(message, false);
    
    ofLogNotice() << "CommunicationManager: OSC pose crossing sent - Line:" << lineId 
                 << " Person:" << personId << " Joint:" << jointName;
}

void CommunicationManager::setupMIDI() {
    refreshMIDIPorts();
    ofLogNotice() << "CommunicationManager: MIDI setup complete";
}

void CommunicationManager::refreshMIDIPorts() {
    // Close existing ports
    for (auto& midiOut : midiOuts) {
        if (midiOut.isOpen()) {
            midiOut.closePort();
        }
    }
    
    midiOuts.clear();
    midiPortNames.clear();
    midiPortSelected.clear();
    midiPortConnected.clear();
    
    // Get available MIDI ports
    ofxMidiOut tempMidiOut;
    tempMidiOut.listOutPorts();
    
    int numPorts = tempMidiOut.getNumOutPorts();
    for (int i = 0; i < numPorts; i++) {
        string portName = tempMidiOut.getOutPortName(i);
        midiPortNames.push_back(portName);
        midiPortSelected.push_back(false);
        midiPortConnected.push_back(false);
        
        ofxMidiOut newMidiOut;
        midiOuts.push_back(newMidiOut);
    }
    
    // Auto-select first available port if available
    if (numPorts > 0) {
        setMIDIPortSelected(0, true);
    }
    
    ofLogNotice() << "CommunicationManager: Found " << numPorts << " MIDI ports";
}

void CommunicationManager::connectMIDIPort(int portIndex) {
    if (portIndex >= 0 && portIndex < midiOuts.size()) {
        if (!midiOuts[portIndex].isOpen()) {
            if (midiOuts[portIndex].openPort(portIndex)) {
                midiPortConnected[portIndex] = true;
                ofLogNotice() << "CommunicationManager: Connected to MIDI port: " 
                             << midiPortNames[portIndex];
            } else {
                midiPortConnected[portIndex] = false;
                ofLogNotice() << "CommunicationManager: Failed to connect to MIDI port: " 
                             << midiPortNames[portIndex];
            }
        }
    }
}

void CommunicationManager::disconnectMIDIPort(int portIndex) {
    if (portIndex >= 0 && portIndex < midiOuts.size()) {
        if (midiOuts[portIndex].isOpen()) {
            midiOuts[portIndex].closePort();
            midiPortConnected[portIndex] = false;
            ofLogNotice() << "CommunicationManager: Disconnected from MIDI port: " 
                         << midiPortNames[portIndex];
        }
    }
}

void CommunicationManager::setMIDIPortSelected(int portIndex, bool selected) {
    if (portIndex >= 0 && portIndex < midiPortSelected.size()) {
        midiPortSelected[portIndex] = selected;
        
        if (selected) {
            connectMIDIPort(portIndex);
        } else {
            disconnectMIDIPort(portIndex);
        }
    }
}

void CommunicationManager::sendMIDINote(int note, int velocity, int channel) {
    if (!midiEnabled) return;
    
    sendMIDINoteToAllPorts(note, velocity, channel);
    midiActivityCounter = 60;  // Show activity for 1 second at 60fps
    totalMidiEvents++;  // Increment MIDI event counter
    
    // Add to active notes for timing
    MidiNoteEvent noteEvent;
    noteEvent.note = note;
    noteEvent.velocity = velocity;
    noteEvent.channel = channel;
    noteEvent.timestamp = ofGetElapsedTimeMillis();
    noteEvent.duration = midiNoteDuration;
    noteEvent.noteOffSent = false;
    
    activeMidiNotes.push_back(noteEvent);
}

void CommunicationManager::sendMIDINoteOff(int note, int channel) {
    if (!midiEnabled) return;
    
    sendMIDINoteOffToAllPorts(note, channel);
}

void CommunicationManager::sendMIDILineCrossing(int lineId, const string& vehicleType, 
                                               float confidence, float speed) {
    if (!midiEnabled || !lineManager) return;
    
    const vector<LineManager::MidiLine>& lines = lineManager->getLines();
    if (lineId < 0 || lineId >= lines.size()) return;
    
    const LineManager::MidiLine& line = lines[lineId];
    
    // Get MIDI note from master scale system
    int midiNote = lineManager->getMidiNoteFromMasterScale(lineId);
    
    // Note generated from master scale system
    
    // Calculate velocity
    int velocity = line.fixedVelocity;
    if (line.velocityType == LineManager::MidiLine::CONFIDENCE_BASED) {
        velocity = (int)(confidence * 127);
    }
    
    // Calculate duration
    int duration = line.fixedDuration;
    if (line.durationType == LineManager::MidiLine::SPEED_BASED) {
        float normalizedSpeed = ofClamp(speed / 10.0f, 0.1f, 2.0f);
        duration = (int)(line.fixedDuration / normalizedSpeed);
    } else if (line.durationType == LineManager::MidiLine::VEHICLE_BASED) {
        if (vehicleType == "car") duration = 250;
        else if (vehicleType == "truck") duration = 750;
        else if (vehicleType == "motorcycle") duration = 150;
        else if (vehicleType == "bus") duration = 500;
        else duration = line.fixedDuration;
    }
    
    // Check if current scale requires microtonal MIDI (with pitch bend)
    bool useMicrotonal = false;
    ScaleManager::MicrotonalNote microNote;
    
    if (scaleManager && scaleManager->isMicrotonalityEnabled()) {
        // Get current scale info from LineManager
        string currentScale = lineManager->getMasterScale();
        int rootNote = lineManager->getMasterRootNote();
        
        // FIXED: Get the same note index that would be used by the regular system
        // This ensures randomization works properly for microtonal notes too
        int noteIndex;
        if (line.randomizeNote) {
            // Extract the scale note index from the generated MIDI note
            // The getMidiNoteFromMasterScale already handles randomization
            vector<int> scaleIntervals = lineManager->getScaleIntervals(currentScale);
            if (!scaleIntervals.empty()) {
                // Convert the generated MIDI note back to scale index
                int baseNote = midiNote - 12 - rootNote - (line.octave * 12);
                noteIndex = 0; // Default to root
                for (int i = 0; i < scaleIntervals.size(); i++) {
                    if (scaleIntervals[i] == baseNote) {
                        noteIndex = i;
                        break;
                    }
                }
            } else {
                noteIndex = 0;
            }
        } else {
            noteIndex = line.scaleNoteIndex;
        }
        
        // Get microtonal note data using the properly calculated note index
        microNote = scaleManager->getMicrotonalNote(currentScale, noteIndex, 
                                                   rootNote, line.octave);
        // Microtonal note generated
        
        // Use microtonal if pitch bend is needed (non-zero)
        useMicrotonal = (microNote.pitchBend != 0);
    }
    
    // Send appropriate MIDI note type
    if (useMicrotonal) {
        // Send microtonal note with pitch bend
        // Using microtonal path
        sendMicrotonalNote(microNote.midiNote, microNote.pitchBend, velocity, line.midiChannel);
        midiNote = microNote.midiNote; // Update for logging
    } else {
        // Send standard MIDI note
        // Using standard path
        sendMIDINote(midiNote, velocity, line.midiChannel);
    }
    
    // Update duration for this specific note
    if (!activeMidiNotes.empty()) {
        activeMidiNotes.back().duration = duration;
    }
    
    ofLogNotice() << "CommunicationManager: MIDI line crossing - Line:" << lineId 
                 << " Note:" << midiNote << " Velocity:" << velocity << " Duration:" << duration;
}

void CommunicationManager::sendTestMIDINote() {
    if (!midiEnabled) return;
    
    int testNote = 60;  // Middle C
    int testVelocity = 100;
    
    sendMIDINote(testNote, testVelocity, 1);
    ofLogNotice() << "CommunicationManager: Test MIDI note sent";
}

void CommunicationManager::sendMIDINoteToAllPorts(int note, int velocity, int channel) {
    for (int i = 0; i < midiOuts.size(); i++) {
        if (midiPortSelected[i] && midiPortConnected[i]) {
            midiOuts[i].sendNoteOn(channel, note, velocity);
        }
    }
}

void CommunicationManager::sendMIDINoteOffToAllPorts(int note, int channel) {
    for (int i = 0; i < midiOuts.size(); i++) {
        if (midiPortSelected[i] && midiPortConnected[i]) {
            midiOuts[i].sendNoteOff(channel, note, 0);
        }
    }
}

void CommunicationManager::updateMIDIConnectionStatus() {
    anyMidiConnected = false;
    for (bool connected : midiPortConnected) {
        if (connected) {
            anyMidiConnected = true;
            break;
        }
    }
}

void CommunicationManager::updateMIDITiming() {
    // This method is called in update() - timing logic handled in processMIDINoteOffs()
}

void CommunicationManager::processMIDINoteOffs() {
    unsigned long currentTime = ofGetElapsedTimeMillis();
    
    // Check for notes that should be turned off
    for (int i = activeMidiNotes.size() - 1; i >= 0; i--) {
        MidiNoteEvent& noteEvent = activeMidiNotes[i];
        
        if (!noteEvent.noteOffSent && (currentTime - noteEvent.timestamp) >= noteEvent.duration) {
            // Send note off
            sendMIDINoteOff(noteEvent.note, noteEvent.channel);
            noteEvent.noteOffSent = true;
            
            // Remove from active notes
            activeMidiNotes.erase(activeMidiNotes.begin() + i);
        }
    }
}

bool CommunicationManager::validateMidiPort(const string& portName) {
    for (const string& availablePort : midiPortNames) {
        if (availablePort == portName) {
            return true;
        }
    }
    return false;
}

string CommunicationManager::findClosestMidiPort(const string& originalPort) {
    if (midiPortNames.empty()) return "";
    
    // Look for partial matches
    for (const string& portName : midiPortNames) {
        if (portName.find(originalPort) != string::npos || 
            originalPort.find(portName) != string::npos) {
            return portName;
        }
    }
    
    // Return first available port as fallback
    return midiPortNames[0];
}

void CommunicationManager::saveToJSON(ofxJSONElement& json) {
    // OSC settings
    json["oscHost"] = oscHost;
    json["oscPort"] = oscPort;
    json["oscEnabled"] = oscEnabled;
    
    // MIDI settings
    json["midiEnabled"] = midiEnabled;
    json["midiNoteDuration"] = midiNoteDuration;
    
    // MIDI port selections
    ofxJSONElement portsJson;
    for (int i = 0; i < midiPortNames.size(); i++) {
        if (midiPortSelected[i]) {
            portsJson[portsJson.size()] = midiPortNames[i];
        }
    }
    json["selectedMidiPorts"] = portsJson;
}

void CommunicationManager::loadFromJSON(const ofxJSONElement& json) {
    // OSC settings
    if (json.isMember("oscHost")) {
        oscHost = json["oscHost"].asString();
    }
    if (json.isMember("oscPort")) {
        oscPort = json["oscPort"].asInt();
        setupOSC(oscHost, oscPort);
    }
    if (json.isMember("oscEnabled")) {
        oscEnabled = json["oscEnabled"].asBool();
    }
    
    // MIDI settings
    if (json.isMember("midiEnabled")) {
        midiEnabled = json["midiEnabled"].asBool();
    }
    if (json.isMember("midiNoteDuration")) {
        midiNoteDuration = json["midiNoteDuration"].asInt();
    }
    
    // MIDI port selections
    if (json.isMember("selectedMidiPorts")) {
        const ofxJSONElement& portsJson = json["selectedMidiPorts"];
        
        // First, deselect all ports
        for (int i = 0; i < midiPortSelected.size(); i++) {
            midiPortSelected[i] = false;
        }
        
        // Then select ports that were saved
        for (int i = 0; i < portsJson.size(); i++) {
            string savedPortName = portsJson[i].asString();
            
            // Find matching port and select it
            for (int j = 0; j < midiPortNames.size(); j++) {
                if (midiPortNames[j] == savedPortName || 
                    findClosestMidiPort(savedPortName) == midiPortNames[j]) {
                    setMIDIPortSelected(j, true);
                    break;
                }
            }
        }
    }
}

void CommunicationManager::setDefaults() {
    oscHost = "127.0.0.1";
    oscPort = 12000;
    oscEnabled = true;
    midiEnabled = true;
    midiNoteDuration = 500;
    midiActivityCounter = 0;
    totalMidiEvents = 0;
    
    // Clear MIDI selections
    for (int i = 0; i < midiPortSelected.size(); i++) {
        midiPortSelected[i] = false;
    }
    
    // Auto-select first port if available
    if (!midiPortNames.empty()) {
        setMIDIPortSelected(0, true);
    }
}

// =============================================================================
// MICROTONAL MIDI SUPPORT
// =============================================================================

void CommunicationManager::sendMIDIPitchBend(int pitchBend, int channel) {
    if (!midiEnabled) return;
    
    // Clamp pitch bend to valid MIDI range: -8192 to +8191
    pitchBend = ofClamp(pitchBend, -8192, 8191);
    
    // Convert to 14-bit unsigned value (0-16383)
    int pitchBendValue = pitchBend + 8192;
    
    // Split into MSB and LSB (7-bit each)
    int lsb = pitchBendValue & 0x7F;          // Lower 7 bits
    int msb = (pitchBendValue >> 7) & 0x7F;   // Upper 7 bits
    
    // Send pitch bend message to all connected ports
    for (int i = 0; i < midiOuts.size(); i++) {
        if (midiPortSelected[i] && midiPortConnected[i]) {
            midiOuts[i].sendPitchBend(channel, lsb, msb);
        }
    }
    
    midiActivityCounter = 30; // Show activity in UI
    totalMidiEvents++;
    
    ofLogVerbose() << "CommunicationManager: MIDI pitch bend sent - Channel:" << channel 
                   << " Value:" << pitchBend << " (14-bit:" << pitchBendValue << ")";
}

void CommunicationManager::sendMIDIControlChange(int controller, int value, int channel) {
    if (!midiEnabled) return;
    
    // Clamp to valid MIDI ranges
    controller = ofClamp(controller, 0, 127);
    value = ofClamp(value, 0, 127);
    
    // Send CC message to all connected ports
    for (int i = 0; i < midiOuts.size(); i++) {
        if (midiPortSelected[i] && midiPortConnected[i]) {
            midiOuts[i].sendControlChange(channel, controller, value);
        }
    }
    
    midiActivityCounter = 30;
    totalMidiEvents++;
    
    ofLogVerbose() << "CommunicationManager: MIDI CC sent - Channel:" << channel 
                   << " CC:" << controller << " Value:" << value;
}

void CommunicationManager::sendMicrotonalNote(int baseNote, int pitchBend, int velocity, int channel) {
    if (!midiEnabled) return;
    
    // First, send pitch bend for microtonal adjustment
    if (pitchBend != 0) {
        sendMIDIPitchBend(pitchBend, channel);
        
        // Small delay to ensure pitch bend is processed before note-on
        // This is handled by the MIDI buffer, no explicit delay needed
    }
    
    // Then send the note-on message
    sendMIDINote(baseNote, velocity, channel);
    
    ofLogNotice() << "CommunicationManager: Microtonal note sent - Note:" << baseNote 
                  << " PitchBend:" << pitchBend << " Velocity:" << velocity << " Channel:" << channel;
}

void CommunicationManager::sendMicrotonalNoteOff(int baseNote, int channel) {
    if (!midiEnabled) return;
    
    // Send note-off first
    sendMIDINoteOff(baseNote, channel);
    
    // Then reset pitch bend to center position
    resetPitchBend(channel);
    
    ofLogVerbose() << "CommunicationManager: Microtonal note off - Note:" << baseNote 
                   << " Channel:" << channel << " (pitch bend reset)";
}

void CommunicationManager::resetPitchBend(int channel) {
    if (!midiEnabled) return;
    
    // Send pitch bend value 0 (center position = 8192 in 14-bit)
    sendMIDIPitchBend(0, channel);
    
    ofLogVerbose() << "CommunicationManager: Pitch bend reset - Channel:" << channel;
}

