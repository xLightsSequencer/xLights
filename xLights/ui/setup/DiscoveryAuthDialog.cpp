/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/app.h>
#include <wx/sizer.h>

#include "DiscoveryAuthDialog.h"

bool xlPasswordEntryDialog::Create(wxWindow* parent,
                                   const wxString& message,
                                   const wxString& caption,
                                   const wxString& value,
                                   long style,
                                   const wxPoint& pos) {
    bool b = wxPasswordEntryDialog::Create(parent, message, caption, value, style, pos);

    savePasswordCheckbox = new wxCheckBox(this, wxID_ANY, _("Save Password"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PWD"));
    savePasswordCheckbox->SetValue(false);

    wxSizerFlags flagsBorder2;
    flagsBorder2.DoubleBorder();

#if wxUSE_SECRETSTORE
    GetSizer()->Insert(2, savePasswordCheckbox, flagsBorder2);
    GetSizer()->SetSizeHints(this);
    GetSizer()->Fit(this);
#endif
    return b;
}

#if wxUSE_SECRETSTORE
static wxSecretStore pwdStore = wxSecretStore::GetDefault();
bool xlPasswordEntryDialog::GetStoredPasswordForService(const std::string& service, std::string& user, std::string& pwd) {
    if (pwdStore.IsOk()) {
        wxSecretValue password;
        wxString usr;
        if (pwdStore.Load("xLights/Discovery/" + service, usr, password)) {
            user = usr;
            pwd = password.GetAsString();
            return true;
        }
    }
    return false;
}

bool xlPasswordEntryDialog::StorePasswordForService(const std::string& service, const std::string& user, const std::string& pwd) {
    if (pwdStore.IsOk()) {
        if (pwd.empty()) {
            pwdStore.Delete("xLights/Discovery/" + service);
        } else {
            wxSecretValue password(pwd);
            if (pwdStore.Save("xLights/Discovery/" + service, user, password)) {
                return true;
            }
        }
    }
    return false;
}
#else
bool xlPasswordEntryDialog::GetStoredPasswordForService(const std::string& service, std::string& user, std::string& pwd) {
    return false;
}
bool xlPasswordEntryDialog::StorePasswordForService(const std::string& service, const std::string& user, const std::string& pwd) {
    return false;
}
#endif

bool wxDiscoveryDelegate::PromptForPassword(const std::string& host, std::string& username,
                                            std::string& password, bool& savePassword) {
    xlPasswordEntryDialog dlg(_parent, "Password needed to connect to " + host, "Password Required");
    int rc = dlg.ShowModal();
    if (rc == wxID_CANCEL) {
        return false;
    }
    username = "admin";
    password = dlg.GetValue().ToStdString();
    savePassword = dlg.shouldSavePassword();
    return true;
}

bool wxDiscoveryDelegate::GetStoredPassword(const std::string& service, std::string& user, std::string& pwd) {
    return xlPasswordEntryDialog::GetStoredPasswordForService(service, user, pwd);
}

bool wxDiscoveryDelegate::StorePassword(const std::string& service, const std::string& user, const std::string& pwd) {
    return xlPasswordEntryDialog::StorePasswordForService(service, user, pwd);
}

void wxDiscoveryDelegate::Yield() {
    wxYieldIfNeeded();
}
