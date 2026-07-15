 /***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef _DEBUG
//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#endif

//(*AppHeaders
#include "xLightsMain.h"
#include "render/HeadlessRenderContext.h"
#include "render/TextDrawingContext.h"
#include "graphics/wxTextDrawingContext.h"
#include "graphics/GLContextManager.h"
#include "render/FSEQFile.h"
#include "render/GPURenderUtils.h"
#include "media/VideoReader.h"
#include <wx/config.h>

#ifdef __APPLE__
void SetHeadlessNoDock(); // ExternalHooksMacOSUI.mm — demote to background (no Dock/focus)
#endif
#include <wx/time.h>
#include <wx/image.h>
//*)
#include <wx/fontmap.h>

#include <wx/stdpaths.h>
#include <wx/cmdline.h>
#include <wx/debugrpt.h>
#include <wx/version.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>
#include <wx/config.h>

#include <stdlib.h>     /* srand */
#include <time.h>       /* time */
#include <thread>
#include <chrono>
#include <iomanip>
#include "utils/ThreadUtils.h"
#include <curl/curl.h>

#include "xLightsApp.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include "utils/TraceLog.h"
#include "utils/ExternalHooks.h"
#include "effects/ShaderEffect.h"  // --shadertranslate spike
#ifdef __APPLE__
#include "effects/metal/MetalShaderTranslator.h"
#endif
#include <fstream>
#include <sstream>
#include "shared/utils/BitmapCache.h"
#include "utils/CurlManager.h"
#include "render/SequencePackage.h"
#include "utils/AppCallbacks.h"
#ifdef __APPLE__
#include "osxUtils/XLMetricKit.h"
#endif
#include <SpecialOptions.h>

#ifndef __WXMSW__
#include "automation/automation.h"
#endif

#include <log.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/common.h"

#ifdef LINUX
#include <GL/glut.h>
#endif

#if defined(_MSC_VER) && !defined(XLIGHTS_CMAKE_BUILD)
#ifdef _DEBUG
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_core.lib")
    #pragma comment(lib, "wxscintillad.lib")
    #pragma comment(lib, "wxregexud.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "ud_xml.lib")
    #pragma comment(lib, "wxtiffd.lib")
    #pragma comment(lib, "wxjpegd.lib")
    #pragma comment(lib, "wxpngd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_gl.lib")
    #pragma comment(lib, "wxzlibd.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "ud_propgrid.lib")
    #pragma comment(lib, "wxexpatd.lib")
    #pragma comment(lib, "msvcprtd.lib")
    #pragma comment(lib, "liquidfund.lib")
    #pragma comment(lib, "libzstdd_static_VS.lib")
    #pragma comment(lib, "xlsxwriterd.lib")
    #pragma comment(lib, "wxwebpd.lib")
#else
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_net.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_core.lib")
    #pragma comment(lib, "wxscintilla.lib")
    #pragma comment(lib, "wxregexu.lib")
    #pragma comment(lib, "wxbase" WXWIDGETS_VERSION "u_xml.lib")
    #pragma comment(lib, "wxtiff.lib")
    #pragma comment(lib, "wxjpeg.lib")
    #pragma comment(lib, "wxpng.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_aui.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_gl.lib")
    #pragma comment(lib, "wxzlib.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_qa.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_html.lib")
    #pragma comment(lib, "wxmsw" WXWIDGETS_VERSION "u_propgrid.lib")
    #pragma comment(lib, "wxexpat.lib")
    #pragma comment(lib, "msvcprt.lib")
    #pragma comment(lib, "liquidfun.lib")
    #pragma comment(lib, "libzstd_static_VS.lib")
    #pragma comment(lib, "xlsxwriter.lib")
    #pragma comment(lib, "wxwebp.lib")
#endif
#pragma comment(lib, "libcurl.dll.a")
#pragma comment(lib, "z.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "WS2_32.Lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ImageHlp.Lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "z.lib")
#pragma comment(lib, "lua5.3.5-static.lib")
//#pragma comment(lib, "libwebp.lib")
//#pragma comment(lib, "libwebpdecoder.lib")
//#pragma comment(lib, "libwebpdemux.lib")
#endif

xLightsFrame* xLightsApp::__frame = nullptr;

#ifdef __WXOSX__
// Resolve the .app bundle path of the currently-running xLights process by
// walking up from the executable. Returns empty if the layout doesn't match
// the expected `<Bundle>.app/Contents/MacOS/<exe>` shape (e.g. unbundled dev
// run). Callers must check for empty before using the result.
wxString GetRunningAppBundlePath() {
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName fn(exePath);
    if (fn.GetDirCount() < 2) {
        spdlog::warn("Cannot resolve .app bundle: executable path too shallow: {}",
                     exePath.ToStdString());
        return wxEmptyString;
    }
    fn.RemoveLastDir(); // strip MacOS
    fn.RemoveLastDir(); // strip Contents
    wxString candidate = fn.GetPath();
    if (!candidate.EndsWith(".app")) {
        spdlog::warn("Cannot resolve .app bundle from executable path '{}' (got '{}')",
                     exePath.ToStdString(), candidate.ToStdString());
        return wxEmptyString;
    }
    return candidate;
}

// Spawn a new xLights instance using `open -n -a <bundle> [--args <file>]`.
// Uses the argv form of wxExecute to avoid shell quoting / escaping issues
// for paths containing spaces, quotes, or other shell metacharacters.
// Returns false if the bundle path cannot be resolved.
bool SpawnNewXLightsInstance(const wxString& fileToOpen) {
    wxString bundle = GetRunningAppBundlePath();
    if (bundle.IsEmpty()) {
        return false;
    }
    // Keep backing storage alive for the duration of the wxExecute call.
    std::string sOpen = "/usr/bin/open";
    std::string sN = "-n";
    std::string sA = "-a";
    std::string sBundle(bundle.ToUTF8().data());
    std::string sArgs = "--args";
    std::string sFile(fileToOpen.ToUTF8().data());

    long pid = 0;
    if (fileToOpen.IsEmpty()) {
        char* argv[] = { sOpen.data(), sN.data(), sA.data(), sBundle.data(), nullptr };
        spdlog::info("Spawning new xLights instance: open -n -a '{}'", sBundle);
        pid = wxExecute(argv, wxEXEC_ASYNC);
    } else {
        char* argv[] = { sOpen.data(), sN.data(), sA.data(), sBundle.data(),
                         sArgs.data(), sFile.data(), nullptr };
        spdlog::info("Spawning new xLights instance: open -n -a '{}' --args '{}'",
                     sBundle, sFile);
        pid = wxExecute(argv, wxEXEC_ASYNC);
    }
    // wxExecute async returns 0 on launch failure; non-zero is the child PID.
    if (pid == 0) {
        spdlog::warn("wxExecute failed to launch new xLights instance for '{}'",
                     sFile.empty() ? sBundle : sFile);
        return false;
    }
    return true;
}
#endif

void InitialiseLogging(bool fromMain)
{
    static bool loggingInitialised = false;

    if (!loggingInitialised) {
        // Stash the exe directory so SpecialOptions can find special.options
        // next to the binary even before the show folder is chosen.
        SpecialOptions::StashExeDir(
            wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().ToStdString());

        std::string const logFilePath = GetLogFilePath().string();


        // wxStandardPaths::Get().Get()

        auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, 1024 * 1024 * 20, 10);

        auto file_logger = std::make_shared<spdlog::logger>("xLights", rotating_file_sink);

        // Set up the default logger before calling SpecialOptions::GetOption,
        // which may use spdlog internally
        loggingInitialised = true;
        spdlog::initialize_logger(file_logger);
        spdlog::set_default_logger(file_logger);
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%n %l] %v");
        spdlog::flush_on(spdlog::level::info);

        auto render_logger = std::make_shared<spdlog::logger>("render", rotating_file_sink);
        auto curl_logger = std::make_shared<spdlog::logger>("curl", rotating_file_sink);
        auto opengl_logger = std::make_shared<spdlog::logger>("opengl", rotating_file_sink);
        auto job_logger = std::make_shared<spdlog::logger>("job", rotating_file_sink);
        auto work_logger = std::make_shared<spdlog::logger>("work", rotating_file_sink);
        render_logger->set_level(spdlog::level::from_str(SpecialOptions::GetOption("render_logger", "warn")));
        curl_logger->set_level(spdlog::level::from_str(SpecialOptions::GetOption("curl_logger", "info")));
        opengl_logger->set_level(spdlog::level::from_str(SpecialOptions::GetOption("opengl_logger", "info")));
        job_logger->set_level(spdlog::level::from_str(SpecialOptions::GetOption("job_logger", "info")));
        work_logger->set_level(spdlog::level::from_str(SpecialOptions::GetOption("work_logger", "info")));
        spdlog::register_logger(render_logger);
        spdlog::register_logger(curl_logger);
        spdlog::register_logger(opengl_logger);
        spdlog::register_logger(job_logger);
        spdlog::register_logger(work_logger);

        // wxOperatingSystemId os = wxGetOsVersion();
        // std::string osStr = DecodeOS(os);
        // std::string initFileName;

        wxDateTime now = wxDateTime::Now();
        int millis = wxGetUTCTimeMillis().GetLo() % 1000;
        wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth() + 1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);
        // spdlog::info("Start Time: {}.", ts);

        spdlog::info("Start Time: {}.", ts.ToStdString());
        spdlog::info("Current working directory {}.", wxGetCwd().ToStdString());
    }
}

void ApplyLoggingSpecialOptions()
{
    auto applyLevel = [](const std::string& name, const std::string& option, const std::string& defaultLevel) {
        std::string level = SpecialOptions::GetOption(option, defaultLevel);
        spdlog::get(name)->set_level(spdlog::level::from_str(level));
        spdlog::info("Logger '{}' level set to '{}'", name, level);
    };
    applyLevel("xLights", "xLights_logger", "info");
    applyLevel("render", "render_logger", "warn");
    applyLevel("curl",   "curl_logger",   "info");
    applyLevel("opengl", "opengl_logger", "info");
    applyLevel("job",    "job_logger",    "info");
    applyLevel("work",   "work_logger",   "info");

    if (SpecialOptions::GetOption("console_logger", "false") != "true") return;

    static bool consoleApplied = false;
    if (consoleApplied) return;
    consoleApplied = true;

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // Called from SetDir on the main thread before rendering is active, so
    // mutating sinks() here does not race with concurrent log calls.
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> logger) {
        logger->sinks().push_back(stdout_sink);
    });
}

std::string DecodeOS(wxOperatingSystemId o)
{
    switch (o)
    {
    case wxOS_UNKNOWN:
        return "Call get get operating system failed.";
    case wxOS_MAC_OS:
        return "Apple Mac OS 8 / 9 / X with Mac paths.";
    case wxOS_MAC_OSX_DARWIN:
        return "Apple OS X with Unix paths.";
    case wxOS_MAC:
        return "An Apple Mac of some type.";
    case wxOS_WINDOWS_NT:
        return "Windows NT family(XP / Vista / 7 / 8 / 10).";
    case wxOS_WINDOWS:
        return "A Windows system of some type.";
    case wxOS_UNIX_LINUX:
        return "Linux.";
    case wxOS_UNIX_FREEBSD:
        return "FreeBSD.";
    case wxOS_UNIX_OPENBSD:
        return "OpenBSD.";
    case wxOS_UNIX_NETBSD:
        return "NetBSD.";
    case wxOS_UNIX_SOLARIS:
        return "Solaris.";
    case wxOS_UNIX_AIX:
        return "AIX.";
    case wxOS_UNIX_HPUX:
        return "HP / UX.";
    case wxOS_UNIX:
        return "Some flavour of Unix.";
    default:
        break;
    }

    return "Unknown Operating System.";
}

void DumpConfig()
{
    std::string versionStr = "Version: " + xlights_version_string;
    if (IsFromAppStore()) {
        versionStr += " - App Store";
    }
    spdlog::info(versionStr);
    spdlog::info("Build Date: " + xlights_build_date);
    spdlog::info("WX Version: " + std::string(wxString( wxVERSION_STRING).c_str()));

    spdlog::info("Machine configuration:");
    spdlog::info("  Total memory: " + std::to_string(GetPhysicalMemorySizeMB()) + " MB");
    wxMemorySize s = wxGetFreeMemory();
    if (s != -1)
    {
#if wxUSE_LONGLONG
        wxString msg = wxString::Format(_T("  Free Memory: %" wxLongLongFmtSpec "d."), s);
#else
        wxString msg = wxString::Format(_T("  Free Memory: %ld."), s);
#endif
        spdlog::info(msg.ToStdString());
    }
    spdlog::info("  Current directory: " + std::string(wxGetCwd().c_str()));
    spdlog::info("  Machine name: " + std::string(wxGetHostName().c_str()));
    spdlog::info("  OS: " + std::string(wxGetOsDescription().c_str()));
    int verMaj = -1;
    int verMin = -1;
    wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
    spdlog::info("  OS: {} {}.{}", (const char*)DecodeOS(o).c_str(), verMaj, verMin);
#ifdef USE_GLES
    spdlog::info("  Graphics backend: ANGLE (OpenGL ES / Direct3D)");
#endif
    if (wxIsPlatform64Bit())
    {
        spdlog::info("      64 bit");
    }
    else
    {
        spdlog::info("      NOT 64 bit");
    }
    if (wxIsPlatformLittleEndian())
    {
        spdlog::info("      Little Endian");
    }
    else
    {
        spdlog::info("      Big Endian");
    }
    spdlog::info("  CPU Arch: {}", wxGetCpuArchitectureName().ToStdString());

#ifdef LINUX
    wxLinuxDistributionInfo l = wxGetLinuxDistributionInfo();
    spdlog::info("  " + std::string(l.Id.c_str()) \
        + " " + std::string(l.Release.c_str()) \
        + " " + std::string(l.CodeName.c_str()) \
        + " " + std::string(l.Description.c_str()));
#endif
}

#ifdef LINUX
    #include <X11/Xlib.h>
#endif // LINUX
//IMPLEMENT_APP(xLightsApp)
int main(int argc, char **argv)
{
    srand(time(nullptr));
    InitialiseLogging(true);
    // Dan/Chris ... if you get an exception here the most likely reason is the line
    // appender.A1.fileName= in the xlights.xxx.properties file
    // it seems to need to be a folder that exists ... ideally it would create it but it doesnt seem to
    // it needs to be:
    //     a folder the user can write to
    //     predictable ... as we want the HandleCrash function to be able to locate the file to include it in the crash
    spdlog::info("******* XLights main function executing.");

#ifdef LINUX
    XInitThreads();
    // Detect virtio-gpu (virgl) DRM device. Virgl's GLX implementation has rendering bugs
    // in VM environments (Parallels, QEMU/KVM) that cause multi-canvas GL apps to fail
    // after the first rendered frame. Mesa's software renderer (llvmpipe, activated via
    // LIBGL_ALWAYS_SOFTWARE=1) works correctly and is fast enough for xLights' 2D rendering.
    // VirtIO GPU: vendor 0x1af4 (Virtio), device 0x1050 (virtio-gpu)
    if (getenv("LIBGL_ALWAYS_SOFTWARE") == nullptr) {
        bool isVirtiogpu = false;
        for (int card = 0; card < 8 && !isVirtiogpu; card++) {
            char vendorPath[64];
            snprintf(vendorPath, sizeof(vendorPath), "/sys/class/drm/card%d/device/vendor", card);
            FILE* f = fopen(vendorPath, "r");
            if (f) {
                char vendor[16] = {};
                if (fgets(vendor, sizeof(vendor), f)) {
                    // VirtIO vendor ID is 0x1af4
                    isVirtiogpu = (strncmp(vendor, "0x1af4", 6) == 0);
                }
                fclose(f);
            }
        }
        if (isVirtiogpu) {
            setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
            spdlog::info("Linux: detected virtio-gpu (virgl), enabling Mesa software rendering");
        }
    }
    // GTK3 prefers the Wayland GDK backend when WAYLAND_DISPLAY is set, which causes
    // wxWidgets to use EGL for GL canvases. EGL surface creation fails in some virtual GPU
    // environments. Force the X11 GDK backend so wxWidgets uses GLX, unless the user has
    // already set GDK_BACKEND. XWayland is always available when DISPLAY is set.
    if (getenv("GDK_BACKEND") == nullptr && getenv("DISPLAY") != nullptr) {
        setenv("GDK_BACKEND", "x11", 1);
        spdlog::info("Linux: forcing GDK_BACKEND=x11 for GLX compatibility");
    }
#endif
    wxDISABLE_DEBUG_SUPPORT();

    curl_global_init(CURL_GLOBAL_DEFAULT);

    #ifndef __WXMSW__
    wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");
    wxString binName(argv[0]);

    if (binName.EndsWith("xlDo")) {
        //symlink xlDo to xLights
        return DoXLDoCommands(argc, argv);
    } else if (argc > 1) {
        std::string argv1 = &argv[1][1];
        if (argv1 == "xlDo") {
            return DoXLDoCommands(argc - 1, &argv[1]);
        }
    }
    #endif

    spdlog::info("Main: Starting wxWidgets ...");
    int rc =  wxEntry(argc, argv);
    spdlog::info("Main: wxWidgets exited with rc={}", rc);

    return rc;
}

#ifdef _MSC_VER
IMPLEMENT_APP(xLightsApp);
#else
wxIMPLEMENT_APP_NO_MAIN(xLightsApp);
#endif

xLightsApp::xLightsApp() :
    xLightsAppBaseClass("xLights")
{
}

wxString xLightsFrame::GetThreadStatusReport() {
    return jobPool.GetThreadStatus();
}
void xLightsFrame::PushTraceContext() {
    TraceLog::PushTraceContext();
}
void xLightsFrame::PopTraceContext() {
    TraceLog::PopTraceContext();
}

void xLightsFrame::AddTraceMessage(const std::string &trc) {
    TraceLog::AddTraceMessage(trc);
}

void xLightsFrame::ClearTraceMessages() {
    TraceLog::ClearTraceMessages();
}


#ifdef __WXOSX__
void xLightsApp::MacOpenFiles(const wxArrayString &fileNames) {
    if (fileNames.empty()) {
        return;
    }
    wxString fileName = fileNames[0];
    spdlog::info("******* MacOpenFiles: {}", fileName.ToStdString());
    ObtainAccessToURL(fileName);

    wxString showDir = wxPathOnly(fileName);
    while (showDir != "" && !FileExists(showDir + "/" + "xlights_rgbeffects.xml")) {
        auto old = showDir;
        showDir = wxPathOnly(showDir);
        if (showDir == old) showDir = "";
    }

    if (__frame) {
        xLightsFrame* frame = __frame;
        frame->CallAfter([showDir, fileName, frame] {

            if (fileName.EndsWith("xsqz") || fileName.EndsWith("zip")) {

                // If a sequence is already loaded in this instance, spawn a separate
                // xLights process for the package instead of replacing the current
                // show. Matches the Windows behavior where double-clicking an xsqz
                // launches a fresh instance with the package as its show folder.
                // Falls back to in-place handling if we can't resolve the bundle
                // (e.g. running unbundled).
                if (frame->IsSequenceLoaded() && SpawnNewXLightsInstance(fileName)) {
                    return;
                }

                SequencePackage xsqPkg(std::filesystem::path(fileName.ToStdString()),
                                       __frame->GetShowDirectory(), __frame->GetSeqXmlFileName().ToStdString(), &__frame->AllModels);

                if (xsqPkg.IsPkg()) {
                    xsqPkg.Extract();
                    xsqPkg.SetLeaveFiles(true);

                    // find the sequence file
                    const auto& xsqFile = xsqPkg.GetXsqFile();

                    // temporarily set the show folder
                    frame->SetReadOnlyMode(false);
                    xLightsApp::showDir = xsqPkg.GetTempShowFolder();
                    frame->SetDir(xLightsApp::showDir, false);

                    // save the folder and we will remove it when we shutdown
                    if (!cleanupDir.empty()) {
                        wxDir::Remove(cleanupDir, wxPATH_RMDIR_RECURSIVE);
                    }
                    cleanupDir = xsqPkg.GetTempDir();

                    // tell xlights not to allow saving ... at least as much as possible
                    frame->SetReadOnlyMode(true);

                    // open the sequence
                    const wxString file = wxString(xsqFile.string());
                    frame->OpenSequence(file, nullptr);
                } else {
                    spdlog::debug("Zip file did not contain sequence.");
                }
            } else {
                if (showDir != "" && showDir != frame->showDirectory) {
                    wxString nsd = showDir;
                    if (!ObtainAccessToURL(nsd)) {
                        wxDirDialog dlg(frame, "Select Show Directory", nsd,  wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
                        if (dlg.ShowModal() == wxID_OK) {
                            nsd = dlg.GetPath();
                        }
                        if (!ObtainAccessToURL(nsd)) {
                            return;
                        }
                    }
                    frame->SetDir(nsd, false);
                }
                frame->OpenSequence(fileName, nullptr);
            }
        });
    } else {
        spdlog::info("       No xLightsFrame");
    }
}
#endif

// --headless has no xLightsFrame/top-level window at all, so whatever the
// desktop's normal startup does to keep wx's font mapper from ever popping
// its "unknown encoding" dialog never runs. Without a human to click Yes/No,
// that dialog blocks the process forever — observed hanging for hours on
// real show sequences with an unusual Text/Shape font encoding. Forcing
// interactive=false makes wx fall back silently (that encoding's text may
// not render pixel-perfect) instead of blocking headless runs.
class HeadlessNonInteractiveFontMapper : public wxFontMapper {
public:
    bool GetAltForEncoding(wxFontEncoding encoding, wxNativeEncodingInfo* info,
                           const wxString& facename = wxEmptyString,
                           bool /*interactive*/ = true) override {
        return wxFontMapper::GetAltForEncoding(encoding, info, facename, false);
    }
    wxFontEncoding CharsetToEncoding(const wxString& charset,
                                     bool /*interactive*/ = true) override {
        return wxFontMapper::CharsetToEncoding(charset, false);
    }
};

bool xLightsApp::OnInit()
{
#ifdef __APPLE__
    // A headless render must not pop a Dock icon or steal focus. The app bundle
    // launches as a foreground (Regular) app, so demote to a background app as
    // early as possible — before the rest of startup — when --headless is on the
    // command line. (Re-asserted in the --headless branch below.)
    for (int i = 1; i < argc; ++i) {
        const wxString a(argv[i]);
        if (a == "--headless" || a == "-hl") {
            SetHeadlessNoDock();
            break;
        }
    }
#endif
    SetMainThreadId();
    InitialiseLogging(false);

    AppCallbacks::SetPostToMainThread([](std::function<void()> fn) {
        wxTheApp->CallAfter(std::move(fn));
    });
    AppCallbacks::SetHandleUnhandledException([] {
        wxTheApp->OnUnhandledException();
    });
    AppCallbacks::SetSetupThreadCrashHandler([] {
        xlCrashHandler::SetupCrashHandlerForNonWxThread();
    });

#ifdef __APPLE__
    // MetricKit (macOS 12+): payloads land in {logfile parent}/Diagnostics/
    // and AddDebugFilesToReport sweeps them into the next crash zip.
    StartMetricKitCollection(
        (GetLogFilePath().parent_path() / "Diagnostics").string());
#endif

    spdlog::info("******* OnInit: XLights started.");
#ifdef __WXGTK__
    // On Linux (GTK), wxWidgets 3.3 defaults to EGL for GL canvases (even on X11).
    // EGL surface creation fails in virtual GPU environments (virgl/Parallels, QEMU).
    // Force GLX, which works reliably over XWayland and native X11. GTK is forced to
    // the X11 backend via GDK_BACKEND=x11 in main() so wxWidgets won't override this
    // back to EGL for Wayland sessions.
    wxGLCanvasUnix::PreferGLX();
#endif
#if wxUSE_GLCANVAS_EGL
    // this is only needed if using the EGL canvas as it's necessary to initialize the
    // GL attributes and pixel formats.  Likely a bug in the EGL implementation,
    // but not really sure.   In anycase, it's not needed on any of our "normal"
    // platforms.  It's only needed if building on an EGL based Linux distribution.
    wxGLApp::OnInit();
#endif

    wxTheApp->SetAppName("xLights");
    SetIsxLights(true);
    GetResourcesDirectory(); // bootstrap GetResourcesDir() with wx-dependent path lookup
    InitializeXLightsConfig();
    DumpConfig();

    // Point VAMP_PATH at the per-user plugin dir + standard location before any
    // Vamp plugin is loaded (xLights doesn't bundle the GPL/AGPL plugin pack).
    ConfigureVampPath();

#ifdef __WXMSW__
    if (!IsSuppressDarkMode()) {
        MSWEnableDarkMode();
    }
#endif

    // Stash the remembered show folder so show-folder special.options is applied
    // before the frame is constructed. InitialiseLogging() only knew the exe folder;
    // this re-applies logger levels now that the show folder is known.
    {
        wxString lastDir;
        if (GetXLightsConfig()->Read("LastDir", &lastDir) && !lastDir.IsEmpty()) {
            SpecialOptions::StashShowDir(lastDir.ToStdString());
            SpecialOptions::GetOption("", ""); // reset cache to pick up show folder
            ApplyLoggingSpecialOptions();
        }
    }

    int id = (int)wxThread::GetCurrentId();
    spdlog::info("Main thread id: 0x{:x} or {}", id, id);

#ifdef _MSC_VER
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

//check for options on command line: -DJ
//TODO: maybe use wxCmdLineParser instead?
//do this before instantiating xLightsFrame so it can use info gathered here
    wxString info;

    static const wxCmdLineEntryDesc cmdLineDesc [] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "r", "render", "render files and exit"},
        { wxCMD_LINE_SWITCH, "hl", "headless", "render sequence(s) to fseq with no window and exit" },
        { wxCMD_LINE_SWITCH, "fc", "fseqcmp", "compare two .fseq files channel-for-channel and exit (0=identical)" },
        { wxCMD_LINE_SWITCH, "st", "shadertranslate", "assemble every .fs shader in the show dir to GLSL (spike) and exit" },
        { wxCMD_LINE_SWITCH, "cs", "checksequence", "run check sequence and exit" },
        { wxCMD_LINE_OPTION, "m", "media", "specify media directory"},
        { wxCMD_LINE_OPTION, "s", "show", "specify show directory" },
        { wxCMD_LINE_OPTION, "od", "outputdir", "output dir for rendered fseq files (-r / --headless); default: show's configured fseq folder" },
        { wxCMD_LINE_SWITCH, "w", "wipe", "wipe settings clean" },
        { wxCMD_LINE_SWITCH, "o", "on", "turn on output to lights" },
        { wxCMD_LINE_SWITCH, "a", "aport", "turn on xFade A port" },
        { wxCMD_LINE_SWITCH, "b", "bport", "turn on xFade B port" },
#ifdef __LINUX__
        { wxCMD_LINE_SWITCH, "x", "xschedule", "run xschedule" },
        { wxCMD_LINE_SWITCH, "xs", "xsmsdaemon", "run xsmsdaemon" },
        { wxCMD_LINE_SWITCH, "c", "xcapture", "run xcapture" },
        { wxCMD_LINE_SWITCH, "f", "xfade", "run xfade" },
        { wxCMD_LINE_SWITCH, "n", "xscanner", "run xscanner" },
#endif
        { wxCMD_LINE_PARAM, "", "", "sequence file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},
        { wxCMD_LINE_NONE }
    };

// Add option to run xutils via xlights on linux (for AppImage usage)
#ifdef __LINUX__
       int run_xsmsdaemon = FALSE;
       int run_xschedule = FALSE;
       int run_xcapture = FALSE;
       int run_xfade = FALSE;
       int run_xscanner = FALSE;
       wxFileName f(wxStandardPaths::Get().GetExecutablePath());
       wxString appPath(f.GetPath());
       wxString cmdlineC(appPath + wxT("/xCapture"));
       wxString cmdlineS(appPath + wxT("/xSchedule"));
       wxString cmdlineF(appPath + wxT("/xFade"));
       wxString cmdlineM(appPath + wxT("/xSMSDaemon"));
       wxString cmdlineN(appPath + wxT("/xScanner"));
        for (int i=1; i< argc;i++) {
            if ((strncmp(argv[i].c_str(), "-xs", 3) == 0 && argv[i].size() == 3)|| strncmp(argv[i].c_str(), "-xsmsdaemon", 11) == 0) {
                run_xsmsdaemon = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-x", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xschedule", 10) == 0) {
                run_xschedule = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-c", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xcapture", 9) == 0) {
                run_xcapture = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-f", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xfade", 6) == 0) {
                run_xfade = TRUE;
            } else if ((strncmp(argv[i].c_str(), "-n", 2) == 0 && argv[i].size() == 2) || strncmp(argv[i].c_str(), "-xscanner", 9) == 0) {
                run_xscanner = TRUE;
            } else {
                cmdlineS += wxT(" ");
                cmdlineS += wxString::FromUTF8(argv[i]);
                cmdlineC += wxT(" ");
                cmdlineC += wxString::FromUTF8(argv[i]);
                cmdlineF += wxT(" ");
                cmdlineF += wxString::FromUTF8(argv[i]);
                cmdlineM += wxT(" ");
                cmdlineM += wxString::FromUTF8(argv[i]);
                cmdlineN += wxT(" ");
                cmdlineN += wxString::FromUTF8(argv[i]);
            }
        }
        if (run_xschedule) {
            wxExecute(cmdlineS, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xcapture) {
            wxExecute(cmdlineC, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xfade) {
            wxExecute(cmdlineF, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xsmsdaemon) {
            wxExecute(cmdlineM, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        } else if (run_xscanner) {
            wxExecute(cmdlineN, wxEXEC_BLOCK,NULL,NULL);
            exit(0);
        }
       // Set App Name for when running via appimage
       SetAppName(wxT("xLights"));
#endif

    int ab = 0;
    std::string readOnlyZipFile = "";

    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    switch (parser.Parse()) {
    case -1:
        // help was given
        return false;
    case 0:
        if (parser.Found("w"))
        {
            spdlog::info("-w: Wiping settings");
            info += _("Wiping settings\n");
            WipeSettings();
        }
        if (parser.Found("s", &showDir)) {
            spdlog::info("-s: Show directory set to {}.", (const char*)showDir.c_str());
            wxString lastDir;
            GetXLightsConfig()->Read("LastDir", &lastDir);
            if (lastDir != showDir) {
                info += _("Setting show directory to ") + showDir + "\n";
                // re-apply logging with the command-line show dir overriding LastDir
                SpecialOptions::StashShowDir(showDir.ToStdString());
                SpecialOptions::GetOption("", "");
                ApplyLoggingSpecialOptions();
            }
        }

        if (parser.Found("a")) {
            spdlog::info("-a: A port enabled.");
            ab = 1;
        } else if (parser.Found("b")) {
            spdlog::info("-b: B port enabled.");
            ab = 2;
        }

        if (parser.Found("m", &mediaDir)) {
            spdlog::info("-m: Media directory set to {}.", (const char*)mediaDir.c_str());
            info += _("Setting media directory to ") + mediaDir + "\n";
        } else if (!showDir.IsNull()) {
            mediaDir = showDir;
        }
        // Expand any glob patterns (containing '*' or '?') in the positional
        // parameters. On macOS/Linux the shell usually expands these before we
        // ever see them, but quoted patterns - and Windows, whose shells don't
        // glob - arrive here literally. This lets "xLights --headless *.xsq" (or
        // a quoted "*.xsq") render every matching sequence in one run. Mirrors
        // the -r glob handling. Scoped in braces so the switch's other case
        // labels can jump past the paramFiles construction.
        {
        wxArrayString paramFiles;
        for (size_t x = 0; x < parser.GetParamCount(); x++) {
            wxString param = parser.GetParam(x);
            if (param.Contains("*") || param.Contains("?")) {
                wxFileName patternFn(param);
                wxString patternDir = patternFn.GetPath();
                if (patternDir.IsEmpty()) {
                    patternDir = wxGetCwd();
                }
                // Glob runs before the show dir's sandbox access is obtained, so
                // wxDir enumeration would otherwise fail on macOS. Activate the
                // security-scoped bookmark for the pattern dir first.
                ObtainAccessToURL(patternDir.ToStdString(), false);
                wxArrayString matches;
                GetAllFilesInDir(patternDir, matches, patternFn.GetFullName());
                if (matches.IsEmpty()) {
                    spdlog::warn("No files matched command line pattern: {}.", (const char*)param.c_str());
                    info += _("No files matched pattern ") + param + "\n";
                } else {
                    matches.Sort();
                    for (const auto& m : matches) {
                        paramFiles.push_back(m);
                    }
                }
            } else {
                paramFiles.push_back(param);
            }
        }
        for (size_t x = 0; x < paramFiles.GetCount(); x++) {
            wxString sequenceFile = paramFiles[x];
            if (sequenceFile.Lower().EndsWith(".zip") || sequenceFile.Lower().EndsWith(".xsqz")) {
                spdlog::info("Sequence zip file passed on command line: {}.", (const char*)sequenceFile.c_str());
                info += _("Loading read only sequence ") + sequenceFile + "\n";
                readOnlyZipFile = sequenceFile;
			} else {
                if (sequenceFiles.Count() == 0) {
                    spdlog::info("Sequence file passed on command line: {}.", (const char*)sequenceFile.c_str());
                    info += _("Loading sequence ") + sequenceFile + "\n";
                }
                if (showDir.IsNull()) {
                    showDir = wxPathOnly(sequenceFile);
                    while (showDir != "" && !FileExists(showDir + "/" + "xlights_rgbeffects.xml")) {
                        wxString old = showDir;
                        showDir = wxPathOnly(showDir);
                        if (showDir == old)
                            showDir = "";
                    }
                }
                sequenceFiles.push_back(sequenceFile);
            }
        }
        } // end paramFiles scope

        if (readOnlyZipFile != "" && sequenceFiles.Count() > 0) {
            // illegal combination of files
            info += _("Illegal combination of files. Load of zip file will take priority.\n");
            sequenceFiles.Clear();
        }

        if (!parser.Found("cs") && !parser.Found("r") && !parser.Found("o") && !parser.Found("hl") && !parser.Found("fc") && !parser.Found("st") && !info.empty() && readOnlyZipFile == "")
        {
            wxMessageBox(info, "Information", wxICON_INFORMATION | wxOK); // pre-frame: callback not yet registered
        }
        break;
    default:
        wxMessageBox(_("Unrecognized command line parameters"), "Error", wxICON_ERROR | wxOK); // pre-frame: callback not yet registered
        return false;
    }

    bool renderOnlyMode = false;
    if (readOnlyZipFile == "" &&  parser.Found("r")) {
        spdlog::info("-r: Render mode is ON");
        renderOnlyMode = true;
    }

    wxFileName xsqFile;

    // opening a zip file with hopefully a packaged sequence in it and then disabling all saving
    if (readOnlyZipFile != "") {
        // this is not allowed
        renderOnlyMode = false;

        SequencePackage xsqPkg(std::filesystem::path(readOnlyZipFile), "", "", nullptr);

        if (xsqPkg.IsPkg()) {
            xsqPkg.Extract();
            xsqPkg.SetLeaveFiles(true);

            // find the sequence file
            xsqFile = wxFileName(xsqPkg.GetXsqFile().string());

            // temporarily set the show folder
            showDir = xsqPkg.GetTempShowFolder();

            spdlog::info("xsqz IsPkg={} IsValid={} tempShowFolder='{}' tempDir='{}' xsqFile='{}'",
                xsqPkg.IsPkg(), xsqPkg.IsValid(),
                showDir.ToStdString(),
                xsqPkg.GetTempDir(),
                xsqFile.GetFullPath().ToStdString());

            // if the package includes xlights_rgbeffects.xml, point the frame constructor
            // at the extracted show folder so it loads it directly (avoids a second SetDir)
            if (!showDir.IsEmpty()) {
                xLightsApp::showDir = showDir;
            }

            // save the temp dir root so it gets cleaned up on shutdown
            cleanupDir = xsqPkg.GetTempDir();

        } else {
            spdlog::debug("Zip file did not contain sequence.");
            readOnlyZipFile = "";
        }
    }

    // ---- shader-translation spike: run every .fs in the show dir through the
    // real ParseShaderFromSource/ShaderConfig assembly and dump the resulting
    // GLSL to <showdir>/_xlate/<name>.frag, so an external glslang+SPIRV-Cross
    // pass can measure how much of the corpus translates to SPIR-V/MSL. No window.
    if (parser.Found("st")) {
        std::string dir = showDir.ToStdString();
        ObtainAccessToURL(dir, true);
        std::string outDir = dir + "/_xlate";
        std::error_code ec;
        std::filesystem::create_directories(outDir, ec);
        wxArrayString files;
        GetAllFilesInDir(showDir, files, "*.fs");
        files.Sort();
        int total = 0, assembled = 0, parseFail = 0, nativeOk = 0, nativeFail = 0;
        for (const auto& fpath : files) {
            total++;
            std::ifstream in(fpath.ToStdString(), std::ios::binary);
            std::stringstream ss;
            ss << in.rdbuf();
            const std::string src = ss.str();
            const std::string base = wxFileName(fpath).GetName().ToStdString();
            ShaderConfig* cfg = ShaderEffect::ParseShaderFromSource(fpath.ToStdString(), src, nullptr);
            if (cfg == nullptr) {
                parseFail++;
                printf("PARSEFAIL\t%s\n", base.c_str());
                continue;
            }
            std::ofstream out(outDir + "/" + base + ".frag", std::ios::binary);
            out << cfg->GetCode();
            out.close();
            assembled++;
#ifdef __APPLE__
            std::string terr;
            if (ShaderTranslate::ValidateRenderPipeline(ShaderEffect::GetNativeVertexShaderSource(), cfg->GetCode(), terr)) {
                nativeOk++;
            } else {
                nativeFail++;
                printf("NATIVEFAIL\t%s\t%s\n", base.c_str(), terr.substr(0, 160).c_str());
            }
#endif
            delete cfg;
        }
        printf("shadertranslate: %d .fs found, %d assembled -> %s, %d parse-fail\n",
               total, assembled, outDir.c_str(), parseFail);
        printf("native VS+FS -> MTLRenderPipelineState: %d ok, %d fail (of %d assembled)\n",
               nativeOk, nativeFail, assembled);
        std::exit(0);
    }

    // ---- fseq comparison utility: decode two .fseq files to full-width
    // per-channel frames (robust to sparse vs non-sparse layout) and report
    // any differing channel samples. Used to prove headless renders match the
    // desktop render. No window.
    if (parser.Found("fc")) {
#ifdef __APPLE__
        // Run as a background app: no Dock icon, no focus stealing, so a
        // background render doesn't disrupt the desktop.
        SetHeadlessNoDock();
#endif
        if (sequenceFiles.GetCount() != 2) {
            printf("--fseqcmp requires exactly two .fseq file arguments (got %u)\n",
                   (unsigned)sequenceFiles.GetCount());
            std::exit(2);
        }
        const std::string pa = sequenceFiles[0].ToStdString();
        const std::string pb = sequenceFiles[1].ToStdString();
        // Restore security-scoped access (sandboxed builds) before the raw
        // fopen inside openFSEQFile.
        ObtainAccessToURL(pa, false);
        ObtainAccessToURL(pb, false);
        std::unique_ptr<FSEQFile> fa(FSEQFile::openFSEQFile(pa));
        std::unique_ptr<FSEQFile> fb(FSEQFile::openFSEQFile(pb));
        if (!fa || !fb) {
            printf("--fseqcmp: could not open %s\n", (!fa ? pa.c_str() : pb.c_str()));
            std::exit(2);
        }
        const uint32_t maxCh = std::max((uint32_t)fa->getMaxChannel(), (uint32_t)fb->getMaxChannel());
        const uint32_t frames = (uint32_t)std::min(fa->getNumFrames(), fb->getNumFrames());
        printf("A %s: %llu frames, maxCh %u, step %d\n", pa.c_str(),
               (unsigned long long)fa->getNumFrames(), (unsigned)fa->getMaxChannel(), fa->getStepTime());
        printf("B %s: %llu frames, maxCh %u, step %d\n", pb.c_str(),
               (unsigned long long)fb->getNumFrames(), (unsigned)fb->getMaxChannel(), fb->getStepTime());
        std::vector<std::pair<uint32_t, uint32_t>> rng{ { 0, maxCh } };
        fa->prepareRead(rng);
        fb->prepareRead(rng);
        std::vector<uint8_t> ba(maxCh, 0), bb(maxCh, 0);
        std::vector<uint32_t> chDiffFrames(maxCh, 0); // per-channel: # frames that differ
        std::vector<uint8_t> chMaxDiff(maxCh, 0);     // per-channel: max |A-B|
        uint64_t diffSamples = 0, diff1 = 0, diffSmall = 0, diffBig = 0;
        int maxAbsDiff = 0;
        long firstFrame = -1;
        uint32_t firstCh = 0;
        // Optional: dump A vs B for one channel across frames (env, 1-based).
        const char* dumpEnv = getenv("XL_FSEQCMP_DUMPCH");
        const uint32_t dumpCh = dumpEnv ? (uint32_t)strtoul(dumpEnv, nullptr, 10) : 0;
        std::vector<std::pair<uint8_t, uint8_t>> series;
        if (dumpCh != 0 && dumpCh <= maxCh) series.reserve(frames);
        for (uint32_t fr = 0; fr < frames; ++fr) {
            std::fill(ba.begin(), ba.end(), 0);
            std::fill(bb.begin(), bb.end(), 0);
            FSEQFile::FrameData* da = fa->getFrame(fr);
            FSEQFile::FrameData* db = fb->getFrame(fr);
            if (da) { da->readFrame(ba.data(), maxCh); delete da; }
            if (db) { db->readFrame(bb.data(), maxCh); delete db; }
            for (uint32_t c = 0; c < maxCh; ++c) {
                if (ba[c] != bb[c]) {
                    ++diffSamples;
                    int d = std::abs((int)ba[c] - (int)bb[c]);
                    if (d > maxAbsDiff) maxAbsDiff = d;
                    if (d == 1) ++diff1;
                    else if (d <= 4) ++diffSmall;
                    else ++diffBig;
                    if (firstFrame < 0) { firstFrame = (long)fr; firstCh = c; }
                    ++chDiffFrames[c];
                    if (d > chMaxDiff[c]) chMaxDiff[c] = (uint8_t)std::min(d, 255);
                }
            }
            if (dumpCh != 0 && dumpCh <= maxCh) series.emplace_back(ba[dumpCh - 1], bb[dumpCh - 1]);
        }
        if (dumpCh != 0 && !series.empty()) {
            printf("\nchannel %u  A vs B (first 80 frames; * = differ):\n", dumpCh);
            for (uint32_t fr = 0; fr < series.size() && fr < 80; ++fr) {
                printf("  fr %3u  A=%3u  B=%3u  %s\n", fr, series[fr].first, series[fr].second,
                       series[fr].first != series[fr].second ? "*" : "");
            }
        }
        if (diffSamples == 0) {
            printf("IDENTICAL: %u frames x %u channels match exactly\n", frames, maxCh);
        } else {
            const uint64_t total = (uint64_t)frames * maxCh;
            printf("DIFFER: %llu / %llu samples (%.2f%%); first at frame %ld channel %u\n",
                   (unsigned long long)diffSamples, (unsigned long long)total,
                   100.0 * (double)diffSamples / (double)total, firstFrame, firstCh);
            printf("  magnitude: maxAbsDiff=%d  |  d==1: %llu  d<=4: %llu  d>4: %llu\n",
                   maxAbsDiff, (unsigned long long)diff1,
                   (unsigned long long)diffSmall, (unsigned long long)diffBig);

            uint32_t diffChans = 0;
            for (uint32_t c = 0; c < maxCh; ++c) if (chDiffFrames[c]) ++diffChans;
            printf("  channels touched: %u / %u\n", diffChans, maxCh);

            // Map differing channels back to models (needs the show for channel
            // assignment). maxΔ>4 tends to be video (independent decode); ≤4 is
            // GPU-context float noise from shaders/Metal. Group rows overlap real
            // models so they're skipped (each channel belongs to one top-model).
            if (!showDir.IsEmpty()) {
                ObtainAccessToURL(showDir.ToStdString(), false);
                HeadlessRenderContext mapCtx;
                std::list<std::string> mf;
                if (mapCtx.LoadShowFolder(showDir.ToStdString(), mf)) {
                    struct MS { std::string name; uint32_t s1; uint32_t e1; uint64_t dch; int mx; };
                    std::vector<MS> ms;
                    // Track union of channels owned by some model. Buffer/overlay
                    // models (ImportBuffer, HiddenRight, ...) can share channels, so
                    // per-model dch sums may exceed the distinct channel count.
                    std::vector<bool> owned(maxCh, false);
                    for (auto it = mapCtx.AllModels.begin(); it != mapCtx.AllModels.end(); ++it) {
                        Model* m = it->second;
                        if (m == nullptr || m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
                        const uint32_t f = m->GetFirstChannel();      // 0-based
                        const uint32_t cc = m->GetChanCount();
                        if (cc == 0) continue;
                        const uint32_t last = (uint32_t)std::min((uint64_t)f + cc, (uint64_t)maxCh);
                        uint64_t dch = 0;
                        int mx = 0;
                        for (uint32_t c = f; c < last; ++c) {
                            if (chDiffFrames[c]) { ++dch; if (chMaxDiff[c] > mx) mx = chMaxDiff[c]; owned[c] = true; }
                        }
                        if (dch > 0) { ms.push_back({ it->first, f + 1, f + cc, dch, mx }); }
                    }
                    std::sort(ms.begin(), ms.end(), [](const MS& a, const MS& b) {
                        if (a.mx != b.mx) return a.mx > b.mx;
                        return a.dch > b.dch;
                    });
                    printf("\nPer-model differences (worst first):\n");
                    printf("  %-5s  %-13s  %-19s  %s\n", "maxD", "diffCh/totCh", "channels(1-based)", "model");
                    for (auto& e : ms) {
                        char chc[48];
                        snprintf(chc, sizeof(chc), "%u-%u", e.s1, e.e1);
                        char dcc[32];
                        snprintf(dcc, sizeof(dcc), "%llu/%llu",
                                 (unsigned long long)e.dch, (unsigned long long)(e.e1 - e.s1 + 1));
                        printf("  %-5d  %-13s  %-19s  %s\n", e.mx, dcc, chc, e.name.c_str());
                    }
                    uint64_t leftover = 0;
                    for (uint32_t c = 0; c < maxCh; ++c) if (chDiffFrames[c] && !owned[c]) ++leftover;
                    if (leftover > 0) {
                        printf("  (%llu differing channels not owned by any model)\n",
                               (unsigned long long)leftover);
                    }
                } else {
                    printf("  (could not load show '%s' for model mapping)\n", showDir.ToStdString().c_str());
                }
            } else {
                printf("  (pass -s <showdir> to map differing channels to model names)\n");
            }

            // Frame-offset probe: shift A by k frames and see if the diff drops.
            // A sharp minimum at k!=0 = a systematic frame offset (a real bug);
            // min at 0 = the content genuinely differs per frame (video/shader).
            printf("\nframe-offset probe (subsampled mean |Δ| per frame):\n");
            {
                const uint32_t step = std::max<uint32_t>(1, frames / 200);
                std::vector<uint8_t> sa(maxCh, 0), sb(maxCh, 0);
                int bestShift = 0;
                double bestAvg = -1;
                for (int shift = -3; shift <= 3; ++shift) {
                    double sum = 0;
                    uint32_t n = 0;
                    for (uint32_t fr = 0; fr < frames; fr += step) {
                        const long fa2 = (long)fr + shift;
                        if (fa2 < 0 || fa2 >= (long)frames) continue;
                        std::fill(sa.begin(), sa.end(), 0);
                        std::fill(sb.begin(), sb.end(), 0);
                        FSEQFile::FrameData* da2 = fa->getFrame((uint32_t)fa2);
                        FSEQFile::FrameData* db2 = fb->getFrame(fr);
                        if (da2) { da2->readFrame(sa.data(), maxCh); delete da2; }
                        if (db2) { db2->readFrame(sb.data(), maxCh); delete db2; }
                        uint64_t s = 0;
                        for (uint32_t c = 0; c < maxCh; ++c) s += std::abs((int)sa[c] - (int)sb[c]);
                        sum += (double)s;
                        ++n;
                    }
                    const double avg = n ? sum / n : 0;
                    printf("  A shift %+d: %.0f%s\n", shift, avg, shift == 0 ? "   (aligned)" : "");
                    if (bestAvg < 0 || avg < bestAvg) { bestAvg = avg; bestShift = shift; }
                }
                if (bestShift != 0) {
                    printf("  --> min at shift %+d: looks like a %d-frame offset between the renders\n",
                           bestShift, bestShift);
                } else {
                    printf("  --> min at shift 0: content genuinely differs per frame (not an offset)\n");
                }
            }
        }
        std::exit(diffSamples == 0 ? 0 : 1);
    }

    // ---- Headless render: no window at all. Load the show + sequence(s),
    // render to fseq, and exit. Proof that xLightsShowContext carries a full
    // render without an xLightsFrame. See HeadlessRenderContext / AGENTS.md.
    if (parser.Found("hl")) {
        if (showDir.IsEmpty() || sequenceFiles.IsEmpty()) {
            spdlog::error("--headless requires a show directory (-s <dir>) and at least one sequence file");
            return false;
        }
#ifdef __APPLE__
        // Run as a background app: no Dock icon, no focus stealing, so a
        // background render doesn't disrupt the desktop.
        SetHeadlessNoDock();
#endif
        wxInitAllImageHandlers();
        // No window means no human to click through wx's "unknown encoding"
        // font dialog; force it to fail silently instead of blocking forever.
        wxFontMapperBase::Set(new HeadlessNonInteractiveFontMapper());
        // Initialize the offscreen GL context manager so ShaderEffect renders
        // (it has no CPU fallback — without this it fills solid colour). On
        // macOS this is self-contained CGL: no window, no worker thread, no
        // event loop. Empty params match the desktop frame (USE_GLES is off on
        // desktop). Metal-compute + GPURenderUtils self-initialize via static
        // ctors, so nothing else is needed for GPU-accelerated effects.
        if (getenv("XL_HEADLESS_NO_GL") == nullptr) {
            GLContextManager::InitParams glParams;
            GLContextManager::Instance().Initialize(glParams);
        } else {
            spdlog::warn("--headless: XL_HEADLESS_NO_GL set — skipping GL init (shaders will fall back to solid colour)");
        }
        // Register the text backend so Text/Shape effects render (the wx backend
        // draws to offscreen bitmaps and is thread-safe on macOS/Windows).
        // Mirrors xLightsFrame's registration.
        TextDrawingContext::RegisterFactory(wxTextDrawingContext::Create,
                                            wxTextDrawingContext::ParseTextFont,
                                            wxTextDrawingContext::ParseShapeFont);
        TextDrawingContext::Initialize();

        // Video decode backend: mirror the desktop's hardware-accel preference.
        // The flag defaults false, so without this headless picks FFmpeg while the
        // desktop picks AVFoundation (macOS) for the same file — different decoders
        // yield different frames and a large fseq divergence on video effects.
        {
            auto* cfg = GetXLightsConfig();
#ifdef __APPLE__
            bool hwVideo = true;
            cfg->Read("xLightsVideoReaderAccelerated", &hwVideo, true);
            VideoReader::SetHardwareAcceleratedVideo(hwVideo);
            VideoReader::InitHWAcceleration();
#else
            bool hwVideo = false;
            int hwRenderer = 1;
            cfg->Read("xLightsVideoReaderAccelerated", &hwVideo, false);
            cfg->Read("xLightsVideoReaderRenderer", &hwRenderer, 1);
            VideoReader::SetHardwareAcceleratedVideo(hwVideo);
            VideoReader::SetHardwareRenderType(hwRenderer);
#endif
        }

        // GPU compute: mirror the desktop's preference (xLightsMain.cpp reads
        // xLightsGPURendering into GPURenderUtils::SetEnabled). XL_NO_GPU_COMPUTE=1
        // overrides for CPU-only A/B determinism testing.
        {
            bool gpuRendering = true;
            GetXLightsConfig()->Read("xLightsGPURendering", &gpuRendering, true);
            if (getenv("XL_NO_GPU_COMPUTE") != nullptr) {
                gpuRendering = false;
                spdlog::warn("--headless: XL_NO_GPU_COMPUTE set — GPU compute disabled");
            }
            GPURenderUtils::SetEnabled(gpuRendering);
        }

        std::list<std::string> mediaFolders;
        if (!mediaDir.IsEmpty() && mediaDir != showDir) {
            mediaFolders.push_back(mediaDir.ToStdString());
        }

        bool allOk = true;
        {
            // Scoped so the context (render engine + job pool) tears down cleanly
            // before we exit the process.
            HeadlessRenderContext ctx;
            if (!ctx.LoadShowFolder(showDir.ToStdString(), mediaFolders)) {
                spdlog::error("--headless: failed to load show folder {}", showDir.ToStdString());
                std::exit(2);
            }

            // Where the fseqs go: --outputdir if given, else the show's configured
            // fseq folder (ctx read it from xlights_rgbeffects.xml, defaulting to
            // the show dir) — matching desktop -r instead of writing next to the
            // .xsq. Authorize it for the macOS sandbox before writing.
            wxString outputDir;
            parser.Found("od", &outputDir);
            const wxString outDir = outputDir.IsEmpty() ? wxString(ctx.GetFseqDirectory()) : outputDir;
            if (!outDir.IsEmpty()) {
                ObtainAccessToURL(outDir.ToStdString(), true);
            }

            for (const auto& seq : sequenceFiles) {
                const std::string seqPath = seq.ToStdString();
                spdlog::info("--headless: rendering {}", seqPath);
                const auto seqStart = std::chrono::steady_clock::now();
                if (!ctx.OpenSequence(seqPath)) { allOk = false; continue; }
                if (!ctx.RenderAndWait()) { allOk = false; }
                wxFileName outFn(seq);
                outFn.SetExt("fseq");
                if (!outDir.IsEmpty()) outFn.SetPath(outDir);
                const std::string outPath = outFn.GetFullPath().ToStdString();
                if (ctx.WriteFseq(outPath)) {
                    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - seqStart).count();
                    // Match the desktop batch-render timing line (TabSequence.cpp)
                    // and flush so per-file progress is visible during a batch.
                    printf("%s     Updated in %7.3f seconds\n", outPath.c_str(), elapsed);
                    fflush(stdout);
                    spdlog::info("--headless: wrote {} in {:.3f} seconds", outPath, elapsed);
                } else {
                    allOk = false;
                }
                ctx.CloseSequence();
            }
        }
        spdlog::info("--headless: done ({})", allOk ? "success" : "with errors");
        // No window, no event loop — exit with a status a script/agent can check.
        std::exit(allOk ? 0 : 1);
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    BitmapCache::SetupArtProvider();
    if (wxsOK)
    {
    	xLightsFrame* Frame = new xLightsFrame(nullptr, ab, -1, renderOnlyMode);
        if (Frame->CurrentDir == "") {
            spdlog::info("Show directory not set");
        }
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    xLightsFrame* const topFrame = (xLightsFrame*)GetTopWindow();
    __frame = topFrame;

    if (renderOnlyMode) {
        wxString outputDir;
        if (parser.Found("od", &outputDir) && !outputDir.IsEmpty()) {
            // Applied at the top of OpenRenderAndSaveSequencesF, after the show
            // (and its configured fseqDir) has finished loading.
            topFrame->SetCommandLineFseqDir(outputDir.ToStdString());
        }
        topFrame->CallAfter(&xLightsFrame::OpenRenderAndSaveSequencesF, sequenceFiles, xLightsFrame::RENDER_EXIT_ON_DONE);
    }

    if (readOnlyZipFile != "") {
        spdlog::info("xsqz: CurrentDir='{}' after construction", topFrame->CurrentDir.ToStdString());

        // tell xlights not to allow saving ... at least as much as possible
        topFrame->SetReadOnlyMode(true);

        // open the sequence
        const wxString file = xsqFile.GetFullPath();
        topFrame->CallAfter(&xLightsFrame::OpenSequence, file);
    }

    if (parser.Found("cs")) {
        spdlog::info("-v: Check sequence mode is ON");
        topFrame->_checkSequenceMode = true;
        topFrame->CallAfter(&xLightsFrame::OpenAndCheckSequence, sequenceFiles, true);
    }

    if (parser.Found("o")) {
        spdlog::info("-o: Turning on output to lights");
        // Turn on output to lights - ignore if another xLights/xSchedule is already outputting
        topFrame->EnableOutputs(true);
    }

    #ifdef LINUX
        glutInit(&(wxApp::argc), wxApp::argv);
    #endif

    spdlog::info("XLightsApp OnInit Done.");

    return wxsOK;
}

void xLightsApp::WipeSettings()
{
    spdlog::info("Wiping settings.");
    WipeXLightsConfig();
}

bool xLightsApp::ProcessIdle() {
    uint64_t now = wxGetLocalTimeMillis().GetValue();
    bool b = CurlManager::INSTANCE.processCurls();
    if (now > _nextIdleTime) {
        _nextIdleTime = now + 100;
        return wxApp::ProcessIdle() | b;
    }
    return b;
}

//global flags from command line:
wxString xLightsApp::mediaDir;
wxString xLightsApp::showDir;
wxString xLightsApp::cleanupDir;
wxArrayString xLightsApp::sequenceFiles;
