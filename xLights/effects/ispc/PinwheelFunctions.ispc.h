//
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#pragma once
#include <stdint.h>

#if !defined(__cplusplus)
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdbool.h>
#else
typedef int bool;
#endif
#endif



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus
///////////////////////////////////////////////////////////////////////////
// Vector types with external visibility from ispc code
///////////////////////////////////////////////////////////////////////////

#ifndef __ISPC_VECTOR_uint8_t4__
#define __ISPC_VECTOR_uint8_t4__
#ifdef _MSC_VER
__declspec( align(4) ) struct uint8_t4 { uint8_t v[4]; };
#else
struct uint8_t4 { uint8_t v[4]; } __attribute__ ((aligned(4)));
#endif
#endif

#ifndef __ISPC_VECTOR_float3__
#define __ISPC_VECTOR_float3__
#ifdef _MSC_VER
__declspec( align(16) ) struct float3 { float v[3]; };
#else
struct float3 { float v[3]; } __attribute__ ((aligned(16)));
#endif
#endif



/* Portable alignment macro that works across different compilers and standards */
#if defined(__cplusplus) && __cplusplus >= 201103L
/* C++11 or newer - use alignas keyword */
#define __ISPC_ALIGN__(x) alignas(x)
#elif defined(__GNUC__) || defined(__clang__)
/* GCC or Clang - use __attribute__ */
#define __ISPC_ALIGN__(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
/* Microsoft Visual C++ - use __declspec */
#define __ISPC_ALIGN__(x) __declspec(align(x))
#else
/* Unknown compiler/standard - alignment not supported */
#define __ISPC_ALIGN__(x)
#warning "Alignment not supported on this compiler"
#endif
#ifndef __ISPC_ALIGNED_STRUCT__
#if defined(_MSC_VER)
// Visual Studio
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#else
// Clang, GCC, ICC
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#endif
#endif

#ifndef __ISPC_STRUCT_PinwheelData__
#define __ISPC_STRUCT_PinwheelData__
struct PinwheelData {
    uint32_t width;
    uint32_t height;
    int32_t pinwheel_arms;
    int32_t xc_adj;
    int32_t yc_adj;
    int32_t degrees_per_arm;
    int32_t pinwheel_twist;
    int32_t max_radius;
    int32_t poffset;
    int32_t pw3dType;
    int32_t pinwheel_rotation;
    float tmax;
    float pos;
    int32_t allowAlpha;
    uint32_t numColors;
    uint8_t4   * colorsAsColor;
    float3   * colorsAsHSV;
    uint8_t * colorIsSpacial;
    uint32_t * colorarray;
    void * bufferData;
};
#endif

#ifndef __ISPC_STRUCT_v16_varying_SpacialData__
#define __ISPC_STRUCT_v16_varying_SpacialData__
__ISPC_ALIGNED_STRUCT__(64) v16_varying_SpacialData {
    __ISPC_ALIGN__(64)     uint32_t colorIdx[16];
    __ISPC_ALIGN__(64)     float x[16];
    __ISPC_ALIGN__(64)     float y[16];
    __ISPC_ALIGN__(64)     float r[16];
    __ISPC_ALIGN__(64)     uint32_t result[16];
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
#if defined(__cplusplus)
    extern void PinwheelEffectStyle0(const struct PinwheelData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PinwheelEffectStyle0(const struct PinwheelData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PinwheelEffectStyle0 function declaraion
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus
