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
    __block std::vector<PersonPose> result;

    // Create a semaphore to wait for the async result
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    // Call the async method
    [impl->detector detectPosesInPixels:pixels.getData()
                                  width:pixels.getWidth()
                                 height:pixels.getHeight()
                               channels:pixels.getNumChannels()
                             completion:^(NSArray<VisionPersonPose*>* poses) {
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
                             completion:^(NSArray<VisionPersonPose*>* poses) {
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
    NSArray<VisionPersonPose*>* poses = (__bridge NSArray<VisionPersonPose*>*)nsPoses;

    for (VisionPersonPose* pose in poses) {
        PersonPose cppPose;
        cppPose.personID = pose.personID;
        cppPose.overallConfidence = pose.overallConfidence;
        cppPose.timestamp = pose.timestamp;
        cppPose.isValid = pose.isValid;

        for (VisionPoseKeypoint* keypoint in pose.keypoints) {
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
