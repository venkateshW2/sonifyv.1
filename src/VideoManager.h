#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class VideoManager {
public:
    // Video source enumeration - EXACT COPY from working backup
    enum VideoSource { CAMERA, VIDEO_FILE, IP_CAMERA };
    
    VideoManager();
    ~VideoManager();
    
    void setup();
    void update();
    void draw();
    
    // Video source management - EXACT same methods as working backup
    void validateAndFixVideoSource();
    
    // Keyboard controls - EXACT same logic
    void handleVideoKeyPress(int key);
    void handleVideoSourceSwitch();
    void handleCameraRestart();
    void handleVideoFileOpen();
    
    // Getters for status - EXACT same as working backup
    bool isCameraConnected() const { return cameraConnected; }
    bool isVideoLoaded() const { return videoLoaded; }
    VideoSource getCurrentVideoSource() const { return currentVideoSource; }
    string getCurrentVideoPath() const { return currentVideoPath; }
    
    // IP Camera methods - EXACT COPY from working backup
    string getIPCameraUrl() const { return ipCameraUrl; }
    void setIPCameraUrl(const string& url) { ipCameraUrl = url; ipCameraSnapshotUrl = url; }
    bool isIPCameraConnected() const { return ipCameraConnected; }
    void connectIPCamera();
    void disconnectIPCamera();
    float getIPCameraFrameInterval() const { return frameRequestInterval; }
    void setIPCameraFrameInterval(float interval) { frameRequestInterval = interval; }
    int getIPCameraFrameSkip() const { return ipFrameSkip; }
    void setIPCameraFrameSkip(int skip) { ipFrameSkip = skip; }
    
    // Detection support - get pixels for CoreML detection
    ofPixels getCurrentPixels();
    
    // Video objects - EXACT same as working backup
    ofVideoGrabber camera;
    ofVideoPlayer videoPlayer;
    
    // Video state variables - EXACT COPY from working backup  
    bool useVideoFile;
    bool videoLoaded;
    bool videoPaused;
    bool cameraConnected;
    string currentVideoPath;
    VideoSource currentVideoSource;
    
    // IP Camera variables - EXACT COPY from working backup
    string ipCameraUrl;
    string ipCameraSnapshotUrl;
    bool ipCameraConnected;
    ofImage currentIPFrame;
    bool ipFrameReady;
    float lastFrameRequest;
    float frameRequestInterval;
    int ipFrameSkip;
    int ipFrameCounter;
    
    // Configuration methods
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // Missing methods referenced in cpp files
    void openVideoFileDialog();
    void setupCamera();
    
private:
    // Helper methods that exist in working backup
    void loadTestVideo();
    void initializeCamera();
    bool trySetupCamera();
};