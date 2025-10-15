# SonifyV1 Distribution Guide

## Overview
This guide explains how to create and distribute standalone executable versions of SonifyV1 for macOS.

---

## Quick Distribution

### Automated Build Script
The project includes an automated distribution builder script that handles all aspects of creating a standalone application:

```bash
cd /Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1
./create_distribution.sh
```

This script will:
1. ✅ Copy the application bundle
2. ✅ Bundle all data files (models, scales, config) into the app's Resources
3. ✅ Verify critical files are present
4. ✅ Create comprehensive user documentation (README, Quick Start Guide)
5. ✅ Optionally create a DMG installer for easy distribution

---

## Distribution Outputs

### 1. Standalone App Folder
**Location:** `distribution/SonifyV1/`

Contains:
- **SonifyV1.app** - The standalone application (77 MB)
- **README.txt** - Full installation and usage instructions
- **QUICK_START.txt** - 60-second quick start guide
- **CLAUDE.md** - Complete technical documentation
- **Launch_SonifyV1.command** - Optional launcher script

**Distribution Method:**
- Compress the entire `SonifyV1` folder as a ZIP file
- Recipients extract and run `SonifyV1.app`
- Can be moved to Applications folder

### 2. DMG Installer (Recommended)
**Location:** `distribution/SonifyV1-v2.0.dmg` (64 MB compressed)

**Advantages:**
- ✅ Professional distribution format
- ✅ 17% smaller than folder (64 MB vs 77 MB)
- ✅ Single file to distribute
- ✅ Native macOS installer experience
- ✅ Prevents file corruption during transfer

**Distribution Method:**
- Share the DMG file directly
- Recipients double-click to mount
- Drag SonifyV1.app to Applications
- Eject the DMG

---

## What's Bundled in the App

### Embedded Resources (in SonifyV1.app/Contents/Resources/)
```
data/
├── models/
│   ├── yolov8n.mlpackage  (22 MB - nano model)
│   ├── yolov8m.mlpackage  (48 MB - medium model, default)
│   ├── yolov8l.mlpackage  (82 MB - large model)
│   └── coco.names         (class labels)
├── scales/
│   ├── just_major.scl
│   ├── pythagorean.scl
│   └── quarter_comma_meantone.scl
└── config.json            (default configuration)
```

### System Dependencies (Already Included in macOS)
All system frameworks are dynamically linked and available on all macOS 11.0+ systems:
- CoreML (object detection)
- CoreVideo (video processing)
- AVFoundation (camera/video input)
- Vision (ML integration)
- Metal (GPU acceleration)
- OpenGL (graphics)
- CoreAudio/AudioToolbox (MIDI)

**No external dependencies need to be installed by users!**

---

## macOS Security Considerations

### Gatekeeper Warning
Since the app is not code-signed with an Apple Developer certificate, users will see:
> "SonifyV1 cannot be opened because it is from an unidentified developer"

**User Instructions (included in README.txt):**
1. Right-click (or Control-click) SonifyV1.app
2. Select "Open" from the menu
3. Click "Open" in the security dialog
4. This only needs to be done once

### Camera Permissions
The app requires camera access. The Info.plist includes:
```xml
<key>NSCameraUsageDescription</key>
<string>This app needs to access the camera</string>
```

Users will be prompted on first launch.

---

## For Code-Signed Distribution (Optional)

If you have an Apple Developer account ($99/year), you can code-sign and notarize the app to avoid Gatekeeper warnings:

### 1. Code Signing
```bash
codesign --deep --force --verify --verbose --sign "Developer ID Application: Your Name" \
    distribution/SonifyV1/SonifyV1.app
```

### 2. Notarization
```bash
# Create a DMG
hdiutil create -volname "SonifyV1" -srcfolder distribution/SonifyV1 -ov -format UDZO \
    SonifyV1-v2.0.dmg

# Submit for notarization
xcrun notarytool submit SonifyV1-v2.0.dmg \
    --apple-id your@email.com \
    --team-id YOUR_TEAM_ID \
    --password your-app-specific-password \
    --wait

# Staple the notarization ticket
xcrun stapler staple SonifyV1-v2.0.dmg
```

**Benefits:**
- No Gatekeeper warnings
- Automatic approval by macOS
- Professional distribution
- Required for Mac App Store

---

## System Requirements for Users

### Minimum Requirements
- **OS:** macOS 11.0 (Big Sur) or later
- **Architecture:** Universal (Apple Silicon M1/M2 or Intel x86_64)
- **Memory:** 4GB RAM minimum, 8GB recommended
- **Storage:** 100 MB free space
- **Camera:** USB webcam or built-in camera (optional)

### Performance Notes
- **Apple Silicon (M1/M2):** Optimized performance with Neural Engine acceleration
- **Intel Macs:** Full compatibility, slightly slower ML inference
- **Detection Speed:** 30-60 fps depending on model size and hardware

---

## Rebuilding from Source

If you need to rebuild the distribution:

### 1. Clean Build
```bash
cd /Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1
make clean
make
```

### 2. Run Distribution Script
```bash
./create_distribution.sh
```

### 3. Test Before Distributing
```bash
cd distribution/SonifyV1
open SonifyV1.app
```

Verify:
- ✅ App launches without errors
- ✅ Camera detection works
- ✅ Object detection loads CoreML models
- ✅ Line drawing and crossing detection functions
- ✅ MIDI output available
- ✅ Scale files load correctly
- ✅ Config saves/loads properly

---

## Distribution Checklist

Before sharing with users:

- [ ] Run `create_distribution.sh` to build fresh package
- [ ] Test the standalone app (not from dev environment)
- [ ] Verify camera permissions prompt works
- [ ] Test on a different Mac if possible
- [ ] Confirm DMG mounts and installs correctly
- [ ] Include README.txt and QUICK_START.txt
- [ ] Provide support contact information
- [ ] Document known issues or limitations

---

## File Sizes Reference

| Item | Size | Description |
|------|------|-------------|
| SonifyV1.app | 77 MB | Standalone application |
| Distribution Folder | 77 MB | App + documentation |
| DMG Installer | 64 MB | Compressed installer (17% smaller) |
| YOLOv8n model | 22 MB | Fastest, less accurate |
| YOLOv8m model | 48 MB | Balanced (default) |
| YOLOv8l model | 82 MB | Slowest, most accurate |

---

## Troubleshooting Distribution Issues

### App Won't Launch
- Verify macOS 11.0+ (check: System Preferences → About This Mac)
- Try the Right-click → Open method for Gatekeeper
- Check Console.app for crash logs

### "Data Files Not Found" Errors
- Verify Resources folder contains `data/` directory:
  ```bash
  ls -la SonifyV1.app/Contents/Resources/data/
  ```
- Rebuild with `create_distribution.sh` if missing

### MIDI Not Working
- User needs to select MIDI port in MIDI Settings tab
- Requires MIDI-capable software running (DAW, synth, etc.)
- Test with built-in macOS Audio MIDI Setup

### Performance Issues
- Switch to smaller YOLOv8n model in Detection tab
- Lower detection frame rate
- Close other resource-intensive applications
- Verify camera resolution (default: 1280x720)

---

## Support & Updates

### Version Tracking
Current version: **2.0** (defined in `create_distribution.sh`)

Update version number before each release:
```bash
# Edit create_distribution.sh
VERSION="2.0"  # Change to new version
```

### Distribution Changelog
Maintain a changelog for users:
- List new features
- Document bug fixes
- Note breaking changes
- Update system requirements if changed

---

## Advanced: Custom Build Configurations

### Reducing App Size
To create a smaller distribution, remove unused models:

```bash
# Edit create_distribution.sh to skip large models
# Comment out in VERIFY_PATHS section
```

Or manually after building:
```bash
rm -rf distribution/SonifyV1/SonifyV1.app/Contents/Resources/data/models/yolov8l.mlpackage
```

### Custom Branding
1. Replace app icon: `bin/data/of.icns`
2. Update Info.plist bundle name
3. Customize README.txt welcome message
4. Rebuild with distribution script

---

## Legal & Licensing Notes

### Dependencies & Licenses
- **openFrameworks:** MIT License
- **YOLOv8:** GPL-3.0 (Ultralytics)
- **CoreML Models:** Subject to Apple's ML terms
- **ofxImGui:** MIT License
- **ofxMidi, ofxOsc:** MIT License

### Distribution Rights
Ensure you have rights to distribute:
- ✅ OpenFrameworks code (MIT - permissive)
- ✅ Your own code (your choice)
- ⚠️  YOLOv8 models (GPL-3.0 - requires attribution)
- ⚠️  Scala scale files (verify individual licenses)

**Recommended:** Include a LICENSE.txt file in distribution with all attributions.

---

## Questions & Support

For technical questions or distribution issues:
1. Check [CLAUDE.md](CLAUDE.md) for full documentation
2. Review this distribution guide
3. Test in a clean environment (fresh Mac, new user account)
4. Check openFrameworks forums for platform-specific issues

---

*Last Updated: 2025-10-15*
*Version: 2.0*
*Distribution Package: Tested and Verified ✅*
