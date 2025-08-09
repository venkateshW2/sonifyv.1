# IP Camera Integration Task Implementation Guide

## Overview
This document outlines the complete implementation plan for integrating Android IP camera support into the SonifyV1 application without breaking existing functionality.

## Android IP Camera App Recommendations

### Primary Recommendation: IP Webcam
**App Name:** 
**Package:** com.pas.webcam  
**Download:** Google Play Store / F-Droid  
**Cost:** Free with ads, Pro version available  

**Key Features:**
- MJPEG and H.264 streaming support
- HTTP/RTSP/RTMP protocols
- Multiple resolution options (up to 4K)
- Audio streaming capability
- Motion detection
- Web interface for configuration
- Battery optimization settings

**Default URL Format:** `http://[phone-ip]:8080/video`

### Alternative Options
1. **DroidCam** - Free tier, good quality, requires client software
2. **Alfred Home Security** - Free with ads, cloud-based
3. **AtHome Camera** - Free version, basic IP streaming
4. **Open Camera** - Open source, requires additional setup

## Technical Implementation Plan

### Phase 1: Core Infrastructure (2-3 hours)
**Goal:** Extend video input system to support network streaming

#### Tasks:
- [ ] Add `enum VideoSource { CAMERA, VIDEO_FILE, IP_CAMERA }`
- [ ] Add `ofVideoPlayer ipCameraPlayer` for network streaming
- [ ] Add IP camera configuration variables:
  - `string ipCameraUrl`
  - `bool ipCameraConnected`
  - `int connectionRetries`
  - `int connectionTimeout`
- [ ] Add connection status tracking
- [ ] Add error handling for network issues

**Success Criteria:**
- New enum compiles without errors
- Variables properly initialized in constructor
- No impact on existing camera/video functionality

### Phase 2: Network Streaming Integration (2-3 hours)
**Goal:** Implement IP camera connection and streaming

#### Tasks:
- [ ] Create `connectToIPCamera(string url)` method
- [ ] Implement connection timeout handling
- [ ] Add retry logic for failed connections
- [ ] Add connection status feedback
- [ ] Handle MJPEG stream parsing
- [ ] Add frame buffering for smooth playback

**Success Criteria:**
- Successfully connects to IP Webcam app
- Displays live video feed
- Handles connection failures gracefully
- No memory leaks during streaming

### Phase 3: GUI Integration (2-3 hours)
**Goal:** Extend existing GUI to support IP camera controls

#### Tasks:
- [ ] Add IP Camera section to "Main Controls" tab
- [ ] Create URL input field with validation
- [ ] Add connection status indicator (LED style)
- [ ] Add "Test Connection" button
- [ ] Add preset buttons for common IP camera apps
- [ ] Add keyboard shortcut 'i' for IP camera toggle

**Success Criteria:**
- GUI elements are responsive and intuitive
- URL validation prevents invalid inputs
- Status indicator updates in real-time
- Preset buttons work with recommended apps

### Phase 4: Configuration & Persistence (1-2 hours)
**Goal:** Save and restore IP camera settings

#### Tasks:
- [ ] Extend config.json schema to include IP camera settings
- [ ] Add save/load methods for IP camera configuration
- [ ] Add IP camera to video source cycling ('v' key)
- [ ] Add configuration validation on startup
- [ ] Add default IP camera settings

**Success Criteria:**
- Settings persist between app launches
- Configuration loads without errors
- Video source cycling includes all three options
- Default settings work with IP Webcam

### Phase 5: Testing & Polish (1-2 hours)
**Goal:** Ensure robust operation and user experience

#### Tasks:
- [ ] Test with IP Webcam app on Android
- [ ] Test connection failure scenarios
- [ ] Test network timeout handling
- [ ] Test configuration save/load
- [ ] Add user documentation in app
- [ ] Add error messages for common issues

**Success Criteria:**
- All tests pass with IP Webcam
- Clear error messages for users
- Smooth transition between video sources
- No crashes or memory issues

## Implementation Details

### Network URL Formats
```
IP Webcam: http://[phone-ip]:8080/video
DroidCam: http://[phone-ip]:4747/video
Alfred: http://[phone-ip]:8080/video
```

### Connection Parameters
- **Timeout:** 5 seconds
- **Max Retries:** 3
- **Buffer Size:** 10 frames
- **Retry Delay:** 2 seconds

### Error Handling
- **Connection Failed:** Show user-friendly message
- **Invalid URL:** Highlight input field in red
- **Network Timeout:** Display timeout message
- **Stream Interrupted:** Attempt reconnection

## Testing Checklist

### Pre-Implementation Tests
- [ ] Verify current camera functionality works
- [ ] Verify video file playback works
- [ ] Verify MIDI functionality unaffected

### Post-Implementation Tests
- [ ] Test IP camera connection with IP Webcam
- [ ] Test switching between all three video sources
- [ ] Test configuration save/load
- [ ] Test error handling scenarios
- [ ] Test with different Android devices
- [ ] Test with different network conditions

### Edge Cases to Test
- [ ] Invalid IP camera URL
- [ ] Network disconnection during streaming
- [ ] Switching video sources rapidly
- [ ] App restart with IP camera configured
- [ ] Multiple connection attempts

## Code Structure Changes

### New Files/Methods
```cpp
// In ofApp.h
enum VideoSource { CAMERA, VIDEO_FILE, IP_CAMERA };
bool connectToIPCamera(const string& url);
void disconnectIPCamera();
void updateIPCameraStatus();
void drawIPCameraControls();

// In ofApp.cpp
// New methods for IP camera handling
// Extended update() and draw() methods
// Extended configuration handling
```

### Modified Files
- `src/ofApp.h` - Add new variables and methods
- `src/ofApp.cpp` - Implement IP camera functionality
- `bin/data/config.json` - Add IP camera configuration

## User Documentation

### Quick Start Guide
1. Install IP Webcam on Android phone
2. Connect phone to same WiFi as computer
3. Open IP Webcam app and start server
4. Note the IP address shown in app
5. In SonifyV1, press 'i' or use GUI to enter IP camera mode
6. Enter URL: `http://[phone-ip]:8080/video`
7. Click "Test Connection" to verify
8. Start detection and sonification

### Troubleshooting
- **No connection:** Check if phone and computer are on same network
- **Poor quality:** Lower resolution in IP Webcam settings
- **Lag:** Reduce frame rate in IP Webcam settings
- **Disconnections:** Check WiFi signal strength

## Future Enhancements
- RTSP protocol support
- Multiple IP camera support
- Camera discovery via mDNS/Bonjour
- Recording IP camera streams
- PTZ (Pan/Tilt/Zoom) control support

## Dependencies
- No additional libraries required
- Uses existing openFrameworks network capabilities
- Compatible with openFrameworks 0.12.1

## Estimated Timeline
- **Total Development Time:** 7-11 hours
- **Testing Time:** 2-3 hours
- **Documentation:** 1 hour
- **Total Project Time:** 10-15 hours

## Risk Assessment
- **Low Risk:** Uses existing openFrameworks functionality
- **Medium Risk:** Network connectivity issues
- **Mitigation:** Robust error handling and user feedback

## Success Metrics
- [ ] IP camera connects within 5 seconds
- [ ] Video displays at 30fps minimum
- [ ] No crashes during 1 hour continuous use
- [ ] All existing functionality preserved
- [ ] User can configure without documentation
