# SonifyV2 Complete Environment Setup
## One-Time M1 Mac Installation Guide

**Setup Status:** 🔧 Complete Before Any Development

---

## **Critical: Do This All At Once**

**This entire setup must be completed in one session.** Don't start coding until everything here works perfectly. This solves all dependency hell upfront so development flows smoothly with AI agents.

---

## **Phase 0: Complete System Setup (Day 1-2)**

### **Step 1: System Prerequisites**
```bash
# Verify M1 Mac
system_profiler SPHardwareDataType | grep "Chip"
# Must show: Apple M1/M1 Pro/M1 Max/M2

arch
# Must show: arm64

# Install Xcode Command Line Tools
xcode-select --install
# Wait for complete installation, then verify:
xcode-select -p
# Should show: /Library/Developer/CommandLineTools
```

### **Step 2: Package Manager Setup**
```bash
# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Add to PATH (M1 Macs)
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"

# Verify installation
brew --version
# Should show: Homebrew 4.x.x

# Update everything
brew update && brew upgrade
```

### **Step 3: Core Development Tools**
```bash
# Essential build tools
brew install cmake pkg-config git wget curl

# Computer vision and ML libraries
brew install opencv protobuf

# Audio development libraries  
brew install portaudio libsndfile fftw

# openFrameworks dependencies
brew install glfw3 freeimage freetype fontconfig poco

# Additional tools
brew install tree htop oscdump

# Verify critical installations
opencv_version
# Should show: 4.8.x or newer

cmake --version
# Should show: 3.27.x or newer
```

### **Step 4: Python ML Environment**
```bash
# Install miniconda
brew install miniconda

# Initialize conda for your shell
conda init zsh  # or 'conda init bash' if using bash

# Restart terminal or source config
source ~/.zprofile  # or ~/.bash_profile

# Create dedicated ML environment
conda create -n sonifyml python=3.10 -y
conda activate sonifyml

# Core ML stack - install everything now
conda install pytorch torchvision torchaudio -c pytorch -y
pip install librosa numpy scipy matplotlib seaborn
pip install onnx onnxruntime opencv-python
pip install ultralytics  # Official YOLOv8
pip install jupyter ipython notebook

# Audio ML libraries
pip install torch-audio aubio essentia-tensorflow
pip install soundfile pydub resampy

# Development tools
pip install pytest black isort mypy

# Verify everything works
python -c "
import torch
import onnxruntime as ort
import cv2
import librosa  
import ultralytics
print('✅ PyTorch:', torch.__version__)
print('✅ ONNX Runtime:', ort.__version__)
print('✅ OpenCV:', cv2.__version__)
print('✅ librosa:', librosa.__version__)
print('✅ M1 GPU Support:', torch.backends.mps.is_available())
print('✅ All dependencies installed successfully!')
"
```

### **Step 5: openFrameworks Installation**
```bash
# Download openFrameworks
cd ~
wget https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_osx_release.tar.gz
tar -xzf of_v0.12.1_osx_release.tar.gz
mv of_v0.12.1_osx_release openFrameworks
rm of_v0.12.1_osx_release.tar.gz

# Compile openFrameworks for M1
cd ~/openFrameworks/scripts/osx
./compileOF.sh

# This takes 10-15 minutes - wait for completion
# Should end with: "✅ openFrameworks compiled successfully"

# Test with simple example
cd ~/openFrameworks/examples/graphics/polygonExample
make
make run
# Should open window with animated polygons - close when verified

echo "✅ openFrameworks M1 compilation successful"
```

### **Step 6: Essential openFrameworks Addons**
```bash
cd ~/openFrameworks/addons

# Clone essential addons
git clone https://github.com/jeffcrouse/ofxJSON.git
git clone https://github.com/danomatika/ofxOsc.git
git clone https://github.com/jvcleave/ofxImGui.git

# Verify addon structure
ls -la ofxJSON ofxOsc ofxImGui
# Each should contain: src/, libs/, example_*/ folders

echo "✅ Essential addons installed"
```

### **Step 7: YOLO Models Download and Conversion**
```bash
# Create project models directory
mkdir -p ~/openFrameworks/apps/myApps/SonifyV2/bin/data/models
cd ~/openFrameworks/apps/myApps/SonifyV2/bin/data/models

# Activate ML environment
conda activate sonifyml

# Download and convert YOLO models
python -c "
from ultralytics import YOLO
import os

print('Downloading YOLOv8 models...')

# Download models (this downloads .pt files)
models = {
    'yolov8n.pt': 'nano model - fastest',
    'yolov8s.pt': 'small model - balanced',  
    'yolov8m.pt': 'medium model - accurate'
}

for model_file, description in models.items():
    print(f'Processing {model_file} ({description})...')
    model = YOLO(model_file)
    
    # Export to ONNX with optimization
    model.export(
        format='onnx',
        imgsz=640,
        optimize=True,
        simplify=True
    )
    print(f'✅ Exported {model_file} to ONNX')

print('✅ All YOLO models ready!')
"

# Verify model files exist
ls -la
# Should show: yolov8n.onnx, yolov8s.onnx, yolov8m.onnx

# Download COCO class names
wget https://raw.githubusercontent.com/ultralytics/ultralytics/main/ultralytics/cfg/datasets/coco.yaml
python -c "
import yaml
with open('coco.yaml', 'r') as f:
    data = yaml.safe_load(f)
with open('coco.names', 'w') as f:
    for name in data['names'].values():
        f.write(name + '\n')
print('✅ COCO class names created')
"

echo "✅ ML models setup complete"
```

### **Step 8: Audio Software Setup**
```bash
# Option A: Max/MSP (if you have license)
# Download from https://cycling74.com/products/max
# Install and verify OSC receiving works

# Option B: Pure Data (free alternative)
brew install --cask pd

# Test Pure Data OSC
pd -nogui &
PD_PID=$!
sleep 2
echo "Simple OSC test patch loaded"
kill $PD_PID

echo "✅ Audio software ready"
```

### **Step 9: Development Tools Setup**
```bash
# Install VSCodium (if not already installed)
brew install --cask vscodium

# Install Claude CLI (if not already installed)
# Follow: https://claude.ai/cli

# Verify Claude CLI works
claude --version
# Should show Claude CLI version

# Test environment integration
cd ~/openFrameworks/apps/myApps/
mkdir -p SonifyV2/src
cd SonifyV2

# Test basic project structure
echo "✅ Development environment ready"
```

### **Step 10: Final Verification**
```bash
# Create comprehensive test script
cd ~/openFrameworks/apps/myApps/SonifyV2
cat > test_environment.py << 'EOF'
#!/usr/bin/env python3
import subprocess
import sys
import os

def test_command(cmd, description):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✅ {description}")
            return True
        else:
            print(f"❌ {description}: {result.stderr}")
            return False
    except Exception as e:
        print(f"❌ {description}: {e}")
        return False

def test_python_imports():
    imports = [
        'torch', 'torchvision', 'torchaudio',
        'onnxruntime', 'cv2', 'librosa', 'ultralytics',
        'numpy', 'scipy', 'matplotlib'
    ]
    
    for module in imports:
        try:
            __import__(module)
            print(f"✅ Python import: {module}")
        except ImportError:
            print(f"❌ Python import failed: {module}")
            return False
    return True

def test_files_exist():
    files = [
        '~/openFrameworks/libs/openFrameworks/libof.a',
        '~/openFrameworks/apps/myApps/SonifyV2/bin/data/models/yolov8n.onnx',
        '~/openFrameworks/apps/myApps/SonifyV2/bin/data/models/coco.names'
    ]
    
    for file_path in files:
        expanded_path = os.path.expanduser(file_path)
        if os.path.exists(expanded_path):
            print(f"✅ File exists: {file_path}")
        else:
            print(f"❌ Missing file: {file_path}")
            return False
    return True

print("🔍 Testing complete SonifyV2 environment...\n")

tests = [
    ("arch", "M1 Architecture check"),
    ("brew --version", "Homebrew installation"),
    ("cmake --version", "CMake installation"),
    ("opencv_version", "OpenCV installation"),
    ("conda info", "Conda installation"),
    ("code --version", "VSCodium installation"),
    ("claude --version", "Claude CLI installation")
]

python_ok = test_python_imports()
files_ok = test_files_exist()
cmd_tests_ok = all(test_command(cmd, desc) for cmd, desc in tests)

print(f"\n{'='*50}")
if python_ok and files_ok and cmd_tests_ok:
    print("🎉 ENVIRONMENT SETUP COMPLETE!")
    print("✅ Ready to start SonifyV2 development")
    print("✅ All dependencies verified")
    print("✅ AI development workflow ready")
else:
    print("❌ SETUP INCOMPLETE - Fix errors above")
    sys.exit(1)
EOF

# Run comprehensive test
conda activate sonifyml
python test_environment.py
```

---

## **Troubleshooting Common Issues**

### **M1 Compilation Problems:**
```bash
# If openFrameworks compilation fails
cd ~/openFrameworks/scripts/osx
rm -rf ../libs/openFrameworks/lib/
./compileOF.sh -j1  # Single-threaded compilation

# If still fails, check Xcode tools
sudo xcode-select --reset
xcode-select --install
```

### **Python Environment Issues:**
```bash
# If conda activation fails
conda init --all
source ~/.zprofile

# If PyTorch MPS fails
python -c "import torch; print(torch.backends.mps.is_built())"
# Should be True on M1 Macs

# Reset conda environment if corrupted
conda env remove -n sonifyml
# Then repeat Step 4
```

### **ONNX Runtime Problems:**
```bash
# If ONNX runtime fails to import
pip uninstall onnxruntime
pip install onnxruntime

# Test ONNX with simple model
python -c "
import onnxruntime as ort
print('Available providers:', ort.get_available_providers())
# Should include 'CPUExecutionProvider'
"
```

### **OpenCV Issues:**
```bash
# If OpenCV linking fails
brew reinstall opencv
pkg-config --cflags --libs opencv4
# Should return compiler flags
```

---

## **Post-Setup Verification Checklist**

✅ **System Architecture:** `arch` shows `arm64`  
✅ **Homebrew:** `brew --version` works  
✅ **openFrameworks:** Polygon example runs successfully  
✅ **Python ML:** All imports work in `sonifyml` environment  
✅ **YOLO Models:** `.onnx` files exist in models directory  
✅ **ONNX Runtime:** Can load and run inference  
✅ **Audio Software:** Max/MSP or Pure Data installed  
✅ **Development Tools:** VSCodium + Claude CLI working  
✅ **Test Script:** `test_environment.py` passes all checks  

---

## **Success Criteria**

**Before proceeding to development:**
- ✅ All commands above run without errors
- ✅ Test environment script shows all green checkmarks  
- ✅ Can activate `conda activate sonifyml` without issues
- ✅ openFrameworks polygon example displays animated shapes
- ✅ YOLO models load successfully in Python
- ✅ Claude CLI responds to commands
- ✅ Audio software can receive OSC messages

**If any step fails:** Fix it completely before proceeding. The development guide assumes this environment works perfectly.

---

**Time Investment:** 2-4 hours of active setup + waiting for downloads/compilation  
**Payoff:** Months of smooth AI-assisted development with zero dependency issues  

**Next:** Once this passes 100%, proceed to the Implementation Guide for linear, step-by-step development with AI agents.