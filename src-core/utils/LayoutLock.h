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

#include <atomic>
#include <mutex>
#include <string>

// Freezes the show's layout (models, groups, view objects), controller setup
// and sequence settings while a collaboration session is running: every peer
// edits the same sequence against the same layout, and layout changes are not
// carried by the session. xLightsShowContext owns the one instance and hands
// a pointer to the model, view object and output managers, so their own
// mutators can refuse even when a UI entry point forgot to check.
//
// The flag is read from render and UI threads; the reason is only changed
// together with the flag.
class LayoutLock {
public:
    static constexpr const char* DEFAULT_REASON = "Locked during collaboration session";

    void Set(bool locked, const std::string& reason = std::string()) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _reason = locked ? (reason.empty() ? std::string(DEFAULT_REASON) : reason) : std::string();
        }
        _locked.store(locked, std::memory_order_release);
    }
    bool IsLocked() const { return _locked.load(std::memory_order_acquire); }
    // Empty when not locked.
    std::string GetReason() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _reason;
    }

    // For the managers' mutators: true (and logs `what`) when the lock refuses
    // the change. A null lock never refuses.
    static bool Refuses(const LayoutLock* lock, const char* what);

private:
    std::atomic<bool> _locked{ false };
    mutable std::mutex _mutex;
    std::string _reason;
};
