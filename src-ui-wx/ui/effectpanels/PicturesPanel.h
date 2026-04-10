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

#include <wx/timer.h>

#include <memory>
#include <vector>

class Model;
class SequenceMedia;
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxSlider;
class wxStaticBitmap;
class wxStaticText;
class wxTextCtrl;
class xlImage;

class PicturesPanel : public JsonEffectPanel {
public:
    PicturesPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~PicturesPanel() override;

    void ValidateWindow() override;
    bool HasAssistPanel() override { return true; }
    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;

    // Provide access to sequence media for the image preview thumbnail (used
    // by the legacy unit-test harness; preserved for compatibility).
    void SetSequenceMedia(SequenceMedia* media) { _sequenceMedia = media; }

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer);

    void OnSelectClick(wxCommandEvent& event);
    void OnAIGenerateClick(wxCommandEvent& event);
    void OnClearClick(wxCommandEvent& event);
    void OnPreviewTimer(wxTimerEvent& event);
    void OnShowPanel(wxShowEvent& event);
    void UpdatePreviewBitmap(const wxString& filename);
    void ShowPreviewFrame(size_t index);

    // Custom-built top block.
    wxButton* _selectButton = nullptr;
    wxButton* _aiGenerateButton = nullptr;
    wxBitmapButton* _clearButton = nullptr;
    wxTextCtrl* _filenameCtrl = nullptr;  // hidden, holds the actual path
    wxStaticBitmap* _previewBitmap = nullptr;
    wxStaticText* _filenameLabel = nullptr;

    // Custom-built TransparentBlack inline row.
    wxCheckBox* _transparentBlackCheck = nullptr;
    wxSlider* _transparentBlackSlider = nullptr;
    wxTextCtrl* _transparentBlackText = nullptr;

    // Cached pointers to framework-built controls used in ValidateWindow.
    wxChoice* _directionChoice = nullptr;
    wxCheckBox* _loopGifCheck = nullptr;
    wxCheckBox* _suppressGifBgCheck = nullptr;
    wxWindow* _xcVcButton = nullptr;
    wxWindow* _ycVcButton = nullptr;

    // Animated preview state.
    wxTimer _previewTimer;
    std::vector<std::shared_ptr<xlImage>> _previewFrames;
    std::vector<long> _previewFrameTimes;
    size_t _currentPreviewFrame = 0;

    SequenceMedia* _sequenceMedia = nullptr;
};
