//
// (Header written to match the struct and export in CirclesFunctions.ispc.)
// Keep in sync with CirclesFunctions.ispc manually.
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



/* Portable alignment macro that works across different compilers and standards */
#if defined(__cplusplus) && __cplusplus >= 201103L
#define __ISPC_ALIGN__(x) alignas(x)
#elif defined(__GNUC__) || defined(__clang__)
#define __ISPC_ALIGN__(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define __ISPC_ALIGN__(x) __declspec(align(x))
#else
#define __ISPC_ALIGN__(x)
#endif
#ifndef __ISPC_ALIGNED_STRUCT__
#if defined(__clang__) || !defined(_MSC_VER) || _MSC_VER > 1943
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#define MAX_ISPC_CIRCLES_BALLS  20
#define MAX_ISPC_CIRCLES_COLORS  8

#define CIRCLES_MODE_RADIAL     0
#define CIRCLES_MODE_RADIAL_3D  1
#define CIRCLES_MODE_METABALLS  2
#define CIRCLES_MODE_REGULAR    3
#define CIRCLES_MODE_FADING     4

#ifndef __ISPC_STRUCT_CirclesBall__
#define __ISPC_STRUCT_CirclesBall__
struct CirclesBall {
    float   x;
    float   y;
    float   radius;
    int32_t colorIdx;
};
#endif

#ifndef __ISPC_STRUCT_CirclesData__
#define __ISPC_STRUCT_CirclesData__
struct CirclesData {
    uint32_t    width;
    uint32_t    height;
    int32_t     mode;
    int32_t     numBalls;
    int32_t     colorCount;
    int32_t     allowAlpha;
    int32_t     wrap;
    int32_t     cx;
    int32_t     cy;
    int32_t     barSize;
    int32_t     blockSize;
    int32_t     f_offset;
    int32_t     maxRadius;
    int32_t     number;
    int32_t     effectState;
    CirclesBall balls[MAX_ISPC_CIRCLES_BALLS];
    uint8_t4    colorsAsRGBA[MAX_ISPC_CIRCLES_COLORS];
    float       colorsH[MAX_ISPC_CIRCLES_COLORS];
    float       colorsS[MAX_ISPC_CIRCLES_COLORS];
    float       colorsV[MAX_ISPC_CIRCLES_COLORS];
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
    extern void CirclesEffectISPC(const struct CirclesData * data, int32_t startIdx, int32_t endIdx, uint8_t4 * result);
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus
