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
#include <string>

// Abstract interface replacing direct wxMessageBox / wxFileSelector /
// wxDirDialog / wxGetNumberFromUser / wxProgressDialog calls in
// render/, effects/, and models/ code.
//
// The UI layer (xLightsFrame) provides a concrete implementation that
// delegates to wx dialogs.  Non-UI callers (tests, headless) can
// supply stubs or alternative implementations.

class UICallbacks {
public:
    virtual ~UICallbacks() = default;

    // ---- message dialogs ----
    // Show an informational / error message.  |caption| is the dialog title.
    virtual void ShowMessage(const std::string& message,
                             const std::string& caption = "xLights") const = 0;

    // Ask a yes/no question.  Returns true for "yes".
    virtual bool PromptYesNo(const std::string& message,
                             const std::string& caption = "xLights") const = 0;

    // ---- file / directory pickers ----
    // Returns the chosen path, or empty string if cancelled.
    virtual std::string PromptForDirectory(const std::string& message,
                                           const std::string& defaultPath = "") const = 0;

    virtual std::string PromptForFile(const std::string& message,
                                      const std::string& wildcard = "",
                                      const std::string& defaultPath = "") const = 0;

    // ---- number input ----
    // Returns the entered number, or |defaultValue| if cancelled.
    virtual long PromptForNumber(const std::string& message,
                                 const std::string& caption,
                                 long defaultValue,
                                 long min, long max) const = 0;

    // ---- text input ----
    // Returns the entered text, or |defaultValue| if cancelled.
    virtual std::string PromptForText(const std::string& message,
                                      const std::string& caption,
                                      const std::string& defaultValue = "") const = 0;

    // ---- configuration queries ----
    virtual bool IsCheckSequenceOptionDisabled(const std::string& option) const { return false; }

    // ---- progress reporting ----
    // Begin a progress operation.  Returns a token that the caller passes
    // to UpdateProgress / EndProgress.  |maximum| is the upper bound of
    // the progress range (0 .. maximum).
    using ProgressToken = int;
    static constexpr ProgressToken INVALID_PROGRESS = -1;

    virtual ProgressToken BeginProgress(const std::string& message,
                                        int maximum = 100) = 0;
    virtual void UpdateProgress(ProgressToken token, int value,
                                const std::string& newMessage = "") = 0;
    virtual void EndProgress(ProgressToken token) = 0;
};
