#!/bin/bash

# SonifyV1 System Status Checker
# Quick status check for running processes

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== SonifyV1 System Status ===${NC}"
echo

# Check MediaPipe pose server
if pgrep -f "mediapipe_pose_server.py" > /dev/null 2>&1; then
    POSE_PID=$(pgrep -f "mediapipe_pose_server.py")
    POSE_CPU=$(ps -p "$POSE_PID" -o %cpu= | tr -d ' ')
    echo -e "${GREEN}✓ MediaPipe Pose Server: RUNNING (PID: $POSE_PID, CPU: ${POSE_CPU}%)${NC}"
else
    echo -e "${RED}✗ MediaPipe Pose Server: NOT RUNNING${NC}"
fi

# Check SonifyV1 application
if pgrep -f "SonifyV1" > /dev/null 2>&1; then
    OF_PID=$(pgrep -f "SonifyV1")
    OF_CPU=$(ps -p "$OF_PID" -o %cpu= | tr -d ' ')
    echo -e "${GREEN}✓ SonifyV1 Application: RUNNING (PID: $OF_PID, CPU: ${OF_CPU}%)${NC}"
else
    echo -e "${RED}✗ SonifyV1 Application: NOT RUNNING${NC}"
fi

# Check UDP port
if lsof -Pi :8080 -t >/dev/null 2>&1; then
    echo -e "${GREEN}✓ UDP Port 8080: ACTIVE${NC}"
else
    echo -e "${RED}✗ UDP Port 8080: NOT ACTIVE${NC}"
fi

# Check recent log activity
echo
echo -e "${BLUE}Recent Activity:${NC}"

if [ -f "bin/pose_server.log" ]; then
    echo -e "${YELLOW}Pose Server (last 3 lines):${NC}"
    tail -3 bin/pose_server.log 2>/dev/null || echo "No recent activity"
fi

if [ -f "bin/of_app.log" ]; then
    echo -e "${YELLOW}OpenFrameworks App (last 3 lines):${NC}"
    tail -3 bin/of_app.log 2>/dev/null || echo "No recent activity"
fi

echo