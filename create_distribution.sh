#!/bin/bash
# SonifyV1 Distribution Builder
# Creates a standalone, distributable macOS application

set -e  # Exit on error

echo "🎵 SonifyV1 Distribution Builder"
echo "================================"

# Configuration
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIST_DIR="${PROJECT_ROOT}/distribution"
APP_NAME="SonifyV1"
APP_BUNDLE="${PROJECT_ROOT}/bin/${APP_NAME}.app"
VERSION="2.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if app exists
if [ ! -d "${APP_BUNDLE}" ]; then
    echo -e "${RED}Error: ${APP_NAME}.app not found in bin/${NC}"
    echo "Please build the project first with: make"
    exit 1
fi

# Clean and create distribution directory
echo -e "${YELLOW}📁 Creating distribution directory...${NC}"
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}/${APP_NAME}"

# Copy app bundle
echo -e "${YELLOW}📦 Copying application bundle...${NC}"
cp -R "${APP_BUNDLE}" "${DIST_DIR}/${APP_NAME}/"

# Copy data directory into app bundle Resources
echo -e "${YELLOW}📂 Bundling data files...${NC}"
DATA_SRC="${PROJECT_ROOT}/bin/data"
RESOURCES_DIR="${DIST_DIR}/${APP_NAME}/${APP_NAME}.app/Contents/Resources"

# Create Resources directory if it doesn't exist
mkdir -p "${RESOURCES_DIR}"

# Copy entire data directory to Resources
cp -R "${DATA_SRC}" "${RESOURCES_DIR}/"

# IMPORTANT: Also create symlink for relative path resolution
# openFrameworks expects data at MacOS/../../../data when not using absolute paths
# Create the directory structure for relative path access
MACOS_DIR="${DIST_DIR}/${APP_NAME}/${APP_NAME}.app/Contents/MacOS"
RELATIVE_DATA_DIR="${DIST_DIR}/${APP_NAME}/data"

# Create symlink from expected relative location to Resources/data
ln -sf "${APP_NAME}.app/Contents/Resources/data" "${RELATIVE_DATA_DIR}"

# Verify critical files
echo -e "${YELLOW}✓ Verifying bundled files...${NC}"
VERIFY_PATHS=(
    "data/models/yolov8m.mlpackage"
    "data/scales"
    "data/config.json"
)

for path in "${VERIFY_PATHS[@]}"; do
    if [ -e "${RESOURCES_DIR}/${path}" ]; then
        echo -e "${GREEN}  ✓ ${path}${NC}"
    else
        echo -e "${RED}  ✗ ${path} - NOT FOUND${NC}"
    fi
done

# Create README for distribution
echo -e "${YELLOW}📝 Creating distribution README...${NC}"
cat > "${DIST_DIR}/${APP_NAME}/README.txt" << 'EOF'
╔══════════════════════════════════════════════════════════════════╗
║                      SonifyV1 v2.0                               ║
║   Advanced Tempo-Synchronized Microtonal Highway Sonification    ║
╚══════════════════════════════════════════════════════════════════╝

INSTALLATION
============
1. Copy SonifyV1.app to your Applications folder (optional)
2. Double-click SonifyV1.app to launch
3. Grant camera permissions when prompted (required for camera input)

FIRST LAUNCH
============
⚠️  macOS Security Notice:
If you see "SonifyV1 cannot be opened because it is from an
unidentified developer":

1. Right-click (or Control-click) SonifyV1.app
2. Select "Open" from the menu
3. Click "Open" in the security dialog
4. This only needs to be done once

QUICK START
===========
1. Launch the application
2. Press 'g' to show/hide GUI
3. Press 'd' to enable object detection
4. Draw lines with LEFT CLICK (start) and RIGHT CLICK (finish)
5. Vehicles crossing lines trigger musical events!

ESSENTIAL CONTROLS
==================
'g'         - Toggle GUI interface
'd'         - Toggle object detection
'o'         - Open video file
'v'         - Switch camera/video source
'r'         - Restart camera
'c'         - Clear all lines
SPACE       - Play/pause video
LEFT/RIGHT  - Seek video
'l'         - Toggle video loop

MIDI SETUP
==========
1. Open MIDI tab in GUI
2. Select your MIDI output port
3. Configure tempo, scale, and per-line settings
4. Draw lines to create musical instruments

SYSTEM REQUIREMENTS
===================
• macOS 11.0 or later
• Apple Silicon (M1/M2) or Intel processor
• 8GB RAM recommended
• USB webcam or built-in camera
• MIDI-compatible audio software (optional)

FEATURES
========
✓ Real-time object detection (80 classes)
✓ Interactive line drawing system
✓ Tempo-synchronized musical output (40-200 BPM)
✓ Microtonal scale support (15+ scales + Scala format)
✓ Multi-port MIDI output with pitch bend
✓ Video file playback and analysis
✓ Professional 4-tab interface

TROUBLESHOOTING
===============
• Camera not working? Press 'r' to restart camera connection
• No MIDI output? Check MIDI tab for port selection
• Performance issues? Lower detection frame rate in Main tab
• App won't open? See "First Launch" security notice above

SUPPORT & DOCUMENTATION
========================
For detailed documentation, see the included CLAUDE.md file
or visit the project repository.

Version: 2.0
Status: Production Ready
Last Updated: 2025-10-15

═══════════════════════════════════════════════════════════════════
EOF

# Create a quick start guide
echo -e "${YELLOW}📖 Creating Quick Start Guide...${NC}"
cat > "${DIST_DIR}/${APP_NAME}/QUICK_START.txt" << 'EOF'
╔══════════════════════════════════════════════════════════════════╗
║              SonifyV1 - 60 Second Quick Start                    ║
╚══════════════════════════════════════════════════════════════════╝

1. OPEN THE APP
   → Double-click SonifyV1.app
   → Grant camera permissions when asked

2. ENABLE DETECTION
   → Press 'g' to show GUI
   → Press 'd' to start detecting objects
   → You should see bounding boxes on vehicles

3. DRAW YOUR FIRST LINE
   → LEFT CLICK to start drawing
   → Move mouse to position the line
   → RIGHT CLICK to finish
   → The line is now a musical instrument!

4. SET UP MIDI (Optional)
   → Click "MIDI Settings" tab
   → Choose your MIDI output port
   → Set tempo (try 120 BPM)
   → Pick a scale (try "Major")

5. MAKE MUSIC
   → Point camera at traffic (or open a highway video with 'o')
   → Vehicles crossing your lines trigger notes!
   → Draw more lines for more instruments
   → Each line can have different settings

TIPS
====
• Press 'c' to clear all lines and start over
• Use different colors for different instruments
• Adjust velocity and duration in MIDI tab
• Try microtonal scales for unique sounds!

HAVE FUN! 🎵
EOF

# Copy additional documentation
if [ -f "${PROJECT_ROOT}/CLAUDE.md" ]; then
    echo -e "${YELLOW}📄 Copying full documentation...${NC}"
    cp "${PROJECT_ROOT}/CLAUDE.md" "${DIST_DIR}/${APP_NAME}/"
fi

if [ -f "${PROJECT_ROOT}/HOW_TO_USE.md" ]; then
    echo -e "${YELLOW}📖 Copying How To Use guide...${NC}"
    cp "${PROJECT_ROOT}/HOW_TO_USE.md" "${DIST_DIR}/${APP_NAME}/"
fi

# Create a simple launcher script (optional - for advanced users)
cat > "${DIST_DIR}/${APP_NAME}/Launch_SonifyV1.command" << 'EOF'
#!/bin/bash
cd "$(dirname "$0")"
open SonifyV1.app
EOF
chmod +x "${DIST_DIR}/${APP_NAME}/Launch_SonifyV1.command"

# Display distribution size
echo ""
echo -e "${GREEN}✓ Distribution package created successfully!${NC}"
echo ""
APP_SIZE=$(du -sh "${DIST_DIR}/${APP_NAME}/${APP_NAME}.app" | cut -f1)
TOTAL_SIZE=$(du -sh "${DIST_DIR}/${APP_NAME}" | cut -f1)
echo "  App Size:       ${APP_SIZE}"
echo "  Total Package:  ${TOTAL_SIZE}"
echo "  Location:       ${DIST_DIR}/${APP_NAME}/"
echo ""

# Offer to create DMG
echo -e "${YELLOW}Would you like to create a DMG installer? (y/n)${NC}"
read -p "Create DMG? " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    DMG_NAME="${APP_NAME}-v${VERSION}.dmg"
    DMG_PATH="${DIST_DIR}/${DMG_NAME}"

    echo -e "${YELLOW}💿 Creating DMG installer...${NC}"

    # Remove old DMG if exists
    rm -f "${DMG_PATH}"

    # Create DMG
    hdiutil create -volname "${APP_NAME}" \
                   -srcfolder "${DIST_DIR}/${APP_NAME}" \
                   -ov -format UDZO \
                   "${DMG_PATH}"

    if [ $? -eq 0 ]; then
        DMG_SIZE=$(du -sh "${DMG_PATH}" | cut -f1)
        echo ""
        echo -e "${GREEN}✓ DMG created successfully!${NC}"
        echo "  DMG Size:   ${DMG_SIZE}"
        echo "  Location:   ${DMG_PATH}"
    else
        echo -e "${RED}✗ DMG creation failed${NC}"
    fi
fi

echo ""
echo -e "${GREEN}════════════════════════════════════════${NC}"
echo -e "${GREEN}  Distribution build complete! 🎉${NC}"
echo -e "${GREEN}════════════════════════════════════════${NC}"
echo ""
echo "Next steps:"
echo "1. Test the app: cd ${DIST_DIR}/${APP_NAME} && open ${APP_NAME}.app"
echo "2. Distribute the folder or DMG file"
echo "3. Recipients can drag to Applications folder"
echo ""
