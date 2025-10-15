# SonifyV1 - Complete How To Use Guide

**Version 2.0** | Advanced Tempo-Synchronized Microtonal Highway Sonification

---

## Table of Contents
1. [Quick Start (5 Minutes)](#quick-start-5-minutes)
2. [Understanding the Interface](#understanding-the-interface)
3. [Basic Workflow](#basic-workflow)
4. [Advanced Features](#advanced-features)
5. [Musical Configuration](#musical-configuration)
6. [Tips & Tricks](#tips--tricks)
7. [Troubleshooting](#troubleshooting)
8. [Workflow Examples](#workflow-examples)

---

## Quick Start (5 Minutes)

### Your First Musical Line in 5 Steps

**1. Launch the Application**
```
Double-click SonifyV1.app
Grant camera permissions when prompted
```

**2. Show the Interface**
```
Press 'g' key → GUI appears
```

**3. Enable Object Detection**
```
Press 'd' key → Detection starts
You'll see bounding boxes around detected objects
```

**4. Draw Your First Line**
```
LEFT CLICK anywhere on the video → Start point
Move mouse to desired end point
RIGHT CLICK → Complete the line
```

**5. Test It!**
```
Point camera at traffic (or press 'o' to load a video file)
Watch for vehicles crossing your line
Each crossing triggers a musical note!
```

🎉 **You're making music with computer vision!**

---

## Understanding the Interface

### Main Window Layout

```
┌─────────────────────────────────────────────────────┐
│  [Video/Camera Feed - 640x640]                      │
│                                                      │
│  • Bounding boxes = Detected objects                │
│  • Colored lines = Your musical instruments         │
│  • Green flash = Line crossing detected!            │
│                                                      │
└─────────────────────────────────────────────────────┘
```

### The 4-Tab GUI System

Press **'g'** to toggle the GUI on/off.

#### **Tab 1: Main Controls**
```
┌─ Main Controls ──────────────────────────────────┐
│                                                   │
│ [Detection Controls]                             │
│   • Enable/Disable detection                      │
│   • Frame skip (performance vs accuracy)          │
│   • Detection confidence threshold                │
│                                                   │
│ [Video Controls]                                  │
│   • Open video file                               │
│   • Camera/Video source toggle                    │
│   • Playback controls (play/pause/seek)           │
│   • Loop enable/disable                           │
│                                                   │
│ [Line Management]                                 │
│   • Clear all lines button                        │
│   • Line list with delete options                 │
│                                                   │
│ [System Status]                                   │
│   • FPS counter                                   │
│   • Detection status                              │
│   • Active lines count                            │
│                                                   │
└───────────────────────────────────────────────────┘
```

#### **Tab 2: MIDI Settings**
```
┌─ MIDI Settings ──────────────────────────────────┐
│                                                   │
│ [Global Musical Framework]                        │
│   • Master MIDI port selection                    │
│   • Root key (C, C#, D, etc.)                     │
│   • Current scale (Major, Minor, etc.)            │
│                                                   │
│ [Tempo & Rhythm]                                  │
│   • BPM slider (40-200)                           │
│   • Note subdivision (whole, half, quarter, etc.) │
│   • Swing percentage (0-100%)                     │
│   • Beat phase indicator (real-time)              │
│                                                   │
│ [Randomization]                                   │
│   • Tempo-synchronized randomization toggle       │
│   • Random note preview (shows current selection) │
│   • Scale degree weighting                        │
│                                                   │
│ [Per-Line Settings] (for each drawn line)         │
│   • MIDI channel (1-16)                           │
│   • Octave range (0-8)                            │
│   • Note duration (ms)                            │
│   • Velocity (0-127)                              │
│   • Tempo sync enabled/disabled                   │
│                                                   │
└───────────────────────────────────────────────────┘
```

#### **Tab 3: Detection Classes**
```
┌─ Detection Classes ──────────────────────────────┐
│                                                   │
│ Complete 80-class COCO dataset organized by      │
│ category:                                         │
│                                                   │
│ [Vehicles]                                        │
│   ☑ Car  ☑ Motorcycle  ☑ Bus  ☑ Truck           │
│   ☑ Bicycle  ☑ Train  ☑ Boat                     │
│                                                   │
│ [People]                                          │
│   ☑ Person                                        │
│                                                   │
│ [Animals]                                         │
│   ☑ Bird  ☑ Cat  ☑ Dog  ☑ Horse  ☑ Sheep        │
│   ... (16 total animal classes)                  │
│                                                   │
│ [Objects]                                         │
│   ☑ Sports  ☑ Kitchen  ☑ Furniture  ☑ Electronics│
│   ... (50+ object classes)                       │
│                                                   │
│ • Check/uncheck to enable/disable detection      │
│ • Disable unused classes to improve performance  │
│                                                   │
└───────────────────────────────────────────────────┘
```

#### **Tab 4: Scale Manager**
```
┌─ Scale Manager ──────────────────────────────────┐
│                                                   │
│ [Current Scale Info]                              │
│   • Name: "Major"                                 │
│   • Notes: 7                                      │
│   • Microtonal: No                                │
│                                                   │
│ [Standard 12-Tone Scales]                         │
│   [Major] [Minor] [Dorian] [Phrygian]           │
│   [Lydian] [Mixolydian] [Locrian]                │
│   [Pentatonic] [Blues] [Chromatic]               │
│                                                   │
│ [Microtonal Scales]                               │
│   [Just Intonation] [Pythagorean]                │
│   [Quarter-Comma Meantone]                        │
│                                                   │
│ [Scala File Scales] (.scl format)                 │
│   • Custom imported scales appear here            │
│   • Click "Refresh" to reload from disk           │
│                                                   │
│ [Scale Analysis Table]                            │
│   Degree | Cents  | Ratio    | MIDI Note         │
│   ─────────────────────────────────────────       │
│   1      | 0.0    | 1/1      | 60 (C4)           │
│   2      | 200.0  | 9/8      | 62 (D4)           │
│   3      | 400.0  | 5/4      | 64 (E4)           │
│   ...                                             │
│                                                   │
│ [File Management]                                 │
│   • Import .scl files                             │
│   • Export current scale                          │
│   • Scala format documentation                    │
│                                                   │
└───────────────────────────────────────────────────┘
```

---

## Basic Workflow

### Workflow 1: Highway Sonification (Original Use Case)

**Goal**: Create music from real-time highway traffic

**Steps**:

1. **Setup Camera**
   ```
   • Position USB camera to view highway/road
   • Or use pre-recorded highway video file
   • Press 'r' if camera needs restart
   ```

2. **Configure Detection**
   ```
   • Press 'g' to show GUI
   • Go to "Detection Classes" tab
   • Enable only: Car, Truck, Bus, Motorcycle
   • Disable all other classes (improves performance)
   ```

3. **Draw Musical Lines**
   ```
   • Draw horizontal line across traffic lanes
   • Each lane can have its own line
   • Lines represent different instruments/channels
   ```

4. **Set Up MIDI Output**
   ```
   • Go to "MIDI Settings" tab
   • Select your MIDI port (connects to DAW/synth)
   • Choose BPM (try 120 for steady rhythm)
   • Select scale (try "Minor" for darker sound)
   ```

5. **Configure Per-Line Settings**
   ```
   Line 1 (Lane 1): Channel 1, Octave 3, Velocity 100
   Line 2 (Lane 2): Channel 2, Octave 4, Velocity 80
   Line 3 (Lane 3): Channel 3, Octave 5, Velocity 60
   ```

6. **Start Making Music!**
   ```
   • Press 'd' to enable detection
   • Watch vehicles cross lines
   • Each crossing = musical note
   • Tempo-synced randomization creates melodies
   ```

**Result**: Real-world traffic creates generative musical patterns!

---

### Workflow 2: Pre-Recorded Video Analysis

**Goal**: Convert a video file into a musical composition

**Steps**:

1. **Load Video**
   ```
   • Press 'o' to open file dialog
   • Select your video file (MP4, MOV, AVI)
   • Video loads and pauses at start
   ```

2. **Preview and Plan**
   ```
   • Use LEFT/RIGHT arrow keys to scrub through video
   • Identify interesting movement patterns
   • Plan where to place lines for musical events
   ```

3. **Draw Strategic Lines**
   ```
   • Place lines where objects frequently cross
   • Consider rhythm and timing
   • Multiple lines = polyphonic output
   ```

4. **Configure Musical Parameters**
   ```
   • Choose appropriate BPM matching video pace
   • Select scale that fits the mood
   • Set note durations (longer = more sustained)
   ```

5. **Record the Output**
   ```
   • Set your DAW to record MIDI
   • Press SPACE to play video
   • Press 'd' to enable detection
   • Let video play through while recording
   ```

6. **Post-Process**
   ```
   • Edit MIDI in your DAW
   • Add instruments/effects
   • Mix and master
   ```

**Result**: Video-driven algorithmic composition!

---

### Workflow 3: Live Performance Setup

**Goal**: Use SonifyV1 as a live performance instrument

**Steps**:

1. **Prepare Your Environment**
   ```
   • Set up camera pointing at performance area
   • Connect MIDI to live synthesis software
   • Test camera visibility and lighting
   ```

2. **Design Your "Instrument"**
   ```
   • Draw lines in strategic positions
   • Each line = different sound/instrument
   • Test by moving through frame
   ```

3. **Configure for Responsiveness**
   ```
   • Lower frame skip for real-time response
   • Choose faster BPM for energetic performance
   • Enable tempo sync for tight rhythmic control
   ```

4. **Set Up Multiple Layers**
   ```
   Line 1: Bass notes (low octave, high velocity)
   Line 2: Melody (mid octave, medium velocity)
   Line 3: Percussion trigger (channel to drums)
   ```

5. **Performance Techniques**
   ```
   • Cross lines with your body
   • Use props/objects for varied timbre
   • Vary crossing speed for expression
   • Combine with other performers
   ```

**Result**: Interactive musical performance with visual feedback!

---

## Advanced Features

### Tempo-Synchronized Randomization

**What It Does**: Generates musical notes on a tempo grid with intelligent scale-aware randomization.

**How to Use**:
1. Go to **MIDI Settings** tab
2. Set your desired BPM (e.g., 120)
3. Choose note subdivision:
   - **Whole Note**: Very slow, sustained
   - **Half Note**: Moderate pace
   - **Quarter Note**: Standard beat
   - **Eighth Note**: Faster, more notes
   - **Sixteenth Note**: Rapid-fire rhythms

4. Enable **Tempo Sync** for each line
5. Watch the **Beat Phase Indicator** (shows timing)

**Musical Intelligence**:
- Root and fifth notes emphasized (stronger harmony)
- Leading tones de-emphasized (smoother progressions)
- Swing parameter adds groove (adjustable 0-100%)

**Use Cases**:
- ✅ Create rhythmically coherent compositions
- ✅ Sync multiple lines to same tempo
- ✅ Generate swing/shuffle rhythms
- ✅ Maintain musical structure in chaotic visuals

---

### Microtonal Scale System

**What It Is**: Support for non-Western tuning systems and pure harmonic ratios.

**Standard Scales** (12-tone equal temperament):
- Major, Minor, Modal scales (Dorian, Phrygian, etc.)
- Pentatonic, Blues, Chromatic

**Microtonal Scales** (pure intervals):
- **Just Intonation**: Pure harmonic ratios (5/4, 3/2, etc.)
- **Pythagorean**: Based on perfect fifths
- **Quarter-Comma Meantone**: Historical temperament

**How to Use Microtonal Scales**:

1. **Select a Microtonal Scale**
   ```
   • Go to "Scale Manager" tab
   • Click [Just Intonation] or [Pythagorean]
   • Notice "Microtonal: Yes" indicator
   ```

2. **MIDI Pitch Bend Setup**
   ```
   • Ensure your synth accepts pitch bend
   • Set pitch bend range to ±2 semitones (200 cents)
   • SonifyV1 sends 14-bit pitch bend automatically
   ```

3. **Understand the Analysis Table**
   ```
   Degree | Cents  | Ratio    | MIDI Note + Bend
   ─────────────────────────────────────────────
   1      | 0.0    | 1/1      | 60 + 0¢
   2      | 204.0  | 9/8      | 62 + 4¢  (slightly sharp)
   3      | 386.3  | 5/4      | 64 - 14¢ (slightly flat)
   ```

4. **Listen to the Difference**
   ```
   • Pure intervals sound more "in tune"
   • Historical temperaments have unique character
   • Great for experimental/ambient music
   ```

**Import Custom Scales**:
1. Find or create .scl files (Scala format)
2. Place in `data/scales/` directory
3. Click "Refresh Scala Files" in Scale Manager
4. Select your custom scale

**Scala File Resources**:
- [Scala Scale Archive](http://www.huygens-fokker.org/scala/)
- Thousands of historical and experimental tunings

---

### Multi-DAW MIDI Routing

**What It Is**: Send MIDI to multiple applications simultaneously.

**Setup**:

1. **macOS MIDI Setup**
   ```
   • Open "Audio MIDI Setup" (in Applications/Utilities)
   • Window → Show MIDI Studio
   • Create IAC Bus (inter-application communication)
   ```

2. **Configure SonifyV1**
   ```
   • Go to MIDI Settings tab
   • Select "IAC Driver Bus 1" as MIDI port
   ```

3. **Connect Your DAWs**
   ```
   • In Ableton Live: MIDI From → IAC Driver
   • In Logic Pro: Create External Instrument → IAC Driver
   • In Max/MSP: midiparse → IAC Driver
   ```

4. **Per-Line Channel Routing**
   ```
   Line 1 → Channel 1 → Ableton Track 1 (Bass)
   Line 2 → Channel 2 → Logic Track 1 (Melody)
   Line 3 → Channel 3 → Max/MSP (Effects)
   ```

**Use Cases**:
- ✅ Complex multi-app setups
- ✅ Route different lines to different software
- ✅ Combine with hardware synths (use MIDI interface)

---

### Video File Playback & Analysis

**Supported Formats**: MP4, MOV, AVI (H.264 recommended)

**Playback Controls**:
- **'o'**: Open video file
- **SPACE**: Play/Pause
- **LEFT/RIGHT arrows**: Seek backward/forward
- **'l'**: Toggle loop mode
- **'v'**: Switch between video and camera

**Analysis Workflow**:

1. **Load & Preview**
   ```
   Press 'o' → Select file
   Use arrow keys to find interesting sections
   ```

2. **Draw Lines at Key Moments**
   ```
   Pause where objects cross frame
   Draw lines at crossing points
   Resume playback to test
   ```

3. **Record MIDI Output**
   ```
   • Start DAW recording
   • Press SPACE to play from beginning
   • Let video play through
   • Stop DAW recording
   ```

4. **Export for Post-Production**
   ```
   • MIDI recorded in DAW
   • Edit, quantize, humanize as needed
   • Add instruments and mix
   ```

**Tips**:
- Loop short sections for rhythmic patterns
- Use frame-by-frame seeking for precision
- Test different line placements
- Experiment with detection thresholds

---

## Musical Configuration

### Understanding MIDI Channels

**What They Are**: MIDI channels (1-16) route notes to different instruments.

**How to Use**:

```
Channel 1  → Piano
Channel 2  → Strings
Channel 3  → Bass
Channel 4  → Pads
Channel 10 → Drums (General MIDI standard)
```

**In SonifyV1**:
1. Draw multiple lines
2. Assign each line a different channel
3. In your DAW, route channels to different instruments

**Example Setup**:
```
Line 1 (Highway Lane 1) → Channel 1 → Electric Piano
Line 2 (Highway Lane 2) → Channel 2 → Strings
Line 3 (Highway Lane 3) → Channel 3 → Bass Synth
Line 4 (Crosswalk)      → Channel 10 → Kick Drum
```

---

### Octave Ranges Explained

**Musical Range**:
- **Octave 0**: Sub-bass (16-31 Hz)
- **Octave 1**: Bass (32-65 Hz)
- **Octave 2**: Low notes (65-130 Hz)
- **Octave 3**: Mid-low (130-261 Hz)
- **Octave 4**: Middle C octave (261-523 Hz)
- **Octave 5**: Mid-high (523-1046 Hz)
- **Octave 6**: High notes (1046-2093 Hz)
- **Octave 7**: Very high (2093-4186 Hz)
- **Octave 8**: Extreme high (4186+ Hz)

**Recommended Settings**:
- **Bass parts**: Octave 1-2
- **Melody**: Octave 3-5
- **High accents**: Octave 6-7
- **Drums** (if using pitched percussion): Octave 2-4

**Per-Line Configuration**:
```
Line 1: Octave 2 (bass line from trucks)
Line 2: Octave 4 (melody from cars)
Line 3: Octave 6 (high notes from bicycles)
```

---

### Velocity Mapping

**What It Is**: MIDI velocity controls note loudness/intensity (0-127).

**Standard Ranges**:
- **0-31**: Very soft (ppp)
- **32-63**: Soft (pp-p)
- **64-95**: Medium (mf)
- **96-127**: Loud (f-ff)

**Creative Uses**:

1. **Distance-Based Dynamics**
   ```
   Close lane (Line 1): Velocity 120 (loud)
   Mid lane (Line 2):   Velocity 80 (medium)
   Far lane (Line 3):   Velocity 40 (soft)
   ```

2. **Instrument-Specific Settings**
   ```
   Drums (Line 4):   Velocity 127 (max impact)
   Bass (Line 1):    Velocity 100 (strong)
   Pads (Line 2):    Velocity 60 (subtle)
   ```

3. **Dynamic Layering**
   ```
   Same line, different channels:
   Channel 1: Velocity 127 (main sound)
   Channel 2: Velocity 40 (soft layer)
   ```

---

### Note Duration Control

**What It Is**: How long each note plays (in milliseconds).

**Settings Guide**:
- **50-100ms**: Staccato, percussive
- **100-250ms**: Short, rhythmic
- **250-500ms**: Standard note length
- **500-1000ms**: Sustained, melodic
- **1000-2000ms**: Long, pad-like
- **2000+ms**: Drones, ambient

**Musical Considerations**:

**Fast Tempo (160+ BPM)**:
```
Use shorter durations (100-250ms)
Prevents note overlap and muddiness
```

**Slow Tempo (60-90 BPM)**:
```
Use longer durations (500-1000ms)
Fills space, creates atmosphere
```

**Per-Line Examples**:
```
Bass line:    500ms (sustained foundation)
Melody:       250ms (clear articulation)
Percussion:   50ms (sharp transients)
Pads:         2000ms (continuous background)
```

---

### Scale Selection Guide

**For Beginners**:
- **Major**: Happy, bright, uplifting
- **Minor**: Sad, dark, emotional
- **Pentatonic**: Safe, pleasant, can't go wrong
- **Blues**: Soulful, expressive

**For Specific Moods**:

**Mysterious/Ancient**:
```
• Phrygian (Spanish/Middle Eastern flavor)
• Locrian (unstable, eerie)
```

**Bright/Uplifting**:
```
• Major (classic happy sound)
• Lydian (dreamy, magical)
```

**Dark/Intense**:
```
• Minor (classic sad sound)
• Phrygian (dark and exotic)
```

**Jazz/Complex**:
```
• Dorian (sophisticated minor)
• Mixolydian (bluesy major)
```

**Experimental**:
```
• Chromatic (all 12 notes)
• Just Intonation (pure harmonies)
• Custom Scala scales
```

---

## Tips & Tricks

### Performance Optimization

**If frame rate is low**:

1. **Reduce Detection Classes**
   ```
   • Disable unused object categories
   • Only enable vehicle classes for highways
   • Each disabled class = faster processing
   ```

2. **Increase Frame Skip**
   ```
   • Main Controls tab → Frame Skip
   • Skip every 2-3 frames still works well
   • Detection updates less often but faster
   ```

3. **Use Smaller Model**
   ```
   • Edit config to use YOLOv8n (nano) instead of medium
   • Faster but slightly less accurate
   • File: bin/data/config.json
   ```

4. **Lower Camera Resolution**
   ```
   • 640x480 instead of 1280x720
   • Reduces processing load significantly
   ```

---

### Musical Tips

**Creating Melodies**:

1. **Use Pentatonic Scale**
   ```
   • 5-note scale - every note sounds good together
   • No "wrong" notes
   • Great for beginners
   ```

2. **Enable Tempo Sync**
   ```
   • Keeps randomization rhythmically coherent
   • Set BPM to match your track
   • Use quarter or eighth note subdivisions
   ```

3. **Layer Multiple Lines**
   ```
   Line 1: Root note octave (bass)
   Line 2: Melody octave (mid)
   Line 3: High octave (sparkle)
   All in same scale = automatic harmony!
   ```

**Creating Rhythm**:

1. **Strategic Line Placement**
   ```
   • Place lines where crossings are frequent
   • More crossings = denser rhythm
   • Fewer crossings = sparse, ambient
   ```

2. **Use Different Subdivisions**
   ```
   Line 1: Quarter notes (steady pulse)
   Line 2: Eighth notes (double-time feel)
   Line 3: Sixteenth notes (rapid patterns)
   ```

3. **Swing for Groove**
   ```
   • Set swing to 50-70%
   • Adds "shuffle" feel
   • More human, less robotic
   ```

---

### Line Drawing Techniques

**Horizontal Lines**:
```
Purpose: Detect left/right movement
Use: Highway lanes, people walking across frame
```

**Vertical Lines**:
```
Purpose: Detect up/down movement
Use: Elevators, jumping, birds flying
```

**Diagonal Lines**:
```
Purpose: Detect angled movement
Use: Stairs, slopes, creative compositions
```

**Multiple Parallel Lines**:
```
Purpose: Create rhythmic patterns
Use: Each lane triggers same note but different velocity
     Creates dynamic variation
```

**Crossing Lines (X pattern)**:
```
Purpose: Trigger multiple notes per crossing
Use: Dense harmonic textures
     Polyrhythmic complexity
```

**Circular Patterns** (draw multiple lines in circle):
```
Purpose: Objects moving in circular paths
Use: Roundabouts, rotating objects
     Creates repeating melodic loops
```

---

### Detection Threshold Tuning

**Located in**: Main Controls tab

**How It Works**:
- **Low threshold (0.3-0.5)**: Detects more objects, more false positives
- **Medium threshold (0.5-0.7)**: Balanced (default)
- **High threshold (0.7-0.9)**: Only very confident detections

**When to Adjust**:

**Missing Detections**:
```
Problem: Objects not being detected
Solution: Lower threshold to 0.4-0.5
Trade-off: May get false detections
```

**Too Many False Positives**:
```
Problem: Detecting things that aren't there
Solution: Raise threshold to 0.7-0.8
Trade-off: May miss some real objects
```

**Noisy/Low-Quality Video**:
```
Problem: Poor lighting, motion blur
Solution: Use higher threshold (0.7+)
Reason: Reduces uncertainty-based false detections
```

---

### Keyboard Shortcuts Reference

**Essential Controls**:
```
'g'          Show/hide GUI
'd'          Toggle object detection on/off
'o'          Open video file dialog
'v'          Switch between camera and video source
'r'          Restart camera connection
'c'          Clear all drawn lines
```

**Video Playback**:
```
SPACE        Play/pause video
LEFT ARROW   Seek backward
RIGHT ARROW  Seek forward
'l'          Toggle loop mode
```

**Mouse Controls**:
```
LEFT CLICK   Start drawing line
RIGHT CLICK  Finish/complete line
DRAG         Interact with GUI elements
```

---

## Troubleshooting

### Camera Issues

**Problem: "No camera detected"**

**Solutions**:
1. Check camera is plugged in (USB cameras)
2. Grant camera permissions:
   ```
   System Preferences → Security & Privacy → Camera
   Enable for SonifyV1
   ```
3. Press **'r'** to restart camera connection
4. Try different USB port
5. Restart application

**Problem: "Camera shows black screen"**

**Solutions**:
1. Check camera isn't in use by another app (Zoom, Skype, etc.)
2. Close other apps and press **'r'**
3. Check camera lens cover is removed
4. Verify camera works in Photo Booth or other app
5. Try different camera if available

---

### Detection Issues

**Problem: "Objects not being detected"**

**Solutions**:
1. **Press 'd' to enable detection**
   ```
   Must be manually enabled each session
   ```

2. **Check detection classes are enabled**
   ```
   Go to "Detection Classes" tab
   Ensure relevant classes are checked (✓)
   ```

3. **Lower confidence threshold**
   ```
   Main Controls tab → Threshold slider
   Try 0.4-0.5 for more sensitive detection
   ```

4. **Verify model loaded**
   ```
   Look for: "CoreML model loaded successfully" in logs
   If not, see "Model Loading Issues" below
   ```

5. **Improve lighting**
   ```
   Better lighting = better detection
   Avoid backlit scenes
   ```

**Problem: "False detections everywhere"**

**Solutions**:
1. Raise confidence threshold to 0.7-0.8
2. Disable irrelevant detection classes
3. Improve video quality/lighting
4. Clean camera lens

---

### MIDI Issues

**Problem: "No MIDI output appearing in DAW"**

**Solutions**:
1. **Select MIDI port in SonifyV1**
   ```
   MIDI Settings tab → MIDI Output Port dropdown
   Choose your DAW or IAC Driver
   ```

2. **Check DAW MIDI input settings**
   ```
   Ableton: Preferences → MIDI → Track Input
   Logic: Track Inspector → MIDI Input
   Ensure selected port is enabled
   ```

3. **Create IAC Bus (for inter-app MIDI)**
   ```
   macOS: Audio MIDI Setup → Show MIDI Studio
   Click IAC Driver → "Device is online" ✓
   ```

4. **Test with simple setup**
   ```
   • Draw one line
   • Enable detection
   • Cross line manually (wave hand if camera)
   • Check DAW for MIDI activity indicator
   ```

**Problem: "MIDI notes playing but wrong pitch"**

**Solutions**:
1. Check scale selection (Scale Manager tab)
2. Verify root key setting (MIDI Settings)
3. Check octave setting for line
4. If using microtonal scale:
   ```
   Ensure synth pitch bend range = ±200 cents (2 semitones)
   ```

**Problem: "MIDI notes too fast/slow"**

**Solutions**:
1. **Adjust BPM** (MIDI Settings tab)
2. **Change note subdivision**
   ```
   Faster: Use eighth or sixteenth notes
   Slower: Use quarter or half notes
   ```
3. **Disable tempo sync** if you want immediate triggers
4. **Adjust note duration** (ms) for shorter/longer notes

---

### Performance Issues

**Problem: "Low frame rate, choppy video"**

**Solutions**:

1. **Increase Frame Skip**
   ```
   Main Controls → Frame Skip: 2 or 3
   Detection runs every 2-3 frames instead of every frame
   ```

2. **Disable Unused Detection Classes**
   ```
   Detection Classes tab → Uncheck irrelevant categories
   Only enable what you need (e.g., just vehicles)
   ```

3. **Close Other Applications**
   ```
   Quit browser, video editors, other heavy apps
   Free up RAM and CPU
   ```

4. **Lower Video Resolution**
   ```
   Use 640x480 camera setting if possible
   Smaller resolution = faster processing
   ```

5. **Switch to Smaller Model** (advanced)
   ```
   Edit bin/data/config.json
   Change model path to yolov8n.mlpackage (nano)
   Restart application
   ```

**Problem: "App crashes or freezes"**

**Solutions**:
1. Check macOS version (needs 11.0+)
2. Restart computer
3. Run from Applications folder (not from DMG)
4. Check Console.app for crash logs
5. Reduce number of drawn lines
6. Disable video file and use camera only

---

### Video File Issues

**Problem: "Video file won't load"**

**Solutions**:
1. **Verify file format**
   ```
   Supported: MP4, MOV, AVI
   Recommended: H.264 codec
   ```

2. **Try converting video**
   ```
   Use HandBrake or FFmpeg to convert to:
   MP4 (H.264 video, AAC audio)
   Resolution: 1920x1080 or lower
   ```

3. **Check file isn't corrupted**
   ```
   Try playing in VLC or QuickTime
   If those work, file is OK
   ```

4. **Reduce file size**
   ```
   Very large files (>2GB) may cause issues
   Split into smaller clips
   ```

**Problem: "Video plays but no detection"**

**Solutions**:
1. Press **'d'** to enable detection
2. Check detection classes enabled
3. Try increasing video brightness/contrast
4. Object might be too small - use closer footage

---

### Audio/Sound Issues

**Problem: "No sound coming out"**

**Important**: SonifyV1 only sends MIDI, not audio!

**To hear sound, you need**:
1. MIDI-compatible software (DAW, synth)
2. Software instrument loaded
3. MIDI routing configured
4. Audio interface/speakers connected

**Setup checklist**:
```
☑ SonifyV1 sending MIDI (check port selection)
☑ DAW receiving MIDI (check input monitoring)
☑ Instrument loaded on track (synth, sampler, etc.)
☑ Track armed/monitoring enabled
☑ DAW audio output to speakers/headphones
```

---

## Workflow Examples

### Example 1: Highway Time-Lapse Composition

**Goal**: Create an evolving ambient piece from highway time-lapse.

**Setup**:
```
Video: 5-minute highway time-lapse (sped up 10x)
Style: Ambient, atmospheric
Tempo: Slow (60 BPM)
Scale: Lydian (dreamy, floating quality)
```

**Steps**:

1. **Load time-lapse video** (press 'o')

2. **Draw 4 horizontal lines** across lanes:
   ```
   Line 1: Far left lane
   Line 2: Center-left lane
   Line 3: Center-right lane
   Line 4: Far right lane
   ```

3. **Configure each line**:
   ```
   Line 1: Channel 1, Octave 2, Velocity 40,  Duration 2000ms (bass drone)
   Line 2: Channel 2, Octave 3, Velocity 60,  Duration 1500ms (low pad)
   Line 3: Channel 3, Octave 5, Velocity 80,  Duration 1000ms (mid melody)
   Line 4: Channel 4, Octave 6, Velocity 100, Duration 500ms  (high sparkle)
   ```

4. **Musical settings**:
   ```
   BPM: 60
   Root: C
   Scale: Lydian
   Subdivision: Quarter notes
   Swing: 0% (straight time)
   Tempo sync: Enabled for all lines
   ```

5. **DAW setup**:
   ```
   Channel 1 → Sub bass synth (filter cutoff low)
   Channel 2 → Warm pad (reverb heavy)
   Channel 3 → Melodic lead (delay, reverb)
   Channel 4 → Soft bells/chimes (short reverb)
   ```

6. **Record**:
   ```
   • Start DAW recording
   • Press SPACE to play video
   • Press 'd' to enable detection
   • Let play through
   • Stop recording
   ```

7. **Post-production**:
   ```
   • Quantize to quarter notes (optional)
   • Add automation (filter sweeps, volume swells)
   • Layer with field recordings
   • Master with compression and limiting
   ```

**Result**: Evolving ambient soundscape driven by traffic flow patterns.

---

### Example 2: Rhythmic Percussion from Pedestrian Crossing

**Goal**: Generate polyrhythmic percussion patterns from people crossing street.

**Setup**:
```
Video: Busy crosswalk footage
Style: Rhythmic, percussive
Tempo: Medium-fast (135 BPM)
Scale: Chromatic (percussion doesn't need tonality)
```

**Steps**:

1. **Camera position**: Overhead view of crosswalk

2. **Draw 6 lines** across different crossing paths:
   ```
   Lines 1-3: Horizontal (left to right movement)
   Lines 4-6: Vertical (top to bottom movement)
   ```

3. **Configure as percussion triggers**:
   ```
   Line 1: Ch 10, Note C1  (Kick),     Velocity 127, Duration 50ms
   Line 2: Ch 10, Note D1  (Snare),    Velocity 110, Duration 60ms
   Line 3: Ch 10, Note F#1 (Hi-hat),   Velocity 90,  Duration 40ms
   Line 4: Ch 10, Note C#1 (Rimshot),  Velocity 100, Duration 45ms
   Line 5: Ch 10, Note E1  (Clap),     Velocity 95,  Duration 55ms
   Line 6: Ch 10, Note G#1 (Cowbell),  Velocity 105, Duration 50ms
   ```

4. **Musical settings**:
   ```
   BPM: 135
   Scale: Chromatic (all notes available)
   Subdivision: Sixteenth notes (rapid triggers)
   Swing: 65% (groove feel)
   Tempo sync: Enabled
   ```

5. **DAW setup**:
   ```
   Channel 10 → Drum sampler or drum machine
   Map MIDI notes to drum sounds
   Add compression for punch
   Add slight reverb for space
   ```

6. **Performance tips**:
   ```
   • Busier crossings = denser rhythms
   • Lulls in traffic = breakdown sections
   • Rush hour = intense polyrhythms
   ```

**Result**: Organic, ever-changing rhythmic patterns from real-world movement.

---

### Example 3: Interactive Installation

**Goal**: Museum/gallery installation where visitors create music by moving.

**Setup**:
```
Camera: Wide-angle, ceiling-mounted
Space: 10x10 foot interaction area
Style: Generative, interactive
```

**Steps**:

1. **Camera setup**:
   ```
   Mount camera overhead pointing down
   Cover entire interaction area
   Good lighting essential
   ```

2. **Draw grid of lines**:
   ```
   Create 3x3 grid (9 intersection points)
   Each line segment = different musical parameter
   Horizontal lines: Bass notes
   Vertical lines: Melody notes
   ```

3. **Configure zones**:
   ```
   Zone 1 (back left):    Dark, low sounds
   Zone 2 (back center):  Mid-range
   Zone 3 (back right):   Bright, high sounds

   Movement through zones creates musical journey
   ```

4. **Musical settings**:
   ```
   BPM: 90 (moderate, contemplative)
   Scale: Pentatonic (always harmonious)
   Root: Changes every 30 seconds (automated in DAW)
   Long durations: 1000-2000ms (sustained tones)
   ```

5. **Detection settings**:
   ```
   Enable only: Person class
   Disable all other classes
   Medium-low threshold (catch all visitors)
   ```

6. **Sound design**:
   ```
   Use evolving pads and atmospheric sounds
   Each crossing adds layer to sonic texture
   Reverb and delay for spatial depth
   No harsh sounds - contemplative atmosphere
   ```

7. **Visitor instructions** (signage):
   ```
   "Move through the space to create music.
    Your movement becomes sound.
    Fast movements = rapid notes.
    Slow movements = sustained tones.
    Explore the sonic landscape."
   ```

**Result**: Interactive sound installation responding to visitor movement.

---

### Example 4: Generative Live Coding Performance

**Goal**: Combine with live coding (TidalCycles, Sonic Pi) for hybrid performance.

**Setup**:
```
SonifyV1: Provides generative melodic material
Live Coding: Adds rhythm, bass, effects, processing
Style: Electronic, experimental
```

**Steps**:

1. **SonifyV1 configuration**:
   ```
   Camera: Abstract visuals (lava lamp, projection mapping)
   3 lines: Minimal placement for sparse melodies
   Scale: Just Intonation (microtonal interest)
   Tempo: 140 BPM (matching live code tempo)
   MIDI out: IAC Bus to live coding environment
   ```

2. **Per-line setup**:
   ```
   Line 1: Ch 1, Octave 5, Velocity 80 (lead melody)
   Line 2: Ch 2, Octave 4, Velocity 70 (harmony)
   Line 3: Ch 3, Octave 6, Velocity 90 (high accents)
   ```

3. **TidalCycles integration** (example):
   ```haskell
   -- Receive MIDI from SonifyV1
   d1 $ note (scale "just" $ "0 2 4 6")
      # sound "superpiano"
      # room 0.8
      # sz 0.6
      |+ note (range (-7) 7 $ slow 16 $ saw)

   -- Add rhythmic elements
   d2 $ sound "bd*4 sd*2 [~ bd] sd"
      # gain 1.2
   ```

4. **Sonic Pi integration** (example):
   ```ruby
   # Receive MIDI from SonifyV1
   live_loop :midi_listener do
     note, velocity = sync "/midi/iac_driver/0/1/note_on"
     synth :prophet, note: note, amp: velocity/127.0,
           cutoff: rrand(60, 120), release: 2
   end

   # Add bass line
   live_loop :bass do
     use_synth :fm
     play :c2, release: 0.5
     sleep 0.5
   end
   ```

5. **Performance workflow**:
   ```
   • SonifyV1 provides melodic seeds
   • Live code responds and elaborates
   • Gradually build complexity
   • Tear down to minimal
   • SonifyV1 as consistent thread throughout
   ```

6. **Visuals**:
   ```
   Project SonifyV1 video output on screen
   Show detections and line crossings
   Audience sees visual-audio connection
   ```

**Result**: Hybrid generative + live-coded electronic performance.

---

## Advanced Topics

### Scale Degree Weighting Explained

**What It Does**: Makes certain scale notes more likely in randomization.

**Default Weights**:
```
Root (1st degree):      200% (emphasized)
Fifth (5th degree):     150% (emphasized)
Leading tone (7th):     50%  (de-emphasized)
All other degrees:      100% (normal)
```

**Why This Matters**:
- Root and fifth are harmonically stable
- Creates more consonant, pleasant randomization
- Leading tones (7th degree) create tension
- Reducing 7th occurrence = smoother progressions

**Customizing Weights** (requires config.json editing):
```json
"scaleDegreeWeights": {
  "1": 2.0,    // Root - very common
  "2": 1.0,    // Second - normal
  "3": 1.2,    // Third - slightly emphasized
  "4": 1.0,    // Fourth - normal
  "5": 1.5,    // Fifth - emphasized
  "6": 1.0,    // Sixth - normal
  "7": 0.5     // Seventh - de-emphasized
}
```

**Creative Uses**:
- All weights = 1.0 (completely random)
- Emphasize 1, 3, 5 (triadic harmony)
- Emphasize 1, 4, 5 (strong harmonic anchors)

---

### Configuration File Deep Dive

**Location**: `bin/data/config.json`

**Key Sections**:

```json
{
  "lines": [
    {
      "startX": 100,
      "startY": 200,
      "endX": 500,
      "endY": 200,
      "midiChannel": 1,
      "octave": 4,
      "duration": 250,
      "velocity": 100,
      "tempoSyncEnabled": true
    }
  ],

  "tempo": {
    "bpm": 120.0,
    "subdivision": 8,    // 8 = eighth note
    "swing": 0.5         // 0.5 = 50%
  },

  "scale": {
    "currentScale": "Major",
    "rootKey": 60,       // MIDI note (60 = C4)
    "microtonalityEnabled": true
  },

  "midi": {
    "portName": "IAC Driver Bus 1",
    "globalChannel": 1
  },

  "detection": {
    "enabledClasses": ["car", "truck", "bus"],
    "confidence": 0.6,
    "frameSkip": 1
  }
}
```

**Manual Editing**:
1. Close SonifyV1
2. Edit `bin/data/config.json`
3. Save file
4. Relaunch SonifyV1 (loads new settings)

**Backup Important**:
```bash
# Before editing, backup:
cp bin/data/config.json bin/data/config.backup.json
```

---

### Creating Custom Scala Scales

**Scala Format Basics**:

```
! my_custom_scale.scl
!
My Custom 7-note Scale
 7
!
 200.0    cents for note 2
 400.0    cents for note 3
 500.0    cents for note 4
 700.0    cents for note 5
 900.0    cents for note 6
 1100.0   cents for note 7
 1200.0   octave (always 1200.0)
```

**Step-by-Step**:

1. **Create text file**: `my_scale.scl`

2. **Add header**:
   ```
   ! my_scale.scl
   !
   My Scale Description
   7
   !
   ```

3. **Add intervals** (in cents):
   ```
   0 cents is root
   1200 cents = octave
   100 cents = 1 semitone
   ```

4. **Save to**: `bin/data/scales/my_scale.scl`

5. **Refresh in SonifyV1**:
   ```
   Scale Manager tab → Refresh Scala Files
   ```

**Example: Bohlen-Pierce Scale** (13-note, 3:1 octave):
```
! bohlen_pierce.scl
!
Bohlen-Pierce 13-note equal-tempered scale
13
!
 92.308
 184.615
 276.923
 369.231
 461.538
 553.846
 646.154
 738.462
 830.769
 923.077
 1015.385
 1107.692
 1200.0
```

**Resources**:
- [Scala Homepage](http://www.huygens-fokker.org/scala/)
- [Scala Archive](http://www.huygens-fokker.org/scala/downloads.html#scales)
- Thousands of historical and experimental scales

---

## Getting Help

### Resources

**Documentation**:
- `README.txt` - Installation and quick start
- `QUICK_START.txt` - 60-second tutorial
- `CLAUDE.md` - Complete technical documentation
- `HOW_TO_USE.md` - This guide
- `DISTRIBUTION.md` - Distribution technical details

**Community**:
- openFrameworks Forum: [forum.openframeworks.cc](https://forum.openframeworks.cc)
- MIDI/Music Tech: Kvraudio, Lines forum
- Scala/Microtonality: Xenharmonic Alliance

**Technical Support**:
- Check Console.app for error messages
- Search openFrameworks forum for similar issues
- GitHub issues for bug reports

---

### Reporting Issues

**Before reporting**:
1. Check Troubleshooting section above
2. Verify macOS version (11.0+ required)
3. Test with fresh config (delete `config.json`)
4. Try running from Applications folder

**What to include**:
```
• macOS version
• SonifyV1 version
• Steps to reproduce
• Console.app logs (if crash)
• Screenshots of issue
• Config.json (if configuration issue)
```

---

## Appendix

### MIDI Note Numbers Reference

**Common Ranges**:
```
C-1  = 0      (8.18 Hz, sub-audio)
C0   = 12     (16.35 Hz, sub-bass)
C1   = 24     (32.70 Hz, bass)
C2   = 36     (65.41 Hz, bass)
C3   = 48     (130.81 Hz, low)
C4   = 60     (261.63 Hz, Middle C) ← DEFAULT ROOT
C5   = 72     (523.25 Hz, mid)
C6   = 84     (1046.50 Hz, high)
C7   = 96     (2093.00 Hz, very high)
C8   = 108    (4186.01 Hz, extreme high)
G9   = 127    (12543.85 Hz, MIDI max)
```

---

### Scale Intervals in Cents

**12-Tone Equal Temperament**:
```
Unison:         0 cents
Minor 2nd:      100 cents (semitone)
Major 2nd:      200 cents (whole tone)
Minor 3rd:      300 cents
Major 3rd:      400 cents
Perfect 4th:    500 cents
Tritone:        600 cents
Perfect 5th:    700 cents
Minor 6th:      800 cents
Major 6th:      900 cents
Minor 7th:      1000 cents
Major 7th:      1100 cents
Octave:         1200 cents
```

**Just Intonation (Pure Ratios)**:
```
Unison:         1/1     (0 cents)
Major 2nd:      9/8     (204 cents)
Major 3rd:      5/4     (386 cents)
Perfect 4th:    4/3     (498 cents)
Perfect 5th:    3/2     (702 cents)
Major 6th:      5/3     (884 cents)
Major 7th:      15/8    (1088 cents)
Octave:         2/1     (1200 cents)
```

---

### File Format Support

**Video Input**:
```
✅ MP4 (H.264/H.265)
✅ MOV (QuickTime)
✅ AVI (older codec support)
⚠️  WebM (limited support)
❌ MKV (not supported)
```

**Recommended Video Specs**:
```
Codec:      H.264
Resolution: 1920x1080 or lower
Framerate:  24-30 fps
Bitrate:    5-10 Mbps
```

**Scale Files**:
```
✅ .scl (Scala format)
❌ .tun (not supported, convert to .scl)
```

---

## Quick Reference Card

### Essential Shortcuts
```
'g'    Show/Hide GUI           'd'    Toggle Detection
'o'    Open Video             'v'    Switch Camera/Video
'c'    Clear Lines            'r'    Restart Camera
SPACE  Play/Pause             ←→     Seek Video
```

### Quick Setup Checklist
```
☐ Launch app, press 'g' for GUI
☐ Press 'd' to enable detection
☐ Select MIDI port (MIDI Settings tab)
☐ Choose scale and BPM
☐ Draw line (LEFT click → RIGHT click)
☐ Configure line settings
☐ Start creating music!
```

### Common Settings
```
Highway Sonification:    120 BPM, Minor scale, Quarter notes
Ambient Composition:     60 BPM, Lydian scale, Long durations
Rhythmic Percussion:     135 BPM, Chromatic, Sixteenth notes
Interactive Installation: 90 BPM, Pentatonic, Medium durations
```

---

**🎵 Now go create some amazing music with computer vision! 🚗📹**

---

*SonifyV1 v2.0 - How To Use Guide*
*Last Updated: 2025-10-15*
*For latest documentation, see CLAUDE.md*
