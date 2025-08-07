# SonifyV1 - Implementation Progress

## Project Overview
SonifyML Highway sonification system built with openFrameworks. Converts visual highway scenes into real-time audio feedback using YOLO object detection and spatial audio mapping.

## Implementation Status

### ✅ Completed Tasks

#### Task 1.1: Basic OpenFrameworks Setup
- **Status**: COMPLETE
- **Features**: 
  - Window size: 1024x768
  - Green background
  - 60fps application framerate
  - FPS counter display
- **Files Modified**: `src/main.cpp`, `src/ofApp.cpp`, `src/ofApp.h`

#### Task 1.2: Video Capture System
- **Status**: COMPLETE  
- **Features**:
  - Camera support (30fps max - hardware limitation)
  - Video file playback support (.mp4, .mov, .avi, etc.)
  - File dialog for video selection (press 'o')
  - Toggle between camera/video (press 'v')
  - Camera restart functionality (press 'r')
  - Muted audio playback
  - Video transport controls:
    - SPACE: Play/pause
    - LEFT/RIGHT arrows: Seek backward/forward (5% increments)
    - 'l': Toggle loop mode
- **Files Modified**: `src/ofApp.h`, `src/ofApp.cpp`

#### Task 1.3: Mouse Zone Drawing  
- **Status**: COMPLETE
- **Features**:
  - Interactive polygon drawing with mouse clicks
  - Left-click to add points, right-click to finish polygon
  - Multiple polygons with different colors (red, blue, green, orange, purple, cyan, yellow, magenta)
  - 'c' key to clear all polygons
  - Real-time visual feedback with polygon preview
  - Zone numbering (Z1, Z2, Z3...) displayed at polygon centers
  - On-screen polygon and point count display
  - Minimum 3 points required to complete polygon
- **Files Modified**: `src/ofApp.h`, `src/ofApp.cpp`

#### Task 1.4: ONNX YOLO Object Detection ❌ ISSUES IDENTIFIED → CoreML Migration
- **ONNX Status**: 🚨 **CRITICAL ISSUES** - Migrating to CoreML
- **Issues with ONNX Implementation**:
  - **Aspect Ratio Distortion**: Force-stretching video (1880x720 → 640x640) breaks detection
  - **Poor Detection Quality**: Only detects in top-half, misclassifies non-vehicles
  - **Sub-optimal M1 Performance**: No Metal Performance Shaders utilization
  - **Coordinate Mapping Problems**: Incorrect bounding box alignment
  - **Repetitive Detections**: Same coordinates across frames indicate pipeline issues
- **CoreML Migration**: 🔄 **IN PROGRESS**
  - **Native M1 Optimization**: 3-5x performance improvement expected
  - **Automatic Preprocessing**: Eliminates aspect ratio distortion issues  
  - **Apple Neural Engine**: Dedicated ML hardware utilization
  - **Better Power Efficiency**: Lower consumption than ONNX Runtime
- **Migration Status**:
  - ✅ Updated headers to use CoreML/Vision frameworks
  - ✅ Modified build system (config.make) for Apple frameworks  
  - ✅ Created model conversion script (convert_to_coreml.py)
  - 🔄 Implementing CoreML inference pipeline
  - ⏳ Model format conversion (ONNX → CoreML)
- **Files Modified**: `src/ofApp.h`, `src/ofApp.cpp`, `config.make`, `convert_to_coreml.py`
- **New Dependencies**: CoreML, Vision, Foundation (native Apple frameworks)

### 📋 Pending Tasks
- Task 1.4b: Complete CoreML Migration (IN PROGRESS)
- Task 1.5: Audio Sonification System  
- Task 1.6: Real-time Performance Optimization (Enhanced with CoreML)
- Task 1.7: GUI Controls
- Task 1.8: Configuration System
- Task 1.9: Testing & Validation
- Task 1.10: Final Integration

## File Structure
```
SonifyV1/
├── src/
│   ├── main.cpp          # Application entry point, window setup
│   ├── ofApp.h           # Main application class definition
│   └── ofApp.cpp         # Main application implementation
├── bin/
│   └── SonifyV1.app      # Compiled application
├── addons.make           # Required addons list
├── Makefile              # Build configuration
└── CLAUDE.md            # This documentation file
```

## Dependencies & Addons
- **ofxImGui**: GUI framework
- **ofxJSON**: JSON configuration handling  
- **ofxOpenCv**: Computer vision operations
- **ofxOsc**: OSC communication
- **CoreML**: Apple's native ML framework (replacing ONNX Runtime)
- **Vision**: Apple's computer vision framework
- **Foundation**: Apple's foundation framework

## Build & Run Commands

### Compile
```bash
cd /Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1
make
```

### Run
```bash
cd bin
DYLD_LIBRARY_PATH=. ./SonifyV1.app/Contents/MacOS/SonifyV1
```

### Clean Build
```bash
make clean
make
```

## Controls Reference

### Video/Camera Controls
- **'o'**: Open file dialog to select video file
- **'v'**: Toggle between video file and camera
- **'r'**: Restart camera connection

### Video Transport Controls  
- **SPACE**: Play/pause video (stop/resume functionality)
- **LEFT ARROW**: Seek backward (10% increments, works when paused)
- **RIGHT ARROW**: Seek forward (10% increments, works when paused)
- **'l'**: Toggle loop mode on/off

### Polygon Drawing Controls
- **LEFT CLICK**: Add point to current polygon
- **RIGHT CLICK**: Finish current polygon and start new one
- **'c'**: Clear all polygons

### YOLO Detection Controls
- **'d'**: Toggle YOLO vehicle detection on/off

## Technical Implementation Notes

### Video System Architecture
- **Dual Source Support**: Seamless switching between live camera and video files
- **Hardware Optimization**: Camera runs at native 30fps, app at 60fps for UI responsiveness
- **Audio Management**: Video audio automatically muted to prevent interference
- **Error Handling**: Graceful fallback between video sources

### Performance Characteristics
- **Camera**: 15-30fps (hardware dependent)
- **Video Files**: Native framerate (typically 24-30fps)
- **Application**: 60fps for smooth UI interactions
- **Resolution**: Supports various input resolutions, scales to 1024x768 display

### File Format Support
- **Video**: MP4, MOV, AVI, and other formats supported by AVFoundation
- **Optimal**: H.264 encoded MP4 files for best performance

## Development Log

### 2025-08-05: Complete Video System + Polygon Drawing + YOLO Detection
- ✅ **Task 1.1**: Basic OpenFrameworks setup with 60fps and green background
- ✅ **Task 1.2**: Dual video source system (camera + file) with transport controls
- ✅ **Task 1.3**: Interactive polygon drawing system with multi-color zones
- ✅ **Task 1.4**: ONNX Runtime YOLO detection with vehicle filtering

#### Major Technical Achievements:
- **Video System**: Seamless camera/video switching with native file dialogs
- **Polygon System**: Real-time drawing with visual feedback and zone numbering
- **YOLO Integration**: Successfully replaced TensorFlow with ONNX Runtime
- **Build System**: Resolved complex library path and JSON amalgamation issues
- **Performance**: Optimized frame skipping for real-time detection (~20fps)

#### Critical Problem Solving:
- **TensorFlow → ONNX**: User insight led to correct model format choice
- **JSON Conflicts**: Fixed ofxJSON amalgamation with `JSON_IS_AMALGAMATION`
- **OpenMP Paths**: Resolved `/opt/homebrew/opt/libomp/lib` library linking
- **Vehicle Filtering**: Implemented COCO class filtering for vehicles only

### 2025-08-06: CoreML Migration Initiated
- 🚨 **ONNX Issues Identified**: Critical problems with aspect ratio, performance, and detection quality
- ✅ **Repository Setup**: Pushed ONNX baseline to GitHub with detailed issue documentation
- 🔄 **CoreML Migration Started**: Updated headers, build system, and core infrastructure
- 📊 **Expected Improvements**: 3-5x performance gain, better preprocessing, M1 optimization

#### Key Changes Made:
- **Headers**: Replaced ONNX Runtime with CoreML/Vision frameworks
- **Build System**: Updated config.make for Apple native frameworks  
- **Architecture**: Maintained OpenFrameworks structure, enhanced with CoreML
- **Conversion Tool**: Created Python script for ONNX → CoreML model conversion

### Next Session Goals
- 🎯 **Task 1.4b**: Complete CoreML implementation and test performance
- 🎯 **Task 1.5**: Audio Sonification System - map detections to spatial audio
- 🎯 **Task 1.6**: Performance validation (CoreML vs ONNX benchmarks)

## Known Issues & Solutions

### Issue: ONNX Runtime Library Path  
**Problem**: `dyld: Library not loaded` errors with ONNX Runtime
**Solution**: ONNX Runtime installed via Homebrew at `/opt/homebrew/lib`, configured in `config.make`

### Issue: Camera Flickering
**Problem**: Framerate mismatch causing video flicker
**Solution**: Synchronized camera (30fps) and app (60fps) framerates

### Issue: Video File Selection
**Problem**: No way to choose video files dynamically
**Solution**: ✅ Implemented native file dialog with 'o' key

## Reference Files
- **Implementation Guide**: `sonifyv2_implementation_guide.md`
- **Test Environment**: `test_environment.py`
- **Main Application**: `src/ofApp.cpp` (236 lines)
- **Header File**: `src/ofApp.h` (36 lines)

---
*Last Updated: 2025-08-06*
*Current Status: Tasks 1.1-1.3 Complete, Task 1.4 Migrating ONNX→CoreML - Critical Performance Issues Addressed*

## Memories
- `memorize` added as a placeholder memory for future tracking