//
// /Users/dkulp/working/xLights/xLights/effects/ispc/LayerBlendingFunctions.ispc.h
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

#ifndef __ISPC_STRUCT_LayerBlendingData__
#define __ISPC_STRUCT_LayerBlendingData__
struct LayerBlendingData {
    uint32_t startNode;
    uint32_t endNode;
    uint32_t nodeCount;
    uint32_t bufferWi;
    uint32_t bufferHi;
    bool useMask;
    float hueAdjust;
    float valueAdjust;
    float saturationAdjust;
    int32_t brightness;
    int32_t contrast;
    float fadeFactor;
    float effectMixThreshold;
    bool effectMixVaries;
    bool brightnessLevel;
    int32_t mixTypeData;
    int32_t outputSparkleCount;
    uint32_t sparkleColor;
    bool isChromaKey;
    int32_t chromaSensitivity;
    uint32_t chromaColor;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
extern "C" {
#endif // __cplusplus
#if defined(__cplusplus)
    extern void AdditiveFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void AdditiveFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // AdditiveFunction function declaraion
#if defined(__cplusplus)
    extern void AdjustBrightnessContrast(const struct LayerBlendingData &data, uint32_t * result);
#else
    extern void AdjustBrightnessContrast(const struct LayerBlendingData *data, uint32_t * result);
#endif // AdjustBrightnessContrast function declaraion
#if defined(__cplusplus)
    extern void AdjustBrightnessLevel(const struct LayerBlendingData &data, uint32_t * result);
#else
    extern void AdjustBrightnessLevel(const struct LayerBlendingData *data, uint32_t * result);
#endif // AdjustBrightnessLevel function declaraion
#if defined(__cplusplus)
    extern void AdjustHSV(const struct LayerBlendingData &data, uint32_t * result);
#else
    extern void AdjustHSV(const struct LayerBlendingData *data, uint32_t * result);
#endif // AdjustHSV function declaraion
#if defined(__cplusplus)
    extern void ApplySparkles(const struct LayerBlendingData &data, uint32_t * result, uint16_t * sparkles);
#else
    extern void ApplySparkles(const struct LayerBlendingData *data, uint32_t * result, uint16_t * sparkles);
#endif // ApplySparkles function declaraion
#if defined(__cplusplus)
    extern void AsBrightnessFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void AsBrightnessFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // AsBrightnessFunction function declaraion
#if defined(__cplusplus)
    extern void AveragedFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void AveragedFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // AveragedFunction function declaraion
#if defined(__cplusplus)
    extern void BottomTopFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void BottomTopFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // BottomTopFunction function declaraion
#if defined(__cplusplus)
    extern void Effect1_2_Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Effect1_2_Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Effect1_2_Function function declaraion
#if defined(__cplusplus)
    extern void FirstLayerFade(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src);
#else
    extern void FirstLayerFade(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src);
#endif // FirstLayerFade function declaraion
#if defined(__cplusplus)
    extern void GetColorsISPCKernel(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint8_t * mask, const uint32_t * indexes);
#else
    extern void GetColorsISPCKernel(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint8_t * mask, const uint32_t * indexes);
#endif // GetColorsISPCKernel function declaraion
#if defined(__cplusplus)
    extern void HighlightFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void HighlightFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // HighlightFunction function declaraion
#if defined(__cplusplus)
    extern void HighlightVibrantFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void HighlightVibrantFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // HighlightVibrantFunction function declaraion
#if defined(__cplusplus)
    extern void LayeredFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void LayeredFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // LayeredFunction function declaraion
#if defined(__cplusplus)
    extern void LeftRightFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void LeftRightFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // LeftRightFunction function declaraion
#if defined(__cplusplus)
    extern void Mask1Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Mask1Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Mask1Function function declaraion
#if defined(__cplusplus)
    extern void Mask2Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Mask2Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Mask2Function function declaraion
#if defined(__cplusplus)
    extern void MaxFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void MaxFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // MaxFunction function declaraion
#if defined(__cplusplus)
    extern void MinFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void MinFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // MinFunction function declaraion
#if defined(__cplusplus)
    extern void NonAlphaFade(const struct LayerBlendingData &data, uint32_t * result);
#else
    extern void NonAlphaFade(const struct LayerBlendingData *data, uint32_t * result);
#endif // NonAlphaFade function declaraion
#if defined(__cplusplus)
    extern void NormalBlendFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void NormalBlendFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // NormalBlendFunction function declaraion
#if defined(__cplusplus)
    extern void PutColorsForNodes(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint8_t * mask, const uint32_t * indexes);
#else
    extern void PutColorsForNodes(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint8_t * mask, const uint32_t * indexes);
#endif // PutColorsForNodes function declaraion
#if defined(__cplusplus)
    extern void Reveal12Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Reveal12Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Reveal12Function function declaraion
#if defined(__cplusplus)
    extern void Reveal21Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Reveal21Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Reveal21Function function declaraion
#if defined(__cplusplus)
    extern void Shadow_1on2Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Shadow_1on2Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Shadow_1on2Function function declaraion
#if defined(__cplusplus)
    extern void Shadow_2on1Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Shadow_2on1Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Shadow_2on1Function function declaraion
#if defined(__cplusplus)
    extern void SubtractiveFunction(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void SubtractiveFunction(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // SubtractiveFunction function declaraion
#if defined(__cplusplus)
    extern void TrueUnmask1Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void TrueUnmask1Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // TrueUnmask1Function function declaraion
#if defined(__cplusplus)
    extern void TrueUnmask2Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void TrueUnmask2Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // TrueUnmask2Function function declaraion
#if defined(__cplusplus)
    extern void Unmask1Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Unmask1Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Unmask1Function function declaraion
#if defined(__cplusplus)
    extern void Unmask2Function(const struct LayerBlendingData &data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#else
    extern void Unmask2Function(const struct LayerBlendingData *data, uint32_t * result, const uint32_t * src, const uint32_t * indexes);
#endif // Unmask2Function function declaraion
#if defined(__cplusplus) && (! defined(__ISPC_NO_EXTERN_C) || !__ISPC_NO_EXTERN_C )
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus