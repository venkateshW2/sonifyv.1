//
//  VisionCompatibility.h
//  Stub implementation for pose detection - avoids Vision framework SIMD issues
//

#pragma once

// TEMPORARY SOLUTION: Disable Vision framework completely for now
// Use stub implementation that generates test poses
#define USE_VISION_FRAMEWORK_STUB 1

// When we solve SIMD issues, set this to 0 and uncomment Vision includes
#if !USE_VISION_FRAMEWORK_STUB
#include <Vision/Vision.h>
#endif