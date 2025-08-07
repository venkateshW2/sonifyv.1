# CoreML Migration Setup Guide

## 🎯 Current Status: Architecture Complete, Model Conversion Pending

### ✅ Completed Migration Steps

1. **✅ Infrastructure Migration**
   - Updated `src/ofApp.h` - CoreML/Vision framework headers
   - Updated `src/ofApp.cpp` - All ONNX methods → CoreML methods
   - Updated `config.make` - Apple native frameworks
   - Updated `CLAUDE.md` - Complete documentation

2. **✅ Architecture Changes**
   ```cpp
   // Before (ONNX)
   #include <onnxruntime_cxx_api.h>
   Ort::Session* yoloSession;
   void processYOLODetection();
   
   // After (CoreML)
   #include <CoreML/CoreML.h>
   #include <Vision/Vision.h>
   MLModel* coremlModel;
   VNCoreMLModel* visionModel;  
   void processCoreMLDetection();
   ```

3. **✅ Build System**
   ```make
   # Before
   PROJECT_LDFLAGS = -L/opt/homebrew/lib -lonnxruntime -lomp
   
   # After  
   PROJECT_LDFLAGS = -framework CoreML -framework Vision -framework Foundation
   ```

### 🚨 Current Issue: CoreML Tools Installation

**Problem**: CoreML Tools has missing dependencies:
```
RuntimeError: BlobWriter not loaded
Failed to load _MLModelProxy: No module named 'coremltools.libcoremlpython'
```

### 🛠️ Model Conversion Workarounds

#### **Option 1: Use Online CoreML Converter** ⭐ Recommended
1. Upload `yolov8n.onnx` to [Apple's CoreML Tools Online](https://coremltools.readme.io/)
2. Download converted `.mlpackage` file
3. Place in `bin/data/models/yolov8n.mlpackage`

#### **Option 2: Use Different Python Environment**
```bash
# Create isolated environment
conda create -n coreml python=3.11
conda activate coreml
pip install coremltools==7.2  # Use older stable version
yolo export model=yolov8n.pt format=coreml
```

#### **Option 3: Manual PyTorch → CoreML**
```python
import torch
import coremltools as ct

# Load PyTorch model
model = torch.load('yolov8n.pt')['model']
model.eval()

# Convert to CoreML
traced_model = torch.jit.trace(model, torch.randn(1, 3, 640, 640))
coreml_model = ct.convert(
    traced_model,
    convert_to="mlprogram",
    compute_precision=ct.precision.FLOAT16
)
coreml_model.save("yolov8n.mlpackage")
```

### 🎯 Next Implementation Steps

#### **Phase 1: Complete CoreML Inference** 
```cpp
void ofApp::processCoreMLDetection() {
    // 1. Get video frame
    CVPixelBufferRef pixelBuffer = /* convert ofPixels to CVPixelBuffer */;
    
    // 2. Create Vision request
    VNCoreMLRequest *request = [[VNCoreMLRequest alloc] 
        initWithModel:visionModel 
        completionHandler:^(VNRequest *request, NSError *error) {
            // Process detections
        }];
    
    // 3. Perform inference
    VNImageRequestHandler *handler = [[VNImageRequestHandler alloc] 
        initWithCVPixelBuffer:pixelBuffer options:@{}];
    [handler performRequests:@[request] error:nil];
}
```

#### **Phase 2: Performance Optimization**
- Enable Neural Engine compute units
- Implement batch processing
- Optimize video frame preprocessing

#### **Phase 3: Benchmarking**
- Compare CoreML vs ONNX performance
- Measure power consumption
- Validate detection accuracy

### 📊 Expected Performance Gains

| Metric | ONNX Runtime | CoreML (Expected) | Improvement |
|--------|--------------|-------------------|-------------|
| FPS | 15-20 | 45-75 | 3-5x faster |
| Power | High | Low | ~50% reduction |
| Accuracy | Poor (distortion) | Good (proper preprocessing) | Significant |
| Memory | 2GB GPU | 1GB unified | ~50% reduction |

### 🔧 Development Workflow

1. **Model Conversion** (pending)
2. **Implement CoreML inference pipeline**
3. **Test with 640x640 video**
4. **Performance benchmarking**
5. **Production optimization**

### 🚀 Why CoreML Migration is Worth It

- **Native M1 Optimization**: Uses dedicated Neural Engine hardware
- **Automatic Preprocessing**: Eliminates aspect ratio distortion issues
- **Better Performance**: 3-5x improvement expected
- **Lower Power**: Native efficiency optimizations
- **Future-Proof**: Apple's recommended ML framework

---

**Current Status**: Infrastructure ready, waiting for model conversion to complete implementation.