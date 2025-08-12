# SonifyV1 - Complete System Launcher

## Quick Start (One Command)

```bash
cd /Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1
./run_sonify.sh
```

## What This Does

The `run_sonify.sh` script automatically:
1. **Checks Prerequisites** - Verifies Python, MediaPipe, and openFrameworks app
2. **Starts MediaPipe Server** - Launches pose detection with camera access
3. **Starts SonifyV1 App** - Opens the main openFrameworks application
4. **Monitors System** - Watches both processes and handles shutdown
5. **Provides Status** - Shows real-time system information

## System Features

### Dual Detection System
- **CoreML YOLOv8**: Detects vehicles, people, animals, objects (80 COCO classes)
- **MediaPipe Pose**: Detects human pose landmarks (33 joints per person)
- **Optimized Resolution**: Both systems use 640x640 for perfect CoreML compatibility
- **Visual Feedback**: Real-time skeleton drawing with color-coded people

### Musical Output
- **Line Crossing Events**: Both detection systems trigger musical events
- **MIDI Output**: Real-time MIDI notes sent to DAW/synthesizers
- **OSC Output**: Rich data for creative coding applications

### User Interface
- **4-Tab GUI**: Main Controls, MIDI Settings, Detection Classes, Pose Detection
- **Real-time Feedback**: Live statistics and connection status
- **Visual Line Drawing**: Interactive line placement for musical triggers

## Usage Instructions

1. **Launch System**: Run `./run_sonify.sh`
2. **Wait for "SYSTEM READY"** message
3. **SonifyV1 window opens automatically**
4. **Enable Pose Detection**:
   - Click "Pose Detection" tab
   - Check "Enable Pose Detection" 
   - Status should show "Connected" (green)
5. **Draw Lines**: Click and drag in video area for musical zones
6. **Make Music**: Move in camera view or drive vehicles for sound triggers

## Status Monitoring

Check system status anytime:
```bash
./check_status.sh
```

Shows:
- Process status (running/stopped)
- CPU usage
- UDP connection status
- Recent log activity

## Shutdown

Press **Ctrl+C** in the terminal running the script. This cleanly shuts down both processes.

## Troubleshooting

### Camera Issues
- Grant camera permission in macOS System Preferences
- Close other applications using the camera (Zoom, FaceTime, etc.)

### Port Issues
- If port 8080 is busy, kill conflicting processes:
  ```bash
  lsof -ti:8080 | xargs kill
  ```

### Python Issues
- Ensure miniconda3 is installed at `~/miniconda3/`
- MediaPipe requires Python 3.9-3.12 (script uses 3.10)

### Build Issues
- Recompile if needed: `make clean && make`

## Log Files

- **Pose Server**: `bin/pose_server.log`
- **OpenFrameworks**: `bin/of_app.log`

## Advanced Usage

### Manual Launch (separate terminals)

**Terminal 1 - MediaPipe Server**:
```bash
~/miniconda3/bin/python bin/mediapipe_pose_server.py --confidence 0.3
```

**Terminal 2 - SonifyV1 App**:
```bash
./bin/SonifyV1.app/Contents/MacOS/SonifyV1
```

### Configuration Options

Edit `run_sonify.sh` to modify:
- `POSE_CONFIDENCE=0.3` - Detection sensitivity (0.1-0.9)
- `UDP_PORT=8080` - Communication port
- Camera ID and other MediaPipe options

---

**System tested and operational on macOS with Apple Silicon M1/M2**