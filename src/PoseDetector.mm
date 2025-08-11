//
//  PoseDetector.mm
//  SonifyV1 - Functional stub implementation that generates test poses
//
//  Created for SonifyV1 Pose Detection
//

#import "PoseDetector.h"
#import "VisionCompatibility.h"

@implementation VisionPoseKeypoint
@end

@implementation VisionPersonPose
@end

@interface PoseDetector ()
@property (nonatomic, assign) float confidenceThreshold;
@property (nonatomic, assign) int maxPeople;
@property (nonatomic, strong) NSArray<NSString*>* jointNames;
@property (nonatomic, assign) int frameCounter;
@end

@implementation PoseDetector

- (instancetype)init {
    self = [super init];
    if (self) {
        _confidenceThreshold = 0.5;
        _maxPeople = 8;
        _frameCounter = 0;
        
        // Initialize joint names array
        _jointNames = @[
            @"nose", @"leftEye", @"rightEye", @"leftEar", @"rightEar",
            @"leftShoulder", @"rightShoulder", @"leftElbow", @"rightElbow",
            @"leftWrist", @"rightWrist", @"leftHip", @"rightHip",
            @"leftKnee", @"rightKnee", @"leftAnkle", @"rightAnkle"
        ];
        
        NSLog(@"PoseDetector: Initialized working stub implementation for testing");
    }
    return self;
}

- (void)detectPosesInPixels:(unsigned char*)pixelData
                      width:(int)width
                     height:(int)height
                   channels:(int)channels
                 completion:(void(^)(NSArray<VisionPersonPose*>* poses))completion {
    
    _frameCounter++;
    
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
            NSLog(@"PoseDetector: Generated %lu test poses (frame %d)", (unsigned long)poses.count, _frameCounter);
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
    NSLog(@"PoseDetector: Set confidence threshold to %.2f (working stub)", threshold);
}

- (void)setMaxPeople:(int)maxPeople {
    _maxPeople = maxPeople;
    NSLog(@"PoseDetector: Set max people to %d (working stub)", maxPeople);
}

- (NSArray<NSString*>*)getJointNames {
    return self.jointNames;
}

@end