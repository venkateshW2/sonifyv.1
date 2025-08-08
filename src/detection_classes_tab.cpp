//--------------------------------------------------------------
// Detection Classes Tab Implementation
//--------------------------------------------------------------

#include "ofApp.h"

void ofApp::drawDetectionClassesTab() {
    // Presets row
    ImGui::Text("Quick Presets:");
    
    // Create preset buttons in a row
    float availWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = (availWidth - 3 * ImGui::GetStyle().ItemSpacing.x) / 4; // 4 buttons with 3 spaces between
    
    if (ImGui::Button("Vehicles Only", ImVec2(buttonWidth, 0))) {
        currentPreset = "Vehicles Only";
        applyPreset(currentPreset);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("People & Animals", ImVec2(buttonWidth, 0))) {
        currentPreset = "People & Animals";
        applyPreset(currentPreset);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Everything Moving", ImVec2(buttonWidth, 0))) {
        currentPreset = "Everything Moving";
        applyPreset(currentPreset);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Custom", ImVec2(buttonWidth, 0))) {
        currentPreset = "Custom";
        // No auto-apply for custom - user will select classes manually
    }
    
    ImGui::Separator();
    
    // Display selected class count
    ImGui::Text("Selected: %d/%d classes", (int)selectedClassIds.size(), maxSelectedClasses);
    
    // Categories sections
    ImGui::Spacing();
    
    // Vehicles Category
    bool vehiclesCategoryEnabled = categoryEnabled[CATEGORY_VEHICLES];
    if (ImGui::CollapsingHeader("Vehicles", vehiclesCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        // Only allow toggling the whole category if we're not at the limit, or if this category is already enabled
        bool canToggleCategory = (selectedClassIds.size() < maxSelectedClasses) || vehiclesCategoryEnabled;
        
        // Show category checkbox - FIXED: use a local bool for checkbox
        bool categoryChecked = vehiclesCategoryEnabled;
        if (ImGui::Checkbox("Enable Vehicles", &categoryChecked)) {
            categoryEnabled[CATEGORY_VEHICLES] = categoryChecked;
            
            if (categoryEnabled[CATEGORY_VEHICLES]) {
                // Add all vehicle classes (but respect the max limit)
                vector<int> vehicleClasses = getClassesInCategory(CATEGORY_VEHICLES);
                for (int classId : vehicleClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                }
                currentPreset = "Custom";
            } else {
                // Remove all vehicle classes
                selectedClassIds.erase(
                    remove_if(selectedClassIds.begin(), selectedClassIds.end(),
                        [this](int classId) {
                            return getCategoryForClass(classId) == CATEGORY_VEHICLES;
                        }),
                    selectedClassIds.end()
                );
                currentPreset = "Custom";
            }
            updateEnabledClassesFromSelection();
        }
        
        // Show individual vehicle classes
        ImGui::Indent();
        vector<int> vehicleClasses = {
            1, // bicycle
            2, // car
            3, // motorcycle
            4, // airplane
            5, // bus
            6, // train
            7, // truck
            8  // boat
        };
        
        // Display vehicle classes in a grid with 4 columns
        int columns = 4;
        int col = 0;
        
        for (int classId : vehicleClasses) {
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            ImGui::BeginGroup();
            
            // If we can't toggle this checkbox (because we're at the limit), make it disabled
            // FIXED: removed BeginDisabled() which isn't available in the current ImGui version
            // Just use different colors to indicate disabled state
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    // Add to selected classes if not already there
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    // Remove from selected classes
                    selectedClassIds.erase(
                        remove(selectedClassIds.begin(), selectedClassIds.end(), classId),
                        selectedClassIds.end()
                    );
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
    bool peopleCategoryEnabled = categoryEnabled[CATEGORY_PEOPLE];
    if (ImGui::CollapsingHeader("People", peopleCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        // Only allow toggling if we're not at the limit, or if this category is already enabled
        bool canToggleCategory = (selectedClassIds.size() < maxSelectedClasses) || peopleCategoryEnabled;
        
        // Show category checkbox - FIXED: use a local bool for checkbox
        bool categoryChecked = peopleCategoryEnabled;
        if (ImGui::Checkbox("Enable People", &categoryChecked)) {
            categoryEnabled[CATEGORY_PEOPLE] = categoryChecked;
            
            if (categoryEnabled[CATEGORY_PEOPLE]) {
                // Add the person class if not at limit
                int personClassId = 0; // person
                if (selectedClassIds.size() < maxSelectedClasses && 
                    find(selectedClassIds.begin(), selectedClassIds.end(), personClassId) == selectedClassIds.end()) {
                    selectedClassIds.push_back(personClassId);
                }
                currentPreset = "Custom";
            } else {
                // Remove person class
                selectedClassIds.erase(
                    remove_if(selectedClassIds.begin(), selectedClassIds.end(),
                        [this](int classId) {
                            return getCategoryForClass(classId) == CATEGORY_PEOPLE;
                        }),
                    selectedClassIds.end()
                );
                currentPreset = "Custom";
            }
            updateEnabledClassesFromSelection();
        }
        
        // Show person class
        ImGui::Indent();
        int personClassId = 0; // person
        string className = getClassNameById(personClassId);
        bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), personClassId) != selectedClassIds.end();
        bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
        
        // FIXED: removed BeginDisabled() which isn't available in the current ImGui version
        // Just use different colors to indicate disabled state
        if (!canToggle && !isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }
        
        if (ImGui::Checkbox(className.c_str(), &isSelected)) {
            if (isSelected) {
                // Add to selected classes if not already there
                if (find(selectedClassIds.begin(), selectedClassIds.end(), personClassId) == selectedClassIds.end()) {
                    selectedClassIds.push_back(personClassId);
                }
            } else {
                // Remove from selected classes
                selectedClassIds.erase(
                    remove(selectedClassIds.begin(), selectedClassIds.end(), personClassId),
                    selectedClassIds.end()
                );
            }
            currentPreset = "Custom";
            updateEnabledClassesFromSelection();
        }
        
        if (!canToggle && !isSelected) {
            ImGui::PopStyleColor();
        }
        
        ImGui::Unindent();
    }
    
    // Animals Category
    bool animalsCategoryEnabled = categoryEnabled[CATEGORY_ANIMALS];
    if (ImGui::CollapsingHeader("Animals", animalsCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        // Only allow toggling if we're not at the limit, or if this category is already enabled
        bool canToggleCategory = (selectedClassIds.size() < maxSelectedClasses) || animalsCategoryEnabled;
        
        // Show category checkbox - FIXED: use a local bool for checkbox
        bool categoryChecked = animalsCategoryEnabled;
        if (ImGui::Checkbox("Enable Animals", &categoryChecked)) {
            categoryEnabled[CATEGORY_ANIMALS] = categoryChecked;
            
            if (categoryEnabled[CATEGORY_ANIMALS]) {
                // Add common animal classes (but respect the max limit)
                vector<int> animalClasses = {14, 15, 16, 17, 18, 19, 20, 21}; // bird, cat, dog, etc.
                for (int classId : animalClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                }
                currentPreset = "Custom";
            } else {
                // Remove all animal classes
                selectedClassIds.erase(
                    remove_if(selectedClassIds.begin(), selectedClassIds.end(),
                        [this](int classId) {
                            return getCategoryForClass(classId) == CATEGORY_ANIMALS;
                        }),
                    selectedClassIds.end()
                );
                currentPreset = "Custom";
            }
            updateEnabledClassesFromSelection();
        }
        
        // Show common animal classes
        ImGui::Indent();
        vector<int> animalClasses = {
            14, // bird
            15, // cat
            16, // dog
            17, // horse
            18, // sheep
            19, // cow
            20, // elephant
            21  // bear
        };
        
        // Display animal classes in a grid with 4 columns
        int columns = 4;
        int col = 0;
        
        for (int classId : animalClasses) {
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            ImGui::BeginGroup();
            
            // FIXED: removed BeginDisabled() which isn't available in the current ImGui version
            // Just use different colors to indicate disabled state
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    // Add to selected classes if not already there
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    // Remove from selected classes
                    selectedClassIds.erase(
                        remove(selectedClassIds.begin(), selectedClassIds.end(), classId),
                        selectedClassIds.end()
                    );
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
    bool objectsCategoryEnabled = categoryEnabled[CATEGORY_OBJECTS];
    if (ImGui::CollapsingHeader("Objects", objectsCategoryEnabled ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        // Only allow toggling if we're not at the limit, or if this category is already enabled
        bool canToggleCategory = (selectedClassIds.size() < maxSelectedClasses) || objectsCategoryEnabled;
        
        // Show category checkbox - FIXED: use a local bool for checkbox
        bool categoryChecked = objectsCategoryEnabled;
        if (ImGui::Checkbox("Enable Objects", &categoryChecked)) {
            categoryEnabled[CATEGORY_OBJECTS] = categoryChecked;
            
            if (categoryEnabled[CATEGORY_OBJECTS]) {
                // Add all object classes from COCO dataset (but respect the max limit)
                vector<int> objectClasses;
                // Add all non-vehicle, non-people, non-animal classes (9-13, 24-79)
                for (int i = 9; i <= 13; i++) objectClasses.push_back(i);     // Street objects
                for (int i = 24; i <= 79; i++) objectClasses.push_back(i);    // All other objects 
                for (int classId : objectClasses) {
                    if (selectedClassIds.size() < maxSelectedClasses && 
                        find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                }
                currentPreset = "Custom";
            } else {
                // Remove all object classes
                selectedClassIds.erase(
                    remove_if(selectedClassIds.begin(), selectedClassIds.end(),
                        [this](int classId) {
                            return getCategoryForClass(classId) == CATEGORY_OBJECTS;
                        }),
                    selectedClassIds.end()
                );
                currentPreset = "Custom";
            }
            updateEnabledClassesFromSelection();
        }
        
        // Show object classes (organized by type)
        ImGui::Indent();
        
        ImGui::Text("Street Objects:");
        vector<int> streetObjects = {9, 10, 11, 12, 13}; // traffic light, fire hydrant, stop sign, parking meter, bench
        int col = 0;
        for (int classId : streetObjects) {
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    selectedClassIds.erase(remove(selectedClassIds.begin(), selectedClassIds.end(), classId), selectedClassIds.end());
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    selectedClassIds.erase(remove(selectedClassIds.begin(), selectedClassIds.end(), classId), selectedClassIds.end());
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    selectedClassIds.erase(remove(selectedClassIds.begin(), selectedClassIds.end(), classId), selectedClassIds.end());
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    selectedClassIds.erase(remove(selectedClassIds.begin(), selectedClassIds.end(), classId), selectedClassIds.end());
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
            string className = getClassNameById(classId);
            bool isSelected = find(selectedClassIds.begin(), selectedClassIds.end(), classId) != selectedClassIds.end();
            bool canToggle = isSelected || selectedClassIds.size() < maxSelectedClasses;
            
            if (!canToggle && !isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            if (ImGui::Checkbox(className.c_str(), &isSelected)) {
                if (isSelected) {
                    if (find(selectedClassIds.begin(), selectedClassIds.end(), classId) == selectedClassIds.end()) {
                        selectedClassIds.push_back(classId);
                    }
                } else {
                    selectedClassIds.erase(remove(selectedClassIds.begin(), selectedClassIds.end(), classId), selectedClassIds.end());
                }
                currentPreset = "Custom";
                updateEnabledClassesFromSelection();
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
        updateEnabledClassesFromSelection();
        ofLogNotice() << "Applied " << selectedClassIds.size() << " classes for detection";
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset to Default", ImVec2(160, 0))) {
        // Reset to default (Vehicles Only)
        currentPreset = "Vehicles Only";
        applyPreset(currentPreset);
        ofLogNotice() << "Reset to default detection classes (Vehicles Only)";
    }
    
    // Help text
    ImGui::Spacing();
    ImGui::TextWrapped("Select up to %d classes for detection. Use presets for quick selection, or choose individual classes.", maxSelectedClasses);
    ImGui::TextWrapped("Detection will only include selected classes. Make sure detection is enabled (D key or checkbox in Main Controls tab).");
}
