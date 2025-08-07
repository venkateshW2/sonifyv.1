#!/usr/bin/env python3
"""
Generate working YOLOv8 CoreML models (n/s/m/l) using existing conda environment
Uses: /Users/justmac/miniconda3/envs/sonifyml/bin/python
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

# Use the conda environment python
CONDA_PYTHON = "/Users/justmac/miniconda3/envs/sonifyml/bin/python"

def run_command(cmd, cwd=None):
    """Run command and return success status"""
    try:
        result = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"❌ Error: {result.stderr}")
            return False
        return True
    except Exception as e:
        print(f"❌ Exception: {e}")
        return False

def convert_yolo_to_coreml(model_size):
    """Convert specific YOLOv8 model to CoreML"""
    
    model_name = f"yolov8{model_size}"
    pt_file = f"{model_name}.pt"
    mlpackage_dir = f"{model_name}.mlpackage"
    
    print(f"\n🔄 Processing {model_name}...")
    
    # Check if we already have the model
    if os.path.exists(os.path.join("bin/data/models", mlpackage_dir)):
        print(f"✅ {mlpackage_dir} already exists")
        return True
    
    # Download model if not exists
    if not os.path.exists(pt_file):
        print(f"📥 Downloading {pt_file}...")
        download_cmd = f"{CONDA_PYTHON} -c \"from ultralytics import YOLO; YOLO('{model_name}.pt')\""
        if not run_command(download_cmd):
            print(f"❌ Failed to download {model_name}")
            return False
    
    # Convert to CoreML
    print(f"⚙️  Converting {model_name} to CoreML...")
    convert_cmd = f"{CONDA_PYTHON} -c \"from ultralytics import YOLO; model = YOLO('{pt_file}'); model.export(format='coreml', half=True, nms=True, optimize=True)\""
    
    if run_command(convert_cmd):
        # Move to models directory
        if os.path.exists(mlpackage_dir):
            target_path = os.path.join("bin/data/models", mlpackage_dir)
            if os.path.exists(target_path):
                shutil.rmtree(target_path)
            shutil.move(mlpackage_dir, target_path)
            print(f"✅ {mlpackage_dir} saved to bin/data/models/")
            return True
        else:
            print(f"❌ Conversion failed - {mlpackage_dir} not created")
            return False
    else:
        print(f"❌ Conversion command failed")
        return False

def verify_model(model_path):
    """Verify CoreML model is valid"""
    try:
        verify_cmd = f"{CONDA_PYTHON} -c \"import coremltools as ct; model = ct.models.MLModel('{model_path}'); print('✅ Model loaded successfully'); print('Input:', model.input_description); print('Output:', model.output_description)\""
        return run_command(verify_cmd)
    except Exception as e:
        print(f"❌ Verification failed: {e}")
        return False

def main():
    """Main conversion process"""
    
    print("🚀 YOLOv8 CoreML Model Generator")
    print("=" * 50)
    print("Using conda environment: sonifyml")
    print("Python:", CONDA_PYTHON)
    
    # Ensure models directory exists
    models_dir = "bin/data/models"
    os.makedirs(models_dir, exist_ok=True)
    
    # Models to convert
    model_sizes = ["n", "s", "m", "l"]
    
    success_count = 0
    
    for size in model_sizes:
        if convert_yolo_to_coreml(size):
            model_path = os.path.join(models_dir, f"yolov8{size}.mlpackage")
            if verify_model(model_path):
                print(f"✅ yolov8{size}.mlpackage verified and ready")
                success_count += 1
            else:
                print(f"⚠️  yolov8{size}.mlpackage created but verification failed")
        else:
            print(f"❌ Failed to convert yolov8{size}")
    
    print(f"\n📊 Summary: {success_count}/{len(model_sizes)} models ready")
    
    if success_count > 0:
        print("\n🎉 CoreML models available in: bin/data/models/")
        print("Files:")
        for size in model_sizes:
            path = os.path.join(models_dir, f"yolov8{size}.mlpackage")
            if os.path.exists(path):
                size_mb = sum(f.stat().st_size for f in Path(path).rglob('*') if f.is_file()) / (1024*1024)
                print(f"  - yolov8{size}.mlpackage ({size_mb:.1f} MB)")

if __name__ == "__main__":
    main()
