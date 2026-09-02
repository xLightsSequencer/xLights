/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple's implementation lives in xlDisplayInfo.mm - NSScreen is the only thing
// that reports a variable range. Xcode compiles every file under src-ui-wx, so
// this one has to exclude itself rather than rely on not being listed.
#if !defined(__APPLE__)

#include "xlDisplayInfo.h"

#include <cstdio>
#include <set>

#if defined(_WIN32)
#include <windows.h>
#include <dxgi1_5.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif

namespace {
std::string Describe(int current, int best, bool vrrCapable) {
    if (current <= 0) {
        return "";
    }
    char buf[128];
    // "up to" only when the panel offers something higher at the resolution it
    // is already running: a 144Hz panel left set to 60 is a user-fixable reason
    // a high frame rate preview stutters, and a 60Hz panel is not.
    if (best > current) {
        snprintf(buf, sizeof(buf), "%dHz (up to %dHz at this resolution)%s",
                 current, best, vrrCapable ? " VRR capable" : "");
    } else {
        snprintf(buf, sizeof(buf), "%dHz%s", current, vrrCapable ? " VRR capable" : "");
    }
    return buf;
}
} // namespace

#if defined(_WIN32)

std::vector<std::string> GetDisplayRefreshInfo(const std::vector<xlDisplayQuery>& displays) {
    std::vector<std::string> result(displays.size());
    for (size_t i = 0; i < displays.size(); i++) {
        // wxDisplayMSW::GetName() returns MONITORINFOEX::szDevice, which is
        // exactly what EnumDisplaySettings wants - so this keys off the caller's
        // own identifier rather than assuming enumeration orders agree.
        if (displays[i].name.empty()) {
            continue;
        }
        std::wstring name(displays[i].name.begin(), displays[i].name.end());
        DEVMODEW cur = {};
        cur.dmSize = sizeof(cur);
        if (!::EnumDisplaySettingsW(name.c_str(), ENUM_CURRENT_SETTINGS, &cur)) {
            continue;
        }
        DWORD best = cur.dmDisplayFrequency;
        DEVMODEW m = {};
        m.dmSize = sizeof(m);
        for (DWORD n = 0; ::EnumDisplaySettingsW(name.c_str(), n, &m); ++n) {
            if (m.dmPelsWidth == cur.dmPelsWidth && m.dmPelsHeight == cur.dmPelsHeight &&
                m.dmDisplayFrequency > best) {
                best = m.dmDisplayFrequency;
            }
        }
        // No per-display VRR query exists on Windows; see
        // GetPresentCapabilityDescription for the closest available signal.
        result[i] = Describe((int)cur.dmDisplayFrequency, (int)best, false);
    }
    return result;
}

std::string GetPresentCapabilityDescription() {
    std::string result;
    // Late bound, like the DXGI adapter probe - nothing new to link.
    HMODULE dxgi = ::LoadLibraryW(L"dxgi.dll");
    if (dxgi == nullptr) {
        return result;
    }
    typedef HRESULT(WINAPI * CreateDXGIFactory1Fn)(REFIID, void**);
    auto createFactory = (CreateDXGIFactory1Fn)::GetProcAddress(dxgi, "CreateDXGIFactory1");
    IDXGIFactory5* factory = nullptr;
    if (createFactory != nullptr && SUCCEEDED(createFactory(__uuidof(IDXGIFactory5), (void**)&factory)) && factory != nullptr) {
        BOOL tearing = FALSE;
        if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing)))) {
            result = tearing ? "allow-tearing supported (variable refresh present path available)"
                             : "allow-tearing not supported";
        }
        factory->Release();
    }
    ::FreeLibrary(dxgi);
    return result;
}

#elif defined(__linux__)

namespace {
double ModeRefresh(const XRRModeInfo& m) {
    if (m.hTotal == 0 || m.vTotal == 0) {
        return 0.0;
    }
    double v = m.vTotal;
    if (m.modeFlags & RR_DoubleScan) {
        v *= 2;
    }
    if (m.modeFlags & RR_Interlace) {
        v /= 2;
    }
    return (double)m.dotClock / (m.hTotal * v);
}
} // namespace

std::vector<std::string> GetDisplayRefreshInfo(const std::vector<xlDisplayQuery>& displays) {
    std::vector<std::string> result(displays.size());
    // Its own connection: this runs from DumpConfig, before wx has a display,
    // and must not disturb one if it does.
    Display* dpy = XOpenDisplay(nullptr);
    if (dpy == nullptr) {
        return result;
    }
    int base = 0;
    int err = 0;
    if (XRRQueryExtension(dpy, &base, &err)) {
        Window root = DefaultRootWindow(dpy);
        XRRScreenResources* res = XRRGetScreenResourcesCurrent(dpy, root);
        if (res != nullptr) {
            // Set by the amdgpu / i915 / nvidia drivers; simply absent on
            // hardware or drivers that cannot do it, which is not the same as
            // "no" - so nothing is claimed when it is missing.
            Atom vrrAtom = XInternAtom(dpy, "vrr_capable", True);
            std::vector<std::string> unmatched;
            for (int o = 0; o < res->noutput; o++) {
                XRROutputInfo* oi = XRRGetOutputInfo(dpy, res, res->outputs[o]);
                if (oi == nullptr) {
                    continue;
                }
                if (oi->connection != RR_Connected || oi->crtc == 0) {
                    XRRFreeOutputInfo(oi);
                    continue;
                }
                XRRCrtcInfo* ci = XRRGetCrtcInfo(dpy, res, oi->crtc);
                if (ci == nullptr) {
                    XRRFreeOutputInfo(oi);
                    continue;
                }
                // wxDisplay carries no usable name on GTK, so match on the
                // geometry the caller already reported. A scaled GDK session
                // reports logical pixels where RandR reports device ones, so a
                // miss here is expected rather than exceptional - unmatched
                // outputs are paired off in order below instead of being
                // dropped, which would silently report nothing at all.
                size_t idx = displays.size();
                for (size_t i = 0; i < displays.size(); i++) {
                    if (displays[i].x == ci->x && displays[i].y == ci->y &&
                        displays[i].width == (int)ci->width && displays[i].height == (int)ci->height) {
                        idx = i;
                        break;
                    }
                }
                {
                    double current = 0.0;
                    double best = 0.0;
                    for (int m = 0; m < res->nmode; m++) {
                        double r = ModeRefresh(res->modes[m]);
                        if (res->modes[m].id == ci->mode) {
                            current = r;
                        }
                        if (res->modes[m].width != ci->width || res->modes[m].height != ci->height) {
                            continue;
                        }
                        for (int k = 0; k < oi->nmode; k++) {
                            if (oi->modes[k] == res->modes[m].id && r > best) {
                                best = r;
                            }
                        }
                    }
                    bool vrr = false;
                    if (vrrAtom != None) {
                        Atom type = None;
                        int fmt = 0;
                        unsigned long n = 0;
                        unsigned long bytes = 0;
                        unsigned char* data = nullptr;
                        if (XRRGetOutputProperty(dpy, res->outputs[o], vrrAtom, 0, 1, False, False,
                                                 AnyPropertyType, &type, &fmt, &n, &bytes, &data) == Success &&
                            data != nullptr && n >= 1) {
                            vrr = ((long*)data)[0] != 0;
                        }
                        if (data != nullptr) {
                            XFree(data);
                        }
                    }
                    std::string described = Describe((int)(current + 0.5), (int)(best + 0.5), vrr);
                    if (idx < displays.size()) {
                        result[idx] = described;
                    } else if (!described.empty()) {
                        unmatched.push_back(described);
                    }
                }
                XRRFreeCrtcInfo(ci);
                XRRFreeOutputInfo(oi);
            }
            // Pair anything geometry could not place with whatever is still
            // blank, in order. Only safe because both lists are the displays
            // this session actually has.
            for (size_t i = 0, u = 0; i < result.size() && u < unmatched.size(); i++) {
                if (result[i].empty()) {
                    result[i] = unmatched[u++];
                }
            }
            XRRFreeScreenResources(res);
        }
    }
    XCloseDisplay(dpy);
    return result;
}

std::string GetPresentCapabilityDescription() {
    return "";
}

#else

std::vector<std::string> GetDisplayRefreshInfo(const std::vector<xlDisplayQuery>& displays) {
    return std::vector<std::string>(displays.size());
}

std::string GetPresentCapabilityDescription() {
    return "";
}

#endif

#endif // !__APPLE__
