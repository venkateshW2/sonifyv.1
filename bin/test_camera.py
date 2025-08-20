#!/usr/bin/env python3
"""
Simple camera test to verify camera access
"""

import cv2
import sys

def test_camera():
    print("Testing camera access...")
    
    try:
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("❌ Error: Cannot open camera 0")
            return False
        
        ret, frame = cap.read()
        if not ret:
            print("❌ Error: Cannot read from camera")
            return False
        
        height, width = frame.shape[:2]
        print(f"✅ Camera working: {width}x{height}")
        
        cap.release()
        return True
        
    except Exception as e:
        print(f"❌ Camera test failed: {e}")
        return False

if __name__ == "__main__":
    success = test_camera()
    sys.exit(0 if success else 1)