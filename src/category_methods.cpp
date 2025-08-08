//--------------------------------------------------------------
// Class Detection Category System Methods
//--------------------------------------------------------------

#include "ofApp.h"

void ofApp::initializeCategories() {
    // Initialize category enabled flags
    categoryEnabled.resize(CATEGORY_COUNT, false);
    categoryEnabled[CATEGORY_VEHICLES] = true;  // Vehicles enabled by default
    
    // Initialize enabled classes vector (80 elements for all YOLO classes)
    enabledClasses.resize(80, false);
    
    // Apply the default "Vehicles Only" preset
    currentPreset = "Vehicles Only";
    applyPreset(currentPreset);
}

void ofApp::applyPreset(const string& presetName) {
    // Clear current selection
    selectedClassIds.clear();
    
    // Reset categories
    for (int i = 0; i < CATEGORY_COUNT; i++) {
        categoryEnabled[i] = false;
    }
    
    // Apply the selected preset
    if (presetName == "Vehicles Only") {
        // Enable Vehicles category
        categoryEnabled[CATEGORY_VEHICLES] = true;
        
        // Select specific vehicle classes
        vector<int> vehicleIds = {1, 2, 3, 5, 7};  // bicycle, car, motorcycle, bus, truck
        selectedClassIds.insert(selectedClassIds.end(), vehicleIds.begin(), vehicleIds.end());
        
    } else if (presetName == "People & Animals") {
        // Enable People and Animals categories
        categoryEnabled[CATEGORY_PEOPLE] = true;
        categoryEnabled[CATEGORY_ANIMALS] = true;
        
        // Add person (0)
        selectedClassIds.push_back(0);
        
        // Add common animals (bird, cat, dog, horse, sheep, cow, etc.)
        vector<int> animalIds = {14, 15, 16, 17, 18, 19, 20, 21};
        selectedClassIds.insert(selectedClassIds.end(), animalIds.begin(), animalIds.end());
        
    } else if (presetName == "Everything Moving") {
        // Enable all categories
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            categoryEnabled[i] = true;
        }
        
        // Add all potentially moving objects
        vector<int> movingObjects = {
            0,  // person
            1, 2, 3, 4, 5, 6, 7, 8,  // vehicles  
            14, 15, 16, 17, 18, 19, 20, 21, 22, 23,  // animals
            32, 33, 37  // sports ball, kite, surfboard (movable objects)
        };
        selectedClassIds.insert(selectedClassIds.end(), movingObjects.begin(), movingObjects.end());
        
    } else if (presetName == "Custom") {
        // Custom preset keeps the current selection
        // Just update the UI to reflect custom mode
    }
    
    // Enforce the max number of selected classes
    if (selectedClassIds.size() > maxSelectedClasses) {
        selectedClassIds.resize(maxSelectedClasses);
    }
    
    // Update enabled classes vector based on selection
    updateEnabledClassesFromSelection();
    
    ofLogNotice() << "Applied preset: " << presetName 
                 << " with " << selectedClassIds.size() << " classes selected";
}

void ofApp::updateEnabledClassesFromSelection() {
    // Reset all classes to disabled
    std::fill(enabledClasses.begin(), enabledClasses.end(), false);
    
    // Enable only the selected class IDs
    for (int classId : selectedClassIds) {
        if (classId >= 0 && classId < enabledClasses.size()) {
            enabledClasses[classId] = true;
        }
    }
}

string ofApp::getClassNameById(int classId) {
    // Return class name from classNames vector if available
    if (classId >= 0 && classId < classNames.size()) {
        return classNames[classId];
    }
    
    // Fallback to basic class ID if not found
    return "Class " + ofToString(classId);
}

ofApp::DetectionCategory ofApp::getCategoryForClass(int classId) {
    // Map YOLO COCO class IDs to our categories
    if (classId == 0) {
        return CATEGORY_PEOPLE;  // person
    } else if (classId >= 1 && classId <= 8) {
        return CATEGORY_VEHICLES;  // bicycle to boat
    } else if (classId >= 14 && classId <= 23) {
        return CATEGORY_ANIMALS;  // bird to zebra
    } else {
        return CATEGORY_OBJECTS;  // everything else
    }
}

vector<int> ofApp::getClassesInCategory(DetectionCategory category) {
    vector<int> result;
    
    switch (category) {
        case CATEGORY_VEHICLES:
            // Vehicles: bicycle(1), car(2), motorcycle(3), airplane(4), bus(5), train(6), truck(7), boat(8)
            result = {1, 2, 3, 4, 5, 6, 7, 8};
            break;
            
        case CATEGORY_PEOPLE:
            // Just person(0)
            result = {0};
            break;
            
        case CATEGORY_ANIMALS:
            // Animals: bird(14) through zebra(23)
            for (int i = 14; i <= 23; i++) {
                result.push_back(i);
            }
            break;
            
        case CATEGORY_OBJECTS:
            // Everything else
            for (int i = 0; i < 80; i++) {
                if (i != 0 && (i < 1 || i > 8) && (i < 14 || i > 23)) {
                    result.push_back(i);
                }
            }
            break;
            
        default:
            break;
    }
    
    return result;
}
