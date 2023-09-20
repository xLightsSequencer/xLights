
#include "CudaComputeUtilities.h"
#include "CudaEffectDataTypes.h"

#include <log4cpp/Category.hh>

#include <cuda_runtime.h>
#include "helper_cuda.h"



CudaComputeUtilities::CudaComputeUtilities() {
    enabled = false;
    int deviceCount {0};
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    //logger don't exist yet, so this does nothing
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
        for (int dev = 0; dev < deviceCount; ++dev) 
        {
            int driverVersion = 0, runtimeVersion = 0;
            cudaSetDevice(dev);
            cudaDeviceProp deviceProp;
            cudaGetDeviceProperties(&deviceProp, dev);
            logger_base.warn("CUDA: Device %d:  %s.", dev, deviceProp.name);


            // Console log
            cudaDriverGetVersion(&driverVersion);
            cudaRuntimeGetVersion(&runtimeVersion);
            logger_base.debug("  CUDA Driver Version / Runtime Version          %d.%d / %d.%d\n",
                   driverVersion / 1000, (driverVersion % 100) / 10,
                   runtimeVersion / 1000, (runtimeVersion % 100) / 10);
            logger_base.debug("  CUDA Capability Major/Minor version number:    %d.%d\n",
                   deviceProp.major, deviceProp.minor);

            char msg[256];
        #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            sprintf_s(msg, sizeof(msg),
                      "  Total amount of global memory:                 %.0f MBytes "
                      "(%llu bytes)\n",
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

            logger_base.debug("  (%03d) Multiprocessors, (%03d) CUDA Cores/MP:    %d CUDA Cores\n",
                   deviceProp.multiProcessorCount,
                   _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor),
                   _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) *
                       deviceProp.multiProcessorCount);
            logger_base.debug(
                "  GPU Max Clock rate:                            %.0f MHz (%0.2f "
                "GHz)\n",
                deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);

            // This is supported in CUDA 5.0 (runtime API device properties)
            logger_base.debug("  Memory Clock rate:                             %.0f Mhz\n",
                   deviceProp.memoryClockRate * 1e-3f);
            logger_base.debug("  Memory Bus Width:                              %d-bit\n",
                   deviceProp.memoryBusWidth);

            if (deviceProp.l2CacheSize) {
             logger_base.debug("  L2 Cache Size:                                 %d bytes\n",
                     deviceProp.l2CacheSize);
            }

        

           logger_base.debug(
                "  Maximum Texture Dimension Size (x,y,z)         1D=(%d), 2D=(%d, "
                "%d), 3D=(%d, %d, %d)\n",
                deviceProp.maxTexture1D, deviceProp.maxTexture2D[0],
                deviceProp.maxTexture2D[1], deviceProp.maxTexture3D[0],
                deviceProp.maxTexture3D[1], deviceProp.maxTexture3D[2]);
            logger_base.debug(
                "  Maximum Layered 1D Texture Size, (num) layers  1D=(%d), %d layers\n",
                deviceProp.maxTexture1DLayered[0], deviceProp.maxTexture1DLayered[1]);
            logger_base.debug(
                "  Maximum Layered 2D Texture Size, (num) layers  2D=(%d, %d), %d "
                "layers\n",
                deviceProp.maxTexture2DLayered[0], deviceProp.maxTexture2DLayered[1],
                deviceProp.maxTexture2DLayered[2]);

            logger_base.debug("  Total amount of constant memory:               %zu bytes\n",
                   deviceProp.totalConstMem);
            logger_base.debug("  Total amount of shared memory per block:       %zu bytes\n",
                   deviceProp.sharedMemPerBlock);
            logger_base.debug("  Total shared memory per multiprocessor:        %zu bytes\n",
                   deviceProp.sharedMemPerMultiprocessor);
            logger_base.debug("  Total number of registers available per block: %d\n",
                   deviceProp.regsPerBlock);
            logger_base.debug("  Warp size:                                     %d\n",
                   deviceProp.warpSize);
            logger_base.debug("  Maximum number of threads per multiprocessor:  %d\n",
                   deviceProp.maxThreadsPerMultiProcessor);
            logger_base.debug("  Maximum number of threads per block:           %d\n",
                   deviceProp.maxThreadsPerBlock);
            logger_base.debug("  Max dimension size of a thread block (x,y,z): (%d, %d, %d)\n",
                   deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1],
                   deviceProp.maxThreadsDim[2]);
            logger_base.debug("  Max dimension size of a grid size    (x,y,z): (%d, %d, %d)\n",
                   deviceProp.maxGridSize[0], deviceProp.maxGridSize[1],
                   deviceProp.maxGridSize[2]);
            logger_base.debug("  Maximum memory pitch:                          %zu bytes\n",
                   deviceProp.memPitch);
            logger_base.debug("  Texture alignment:                             %zu bytes\n",
                   deviceProp.textureAlignment);
            logger_base.debug(
                "  Concurrent copy and kernel execution:          %s with %d copy "
                "engine(s)\n",
                (deviceProp.deviceOverlap ? "Yes" : "No"), deviceProp.asyncEngineCount);
            logger_base.debug("  Run time limit on kernels:                     %s\n",
                   deviceProp.kernelExecTimeoutEnabled ? "Yes" : "No");
            logger_base.debug("  Integrated GPU sharing Host Memory:            %s\n",
                   deviceProp.integrated ? "Yes" : "No");
            logger_base.debug("  Support host page-locked memory mapping:       %s\n",
                   deviceProp.canMapHostMemory ? "Yes" : "No");
            logger_base.debug("  Alignment requirement for Surfaces:            %s\n",
                   deviceProp.surfaceAlignment ? "Yes" : "No");
            logger_base.debug("  Device has ECC support:                        %s\n",
                   deviceProp.ECCEnabled ? "Enabled" : "Disabled");
        #if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            logger_base.debug("  CUDA Device Driver Mode (TCC or WDDM):         %s\n",
                   deviceProp.tccDriver ? "TCC (Tesla Compute Cluster Driver)"
                                        : "WDDM (Windows Display Driver Model)");
        #endif
            logger_base.debug("  Device supports Unified Addressing (UVA):      %s\n",
                   deviceProp.unifiedAddressing ? "Yes" : "No");
            logger_base.debug("  Device supports Managed Memory:                %s\n",
                   deviceProp.managedMemory ? "Yes" : "No");
            logger_base.debug("  Device supports Compute Preemption:            %s\n",
                   deviceProp.computePreemptionSupported ? "Yes" : "No");
            logger_base.debug("  Supports Cooperative Kernel Launch:            %s\n",
                   deviceProp.cooperativeLaunch ? "Yes" : "No");
            logger_base.debug("  Supports MultiDevice Co-op Kernel Launch:      %s\n",
                   deviceProp.cooperativeMultiDeviceLaunch ? "Yes" : "No");
            logger_base.debug("  Device PCI Domain ID / Bus ID / location ID:   %d / %d / %d\n",
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
            logger_base.debug("  Compute Mode:\n");
            logger_base.debug("     < %s >\n", sComputeMode[deviceProp.computeMode]);
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