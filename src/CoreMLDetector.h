//
//  CoreMLDetector.h
//  SonifyV1
//
//  Created by Claude on 2025-08-06.
//

#import <Foundation/Foundation.h>

@interface CoreMLDetection : NSObject
@property (nonatomic, assign) float x;
@property (nonatomic, assign) float y;
@property (nonatomic, assign) float width;
@property (nonatomic, assign) float height;
@property (nonatomic, assign) float confidence;
@property (nonatomic, assign) int classId;
@property (nonatomic, strong) NSString* className;
@end

@interface CoreMLDetector : NSObject

- (BOOL)loadModelAtPath:(NSString*)modelPath;

- (void)detectObjectsInPixels:(unsigned char*)pixelData 
                        width:(int)width 
                       height:(int)height 
                     channels:(int)channels
                   completion:(void(^)(NSArray<CoreMLDetection*>* detections))completion;

@end