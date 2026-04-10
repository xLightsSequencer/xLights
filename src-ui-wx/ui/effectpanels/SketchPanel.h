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

#include "JsonEffectPanel.h"
#include "ui/shared/controls/BulkEditControls.h"

class BulkEditTextCtrl;
class MediaPickerCtrl;
class SketchAssistPanel;
class wxFilePickerCtrl;
class wxSlider;

// File picker subclass that defaults the wildcard to wxImage::GetImageExtWildcard().
// Kept here so the subclass can hand the same control to MediaPickerCtrl.
class xlSketchFilePickerCtrl : public BulkEditFilePickerCtrl
{
public:
    xlSketchFilePickerCtrl(wxWindow* parent,
                           wxWindowID id,
                           const wxString& path = wxEmptyString,
                           const wxString& message = wxFileSelectorPromptStr,
                           const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
                           const wxPoint& pos = wxDefaultPosition,
                           const wxSize& size = wxDefaultSize,
                           long style = wxFLP_DEFAULT_STYLE,
                           const wxValidator& validator = wxDefaultValidator,
                           const wxString& name = wxFilePickerCtrlNameStr) :
        BulkEditFilePickerCtrl(parent, id, path, message, wxImage::GetImageExtWildcard(), pos, size, style, validator, name)
    {
    }
    ~xlSketchFilePickerCtrl() override = default;
};

class SketchPanel : public JsonEffectPanel {
public:
    SketchPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~SketchPanel() override = default;

    void ValidateWindow() override;
    void SetDefaultParameters() override;

    bool HasAssistPanel() override { return true; }
    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildInfoBlock(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildSketchDefRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildBackgroundRow(wxWindow* parentWin, wxSizer* sizer);

    void OnFilePickerChanged(wxCommandEvent& event);
    void OnOpacitySliderChanged(wxCommandEvent& event);
    void UpdateSketchAssistFromControls();
    void UpdateSketchAssistBackground() const;

    // Custom-built controls (not in properties_ map).
    BulkEditTextCtrl* _sketchDefText = nullptr;
    xlSketchFilePickerCtrl* _hiddenFilePicker = nullptr;
    MediaPickerCtrl* _mediaPicker = nullptr;

    // Cached pointer to the framework-built opacity slider so the
    // file-picker / opacity event handlers can read it without re-walking.
    wxSlider* _opacitySlider = nullptr;

    mutable SketchAssistPanel* _sketchAssistPanel = nullptr;
};
