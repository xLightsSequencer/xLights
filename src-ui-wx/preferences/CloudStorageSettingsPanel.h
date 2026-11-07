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

#include <wx/panel.h>

#include <string>

class wxButton;
class wxCheckBox;
class wxChoice;
class wxSpinCtrl;
class wxStaticText;
class wxTextCtrl;
struct CloudStorageSettings;

class CloudStorageSettingsPanel : public wxPanel {
public:
    explicit CloudStorageSettingsPanel(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~CloudStorageSettingsPanel();

    virtual bool TransferDataFromWindow() override;
    virtual bool TransferDataToWindow() override;

private:
    CloudStorageSettings FromControls() const;
    void UpdateEnabled();
    void ApplyIfImmediate(bool includeSecret);
    void OnTestConnection(wxCommandEvent& event);

    wxChoice* _provider = nullptr;
    wxTextCtrl* _accountId = nullptr;
    wxTextCtrl* _endpoint = nullptr;
    wxTextCtrl* _region = nullptr;
    wxCheckBox* _pathStyle = nullptr;
    wxTextCtrl* _bucket = nullptr;
    wxTextCtrl* _prefix = nullptr;
    wxTextCtrl* _accessKey = nullptr;
    wxTextCtrl* _secretKey = nullptr;
    wxButton* _testButton = nullptr;
    wxStaticText* _testResult = nullptr;
    wxCheckBox* _includeFseq = nullptr;
    wxCheckBox* _includeThumbnail = nullptr;
    wxSpinCtrl* _keepBackups = nullptr;

    std::string _savedSecret;
    bool _loading = false;
};
