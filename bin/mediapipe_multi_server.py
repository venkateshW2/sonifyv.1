#!/usr/bin/env python3
"""
MediaPipe Multi-Detection UDP Server for SonifyV1
Supports pose, hands, face mesh, and selfie segmentation detection
Streams detection data via UDP to openFrameworks application

Usage: python3 mediapipe_multi_server.py [options]
"""

import cv2
import mediapipe as mp
import socket
import json
import time
import argparse
import sys
import numpy as np
from typing import Dict, List, Optional, Tuple

class MultiDetectionServer:
    def __init__(self, 
                 camera_id: int = 0,
                 udp_host: str = 'localhost', 
                 udp_port: int = 8888,
                 confidence_threshold: float = 0.5):
        """
        Initialize MediaPipe multi-detection server
        
        Args:
            camera_id: Camera device ID (0 for default)
            udp_host: UDP destination host
            udp_port: UDP destination port 
            confidence_threshold: Minimum confidence for all detections
        """
        self.camera_id = camera_id
        self.udp_host = udp_host
        self.udp_port = udp_port
        self.confidence_threshold = confidence_threshold
        
        # Detection enable flags (can be controlled via config)
        self.enable_pose = True
        self.enable_hands = True  
        self.enable_face = True
        self.enable_segmentation = True
        
        # MediaPipe solutions (initialized later when needed)
        self.pose = None
        self.hands = None  
        self.face_mesh = None
        self.selfie_segmentation = None
        
        # UDP socket for communication
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Camera capture
        self.cap = None
        
        # Performance tracking
        self.frame_count = 0
        self.start_time = time.time()
        self.last_fps_print = time.time()
        self.fps_interval = 5.0
        
        # Detection counters
        self.detection_counts = {
            'pose': 0,
            'hands': 0, 
            'face': 0,
            'segmentation': 0
        }
        
        print(f"🎭 MediaPipe Multi-Detection Server initialized")
        print(f"   Camera: {camera_id}")
        print(f"   UDP: {udp_host}:{udp_port}")
        print(f"   Confidence: {confidence_threshold}")
        print(f"   Detections: Pose={self.enable_pose}, Hands={self.enable_hands}, Face={self.enable_face}, Seg={self.enable_segmentation}")
    
    def _init_mediapipe_solutions(self):
        """Initialize all MediaPipe detection solutions"""
        
        # Pose Detection (optimized for smooth tracking)
        if self.enable_pose:
            self.mp_pose = mp.solutions.pose
            self.pose = self.mp_pose.Pose(
                static_image_mode=False,
                model_complexity=0,  # 0=lite for speed
                smooth_landmarks=True,  # Smooth tracking
                enable_segmentation=False,
                min_detection_confidence=self.confidence_threshold,
                min_tracking_confidence=0.7
            )
            print("✅ Pose detection initialized")
        
        # Hand Detection
        if self.enable_hands:
            self.mp_hands = mp.solutions.hands
            self.hands = self.mp_hands.Hands(
                static_image_mode=False,
                max_num_hands=2,  # Detect up to 2 hands
                model_complexity=0,  # 0=lite for speed
                min_detection_confidence=self.confidence_threshold,
                min_tracking_confidence=0.7
            )
            print("✅ Hand detection initialized")
        
        # Face Mesh Detection
        if self.enable_face:
            self.mp_face_mesh = mp.solutions.face_mesh
            self.face_mesh = self.mp_face_mesh.FaceMesh(
                static_image_mode=False,
                max_num_faces=1,  # Single face detection
                refine_landmarks=True,  # Get detailed landmarks
                min_detection_confidence=self.confidence_threshold,
                min_tracking_confidence=0.7
            )
            print("✅ Face mesh detection initialized")
        
        # Selfie Segmentation
        if self.enable_segmentation:
            self.mp_selfie_segmentation = mp.solutions.selfie_segmentation
            self.selfie_segmentation = self.mp_selfie_segmentation.SelfieSegmentation(
                model_selection=0  # 0=general model (faster), 1=landscape model
            )
            print("✅ Selfie segmentation initialized")
    
    def initialize_camera(self) -> bool:
        """Initialize camera capture"""
        try:
            self.cap = cv2.VideoCapture(self.camera_id)
            if not self.cap.isOpened():
                print(f"❌ Error: Cannot open camera {self.camera_id}")
                return False
            
            # Test camera
            ret, test_frame = self.cap.read()
            if not ret:
                print(f"❌ Error: Cannot read from camera {self.camera_id}")
                return False
            
            height, width = test_frame.shape[:2]
            print(f"✅ Camera {self.camera_id} initialized: {width}x{height}")
            
            # Set optimal resolution
            try:
                self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
                self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
                self.cap.set(cv2.CAP_PROP_FPS, 30)
                self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
                
                actual_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                actual_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
                print(f"🎯 Camera set to {actual_width}x{actual_height}")
                
                self.frame_width = actual_width
                self.frame_height = actual_height
                
            except Exception as e:
                print(f"⚠️ Could not set camera properties: {e}")
                self.frame_width = int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
                self.frame_height = int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            
            return True
        except Exception as e:
            print(f"❌ Camera initialization error: {e}")
            return False
    
    def transform_coordinates(self, x: float, y: float, z: float = 0.0,
                            orig_w: int = 640, orig_h: int = 480,
                            crop_offset_x: int = 0, crop_offset_y: int = 0, 
                            crop_size: int = 480) -> Dict[str, float]:
        """
        Transform normalized coordinates to 640x640 openFrameworks space
        
        Args:
            x, y, z: Normalized coordinates from MediaPipe (0-1)
            orig_w, orig_h: Original frame dimensions
            crop_offset_x, crop_offset_y: Crop offsets
            crop_size: Size of square crop
            
        Returns:
            Dict with transformed x, y, z coordinates
        """
        # Convert from normalized crop coordinates to crop pixels
        crop_x = x * crop_size
        crop_y = y * crop_size
        
        # Transform back to original frame coordinates  
        orig_x = crop_x + crop_offset_x
        orig_y = crop_y + crop_offset_y
        
        # Transform to openFrameworks 640x640 space (stretched to square)
        of_x = (orig_x / orig_w) * 640
        of_y = (orig_y / orig_h) * 640
        
        return {
            "x": of_x,
            "y": of_y, 
            "z": z  # Depth is relative, no transformation needed
        }
    
    def process_pose_detection(self, rgb_frame_cropped: np.ndarray, 
                             orig_w: int, orig_h: int,
                             crop_offset_x: int, crop_offset_y: int, 
                             crop_size: int) -> Optional[Dict]:
        """Process pose detection and format data"""
        if not self.enable_pose:
            return None
            
        try:
            results = self.pose.process(rgb_frame_cropped)
            
            if not results.pose_landmarks:
                return None
            
            # Format pose data with coordinate transformation
            landmarks = []
            for idx, landmark in enumerate(results.pose_landmarks.landmark):
                if landmark.visibility > 0.1:
                    coords = self.transform_coordinates(
                        landmark.x, landmark.y, landmark.z,
                        orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                    )
                    
                    landmarks.append({
                        "id": idx,
                        "x": coords["x"],
                        "y": coords["y"], 
                        "z": coords["z"],
                        "confidence": landmark.visibility,
                        "visibility": landmark.visibility > 0.5,
                        "joint_name": self.get_pose_joint_name(idx)
                    })
            
            if landmarks:
                self.detection_counts['pose'] += 1
                return {
                    "detection_type": "pose",
                    "timestamp": time.time(),
                    "people": [{
                        "person_id": 0,
                        "confidence": sum(lm["confidence"] for lm in landmarks) / len(landmarks),
                        "landmarks": landmarks
                    }]
                }
                
        except Exception as e:
            print(f"❌ Pose detection error: {e}")
        
        return None
    
    def process_hand_detection(self, rgb_frame_cropped: np.ndarray,
                             orig_w: int, orig_h: int, 
                             crop_offset_x: int, crop_offset_y: int,
                             crop_size: int) -> Optional[Dict]:
        """Process hand detection and format data"""
        if not self.enable_hands:
            return None
            
        try:
            results = self.hands.process(rgb_frame_cropped)
            
            if not results.multi_hand_landmarks:
                return None
            
            hands_data = []
            for idx, (hand_landmarks, handedness) in enumerate(
                zip(results.multi_hand_landmarks, results.multi_handedness)
            ):
                landmarks = []
                for landmark_idx, landmark in enumerate(hand_landmarks.landmark):
                    coords = self.transform_coordinates(
                        landmark.x, landmark.y, landmark.z,
                        orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                    )
                    
                    # Ultra-compact format: [x, y, z] to reduce UDP message size
                    landmarks.append([coords["x"], coords["y"], coords["z"]])
                
                if landmarks:
                    hand_label = handedness.classification[0].label
                    hand_confidence = handedness.classification[0].score
                    
                    hands_data.append({
                        "hand_id": idx,
                        "handedness": hand_label,
                        "confidence": hand_confidence,
                        "landmarks": landmarks
                    })
            
            if hands_data:
                self.detection_counts['hands'] += 1
                return {
                    "detection_type": "hands",
                    "timestamp": time.time(),
                    "hands": hands_data
                }
                
        except Exception as e:
            print(f"❌ Hand detection error: {e}")
        
        return None
    
    def process_face_detection(self, rgb_frame_cropped: np.ndarray,
                             orig_w: int, orig_h: int,
                             crop_offset_x: int, crop_offset_y: int, 
                             crop_size: int) -> Optional[Dict]:
        """Process face mesh detection and format data"""
        if not self.enable_face:
            return None
            
        try:
            results = self.face_mesh.process(rgb_frame_cropped)
            
            if not results.multi_face_landmarks:
                return None
            
            faces_data = []
            for face_idx, face_landmarks in enumerate(results.multi_face_landmarks):
                # Only send key landmarks to avoid massive data (subset of 468)
                key_landmark_indices = self.get_key_face_landmarks()
                
                landmarks = []
                for landmark_idx in key_landmark_indices:
                    if landmark_idx < len(face_landmarks.landmark):
                        landmark = face_landmarks.landmark[landmark_idx]
                        coords = self.transform_coordinates(
                            landmark.x, landmark.y, landmark.z,
                            orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                        )
                        
                        # Ultra-compact format: [x, y, z] to reduce UDP message size
                        landmarks.append([coords["x"], coords["y"], coords["z"]])
                
                if landmarks:
                    faces_data.append({
                        "face_id": face_idx,
                        "confidence": 0.9,  # Assume high confidence if detected
                        "landmarks": landmarks
                    })
            
            if faces_data:
                self.detection_counts['face'] += 1
                return {
                    "detection_type": "face", 
                    "timestamp": time.time(),
                    "faces": faces_data
                }
                
        except Exception as e:
            print(f"❌ Face detection error: {e}")
        
        return None
    
    def process_segmentation(self, rgb_frame_cropped: np.ndarray) -> Optional[Dict]:
        """Process selfie segmentation and format data"""
        if not self.enable_segmentation:
            return None
            
        try:
            results = self.selfie_segmentation.process(rgb_frame_cropped)
            
            if results.segmentation_mask is None:
                return None
            
            # For UDP efficiency, downsample the mask significantly
            mask_height, mask_width = results.segmentation_mask.shape
            
            # Downsample mask to 32x32 for UDP transmission (much smaller)
            downsampled_mask = cv2.resize(results.segmentation_mask, (32, 32))
            mask_data = downsampled_mask.flatten().tolist()
            
            self.detection_counts['segmentation'] += 1
            return {
                "detection_type": "segmentation",
                "timestamp": time.time(),
                "mask": {
                    "width": 32,
                    "height": 32, 
                    "threshold": 0.5,
                    "data": mask_data  # Downsampled probability values
                }
            }
            
        except Exception as e:
            print(f"❌ Segmentation error: {e}")
        
        return None
    
    def get_pose_joint_name(self, landmark_id: int) -> str:
        """Get human-readable joint name from MediaPipe pose landmark ID"""
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
    
    def get_key_face_landmarks(self) -> List[int]:
        """Get minimal key face landmark indices for UDP efficiency"""
        # Return minimal set of landmarks for face outline, eyes, nose, mouth
        return [
            # Face oval (minimal outline) 
            10, 338, 297, 284, 389, 454, 323, 361, 397, 378, 152, 176, 150, 172, 132, 93, 162, 21, 54, 67,
            
            # Eyes (basic outline)
            33, 133, 159, 145,  # Left eye
            362, 263, 386, 374, # Right eye
            
            # Nose (central points)
            1, 2, 19, 94, 
            
            # Mouth (basic outline) 
            61, 291, 13, 14, 12, 15
        ]
    
    def send_udp_data(self, data: Dict) -> bool:
        """Send detection data via UDP"""
        try:
            json_data = json.dumps(data, separators=(',', ':'))
            message = json_data.encode('utf-8')
            self.udp_socket.sendto(message, (self.udp_host, self.udp_port))
            return True
        except Exception as e:
            print(f"❌ UDP transmission error: {e}")
            return False
    
    def run(self) -> None:
        """Main processing loop"""
        print(f"🔧 Initializing camera...")
        if not self.initialize_camera():
            print(f"❌ Camera initialization failed, exiting")
            return
        
        print(f"🧠 Initializing MediaPipe solutions...")
        self._init_mediapipe_solutions()
        
        print(f"🚀 Starting multi-detection server...")
        print(f"📡 Broadcasting to {self.udp_host}:{self.udp_port}")  
        print(f"Press 'q' to quit, 'i' for info, '1-4' to toggle detections")
        print(f"🎥 Starting detection loop...")
        
        try:
            while True:
                ret, frame = self.cap.read()
                if not ret:
                    print("❌ Failed to read from camera")
                    break
                
                # Convert BGR to RGB for MediaPipe
                rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                orig_h, orig_w = rgb_frame.shape[:2]
                
                # Crop to square for optimal detection
                if orig_h != orig_w:
                    size = min(orig_h, orig_w)
                    start_x = (orig_w - size) // 2
                    start_y = (orig_h - size) // 2
                    rgb_frame_cropped = rgb_frame[start_y:start_y+size, start_x:start_x+size]
                    crop_offset_x = start_x
                    crop_offset_y = start_y
                    crop_size = size
                else:
                    rgb_frame_cropped = rgb_frame
                    crop_offset_x = 0
                    crop_offset_y = 0
                    crop_size = orig_w
                
                # Process each enabled detection type
                detections = []
                
                # Pose detection
                pose_data = self.process_pose_detection(
                    rgb_frame_cropped, orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                )
                if pose_data:
                    detections.append(pose_data)
                
                # Hand detection  
                hand_data = self.process_hand_detection(
                    rgb_frame_cropped, orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                )
                if hand_data:
                    detections.append(hand_data)
                
                # Face detection
                face_data = self.process_face_detection(
                    rgb_frame_cropped, orig_w, orig_h, crop_offset_x, crop_offset_y, crop_size
                )
                if face_data:
                    detections.append(face_data)
                
                # Segmentation (uses full cropped frame size)
                seg_data = self.process_segmentation(rgb_frame_cropped)
                if seg_data:
                    detections.append(seg_data)
                
                # Send all detection data
                for detection in detections:
                    self.send_udp_data(detection)
                
                # Performance tracking
                self.frame_count += 1
                
                # Print FPS periodically  
                current_time = time.time()
                if current_time - self.last_fps_print >= self.fps_interval:
                    elapsed = current_time - self.start_time
                    fps = self.frame_count / elapsed if elapsed > 0 else 0
                    print(f"🚀 Performance: {fps:.1f} FPS | Detections: P={self.detection_counts['pose']} H={self.detection_counts['hands']} F={self.detection_counts['face']} S={self.detection_counts['segmentation']}")
                    self.last_fps_print = current_time
                
                # Handle keyboard input
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    break
                elif key == ord('i'):
                    self.print_info()
                elif key == ord('1'):
                    self.enable_pose = not self.enable_pose
                    print(f"Pose detection: {'ON' if self.enable_pose else 'OFF'}")
                elif key == ord('2'):
                    self.enable_hands = not self.enable_hands  
                    print(f"Hand detection: {'ON' if self.enable_hands else 'OFF'}")
                elif key == ord('3'):
                    self.enable_face = not self.enable_face
                    print(f"Face detection: {'ON' if self.enable_face else 'OFF'}")
                elif key == ord('4'):
                    self.enable_segmentation = not self.enable_segmentation
                    print(f"Segmentation: {'ON' if self.enable_segmentation else 'OFF'}")
        
        except KeyboardInterrupt:
            print("\n🛑 Shutting down multi-detection server...")
        
        finally:
            self.cleanup()
    
    def print_info(self) -> None:
        """Print performance and status information"""
        elapsed = time.time() - self.start_time
        fps = self.frame_count / elapsed if elapsed > 0 else 0
        
        print(f"\n📊 Multi-Detection Server Status:")
        print(f"   Runtime: {elapsed:.1f}s")
        print(f"   Frames processed: {self.frame_count}")
        print(f"   FPS: {fps:.1f}")
        print(f"   UDP: {self.udp_host}:{self.udp_port}")
        print(f"   Camera: {self.camera_id}")
        print(f"   Enabled: Pose={self.enable_pose}, Hands={self.enable_hands}, Face={self.enable_face}, Seg={self.enable_segmentation}")
        print(f"   Detections: Pose={self.detection_counts['pose']}, Hands={self.detection_counts['hands']}, Face={self.detection_counts['face']}, Seg={self.detection_counts['segmentation']}")
    
    def cleanup(self) -> None:
        """Clean up resources"""
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()
        self.udp_socket.close()
        print("✅ Cleanup completed")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='MediaPipe Multi-Detection UDP Server')
    parser.add_argument('--camera', type=int, default=0, help='Camera device ID (default: 0)')
    parser.add_argument('--host', type=str, default='localhost', help='UDP host (default: localhost)')
    parser.add_argument('--port', type=int, default=8888, help='UDP port (default: 8888)')  
    parser.add_argument('--confidence', type=float, default=0.5, help='Confidence threshold (default: 0.5)')
    
    # Detection enable/disable flags
    parser.add_argument('--enable-pose', action='store_true', default=True, help='Enable pose detection')
    parser.add_argument('--enable-hands', action='store_true', default=True, help='Enable hand detection')
    parser.add_argument('--enable-face', action='store_true', default=True, help='Enable face detection')
    parser.add_argument('--enable-segmentation', action='store_true', default=True, help='Enable segmentation')
    
    args = parser.parse_args()
    
    # Create and run multi-detection server
    server = MultiDetectionServer(
        camera_id=args.camera,
        udp_host=args.host,
        udp_port=args.port,
        confidence_threshold=args.confidence
    )
    
    # Set detection flags
    server.enable_pose = args.enable_pose
    server.enable_hands = args.enable_hands
    server.enable_face = args.enable_face
    server.enable_segmentation = args.enable_segmentation
    
    server.run()

if __name__ == "__main__":
    main()