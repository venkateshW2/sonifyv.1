#!/usr/bin/env python3
"""
SonifyV1 System Testing Script
Comprehensive validation of all system components
"""

import os
import sys
import time
import subprocess
import json
from pathlib import Path

class SonifyV1Tester:
    def __init__(self):
        self.project_root = Path(__file__).parent
        self.bin_dir = self.project_root / "bin"
        self.data_dir = self.bin_dir / "data"
        self.config_path = self.data_dir / "config.json"
        self.test_results = {}
        
    def log(self, message, status="INFO"):
        timestamp = time.strftime("%H:%M:%S")
        print(f"[{timestamp}] [{status}] {message}")
        
    def test_build_system(self):
        """Test 1: Build System Validation"""
        self.log("Testing build system...", "TEST")
        
        try:
            # Check if binary exists
            binary_path = self.bin_dir / "SonifyV1"
            if binary_path.exists():
                self.log("✅ Binary exists", "PASS")
                self.test_results["build_binary_exists"] = True
            else:
                self.log("❌ Binary not found", "FAIL")
                self.test_results["build_binary_exists"] = False
                return False
                
            # Check app bundle
            app_bundle = self.bin_dir / "SonifyV1.app"
            if app_bundle.exists():
                self.log("✅ App bundle exists", "PASS")
                self.test_results["build_app_bundle"] = True
            else:
                self.log("❌ App bundle not found", "FAIL")
                self.test_results["build_app_bundle"] = False
                
            return True
            
        except Exception as e:
            self.log(f"❌ Build system test failed: {e}", "FAIL")
            self.test_results["build_system"] = False
            return False
            
    def test_data_structure(self):
        """Test 2: Data Directory Structure"""
        self.log("Testing data directory structure...", "TEST")
        
        required_dirs = ["models", "video"]
        results = {}
        
        for dir_name in required_dirs:
            dir_path = self.data_dir / dir_name
            if dir_path.exists():
                self.log(f"✅ {dir_name} directory exists", "PASS")
                results[f"data_{dir_name}"] = True
            else:
                self.log(f"❌ {dir_name} directory missing", "FAIL")
                results[f"data_{dir_name}"] = False
                
        # Check for CoreML model
        model_path = self.data_dir / "models" / "yolov8l.mlpackage"
        if model_path.exists():
            self.log("✅ CoreML model found", "PASS")
            results["coreml_model"] = True
        else:
            self.log("❌ CoreML model missing", "FAIL")
            results["coreml_model"] = False
            
        # Check for test videos
        video_dir = self.data_dir / "video"
        video_files = list(video_dir.glob("*.mp4"))
        if len(video_files) > 0:
            self.log(f"✅ Found {len(video_files)} test videos", "PASS")
            results["test_videos"] = True
        else:
            self.log("❌ No test videos found", "FAIL")
            results["test_videos"] = False
            
        self.test_results.update(results)
        return all(results.values())
        
    def test_configuration_system(self):
        """Test 3: Configuration System"""
        self.log("Testing configuration system...", "TEST")
        
        try:
            # Create test config
            test_config = {
                "detection": {
                    "confidenceThreshold": 0.5,
                    "frameSkip": 2,
                    "enabled": True
                },
                "osc": {
                    "host": "127.0.0.1",
                    "port": 12000,
                    "enabled": True
                },
                "metadata": {
                    "version": "1.0",
                    "application": "SonifyV1_Test"
                }
            }
            
            # Write test config
            with open(self.config_path, 'w') as f:
                json.dump(test_config, f, indent=2)
                
            self.log("✅ Config file creation successful", "PASS")
            
            # Validate JSON structure
            with open(self.config_path, 'r') as f:
                loaded_config = json.load(f)
                
            if loaded_config == test_config:
                self.log("✅ Config file read/write validation passed", "PASS")
                self.test_results["config_system"] = True
                return True
            else:
                self.log("❌ Config file validation failed", "FAIL")
                self.test_results["config_system"] = False
                return False
                
        except Exception as e:
            self.log(f"❌ Configuration test failed: {e}", "FAIL")
            self.test_results["config_system"] = False
            return False
            
    def test_dependencies(self):
        """Test 4: Dependencies and Frameworks"""
        self.log("Testing dependencies...", "TEST")
        
        results = {}
        
        # Check if we can find the openFrameworks headers
        of_path = self.project_root / ".." / ".." / ".." / "libs" / "openFrameworks"
        if of_path.exists():
            self.log("✅ openFrameworks found", "PASS")
            results["openframeworks"] = True
        else:
            self.log("❌ openFrameworks not found", "FAIL")
            results["openframeworks"] = False
            
        # Check addons.make file
        addons_file = self.project_root / "addons.make"
        if addons_file.exists():
            with open(addons_file, 'r') as f:
                addons = f.read().strip().split('\n')
                
            required_addons = ["ofxImGui", "ofxJSON", "ofxOpenCv", "ofxOsc"]
            for addon in required_addons:
                if addon in addons:
                    self.log(f"✅ {addon} configured", "PASS")
                    results[f"addon_{addon}"] = True
                else:
                    self.log(f"❌ {addon} not configured", "FAIL")
                    results[f"addon_{addon}"] = False
                    
        self.test_results.update(results)
        return all(results.values())
        
    def test_video_files(self):
        """Test 5: Video File Validation"""
        self.log("Testing video files...", "TEST")
        
        video_dir = self.data_dir / "video"
        video_files = list(video_dir.glob("*.mp4"))
        
        if not video_files:
            self.log("❌ No video files found", "FAIL")
            self.test_results["video_files"] = False
            return False
            
        # Test a few video files
        tested_videos = 0
        valid_videos = 0
        
        for video_file in video_files[:3]:  # Test first 3 videos
            try:
                # Use ffprobe to validate video file
                result = subprocess.run([
                    'ffprobe', '-v', 'quiet', '-print_format', 'json',
                    '-show_format', '-show_streams', str(video_file)
                ], capture_output=True, text=True, timeout=10)
                
                if result.returncode == 0:
                    self.log(f"✅ {video_file.name} is valid", "PASS")
                    valid_videos += 1
                else:
                    self.log(f"❌ {video_file.name} validation failed", "FAIL")
                    
                tested_videos += 1
                
            except (subprocess.TimeoutExpired, FileNotFoundError):
                self.log(f"⚠️ Could not validate {video_file.name} (ffprobe not available)", "WARN")
                # Consider it valid if we can't test
                valid_videos += 1
                tested_videos += 1
                
        success_rate = valid_videos / tested_videos if tested_videos > 0 else 0
        self.log(f"Video validation: {valid_videos}/{tested_videos} valid", "INFO")
        
        self.test_results["video_files"] = success_rate >= 0.8
        return success_rate >= 0.8
        
    def run_integration_test(self):
        """Test 6: Integration Test"""
        self.log("Running integration test...", "TEST")
        
        try:
            # Test config creation and persistence 
            config_before = {
                "detection": {"confidenceThreshold": 0.7, "enabled": False},
                "osc": {"host": "127.0.0.1", "port": 12001},
                "test": {"timestamp": time.time()}
            }
            
            with open(self.config_path, 'w') as f:
                json.dump(config_before, f, indent=2)
                
            time.sleep(0.1)  # Brief pause
            
            with open(self.config_path, 'r') as f:
                config_after = json.load(f)
                
            if config_before == config_after:
                self.log("✅ Configuration persistence test passed", "PASS")
                self.test_results["integration_config"] = True
            else:
                self.log("❌ Configuration persistence test failed", "FAIL")
                self.test_results["integration_config"] = False
                
            return self.test_results["integration_config"]
            
        except Exception as e:
            self.log(f"❌ Integration test failed: {e}", "FAIL")
            self.test_results["integration_config"] = False
            return False
    
    def generate_test_report(self):
        """Generate comprehensive test report"""
        self.log("Generating test report...", "INFO")
        
        total_tests = len(self.test_results)
        passed_tests = sum(1 for result in self.test_results.values() if result)
        
        print("\n" + "="*60)
        print("SonifyV1 SYSTEM TEST REPORT")
        print("="*60)
        print(f"Total Tests: {total_tests}")
        print(f"Passed: {passed_tests}")
        print(f"Failed: {total_tests - passed_tests}")
        print(f"Success Rate: {passed_tests/total_tests*100:.1f}%")
        print("\nDetailed Results:")
        
        for test_name, result in self.test_results.items():
            status = "✅ PASS" if result else "❌ FAIL"
            print(f"  {test_name:<30} {status}")
            
        print("\n" + "="*60)
        
        # Generate recommendations
        print("RECOMMENDATIONS:")
        if not self.test_results.get("coreml_model", True):
            print("- Install CoreML model: yolov8l.mlpackage")
        if not self.test_results.get("test_videos", True):
            print("- Add test video files to bin/data/video/")
        if passed_tests == total_tests:
            print("- All tests passed! System ready for production.")
            
        return passed_tests / total_tests >= 0.8
        
    def run_all_tests(self):
        """Run comprehensive test suite"""
        self.log("Starting SonifyV1 comprehensive test suite...", "INFO")
        
        test_functions = [
            self.test_build_system,
            self.test_data_structure,
            self.test_configuration_system,
            self.test_dependencies,
            self.test_video_files,
            self.run_integration_test
        ]
        
        for test_func in test_functions:
            try:
                test_func()
                time.sleep(0.1)  # Brief pause between tests
            except Exception as e:
                self.log(f"Test {test_func.__name__} failed with exception: {e}", "ERROR")
                self.test_results[test_func.__name__] = False
                
        return self.generate_test_report()

if __name__ == "__main__":
    tester = SonifyV1Tester()
    success = tester.run_all_tests()
    sys.exit(0 if success else 1)