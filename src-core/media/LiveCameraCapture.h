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

// Windows-only. Captures live frames from a local webcam via Media Foundation
// on a background thread and keeps only the most recent frame, so a UI can
// poll TryGetLatestFrame() indefinitely (live preview, live bulb detection)
// without memory growing over the session.

#ifdef _WIN32

#include <cstdint>
#include <string>
#include <vector>

struct LiveCameraDevice {
    std::string name;         // friendly name shown to the user
    std::string symbolicLink; // Media Foundation device symbolic link, used to (re)open it
};

// Enumerates the video capture devices Windows currently has attached (webcams,
// capture cards, etc). Safe to call repeatedly, e.g. each time a dialog populates
// its camera dropdown.
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

    // Dark mode: locks the camera to a short manual exposure with minimum gain
    // and backlight compensation off, so the ambient scene goes near-black and
    // lit pixels stand out with maximum contrast. Passing false (or stopping
    // capture) restores the camera's previous/automatic settings. Applied
    // asynchronously on the capture thread; takes effect within a frame or two.
    // Cameras whose drivers don't allow manual control are left unchanged.
    void SetDarkMode(bool enabled);

    // Non-destructive peek at the most recently captured frame (packed top-down
    // RGB24). Safe to call repeatedly while capture is running. Returns false if
    // no frame has arrived yet.
    [[nodiscard]] bool TryGetLatestFrame(std::vector<uint8_t>& outRgb, int& outWidth, int& outHeight) const;

private:
    struct Impl;
    Impl* _impl;
};

#endif // _WIN32
