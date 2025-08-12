#!/bin/bash

# SonifyV1 Complete System Launcher
# Starts both MediaPipe pose server and openFrameworks application

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PYTHON_PATH="$HOME/miniconda3/bin/python"
POSE_SERVER_SCRIPT="bin/mediapipe_pose_server.py"
OF_APP_PATH="bin/SonifyV1.app/Contents/MacOS/SonifyV1"
UDP_PORT=8080
POSE_CONFIDENCE=0.3

# Log files
POSE_LOG="bin/pose_server.log"
OF_LOG="bin/of_app.log"

echo -e "${BLUE}=== SonifyV1 Complete System Launcher ===${NC}"
echo -e "${BLUE}Dual-Detection Creative Sonification System${NC}"
echo

# Function to check if a process is running
check_process() {
    local process_name="$1"
    pgrep -f "$process_name" > /dev/null 2>&1
}

# Function to cleanup processes on exit
cleanup() {
    echo -e "\n${YELLOW}Shutting down SonifyV1 system...${NC}"
    
    # Kill MediaPipe pose server
    if check_process "mediapipe_pose_server.py"; then
        echo -e "${YELLOW}Stopping MediaPipe pose server...${NC}"
        pkill -f "mediapipe_pose_server.py" 2>/dev/null || true
    fi
    
    # Kill openFrameworks app
    if check_process "SonifyV1"; then
        echo -e "${YELLOW}Stopping SonifyV1 application...${NC}"
        pkill -f "SonifyV1" 2>/dev/null || true
    fi
    
    echo -e "${GREEN}System shutdown complete${NC}"
    exit 0
}

# Set up signal handlers for clean shutdown
trap cleanup SIGINT SIGTERM

# Check prerequisites
echo -e "${BLUE}1. Checking system prerequisites...${NC}"

# Check Python with MediaPipe
if [ ! -f "$PYTHON_PATH" ]; then
    echo -e "${RED}Error: Python not found at $PYTHON_PATH${NC}"
    echo -e "${RED}Please install miniconda3 or update PYTHON_PATH${NC}"
    exit 1
fi

# Test MediaPipe import
if ! "$PYTHON_PATH" -c "import mediapipe; import cv2" 2>/dev/null; then
    echo -e "${RED}Error: MediaPipe or OpenCV not installed${NC}"
    echo -e "${RED}Install with: $PYTHON_PATH -m pip install mediapipe opencv-python${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Python 3.10 with MediaPipe found${NC}"

# Check openFrameworks app
if [ ! -f "$OF_APP_PATH" ]; then
    echo -e "${RED}Error: SonifyV1 app not found at $OF_APP_PATH${NC}"
    echo -e "${RED}Please compile the project first: make${NC}"
    exit 1
fi

echo -e "${GREEN}✓ SonifyV1 openFrameworks app found${NC}"

# Check UDP port availability
if lsof -Pi :$UDP_PORT -sTCP:LISTEN -t >/dev/null 2>&1; then
    echo -e "${RED}Error: Port $UDP_PORT is already in use${NC}"
    echo -e "${RED}Please close other applications using this port${NC}"
    exit 1
fi

echo -e "${GREEN}✓ UDP port $UDP_PORT available${NC}"
echo

# Start MediaPipe pose server
echo -e "${BLUE}2. Starting MediaPipe pose server...${NC}"
echo -e "${YELLOW}   Camera will be accessed for pose detection${NC}"
echo -e "${YELLOW}   Server logs: $POSE_LOG${NC}"

"$PYTHON_PATH" "$POSE_SERVER_SCRIPT" \
    --confidence "$POSE_CONFIDENCE" \
    --host localhost \
    --port "$UDP_PORT" \
    > "$POSE_LOG" 2>&1 &

POSE_PID=$!

# Wait for pose server to initialize
sleep 3

# Check if pose server started successfully
if ! check_process "mediapipe_pose_server.py"; then
    echo -e "${RED}Error: MediaPipe pose server failed to start${NC}"
    echo -e "${RED}Check log: $POSE_LOG${NC}"
    tail -10 "$POSE_LOG"
    exit 1
fi

echo -e "${GREEN}✓ MediaPipe pose server running (PID: $POSE_PID)${NC}"

# Start openFrameworks application
echo -e "${BLUE}3. Starting SonifyV1 openFrameworks application...${NC}"
echo -e "${YELLOW}   Application logs: $OF_LOG${NC}"

"$OF_APP_PATH" > "$OF_LOG" 2>&1 &
OF_PID=$!

# Wait for openFrameworks app to initialize
sleep 5

# Check if openFrameworks app started successfully
if ! check_process "SonifyV1"; then
    echo -e "${RED}Error: SonifyV1 application failed to start${NC}"
    echo -e "${RED}Check log: $OF_LOG${NC}"
    tail -10 "$OF_LOG"
    cleanup
    exit 1
fi

echo -e "${GREEN}✓ SonifyV1 application running (PID: $OF_PID)${NC}"
echo

# System status monitoring
echo -e "${BLUE}4. System Status:${NC}"
echo -e "${GREEN}✓ MediaPipe Pose Server: RUNNING${NC}"
echo -e "${GREEN}✓ SonifyV1 Application: RUNNING${NC}"
echo -e "${GREEN}✓ UDP Communication: Port $UDP_PORT${NC}"
echo

echo -e "${BLUE}=== SYSTEM READY ===${NC}"
echo -e "${GREEN}SonifyV1 Dual-Detection System is now operational!${NC}"
echo
echo -e "${YELLOW}Instructions:${NC}"
echo -e "1. The SonifyV1 window should now be open"
echo -e "2. Click on the 'Pose Detection' tab"
echo -e "3. Enable 'Pose Detection' checkbox"
echo -e "4. Status should show 'Connected' with green indicator"
echo -e "5. Draw lines in the video area for musical interaction"
echo
echo -e "${YELLOW}Features Available:${NC}"
echo -e "• CoreML YOLOv8: Vehicle/object detection (80 COCO classes)"
echo -e "• MediaPipe Pose: Human pose landmarks (33 joints)"
echo -e "• Line Crossing: Musical triggers for both detection systems"
echo -e "• MIDI + OSC Output: Real-time musical events"
echo
echo -e "${YELLOW}Press Ctrl+C to shutdown the complete system${NC}"
echo

# Monitor system health
while true; do
    sleep 10
    
    # Check if both processes are still running
    if ! check_process "mediapipe_pose_server.py"; then
        echo -e "\n${RED}Warning: MediaPipe pose server stopped unexpectedly${NC}"
        echo -e "${RED}Check log: $POSE_LOG${NC}"
        break
    fi
    
    if ! check_process "SonifyV1"; then
        echo -e "\n${RED}Warning: SonifyV1 application stopped unexpectedly${NC}"
        echo -e "${RED}Check log: $OF_LOG${NC}"
        break
    fi
    
    # Optional: Show brief status every minute
    # echo -e "${GREEN}System running... ($(date))${NC}"
done

cleanup