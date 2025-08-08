# SonifyV1 MIDI Implementation Guide
## Comprehensive Musical Integration - Phase 1.5

**Prerequisites:** ✅ Week 1 Complete - CoreML detection, OSC communication, and professional GUI working perfectly

---

## **Development Philosophy**

### **Linear Progression Rule:**
**Never proceed to the next task until the current task works perfectly.** Each task builds upon the previous one and must be tested and verified before continuing.

### **Success Criteria Format:**
Every task ends with **"You should see:"** - clear visual or functional proof that the step worked. No moving forward until these criteria are met.

### **AI Agent Workflow:**
```bash
# Start each development session
cd /Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1
make && ./bin/SonifyV1.app/Contents/MacOS/SonifyV1

# For each task below:
claude "[AI Task from this guide]"
# Implement code
make && ./bin/SonifyV1.app/Contents/MacOS/SonifyV1
# Verify success criteria
# Only proceed when working perfectly
```

---

## **Phase 1: UI Foundation & Layout** (2-3 hours)

### **AI Task 1.1: Expand Window Layout for MIDI Controls**
```bash
claude "Expand SonifyV1 window layout to accommodate MIDI controls:

Requirements:
- Change window size from 640x640 to 1200x640
- Keep video/camera display area at 640x640 on the left side
- Add 560x640 sidebar area on the right for MIDI controls
- Ensure all existing functionality (video, detection, lines, OSC) works unchanged
- Video area must not be affected by the expansion
- Reorganize ImGui panels to fit new layout

Current files to modify:
- src/main.cpp: Update window creation to 1200x640
- src/ofApp.cpp: Adjust GUI layout for sidebar structure
- src/ofApp.h: Add any necessary UI state variables

Keep all existing functionality working - just expand the interface."
```

**Success Criteria:**
- ✅ Window opens at 1200x640 pixels
- ✅ Video/camera displays in left 640x640 area unchanged
- ✅ Right 560px area is empty space ready for MIDI controls
- ✅ All existing controls (detection, OSC, lines) still work
- ✅ Line drawing still works in left video area
- ✅ Application runs smoothly with new window size

### **AI Task 1.2: Implement Line Selection System**
```bash
claude "Add line selection and highlighting system to SonifyV1:

Requirements:
- Click on any drawn line to select it
- Selected line shows visual highlight (different color or outline)
- Only one line can be selected at a time
- Show selected line information in console for now
- Add selectedLineIndex variable to track which line is selected
- Implement click detection for line selection (point-to-line distance)

Implementation details:
- Calculate distance from mouse click to each line
- Select closest line within reasonable threshold (10-15 pixels)
- Highlight selected line by drawing with thicker outline or different color
- Show selected line index and properties in console log
- Add keyboard shortcut 'ESC' to deselect all lines

Keep all existing line drawing functionality unchanged."
```

**Success Criteria:**
- ✅ Click near any line to select it
- ✅ Selected line shows visual highlight (thicker or different color)
- ✅ Console shows selected line index and basic properties
- ✅ 'ESC' key deselects all lines
- ✅ Line selection doesn't interfere with new line drawing
- ✅ Only one line selected at a time

### **AI Task 1.3: Create MIDI Sidebar Panel Structure**
```bash
claude "Create sidebar panel structure for MIDI controls in SonifyV1:

Requirements:
- Organize right sidebar (560px wide) into three main panels
- Panel 1 (top): 'Lines List' - shows all drawn lines
- Panel 2 (middle): 'MIDI Properties' - shows selected line properties  
- Panel 3 (bottom): 'MIDI System' - shows MIDI ports and global settings
- Use ImGui Begin/End with appropriate sizing for each panel
- Add placeholders for future MIDI controls
- Maintain existing GUI panels on left side if needed

Panel layout:
- Lines List Panel: 560x180 pixels (top of sidebar)
- MIDI Properties Panel: 560x280 pixels (middle)
- MIDI System Panel: 560x180 pixels (bottom)

Show basic placeholder content in each panel for now."
```

**Success Criteria:**
- ✅ Right sidebar shows three organized panels
- ✅ 'Lines List' panel shows placeholder content
- ✅ 'MIDI Properties' panel shows placeholder for selected line
- ✅ 'MIDI System' panel shows placeholder for MIDI settings
- ✅ Panels are properly sized and positioned
- ✅ All existing GUI controls still accessible and functional

---

## **Phase 2: MIDI Infrastructure** (3-4 hours)

### **AI Task 2.1: Add ofxMidi Integration**
```bash
claude "Add ofxMidi addon and basic MIDI infrastructure to SonifyV1:

Requirements:
- Add ofxMidi to addons.make
- Include ofxMidi headers in ofApp.h
- Create basic MidiManager class or integrate MIDI directly in ofApp
- Initialize MIDI system in setup()
- Add MIDI port detection and enumeration
- Test basic MIDI note-on/note-off functionality
- Handle MIDI initialization errors gracefully

Implementation:
- Add 'ofxMidi' to addons.make file
- Include <ofxMidi.h> in ofApp.h
- Add ofxMidiOut member variable
- Add available MIDI ports vector<string>
- Initialize MIDI in setup() with error handling
- Create test function to send MIDI note (for testing)

Test with 'T' key sending test MIDI note to verify MIDI output works."
```

**Success Criteria:**
- ✅ Project compiles with ofxMidi addon successfully
- ✅ MIDI system initializes without errors
- ✅ Available MIDI ports detected and listed in console
- ✅ Test key 'T' sends MIDI note to default port
- ✅ MIDI output works with DAW or software MIDI monitor
- ✅ Application handles MIDI port unavailable gracefully

### **AI Task 2.2: Implement MIDI Port Detection and Management**
```bash
claude "Implement comprehensive MIDI port management for SonifyV1:

Requirements:
- Detect all available MIDI output ports on startup
- Display available ports in 'MIDI System' panel
- Allow selection of active MIDI ports (support multiple ports)
- Add 'Refresh Ports' button to re-scan for MIDI devices
- Show port connection status (connected/disconnected)
- Default to IAC Driver if available, otherwise first available port
- Handle port disconnect/reconnect gracefully

MIDI System Panel should show:
- List of all detected MIDI ports with checkboxes
- Connection status indicator for each port
- 'Refresh Ports' button
- 'Test MIDI' button to send test note to selected ports

Create multi-port output system for future line-specific port assignment."
```

**Success Criteria:**
- ✅ MIDI System panel shows all available MIDI ports
- ✅ Can select/deselect multiple ports with checkboxes
- ✅ IAC Driver automatically selected if available
- ✅ 'Refresh Ports' button rescans and updates port list
- ✅ 'Test MIDI' sends note to all selected ports
- ✅ Port connection status shows correctly
- ✅ System handles port disconnection without crashing

### **AI Task 2.3: Create Basic Line-MIDI Integration**
```bash
claude "Integrate basic MIDI output with line crossing system:

Requirements:
- Send MIDI note-on when vehicle crosses any line
- Send MIDI note-off after fixed duration (500ms for now)
- Use default MIDI note C4 (note 60) for all lines initially
- Use fixed velocity 100 for all notes initially
- Send to all currently selected MIDI ports
- Add MIDI activity indicator in MIDI System panel
- Show MIDI messages in console for debugging

Integration points:
- Modify existing line crossing detection (checkLineCrossings function)
- Add MIDI note transmission alongside existing OSC messages
- Implement note-off timer system for duration control
- Add MIDI message counter and activity display

Keep all existing OSC functionality unchanged - just add MIDI alongside."
```

**Success Criteria:**
- ✅ Vehicle crossing any line triggers MIDI note-on
- ✅ MIDI note-off sent after 500ms automatically
- ✅ MIDI notes sent to all selected MIDI ports
- ✅ Console shows MIDI messages for debugging
- ✅ MIDI activity indicator shows in system panel
- ✅ OSC messages continue working alongside MIDI
- ✅ Multiple vehicles can trigger overlapping MIDI notes

---

## **Phase 3: Musical Properties System** (4-5 hours)

### **AI Task 3.1: Extend Line Data Structure with Musical Properties**
```bash
claude "Add comprehensive musical properties to line data structure:

Requirements:
- Extend existing line structure with MIDI properties
- Add musical properties for each line:
  - MIDI note (0-11 for C, C#, D, D#, E, F, F#, G, G#, A, A#, B)
  - Octave (0-10, default 4 for middle octave)
  - Musical scale (Major, Minor, Pentatonic, Blues, Chromatic)
  - MIDI channel (1-16, default 1)
  - MIDI port name (string, default to first available)
  - Duration type (Fixed, Speed-based, Vehicle-based)
  - Fixed duration value (milliseconds, default 500)
  - Velocity type (Fixed, Confidence-based) 
  - Fixed velocity (0-127, default 100)

Update line creation, saving, and loading to include MIDI properties.
Initialize new lines with sensible musical defaults."
```

**Success Criteria:**
- ✅ Each line has complete set of musical properties
- ✅ New lines created with default MIDI properties
- ✅ Line properties save/load in JSON configuration
- ✅ Console shows MIDI properties when line selected
- ✅ Application compiles and runs with extended data structure
- ✅ Existing lines work with new properties system

### **AI Task 3.2: Implement Musical Scale System**
```bash
claude "Create musical scale system with intelligent note selection:

Requirements:
- Define musical scales as note interval arrays:
  - Major: [0,2,4,5,7,9,11] (C,D,E,F,G,A,B)
  - Minor: [0,2,3,5,7,8,10] (C,D,Eb,F,G,Ab,Bb)
  - Pentatonic: [0,2,4,7,9] (C,D,E,G,A)
  - Blues: [0,3,5,6,7,10] (C,Eb,F,F#,G,Bb)
  - Chromatic: [0,1,2,3,4,5,6,7,8,9,10,11] (all notes)
- Random note selection from assigned scale on each line crossing
- Calculate final MIDI note: baseNote + scale[randomIndex] + (octave * 12)
- Update line crossing to use scale-based random note selection
- Add scale name display in console when note played

Create getMidiNoteFromScale() function that returns random note from line's scale."
```

**Success Criteria:**
- ✅ Musical scales defined and working correctly
- ✅ Each line crossing plays random note from assigned scale
- ✅ Different scales produce recognizably different musical patterns
- ✅ Octave setting correctly shifts notes up/down
- ✅ Console shows note name and scale when played
- ✅ Chromatic scale plays any of 12 notes randomly

### **AI Task 3.3: Create MIDI Properties GUI Panel**
```bash
claude "Build comprehensive MIDI properties panel for selected line:

Requirements:
- Show detailed MIDI properties for currently selected line
- MIDI Properties Panel should include:
  - Note selector: dropdown or buttons for C, C#, D, D#, E, F, F#, G, G#, A, A#, B
  - Octave slider: 0-10 with current value display
  - Scale dropdown: Major, Minor, Pentatonic, Blues, Chromatic
  - MIDI channel slider: 1-16
  - Port selection: dropdown of available MIDI ports
  - Duration mode: radio buttons for Fixed/Speed-based/Vehicle-based
  - Duration value: slider for fixed duration in ms
  - Velocity mode: radio buttons for Fixed/Confidence-based
  - Velocity value: slider for fixed velocity 0-127
- Real-time updates: changes immediately affect line behavior
- Show 'No line selected' message when no line selected

All changes should immediately update the selected line's properties."
```

**Success Criteria:**
- ✅ MIDI Properties panel shows all controls for selected line
- ✅ Note selector changes line's base note correctly
- ✅ Octave slider shifts notes by octaves (12 semitones)
- ✅ Scale dropdown changes random note selection behavior
- ✅ MIDI channel selection works (test with MIDI monitor)
- ✅ Duration and velocity controls update line behavior
- ✅ Real-time property changes immediately affect MIDI output
- ✅ Panel shows 'No line selected' when appropriate

### **AI Task 3.4: Implement Lines List Panel**
```bash
claude "Create comprehensive lines list panel showing all drawn lines:

Requirements:
- Lines List Panel should display:
  - List of all drawn lines (Line 1, Line 2, etc.)
  - Each line's current musical properties (note, scale, channel)
  - Visual color indicator matching line color
  - Click line in list to select it (same as clicking line in video)
  - Show which line is currently selected
- Add line management buttons:
  - 'Delete Selected' button
  - 'Duplicate Selected' button  
  - 'Clear All Lines' button (same as 'C' key)
- Display total line count
- Show basic line info: start/end coordinates

List format example:
'Line 1: C4 Major Ch1 [Red]'
'Line 2: F#3 Blues Ch2 [Blue]' <- selected
etc."
```

**Success Criteria:**
- ✅ Lines List shows all drawn lines with musical properties
- ✅ Click line in list selects it (highlights both list and video)
- ✅ Selected line clearly highlighted in list
- ✅ Line colors match between list and video display
- ✅ 'Delete Selected' removes selected line completely
- ✅ 'Duplicate Selected' creates copy with same properties
- ✅ Line list updates in real-time as lines added/removed

---

## **Phase 4: Advanced Features** (3-4 hours)

### **AI Task 4.1: Implement Vehicle-Type Duration Mapping**
```bash
claude "Add intelligent vehicle-type duration mapping system:

Requirements:
- Vehicle-type duration mapping:
  - Car (class 2): 250ms (standard duration)
  - Motorcycle (class 3): 150ms (quick, agile)
  - Bus (class 5): 500ms (substantial, longer)
  - Truck (class 7): 750ms (massive, longest)
- Speed-based duration calculation:
  - Formula: baseDuration * (1.0 / normalizedSpeed)
  - Faster vehicles = shorter notes, slower = longer notes
- Update line crossing system to use vehicle-specific durations
- Add duration mode selection in MIDI Properties panel
- Show calculated duration in console for debugging

Duration modes:
- Fixed: Use user-defined milliseconds
- Speed-based: Calculate from vehicle speed
- Vehicle-based: Use vehicle type mapping"
```

**Success Criteria:**
- ✅ Different vehicle types produce different note durations
- ✅ Trucks play longer notes than cars than motorcycles
- ✅ Speed-based mode makes faster vehicles shorter notes
- ✅ Fixed mode uses exact user-specified duration
- ✅ Duration mode selection works in GUI
- ✅ Console shows calculated duration for each note played

### **AI Task 4.2: Add Line Editing Capabilities**
```bash
claude "Implement comprehensive line editing and management:

Requirements:
- Move lines by dragging endpoints:
  - Detect mouse near line endpoints (within 15 pixels)
  - Cursor changes to resize cursor when near endpoint
  - Drag endpoint to move line start or end point
  - Visual feedback during drag operation
- Right-click context menu on lines:
  - 'Delete Line' option
  - 'Duplicate Line' option
  - 'Edit Properties' option (selects line)
- Delete key removes currently selected line
- Undo system for line modifications (optional but nice)
- Maintain MIDI properties when moving lines
- Update line list display when lines modified

Interaction priority:
1. Line endpoint dragging (if near endpoint)
2. Line selection (if clicking on line)
3. New line creation (if clicking empty space)"
```

**Success Criteria:**
- ✅ Can drag line endpoints to move lines around
- ✅ Cursor changes appropriately near endpoints
- ✅ Right-click on line shows context menu
- ✅ Context menu options work correctly
- ✅ Delete key removes selected line
- ✅ Line modifications preserve MIDI properties
- ✅ Moved lines still trigger MIDI correctly at new positions

### **AI Task 4.3: Implement MIDI CC Placeholder System**
```bash
claude "Add MIDI Continuous Controller (CC) placeholder system:

Requirements:
- Add CC properties to each line:
  - Enable/disable CC sending (checkbox)
  - CC controller number (0-127, default suggestions)
  - CC value calculation mode (Fixed, Speed, Confidence, Position)
- Default CC mappings:
  - CC1 (Modulation): Vehicle speed (0-127)
  - CC7 (Volume): Detection confidence (0-127)
  - CC10 (Pan): Line position - left=0, center=64, right=127
  - CC11 (Expression): Vehicle type (car=32, truck=96, etc.)
- Add CC controls to MIDI Properties panel
- Send CC messages alongside note-on messages
- Show CC activity in MIDI System panel

CC calculation examples:
- Speed: normalize vehicle.speed to 0-127 range
- Confidence: detection.confidence * 127
- Position: (lineCenter.x / windowWidth) * 127"
```

**Success Criteria:**
- ✅ Each line can send CC messages alongside notes
- ✅ CC controller number selectable per line
- ✅ Different CC value modes produce different ranges
- ✅ Speed-based CC shows vehicle speed variations
- ✅ Position-based CC reflects line's screen position
- ✅ CC messages visible in MIDI monitor software
- ✅ CC system doesn't interfere with note messages

---

## **Phase 5: Integration & Polish** (2-3 hours)

### **AI Task 5.1: Window Resize Warning and Line Rescaling**
```bash
claude "Implement window resize management and line coordinate preservation:

Requirements:
- Detect window size changes and show warning dialog
- Warning message: 'Window size changed. Line positions may be incorrect. Would you like to rescale lines to fit new window size?'
- Dialog options:
  - 'Rescale Lines' - proportionally scale all line coordinates
  - 'Keep Original' - maintain absolute coordinates
  - 'Reset Lines' - clear all lines and start over
- Save original window size in configuration
- Show resolution mismatch warning on application startup
- Add 'Rescale Lines' button in Lines List panel

Rescaling formula:
- newX = (oldX / oldWindowWidth) * newWindowWidth
- newY = (oldY / oldWindowHeight) * newWindowHeight
- Apply to all line start and end points"
```

**Success Criteria:**
- ✅ Window resize triggers warning dialog
- ✅ 'Rescale Lines' option proportionally adjusts all lines
- ✅ Rescaled lines maintain relative positions
- ✅ 'Keep Original' preserves exact coordinates
- ✅ Application warns about resolution mismatch on startup
- ✅ Manual rescale button works in Lines List panel

### **AI Task 5.2: MIDI Configuration System Integration**
```bash
claude "Integrate MIDI properties into configuration save/load system:

Requirements:
- Extend JSON configuration to include all MIDI properties
- Save/load MIDI properties for each line:
  - Musical note, octave, scale
  - MIDI channel, port name
  - Duration and velocity settings
  - CC properties and values
- Save MIDI system settings:
  - Selected MIDI ports
  - Global MIDI preferences
- Add MIDI-specific save/load validation
- Handle missing MIDI ports gracefully (port no longer available)
- Maintain backward compatibility with existing configurations

JSON structure additions:
- lines[i].midi.note, lines[i].midi.octave, etc.
- midiSystem.selectedPorts[], midiSystem.defaultChannel, etc."
```

**Success Criteria:**
- ✅ All MIDI properties save to configuration file
- ✅ MIDI properties restore correctly on application restart
- ✅ Missing MIDI ports handled gracefully with fallback
- ✅ Existing non-MIDI configurations still load correctly
- ✅ Save/load preserves complex musical scale settings
- ✅ Configuration file remains human-readable

### **AI Task 5.3: MIDI Testing and Validation**
```bash
claude "Comprehensive MIDI testing and performance validation:

Requirements:
- Test MIDI output with multiple DAWs:
  - Logic Pro (if available)
  - GarageBand
  - Ableton Live (if available)
  - Any available software MIDI monitor
- Performance testing:
  - Multiple lines with different vehicle crossings
  - High-speed vehicle detection with rapid MIDI triggering
  - Multiple simultaneous MIDI notes (polyphony)
  - Extended operation stability (30+ minutes)
- MIDI timing accuracy validation
- Multi-port output testing
- Error recovery testing (disconnect/reconnect MIDI devices)
- Memory usage monitoring during extended MIDI operation

Create comprehensive test scenarios document."
```

**Success Criteria:**
- ✅ MIDI output works correctly in multiple DAWs
- ✅ No MIDI timing issues or jitter during rapid triggering
- ✅ Multiple simultaneous notes play correctly (polyphony)
- ✅ System stable during extended MIDI operation
- ✅ MIDI port disconnection handled gracefully
- ✅ No memory leaks during continuous MIDI output
- ✅ Performance remains at 60fps with heavy MIDI activity

---

## **MIDI Integration Success Milestone**

**At the end of MIDI integration, you should have:**
- ✅ **Expanded UI** with 1200x640 layout and dedicated MIDI sidebar
- ✅ **Complete MIDI Output** with multi-port support and comprehensive controls
- ✅ **Musical Line System** with scales, octaves, and intelligent note selection
- ✅ **Vehicle-Intelligent Duration** mapping truck→long notes, motorcycles→short notes
- ✅ **Professional MIDI Controls** with real-time property editing
- ✅ **Line Management** with selection, editing, moving, and deletion
- ✅ **CC Controller System** for advanced MIDI expression
- ✅ **Robust Configuration** with complete save/load for all MIDI properties
- ✅ **Production Stability** tested with multiple DAWs and extended operation

**Demo capability:** Draw lines with different musical properties, watch vehicles cross and trigger contextually appropriate musical notes sent to multiple DAWs simultaneously, with intelligent duration and velocity based on vehicle type and detection confidence.

---

## **AI Development Notes for MIDI Implementation**

### **Critical Success Patterns:**
- **Test immediately** - every task must produce observable MIDI output
- **Preserve existing** - never break CoreML detection or OSC functionality  
- **Incremental complexity** - start simple (fixed notes) then add musical intelligence
- **Real-time feedback** - every change immediately audible in connected DAW

### **Common Pitfalls to Avoid:**
- Don't break existing line drawing system
- Don't compromise video detection performance with heavy MIDI processing
- Don't create MIDI feedback loops or stuck notes
- Don't forget note-off messages (causes stuck notes)

### **Testing Requirements:**
- Always test with actual DAW or MIDI monitor software
- Verify note-off messages prevent stuck notes
- Test multi-port output doesn't cause timing issues
- Confirm UI changes immediately affect MIDI behavior

**This MIDI integration transforms SonifyV1 from a highway sonification system into a comprehensive real-time musical instrument that intelligently responds to traffic patterns with contextually appropriate musical output.**