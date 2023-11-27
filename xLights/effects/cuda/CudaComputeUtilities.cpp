
#include "CudaComputeUtilities.h"
#include "CudaEffectDataTypes.h"

#include <log4cpp/Category.hh>

#include <cuda_runtime.h>
#include "helper_cuda.h"
#include <wx/msgdlg.h>

CudaComputeUtilities::CudaComputeUtilities() {
    enabled = false;
    int deviceCount {0};
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    if (error_id != cudaSuccess) {
        return;
    }
    if (deviceCount == 0) {
      
    } else {       
        enabled = true;
    }

}

void CudaComputeUtilities::printGPUInfo() 
{    
    int deviceCount {0};
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (error_id != cudaSuccess) {
        logger_base.warn("CUDA: GetDeviceCount Returned %d -> %s.", static_cast<int>(error_id), cudaGetErrorString(error_id));
        return;
    }
    if (deviceCount == 0) {
       logger_base.debug("CUDA: There are no available device(s) that support CUDA.");
    } else {
        logger_base.debug("CUDA: Detected %d CUDA Capable device(s)", deviceCount);
        for (int dev = 0; dev < deviceCount; ++dev) 
        {
            int driverVersion = 0, runtimeVersion = 0;
            cudaSetDevice(dev);
            cudaDeviceProp deviceProp;
            cudaGetDeviceProperties(&deviceProp, dev);
            logger_base.debug("CUDA: Device %d:  %s.", dev, deviceProp.name);

            // Console log
            cudaDriverGetVersion(&driverVersion);
            cudaRuntimeGetVersion(&runtimeVersion);
            logger_base.debug("  CUDA Driver Version / Runtime Version          %d.%d / %d.%d",
                   driverVersion / 1000, (driverVersion % 100) / 10,
                   runtimeVersion / 1000, (runtimeVersion % 100) / 10);
            logger_base.debug("  CUDA Capability Major/Minor version number:    %d.%d",
                   deviceProp.major, deviceProp.minor);

            char msg[256];
        #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            sprintf_s(msg, sizeof(msg),
                      "  Total amount of global memory:                 %.0f MBytes "
                      "(%llu bytes)",
                      static_cast<float>(deviceProp.totalGlobalMem / 1048576.0f),
                      (unsigned long long)deviceProp.totalGlobalMem);
        #else
            snprintf(msg, sizeof(msg),
                     "  Total amount of global memory:                 %.0f MBytes "
                     "(%llu bytes)\n",
                     static_cast<float>(deviceProp.totalGlobalMem / 1048576.0f),
                     (unsigned long long)deviceProp.totalGlobalMem);
        #endif
            logger_base.debug("%s", msg);

            logger_base.debug("  (%03d) Multiprocessors, (%03d) CUDA Cores/MP:    %d CUDA Cores",
                   deviceProp.multiProcessorCount,
                   _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor),
                   _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) *
                       deviceProp.multiProcessorCount);
            logger_base.debug(
                "  GPU Max Clock rate:                            %.0f MHz (%0.2f "
                "GHz)",
                deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);

            // This is supported in CUDA 5.0 (runtime API device properties)
            logger_base.debug("  Memory Clock rate:                             %.0f Mhz",
                   deviceProp.memoryClockRate * 1e-3f);
            logger_base.debug("  Memory Bus Width:                              %d-bit",
                   deviceProp.memoryBusWidth);

            if (deviceProp.l2CacheSize) {
             logger_base.debug("  L2 Cache Size:                                 %d bytes",
                     deviceProp.l2CacheSize);
            }      

           logger_base.debug(
                "  Maximum Texture Dimension Size (x,y,z)         1D=(%d), 2D=(%d, "
                "%d), 3D=(%d, %d, %d)",
                deviceProp.maxTexture1D, deviceProp.maxTexture2D[0],
                deviceProp.maxTexture2D[1], deviceProp.maxTexture3D[0],
                deviceProp.maxTexture3D[1], deviceProp.maxTexture3D[2]);
            logger_base.debug(
                "  Maximum Layered 1D Texture Size, (num) layers  1D=(%d), %d layers",
                deviceProp.maxTexture1DLayered[0], deviceProp.maxTexture1DLayered[1]);
            logger_base.debug(
                "  Maximum Layered 2D Texture Size, (num) layers  2D=(%d, %d), %d "
                "layers",
                deviceProp.maxTexture2DLayered[0], deviceProp.maxTexture2DLayered[1],
                deviceProp.maxTexture2DLayered[2]);

            logger_base.debug("  Total amount of constant memory:               %zu bytes",
                   deviceProp.totalConstMem);
            logger_base.debug("  Total amount of shared memory per block:       %zu bytes",
                   deviceProp.sharedMemPerBlock);
            logger_base.debug("  Total shared memory per multiprocessor:        %zu bytes",
                   deviceProp.sharedMemPerMultiprocessor);
            logger_base.debug("  Total number of registers available per block: %d",
                   deviceProp.regsPerBlock);
            logger_base.debug("  Warp size:                                     %d",
                   deviceProp.warpSize);
            logger_base.debug("  Maximum number of threads per multiprocessor:  %d",
                   deviceProp.maxThreadsPerMultiProcessor);
            logger_base.debug("  Maximum number of threads per block:           %d",
                   deviceProp.maxThreadsPerBlock);
            logger_base.debug("  Max dimension size of a thread block (x,y,z): (%d, %d, %d)",
                   deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1],
                   deviceProp.maxThreadsDim[2]);
            logger_base.debug("  Max dimension size of a grid size    (x,y,z): (%d, %d, %d)",
                   deviceProp.maxGridSize[0], deviceProp.maxGridSize[1],
                   deviceProp.maxGridSize[2]);
            logger_base.debug("  Maximum memory pitch:                          %zu bytes",
                   deviceProp.memPitch);
            logger_base.debug("  Texture alignment:                             %zu bytes",
                   deviceProp.textureAlignment);
            logger_base.debug(
                "  Concurrent copy and kernel execution:          %s with %d copy "
                "engine(s)",
                (deviceProp.deviceOverlap ? "Yes" : "No"), deviceProp.asyncEngineCount);
            logger_base.debug("  Run time limit on kernels:                     %s",
                   deviceProp.kernelExecTimeoutEnabled ? "Yes" : "No");
            logger_base.debug("  Integrated GPU sharing Host Memory:            %s",
                   deviceProp.integrated ? "Yes" : "No");
            logger_base.debug("  Support host page-locked memory mapping:       %s",
                   deviceProp.canMapHostMemory ? "Yes" : "No");
            logger_base.debug("  Alignment requirement for Surfaces:            %s",
                   deviceProp.surfaceAlignment ? "Yes" : "No");
            logger_base.debug("  Device has ECC support:                        %s",
                   deviceProp.ECCEnabled ? "Enabled" : "Disabled");
        #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            logger_base.debug("  CUDA Device Driver Mode (TCC or WDDM):         %s",
                   deviceProp.tccDriver ? "TCC (Tesla Compute Cluster Driver)"
                                        : "WDDM (Windows Display Driver Model)");
        #endif
            logger_base.debug("  Device supports Unified Addressing (UVA):      %s",
                   deviceProp.unifiedAddressing ? "Yes" : "No");
            logger_base.debug("  Device supports Managed Memory:                %s",
                   deviceProp.managedMemory ? "Yes" : "No");
            logger_base.debug("  Device supports Compute Preemption:            %s",
                   deviceProp.computePreemptionSupported ? "Yes" : "No");
            logger_base.debug("  Supports Cooperative Kernel Launch:            %s",
                   deviceProp.cooperativeLaunch ? "Yes" : "No");
            logger_base.debug("  Supports MultiDevice Co-op Kernel Launch:      %s",
                   deviceProp.cooperativeMultiDeviceLaunch ? "Yes" : "No");
            logger_base.debug("  Device PCI Domain ID / Bus ID / location ID:   %d / %d / %d",
                   deviceProp.pciDomainID, deviceProp.pciBusID, deviceProp.pciDeviceID);

            const char *sComputeMode[] = {
                "Default (multiple host threads can use ::cudaSetDevice() with device "
                "simultaneously)",
                "Exclusive (only one host thread in one process is able to use "
                "::cudaSetDevice() with this device)",
                "Prohibited (no host thread can use ::cudaSetDevice() with this "
                "device)",
                "Exclusive Process (many threads in one process is able to use "
                "::cudaSetDevice() with this device)",
                "Unknown", NULL};
            logger_base.debug("  Compute Mode:");
            logger_base.debug("     < %s >", sComputeMode[deviceProp.computeMode]);
          }
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