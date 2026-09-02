/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

// What the machine can actually render on, in one place.
//
// The facts arrive at two different times and that split is the whole reason
// this exists. The adapters the OS has bound, and whether this is a Remote
// Desktop session, are knowable before any graphics context is created - which
// matters because the backend choice is made then, and because on a machine
// running the generic Microsoft display driver creating a context is the thing
// that fails. The GL renderer and the Vulkan device are only knowable after
// bring-up, so those paths record what they found as they learn it.
//
// Before this, each consumer sniffed its own signal or none: the "Auto" preview
// backend looked only at Linux env vars, the GL 1.x warning fired only if a
// context had been created at all, and the GPU-rendering checkbox simply
// vanished with no explanation.

struct xlGPUAdapter {
    std::string name;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
    uint64_t dedicatedVideoMemoryMB = 0;
    // No vendor driver behind this adapter: DXGI's software flag, or Microsoft
    // as the vendor (the Basic Render Driver and WARP both report 0x1414).
    bool software = false;
};

enum class xlGraphicsHealth {
    Unknown,               // nothing probed on this platform
    OK,                    // a hardware adapter, and nothing has reported a failure
    RemoteSession,         // GL is generic because of RDP, not a missing driver
    GenericDisplayDriver,  // no hardware adapter bound at all
    NoUsableGL,            // an adapter is present but GL is 1.x or failed to come up
};

class xlGraphicsCapability {
public:
    static xlGraphicsCapability& Instance();

    // --- Pre-context facts (probed once, cached) ---

    const std::vector<xlGPUAdapter>& Adapters();
    // False when every adapter the OS enumerated is a software one. Note that a
    // healthy machine commonly enumerates the Basic Render Driver ALONGSIDE its
    // real GPU, so "a software adapter exists" is not the signal - this is.
    bool HasHardwareAdapter();
    bool IsRemoteSession();

    // --- Post-context facts, recorded by the bring-up paths ---

    void RecordGL(const char* version, const char* renderer, const char* vendor);
    void RecordGLInitFailed();
    void RecordVulkan(const std::string& deviceName, int deviceType, bool computeUsable);
    void RecordNoVulkanDevice();

    // --- Derived ---

    xlGraphicsHealth Health();
    // The adapter list as one line, for the machine-config banner and the log.
    // Empty where nothing can be enumerated (Linux, where the GL renderer
    // string is the only thing available and is logged in its own right).
    std::string DescribeAdapters();

private:
    xlGraphicsCapability() = default;
    void probe();

    std::mutex _lock;
    bool _probed = false;
    std::vector<xlGPUAdapter> _adapters;
    bool _remoteSession = false;
    int _glMajor = -1;
    bool _glFailed = false;
    bool _glRecorded = false;
};
