#!/bin/bash

# SonifyV1 - Compile and Run Script
# Usage: ./run.sh [clean|run|build|help]

set -e  # Exit on any error

PROJECT_DIR="/Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1"
BIN_DIR="$PROJECT_DIR/bin"
APP_PATH="$BIN_DIR/SonifyV1.app/Contents/MacOS/SonifyV1"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

echo_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

echo_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}    SonifyV1 Build & Run Tool    ${NC}"
    echo -e "${BLUE}================================${NC}"
    echo
}

show_help() {
    print_header
    echo "Usage: ./run.sh [command]"
    echo
    echo "Commands:"
    echo "  clean   - Clean build files and rebuild from scratch"
    echo "  build   - Compile the application (default)"
    echo "  run     - Compile and run the application"
    echo "  help    - Show this help message"
    echo
    echo "Examples:"
    echo "  ./run.sh        # Build only"
    echo "  ./run.sh run    # Build and run"
    echo "  ./run.sh clean  # Clean build"
    echo
}

clean_build() {
    echo_info "Cleaning build files..."
    cd "$PROJECT_DIR"
    
    if [ -d "obj" ]; then
        rm -rf obj/
        echo_success "Removed obj/ directory"
    fi
    
    if [ -f "bin/SonifyV1.app/Contents/MacOS/SonifyV1" ]; then
        rm -f "bin/SonifyV1.app/Contents/MacOS/SonifyV1"
        echo_success "Removed existing binary"
    fi
    
    make clean 2>/dev/null || true
    echo_success "Clean completed"
}

build_app() {
    echo_info "Starting build process..."
    cd "$PROJECT_DIR"
    
    echo_info "Current directory: $(pwd)"
    echo_info "Checking required files..."
    
    # Check for required model files
    if [ ! -f "bin/data/models/yolov8n.onnx" ]; then
        echo_warning "YOLO model not found at bin/data/models/yolov8n.onnx"
        echo_warning "Detection will be disabled without the model file"
    else
        echo_success "YOLO model found"
    fi
    
    if [ ! -f "bin/data/models/coco.names" ]; then
        echo_warning "COCO names file not found at bin/data/models/coco.names"
        echo_warning "Detection will fail without class names"
    else
        echo_success "COCO names file found"
    fi
    
    echo_info "Building with make..."
    echo_info "Build output:"
    echo "----------------------------------------"
    
    # Run make with verbose output
    make -j$(sysctl -n hw.ncpu) 2>&1 | while IFS= read -r line; do
        if [[ "$line" == *"error"* ]] || [[ "$line" == *"Error"* ]]; then
            echo_error "$line"
        elif [[ "$line" == *"warning"* ]] || [[ "$line" == *"Warning"* ]]; then
            echo_warning "$line"
        else
            echo "$line"
        fi
    done
    
    echo "----------------------------------------"
    
    # Check if build was successful
    if [ -f "$APP_PATH" ]; then
        echo_success "Build completed successfully!"
        echo_info "Binary created at: $APP_PATH"
        
        # Show binary info
        echo_info "Binary size: $(ls -lh "$APP_PATH" | awk '{print $5}')"
        echo_info "Binary permissions: $(ls -l "$APP_PATH" | awk '{print $1}')"
        
        return 0
    else
        echo_error "Build failed - no binary created"
        return 1
    fi
}

run_app() {
    echo_info "Preparing to run SonifyV1..."
    
    if [ ! -f "$APP_PATH" ]; then
        echo_error "Binary not found at $APP_PATH"
        echo_info "Attempting to build first..."
        build_app || return 1
    fi
    
    cd "$BIN_DIR"
    echo_info "Changed to binary directory: $(pwd)"
    
    # Check library dependencies
    echo_info "Checking library dependencies..."
    if command -v otool >/dev/null 2>&1; then
        echo_info "Dynamic library dependencies:"
        otool -L "$APP_PATH" | grep -E "(onnx|omp)" || echo "  No ONNX/OpenMP dependencies found in otool output"
    fi
    
    # Set library path and run
    echo_info "Setting DYLD_LIBRARY_PATH=."
    echo_info "Launching application..."
    echo "========================================"
    echo_success "SonifyV1 is starting..."
    echo_warning "Press Ctrl+C in this terminal to stop the application"
    echo "========================================"
    echo
    
    # Run with library path and capture exit code
    DYLD_LIBRARY_PATH=. ./SonifyV1.app/Contents/MacOS/SonifyV1
    EXIT_CODE=$?
    
    echo
    echo "========================================"
    if [ $EXIT_CODE -eq 0 ]; then
        echo_success "Application exited normally (code: $EXIT_CODE)"
    else
        echo_warning "Application exited with code: $EXIT_CODE"
    fi
    echo "========================================"
    
    return $EXIT_CODE
}

# Main script logic
case "${1:-build}" in
    "help"|"-h"|"--help")
        show_help
        ;;
    "clean")
        print_header
        clean_build
        echo_success "Clean operation completed"
        ;;
    "build"|"")
        print_header
        build_app
        ;;
    "run")
        print_header
        build_app && run_app
        ;;
    *)
        echo_error "Unknown command: $1"
        echo
        show_help
        exit 1
        ;;
esac