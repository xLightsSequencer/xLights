
#include "CudaComputeUtilities.h"
#include "CudaEffectDataTypes.h"

#include <log4cpp/Category.hh>

#include <cuda_runtime.h>

CudaComputeUtilities::CudaComputeUtilities() {
    enabled = false;
    int deviceCount {0};
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    //logger dont exist yet, so this does nothing
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (error_id != cudaSuccess) {
        logger_base.warn("CUDA: GetDeviceCount Returned %d -> %s.", static_cast<int>(error_id), cudaGetErrorString(error_id));
        return;
    }
    if (deviceCount == 0) {
       logger_base.debug("CUDA: There are no available device(s) that support CUDA.");
    } else {
        logger_base.debug("CUDA: Detected %d CUDA Capable device(s)", deviceCount);
        enabled = true;
    }
}

CudaComputeUtilities::~CudaComputeUtilities() {

}

CudaComputeUtilities CudaComputeUtilities::INSTANCE;

extern "C" {
bool isCudaComputeSupported() {
    return CudaComputeUtilities::INSTANCE.enabled;
}
}