#pragma once

#include "ofMain.h"
#include "ofxJSON.h"

class ScaleManager {
public:
    // Scale data structures
    struct ScaleInterval {
        float cents;           // Interval in cents (1200 cents = 1 octave)
        float ratio;           // Just intonation ratio (optional)
        string description;    // Interval description (e.g. "major third")
    };
    
    struct Scale {
        string name;                        // Scale name (e.g. "Major", "Bohlen-Pierce")
        string filename;                    // Source filename for Scala scales
        string description;                 // Scale description
        vector<ScaleInterval> intervals;    // Interval definitions
        bool isMicrotonal;                 // True if requires pitch bend
        int baseNoteMidi;                  // Base MIDI note (usually 60 = middle C)
        string source;                     // "builtin", "scala", "custom"
        
        // Constructor
        Scale() : name(""), filename(""), description(""), isMicrotonal(false), 
                 baseNoteMidi(60), source("builtin") {}
    };
    
    // MIDI pitch bend data for microtonal support
    struct MicrotonalNote {
        int midiNote;          // Base MIDI note
        int pitchBend;         // Pitch bend value (-8192 to +8191)
        float centsOffset;     // Exact cents offset from base note
    };

public:
    ScaleManager();
    ~ScaleManager();
    
    void setup();
    void update();
    
    // Scale management
    vector<string> getAvailableScaleNames() const;
    const Scale* getScale(const string& scaleName) const;
    bool setCurrentScale(const string& scaleName);
    string getCurrentScaleName() const { return currentScaleName; }
    
    // Note calculation methods
    vector<string> getScaleNoteNames(const string& scaleName, int rootNote = 0) const;
    MicrotonalNote getMicrotonalNote(const string& scaleName, int scaleIndex, int rootNote, int octave) const;
    vector<int> getScaleIntervals(const string& scaleName) const; // For backward compatibility
    
    // Scala file support (.scl format)
    bool loadScalaFile(const string& filepath);
    bool saveScalaFile(const string& scaleName, const string& filepath);
    vector<string> getScalaFilesInDirectory(const string& directory);
    
    // Built-in scale initialization
    void initializeBuiltinScales();
    
    // Custom scale creation
    bool createCustomScale(const string& name, const vector<float>& centsIntervals, const string& description = "");
    bool deleteCustomScale(const string& name);
    
    // Configuration methods
    void saveToJSON(ofxJSONElement& json);
    void loadFromJSON(const ofxJSONElement& json);
    void setDefaults();
    
    // MIDI pitch bend support
    bool requiresPitchBend(const string& scaleName) const;
    void enableMicrotonality(bool enable) { microtonalityEnabled = enable; }
    bool isMicrotonalityEnabled() const { return microtonalityEnabled; }
    void setMicrotonalityEnabled(bool enable) { microtonalityEnabled = enable; }
    
    // UI support methods
    vector<string> getBuiltinScales() const;
    vector<string> getScalaScales() const;
    vector<float> getScaleNotes(const string& scaleName) const;
    bool isScaleMicrotonal(const string& scaleName) const;
    void refreshScalaFiles();
    bool exportScalaFile(const string& scaleName, const string& filename);
    
    // Scale analysis
    int getScaleSize(const string& scaleName) const;
    float getScaleRange(const string& scaleName) const; // Range in cents
    bool isScaleEqual(const string& scaleName) const;   // Equal temperament check
    
private:
    // Internal data
    map<string, Scale> scales;              // All available scales
    string currentScaleName;                // Currently selected scale
    bool microtonalityEnabled;              // Global microtonal support flag
    string scalaDirectory;                  // Directory for Scala files
    
    // Built-in scale definitions (12-tone equal temperament)
    void createMajorScale();
    void createMinorScale();
    void createPentatonicScale();
    void createBluesScale();
    void createChromaticScale();
    void createDorian();
    void createPhrygian();
    void createLydian();
    void createMixolydian();
    void createAeolian();
    void createLocrian();
    
    // Microtonal scale examples
    void createJustIntonationScale();
    void createBohlenPierceScale();
    void create19ToneEqualScale();
    void create31ToneEqualScale();
    
    // Helper methods
    vector<ScaleInterval> parseScalaContent(const string& content);
    string generateScalaContent(const Scale& scale);
    float centsToRatio(float cents);
    float ratioToCents(float ratio);
    int centsToPitchBend(float cents) const;
    string formatNoteName(int midiNote);
    bool validateScaleIntervals(const vector<ScaleInterval>& intervals);
    
    // File I/O helpers
    bool fileExists(const string& path);
    vector<string> getFilesWithExtension(const string& directory, const string& extension);
};