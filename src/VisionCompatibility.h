//
//  VisionCompatibility.h
//  Vision framework integration with SIMD compatibility workaround
//

#pragma once

// SIMD ISSUES: Reverting to stub mode due to persistent compilation errors
// Will implement Vision framework once SIMD compatibility is fully resolved
#define USE_VISION_FRAMEWORK_STUB 1

#if !USE_VISION_FRAMEWORK_STUB

// SIMD compatibility workaround - provide missing functions before including Vision
#include <simd/simd.h>

// Define missing simd_reduce_add functions for Vision framework compatibility
// These are based on the expected behavior from Apple's SIMD library
static inline _Float16 simd_reduce_add(simd_half2 __x) {
    return __x[0] + __x[1];
}

static inline _Float16 simd_reduce_add(simd_half3 __x) {
    return __x[0] + __x[1] + __x[2];
}

static inline _Float16 simd_reduce_add(simd_half4 __x) {
    return __x[0] + __x[1] + __x[2] + __x[3];
}

static inline _Float16 simd_reduce_add(simd_half8 __x) {
    return __x[0] + __x[1] + __x[2] + __x[3] + __x[4] + __x[5] + __x[6] + __x[7];
}

static inline _Float16 simd_reduce_add(simd_half16 __x) {
    _Float16 sum = 0;
    for (int i = 0; i < 16; i++) sum += __x[i];
    return sum;
}

static inline _Float16 simd_reduce_add(simd_half32 __x) {
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) sum += __x[i];
    return sum;
}

static inline float simd_reduce_add(simd_float2 __x) {
    return __x[0] + __x[1];
}

static inline float simd_reduce_add(simd_float3 __x) {
    return __x[0] + __x[1] + __x[2];
}

static inline float simd_reduce_add(simd_float4 __x) {
    return __x[0] + __x[1] + __x[2] + __x[3];
}

static inline float simd_reduce_add(simd_float8 __x) {
    return __x[0] + __x[1] + __x[2] + __x[3] + __x[4] + __x[5] + __x[6] + __x[7];
}

static inline float simd_reduce_add(simd_float16 __x) {
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) sum += __x[i];
    return sum;
}

static inline double simd_reduce_add(simd_double2 __x) {
    return __x[0] + __x[1];
}

static inline double simd_reduce_add(simd_double3 __x) {
    return __x[0] + __x[1] + __x[2];
}

static inline double simd_reduce_add(simd_double4 __x) {
    return __x[0] + __x[1] + __x[2] + __x[3];
}

static inline double simd_reduce_add(simd_double8 __x) {
    double sum = 0.0;
    for (int i = 0; i < 8; i++) sum += __x[i];
    return sum;
}

// Now include Vision framework with SIMD compatibility
#include <Vision/Vision.h>

#endif