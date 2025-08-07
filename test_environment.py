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
        '/Users/justmac/Documents/OF/of_v0.12.1_osx_release/libs/openFrameworksCompiled/lib/osx/libopenFrameworks.a',
        '/Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1/bin/data/models/yolov8n.onnx',
        '/Users/justmac/Documents/OF/of_v0.12.1_osx_release/apps/myApps/SonifyV1/bin/data/models/coco.names'
    ]
    
    for file_path in files:
        if os.path.exists(file_path):
            print(f"✅ File exists: {os.path.basename(file_path)}")
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