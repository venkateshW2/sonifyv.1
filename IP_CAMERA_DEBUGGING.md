# IP Camera Debugging Guide

## Problem: Browser Works but openFrameworks Doesn't

### Quick Diagnostic Steps

#### 1. **Immediate Testing**
Press **'p'** key or click **"Debug Connection"** in the IP Camera GUI panel to run comprehensive diagnostics.

#### 2. **Network Connectivity Test**
```bash
# Test basic connectivity
ping 192.168.29.151

# Test HTTP response
curl -I http://192.168.29.151:8080/video

# Test with detailed headers
curl -v http://192.168.29.151:8080/video
```

#### 3. **Browser vs openFrameworks Differences**

**Browser advantages:**
- Automatic user-agent spoofing
- Built-in MJPEG parsing
- HTTP header handling
- Authentication handling
- Redirect following

**openFrameworks limitations:**
- Uses AVFoundation on macOS
- Limited MJPEG format support
- No automatic user-agent
- Strict URL format requirements

### Common Issues and Solutions

#### **Issue 1: MJPEG Format Compatibility**
**Symptoms:** Connection fails, black screen, or immediate disconnection
**Solution:** Try alternative endpoints:
- `http://192.168.29.151:8080/video.mjpg`
- `http://192.168.29.151:8080/mjpeg`
- `http://192.168.29.151:8080/mjpeg.cgi`

#### **Issue 2: User-Agent Restrictions**
**Symptoms:** 403 Forbidden or connection refused
**Solution:** Test with curl:
```bash
curl -A "VLC/3.0.18" http://192.168.29.151:8080/video
```

#### **Issue 3: Authentication Required**
**Symptoms:** 401 Unauthorized
**Solution:** Add credentials to URL:
- `http://username:password@192.168.29.151:8080/video`

#### **Issue 4: Port/Protocol Issues**
**Symptoms:** Connection timeout or refused
**Solutions:**
- **HTTP MJPEG:** `http://192.168.29.151:8080/video`
- **RTSP:** `rtsp://192.168.29.151:8080/h264_ulaw.sdp`
- **RTMP:** Check IP Webcam app for RTMP endpoint

#### **Issue 5: Content-Type Issues**
**Symptoms:** AVFoundation rejects stream
**Diagnosis:** Check content-type headers:
```bash
curl -I http://192.168.29.151:8080/video
```

**Expected:** `Content-Type: multipart/x-mixed-replace; boundary=--ipcamera`

### Alternative Connection Methods

#### **Method 1: VLC Bridge**
If direct connection fails, use VLC as a bridge:
1. Open VLC
2. Media → Open Network Stream
3. Enter your IP camera URL
4. Stream → HTTP output to local port
5. Use `http://localhost:8081` in openFrameworks

#### **Method 2: FFmpeg Bridge**
```bash
ffmpeg -i http://192.168.29.151:8080/video -vcodec copy -f mjpeg http://localhost:8081
```

#### **Method 3: GStreamer Pipeline**
```bash
gst-launch-1.0 souphttpsrc location=http://192.168.29.151:8080/video ! multipartdemux ! jpegdec ! videoconvert ! autovideosink
```

### IP Webcam App Specific Settings

#### **Android IP Webcam App:**
1. **Video Preferences:**
   - Video resolution: 640x480 (recommended)
   - Video quality: Medium
   - Orientation: Landscape

2. **Connection Settings:**
   - Port: 8080 (default)
   - Authentication: Optional (test without first)

3. **Advanced Settings:**
   - Enable "MJPEG over HTTP"
   - Disable "HTTPS only"
   - Check "Allow remote access"

#### **Testing Endpoints:**
```
http://192.168.29.151:8080/video          # Primary MJPEG
http://192.168.29.151:8080/video.mjpg     # Forced MJPEG
http://192.168.29.151:8080/mjpeg          # Alternative MJPEG
rtsp://192.168.29.151:8080/h264_ulaw.sdp  # RTSP stream
```

### Debugging Workflow

1. **Start with browser verification:**
   - Confirm URL works in browser
   - Note exact URL format
   - Check if authentication required

2. **Use built-in debugging:**
   - Press 'p' key for comprehensive diagnostics
   - Check GUI "Debug Connection" button
   - Review console output for specific errors

3. **Test with curl:**
   ```bash
   curl -v http://192.168.29.151:8080/video
   ```

4. **Try VLC test:**
   - Open VLC → Open Network Stream
   - Test your URL
   - If VLC works, note exact format

5. **Check firewall/network:**
   - Ensure devices on same network
   - Check firewall settings
   - Verify IP address hasn't changed

### macOS Specific Issues

#### **AVFoundation Limitations:**
- **Supported formats:** MP4, MOV, MJPEG
- **Unsupported:** Some H.264 streams, certain MJPEG variants
- **Solution:** Use MJPEG format or RTSP

#### **Network Permissions:**
- **macOS 10.15+:** May require network permissions
- **Solution:** Add Terminal/app to System Preferences → Security & Privacy → Firewall

### Emergency Fallback Solutions

#### **Solution A: Screen Recording**
If IP camera fails completely:
1. Use browser to view camera
2. Screen record browser window
3. Use recorded video as input

#### **Solution B: Phone as Webcam**
1. Use phone as USB webcam via apps like:
   - DroidCam (Android)
   - EpocCam (iOS)
   - iVCam

#### **Solution C: OBS Virtual Camera**
1. Install OBS Studio
2. Add IP camera as source
3. Use OBS Virtual Camera as input

### Testing Checklist

- [ ] URL works in browser
- [ ] URL works in VLC
- [ ] Network connectivity confirmed
- [ ] Firewall settings checked
- [ ] IP Webcam app permissions granted
- [ ] Alternative endpoints tested
- [ ] User-agent restrictions checked
- [ ] Authentication requirements verified

### Quick Fix Commands

```bash
# Test basic connectivity
ping 192.168.29.151

# Test HTTP response
curl -I http://192.168.29.151:8080/video

# Test with different user agents
curl -A "VLC/3.0.18" http://192.168.29.151:8080/video

# Test RTSP (if available)
vlc rtsp://192.168.29.151:8080/h264_ulaw.sdp
```

### Getting Help

If issues persist:
1. Run debugging tool (press 'p')
2. Copy console output
3. Check IP Webcam app logs
4. Verify network configuration
5. Test with alternative apps (VLC, browser)

**Remember:** The debugging tool provides the most comprehensive diagnostic information. Always start there!