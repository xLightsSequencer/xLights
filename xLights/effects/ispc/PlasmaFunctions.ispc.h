//
// /Users/dkulp/working/xLights/xLights/effects/ispc/PlasmaFunctions.ispc.h
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



#ifndef __ISPC_ALIGN__
#if defined(__clang__) || !defined(_MSC_VER)
// Clang, GCC, ICC
#define __ISPC_ALIGN__(s) __attribute__((aligned(s)))
#define __ISPC_ALIGNED_STRUCT__(s) struct __ISPC_ALIGN__(s)
#else
// Visual Studio
#define __ISPC_ALIGN__(s) __declspec(align(s))
#define __ISPC_ALIGNED_STRUCT__(s) __ISPC_ALIGN__(s) struct
#endif
#endif

#ifndef __ISPC_STRUCT_PlasmaData__
#define __ISPC_STRUCT_PlasmaData__
struct PlasmaData {
    float Style;
    float state;
    float Line_Density;
    float time;
    float sin_time_5;
    float cos_time_3;
    float sin_time_2;
    uint32_t width;
    uint32_t height;
    uint8_t4  colors[8];
    uint16_t numColors;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
#if defined(__cplusplus)
    extern void PlasmaEffectStyle0(const struct PlasmaData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PlasmaEffectStyle0(const struct PlasmaData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PlasmaEffectStyle0 function declaraion
#if defined(__cplusplus)
    extern void PlasmaEffectStyle1(const struct PlasmaData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PlasmaEffectStyle1(const struct PlasmaData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PlasmaEffectStyle1 function declaraion
#if defined(__cplusplus)
    extern void PlasmaEffectStyle2(const struct PlasmaData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PlasmaEffectStyle2(const struct PlasmaData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PlasmaEffectStyle2 function declaraion
#if defined(__cplusplus)
    extern void PlasmaEffectStyle3(const struct PlasmaData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PlasmaEffectStyle3(const struct PlasmaData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PlasmaEffectStyle3 function declaraion
#if defined(__cplusplus)
    extern void PlasmaEffectStyle4(const struct PlasmaData &data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#else
    extern void PlasmaEffectStyle4(const struct PlasmaData *data, int32_t startIdx, int32_t endIdx, uint8_t4   * result);
#endif // PlasmaEffectStyle4 function declaraion
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus