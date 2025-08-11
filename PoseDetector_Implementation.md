# SonifyV1 Pose Detection Implementation - Complete .mm Files

This document provides the complete implementation for the Objective-C++ (.mm) files needed for the pose detection system.

## 1. PoseDetector.mm

```objc
//
//  PoseDetector.mm
//  SonifyV1
//
//  Created for SonifyV1 Pose Detection
//

#import "PoseDetector.h"
#import <Vision/Vision.h>

@implementation PoseKeypoint
@end

@implementation PersonPose
@end

@interface PoseDetector ()
@property (nonatomic, strong) VNDetectHumanBodyPoseRequest* poseRequest;
@property (nonatomic, assign) float confidenceThreshold;
@property (nonatomic, assign) int maxPeople;
@property (nonatomic, strong) NSArray<NSString*>* jointNames;
@end

@implementation PoseDetector

- (instancetype)init {
    self = [super init];
    if (self) {
        _poseRequest = [[VNDetectHumanBodyPoseRequest alloc] init];
        _confidenceThreshold = 0.5;
        _maxPeople = 8;
        
        // Initialize joint names array
        _jointNames = @[
            @"nose", @"leftEye", @"rightEye", @"leftEar", @"rightEar",
            @"leftShoulder", @"rightShoulder", @"leftElbow", @"rightElbow",
            @"leftWrist", @"rightWrist", @"leftHip", @"rightHip",
            @"leftKnee", @"rightKnee", @"leftAnkle", @"rightAnkle"
        ];
    }
    return self;
}

- (void)detectPosesInPixels:(unsigned char*)pixelData
                      width:(int)width
                     height:(int)height
                   channels:(int)channels
                 completion:(void(^)(NSArray<PersonPose*>* poses))completion {
    
    // Create a CGImage from the pixel data
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pixelData, width, height, 8, width * channels, colorSpace, kCGImageAlphaNoneSkipLast);
    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    
    // Create a Vision image request handler
    VNImageRequestHandler* handler = [[VNImageRequestHandler alloc] initWithCGImage:cgImage options:@{}];
    
    // Perform the pose detection asynchronously
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError* error = nil;
        [handler performRequests:@[self.poseRequest] error:&error];
        
        if (error) {
            NSLog(@"Pose detection error: %@", error);
            completion(@[]);
            CGImageRelease(cgImage);
            CGContextRelease(context);
            CGColorSpaceRelease(colorSpace);
            return;
        }
        
        // Process the results
        NSArray<PersonPose*>* poses = [self processResults:self.poseRequest.results width:width height:height];
        
        // Call the completion handler with the results
        completion(poses);
        
        // Clean up
        CGImageRelease(cgImage);
        CGContextRelease(context);
        CGColorSpaceRelease(colorSpace);
    });
}

- (NSArray<PersonPose*>*)processResults:(NSArray<VNHumanBodyPoseObservation*>*)observations width:(int)width height:(int)height {
    NSMutableArray<PersonPose*>* poses = [NSMutableArray array];
    
    // Limit to max people
    int count = MIN(observations.count, self.maxPeople);
    
    for (int i = 0; i < count; i++) {
        VNHumanBodyPoseObservation* observation = observations[i];
        
        // Create a new person pose
        PersonPose* pose = [[PersonPose alloc] init];
        pose.personID = i;  // Assign sequential IDs for now
        pose.timestamp = [[NSDate date] timeIntervalSince1970] * 1000;  // Milliseconds
        
        // Process all keypoints
        NSMutableArray<PoseKeypoint*>* keypoints = [NSMutableArray array];
        float totalConfidence = 0.0;
        int validKeypoints = 0;
        
        for (int j = 0; j < self.jointNames.count; j++) {
            NSString* jointName = self.jointNames[j];
            
            // Get the keypoint for this joint
            VNRecognizedPoint* point = [self getJointPoint:observation jointName:jointName];
            
            PoseKeypoint* keypoint = [[PoseKeypoint alloc] init];
            keypoint.jointName = jointName;
            
            if (point && point.confidence >= self.confidenceThreshold) {
                keypoint.x = point.location.x * width;
                keypoint.y = (1 - point.location.y) * height;  // Flip Y coordinate
                keypoint.confidence = point.confidence;
                keypoint.isVisible = YES;
                
                totalConfidence += point.confidence;
                validKeypoints++;
            } else {
                keypoint.x = 0;
                keypoint.y = 0;
                keypoint.confidence = 0;
                keypoint.isVisible = NO;
            }
            
            [keypoints addObject:keypoint];
        }
        
        pose.keypoints = keypoints;
        pose.overallConfidence = validKeypoints > 0 ? totalConfidence / validKeypoints : 0;
        pose.isValid = validKeypoints >= 5;  // At least 5 keypoints visible
        
        [poses addObject:pose];
    }
    
    return poses;
}

- (VNRecognizedPoint*)getJointPoint:(VNHumanBodyPoseObservation*)observation jointName:(NSString*)jointName {
    NSError* error = nil;
    
    // Map our joint names to Vision framework joint names
    NSString* visionJointName = [self mapToVisionJointName:jointName];
    
    VNRecognizedPoint* point = [observation recognizedPointForKey:visionJointName error:&error];
    
    if (error) {
        NSLog(@"Error getting joint %@: %@", jointName, error);
        return nil;
    }
    
    return point;
}

- (NSString*)mapToVisionJointName:(NSString*)jointName {
    // Map our joint names to Vision framework joint names
    NSDictionary* jointMap = @{
        @"nose": VNHumanBodyPoseObservationJointNameNose,
        @"leftEye": VNHumanBodyPoseObservationJointNameLeftEye,
        @"rightEye": VNHumanBodyPoseObservationJointNameRightEye,
        @"leftEar": VNHumanBodyPoseObservationJointNameLeftEar,
        @"rightEar": VNHumanBodyPoseObservationJointNameRightEar,
        @"leftShoulder": VNHumanBodyPoseObservationJointNameLeftShoulder,
        @"rightShoulder": VNHumanBodyPoseObservationJointNameRightShoulder,
        @"leftElbow": VNHumanBodyPoseObservationJointNameLeftElbow,
        @"rightElbow": VNHumanBodyPoseObservationJointNameRightElbow,
        @"leftWrist": VNHumanBodyPoseObservationJointNameLeftWrist,
        @"rightWrist": VNHumanBodyPoseObservationJointNameRightWrist,
        @"leftHip": VNHumanBodyPoseObservationJointNameLeftHip,
        @"rightHip": VNHumanBodyPoseObservationJointNameRightHip,
        @"leftKnee": VNHumanBodyPoseObservationJointNameLeftKnee,
        @"rightKnee": VNHumanBodyPoseObservationJointNameRightKnee,
        @"leftAnkle": VNHumanBodyPoseObservationJointNameLeftAnkle,
        @"rightAnkle": VNHumanBodyPoseObservationJointNameRightAnkle
    };
    
    return jointMap[jointName] ?: jointName;
}

- (void)setConfidenceThreshold:(float)threshold {
    _confidenceThreshold = threshold;
}

- (void)setMaxPeople:(int)maxPeople {
    _maxPeople = maxPeople;
}

- (NSArray<NSString*>*)getJointNames {
    return self.jointNames;
}

@end
```

## 2. PoseDetectorWrapper.mm

```objc
//
//  PoseDetectorWrapper.mm
//  SonifyV1
//
//  Created for SonifyV1 Pose Detection
//

#import "PoseDetectorWrapper.h"
#import "PoseDetector.h"

// Implementation class to bridge C++ and Objective-C
class PoseDetectorWrapper::Impl {
public:
    PoseDetector* detector;
    
    Impl() {
        detector = [[PoseDetector alloc] init];
    }
    
    ~Impl() {
        detector = nil;
    }
};

PoseDetectorWrapper::PoseDetectorWrapper() {
    impl = new Impl();
}

PoseDetectorWrapper::~PoseDetectorWrapper() {
    delete impl;
}

bool PoseDetectorWrapper::setup() {
    return (impl->detector != nil);
}

std::vector<PersonPose> PoseDetectorWrapper::detectPoses(ofPixels& pixels) {
    std::vector<PersonPose> result;
    
    // Create a semaphore to wait for the async result
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    // Call the async method
    [impl->detector detectPosesInPixels:pixels.getData()
                                  width:pixels.getWidth()
                                 height:pixels.getHeight()
                               channels:pixels.getNumChannels()
                             completion:^(NSArray<PersonPose*>* poses) {
        // Convert the result
        result = convertToCppPoses((__bridge void*)poses);
        
        // Signal that we're done
        dispatch_semaphore_signal(semaphore);
    }];
    
    // Wait for the result (with timeout)
    dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
    
    return result;
}

void PoseDetectorWrapper::detectPosesAsync(ofPixels& pixels, std::function<void(std::vector<PersonPose>)> callback) {
    // Call the Objective-C method
    [impl->detector detectPosesInPixels:pixels.getData()
                                  width:pixels.getWidth()
                                 height:pixels.getHeight()
                               channels:pixels.getNumChannels()
                             completion:^(NSArray<PersonPose*>* poses) {
        // Convert the result
        std::vector<PersonPose> cppPoses = convertToCppPoses((__bridge void*)poses);
        
        // Call the callback on the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            callback(cppPoses);
        });
    }];
}

void PoseDetectorWrapper::setConfidenceThreshold(float threshold) {
    [impl->detector setConfidenceThreshold:threshold];
}

void PoseDetectorWrapper::setMaxPeople(int maxPeople) {
    [impl->detector setMaxPeople:maxPeople];
}

std::vector<std::string> PoseDetectorWrapper::getJointNames() {
    NSArray<NSString*>* names = [impl->detector getJointNames];
    std::vector<std::string> result;
    
    for (NSString* name in names) {
        result.push_back([name UTF8String]);
    }
    
    return result;
}

std::vector<PersonPose> PoseDetectorWrapper::convertToCppPoses(void* nsPoses) {
    std::vector<PersonPose> result;
    NSArray<PersonPose*>* poses = (__bridge NSArray<PersonPose*>*)nsPoses;
    
    for (PersonPose* pose in poses) {
        PersonPose cppPose;
        cppPose.personID = pose.personID;
        cppPose.overallConfidence = pose.overallConfidence;
        cppPose.timestamp = pose.timestamp;
        cppPose.isValid = pose.isValid;
        
        for (PoseKeypoint* keypoint in pose.keypoints) {
            PoseKeypoint cppKeypoint;
            cppKeypoint.jointName = [keypoint.jointName UTF8String];
            cppKeypoint.position.x = keypoint.x;
            cppKeypoint.position.y = keypoint.y;
            cppKeypoint.confidence = keypoint.confidence;
            cppKeypoint.velocity.x = keypoint.velocityX;
            cppKeypoint.velocity.y = keypoint.velocityY;
            cppKeypoint.isVisible = keypoint.isVisible;
            
            cppPose.keypoints.push_back(cppKeypoint);
        }
        
        result.push_back(cppPose);
    }
    
    return result;
}
```

## 3. PoseSkeletonDefinition Implementation

Add this to PoseStructures.h or create a separate file:

```cpp
// Define the connections between joints for skeleton visualization
struct JointConnection {
    std::string joint1;
    std::string joint2;
    
    JointConnection(const std::string& j1, const std::string& j2) : joint1(j1), joint2(j2) {}
};

// Helper class to define the skeleton structure
class PoseSkeletonDefinition {
public:
    static std::vector<JointConnection> getConnections() {
        return {
            // Head connections
            {"nose", "leftEye"}, {"nose", "rightEye"},
            {"leftEye", "leftEar"}, {"rightEye", "rightEar"},
            
            // Torso connections
            {"leftShoulder", "rightShoulder"}, {"leftShoulder", "leftHip"},
            {"rightShoulder", "rightHip"}, {"leftHip", "rightHip"},
            
            // Arms
            {"leftShoulder", "leftElbow"}, {"leftElbow", "leftWrist"},
            {"rightShoulder", "rightElbow"}, {"rightElbow", "rightWrist"},
            
            // Legs
            {"leftHip", "leftKnee"}, {"leftKnee", "leftAnkle"},
            {"rightHip", "rightKnee"}, {"rightKnee", "rightAnkle"}
        };
    }
    
    static std::vector<std::string> getJointNames() {
        return {
            "nose", "leftEye", "rightEye", "leftEar", "rightEar",
            "leftShoulder", "rightShoulder", "leftElbow", "rightElbow",
            "leftWrist", "rightWrist", "leftHip", "rightHip",
            "leftKnee", "rightKnee", "leftAnkle", "rightAnkle"
        };
    }
};
```

## 4. Complete drawPoseDetection Method

```cpp
void ofApp::drawPoseDetection() {
    if (!poseDetectionEnabled || detectedPoses.empty()) return;
    
    // Draw skeletons
    if (showSkeletonOverlay) {
        for (const auto& pose : detectedPoses) {
            if (!pose.isValid) continue;
            
            // Draw connections between joints
            auto connections = PoseSkeletonDefinition::getConnections();
            for (const auto& connection : connections) {
                // Find the keypoints
                const PoseKeypoint* joint1 = nullptr;
                const PoseKeypoint* joint2 = nullptr;
                
                for (const auto& keypoint : pose.keypoints) {
                    if (keypoint.jointName == connection.joint1) joint1 = &keypoint;
                    if (keypoint.jointName == connection.joint2) joint2 = &keypoint;
                }
                
                // Draw the connection if both joints are visible
                if (joint1 && joint2 && joint1->isVisible && joint2->isVisible) {
                    // Color based on confidence
                    float avgConfidence = (joint1->confidence + joint2->confidence) / 2.0f;
                    ofColor color;
                    
                    if (avgConfidence > 0.8f) {
                        color = ofColor::green;
                    } else if (avgConfidence > 0.5f) {
                        color = ofColor::yellow;
                    } else {
                        color = ofColor::red;
                    }
                    
                    ofSetColor(color);
                    ofSetLineWidth(2.0f);
                    ofDrawLine(joint1->position, joint2->position);
                }
            }
            
            // Draw keypoints
            for (const auto& keypoint : pose.keypoints) {
                if (!keypoint.isVisible) continue;
                
                // Color based on confidence
                ofColor color;
                if (keypoint.confidence > 0.8f) {
                    color = ofColor::green;
                } else if (keypoint.confidence > 0.5f) {
                    color = ofColor::yellow;
                } else {
                    color = ofColor::red;
                }
                
                ofSetColor(color);
                ofDrawCircle(keypoint.position, 5);
                
                // Draw keypoint name if enabled
                if (showPoseLabels) {
                    ofSetColor(ofColor::white);
                    ofDrawBitmapString(keypoint.jointName, 
                                      keypoint.position.x + 5, 
                                      keypoint.position.y + 5);
                }
            }
            
            // Draw pose ID
            if (showPoseLabels) {
                ofSetColor(ofColor::white);
                // Find the top-most visible keypoint for label placement
                float minY = 1000000;
                ofPoint labelPos;
                
                for (const auto& keypoint : pose.keypoints) {
                    if (keypoint.isVisible && keypoint.position.y < minY) {
                        minY = keypoint.position.y;
                        labelPos = keypoint.position;
                    }
                }
                
                ofDrawBitmapString("Person " + ofToString(pose.personID) + 
                                  " (" + ofToString(pose.overallConfidence, 2) + ")", 
                                  labelPos.x, labelPos.y - 15);
            }
            
            // Draw keypoint trails if enabled
            if (showKeypointTrails) {
                // Implementation for keypoint trails would go here
                // This would require storing history of keypoint positions
            }
        }
    }
}
```

## 5. Implementation Notes

### Vision Framework Requirements
- macOS 10.15+ (Catalina or newer)
- Xcode 12.0+ for development
- Vision.framework must be linked in the project

### Performance Considerations
- Pose detection is computationally intensive
- Use async detection to avoid blocking the main thread
- Consider frame skipping for better performance (e.g., detect every 2-3 frames)
- Monitor CPU usage and adjust maxPeopleToDetect accordingly

### Memory Management
- The Objective-C++ wrapper handles memory management between C++ and Objective-C
- Use ARC (Automatic Reference Counting) for Objective-C objects
- Manually manage C++ objects with proper constructors/destructors

### Future 3D Integration
- The current implementation focuses on 2D pose detection
- Data structures include placeholders for 3D information (depth, position3D)
- When upgrading to OAK-D-Lite, extend these structures with actual 3D data

## 6. Troubleshooting Common Issues

### "Vision/Vision.h file not found"
- Make sure Vision.framework is properly linked in your project
- Add `#import <Vision/Vision.h>` at the top of PoseDetector.mm

### "No visible @interface for 'VNHumanBodyPoseObservation' declares the selector 'recognizedPointForKey:error:'"
- This usually means you're using an older version of macOS/Xcode
- Ensure you're on macOS 10.15+ and Xcode 12.0+

### Poor detection performance
- Reduce the frame resolution (e.g., scale down to 640x480)
- Increase frame skipping (process every 2-3 frames)
- Reduce maxPeopleToDetect to focus on fewer people

### Memory leaks
- Check for proper cleanup in the completion handlers
- Ensure CGImage, CGContext, and CGColorSpace are properly released
- Use Instruments to profile memory usage
