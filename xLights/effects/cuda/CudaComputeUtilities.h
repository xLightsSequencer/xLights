#pragma once


#include "GPURenderUtils.h"
#include "CudaEffectDataTypes.h"


class CudaComputeUtilities {
public:
    CudaComputeUtilities();
    ~CudaComputeUtilities();
    bool computeEnabled() {
        return enabled;
    }

    bool enabled {true};

    static CudaComputeUtilities INSTANCE;
};

