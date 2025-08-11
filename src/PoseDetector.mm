//
//  PoseDetector.mm
//  SonifyV1 - Apple Vision Framework Implementation for Real Pose Detection
//
//  Created for SonifyV1 Pose Detection
//

#import "PoseDetector.h"
#import "VisionCompatibility.h"

#if !USE_VISION_FRAMEWORK_STUB
#import <Vision/Vision.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreML/CoreML.h>
#endif

@implementation VisionPoseKeypoint
@end

@implementation VisionPersonPose
@end

@interface PoseDetector ()
@property (nonatomic, assign) float confidenceThreshold;
@property (nonatomic, assign) int maxPeople;
@property (nonatomic, strong) NSArray<NSString*>* jointNames;
@property (nonatomic, assign) int frameCounter;

#if !USE_VISION_FRAMEWORK_STUB
// Vision framework properties
@property (nonatomic, strong) dispatch_queue_t processingQueue;
#endif
@end

@implementation PoseDetector

- (instancetype)init {
    self = [super init];
    if (self) {
        _confidenceThreshold = 0.5;
        _maxPeople = 8;
        _frameCounter = 0;
        
        // Initialize joint names array (COCO pose format - 17 keypoints)
        _jointNames = @[
            @"nose", @"leftEye", @"rightEye", @"leftEar", @"rightEar",
            @"leftShoulder", @"rightShoulder", @"leftElbow", @"rightElbow",
            @"leftWrist", @"rightWrist", @"leftHip", @"rightHip",
            @"leftKnee", @"rightKnee", @"leftAnkle", @"rightAnkle"
        ];
        
#if !USE_VISION_FRAMEWORK_STUB
        // Initialize processing queue for Vision framework
        _processingQueue = dispatch_queue_create("com.sonifyv1.posedetection", DISPATCH_QUEUE_SERIAL);
        NSLog(@"PoseDetector: Initialized Apple Vision Framework for real pose detection");
#else
        NSLog(@"PoseDetector: Initialized working stub implementation for testing");
#endif
    }
    return self;
}

- (void)detectPosesInPixels:(unsigned char*)pixelData
                      width:(int)width
                     height:(int)height
                   channels:(int)channels
                 completion:(void(^)(NSArray<VisionPersonPose*>* poses))completion {
    
    _frameCounter++;
    
#if !USE_VISION_FRAMEWORK_STUB
    // Real Vision framework implementation
    [self performVisionDetectionWithPixelData:pixelData 
                                        width:width 
                                       height:height 
                                     channels:channels 
                                   completion:completion];
#else
    // Fallback stub implementation for testing/debugging
    [self performStubDetectionWithWidth:width 
                                 height:height 
                             completion:completion];
#endif
}

#if !USE_VISION_FRAMEWORK_STUB
// Real Apple Vision Framework Implementation
- (void)performVisionDetectionWithPixelData:(unsigned char*)pixelData
                                      width:(int)width
                                     height:(int)height
                                   channels:(int)channels
                                 completion:(void(^)(NSArray<VisionPersonPose*>* poses))completion {
    
    // Create CVPixelBuffer from raw pixel data
    CVPixelBufferRef pixelBuffer = NULL;
    CVReturn result = CVPixelBufferCreateWithBytes(kCFAllocatorDefault,
                                                  width, height,
                                                  kCVPixelFormatType_24RGB,
                                                  pixelData,
                                                  width * channels,
                                                  NULL, NULL, NULL,
                                                  &pixelBuffer);
    
    if (result != kCVReturnSuccess || !pixelBuffer) {
        NSLog(@"PoseDetector: Failed to create CVPixelBuffer");
        completion(@[]);
        return;
    }
    
    // Create Vision request
    VNDetectHumanBodyPoseRequest *poseRequest = [[VNDetectHumanBodyPoseRequest alloc] init];
    poseRequest.maximumObservationCount = _maxPeople;
    
    // Create request handler
    VNImageRequestHandler *requestHandler = [[VNImageRequestHandler alloc] initWithCVPixelBuffer:pixelBuffer options:@{}];
    
    // Perform detection asynchronously
    dispatch_async(_processingQueue, ^{
        NSError *error = nil;
        BOOL success = [requestHandler performRequests:@[poseRequest] error:&error];
        
        // Release pixel buffer
        CVPixelBufferRelease(pixelBuffer);
        
        if (!success || error) {
            NSLog(@"PoseDetector: Vision request failed: %@", error.localizedDescription);
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(@[]);
            });
            return;
        }
        
        // Process results
        NSArray<VNHumanBodyPoseObservation *> *observations = poseRequest.results;
        NSMutableArray<VisionPersonPose*>* poses = [NSMutableArray array];
        
        int personId = 1;
        for (VNHumanBodyPoseObservation *observation in observations) {
            
            // Skip low-confidence detections
            if (observation.confidence < self->_confidenceThreshold) {
                continue;
            }
            
            VisionPersonPose* pose = [[VisionPersonPose alloc] init];
            pose.personID = personId++;
            pose.timestamp = [[NSDate date] timeIntervalSince1970] * 1000;
            pose.overallConfidence = observation.confidence;
            pose.isValid = YES;
            
            NSMutableArray<VisionPoseKeypoint*>* keypoints = [NSMutableArray array];
            
            // Extract all recognized points
            NSError *keypointError = nil;
            NSDictionary<VNHumanBodyPoseObservationJointName, VNRecognizedPoint *> *recognizedPoints = 
                [observation recognizedPointsForGroupKey:VNRecognizedPointGroupKeyAll error:&keypointError];
            
            if (keypointError) {
                NSLog(@"PoseDetector: Error extracting keypoints: %@", keypointError.localizedDescription);
                continue;
            }
            
            // Map Vision joint names to our joint names
            NSArray<VNHumanBodyPoseObservationJointName>* visionJointNames = @[
                VNHumanBodyPoseObservationJointNameNose,
                VNHumanBodyPoseObservationJointNameLeftEye,
                VNHumanBodyPoseObservationJointNameRightEye,
                VNHumanBodyPoseObservationJointNameLeftEar,
                VNHumanBodyPoseObservationJointNameRightEar,
                VNHumanBodyPoseObservationJointNameLeftShoulder,
                VNHumanBodyPoseObservationJointNameRightShoulder,
                VNHumanBodyPoseObservationJointNameLeftElbow,
                VNHumanBodyPoseObservationJointNameRightElbow,
                VNHumanBodyPoseObservationJointNameLeftWrist,
                VNHumanBodyPoseObservationJointNameRightWrist,
                VNHumanBodyPoseObservationJointNameLeftHip,
                VNHumanBodyPoseObservationJointNameRightHip,
                VNHumanBodyPoseObservationJointNameLeftKnee,
                VNHumanBodyPoseObservationJointNameRightKnee,
                VNHumanBodyPoseObservationJointNameLeftAnkle,
                VNHumanBodyPoseObservationJointNameRightAnkle
            ];
            
            for (int i = 0; i < self->_jointNames.count && i < visionJointNames.count; i++) {
                NSString* jointName = self->_jointNames[i];
                VNHumanBodyPoseObservationJointName visionJointName = visionJointNames[i];
                
                VisionPoseKeypoint* keypoint = [[VisionPoseKeypoint alloc] init];
                keypoint.jointName = jointName;
                
                VNRecognizedPoint* point = recognizedPoints[visionJointName];
                if (point) {
                    keypoint.confidence = point.confidence;
                    keypoint.isVisible = keypoint.confidence > self->_confidenceThreshold;
                    
                    if (keypoint.isVisible) {
                        // Convert from Vision normalized coordinates (0-1) to pixel coordinates
                        // Vision uses bottom-left origin, we need top-left
                        CGPoint location = point.location;
                        keypoint.x = location.x * width;
                        keypoint.y = (1.0 - location.y) * height; // Flip Y coordinate
                    } else {
                        keypoint.x = 0;
                        keypoint.y = 0;
                    }
                } else {
                    keypoint.confidence = 0.0;
                    keypoint.isVisible = NO;
                    keypoint.x = 0;
                    keypoint.y = 0;
                }
                
                keypoint.velocityX = 0; // TODO: Calculate velocity from previous frames
                keypoint.velocityY = 0;
                
                [keypoints addObject:keypoint];
            }
            
            pose.keypoints = keypoints;
            [poses addObject:pose];
        }
        
        if (poses.count > 0) {
            NSLog(@"PoseDetector: Detected %lu real poses using Vision framework", (unsigned long)poses.count);
        }
        
        // Return results on main queue
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(poses);
        });
    });
}
#endif

// Stub implementation (kept for fallback/testing)
- (void)performStubDetectionWithWidth:(int)width
                               height:(int)height
                           completion:(void(^)(NSArray<VisionPersonPose*>* poses))completion {
    
    // Generate test poses every 30 frames (once per second at 30fps)
    NSMutableArray<VisionPersonPose*>* poses = [NSMutableArray array];
    
    if (_frameCounter % 30 == 0) {
        // Generate 1-2 test poses
        int numPoses = 1 + (arc4random() % 2); // 1 or 2 poses
        numPoses = MIN(numPoses, _maxPeople);
        
        for (int i = 0; i < numPoses; i++) {
            VisionPersonPose* pose = [[VisionPersonPose alloc] init];
            pose.personID = i + 1;
            pose.timestamp = [[NSDate date] timeIntervalSince1970] * 1000;
            pose.overallConfidence = 0.8 + (arc4random() % 20) / 100.0; // 0.8-0.99
            pose.isValid = YES;
            
            NSMutableArray<VisionPoseKeypoint*>* keypoints = [NSMutableArray array];
            
            // Generate realistic human pose keypoints
            float centerX = width * (0.3 + i * 0.4); // Spread poses across frame
            float centerY = height * 0.5;
            
            for (int j = 0; j < _jointNames.count; j++) {
                NSString* jointName = _jointNames[j];
                VisionPoseKeypoint* keypoint = [[VisionPoseKeypoint alloc] init];
                keypoint.jointName = jointName;
                keypoint.confidence = 0.7 + (arc4random() % 30) / 100.0; // 0.7-0.99
                keypoint.isVisible = keypoint.confidence > _confidenceThreshold;
                
                if (keypoint.isVisible) {
                    // Generate realistic human pose positions
                    float x = centerX;
                    float y = centerY;
                    
                    // Adjust positions based on body part
                    if ([jointName containsString:@"Eye"] || [jointName isEqualToString:@"nose"]) {
                        y = centerY - 100; // Head area
                        x += (arc4random() % 20) - 10; // Small variation
                    } else if ([jointName containsString:@"Ear"]) {
                        y = centerY - 95;
                        x += [jointName containsString:@"left"] ? -15 : 15;
                    } else if ([jointName containsString:@"Shoulder"]) {
                        y = centerY - 50;
                        x += [jointName containsString:@"left"] ? -40 : 40;
                    } else if ([jointName containsString:@"Elbow"]) {
                        y = centerY;
                        x += [jointName containsString:@"left"] ? -60 : 60;
                    } else if ([jointName containsString:@"Wrist"]) {
                        y = centerY + 20;
                        x += [jointName containsString:@"left"] ? -80 : 80;
                    } else if ([jointName containsString:@"Hip"]) {
                        y = centerY + 50;
                        x += [jointName containsString:@"left"] ? -20 : 20;
                    } else if ([jointName containsString:@"Knee"]) {
                        y = centerY + 100;
                        x += [jointName containsString:@"left"] ? -25 : 25;
                    } else if ([jointName containsString:@"Ankle"]) {
                        y = centerY + 150;
                        x += [jointName containsString:@"left"] ? -30 : 30;
                    }
                    
                    // Add some animation over time
                    float timeOffset = _frameCounter * 0.1;
                    x += sin(timeOffset + j) * 5; // Slight movement
                    y += cos(timeOffset + j) * 3;
                    
                    // Ensure coordinates are within frame bounds
                    keypoint.x = MAX(0, MIN(width, x));
                    keypoint.y = MAX(0, MIN(height, y));
                } else {
                    keypoint.x = 0;
                    keypoint.y = 0;
                }
                
                keypoint.velocityX = 0;
                keypoint.velocityY = 0;
                
                [keypoints addObject:keypoint];
            }
            
            pose.keypoints = keypoints;
            [poses addObject:pose];
        }
        
        if (poses.count > 0) {
            NSLog(@"PoseDetector: Generated %lu test poses (frame %d) - STUB MODE", (unsigned long)poses.count, _frameCounter);
        }
    }
    
    // Call completion handler asynchronously to simulate real detection
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(poses);
        });
    });
}

- (void)setConfidenceThreshold:(float)threshold {
    _confidenceThreshold = threshold;
#if !USE_VISION_FRAMEWORK_STUB
    NSLog(@"PoseDetector: Set confidence threshold to %.2f (Vision framework)", threshold);
#else
    NSLog(@"PoseDetector: Set confidence threshold to %.2f (stub mode)", threshold);
#endif
}

- (void)setMaxPeople:(int)maxPeople {
    _maxPeople = maxPeople;
#if !USE_VISION_FRAMEWORK_STUB
    NSLog(@"PoseDetector: Set max people to %d (Vision framework)", maxPeople);
#else
    NSLog(@"PoseDetector: Set max people to %d (stub mode)", maxPeople);
#endif
}

- (NSArray<NSString*>*)getJointNames {
    return self.jointNames;
}

@end