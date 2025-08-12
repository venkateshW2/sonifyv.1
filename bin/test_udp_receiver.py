#!/usr/bin/env python3
"""
Test UDP receiver to listen for pose data
"""

import socket
import json
import time

def test_udp_receiver(port=8080, timeout=10):
    """Test UDP receiver for pose data"""
    print(f"🔍 Testing UDP receiver on port {port}...")
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('localhost', port))
    sock.settimeout(1.0)  # 1 second timeout
    
    start_time = time.time()
    received_count = 0
    
    print(f"📡 Listening for pose data (timeout: {timeout}s)...")
    
    try:
        while time.time() - start_time < timeout:
            try:
                data, addr = sock.recvfrom(4096)  # Buffer size
                message = data.decode('utf-8')
                pose_data = json.loads(message)
                
                received_count += 1
                print(f"✅ Received pose data #{received_count}")
                print(f"   Timestamp: {pose_data.get('timestamp', 'N/A')}")
                print(f"   Frame size: {pose_data.get('frame_size', 'N/A')}")
                print(f"   Poses detected: {len(pose_data.get('poses', []))}")
                
                if pose_data.get('poses'):
                    for i, pose in enumerate(pose_data['poses']):
                        landmarks = pose.get('landmarks', [])
                        print(f"   Person {i}: {len(landmarks)} landmarks, confidence: {pose.get('confidence', 'N/A'):.3f}")
                
                print("")
                
            except socket.timeout:
                print(".", end="", flush=True)
                continue
            except json.JSONDecodeError as e:
                print(f"❌ JSON decode error: {e}")
            except Exception as e:
                print(f"❌ Receive error: {e}")
    
    except KeyboardInterrupt:
        print("\n🛑 Interrupted by user")
    
    finally:
        sock.close()
        print(f"\n📊 Summary: Received {received_count} pose data packets")
        return received_count > 0

if __name__ == "__main__":
    test_udp_receiver()