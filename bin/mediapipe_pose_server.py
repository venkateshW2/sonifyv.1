#!/usr/bin/env python3
"""
MediaPipe Pose Detection UDP Server for SonifyV1
Streams pose landmarks via UDP to openFrameworks application

Usage: python3 mediapipe_pose_server.py
"""

import cv2
import mediapipe as mp
import socket
import json
import time
import argparse
import sys
from typing import Dict, List, Optional

class PoseDetectionServer:
    def __init__(self, 
                 camera_id: int = 0, 
                 udp_host: str = 'localhost', 
                 udp_port: int = 8080,
                 confidence_threshold: float = 0.5,
                 max_people: int = 8):
        """
        Initialize MediaPipe pose detection server
        
        Args:
            camera_id: Camera device ID (0 for default)
            udp_host: UDP destination host
            udp_port: UDP destination port
            confidence_threshold: Minimum confidence for pose detection
            max_people: Maximum number of people to detect
        """
        self.camera_id = camera_id
        self.udp_host = udp_host
        self.udp_port = udp_port
        self.confidence_threshold = confidence_threshold
        self.max_people = max_people
        
        # Initialize MediaPipe pose detection (optimized for smooth, stable tracking)
        self.mp_pose = mp.solutions.pose
        self.pose = self.mp_pose.Pose(
            static_image_mode=False,
            model_complexity=0,  # 0=lite (FASTEST), 1=full, 2=heavy  
            smooth_landmarks=True,  # Enable for smooth tracking - reduces jitter
            enable_segmentation=False,
            min_detection_confidence=confidence_threshold,
            min_tracking_confidence=0.7  # Higher for more stable tracking
        )
        
        # UDP socket for communication
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Camera capture
        self.cap = None
        
        # Performance tracking
        self.frame_count = 0
        self.start_time = time.time()
        self.last_fps_print = time.time()
        self.fps_interval = 5.0  # Print FPS every 5 seconds
        
        print(f"🎭 MediaPipe Pose Server initialized")
        print(f"   Camera: {camera_id}")
        print(f"   UDP: {udp_host}:{udp_port}")
        print(f"   Confidence: {confidence_threshold}")
        print(f"   Max People: {max_people}")
    
    def initialize_camera(self) -> bool:
        """Initialize camera capture"""
        try:
            self.cap = cv2.VideoCapture(self.camera_id)
            if not self.cap.isOpened():
                print(f"❌ Error: Cannot open camera {self.camera_id}")
                return False
            
            # Test if we can read a frame
            ret, test_frame = self.cap.read()
            if not ret:
                print(f"❌ Error: Cannot read from camera {self.camera_id}")
                return False
            
            height, width = test_frame.shape[:2]
            print(f"✅ Camera {self.camera_id} initialized: {width}x{height}")
            
            # Try to set optimal resolution, but accept what camera supports
            try:
                # Try 640x480 first (standard webcam resolution)
                self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                self.cap.set(cv2.CAP_PROP_FPS, 30)
                self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # Reduce buffer for lower latency
                
                # Get actual resolution after setting
                actual_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                actual_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
                print(f"🎯 Camera set to {actual_width}x{actual_height} (will crop to square for pose detection)")
                
                # Store for cropping to square later
                self.frame_width = actual_width
                self.frame_height = actual_height
                
            except Exception as e:
                print(f"⚠️ Could not set camera properties: {e}")
                # Use whatever the camera gives us
                self.frame_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                self.frame_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            
            return True
        except Exception as e:
            print(f"❌ Camera initialization error: {e}")
            return False
    
    def format_pose_landmarks_with_transform(self, results, orig_w: int, orig_h: int, 
                                            crop_offset_x: int, crop_offset_y: int, crop_size: int) -> Dict:
        """
        Format pose landmarks with smart coordinate transformation to 640x640 openFrameworks space
        
        Args:
            results: MediaPipe pose detection results
            orig_w, orig_h: Original camera frame dimensions 
            crop_offset_x, crop_offset_y: Crop offsets applied to frame
            crop_size: Size of square crop used for pose detection
        
        Returns:
            Dict with pose data in 640x640 coordinate space
        """
        pose_dict = {
            "timestamp": time.time(),
            "frame_size": {"width": 640, "height": 640},  # Always report 640x640 to openFrameworks
            "poses": []
        }
        
        if not results.pose_landmarks:
            return pose_dict
            
        # Transform coordinates from cropped space to openFrameworks 640x640 space
        landmarks = []
        for idx, landmark in enumerate(results.pose_landmarks.landmark):
            if landmark.visibility > 0.1:  # Only include visible landmarks
                # Convert from normalized crop coordinates [0-1] to crop pixel coordinates
                crop_x = landmark.x * crop_size
                crop_y = landmark.y * crop_size
                
                # Transform back to original frame coordinates
                orig_x = crop_x + crop_offset_x
                orig_y = crop_y + crop_offset_y
                
                # Transform to openFrameworks 640x640 space (stretched to square)
                of_x = (orig_x / orig_w) * 640
                of_y = (orig_y / orig_h) * 640
                
                landmark_dict = {
                    "id": idx,
                    "x": of_x,
                    "y": of_y,
                    "z": landmark.z,  # Relative depth
                    "confidence": landmark.visibility,
                    "joint_name": self.get_joint_name(idx)
                }
                landmarks.append(landmark_dict)
        
        if landmarks:
            person_pose = {
                "person_id": 1,  # Single person for now
                "confidence": sum(lm["confidence"] for lm in landmarks) / len(landmarks),
                "landmarks": landmarks
            }
            pose_dict["poses"].append(person_pose)
            
        return pose_dict
    
    def format_pose_landmarks(self, results, frame_width: int, frame_height: int) -> Dict:
        """
        Format pose landmarks for UDP transmission
        
        Args:
            results: MediaPipe pose detection results
            frame_width: Video frame width
            frame_height: Video frame height
            
        Returns:
            Dictionary with pose data for JSON transmission
        """
        pose_data = {
            'timestamp': time.time(),
            'frame_size': {'width': frame_width, 'height': frame_height},
            'poses': []
        }
        
        if results.pose_landmarks:
            # MediaPipe detects single person, but we format for multi-person structure
            landmarks = []
            
            for idx, landmark in enumerate(results.pose_landmarks.landmark):
                # Convert normalized coordinates to pixel coordinates
                x = int(landmark.x * frame_width)
                y = int(landmark.y * frame_height)
                z = landmark.z  # Relative depth
                visibility = landmark.visibility
                
                # Only include landmarks above confidence threshold
                if visibility >= self.confidence_threshold:
                    landmarks.append({
                        'id': idx,
                        'x': x,
                        'y': y,
                        'z': z,
                        'confidence': visibility,
                        'joint_name': self.get_joint_name(idx)
                    })
            
            if landmarks:
                pose_data['poses'].append({
                    'person_id': 0,  # Single person for now
                    'confidence': max([lm['confidence'] for lm in landmarks]),
                    'landmarks': landmarks
                })
        
        return pose_data
    
    def get_joint_name(self, landmark_id: int) -> str:
        """Get human-readable joint name from MediaPipe landmark ID"""
        joint_names = {
            0: 'nose', 1: 'left_eye_inner', 2: 'left_eye', 3: 'left_eye_outer',
            4: 'right_eye_inner', 5: 'right_eye', 6: 'right_eye_outer',
            7: 'left_ear', 8: 'right_ear', 9: 'mouth_left', 10: 'mouth_right',
            11: 'left_shoulder', 12: 'right_shoulder', 13: 'left_elbow', 14: 'right_elbow',
            15: 'left_wrist', 16: 'right_wrist', 17: 'left_pinky', 18: 'right_pinky',
            19: 'left_index', 20: 'right_index', 21: 'left_thumb', 22: 'right_thumb',
            23: 'left_hip', 24: 'right_hip', 25: 'left_knee', 26: 'right_knee',
            27: 'left_ankle', 28: 'right_ankle', 29: 'left_heel', 30: 'right_heel',
            31: 'left_foot_index', 32: 'right_foot_index'
        }
        return joint_names.get(landmark_id, f'joint_{landmark_id}')
    
    def send_udp_data(self, pose_data: Dict) -> bool:
        """
        Send pose data via UDP
        
        Args:
            pose_data: Formatted pose data dictionary
            
        Returns:
            True if successful, False otherwise
        """
        try:
            # Convert to JSON and encode
            json_data = json.dumps(pose_data, separators=(',', ':'))  # Compact JSON
            message = json_data.encode('utf-8')
            
            # Send via UDP
            self.udp_socket.sendto(message, (self.udp_host, self.udp_port))
            return True
        except Exception as e:
            print(f"❌ UDP transmission error: {e}")
            return False
    
    def run(self) -> None:
        """Main processing loop"""
        if not self.initialize_camera():
            return
        
        print(f"🚀 Starting pose detection server...")
        print(f"📡 Broadcasting to {self.udp_host}:{self.udp_port}")
        print(f"Press 'q' to quit, 'i' for info")
        
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    print("❌ Failed to read from camera")
                    break
                
                # Convert BGR to RGB for MediaPipe
                rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                
                # Store original frame dimensions for coordinate transformation
                orig_h, orig_w = rgb_frame.shape[:2]
                
                # Crop to square for better pose detection (center crop)
                if orig_h != orig_w:
                    size = min(orig_h, orig_w)
                    start_x = (orig_w - size) // 2
                    start_y = (orig_h - size) // 2
                    rgb_frame_cropped = rgb_frame[start_y:start_y+size, start_x:start_x+size]
                    
                    # Store crop offset for coordinate transformation
                    crop_offset_x = start_x
                    crop_offset_y = start_y
                    crop_size = size
                else:
                    rgb_frame_cropped = rgb_frame
                    crop_offset_x = 0
                    crop_offset_y = 0
                    crop_size = orig_w
                
                # Process pose detection on cropped frame
                results = self.pose.process(rgb_frame_cropped)
                
                # Format pose data with smart coordinate transformation
                # Transform from cropped coordinates back to openFrameworks 640x640 space
                pose_data = self.format_pose_landmarks_with_transform(
                    results, orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                )
                
                # Send data via UDP
                if pose_data['poses']:  # Only send if poses detected
                    self.send_udp_data(pose_data)
                
                # Performance tracking
                self.frame_count += 1
                
                # Print FPS periodically
                current_time = time.time()
                if current_time - self.last_fps_print >= self.fps_interval:
                    elapsed = current_time - self.start_time
                    fps = self.frame_count / elapsed if elapsed > 0 else 0
                    print(f"🚀 Performance: {fps:.1f} FPS | Frames: {self.frame_count} | Runtime: {elapsed:.1f}s")
                    self.last_fps_print = current_time
                
                # Optional: Display frame with pose overlay (for debugging)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
                elif cv2.waitKey(1) & 0xFF == ord('i'):
                    self.print_info()
        
        except KeyboardInterrupt:
            print("\n🛑 Shutting down pose detection server...")
        
        finally:
            self.cleanup()
    
    def print_info(self) -> None:
        """Print performance and status information"""
        elapsed = time.time() - self.start_time
        fps = self.frame_count / elapsed if elapsed > 0 else 0
        
        print(f"\n📊 Pose Detection Server Status:")
        print(f"   Runtime: {elapsed:.1f}s")
        print(f"   Frames processed: {self.frame_count}")
        print(f"   FPS: {fps:.1f}")
        print(f"   UDP: {self.udp_host}:{self.udp_port}")
        print(f"   Camera: {self.camera_id}")
    
    def cleanup(self) -> None:
        """Clean up resources"""
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()
        self.udp_socket.close()
        print("✅ Cleanup completed")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='MediaPipe Pose Detection UDP Server')
    parser.add_argument('--camera', type=int, default=0, help='Camera device ID (default: 0)')
    parser.add_argument('--host', type=str, default='localhost', help='UDP host (default: localhost)')
    parser.add_argument('--port', type=int, default=8080, help='UDP port (default: 8080)')
    parser.add_argument('--confidence', type=float, default=0.5, help='Confidence threshold (default: 0.5)')
    parser.add_argument('--max-people', type=int, default=8, help='Maximum people to detect (default: 8)')
    
    args = parser.parse_args()
    
    # Create and run pose detection server
    server = PoseDetectionServer(
        camera_id=args.camera,
        udp_host=args.host,
        udp_port=args.port,
        confidence_threshold=args.confidence,
        max_people=args.max_people
    )
    
    server.run()

if __name__ == "__main__":
    main()