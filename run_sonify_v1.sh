#!/bin/bash

# SonifyV1 Control Script
# Manages SonifyV1 object detection application

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="SonifyV1"
BUILD_TYPE="Debug"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Clean build function
clean_build() {
    log_info "Cleaning previous build..."
    cd "$SCRIPT_DIR"
    
    if make clean; then
        log_success "Clean completed"
    else
        log_error "Clean failed"
        return 1
    fi
    
    log_info "Building $APP_NAME ($BUILD_TYPE)..."
    if make $BUILD_TYPE; then
        log_success "Build completed successfully"
        return 0
    else
        log_error "Build failed"
        return 1
    fi
}

# Build function (without clean)
build_app() {
    log_info "Building $APP_NAME ($BUILD_TYPE)..."
    cd "$SCRIPT_DIR"
    
    if make $BUILD_TYPE; then
        log_success "Build completed successfully"
        return 0
    else
        log_error "Build failed"
        return 1
    fi
}

# Run application
run_app() {
    local app_path="$SCRIPT_DIR/bin/${APP_NAME}.app/Contents/MacOS/${APP_NAME}"
    
    if [ ! -f "$app_path" ]; then
        log_error "Application not found at $app_path"
        log_info "Try building first with: $0 build"
        return 1
    fi
    
    log_info "Starting SonifyV1 object detection application..."
    log_info "Controls: 'g' toggle GUI, 'd' toggle detection, 'o' open video, 'r' restart camera"
    cd "$SCRIPT_DIR"
    
    # Run app directly (foreground)
    "$app_path"
    
    log_info "SonifyV1 application closed"
}

# Check status
check_status() {
    echo -e "\n${BLUE}=== SonifyV1 Object Detection Status ===${NC}"
    
    # Check build status
    local app_path="$SCRIPT_DIR/bin/${APP_NAME}.app/Contents/MacOS/${APP_NAME}"
    if [ -f "$app_path" ]; then
        local build_time=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$app_path")
        echo -e "Build Status: ${GREEN}READY${NC} (Built: $build_time)"
        echo -e "Application:  $app_path"
        echo -e "Features:     ${GREEN}CoreML YOLO${NC}, ${GREEN}MIDI Output${NC}, ${GREEN}OSC Communication${NC}, ${GREEN}Professional GUI${NC}"
    else
        echo -e "Build Status: ${RED}NOT BUILT${NC}"
        echo -e "Run: $0 build"
    fi
    echo ""
}

# Show usage
show_usage() {
    echo "SonifyV1 - Object Detection Application Control Script"
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build    - Build the application (incremental)"
    echo "  clean    - Clean build and rebuild application"
    echo "  run      - Run SonifyV1 object detection application"
    echo "  status   - Show build status"
    echo "  help     - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 clean    # Clean build and run"
    echo "  $0 run      # Run the application"
    echo "  $0 status   # Check build status"
    echo ""
    echo "SonifyV1 Features:"
    echo "  ✅ Real-time object detection (80 COCO classes)"
    echo "  ✅ CoreML YOLOv8 optimization for Apple Silicon" 
    echo "  ✅ MIDI output with musical line crossing system"
    echo "  ✅ OSC communication to Max/MSP"
    echo "  ✅ Professional ImGui interface with tabs"
    echo "  ✅ Interactive line drawing for musical triggers"
    echo "  ✅ Complete configuration save/load system"
}

# Main script logic
case "${1:-help}" in
    "build")
        build_app
        ;;
    "clean")
        if clean_build; then
            log_success "Clean build completed - ready to run!"
            echo ""
            echo "To start the application: $0 run"
        else
            log_error "Clean build failed"
            exit 1
        fi
        ;;
    "run")
        run_app
        ;;
    "status")
        check_status
        ;;
    "help"|*)
        show_usage
        ;;
esac