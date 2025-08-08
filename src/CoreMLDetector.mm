//
//  CoreMLDetector.mm
//  SonifyV1
//
//  Created by Claude on 2025-08-06.
//

#import "CoreMLDetector.h"
#import <CoreML/CoreML.h>

@interface CoreMLDetector()
@property (nonatomic, strong) MLModel* coremlModel;
@property (nonatomic, strong) NSArray* classNames;
@end

@implementation CoreMLDetector

- (instancetype)init {
    self = [super init];
    if (self) {
        self.coremlModel = nil;
        // Complete COCO 80-class dataset names
        self.classNames = @[
            @"person", @"bicycle", @"car", @"motorcycle", @"airplane",            // 0-4
            @"bus", @"train", @"truck", @"boat", @"traffic light",               // 5-9
            @"fire hydrant", @"stop sign", @"parking meter", @"bench", @"bird",  // 10-14
            @"cat", @"dog", @"horse", @"sheep", @"cow",                          // 15-19
            @"elephant", @"bear", @"zebra", @"giraffe", @"backpack",             // 20-24
            @"umbrella", @"handbag", @"tie", @"suitcase", @"frisbee",            // 25-29
            @"skis", @"snowboard", @"sports ball", @"kite", @"baseball bat",     // 30-34
            @"baseball glove", @"skateboard", @"surfboard", @"tennis racket", @"bottle", // 35-39
            @"wine glass", @"cup", @"fork", @"knife", @"spoon",                  // 40-44
            @"bowl", @"banana", @"apple", @"sandwich", @"orange",                // 45-49
            @"broccoli", @"carrot", @"hot dog", @"pizza", @"donut",              // 50-54
            @"cake", @"chair", @"couch", @"potted plant", @"bed",                // 55-59
            @"dining table", @"toilet", @"tv", @"laptop", @"mouse",              // 60-64
            @"remote", @"keyboard", @"cell phone", @"microwave", @"oven",        // 65-69
            @"toaster", @"sink", @"refrigerator", @"book", @"clock",             // 70-74
            @"vase", @"scissors", @"teddy bear", @"hair drier", @"toothbrush"    // 75-79
        ];
    }
    return self;
}

- (BOOL)loadModelAtPath:(NSString*)modelPath {
    if (![[NSFileManager defaultManager] fileExistsAtPath:modelPath]) {
        NSLog(@"CoreML model not found at path: %@", modelPath);
        return NO;
    }
    
    NSURL* modelURL = [NSURL fileURLWithPath:modelPath];
    NSError* error;
    
    // Try to compile the model first if it's an mlpackage
    if ([modelPath.pathExtension isEqualToString:@"mlpackage"]) {
        NSLog(@"Compiling CoreML model package...");
        NSURL* compiledModelURL = [MLModel compileModelAtURL:modelURL error:&error];
        
        if (compiledModelURL && !error) {
            NSLog(@"Model compiled successfully");
            self.coremlModel = [MLModel modelWithContentsOfURL:compiledModelURL error:&error];
        } else {
            NSLog(@"Failed to compile CoreML model: %@", error.localizedDescription);
            return NO;
        }
    } else {
        // Direct loading for .mlmodel files
        self.coremlModel = [MLModel modelWithContentsOfURL:modelURL error:&error];
    }
    
    if (self.coremlModel && !error) {
        NSLog(@"CoreML YOLOv8 model loaded successfully");
        NSLog(@"Model description: %@", self.coremlModel.modelDescription);
        return YES;
    } else {
        NSLog(@"Failed to load CoreML model: %@", error.localizedDescription);
        return NO;
    }
}

- (void)detectObjectsInPixels:(unsigned char*)pixelData 
                        width:(int)width 
                       height:(int)height 
                     channels:(int)channels
                   completion:(void(^)(NSArray<CoreMLDetection*>* detections))completion {
    
    if (!self.coremlModel) {
        NSLog(@"CoreML model not loaded");
        completion(@[]);
        return;
    }
    
    @try {
        NSLog(@"Preprocessing image: %dx%d -> 640x640", width, height);
        
        // Model expects 640x640 - we need to resize and letterbox the input
        const int modelSize = 640;
        
        // Calculate scaling to maintain aspect ratio
        float scaleX = (float)modelSize / width;
        float scaleY = (float)modelSize / height;
        float scale = MIN(scaleX, scaleY); // Use smaller scale to maintain aspect ratio
        
        int scaledWidth = (int)(width * scale);
        int scaledHeight = (int)(height * scale);
        
        // Calculate letterbox offsets to center the image
        int offsetX = (modelSize - scaledWidth) / 2;
        int offsetY = (modelSize - scaledHeight) / 2;
        
        NSLog(@"Letterbox: scale=%.3f, scaled=%dx%d, offset=(%d,%d)", 
              scale, scaledWidth, scaledHeight, offsetX, offsetY);
        
        // Allocate 640x640 BGRA buffer (letterboxed)
        size_t modelBytesPerRow = modelSize * 4;
        unsigned char* bgraData = (unsigned char*)calloc(modelSize * modelSize, 4);
        if (!bgraData) {
            NSLog(@"Failed to allocate letterbox buffer");
            completion(@[]);
            return;
        }
        
        // Fill letterbox buffer with gray background (128,128,128,255)
        for (int i = 0; i < modelSize * modelSize * 4; i += 4) {
            bgraData[i + 0] = 128; // B
            bgraData[i + 1] = 128; // G
            bgraData[i + 2] = 128; // R
            bgraData[i + 3] = 255; // A
        }
        
        // Copy and scale original image into letterbox buffer
        for (int dstY = 0; dstY < scaledHeight; dstY++) {
            for (int dstX = 0; dstX < scaledWidth; dstX++) {
                // Map destination pixel back to source
                int srcX = (int)(dstX / scale);
                int srcY = (int)(dstY / scale);
                
                // Clamp to source bounds
                srcX = MIN(srcX, width - 1);
                srcY = MIN(srcY, height - 1);
                
                int srcIndex = (srcY * width + srcX) * channels;
                int dstIndex = ((dstY + offsetY) * modelSize + (dstX + offsetX)) * 4;
                
                if (channels == 3) {
                    // RGB to BGRA conversion
                    bgraData[dstIndex + 0] = pixelData[srcIndex + 2]; // B
                    bgraData[dstIndex + 1] = pixelData[srcIndex + 1]; // G  
                    bgraData[dstIndex + 2] = pixelData[srcIndex + 0]; // R
                    bgraData[dstIndex + 3] = 255;                     // A
                }
            }
        }
        
        // Create CVPixelBuffer for model input (640x640)
        CVPixelBufferRef pixelBuffer = NULL;
        CVReturn result = CVPixelBufferCreateWithBytes(NULL,
                                                      modelSize, modelSize,
                                                      kCVPixelFormatType_32BGRA,
                                                      bgraData,
                                                      modelBytesPerRow,
                                                      NULL, NULL, NULL,
                                                      &pixelBuffer);
        
        if (result != kCVReturnSuccess) {
            NSLog(@"Failed to create CVPixelBuffer: %d", result);
            free(bgraData);
            completion(@[]);
            return;
        }
        
        NSLog(@"Letterboxed CVPixelBuffer created successfully");
        
        // Create MLFeatureProvider input with thresholds
        MLFeatureValue* imageFeature = [MLFeatureValue featureValueWithPixelBuffer:pixelBuffer];
        MLFeatureValue* confThresholdValue = [MLFeatureValue featureValueWithDouble:0.15]; // Lower threshold for debugging
        MLFeatureValue* iouThreshold = [MLFeatureValue featureValueWithDouble:0.7];
        
        NSError* inputError;
        MLDictionaryFeatureProvider* input = [[MLDictionaryFeatureProvider alloc] 
                                            initWithDictionary:@{@"image": imageFeature,
                                                               @"confidenceThreshold": confThresholdValue,
                                                               @"iouThreshold": iouThreshold} 
                                            error:&inputError];
        
        if (inputError) {
            NSLog(@"Failed to create input features: %@", inputError.localizedDescription);
            CVPixelBufferRelease(pixelBuffer);
            free(bgraData);
            completion(@[]);
            return;
        }
        
        NSLog(@"Running CoreML prediction...");
        
        // Run prediction
        NSError* error;
        id<MLFeatureProvider> output = [self.coremlModel predictionFromFeatures:input error:&error];
        
        CVPixelBufferRelease(pixelBuffer);
        free(bgraData);
        
        if (error) {
            NSLog(@"CoreML prediction error: %@", error.localizedDescription);
            completion(@[]);
            return;
        }
        
        // Process YOLO output with letterbox parameters
        NSArray<CoreMLDetection*>* processedDetections = [self processYOLOOutput:output 
                                                                       inputWidth:width 
                                                                      inputHeight:height
                                                                     letterboxScale:scale
                                                                     letterboxOffsetX:offsetX
                                                                     letterboxOffsetY:offsetY];
        
        completion(processedDetections);
        
    } @catch (NSException* exception) {
        NSLog(@"CoreML detection exception: %@", exception.reason);
        completion(@[]);
    }
}

- (NSArray<CoreMLDetection*>*)processYOLOOutput:(id<MLFeatureProvider>)output 
                                     inputWidth:(int)inputWidth 
                                    inputHeight:(int)inputHeight
                                  letterboxScale:(float)letterboxScale
                                letterboxOffsetX:(int)letterboxOffsetX
                                letterboxOffsetY:(int)letterboxOffsetY {
    
    NSMutableArray<CoreMLDetection*>* detections = [NSMutableArray new];
    
    // This model outputs "coordinates" and "confidence" separately (with built-in NMS)
    MLFeatureValue* coordinatesFeature = [output featureValueForName:@"coordinates"];
    MLFeatureValue* confidenceFeature = [output featureValueForName:@"confidence"];
    
    if (!coordinatesFeature || !confidenceFeature || 
        coordinatesFeature.type != MLFeatureTypeMultiArray ||
        confidenceFeature.type != MLFeatureTypeMultiArray) {
        NSLog(@"Invalid YOLO output format");
        return detections;
    }
    
    MLMultiArray* coordinatesArray = coordinatesFeature.multiArrayValue;
    MLMultiArray* confidenceArray = confidenceFeature.multiArrayValue;
    
    NSLog(@"YOLO coordinates shape: %@", coordinatesArray.shape);
    NSLog(@"YOLO confidence shape: %@", confidenceArray.shape);
    
    // Get number of detections from the coordinates array (N x 4)
    int numDetections = [coordinatesArray.shape[0] intValue];
    
    NSLog(@"Processing %d detections", numDetections);
    
    // Use configurable confidence threshold instead of hardcoded vehicle filter
    
    for (int i = 0; i < numDetections; i++) {
        // Extract box coordinates in center format (center_x, center_y, width, height)
        float centerX = [coordinatesArray[i * 4 + 0] floatValue];
        float centerY = [coordinatesArray[i * 4 + 1] floatValue];
        float boxWidth = [coordinatesArray[i * 4 + 2] floatValue];
        float boxHeight = [coordinatesArray[i * 4 + 3] floatValue];
        
        NSLog(@"Raw coordinates for detection %d: center[%.3f, %.3f] size[%.3f, %.3f]", 
              i, centerX, centerY, boxWidth, boxHeight);
        
        // Find best class and its confidence for this detection
        float maxConfidence = 0.0f;
        int bestClassId = -1;
        
        for (int c = 0; c < 80; c++) { // 80 COCO classes
            float confidence = [confidenceArray[i * 80 + c] floatValue];
            if (confidence > maxConfidence) {
                maxConfidence = confidence;
                bestClassId = c;
            }
        }
        
        // Apply low threshold for now - class filtering happens in ofApp
        if (maxConfidence > 0.15f) {
            CoreMLDetection* detection = [[CoreMLDetection alloc] init];
            
            // Convert from normalized coordinates to 640x640 model space
            float modelCenterX = centerX * 640.0f;
            float modelCenterY = centerY * 640.0f;  
            float modelWidth = boxWidth * 640.0f;
            float modelHeight = boxHeight * 640.0f;
            
            // Convert from center format to corner format (still in model space)
            float modelX = modelCenterX - (modelWidth / 2.0f);
            float modelY = modelCenterY - (modelHeight / 2.0f);
            
            // Return coordinates in 640x640 model/display space (no reverse transformation)
            // This matches the fixed display window size for direct mapping
            detection.x = modelX;
            detection.y = modelY;
            detection.width = modelWidth;
            detection.height = modelHeight;
            
            // Clamp to 640x640 display boundaries
            detection.x = MAX(0, detection.x);
            detection.y = MAX(0, detection.y);
            detection.width = MIN(640 - detection.x, detection.width);
            detection.height = MIN(640 - detection.y, detection.height);
            
            detection.confidence = maxConfidence;
            detection.classId = bestClassId;
            detection.className = bestClassId < self.classNames.count ? 
                                 self.classNames[bestClassId] : @"unknown";
            
            [detections addObject:detection];
            NSLog(@"Vehicle detected: %@ (%.2f)", detection.className, detection.confidence);
            NSLog(@"  640x640 display coords: [%.0f, %.0f, %.0f, %.0f]", 
                  detection.x, detection.y, detection.width, detection.height);
        }
    }
    
    NSLog(@"CoreML detected %lu vehicles", (unsigned long)detections.count);
    return detections;
}


@end

@implementation CoreMLDetection
@end