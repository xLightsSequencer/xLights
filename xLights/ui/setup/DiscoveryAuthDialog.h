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

#include <string>

#include <wx/checkbox.h>
#include <wx/secretstore.h>
#include <wx/textdlg.h>

class WXDLLIMPEXP_CORE xlPasswordEntryDialog : public wxPasswordEntryDialog {
public:
    xlPasswordEntryDialog(wxWindow* parent,
                          const wxString& message,
                          const wxString& caption = wxGetPasswordFromUserPromptStr,
                          const wxString& value = wxEmptyString,
                          long style = wxTextEntryDialogStyle,
                          const wxPoint& pos = wxDefaultPosition) {
        Create(parent, message, caption, value, style, pos);
    }

    bool Create(wxWindow* parent,
                const wxString& message,
                const wxString& caption = wxGetPasswordFromUserPromptStr,
                const wxString& value = wxEmptyString,
                long style = wxTextEntryDialogStyle,
                const wxPoint& pos = wxDefaultPosition);

    bool shouldSavePassword() const {
        return savePasswordCheckbox && savePasswordCheckbox->IsChecked();
    }

    static bool GetStoredPasswordForService(const std::string& service, std::string& user, std::string& pwd);
    static bool StorePasswordForService(const std::string& service, const std::string& user, const std::string& pwd);

private:
    wxCheckBox* savePasswordCheckbox = nullptr;
};

#include "discovery/Discovery.h"

// wx-based DiscoveryDelegate that uses wxSecretStore for credential storage,
// xlPasswordEntryDialog for auth prompts, and wxYieldIfNeeded for yielding.
class wxDiscoveryDelegate : public DiscoveryDelegate {
public:
    explicit wxDiscoveryDelegate(wxWindow* parent) : _parent(parent) {}

    bool PromptForPassword(const std::string& host, std::string& username,
                           std::string& password, bool& savePassword) override;
    bool GetStoredPassword(const std::string& service, std::string& user, std::string& pwd) override;
    bool StorePassword(const std::string& service, const std::string& user, const std::string& pwd) override;
    void Yield() override;

private:
    wxWindow* _parent = nullptr;
};