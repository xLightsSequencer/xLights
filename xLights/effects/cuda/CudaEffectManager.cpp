#include "../EffectManager.h"

#include "CudaRenderUtils.h"
#include "CudaComputeUtilities.h"

RenderableEffect* CreateCudaEffect(EffectManager::RGB_EFFECTS_e eff) {
    if (CudaComputeUtilities::INSTANCE.computeEnabled()) {
        switch (eff) {
        case EffectManager::eff_BUTTERFLY:
            //return new CudaButterflyEffect(eff);
             return nullptr;
        case EffectManager::eff_PLASMA:
            //return new CudaPlasmaEffect(eff);
             return nullptr;
        case EffectManager::eff_WARP:
            //return new CudaWarpEffect(eff);
             return nullptr;
        default:
            return nullptr;
        }
    }
    return nullptr;
}
