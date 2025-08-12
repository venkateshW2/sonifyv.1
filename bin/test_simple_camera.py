#!/usr/bin/env python3
"""
Simple camera test without MediaPipe
"""

import cv2
import time

def test_simple_camera():
    """Test basic camera capture"""
    print("📹 Testing simple camera capture...")
    
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("❌ Cannot open camera")
        return False
    
    print("✅ Camera opened successfully")
    
    # Test reading frames
    for i in range(10):
        ret, frame = cap.read()
        if ret:
            print(f"✅ Frame {i+1}: {frame.shape}")
        else:
            print(f"❌ Failed to read frame {i+1}")
            break
        time.sleep(0.1)
    
    cap.release()
    print("✅ Camera test completed")
    return True

if __name__ == "__main__":
    test_simple_camera()