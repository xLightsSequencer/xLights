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

#include <wx/timer.h>

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#define VIDEOWILDCARD "Video Files|*.avi;*.mp4;*.mkv;*.mov;*.asf;*.flv;*.mpg;*.mpeg;*.m4v;*.wmv;*.gif"

class Model;
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFilePickerCtrl;
class wxSlider;
class wxStaticBitmap;
class wxStaticText;
class wxTextCtrl;
class wxFileDirPickerEvent;
class xlImage;

wxDECLARE_EVENT(EVT_VIDEODETAILS, wxCommandEvent);

// Subclass of BulkEditFilePickerCtrl that bakes in the video wildcard.
// Kept here so the file picker the subclass creates inside its custom block
// has the same behavior as the legacy panel.
class xlVideoFilePickerCtrl : public BulkEditFilePickerCtrl {
public:
    xlVideoFilePickerCtrl(wxWindow* parent,
                          wxWindowID id,
                          const wxString& path = wxEmptyString,
                          const wxString& message = wxFileSelectorPromptStr,
                          const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxDefaultSize,
                          long style = wxFLP_DEFAULT_STYLE,
                          const wxValidator& validator = wxDefaultValidator,
                          const wxString& name = wxFilePickerCtrlNameStr) :
        BulkEditFilePickerCtrl(parent, id, path, message, VIDEOWILDCARD, pos, size, style, validator, name) {}
    ~xlVideoFilePickerCtrl() override = default;
};

class VideoPanel : public JsonEffectPanel {
public:
    VideoPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~VideoPanel() override;

    void ValidateWindow() override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer);

    void OnSelectClick(wxCommandEvent& event);
    void OnClearClick(wxCommandEvent& event);
    void OnFilePickerChanged(wxFileDirPickerEvent& event);
    void OnMatchVideoDurationClick(wxCommandEvent& event);
    void OnVideoDetails(wxCommandEvent& event);
    void OnPreviewTimer(wxTimerEvent& event);
    void AddVideoTime(std::string fn, unsigned long ms);
    void UpdatePreview();
    void ShowPreviewFrame(size_t index);

    // Custom-built top block (file picker + select/clear/preview/duration/match).
    xlVideoFilePickerCtrl* _hiddenFilePicker = nullptr;
    wxStaticBitmap* _previewBitmap = nullptr;
    wxStaticText* _filenameLabel = nullptr;
    wxButton* _selectButton = nullptr;
    wxBitmapButton* _clearButton = nullptr;
    wxTextCtrl* _durationDisplay = nullptr;
    wxButton* _matchDurationButton = nullptr;

    // Custom-built TransparentBlack inline row.
    wxCheckBox* _transparentBlackCheck = nullptr;
    wxSlider* _transparentBlackSlider = nullptr;
    wxTextCtrl* _transparentBlackText = nullptr;

    // Cached pointers to framework-built controls used in ValidateWindow /
    // dynamic Start Time max updates.
    wxSlider* _startTimeSlider = nullptr;
    wxTextCtrl* _startTimeText = nullptr;
    wxChoice* _durationTreatmentChoice = nullptr;
    wxCheckBox* _syncWithAudioCheck = nullptr;

    // Animated preview state.
    wxTimer _previewTimer;
    std::vector<std::shared_ptr<xlImage>> _previewFrames;
    std::vector<long> _previewFrameTimes;
    size_t _currentPreviewFrame = 0;

    // Video duration cache (filename → milliseconds), populated by the
    // EVT_VIDEODETAILS event posted from the background video probe thread.
    std::mutex _videoTimeLock;
    std::map<std::string, unsigned long> _videoTimeCache;
};
