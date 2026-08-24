#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Captures live frames from a local webcam on a background thread and keeps
// only the most recent frame, so a UI can poll TryGetLatestFrame()
// indefinitely (live preview, live bulb detection) without memory growing
// over the session.
//
// Two backends behind one API: Media Foundation on Windows (in the .cpp),
// AVFoundation on Apple (via AppleLiveCameraBridge, so no ObjC leaks into
// src-core).
//
// There is no third backend, so nothing here is DEFINED on other platforms.
// Callers must gate on XLIGHTS_HAVE_LIVE_CAMERA and offer the feature only
// where it exists, rather than link against a stub that always says "no
// camera" - a greyed-out menu entry the user can't explain is worse than an
// absent one.

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

// Desktop only. src-core and macOS/src-apple-core are both linked into the
// iPad app, so without this the whole AVFoundation capture stack would
// compile and link into an app that has no UI for it - dead weight, and a
// camera code path in a target that never offers a camera.
#if defined(_WIN32) || (defined(__APPLE__) && !TARGET_OS_IPHONE)
#define XLIGHTS_HAVE_LIVE_CAMERA 1
#endif

#include <cstdint>
#include <string>
#include <vector>

struct LiveCameraDevice {
    std::string name;         // friendly name shown to the user
    std::string symbolicLink; // opaque device id used to (re)open it: a Media
                              // Foundation symbolic link on Windows, an
                              // AVCaptureDevice uniqueID on Apple
};

// Enumerates the video capture devices currently attached (webcams, capture
// cards, and on macOS 14+ Continuity Cameras). Safe to call repeatedly, e.g.
// each time a dialog populates its camera dropdown. Empty on platforms with
// no backend.
std::vector<LiveCameraDevice> EnumerateLiveCameras();

class LiveCameraCapture
{
public:
    // Opens the device and immediately starts capturing frames on a background
    // thread. The constructor waits briefly for the device to open so IsValid()
    // is meaningful straight away.
    explicit LiveCameraCapture(const std::string& symbolicLink);
    ~LiveCameraCapture();

    [[nodiscard]] bool IsValid() const;

    // Stops the background capture thread. Safe to call more than once; the
    // destructor calls it too.
    void Stop();

    // Dark mode: stops the camera re-metering, so the ambient scene stays put
    // and lit pixels stand out against a fixed baseline. Passing false (or
    // stopping capture) restores the camera's previous/automatic settings.
    // Cameras whose drivers don't allow manual control are left unchanged.
    //
    // How far it can go is platform-limited. Windows pins a short manual
    // exposure with minimum gain and backlight compensation off (UVC gives
    // real control). Apple only locks exposure and white balance where they
    // sit: setExposureModeCustomWithDuration:ISO: is unavailable on macOS, so
    // there is no supported way to force a short exposure or floor the gain.
    // Expect less ambient rejection there in a brightly lit room.
    void SetDarkMode(bool enabled);

    // Non-destructive peek at the most recently captured frame (packed top-down
    // RGB24). Safe to call repeatedly while capture is running. Returns false if
    // no frame has arrived yet.
    [[nodiscard]] bool TryGetLatestFrame(std::vector<uint8_t>& outRgb, int& outWidth, int& outHeight) const;

private:
    struct Impl;
    Impl* _impl;
};
