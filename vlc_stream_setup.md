# VLC HTTP Streaming Setup for IP Webcam

## Method 1: VLC as HTTP Proxy (Recommended)

### Step 1: Install VLC Media Player
Download from: https://www.videolan.org/vlc/

### Step 2: Create VLC Stream
```bash
# Command to run in Terminal:
/Applications/VLC.app/Contents/MacOS/VLC \
  http://192.168.29.151:8080/video \
  --intf dummy \
  --sout '#http{mux=ts,dst=:8081/stream.ts}' \
  --no-sout-display \
  --sout-keep

# This creates a local HTTP stream at: http://localhost:8081/stream.ts
```

### Step 3: Use in SonifyV1
- IP Camera URL: `http://localhost:8081/stream.ts`
- VLC handles the MJPEG conversion automatically
- More compatible with openFrameworks ofVideoPlayer

## Method 2: VLC GUI Setup (User-Friendly)

### Step 1: Open VLC
1. Open VLC Media Player
2. Go to "Media" → "Stream..."

### Step 2: Add Network Stream
1. Click "Network" tab
2. Enter: `http://192.168.29.151:8080/video`
3. Click "Stream"

### Step 3: Configure Output
1. Click "Next" (keep source as is)
2. Select "HTTP" from dropdown
3. Set Path: `/stream.ts`
4. Set Port: `8081`
5. Click "Stream"

### Step 4: Use in App
- URL in SonifyV1: `http://localhost:8081/stream.ts`

## Method 3: FFmpeg HTTP Re-streaming

### Install FFmpeg
```bash
brew install ffmpeg
```

### Create HTTP Stream
```bash
ffmpeg -i http://192.168.29.151:8080/video \
       -c copy \
       -f mpegts \
       http://localhost:8082/stream.ts
```

### Use in App
- URL: `http://localhost:8082/stream.ts`

## Method 4: Simple Python HTTP Server

### Create stream_proxy.py
```python
#!/usr/bin/env python3
import http.server
import socketserver
import requests
import threading

class StreamHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/video':
            # Proxy the IP camera stream
            response = requests.get('http://192.168.29.151:8080/video', stream=True)
            self.send_response(200)
            self.send_header('Content-Type', 'video/x-motion-jpeg')
            self.end_headers()
            
            for chunk in response.iter_content(chunk_size=1024):
                if chunk:
                    self.wfile.write(chunk)

# Run server on port 8083
PORT = 8083
with socketserver.TCPServer(("", PORT), StreamHandler) as httpd:
    print(f"Server at http://localhost:{PORT}/video")
    httpd.serve_forever()
```

### Run and Use
```bash
python3 stream_proxy.py
# Use URL: http://localhost:8083/video
```

## Recommended Solution: VLC Method

The VLC method is best because:
- **Easy Setup**: Just one command or GUI clicks
- **Reliable**: VLC handles all format conversion
- **Compatible**: Works well with openFrameworks
- **No Coding**: Uses existing, tested software
- **Free**: No additional cost

## Testing the Solutions

1. Start your IP Webcam app on phone (http://192.168.29.151:8080/)
2. Choose one of the methods above to create local HTTP stream
3. In SonifyV1, use the local URL (like http://localhost:8081/stream.ts)
4. Test connection with the debug tools we built

This should resolve the HTTP video access issues and provide a stable streaming solution.