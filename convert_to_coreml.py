#!/usr/bin/env python3
"""
Convert YOLOv8 ONNX model to CoreML format for M1 Mac optimization
"""

import os
import sys

def convert_yolo_to_coreml():
    """Convert YOLOv8 ONNX to CoreML format"""
    
    try:
        import coremltools as ct
        print("✅ CoreML Tools found")
    except ImportError:
        print("❌ CoreML Tools not found. Installing...")
        os.system("pip3 install coremltools")
        import coremltools as ct
        
    # Paths
    onnx_path = "bin/data/models/yolov8n.onnx"
    coreml_path = "bin/data/models/yolov8n.mlpackage"
    
    if not os.path.exists(onnx_path):
        print(f"❌ ONNX model not found at: {onnx_path}")
        print("Please ensure you have the YOLOv8 ONNX model in the correct location")
        return False
        
    print(f"🔄 Converting {onnx_path} to CoreML...")
    
    try:
        # Convert ONNX to CoreML
        print("Converting ONNX model to CoreML format...")
        model = ct.convert(
            onnx_path,
            convert_to="mlprogram",  # Use ML Program format for M1 optimization
            compute_precision=ct.precision.FLOAT16,  # Use FP16 for better M1 performance
            compute_units=ct.ComputeUnit.ALL  # Use Neural Engine + GPU
        )
        
        # Add metadata
        model.short_description = "YOLOv8n vehicle detection model optimized for M1 Mac"
        model.author = "SonifyV1"
        model.license = "MIT"
        model.version = "1.0"
        
        # Save the model
        model.save(coreml_path)
        print(f"✅ CoreML model saved to: {coreml_path}")
        
        # Print model info
        print("\n📊 Model Information:")
        print(f"Input shape: {model.input_description}")
        print(f"Output shape: {model.output_description}")
        
        return True
        
    except Exception as e:
        print(f"❌ Conversion failed: {e}")
        return False

if __name__ == "__main__":
    print("🚀 YOLOv8 ONNX → CoreML Converter for M1 Mac")
    print("=" * 50)
    
    success = convert_yolo_to_coreml()
    
    if success:
        print("\n✅ Conversion completed successfully!")
        print("💡 Next steps:")
        print("1. Implement CoreML inference in ofApp.cpp")
        print("2. Build and test the application")
        print("3. Compare performance with ONNX version")
    else:
        print("\n❌ Conversion failed. Check error messages above.")
        sys.exit(1)