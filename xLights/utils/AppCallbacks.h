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

#include <functional>

// Platform-neutral callbacks for core code that needs to interact with
// the application layer without depending on wx.  The wx app registers
// concrete implementations at startup; non-wx hosts can register their
// own or leave the defaults (which are safe no-ops or std::terminate).

namespace AppCallbacks {

// Post a callable to execute on the main/UI thread.
// Default: executes the callable immediately on the calling thread.
void PostToMainThread(std::function<void()> fn);
void SetPostToMainThread(std::function<void(std::function<void()>)> handler);

// Called when an unhandled exception escapes a worker thread.
// Default: calls std::terminate().
void HandleUnhandledException();
void SetHandleUnhandledException(std::function<void()> handler);

// Called at worker thread startup to install platform crash handlers.
// Default: no-op.
void SetupThreadCrashHandler();
void SetSetupThreadCrashHandler(std::function<void()> handler);

} // namespace AppCallbacks
