#include "ScaleManager.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

ScaleManager::ScaleManager() {
    currentScaleName = "Major";
    microtonalityEnabled = true;
    scalaDirectory = ofToDataPath("scales/");
}

ScaleManager::~ScaleManager() {
}

void ScaleManager::setup() {
    // Ensure scales directory exists
    ofDirectory scalesDir(scalaDirectory);
    if (!scalesDir.exists()) {
        scalesDir.create(true);
        ofLogNotice() << "ScaleManager: Created scales directory: " << scalaDirectory;
    }
    
    // Initialize all built-in scales
    initializeBuiltinScales();
    
    // Load any existing Scala files
    vector<string> scalaFiles = getScalaFilesInDirectory(scalaDirectory);
    for (const string& file : scalaFiles) {
        loadScalaFile(scalaDirectory + file);
    }
    
    ofLogNotice() << "ScaleManager: Setup complete - " << scales.size() << " scales available";
}

void ScaleManager::update() {
    // Nothing to update in real-time currently
}

// =============================================================================
// SCALE MANAGEMENT
// =============================================================================

vector<string> ScaleManager::getAvailableScaleNames() const {
    vector<string> names;
    for (const auto& pair : scales) {
        names.push_back(pair.first);
    }
    return names;
}

const ScaleManager::Scale* ScaleManager::getScale(const string& scaleName) const {
    auto it = scales.find(scaleName);
    if (it != scales.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool ScaleManager::setCurrentScale(const string& scaleName) {
    if (scales.find(scaleName) != scales.end()) {
        currentScaleName = scaleName;
        ofLogNotice() << "ScaleManager: Current scale set to: " << scaleName;
        return true;
    }
    ofLogWarning() << "ScaleManager: Scale not found: " << scaleName;
    return false;
}

vector<string> ScaleManager::getScaleNoteNames(const string& scaleName, int rootNote) const {
    const Scale* scale = getScale(scaleName);
    if (!scale) {
        return {"C"}; // Fallback
    }
    
    vector<string> noteNames;
    const string noteLetters[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    // Add root note
    noteNames.push_back(noteLetters[rootNote % 12]);
    
    // Add scale intervals
    for (const ScaleInterval& interval : scale->intervals) {
        if (interval.cents >= 1200.0f) break; // Don't go past octave
        
        // Convert cents to semitones for note name approximation
        int semitones = round(interval.cents / 100.0f);
        int noteIndex = (rootNote + semitones) % 12;
        
        if (scale->isMicrotonal && fmod(interval.cents, 100.0f) > 10.0f) {
            // Microtonal interval - show cents offset
            noteNames.push_back(noteLetters[noteIndex] + "+" + ofToString((int)interval.cents, 0) + "¢");
        } else {
            noteNames.push_back(noteLetters[noteIndex]);
        }
    }
    
    return noteNames;
}

ScaleManager::MicrotonalNote ScaleManager::getMicrotonalNote(const string& scaleName, int scaleIndex, 
                                                           int rootNote, int octave) const {
    const Scale* scale = getScale(scaleName);
    if (!scale || scaleIndex < 0) {
        return {60, 0, 0.0f}; // Fallback to middle C
    }
    
    MicrotonalNote result;
    result.midiNote = scale->baseNoteMidi + rootNote + (octave * 12);
    result.centsOffset = 0.0f;
    result.pitchBend = 0;
    
    if (scaleIndex == 0) {
        // Root note - no offset
        return result;
    }
    
    if (scaleIndex > 0 && scaleIndex <= scale->intervals.size()) {
        const ScaleInterval& interval = scale->intervals[scaleIndex - 1];
        result.centsOffset = interval.cents;
        
        // Calculate base MIDI note and pitch bend
        int semitones = (int)(interval.cents / 100.0f);
        float remainingCents = interval.cents - (semitones * 100.0f);
        
        result.midiNote += semitones;
        
        if (microtonalityEnabled && scale->isMicrotonal) {
            result.pitchBend = centsToPitchBend(remainingCents);
        }
    }
    
    return result;
}

vector<int> ScaleManager::getScaleIntervals(const string& scaleName) const {
    // Backward compatibility - return 12-tone equal temperament approximations
    const Scale* scale = getScale(scaleName);
    if (!scale) {
        return {0, 2, 4, 5, 7, 9, 11}; // Major scale fallback
    }
    
    vector<int> intervals = {0}; // Root
    for (const ScaleInterval& interval : scale->intervals) {
        if (interval.cents >= 1200.0f) break; // Don't exceed octave
        int semitones = round(interval.cents / 100.0f);
        intervals.push_back(semitones);
    }
    
    return intervals;
}

// =============================================================================
// BUILT-IN SCALES
// =============================================================================

void ScaleManager::initializeBuiltinScales() {
    scales.clear();
    
    // Traditional Western scales (12-tone equal temperament)
    createMajorScale();
    createMinorScale();
    createPentatonicScale();
    createBluesScale();
    createChromaticScale();
    createDorian();
    createPhrygian();
    createLydian();
    createMixolydian();
    createAeolian();
    createLocrian();
    
    // Microtonal scales
    createJustIntonationScale();
    createBohlenPierceScale();
    create19ToneEqualScale();
    create31ToneEqualScale();
    
    ofLogNotice() << "ScaleManager: Initialized " << scales.size() << " built-in scales";
}

void ScaleManager::createMajorScale() {
    Scale scale;
    scale.name = "Major";
    scale.description = "Major scale (Ionian mode) - happy, bright character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    // Major scale intervals in cents (12-tone equal temperament)
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {400.0f, 5.0f/4.0f, "major third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {900.0f, 27.0f/16.0f, "major sixth"},
        {1100.0f, 15.0f/8.0f, "major seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createMinorScale() {
    Scale scale;
    scale.name = "Minor";
    scale.description = "Natural minor scale (Aeolian mode) - sad, introspective character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {800.0f, 8.0f/5.0f, "minor sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createPentatonicScale() {
    Scale scale;
    scale.name = "Pentatonic";
    scale.description = "Major pentatonic scale - universal, folk character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {400.0f, 5.0f/4.0f, "major third"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {900.0f, 27.0f/16.0f, "major sixth"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createBluesScale() {
    Scale scale;
    scale.name = "Blues";
    scale.description = "Blues scale - expressive, soulful character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {600.0f, 7.0f/5.0f, "tritone"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createChromaticScale() {
    Scale scale;
    scale.name = "Chromatic";
    scale.description = "All 12 semitones - complete chromatic spectrum";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {100.0f, pow(2.0f, 1.0f/12.0f), "minor second"},
        {200.0f, pow(2.0f, 2.0f/12.0f), "major second"},
        {300.0f, pow(2.0f, 3.0f/12.0f), "minor third"},
        {400.0f, pow(2.0f, 4.0f/12.0f), "major third"},
        {500.0f, pow(2.0f, 5.0f/12.0f), "perfect fourth"},
        {600.0f, pow(2.0f, 6.0f/12.0f), "tritone"},
        {700.0f, pow(2.0f, 7.0f/12.0f), "perfect fifth"},
        {800.0f, pow(2.0f, 8.0f/12.0f), "minor sixth"},
        {900.0f, pow(2.0f, 9.0f/12.0f), "major sixth"},
        {1000.0f, pow(2.0f, 10.0f/12.0f), "minor seventh"},
        {1100.0f, pow(2.0f, 11.0f/12.0f), "major seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createDorian() {
    Scale scale;
    scale.name = "Dorian";
    scale.description = "Dorian mode - minor with raised 6th, jazzy character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {900.0f, 27.0f/16.0f, "major sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createPhrygian() {
    Scale scale;
    scale.name = "Phrygian";
    scale.description = "Phrygian mode - minor with flat 2nd, Spanish/Middle Eastern character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {100.0f, 16.0f/15.0f, "minor second"},
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {800.0f, 8.0f/5.0f, "minor sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createLydian() {
    Scale scale;
    scale.name = "Lydian";
    scale.description = "Lydian mode - major with raised 4th, dreamy character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {400.0f, 5.0f/4.0f, "major third"},
        {600.0f, 45.0f/32.0f, "augmented fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {900.0f, 27.0f/16.0f, "major sixth"},
        {1100.0f, 15.0f/8.0f, "major seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createMixolydian() {
    Scale scale;
    scale.name = "Mixolydian";
    scale.description = "Mixolydian mode - major with flat 7th, bluesy character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {400.0f, 5.0f/4.0f, "major third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {900.0f, 27.0f/16.0f, "major sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createAeolian() {
    Scale scale;
    scale.name = "Aeolian";
    scale.description = "Aeolian mode (Natural Minor) - melancholic character";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {200.0f, 9.0f/8.0f, "major second"},
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {700.0f, 3.0f/2.0f, "perfect fifth"},
        {800.0f, 8.0f/5.0f, "minor sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createLocrian() {
    Scale scale;
    scale.name = "Locrian";
    scale.description = "Locrian mode - diminished character, theoretical";
    scale.isMicrotonal = false;
    scale.source = "builtin";
    
    scale.intervals = {
        {100.0f, 16.0f/15.0f, "minor second"},
        {300.0f, 6.0f/5.0f, "minor third"},
        {500.0f, 4.0f/3.0f, "perfect fourth"},
        {600.0f, 64.0f/45.0f, "diminished fifth"},
        {800.0f, 8.0f/5.0f, "minor sixth"},
        {1000.0f, 16.0f/9.0f, "minor seventh"}
    };
    
    scales[scale.name] = scale;
}

// =============================================================================
// MICROTONAL SCALES
// =============================================================================

void ScaleManager::createJustIntonationScale() {
    Scale scale;
    scale.name = "Just Intonation";
    scale.description = "Just intonation major scale - pure harmonic ratios";
    scale.isMicrotonal = true;
    scale.source = "builtin";
    
    // Just intonation intervals (pure ratios)
    scale.intervals = {
        {ratioToCents(9.0f/8.0f), 9.0f/8.0f, "major second (9:8)"},
        {ratioToCents(5.0f/4.0f), 5.0f/4.0f, "major third (5:4)"},
        {ratioToCents(4.0f/3.0f), 4.0f/3.0f, "perfect fourth (4:3)"},
        {ratioToCents(3.0f/2.0f), 3.0f/2.0f, "perfect fifth (3:2)"},
        {ratioToCents(5.0f/3.0f), 5.0f/3.0f, "major sixth (5:3)"},
        {ratioToCents(15.0f/8.0f), 15.0f/8.0f, "major seventh (15:8)"}
    };
    
    scales[scale.name] = scale;
}

void ScaleManager::createBohlenPierceScale() {
    Scale scale;
    scale.name = "Bohlen-Pierce";
    scale.description = "13-tone equal temperament, 3:1 tritave";
    scale.isMicrotonal = true;
    scale.source = "builtin";
    
    // 13 equal divisions of tritave (3:1 ratio = 1901.955 cents)
    float tritaveCents = 1901.955f;
    scale.intervals.clear();
    for (int i = 1; i < 13; i++) {
        float cents = (i * tritaveCents) / 13.0f;
        scale.intervals.push_back({cents, pow(3.0f, i/13.0f), "BP step " + ofToString(i)});
    }
    
    scales[scale.name] = scale;
}

void ScaleManager::create19ToneEqualScale() {
    Scale scale;
    scale.name = "19-Tone Equal";
    scale.description = "19 equal divisions of the octave";
    scale.isMicrotonal = true;
    scale.source = "builtin";
    
    // 19 equal divisions of octave (1200 cents)
    for (int i = 1; i < 19; i++) {
        float cents = (i * 1200.0f) / 19.0f;
        scale.intervals.push_back({cents, pow(2.0f, i/19.0f), "19ED step " + ofToString(i)});
    }
    
    scales[scale.name] = scale;
}

void ScaleManager::create31ToneEqualScale() {
    Scale scale;
    scale.name = "31-Tone Equal";
    scale.description = "31 equal divisions of the octave - quarter-comma meantone approximation";
    scale.isMicrotonal = true;
    scale.source = "builtin";
    
    // 31 equal divisions of octave (1200 cents)
    for (int i = 1; i < 31; i++) {
        float cents = (i * 1200.0f) / 31.0f;
        scale.intervals.push_back({cents, pow(2.0f, i/31.0f), "31ED step " + ofToString(i)});
    }
    
    scales[scale.name] = scale;
}

// =============================================================================
// HELPER METHODS
// =============================================================================

float ScaleManager::centsToRatio(float cents) {
    return pow(2.0f, cents / 1200.0f);
}

float ScaleManager::ratioToCents(float ratio) {
    return 1200.0f * log2(ratio);
}

int ScaleManager::centsToPitchBend(float cents) const {
    // MIDI pitch bend range is typically ±200 cents (2 semitones)
    // Pitch bend values: -8192 to +8191 (14-bit)
    float bendRange = 200.0f; // cents
    float normalizedCents = ofClamp(cents, -bendRange, bendRange) / bendRange;
    return (int)(normalizedCents * 8191.0f);
}

string ScaleManager::formatNoteName(int midiNote) {
    const string noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;
    return noteNames[noteIndex] + ofToString(octave);
}

bool ScaleManager::validateScaleIntervals(const vector<ScaleInterval>& intervals) {
    if (intervals.empty()) return false;
    
    float lastCents = 0.0f;
    for (const ScaleInterval& interval : intervals) {
        if (interval.cents <= lastCents) {
            return false; // Intervals must be ascending
        }
        if (interval.cents >= 2400.0f) {
            return false; // Don't exceed 2 octaves
        }
        lastCents = interval.cents;
    }
    return true;
}

// =============================================================================
// CONFIGURATION
// =============================================================================

void ScaleManager::saveToJSON(ofxJSONElement& json) {
    json["currentScale"] = currentScaleName;
    json["microtonalityEnabled"] = microtonalityEnabled;
    json["scalaDirectory"] = scalaDirectory;
    
    // Save custom scales (don't save built-in scales)
    ofxJSONElement customScalesJson;
    for (const auto& pair : scales) {
        if (pair.second.source == "custom" || pair.second.source == "scala") {
            ofxJSONElement scaleJson;
            scaleJson["name"] = pair.second.name;
            scaleJson["description"] = pair.second.description;
            scaleJson["isMicrotonal"] = pair.second.isMicrotonal;
            scaleJson["source"] = pair.second.source;
            scaleJson["filename"] = pair.second.filename;
            
            ofxJSONElement intervalsJson;
            for (size_t i = 0; i < pair.second.intervals.size(); i++) {
                const ScaleInterval& interval = pair.second.intervals[i];
                ofxJSONElement intervalJson;
                intervalJson["cents"] = interval.cents;
                intervalJson["ratio"] = interval.ratio;
                intervalJson["description"] = interval.description;
                intervalsJson[(int)i] = intervalJson;
            }
            scaleJson["intervals"] = intervalsJson;
            customScalesJson[pair.first] = scaleJson;
        }
    }
    json["customScales"] = customScalesJson;
}

void ScaleManager::loadFromJSON(const ofxJSONElement& json) {
    if (json.isMember("currentScale")) {
        string scaleName = json["currentScale"].asString();
        setCurrentScale(scaleName);
    }
    
    if (json.isMember("microtonalityEnabled")) {
        microtonalityEnabled = json["microtonalityEnabled"].asBool();
    }
    
    if (json.isMember("scalaDirectory")) {
        scalaDirectory = json["scalaDirectory"].asString();
    }
    
    // Load custom scales
    if (json.isMember("customScales")) {
        const ofxJSONElement& customScalesJson = json["customScales"];
        for (auto it = customScalesJson.begin(); it != customScalesJson.end(); ++it) {
            const ofxJSONElement& scaleJson = *it;
            
            Scale scale;
            if (scaleJson.isMember("name")) {
                scale.name = scaleJson["name"].asString();
            }
            if (scaleJson.isMember("description")) {
                scale.description = scaleJson["description"].asString();
            }
            if (scaleJson.isMember("isMicrotonal")) {
                scale.isMicrotonal = scaleJson["isMicrotonal"].asBool();
            }
            if (scaleJson.isMember("source")) {
                scale.source = scaleJson["source"].asString();
            }
            if (scaleJson.isMember("filename")) {
                scale.filename = scaleJson["filename"].asString();
            }
            
            if (scaleJson.isMember("intervals")) {
                const ofxJSONElement& intervalsJson = scaleJson["intervals"];
                for (auto intervalIt = intervalsJson.begin(); intervalIt != intervalsJson.end(); ++intervalIt) {
                    const ofxJSONElement& intervalJson = *intervalIt;
                    ScaleInterval interval;
                    if (intervalJson.isMember("cents")) {
                        interval.cents = intervalJson["cents"].asFloat();
                    }
                    if (intervalJson.isMember("ratio")) {
                        interval.ratio = intervalJson["ratio"].asFloat();
                    }
                    if (intervalJson.isMember("description")) {
                        interval.description = intervalJson["description"].asString();
                    }
                    scale.intervals.push_back(interval);
                }
            }
            
            if (!scale.name.empty()) {
                scales[scale.name] = scale;
            }
        }
    }
}

void ScaleManager::setDefaults() {
    currentScaleName = "Major";
    microtonalityEnabled = true;
    scalaDirectory = ofToDataPath("scales/");
    initializeBuiltinScales();
}

// =============================================================================
// SCALA FILE SUPPORT
// =============================================================================

bool ScaleManager::loadScalaFile(const string& filepath) {
    if (!fileExists(filepath)) {
        ofLogWarning() << "ScaleManager: Scala file not found: " << filepath;
        return false;
    }
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        ofLogWarning() << "ScaleManager: Could not open Scala file: " << filepath;
        return false;
    }
    
    string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    vector<ScaleInterval> intervals = parseScalaContent(content);
    if (intervals.empty()) {
        ofLogWarning() << "ScaleManager: Invalid Scala file format: " << filepath;
        return false;
    }
    
    // Extract filename for scale name
    size_t lastSlash = filepath.find_last_of("/\\");
    size_t lastDot = filepath.find_last_of(".");
    string filename = (lastSlash != string::npos) ? filepath.substr(lastSlash + 1) : filepath;
    string scaleName = (lastDot != string::npos) ? filename.substr(0, lastDot) : filename;
    
    // Create scale
    Scale scale;
    scale.name = scaleName;
    scale.filename = filename;
    scale.intervals = intervals;
    scale.isMicrotonal = true; // Assume Scala files are microtonal
    scale.source = "scala";
    scale.description = "Imported from " + filename;
    
    scales[scaleName] = scale;
    
    ofLogNotice() << "ScaleManager: Loaded Scala file: " << scaleName 
                 << " (" << intervals.size() << " intervals)";
    return true;
}

bool ScaleManager::saveScalaFile(const string& scaleName, const string& filepath) {
    const Scale* scale = getScale(scaleName);
    if (!scale) {
        ofLogWarning() << "ScaleManager: Scale not found for export: " << scaleName;
        return false;
    }
    
    string content = generateScalaContent(*scale);
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        ofLogWarning() << "ScaleManager: Could not create Scala file: " << filepath;
        return false;
    }
    
    file << content;
    file.close();
    
    ofLogNotice() << "ScaleManager: Exported scale to Scala file: " << filepath;
    return true;
}

vector<string> ScaleManager::getScalaFilesInDirectory(const string& directory) {
    return getFilesWithExtension(directory, ".scl");
}

vector<ScaleManager::ScaleInterval> ScaleManager::parseScalaContent(const string& content) {
    vector<ScaleInterval> intervals;
    stringstream ss(content);
    string line;
    
    bool foundDescription = false;
    bool foundCount = false;
    int expectedIntervals = 0;
    int currentInterval = 0;
    
    while (getline(ss, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '!') {
            continue;
        }
        
        if (!foundDescription) {
            // First non-comment line is description
            foundDescription = true;
            continue;
        }
        
        if (!foundCount) {
            // Second non-comment line is interval count
            try {
                expectedIntervals = stoi(line);
                foundCount = true;
                continue;
            } catch (...) {
                ofLogWarning() << "ScaleManager: Invalid interval count in Scala file: " << line;
                return intervals;
            }
        }
        
        // Parse interval lines
        if (currentInterval < expectedIntervals) {
            ScaleInterval interval;
            interval.description = "Interval " + ofToString(currentInterval + 1);
            
            // Check if it's a ratio (contains '/')
            if (line.find('/') != string::npos) {
                // Parse ratio
                size_t slashPos = line.find('/');
                try {
                    float numerator = stof(line.substr(0, slashPos));
                    float denominator = stof(line.substr(slashPos + 1));
                    interval.ratio = numerator / denominator;
                    interval.cents = ratioToCents(interval.ratio);
                } catch (...) {
                    ofLogWarning() << "ScaleManager: Invalid ratio format: " << line;
                    continue;
                }
            } else {
                // Parse cents value
                try {
                    interval.cents = stof(line);
                    interval.ratio = centsToRatio(interval.cents);
                } catch (...) {
                    ofLogWarning() << "ScaleManager: Invalid cents format: " << line;
                    continue;
                }
            }
            
            intervals.push_back(interval);
            currentInterval++;
        }
    }
    
    if ((int)intervals.size() != expectedIntervals) {
        ofLogWarning() << "ScaleManager: Scala file interval count mismatch. Expected: " 
                      << expectedIntervals << ", Found: " << intervals.size();
    }
    
    return intervals;
}

string ScaleManager::generateScalaContent(const Scale& scale) {
    stringstream ss;
    
    // Header comment
    ss << "! " << scale.name << ".scl" << endl;
    ss << "!" << endl;
    ss << "! " << scale.description << endl;
    ss << "!" << endl;
    
    // Description line
    ss << scale.name << endl;
    
    // Interval count
    ss << scale.intervals.size() << endl;
    
    // Intervals
    for (const ScaleInterval& interval : scale.intervals) {
        if (interval.ratio > 0.0f && interval.ratio != centsToRatio(interval.cents)) {
            // Output as ratio if we have a meaningful ratio
            // Find simple ratio approximation
            float ratio = interval.ratio;
            int denominator = 1;
            for (int d = 1; d <= 32; d++) {
                if (abs(ratio * d - round(ratio * d)) < 0.001f) {
                    denominator = d;
                    break;
                }
            }
            int numerator = round(ratio * denominator);
            ss << numerator << "/" << denominator << endl;
        } else {
            // Output as cents
            ss << std::fixed << std::setprecision(3) << interval.cents << endl;
        }
    }
    
    return ss.str();
}

// =============================================================================
// UTILITY METHODS
// =============================================================================

bool ScaleManager::requiresPitchBend(const string& scaleName) const {
    const Scale* scale = getScale(scaleName);
    return scale ? scale->isMicrotonal : false;
}

int ScaleManager::getScaleSize(const string& scaleName) const {
    const Scale* scale = getScale(scaleName);
    return scale ? (scale->intervals.size() + 1) : 7; // +1 for root note
}

float ScaleManager::getScaleRange(const string& scaleName) const {
    const Scale* scale = getScale(scaleName);
    if (!scale || scale->intervals.empty()) {
        return 1200.0f; // One octave default
    }
    
    float maxCents = 0.0f;
    for (const ScaleInterval& interval : scale->intervals) {
        maxCents = max(maxCents, interval.cents);
    }
    
    return maxCents;
}

bool ScaleManager::isScaleEqual(const string& scaleName) const {
    const Scale* scale = getScale(scaleName);
    if (!scale || scale->intervals.empty()) {
        return true; // Assume equal temperament for empty scales
    }
    
    // Check if intervals are equally spaced
    if (scale->intervals.size() < 2) return true;
    
    float expectedStep = scale->intervals[0].cents;
    for (size_t i = 1; i < scale->intervals.size(); i++) {
        float actualStep = scale->intervals[i].cents - scale->intervals[i-1].cents;
        if (abs(actualStep - expectedStep) > 5.0f) { // 5 cent tolerance
            return false;
        }
    }
    
    return true;
}

bool ScaleManager::createCustomScale(const string& name, const vector<float>& centsIntervals, const string& description) {
    if (name.empty() || centsIntervals.empty()) {
        return false;
    }
    
    Scale scale;
    scale.name = name;
    scale.description = description.empty() ? "Custom scale" : description;
    scale.source = "custom";
    scale.isMicrotonal = false;
    
    // Convert cents to intervals
    for (float cents : centsIntervals) {
        ScaleInterval interval;
        interval.cents = cents;
        interval.ratio = centsToRatio(cents);
        interval.description = ofToString(cents, 1) + " cents";
        
        // Check if microtonal (not a multiple of 100 cents)
        if (fmod(cents, 100.0f) > 5.0f) {
            scale.isMicrotonal = true;
        }
        
        scale.intervals.push_back(interval);
    }
    
    if (!validateScaleIntervals(scale.intervals)) {
        ofLogWarning() << "ScaleManager: Invalid scale intervals for: " << name;
        return false;
    }
    
    scales[name] = scale;
    ofLogNotice() << "ScaleManager: Created custom scale: " << name;
    return true;
}

bool ScaleManager::deleteCustomScale(const string& name) {
    auto it = scales.find(name);
    if (it != scales.end() && (it->second.source == "custom" || it->second.source == "scala")) {
        scales.erase(it);
        ofLogNotice() << "ScaleManager: Deleted custom scale: " << name;
        return true;
    }
    return false;
}

bool ScaleManager::fileExists(const string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

vector<string> ScaleManager::getFilesWithExtension(const string& directory, const string& extension) {
    vector<string> files;
    ofDirectory dir(directory);
    if (dir.exists()) {
        dir.listDir();
        for (int i = 0; i < dir.size(); i++) {
            if (dir.getName(i).find(extension) != string::npos) {
                files.push_back(dir.getName(i));
            }
        }
    }
    return files;
}

// UI support methods implementation

vector<string> ScaleManager::getBuiltinScales() const {
    vector<string> builtinScales;
    
    for (const auto& pair : scales) {
        if (pair.second.source == "builtin") {
            builtinScales.push_back(pair.first);
        }
    }
    
    // Sort alphabetically
    sort(builtinScales.begin(), builtinScales.end());
    return builtinScales;
}

vector<string> ScaleManager::getScalaScales() const {
    vector<string> scalaScales;
    
    for (const auto& pair : scales) {
        if (pair.second.source == "scala") {
            scalaScales.push_back(pair.first);
        }
    }
    
    // Sort alphabetically
    sort(scalaScales.begin(), scalaScales.end());
    return scalaScales;
}

vector<float> ScaleManager::getScaleNotes(const string& scaleName) const {
    auto it = scales.find(scaleName);
    if (it == scales.end()) {
        ofLogError() << "ScaleManager: Scale not found: " << scaleName;
        return vector<float>();
    }
    
    vector<float> notes;
    for (const auto& interval : it->second.intervals) {
        notes.push_back(interval.cents);
    }
    
    return notes;
}

bool ScaleManager::isScaleMicrotonal(const string& scaleName) const {
    auto it = scales.find(scaleName);
    if (it == scales.end()) {
        return false;
    }
    
    return it->second.isMicrotonal;
}

void ScaleManager::refreshScalaFiles() {
    // Remove existing scala scales
    auto it = scales.begin();
    while (it != scales.end()) {
        if (it->second.source == "scala") {
            it = scales.erase(it);
        } else {
            ++it;
        }
    }
    
    // Reload scala files from directory
    string scalaDir = ofToDataPath("scales/");
    ofDirectory dir(scalaDir);
    
    if (dir.exists()) {
        dir.listDir();
        for (int i = 0; i < dir.size(); i++) {
            string filename = dir.getName(i);
            if (filename.find(".scl") != string::npos) {
                string fullPath = dir.getPath(i);
                loadScalaFile(fullPath);
            }
        }
    }
    
    ofLogNotice() << "ScaleManager: Refreshed Scala files from " << scalaDir;
}

bool ScaleManager::exportScalaFile(const string& scaleName, const string& filename) {
    auto it = scales.find(scaleName);
    if (it == scales.end()) {
        ofLogError() << "ScaleManager: Cannot export - scale not found: " << scaleName;
        return false;
    }
    
    const Scale& scale = it->second;
    string fullPath = ofToDataPath("scales/" + filename);
    
    ofFile file(fullPath, ofFile::WriteOnly);
    if (!file.is_open()) {
        ofLogError() << "ScaleManager: Cannot create export file: " << fullPath;
        return false;
    }
    
    // Write Scala format
    file << "! " << filename << "\n";
    file << "!\n";
    file << "! " << scale.description << "\n";
    file << "!\n";
    file << scale.name << "\n";
    file << scale.intervals.size() << "\n";
    file << "!\n";
    
    for (const auto& interval : scale.intervals) {
        if (interval.ratio > 0.0f && interval.ratio != 1.0f) {
            // Use ratio if available and not unity
            file << std::fixed << std::setprecision(6) << interval.ratio << "\n";
        } else {
            // Use cents
            file << std::fixed << std::setprecision(5) << interval.cents << "\n";
        }
    }
    
    file << "!\n";
    file.close();
    
    ofLogNotice() << "ScaleManager: Exported scale '" << scaleName << "' to " << fullPath;
    return true;
}