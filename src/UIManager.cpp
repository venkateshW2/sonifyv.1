#include "UIManager.h"
#include "ofxImGui.h"
#include "LineManager.h"
#include "VideoManager.h"
#include "DetectionManager.h"
#include "CommunicationManager.h"
#include "ConfigManager.h"
#include "ScaleManager.h"

UIManager::UIManager() {
    // EXACT COPY from working backup
    showGUI = true;
    showResizeWarning_ = false;
    
    // Initialize GUI state variables - EXACT COPY from working backup
    confidenceThreshold = 0.25f;  // Lower threshold to allow more detections through
    frameSkipValue = 3;
    showTrajectoryTrails = false;
    showVelocityVectors = false;
    enableOcclusionTracking = false;
    trailFadeTime = 3.0f;
    maxTrajectoryPoints = 50;
    
    // Initialize missing GUI variables
    showDetections = true;
    showLines = true;
    
    // Initialize manager pointers
    videoManager = nullptr;
    lineManager = nullptr;
    detectionManager = nullptr;
    communicationManager = nullptr;
    commManager = nullptr;
    configManager = nullptr;
}

UIManager::~UIManager() {
}

void UIManager::setup() {
    // EXACT COPY from working backup
    gui.setup();
    
    // Set a more subtle and calmer color scheme - EXACT COPY from working backup
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2.0f;         // Slightly rounded corners
    style.FrameRounding = 3.0f;          // Rounded buttons and frames
    style.ItemSpacing = ImVec2(8, 6);    // Add more space between items
    style.ScrollbarSize = 14.0f;         // Wider scrollbars
    
    ImVec4* colors = style.Colors;
    
    // More muted, calmer color theme - lighter background - EXACT COPY from working backup
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.18f, 0.94f);  // Lighter background
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.58f, 0.31f);    // More muted header
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.40f, 0.65f, 0.50f); // Softer highlight
    colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.35f, 0.58f, 0.40f);    // More muted buttons
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.40f, 0.65f, 0.60f); // Softer highlight
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.40f, 0.65f, 0.50f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25f, 0.45f, 0.70f, 0.80f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.45f, 0.70f, 0.90f); // Softer checkmarks
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);   // Match window background
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.45f, 0.70f, 0.30f);    // Subtle borders
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.22f, 0.54f);   // Subtle frame background
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 0.54f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.20f, 0.80f);   // Title background
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.35f, 0.58f, 0.80f); // Active title
    
    ofLogNotice() << "UIManager: ImGui setup complete with blue calm theme";
}

void UIManager::update() {
    // Minimal update - most GUI logic is in draw
}

void UIManager::draw() {
    drawGUI();
}

bool UIManager::wantsKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

void UIManager::toggleGUI() {
    showGUI = !showGUI;
    ofLogNotice() << "UIManager: GUI " << (showGUI ? "enabled" : "disabled");
}

void UIManager::showResizeWarningDialog() {
    // This will be implemented later - needs window resize detection
    showResizeWarning_ = false;
}

void UIManager::setManagers(VideoManager* videoMgr, LineManager* lineMgr, DetectionManager* detMgr, CommunicationManager* commMgr, ConfigManager* confMgr, ScaleManager* scaleMgr) {
    videoManager = videoMgr;
    lineManager = lineMgr;
    detectionManager = detMgr;
    communicationManager = commMgr;
    commManager = commMgr;  // Alias for consistency with code
    configManager = confMgr;
    scaleManager = scaleMgr;
}

// EXACT COPY from working backup
void UIManager::drawGUI() {
    if (!showGUI) return;
    
    gui.begin();
    
    // Task 5.1: Show window resize warning dialog if needed
    if (showResizeWarning_) {
        showResizeWarningDialog();
    }
    
    // Main tabbed interface window - fixed position and size
    ImGui::SetNextWindowPos(ImVec2(640, 0));
    ImGui::SetNextWindowSize(ImVec2(410, 640));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoCollapse | 
                                   ImGuiWindowFlags_NoTitleBar;
    
    if (ImGui::Begin("SonifyV1 Controls", nullptr, window_flags)) {
        // Create tab bar
        if (ImGui::BeginTabBar("ControlTabs")) {
            
            // Main Controls Tab
            if (ImGui::BeginTabItem("Main Controls")) {
                drawMainControlsTab();
                ImGui::EndTabItem();
            }
            
            // MIDI Settings Tab  
            if (ImGui::BeginTabItem("MIDI Settings")) {
                drawMIDISettingsTab();
                ImGui::EndTabItem();
            }
            
            // Detection Classes Tab
            if (ImGui::BeginTabItem("Detection Classes")) {
                drawDetectionClassesTab();
                ImGui::EndTabItem();
            }
            
            // Scale Manager Tab
            if (ImGui::BeginTabItem("Scale Manager")) {
                drawScaleManagerTab();
                ImGui::EndTabItem();
            }
            
            // Pose Detection Tab
            
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    
    gui.end();
}

void UIManager::drawMainControlsTab() {
    // Detection Controls
    if (ImGui::CollapsingHeader("Detection Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (detectionManager) {
            bool enabled = detectionManager->isDetectionEnabled();
            if (ImGui::Checkbox("Enable Detection", &enabled)) {
                detectionManager->setDetectionEnabled(enabled);
            }
            
            float confidence = detectionManager->getConfidenceThreshold();
            if (ImGui::SliderFloat("Confidence Threshold", &confidence, 0.1f, 1.0f, "%.2f")) {
                detectionManager->setConfidenceThreshold(confidence);
                confidenceThreshold = confidence;
            }
            
            // Frame skip is read-only display for modular version
            // RESTORED: Interactive frame skip slider from working backup
            if (ImGui::SliderInt("Frame Skip", &frameSkipValue, 1, 10, "%d")) {
                // Apply changes to detection system immediately
                if (detectionManager) {
                    detectionManager->setDetectionFrameSkip(frameSkipValue);
                }
            }
            ImGui::Checkbox("Show Detections", &showDetections);
        }
    }
    
    // Enhanced Tracking Controls
    if (ImGui::CollapsingHeader("Enhanced Tracking")) {
        ImGui::Checkbox("Show Trajectory Trails", &showTrajectoryTrails);
        ImGui::Checkbox("Show Velocity Vectors", &showVelocityVectors);
        ImGui::Checkbox("Enable Occlusion Tracking", &enableOcclusionTracking);
        
        ImGui::SliderFloat("Trail Fade Time", &trailFadeTime, 0.5f, 10.0f, "%.1f sec");
        ImGui::SliderInt("Max Trajectory Points", &maxTrajectoryPoints, 10, 100, "%d");
    }
    
    // OSC Settings Section  
    if (ImGui::CollapsingHeader("OSC Settings")) {
        if (commManager) {
            bool oscEnabled = true;  // Assuming always enabled in modular version
            ImGui::Checkbox("OSC Enabled", &oscEnabled);
            
            ImGui::Text("Host: 127.0.0.1");
            ImGui::Text("Port: 12000");
            
            if (ImGui::Button("Test OSC Connection")) {
                // Could trigger a test message
            }
        }
    }
    
    // Line Drawing Section
    if (ImGui::CollapsingHeader("Line Drawing", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Lines", &showLines);
        
        if (lineManager) {
            ImGui::Text("Lines drawn: %d", lineManager->getLineCount());
            
            if (ImGui::Button("Clear All Lines")) {
                lineManager->clearAllLines();
            }
        }
        
        // Instructions
        ImGui::TextWrapped("Left click to start line, right click to finish line");
        ImGui::TextWrapped("Click on line to select, Delete/Backspace to remove selected line");
        ImGui::TextWrapped("Drag line endpoints to move lines");
    }
    
    // Video Controls Section
    if (ImGui::CollapsingHeader("Video Controls")) {
        if (videoManager) {
            if (ImGui::Button("Switch Video Source")) {
                videoManager->handleVideoSourceSwitch();
            }
            
            if (ImGui::Button("Open Video File")) {
                videoManager->openVideoFileDialog();
            }
            
            if (ImGui::Button("Initialize Camera")) {
                videoManager->setupCamera();
            }
            
            // Display current video source
            const char* sourceNames[] = {"Camera", "Video File", "IP Camera"};
            int currentSource = (int)videoManager->getCurrentVideoSource();
            ImGui::Text("Current Source: %s", sourceNames[currentSource]);
        }
    }
    
    // USB Camera Selection Section
    if (ImGui::CollapsingHeader("USB Camera Selection")) {
        if (videoManager) {
            // Display current camera
            ImGui::Text("Current Camera: %s (ID: %d)", 
                       videoManager->getCurrentCameraName().c_str(),
                       videoManager->getCurrentCameraDevice());
            
            if (ImGui::Button("Refresh Camera Devices")) {
                videoManager->refreshCameraDevices();
            }
            ImGui::SameLine();
            
            // Camera device dropdown
            vector<ofVideoDevice> cameras = videoManager->getAvailableCameras();
            if (!cameras.empty()) {
                // Create combo box items
                vector<const char*> cameraNames;
                for (const auto& cam : cameras) {
                    cameraNames.push_back(cam.deviceName.c_str());
                }
                
                int currentCamera = videoManager->getCurrentCameraDevice();
                if (ImGui::Combo("Camera Device", &currentCamera, cameraNames.data(), cameraNames.size())) {
                    videoManager->setCameraDevice(currentCamera);
                }
                
                // Show camera details
                if (currentCamera >= 0 && currentCamera < cameras.size()) {
                    ImGui::Text("Device Details:");
                    ImGui::Text("  Name: %s", cameras[currentCamera].deviceName.c_str());
                    ImGui::Text("  ID: %d", cameras[currentCamera].id);
                    ImGui::Text("  Available: %s", cameras[currentCamera].bAvailable ? "Yes" : "No");
                }
            } else {
                ImGui::Text("No cameras detected");
            }
        }
    }
    
    // IP Camera Settings Section - EXACT COPY from working backup
    if (ImGui::CollapsingHeader("IP Camera Settings")) {
        if (videoManager) {
            // IP Camera URL input
            static char ipCameraUrlBuffer[256];
            string currentUrl = videoManager->getIPCameraUrl();
            strcpy(ipCameraUrlBuffer, currentUrl.c_str());
            
            if (ImGui::InputText("IP Camera URL", ipCameraUrlBuffer, sizeof(ipCameraUrlBuffer))) {
                videoManager->setIPCameraUrl(string(ipCameraUrlBuffer));
            }
            
            // Connection status
            bool connected = videoManager->isIPCameraConnected();
            ImGui::Text("Status: %s", connected ? "Connected" : "Disconnected");
            
            if (connected) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "●");
            } else {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "●");
            }
            
            // Connect/Disconnect buttons
            if (!connected) {
                if (ImGui::Button("Connect IP Camera")) {
                    videoManager->connectIPCamera();
                }
            } else {
                if (ImGui::Button("Disconnect IP Camera")) {
                    videoManager->disconnectIPCamera();
                }
            }
            
            // Performance settings
            ImGui::Separator();
            ImGui::Text("Performance Settings:");
            
            float frameInterval = videoManager->getIPCameraFrameInterval();
            if (ImGui::SliderFloat("Frame Interval", &frameInterval, 0.1f, 2.0f, "%.1f sec")) {
                videoManager->setIPCameraFrameInterval(frameInterval);
            }
            
            int frameSkip = videoManager->getIPCameraFrameSkip();
            if (ImGui::SliderInt("Frame Skip", &frameSkip, 1, 10)) {
                videoManager->setIPCameraFrameSkip(frameSkip);
            }
            
            // Instructions
            ImGui::Separator();
            ImGui::TextWrapped("Enter IP camera HTTP/MJPEG stream URL (e.g., http://192.168.1.100:8080/video). Use IP Webcam app or similar.");
            ImGui::TextWrapped("Lower frame rates improve performance. Try 2fps (0.5 sec interval) for best results.");
        }
    }
    
    // Performance Stats
    if (ImGui::CollapsingHeader("Performance Stats")) {
        ImGui::Text("FPS: %.1f", ofGetFrameRate());
        ImGui::Text("System Status:");
        ImGui::Text("LineManager: %s", lineManager ? "OK" : "NULL");
        ImGui::Text("VideoManager: %s", videoManager ? "OK" : "NULL");
        ImGui::Text("DetectionManager: %s", detectionManager ? "OK" : "NULL");
        ImGui::Text("CommunicationManager: %s", commManager ? "OK" : "NULL");
    }
    
    // Configuration Section - EXACT COPY from working backup
    if (ImGui::CollapsingHeader("Configuration")) {
        ImGui::Text("Save/Load Settings");
        
        if (ImGui::Button("Save Config")) {
            if (configManager) {
                configManager->saveConfig();
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Load Config")) {
            if (configManager) {
                configManager->loadConfig();
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Reset to Defaults")) {
            if (configManager) {
                configManager->resetToDefaults();
            }
        }
        
        ImGui::Separator();
        
        // Config path display
        string configPath = ofToDataPath("config.json");
        ImGui::Text("Config Path:");
        ImGui::TextWrapped("%s", configPath.c_str());
        
        ImGui::Text("Auto-save: On application exit");
    }
    
    // Live Tracking Data - EXACT COPY from working backup (at bottom to avoid disrupting other controls)
    if (ImGui::CollapsingHeader("Live Tracking Data")) {
        if (detectionManager) {
            ImGui::Text("Tracking Statistics:");
            ImGui::Text("Active Objects: %d", detectionManager->getTrackedVehiclesCount());
            ImGui::Text("Visible: %d, Occluded: %d", 
                       detectionManager->getVisibleVehiclesCount(), 
                       detectionManager->getOccludedVehiclesCount());
            
            ImGui::Separator();
            
            // Show tracked object details (all COCO classes)
            const auto& trackedVehicles = detectionManager->getTrackedVehicles();
            if (trackedVehicles.empty()) {
                ImGui::Text("Object Details: None currently tracked");
            } else {
                ImGui::Text("Tracked Objects (All Classes):");
                for (size_t i = 0; i < std::min(trackedVehicles.size(), (size_t)5); i++) {
                    const auto& vehicle = trackedVehicles[i];
                    ImGui::Text("ID %d: %s (%.1f%% conf, %.1f speed)", 
                               vehicle.id, vehicle.className.c_str(), 
                               vehicle.confidence * 100.0f, vehicle.speed);
                }
                if (trackedVehicles.size() > 5) {
                    ImGui::Text("... and %d more", (int)(trackedVehicles.size() - 5));
                }
            }
        }
        
        if (commManager) {
            ImGui::Separator();
            ImGui::Text("MIDI Activity:");
            ImGui::Text("Total MIDI Events: %d", commManager->getTotalMidiEvents());
            
            // Activity indicator bar
            float activityLevel = commManager->midiActivityCounter > 0 ? 1.0f : 0.0f;
            string activityText = commManager->midiActivityCounter > 0 ? "Active" : "Ready";
            ImGui::ProgressBar(activityLevel, ImVec2(-1, 0), activityText.c_str());
        }
    }
    
    // Keyboard Shortcuts
    if (ImGui::CollapsingHeader("Keyboard Shortcuts")) {
        ImGui::Text("'g' - Toggle GUI");
        ImGui::Text("'d' - Toggle Detection");
        ImGui::Text("'v' - Switch Video Source");
        ImGui::Text("'o' - Open Video File");
        ImGui::Text("'r' - Restart Camera");
        ImGui::Text("'c' - Clear All Lines");
        ImGui::Text("SPACE - Play/Pause Video");
        ImGui::Text("LEFT/RIGHT - Seek Video");
        ImGui::Text("'l' - Toggle Loop");
    }
}

void UIManager::drawMIDISettingsTab() {
    // Master Musical System Panel
    if (ImGui::CollapsingHeader("Master Musical System", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (lineManager) {
            // Root note selector
            const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            int rootNote = lineManager->getMasterRootNote();
            if (ImGui::Combo("Root Key", &rootNote, noteNames, 12)) {
                lineManager->setMasterRootNote(rootNote);
            }
            
            // Scale selector
            vector<string> scales = lineManager->getAvailableScales();
            string currentScale = lineManager->getMasterScale();
            if (ImGui::BeginCombo("Scale", currentScale.c_str())) {
                for (const string& scale : scales) {
                    bool selected = (scale == currentScale);
                    if (ImGui::Selectable(scale.c_str(), selected)) {
                        lineManager->setMasterScale(scale);
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            
            // Show current scale notes
            vector<string> scaleNotes = lineManager->getScaleNoteNames();
            ImGui::Text("Scale notes: ");
            ImGui::SameLine();
            for (size_t i = 0; i < scaleNotes.size(); i++) {
                if (i > 0) ImGui::SameLine();
                ImGui::Text("%s", scaleNotes[i].c_str());
                if (i < scaleNotes.size() - 1) {
                    ImGui::SameLine();
                    ImGui::Text(",");
                }
            }
        }
    }
    
    // Lines List Panel
    if (ImGui::CollapsingHeader("Lines & Musical Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (lineManager) {
            const vector<LineManager::MidiLine>& lines = lineManager->getLines();
            ImGui::Text("Lines List (%d)", (int)lines.size());
            ImGui::Separator();
            
            // List all lines with musical properties
            for (int i = 0; i < lines.size(); i++) {
                bool isSelected = (i == lineManager->getSelectedLineIndex());
                const LineManager::MidiLine& line = lines[i];
                
                ImGui::PushID(i);
                
                // Create display text with musical info
                vector<string> scaleNoteNames = lineManager->getScaleNoteNames();
                string noteName = "?";
                if (line.scaleNoteIndex >= 0 && line.scaleNoteIndex < scaleNoteNames.size()) {
                    noteName = scaleNoteNames[line.scaleNoteIndex];
                }
                
                string lineLabel = "L" + ofToString(i + 1) + ": " + 
                                   (line.randomizeNote ? "Random" : noteName) + ofToString(line.octave) + 
                                   " Ch" + ofToString(line.midiChannel) + 
                                   (line.randomizeNote ? " (R)" : "");
                
                if (ImGui::Selectable(lineLabel.c_str(), isSelected)) {
                    lineManager->selectLine(i);
                }
                
                // Show line color indicator
                ImGui::SameLine();
                ImVec2 colorPos = ImGui::GetCursorScreenPos();
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec4 lineColor = ImVec4(line.color.r/255.0f, line.color.g/255.0f, line.color.b/255.0f, 1.0f);
                drawList->AddRectFilled(colorPos, ImVec2(colorPos.x + 12, colorPos.y + 12), ImGui::ColorConvertFloat4ToU32(lineColor));
                
                ImGui::PopID();
            }
            
            // Selected line properties
            if (lineManager->getSelectedLineIndex() >= 0) {
                ImGui::Separator();
                ImGui::Text("Selected Line Properties:");
                LineManager::MidiLine* selectedLine = lineManager->getSelectedLine();
                if (selectedLine) {
                    // Randomization toggle - EXACT COPY from working backup
                    if (ImGui::Checkbox("Randomize Note", &selectedLine->randomizeNote)) {
                        ofLogNotice() << "Line " << (lineManager->getSelectedLineIndex() + 1) << " randomization " 
                                      << (selectedLine->randomizeNote ? "enabled" : "disabled");
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()) {
                        if (selectedLine->randomizeNote) {
                            ImGui::SetTooltip("When enabled, line will play random notes from the master scale");
                        } else {
                            ImGui::SetTooltip("When disabled, line will always play the selected scale note");
                        }
                    }
                    
                    // Scale note selection (with visual feedback)
                    ImGui::Text("Scale Note Selection:");
                    vector<string> scaleNotes = lineManager->getScaleNoteNames();
                    
                    // Create a grid of note buttons like in working backup
                    int columns = ofClamp((int)scaleNotes.size(), 3, 7); // Responsive columns
                    float buttonWidth = (ImGui::GetContentRegionAvail().x - (columns - 1) * ImGui::GetStyle().ItemSpacing.x) / columns;
                    
                    for (int i = 0; i < scaleNotes.size(); i++) {
                        bool isSelected = (i == selectedLine->scaleNoteIndex && !selectedLine->randomizeNote);
                        bool isCurrentlyPlaying = false;
                        
                        if (selectedLine->randomizeNote) {
                            // Show cycling preview for random mode
                            isCurrentlyPlaying = (i == (ofGetElapsedTimeMillis() / 500) % scaleNotes.size());
                        }
                        
                        // Button styling
                        if (isSelected) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f)); // Green for selected
                        } else if (isCurrentlyPlaying && selectedLine->randomizeNote) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.2f, 1.0f)); // Yellow for random preview
                        } else if (selectedLine->randomizeNote) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 0.6f)); // Dim when in random mode
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.40f)); // Default blue
                        }
                        
                        string buttonLabel = scaleNotes[i];
                        if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonWidth, 30))) {
                            // Only allow selection when not in random mode
                            if (!selectedLine->randomizeNote) {
                                selectedLine->scaleNoteIndex = i;
                                ofLogNotice() << "Line " << (lineManager->getSelectedLineIndex() + 1) << " note changed to: " << scaleNotes[i];
                            }
                        }
                        
                        ImGui::PopStyleColor();
                        
                        // Show tooltip
                        if (ImGui::IsItemHovered()) {
                            if (selectedLine->randomizeNote) {
                                ImGui::SetTooltip("Random mode: Any note from scale can play");
                            } else if (isSelected) {
                                ImGui::SetTooltip("Selected note: %s", scaleNotes[i].c_str());
                            } else {
                                ImGui::SetTooltip("Click to select note: %s", scaleNotes[i].c_str());
                            }
                        }
                        
                        // Layout in columns
                        if ((i + 1) % columns != 0 && i < scaleNotes.size() - 1) {
                            ImGui::SameLine();
                        }
                    }
                    
                    // Status text
                    ImGui::Separator();
                    if (selectedLine->randomizeNote) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Mode: Immediate random selection");
                        ImGui::TextDisabled("(Yellow highlight shows random preview)");
                    } else {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mode: Fixed note - %s", scaleNotes[selectedLine->scaleNoteIndex].c_str());
                        ImGui::TextDisabled("(Green button shows selected note)");
                    }
                    
                    ImGui::Spacing();
                    ImGui::SliderInt("Octave", &selectedLine->octave, 0, 10);
                    ImGui::SliderInt("MIDI Channel", &selectedLine->midiChannel, 1, 16);
                    
                    // Randomization settings
                    ImGui::Separator();
                    ImGui::Text("Randomization Settings:");
                    
                    // Random seed control
                    if (ImGui::SliderInt("Random Seed", &selectedLine->randomSeed, 0, 999)) {
                        ofLogNotice() << "Line " << (lineManager->getSelectedLineIndex() + 1) << " random seed: " << selectedLine->randomSeed;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("New Seed")) {
                        selectedLine->randomSeed = rand() % 1000;
                    }
                    
                    // Scale degree weights editor
                    ImGui::Text("Scale Degree Weights:");
                    ImGui::SameLine();
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Higher values = more likely to be selected. Adjust for musical emphasis.");
                    }
                    
                    // Ensure weights vector matches current scale size
                    vector<string> scaleNotes = lineManager->getScaleNoteNames();
                    if (selectedLine->scaleDegreeWeights.size() != scaleNotes.size()) {
                        selectedLine->scaleDegreeWeights.resize(scaleNotes.size(), 1.0f);
                    }
                    
                    // Weight sliders for each scale degree
                    for (int i = 0; i < scaleNotes.size() && i < selectedLine->scaleDegreeWeights.size(); i++) {
                        string label = scaleNotes[i] + " Weight";
                        if (ImGui::SliderFloat(label.c_str(), &selectedLine->scaleDegreeWeights[i], 0.1f, 2.0f, "%.2f")) {
                            // Ensure minimum weight
                            selectedLine->scaleDegreeWeights[i] = std::max(0.1f, selectedLine->scaleDegreeWeights[i]);
                        }
                    }
                    
                    if (ImGui::Button("Reset Weights to Default")) {
                        // Set musical defaults (emphasize root and fifth)
                        for (int i = 0; i < selectedLine->scaleDegreeWeights.size(); i++) {
                            if (i == 0) selectedLine->scaleDegreeWeights[i] = 1.5f; // Root
                            else if (i == 4 && i < selectedLine->scaleDegreeWeights.size()) selectedLine->scaleDegreeWeights[i] = 1.4f; // Fifth (in major scale)
                            else selectedLine->scaleDegreeWeights[i] = 1.0f; // Others
                        }
                    }
                    
                    // Duration settings
                    ImGui::Separator();
                    ImGui::Text("Duration Settings:");
                    const char* durationTypes[] = {"Fixed", "Speed-Based", "Vehicle-Based"};
                    int durationType = (int)selectedLine->durationType;
                    if (ImGui::Combo("Duration Mode", &durationType, durationTypes, 3)) {
                        selectedLine->durationType = (LineManager::MidiLine::DurationType)durationType;
                    }
                    
                    if (selectedLine->durationType == LineManager::MidiLine::DURATION_FIXED) {
                        ImGui::SliderInt("Duration (ms)", &selectedLine->fixedDuration, 50, 2000);
                    }
                }
            }
        }
    }
    
    // MIDI Port Selection - Multi-port system with checkboxes - EXACT COPY from working backup
    if (ImGui::CollapsingHeader("MIDI Ports", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (commManager) {
            auto portNames = commManager->getMidiPortNames();
            auto portSelected = commManager->getMidiPortSelected();
            auto portConnected = commManager->getMidiPortConnected();
            
            if (!portNames.empty()) {
                ImGui::Text("Select MIDI output ports:");
                ImGui::Separator();
                
                for (int i = 0; i < portNames.size(); i++) {
                    ImGui::PushID(i);
                    
                    bool wasSelected = portSelected[i];
                    bool currentSelected = portSelected[i];
                    if (ImGui::Checkbox("##port", &currentSelected)) {
                        commManager->setMIDIPortSelected(i, currentSelected);
                        
                        if (currentSelected && !wasSelected) {
                            // Connect this port
                            commManager->connectMIDIPort(i);
                        } else if (!currentSelected && wasSelected) {
                            // Disconnect this port
                            commManager->disconnectMIDIPort(i);
                        }
                    }
                    
                    // Port name
                    ImGui::SameLine();
                    ImGui::Text("%s", portNames[i].c_str());
                    
                    // Connection status
                    ImGui::SameLine();
                    if (portSelected[i]) {
                        ImGui::SameLine();
                        if (portConnected[i]) {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " [Connected]");
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " [Failed]");
                        }
                    } else {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " [Disconnected]");
                    }
                    
                    ImGui::PopID();
                }
                
                // Summary
                ImGui::Separator();
                int connectedCount = 0;
                for (int i = 0; i < portSelected.size(); i++) {
                    if (portSelected[i]) {
                        connectedCount++;
                        if (portConnected[i]) {
                            // Port is connected
                        }
                    }
                }
                ImGui::Text("Status: %d/%d ports selected", connectedCount, (int)portNames.size());
            } else {
                ImGui::Text("No MIDI ports available");
            }
            
            if (ImGui::Button("Refresh Ports")) {
                commManager->refreshMIDIPorts();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Test MIDI")) {
                commManager->sendTestMIDINote();
            }
        }
    }
}

void UIManager::drawDetectionClassesTab() {
    if (!detectionManager) return;
    
    // EXACT COPY from working backup detection_classes_tab.cpp
    // Presets row
    ImGui::Text("Quick Presets:");
    
    // Create preset buttons in a row
    float availWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = (availWidth - 3 * ImGui::GetStyle().ItemSpacing.x) / 4; // 4 buttons with 3 spaces between
    
    string currentPreset = detectionManager->getCurrentPreset();
    if (ImGui::Button("Vehicles Only", ImVec2(buttonWidth, 0))) {
        detectionManager->applyPreset("Vehicles Only");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("People & Animals", ImVec2(buttonWidth, 0))) {
        detectionManager->applyPreset("People & Animals");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Everything Moving", ImVec2(buttonWidth, 0))) {
        detectionManager->applyPreset("Everything Moving");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Custom", ImVec2(buttonWidth, 0))) {
        detectionManager->setCurrentPreset("Custom");
        // No auto-apply for custom - user will select classes manually
    }
    
    ImGui::Separator();
    
    // Display selected class count
    auto selectedClassIds = detectionManager->getSelectedClassIds();
    int maxSelectedClasses = detectionManager->getMaxSelectedClasses();
    ImGui::Text("Selected: %d/%d classes", (int)selectedClassIds.size(), maxSelectedClasses);
    
    // Categories sections
    ImGui::Spacing();
    
    // Vehicles Category
    bool vehiclesCategoryEnabled = detectionManager->getCategoryEnabled(DetectionManager::CATEGORY_VEHICLES);
    if (ImGui::CollapsingHeader("Vehicles", vehiclesCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        // Show category checkbox
        bool categoryChecked = vehiclesCategoryEnabled;
        if (ImGui::Checkbox("Enable Vehicles", &categoryChecked)) {
            detectionManager->setCategoryEnabled(DetectionManager::CATEGORY_VEHICLES, categoryChecked);
            
            if (categoryChecked) {
                // Add all vehicle classes (but respect the max limit)
                vector<int> vehicleClasses = detectionManager->getClassesInCategory(DetectionManager::CATEGORY_VEHICLES);
                for (int classId : vehicleClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        detectionManager->addSelectedClass(classId);
                    }
                }
                detectionManager->setCurrentPreset("Custom");
            } else {
                // Remove all vehicle classes
                detectionManager->removeClassesByCategory(DetectionManager::CATEGORY_VEHICLES);
                detectionManager->setCurrentPreset("Custom");
            }
            detectionManager->updateEnabledClassesFromSelection();
        }
        
        // Show individual vehicle classes
        ImGui::Indent();
        vector<int> vehicleClasses = {1, 2, 3, 4, 5, 6, 7, 8}; // bicycle through boat
        
        // Display vehicle classes in a grid with 4 columns
        int columns = 4;
        int col = 0;
        
        for (int classId : vehicleClasses) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            ImGui::BeginGroup();
            
            // If we can't toggle this checkbox (because we're at the limit), make it disabled
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            ImGui::EndGroup();
            
            // Layout in columns
            if (++col < columns) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Unindent();
    }
    
    // People Category
    bool peopleCategoryEnabled = detectionManager->getCategoryEnabled(DetectionManager::CATEGORY_PEOPLE);
    if (ImGui::CollapsingHeader("People", peopleCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        bool categoryChecked = peopleCategoryEnabled;
        if (ImGui::Checkbox("Enable People", &categoryChecked)) {
            detectionManager->setCategoryEnabled(DetectionManager::CATEGORY_PEOPLE, categoryChecked);
            
            if (categoryChecked) {
                // Add the person class if not at limit
                int personClassId = 0; // person
                if (selectedClassIds.size() < maxSelectedClasses && 
                    find(selectedClassIds.begin(), selectedClassIds.end(), personClassId) == selectedClassIds.end()) {
                    detectionManager->addSelectedClass(personClassId);
                }
                detectionManager->setCurrentPreset("Custom");
            } else {
                // Remove person class
                detectionManager->removeClassesByCategory(DetectionManager::CATEGORY_PEOPLE);
                detectionManager->setCurrentPreset("Custom");
            }
            detectionManager->updateEnabledClassesFromSelection();
        }
        
        // Show person class
        ImGui::Indent();
        int personClassId = 0; // person
        string className = detectionManager->getClassNameById(personClassId);
        bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), personClassId) != selectedClassIds.end();
        bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
        
        if (!canToggle && !isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }
        
        if (ImGui::Checkbox(className.c_str(), &isSelected)) {
            if (isSelected) {
                detectionManager->addSelectedClass(personClassId);
            } else {
                detectionManager->removeSelectedClass(personClassId);
            }
            detectionManager->setCurrentPreset("Custom");
            detectionManager->updateEnabledClassesFromSelection();
        }
        
        if (!canToggle && !isSelected) {
            ImGui::PopStyleColor();
        }
        
        ImGui::Unindent();
    }
    
    // Animals Category
    bool animalsCategoryEnabled = detectionManager->getCategoryEnabled(DetectionManager::CATEGORY_ANIMALS);
    if (ImGui::CollapsingHeader("Animals", animalsCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        bool categoryChecked = animalsCategoryEnabled;
        if (ImGui::Checkbox("Enable Animals", &categoryChecked)) {
            detectionManager->setCategoryEnabled(DetectionManager::CATEGORY_ANIMALS, categoryChecked);
            
            if (categoryChecked) {
                // Add common animal classes (but respect the max limit)
                vector<int> animalClasses = {14, 15, 16, 17, 18, 19, 20, 21}; // bird, cat, dog, etc.
                for (int classId : animalClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        detectionManager->addSelectedClass(classId);
                    }
                }
                detectionManager->setCurrentPreset("Custom");
            } else {
                // Remove all animal classes
                detectionManager->removeClassesByCategory(DetectionManager::CATEGORY_ANIMALS);
                detectionManager->setCurrentPreset("Custom");
            }
            detectionManager->updateEnabledClassesFromSelection();
        }
        
        // Show common animal classes
        ImGui::Indent();
        vector<int> animalClasses = {14, 15, 16, 17, 18, 19, 20, 21}; // bird, cat, dog, horse, sheep, cow, elephant, bear
        
        // Display animal classes in a grid with 4 columns
        int columns = 4;
        int col = 0;
        
        for (int classId : animalClasses) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            ImGui::BeginGroup();
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            ImGui::EndGroup();
            
            // Layout in columns
            if (++col < columns) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Unindent();
    }
    
    // Objects Category
    bool objectsCategoryEnabled = detectionManager->getCategoryEnabled(DetectionManager::CATEGORY_OBJECTS);
    if (ImGui::CollapsingHeader("Objects", objectsCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        bool categoryChecked = objectsCategoryEnabled;
        if (ImGui::Checkbox("Enable Objects", &categoryChecked)) {
            detectionManager->setCategoryEnabled(DetectionManager::CATEGORY_OBJECTS, categoryChecked);
            
            if (categoryChecked) {
                // Add all object classes from COCO dataset (but respect the max limit)
                vector<int> objectClasses;
                // Add all non-vehicle, non-people, non-animal classes (9-13, 24-79)
                for (int i = 9; i <= 13; i++) objectClasses.push_back(i);     // Street objects
                for (int i = 24; i <= 79; i++) objectClasses.push_back(i);    // All other objects 
                for (int classId : objectClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        detectionManager->addSelectedClass(classId);
                    }
                }
                detectionManager->setCurrentPreset("Custom");
            } else {
                // Remove all object classes
                detectionManager->removeClassesByCategory(DetectionManager::CATEGORY_OBJECTS);
                detectionManager->setCurrentPreset("Custom");
            }
            detectionManager->updateEnabledClassesFromSelection();
        }
        
        // Show object classes (organized by type)
        ImGui::Indent();
        
        ImGui::Text("Street Objects:");
        vector<int> streetObjects = {9, 10, 11, 12, 13}; // traffic light, fire hydrant, stop sign, parking meter, bench
        int col = 0;
        for (int classId : streetObjects) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            if (++col < 3) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Spacing();
        ImGui::Text("Personal Items:");
        vector<int> personalItems = {24, 25, 26, 27, 28, 39, 40, 41}; // backpack, umbrella, handbag, tie, suitcase, bottle, wine glass, cup
        col = 0;
        for (int classId : personalItems) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            if (++col < 4) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Spacing();  
        ImGui::Text("Food & Kitchen:");
        vector<int> foodKitchen = {42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55}; // fork, knife, spoon, bowl, banana, apple, sandwich, orange, broccoli, carrot, hot dog, pizza, donut, cake
        col = 0;
        for (int classId : foodKitchen) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            if (++col < 4) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Spacing();  
        ImGui::Text("Furniture & Electronics:");
        vector<int> furnitureElectronics = {56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79}; // chair through toothbrush
        col = 0;
        for (int classId : furnitureElectronics) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            if (++col < 4) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Spacing();  
        ImGui::Text("Sports Equipment:");
        vector<int> sportsEquipment = {29, 30, 31, 32, 33, 34, 35, 36, 37, 38}; // frisbee, skis, snowboard, sports ball, kite, baseball bat, baseball glove, skateboard, surfboard, tennis racket
        col = 0;
        for (int classId : sportsEquipment) {
            string className = detectionManager->getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    detectionManager->addSelectedClass(classId);
                } else {
                    detectionManager->removeSelectedClass(classId);
                }
                detectionManager->setCurrentPreset("Custom");
                detectionManager->updateEnabledClassesFromSelection();
            }
            
            if (!canToggle && !isSelected) {
                ImGui::PopStyleColor();
            }
            
            if (++col < 4) {
                ImGui::SameLine();
            } else {
                col = 0;
            }
        }
        
        ImGui::Unindent();
    }
    
    // Apply and Reset buttons
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Apply", ImVec2(120, 0))) {
        // Apply selected classes to detection system
        detectionManager->updateEnabledClassesFromSelection();
        ofLogNotice() << "Applied " << selectedClassIds.size() << " classes for detection";
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset to Default", ImVec2(160, 0))) {
        // Reset to default (Vehicles Only)
        detectionManager->applyPreset("Vehicles Only");
        ofLogNotice() << "Reset to default detection classes (Vehicles Only)";
    }
    
    // Help text
    ImGui::Spacing();
    ImGui::TextWrapped("Select up to %d classes for detection. Use presets for quick selection, or choose individual classes.", maxSelectedClasses);
    ImGui::TextWrapped("Detection will only include selected classes. Make sure detection is enabled (D key or checkbox in Main Controls tab).");
}


void UIManager::drawScaleManagerTab() {
    if (!scaleManager) return;
    
    ImGui::Spacing();
    
    // Current Scale Info Panel
    if (ImGui::CollapsingHeader("Current Scale System", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Master musical system info
        if (lineManager) {
            string currentScale = lineManager->getMasterScale();
            int rootNote = lineManager->getMasterRootNote();
            
            // Note names for display
            const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            
            ImGui::Text("Current Root: %s", noteNames[rootNote]);
            ImGui::SameLine();
            ImGui::Text("  |  Scale: %s", currentScale.c_str());
            
            // Microtonality status
            bool isMicrotonal = scaleManager->isMicrotonalityEnabled() && 
                               scaleManager->isScaleMicrotonal(currentScale);
            if (isMicrotonal) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), " [MICROTONAL]");
            }
        }
        
        ImGui::Separator();
        
        // Scale notes display
        if (lineManager && scaleManager) {
            string currentScale = lineManager->getMasterScale();
            auto scaleNotes = scaleManager->getScaleNotes(currentScale);
            
            ImGui::Text("Scale Notes: ");
            ImGui::SameLine();
            
            for (size_t i = 0; i < scaleNotes.size(); i++) {
                if (i > 0) {
                    ImGui::SameLine();
                    ImGui::Text(" - ");
                    ImGui::SameLine();
                }
                
                if (scaleManager->isScaleMicrotonal(currentScale)) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "%.1f¢", scaleNotes[i]);
                } else {
                    ImGui::Text("%.0f", scaleNotes[i]);
                }
            }
        }
        
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    
    // Scale Selection Panel
    if (ImGui::CollapsingHeader("Scale Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Built-in scales section
        ImGui::Text("Built-in Scales:");
        ImGui::Separator();
        
        auto builtinScales = scaleManager->getBuiltinScales();
        int columns = 2;
        
        for (size_t i = 0; i < builtinScales.size(); i++) {
            if (i % columns != 0) ImGui::SameLine();
            
            string scaleName = builtinScales[i];
            bool isSelected = (lineManager && lineManager->getMasterScale() == scaleName);
            
            // Color for microtonal scales
            if (scaleManager->isScaleMicrotonal(scaleName)) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.8f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.5f, 0.9f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.6f, 1.0f, 1.0f));
            } else if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.3f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
            }
            
            if (ImGui::Button(scaleName.c_str(), ImVec2(140, 0))) {
                if (lineManager) {
                    lineManager->setMasterScale(scaleName);
                    ofLogNotice() << "Scale changed to: " << scaleName;
                }
            }
            
            // Pop colors if we pushed them
            if (scaleManager->isScaleMicrotonal(scaleName) || isSelected) {
                ImGui::PopStyleColor(3);
            }
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // Scala files section
        ImGui::Text("Scala Files (.scl):");
        ImGui::Separator();
        
        auto scalaScales = scaleManager->getScalaScales();
        
        if (scalaScales.empty()) {
            string scalaPath = ofToDataPath("scales/");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No Scala files found in scales directory");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Add .scl files to: %s", scalaPath.c_str());
        } else {
            for (size_t i = 0; i < scalaScales.size(); i++) {
                if (i % columns != 0) ImGui::SameLine();
                
                string scaleName = scalaScales[i];
                bool isSelected = (lineManager && lineManager->getMasterScale() == scaleName);
                
                // Scala scales are always microtonal
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.7f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.6f, 0.8f, 1.0f));
                
                if (isSelected) {
                    ImGui::PopStyleColor(3);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.3f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
                }
                
                if (ImGui::Button(scaleName.c_str(), ImVec2(140, 0))) {
                    if (lineManager) {
                        lineManager->setMasterScale(scaleName);
                        ofLogNotice() << "Scala scale changed to: " << scaleName;
                    }
                }
                
                ImGui::PopStyleColor(3);
            }
        }
        
        // Refresh button
        ImGui::Spacing();
        if (ImGui::Button("Refresh Scala Files", ImVec2(150, 0))) {
            scaleManager->refreshScalaFiles();
            ofLogNotice() << "Scala files refreshed";
        }
        
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    
    // Microtonality Settings Panel
    if (ImGui::CollapsingHeader("Microtonality Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        bool microtonalEnabled = scaleManager->isMicrotonalityEnabled();
        if (ImGui::Checkbox("Enable Microtonality (MIDI Pitch Bend)", &microtonalEnabled)) {
            scaleManager->setMicrotonalityEnabled(microtonalEnabled);
            ofLogNotice() << "Microtonality " << (microtonalEnabled ? "enabled" : "disabled");
        }
        
        if (microtonalEnabled) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Pitch Bend Range: ±200 cents (2 semitones)");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Resolution: 14-bit MIDI pitch bend");
            
            // Pitch bend test controls
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Pitch Bend Testing:");
            
            static int testChannel = 1;
            static int testPitchBend = 0;
            
            ImGui::SliderInt("Test Channel", &testChannel, 1, 16);
            ImGui::SliderInt("Pitch Bend", &testPitchBend, -8192, 8191);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(%.1f cents)", 
                              (float)testPitchBend / 8192.0f * 200.0f);
            
            if (ImGui::Button("Send Test Pitch Bend", ImVec2(150, 0))) {
                if (communicationManager) {
                    communicationManager->sendMIDIPitchBend(testPitchBend, testChannel);
                    ofLogNotice() << "Test pitch bend sent: " << testPitchBend << " on channel " << testChannel;
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Reset Pitch Bend", ImVec2(120, 0))) {
                if (communicationManager) {
                    communicationManager->resetPitchBend(testChannel);
                    testPitchBend = 0;
                    ofLogNotice() << "Pitch bend reset on channel " << testChannel;
                }
            }
        } else {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Microtonality disabled - using standard 12-tone MIDI");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "All scales will snap to nearest semitone");
        }
        
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    
    // Scale File Management Panel
    if (ImGui::CollapsingHeader("Scale File Management")) {
        ImGui::Indent();
        
        string scalaDir = ofToDataPath("scales/");
        ImGui::Text("Scala File Directory:");
        ImGui::TextWrapped("%s", scalaDir.c_str());
        ImGui::TextWrapped("Place .scl files in this directory to import custom scales.");
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // Export current scale
        if (ImGui::Button("Export Current Scale as .scl", ImVec2(200, 0))) {
            if (lineManager && scaleManager) {
                string currentScale = lineManager->getMasterScale();
                string filename = "exported_" + currentScale + ".scl";
                bool success = scaleManager->exportScalaFile(currentScale, filename);
                if (success) {
                    ofLogNotice() << "Scale exported to: " << filename;
                } else {
                    ofLogError() << "Failed to export scale: " << currentScale;
                }
            }
        }
        
        ImGui::SameLine();
        
        // Import scale file
        if (ImGui::Button("Import .scl File", ImVec2(120, 0))) {
            // Note: This would typically open a file dialog
            // For now, just refresh the scala files directory
            scaleManager->refreshScalaFiles();
            ofLogNotice() << "Checking for new Scala files...";
        }
        
        ImGui::Spacing();
        
        // File format help
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Scala File Format (.scl):");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "• First line: description");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "• Second line: number of notes");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "• Following lines: ratios (3/2) or cents (701.955)");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "• Lines starting with ! are comments");
        
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    
    // Scale Preview Panel  
    if (ImGui::CollapsingHeader("Scale Preview & Analysis")) {
        ImGui::Indent();
        
        if (lineManager && scaleManager) {
            string currentScale = lineManager->getMasterScale();
            int rootNote = lineManager->getMasterRootNote();
            
            // Get scale intervals for analysis
            auto scaleNotes = scaleManager->getScaleNotes(currentScale);
            
            ImGui::Text("Scale Analysis: %s", currentScale.c_str());
            ImGui::Separator();
            
            // Display intervals in columns (compatible with older ImGui)
            ImGui::Columns(4, "ScaleIntervals", true);
            ImGui::Text("Note #"); ImGui::NextColumn();
            ImGui::Text("Cents"); ImGui::NextColumn();
            ImGui::Text("Ratio"); ImGui::NextColumn();
            ImGui::Text("MIDI Note"); ImGui::NextColumn();
            ImGui::Separator();
            
            for (size_t i = 0; i < scaleNotes.size(); i++) {
                ImGui::Text("%d", (int)i + 1);
                ImGui::NextColumn();
                
                ImGui::Text("%.2f¢", scaleNotes[i]);
                ImGui::NextColumn();
                
                if (scaleNotes[i] == 0.0f) {
                    ImGui::Text("1/1");
                } else {
                    // Approximate ratio display
                    float ratio = pow(2.0f, scaleNotes[i] / 1200.0f);
                    ImGui::Text("%.3f", ratio);
                }
                ImGui::NextColumn();
                
                // Calculate MIDI note with microtonal adjustment
                if (scaleManager->isMicrotonalityEnabled()) {
                    ScaleManager::MicrotonalNote microNote = 
                        scaleManager->getMicrotonalNote(currentScale, i, rootNote, 4);
                    if (microNote.pitchBend != 0) {
                        ImGui::Text("%d + %.0f¢", microNote.midiNote, microNote.centsOffset);
                    } else {
                        ImGui::Text("%d", microNote.midiNote);
                    }
                } else {
                    int midiNote = rootNote + 60 + (int)round(scaleNotes[i] / 100.0f);
                    ImGui::Text("%d", midiNote);
                }
                ImGui::NextColumn();
            }
            
            ImGui::Columns(1); // Reset to single column
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No scale selected");
        }
        
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    
    // Color legend
    ImGui::Separator();
    ImGui::Text("Scale Types:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "12-tone");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.6f, 1.0f, 1.0f), "Microtonal");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.6f, 1.0f), "Scala");
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Selected");
}

void UIManager::handleWindowResize(int width, int height) {
    ofLogNotice() << "UIManager: Window resized to " << width << "x" << height;
}