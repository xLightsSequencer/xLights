/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGraphicsCapability.h"

#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <dxgi.h>
#endif

#include <log.h>

#include "UtilFunctions.h"

#ifdef _WIN32
static std::string WideToUTF8(const wchar_t* w) {
    if (w == nullptr || *w == 0) {
        return "";
    }
    int need = ::WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (need <= 1) {
        return "";
    }
    std::string out((size_t)need - 1, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), need, nullptr, nullptr);
    return out;
}
#endif

xlGraphicsCapability& xlGraphicsCapability::Instance() {
    static xlGraphicsCapability instance;
    return instance;
}

void xlGraphicsCapability::probe() {
    // Caller holds _lock.
    if (_probed) {
        return;
    }
    _probed = true;

#ifdef _WIN32
    // DXGI rather than the GL or Vulkan banner: it answers before any context
    // exists, and it still names the adapter when the machine is running the
    // Microsoft Basic Display Adapter - the case where GL reports nothing but
    // "GDI Generic" and Vulkan has no ICD at all. Late bound so no new import
    // is added to the binary.
    HMODULE dxgi = ::LoadLibraryW(L"dxgi.dll");
    if (dxgi != nullptr) {
        typedef HRESULT(WINAPI * CreateDXGIFactory1Fn)(REFIID, void**);
        auto createFactory = (CreateDXGIFactory1Fn)::GetProcAddress(dxgi, "CreateDXGIFactory1");
        IDXGIFactory1* factory = nullptr;
        if (createFactory != nullptr && SUCCEEDED(createFactory(__uuidof(IDXGIFactory1), (void**)&factory)) && factory != nullptr) {
            IDXGIAdapter1* adapter = nullptr;
            for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                DXGI_ADAPTER_DESC1 desc = {};
                if (adapter != nullptr && SUCCEEDED(adapter->GetDesc1(&desc))) {
                    xlGPUAdapter a;
                    a.name = WideToUTF8(desc.Description);
                    a.vendorId = desc.VendorId;
                    a.deviceId = desc.DeviceId;
                    a.dedicatedVideoMemoryMB = (uint64_t)(desc.DedicatedVideoMemory / (1024 * 1024));
                    a.software = (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0 || desc.VendorId == 0x1414;
                    _adapters.push_back(a);
                }
                if (adapter != nullptr) {
                    adapter->Release();
                    adapter = nullptr;
                }
            }
            factory->Release();
        }
        ::FreeLibrary(dxgi);
    }
    _remoteSession = ::GetSystemMetrics(SM_REMOTESESSION) != 0;
#endif
}

const std::vector<xlGPUAdapter>& xlGraphicsCapability::Adapters() {
    std::unique_lock<std::mutex> lock(_lock);
    probe();
    return _adapters;
}

bool xlGraphicsCapability::HasHardwareAdapter() {
    std::unique_lock<std::mutex> lock(_lock);
    probe();
    if (_adapters.empty()) {
        // Nothing enumerable (not Windows, or DXGI unavailable). Absence of
        // evidence only - never report a healthy machine as driverless.
        return true;
    }
    for (auto const& a : _adapters) {
        if (!a.software) {
            return true;
        }
    }
    return false;
}

bool xlGraphicsCapability::IsRemoteSession() {
    std::unique_lock<std::mutex> lock(_lock);
    probe();
    return _remoteSession;
}

void xlGraphicsCapability::RecordGL(const char* version, const char* renderer, const char* vendor) {
    {
        std::unique_lock<std::mutex> lock(_lock);
        _glRecorded = true;
        _glMajor = (version != nullptr && version[0] >= '0' && version[0] <= '9') ? version[0] - '0' : -1;
    }
    AppendMachineConfig(fmt::format("  OpenGL: {} ({}) ({})",
                                    version ? version : "?",
                                    renderer ? renderer : "?",
                                    vendor ? vendor : "?"));
}

void xlGraphicsCapability::RecordGLInitFailed() {
    {
        std::unique_lock<std::mutex> lock(_lock);
        _glFailed = true;
    }
    AppendMachineConfig("  OpenGL: shared context init FAILED - previews will not draw");
}

void xlGraphicsCapability::RecordVulkan(const std::string& deviceName, int deviceType, bool computeUsable) {
    AppendMachineConfig(fmt::format("  Vulkan: {} (type {}){}", deviceName, deviceType,
                                    computeUsable ? "" : " - CPU implementation, compute effects stay on the CPU"));
}

void xlGraphicsCapability::RecordNoVulkanDevice() {
    AppendMachineConfig("  Vulkan: no usable device");
}

xlGraphicsHealth xlGraphicsCapability::Health() {
    bool hardware = HasHardwareAdapter();
    bool remote = IsRemoteSession();

    std::unique_lock<std::mutex> lock(_lock);
    // Order matters. A Remote Desktop session reports generic GL whether or not
    // the machine has a driver, and the answer for the user is different, so it
    // is named first rather than folded into the driver case.
    if (remote) {
        return xlGraphicsHealth::RemoteSession;
    }
    if (!hardware) {
        return xlGraphicsHealth::GenericDisplayDriver;
    }
    if (_glFailed || (_glRecorded && _glMajor >= 0 && _glMajor <= 1)) {
        return xlGraphicsHealth::NoUsableGL;
    }
    if (_adapters.empty() && !_glRecorded) {
        return xlGraphicsHealth::Unknown;
    }
    return xlGraphicsHealth::OK;
}

std::string xlGraphicsCapability::DescribeAdapters() {
    std::unique_lock<std::mutex> lock(_lock);
    probe();
    if (_adapters.empty()) {
        return "";
    }
    std::string result;
    bool anyHardware = false;
    for (auto const& a : _adapters) {
        anyHardware = anyHardware || !a.software;
        if (!result.empty()) {
            result += "; ";
        }
        result += a.name;
        result += fmt::format(" [{:04x}:{:04x}] {}MB{}", a.vendorId, a.deviceId,
                              a.dedicatedVideoMemoryMB, a.software ? " (software)" : "");
    }
    if (!anyHardware) {
        // A single greppable token: this is the state we want to be able to
        // count across crash reports, and "software" alone also appears on
        // healthy machines that merely enumerate the Basic Render Driver
        // alongside a real GPU.
        result += " -- NO HARDWARE GPU ADAPTER";
    }
    return result;
}

#ifdef _WIN32
std::string GetGPUDescription() {
    return xlGraphicsCapability::Instance().DescribeAdapters();
}
#endif
