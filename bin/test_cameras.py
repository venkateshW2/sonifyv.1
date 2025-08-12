#!/usr/bin/env python3
"""
Test available cameras on the system
"""

import cv2

def test_cameras():
    """Test which camera IDs are available"""
    print("🔍 Testing available cameras...")
    
    available_cameras = []
    
    for i in range(10):  # Test camera IDs 0-9
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            ret, frame = cap.read()
            if ret:
                height, width = frame.shape[:2]
                print(f"✅ Camera {i}: {width}x{height}")
                available_cameras.append(i)
            else:
                print(f"⚠️  Camera {i}: Opens but no frame")
            cap.release()
        else:
            print(f"❌ Camera {i}: Not available")
    
    print(f"\n📊 Summary: {len(available_cameras)} cameras available: {available_cameras}")
    return available_cameras

if __name__ == "__main__":
    test_cameras()