//
//  VisionCompatibility.h
//  Vision framework integration with SIMD compatibility workaround
//

#pragma once

// SIMD ISSUES: Reverting to stub mode due to persistent compilation errors
// Will implement Vision framework once SIMD compatibility is fully resolved
#define USE_VISION_FRAMEWORK_STUB 1

#if !USE_VISION_FRAMEWORK_STUB

// Different approach: Define Vision framework availability without SIMD geometry functions
#define SIMD_COMPILER_HAS_REQUIRED_FEATURES 0
#include <Vision/Vision.h>

#endif