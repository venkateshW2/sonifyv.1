# SonifyV1 🎵🚗📹

**Advanced Tempo-Synchronized Microtonal Highway Sonification System**

[![Version](https://img.shields.io/badge/version-2.0-blue.svg)](https://github.com/venkateshW2/sonifyv.1/releases)
[![Platform](https://img.shields.io/badge/platform-macOS%2011.0%2B-lightgrey.svg)](https://www.apple.com/macos/)
[![Status](https://img.shields.io/badge/status-production%20ready-brightgreen.svg)](https://github.com/venkateshW2/sonifyv.1)

Convert real-world visual scenes into sophisticated musical output using computer vision, tempo-synchronized randomization, and microtonal scales.

---

## 🎯 What is SonifyV1?

SonifyV1 transforms **visual movement into music** using:
- **CoreML YOLOv8** object detection (80 classes, 60fps)
- **Interactive line drawing** - each line is a musical instrument
- **Tempo-synchronized** note generation (40-200 BPM with swing)
- **Microtonal scales** - 15+ built-in + Scala format support
- **Multi-DAW MIDI** output with pitch bend microtonality
- **Real-time video** analysis and camera input

### Use Cases
- 🎼 **Algorithmic Composition** - Highway traffic generates musical patterns
- 🎭 **Live Performance** - Interactive sound installation
- 🎓 **Education** - Teach microtonal music theory
- 🔬 **Research** - Explore generative music systems

---

## ✨ Key Features

### Real-Time Object Detection
- **CoreML YOLOv8** models (nano, medium, large)
- **80 COCO classes**: Vehicles, people, animals, objects
- **Apple Neural Engine** optimized for M1/M2
- **60fps** performance with configurable detection

### Interactive Line System
- **Mouse-based drawing** (LEFT click start, RIGHT click finish)
- **Multi-color management** with automatic line numbering
- **Line crossing detection** - precise intersection algorithms
- **Per-line musical properties** - channel, octave, velocity, duration

### Advanced Musical Features
- ✅ **Tempo Synchronization** (40-200 BPM)
- ✅ **Note Subdivisions** (whole, half, quarter, eighth, sixteenth)
- ✅ **Swing Timing** (0-100% adjustable groove)
- ✅ **Scale Degree Weighting** (musical intelligence)
- ✅ **Microtonal Scales** (Just Intonation, Pythagorean, etc.)
- ✅ **MIDI Pitch Bend** (14-bit precision, ±200 cents)
- ✅ **Multi-Port MIDI** output

---

## 🚀 Quick Start

### Download Pre-Built App (Recommended)

**Latest Release**: [SonifyV1-v2.0.dmg](https://github.com/venkateshW2/sonifyv.1/releases/latest)

1. Download DMG from [Releases](https://github.com/venkateshW2/sonifyv.1/releases)
2. Open DMG and drag **SonifyV1.app** to Applications
3. Right-click → Open (first time only)
4. Grant camera permissions

### Build from Source

```bash
# Clone repository
git clone https://github.com/venkateshW2/sonifyv.1.git
cd sonifyv.1

# Build
make clean && make

# Run
./bin/SonifyV1.app/Contents/MacOS/SonifyV1

# Create distribution
./create_distribution.sh
```

---

## 📖 Documentation

| Document | Description | Size |
|----------|-------------|------|
| [**HOW_TO_USE.md**](HOW_TO_USE.md) | Complete user guide | 44 KB |
| [**CLAUDE.md**](CLAUDE.md) | Technical overview | 10 KB |
| [**DISTRIBUTION.md**](DISTRIBUTION.md) | Distribution guide | 15 KB |

### Quick Links
- 🎯 [5-Minute Quick Start](HOW_TO_USE.md#quick-start-5-minutes)
- 🎵 [4 Workflow Examples](HOW_TO_USE.md#workflow-examples)
- 🎹 [Musical Configuration](HOW_TO_USE.md#musical-configuration)
- 🐛 [Troubleshooting](HOW_TO_USE.md#troubleshooting)

---

## 🎮 Basic Usage

```
1. Press 'g' → Show GUI
2. Press 'd' → Enable detection  
3. LEFT CLICK → Start drawing line
4. RIGHT CLICK → Finish line
5. Configure MIDI (MIDI Settings tab)
6. Watch vehicles cross lines = music! 🎵
```

### Keyboard Shortcuts
```
'g'    Show/hide GUI      'd'    Toggle detection
'o'    Open video         'v'    Switch camera/video
'c'    Clear lines        'r'    Restart camera
SPACE  Play/pause         ←→     Seek video
```

---

## 🎵 Musical Features

**Scales**: Major, Minor, Dorian, Phrygian, Lydian, Mixolydian, Locrian, Pentatonic, Blues, Chromatic, Just Intonation, Pythagorean, Quarter-Comma Meantone, + Custom Scala files

**Tempo**: 40-200 BPM with swing, subdivisions, and musical intelligence

**MIDI**: Multi-port output, 16 channels, pitch bend microtonality, per-line properties

---

## 📊 System Requirements

- **OS**: macOS 11.0+ (Big Sur or later)
- **Architecture**: Universal (Apple Silicon M1/M2 or Intel)
- **Memory**: 8GB RAM recommended
- **Storage**: 100 MB free space
- **Camera**: USB webcam or built-in (optional)

---

## 🏗️ Architecture

```
ofApp (main)
├── VideoManager         # Camera/video input
├── LineManager          # Line drawing
├── DetectionManager     # CoreML YOLOv8
├── TempoManager         # BPM sync
├── ScaleManager         # Microtonal scales
├── CommunicationManager # MIDI/OSC output
├── UIManager            # 4-tab GUI
└── ConfigManager        # JSON persistence
```

---

## 🎓 Workflow Examples

### 1. Highway Time-Lapse Ambient
Slow BPM, Lydian scale, 4 lanes → Atmospheric soundscape

### 2. Pedestrian Percussion
Fast BPM, Chromatic, crossing patterns → Polyrhythmic drums

### 3. Interactive Installation
Medium BPM, Pentatonic, 3x3 grid → Visitor-created music

### 4. Live Coding Integration
Sync with TidalCycles/Sonic Pi → Hybrid performance

**See [HOW_TO_USE.md](HOW_TO_USE.md#workflow-examples) for details.**

---

## 🐛 Troubleshooting

**Camera not working**: System Preferences → Camera → Enable SonifyV1

**No MIDI output**: Select MIDI port in MIDI Settings tab

**Slow performance**: Increase frame skip, disable unused classes

**See [HOW_TO_USE.md#troubleshooting](HOW_TO_USE.md#troubleshooting) for 20+ solutions.**

---

## 🤝 Contributing

1. Fork repository
2. Create feature branch
3. Commit changes
4. Push and open Pull Request

---

## 📜 License

MIT License - see [LICENSE](LICENSE)

**Dependencies**: openFrameworks (MIT), YOLOv8 (GPL-3.0), ofxImGui/Midi/Osc/JSON (MIT)

---

## 🎯 Status

**Version 2.0** - Production Ready ✅

- ✅ Complete highway sonification
- ✅ Tempo sync with musical intelligence
- ✅ Microtonal scale system
- ✅ Configuration persistence
- ✅ Professional documentation

---

## 📬 Links

- [Latest Release](https://github.com/venkateshW2/sonifyv.1/releases/latest)
- [Documentation](HOW_TO_USE.md)
- [Issues](https://github.com/venkateshW2/sonifyv.1/issues)
- [openFrameworks](https://openframeworks.cc)

---

**🎵 Start creating music with computer vision today!**

*Version 2.0 | 2025-10-15*
