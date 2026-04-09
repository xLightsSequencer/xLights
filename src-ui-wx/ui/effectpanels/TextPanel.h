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

class Model;
class MediaPickerCtrl;
class wxChoice;
class wxFilePickerCtrl;
class wxStaticText;
class wxTextCtrl;

class TextPanel : public JsonEffectPanel {
public:
    TextPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~TextPanel() override = default;

    void ValidateWindow() override;
    void SetPanelStatus(Model* cls) override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildFileRow(wxWindow* parentWin, wxSizer* sizer, int cols);
    wxWindow* BuildXLFontRow(wxWindow* parentWin, wxSizer* sizer, int cols);

    // Custom-built controls (not in properties_ map).
    wxFilePickerCtrl* _hiddenFilePicker = nullptr;
    MediaPickerCtrl* _filePickerMedia = nullptr;
    wxChoice* _xlFontChoice = nullptr;

    // Cached pointers to framework-built controls used by ValidateWindow.
    wxTextCtrl* _textInput = nullptr;
    wxChoice* _movementChoice = nullptr;
    wxChoice* _lyricTrackChoice = nullptr;
};
