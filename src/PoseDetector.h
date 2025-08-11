// PoseDetector.h
#import <Foundation/Foundation.h>

@interface VisionPoseKeypoint : NSObject
@property (nonatomic, assign) float x;
@property (nonatomic, assign) float y;
@property (nonatomic, assign) float confidence;
@property (nonatomic, strong) NSString* jointName;
@property (nonatomic, assign) BOOL isVisible;
@property (nonatomic, assign) float velocityX;
@property (nonatomic, assign) float velocityY;
@end

@interface VisionPersonPose : NSObject
@property (nonatomic, assign) int personID;
@property (nonatomic, strong) NSArray<VisionPoseKeypoint*>* keypoints;
@property (nonatomic, assign) float overallConfidence;
@property (nonatomic, assign) unsigned long timestamp;
@property (nonatomic, assign) BOOL isValid;
@end

@interface PoseDetector : NSObject
- (instancetype)init;
- (void)detectPosesInPixels:(unsigned char*)pixelData
                      width:(int)width
                     height:(int)height
                   channels:(int)channels
                 completion:(void(^)(NSArray<VisionPersonPose*>* poses))completion;
- (void)setConfidenceThreshold:(float)threshold;
- (void)setMaxPeople:(int)maxPeople;
- (NSArray<NSString*>*)getJointNames;
@end