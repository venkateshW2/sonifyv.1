# SonifyV1 - Production Highway Sonification System

## Overview
Production-ready highway sonification system that converts visual highway scenes into spatial audio feedback using CoreML YOLO object detection, interactive line crossing zones, and MIDI/OSC communication.

## ✅ System Status: FULLY OPERATIONAL

### Core Features
- **Real-time Object Detection**: CoreML YOLOv8 with all 80 COCO classes
- **Interactive Line Drawing**: Mouse-based line creation with line crossing detection
- **Professional GUI**: Tabbed ImGui interface (Main Controls + MIDI + Detection Classes)
- **MIDI Output**: Multi-port MIDI with musical scale system and line-based instruments
- **OSC Communication**: Real-time messaging to Max/MSP or other audio software
- **Video Input**: Camera + video file support with seamless switching
- **Configuration**: Complete JSON save/load system for all application state

## Quick Start

### Build and Run
```bash
make clean && make
./bin/SonifyV1.app/Contents/MacOS/SonifyV1
```

### Essential Controls
- **'g'**: Toggle GUI interface
- **'d'**: Toggle object detection
- **'o'**: Open video file
- **'v'**: Switch between camera and video
- **'c'**: Clear all lines
- **SPACE**: Video play/pause
- **'t'**: Test MIDI output

### Drawing Lines
- **Left Click**: Start drawing line
- **Right Click**: Finish line
- **Mouse Drag**: Move line endpoints (when selected)
- **Delete Key**: Remove selected line

## System Architecture

### Core Managers
- **VideoManager**: Camera and video file handling
- **DetectionManager**: CoreML YOLO object detection
- **LineManager**: Interactive line drawing and crossing detection
- **CommunicationManager**: MIDI and OSC output
- **UIManager**: Professional ImGui interface
- **ConfigManager**: JSON configuration persistence

### File Structure
```
SonifyV1/
├── src/                    # Core application source
│   ├── main.cpp           # Application entry point
│   ├── ofApp.h/.cpp       # Main application class
│   ├── *Manager.h/.cpp    # Core system managers
│   └── CoreMLDetector.*   # YOLO detection interface
├── bin/
│   ├── data/
│   │   ├── models/        # CoreML YOLO models
│   │   └── config.json    # Application configuration
│   └── SonifyV1.app       # Compiled application
├── obj/                   # Build files
├── CLAUDE.md             # Detailed technical documentation
├── Makefile              # Build configuration
├── config.make           # openFrameworks configuration
├── addons.make           # Required addons list
├── Project.xcconfig      # Xcode project settings
├── openFrameworks-Info.plist
├── of.entitlements       # macOS permissions
└── trashed/              # Removed development files
```

## Configuration

### Detection Classes
The system supports all 80 COCO classes organized in categories:
- **Vehicles**: car, truck, bus, motorcycle, bicycle, airplane, train, boat
- **People**: person
- **Animals**: bird, cat, dog, horse, sheep, cow, elephant, bear, zebra, giraffe
- **Objects**: Various everyday objects, furniture, electronics, food items

### MIDI System
- **Multi-Port Output**: Connects to available MIDI ports (IAC Driver, Network, etc.)
- **Musical Scale System**: Global root key and scale (Major, Minor, Pentatonic, Blues, Chromatic)
- **Per-Line Properties**: Each line acts as a musical instrument with individual settings
- **Duration Mapping**: Vehicle-type based note lengths (truck=750ms, car=250ms, motorcycle=150ms)

### OSC Communication
- **Default**: 127.0.0.1:12000
- **Message Format**: Rich line crossing data for Max/MSP integration
- **Real-time**: Sub-10ms latency from detection to message

## Performance
- **Detection**: 60fps real-time processing with CoreML optimization
- **Camera**: HD 1280x720 at 30fps
- **Memory**: Stable operation during extended sessions
- **CPU**: Optimized for Apple Silicon (M1/M2)

## Requirements
- **Platform**: macOS with Apple Silicon recommended
- **Camera**: USB webcam or built-in camera
- **Models**: YOLOv8 CoreML models (auto-loaded from bin/data/models/)
- **Audio**: Max/MSP or compatible OSC/MIDI receiver

## Development Notes

### Clean Architecture
This version represents the stable, production-ready SonifyV1 system with:
- ✅ All pose detection components cleanly removed
- ✅ No camera access conflicts
- ✅ Full MIDI and OSC functionality operational
- ✅ Professional tabbed GUI interface
- ✅ Complete configuration system
- ✅ Robust error handling and logging

### Removed Components
Development files moved to `trashed/` folder:
- Pose detection source files and headers
- Development documentation and guides
- Python conversion scripts and test files
- Log files and temporary build artifacts
- Xcode project backups and workspace files

### Related Projects
- **SonifyPose**: Separate pose detection application (camera conflicts resolved)
- **MediaPipe Integration**: Advanced multi-modal detection (future enhancement)

## Troubleshooting

### Common Issues
1. **No video**: Press 'o' to load video file or 'r' to restart camera
2. **No MIDI output**: Check MIDI port connections in MIDI tab
3. **Performance issues**: Reduce detection frame skip in Main Controls tab
4. **Line drawing issues**: Press 'c' to clear all lines and restart

### Logging
Check console output for detailed system status and error messages.

---

**Version**: Production Stable (August 2024)  
**Status**: ✅ Fully Operational  
**Platform**: openFrameworks + CoreML + MIDI/OSC