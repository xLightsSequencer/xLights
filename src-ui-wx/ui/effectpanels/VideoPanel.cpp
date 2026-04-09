/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoPanel.h"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/utils/wxUtilities.h"
#include "ui/media/ManageMediaPanel.h"
#include "render/SequenceMedia.h"
#include "render/SequenceElements.h"
#include "utils/ExternalHooks.h"
#include "utils/xlImage.h"
#include "UtilFunctions.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "effects/VideoEffect.h"

wxDEFINE_EVENT(EVT_VIDEODETAILS, wxCommandEvent);

VideoPanel::VideoPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // Cache pointers to framework-built controls.
    _startTimeSlider = dynamic_cast<wxSlider*>(
        wxWindow::FindWindowByName("IDD_SLIDER_Video_Starttime", this));
    _startTimeText = dynamic_cast<wxTextCtrl*>(
        wxWindow::FindWindowByName("ID_TEXTCTRL_Video_Starttime", this));
    _durationTreatmentChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_Video_DurationTreatment", this));
    _syncWithAudioCheck = dynamic_cast<wxCheckBox*>(
        wxWindow::FindWindowByName("ID_CHECKBOX_SynchroniseWithAudio", this));

    // Bulk-edit isn't useful for the per-effect Start Time slider — its max
    // is computed per video. Match the legacy behavior of disabling bulk edit
    // on it explicitly.
    if (auto* bes = dynamic_cast<BulkEditSlider*>(_startTimeSlider)) {
        bes->SetSupportsBulkEdit(false);
    }
    if (auto* bet = dynamic_cast<BulkEditTextCtrl*>(_startTimeText)) {
        bet->SetSupportsBulkEdit(false);
    }

    // Sync-with-audio and DurationTreatment changes both feed ValidateWindow
    // — the JSON visibility rules cover most of it but the file picker enable
    // state lives outside the framework's properties_ map.
    if (_syncWithAudioCheck) {
        _syncWithAudioCheck->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }

    Bind(EVT_VIDEODETAILS, &VideoPanel::OnVideoDetails, this);

    _previewTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &VideoPanel::OnPreviewTimer, this, _previewTimer.GetId());

    ValidateWindow();
}

VideoPanel::~VideoPanel() {
    _previewTimer.Stop();
}

wxWindow* VideoPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Video_FilenameBlock") {
        return BuildFilenameBlock(parentWin, sizer);
    }
    if (id == "Video_TransparentBlackRow") {
        return BuildTransparentBlackRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* VideoPanel::BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer) {
    // Layout:
    //   [Select] [x]   [animated preview bitmap                      ]
    //   filename label (full width, ellipsized)
    //   Duration: <readonly>          [Match Effect To Video Duration]
    //
    // The hidden xlVideoFilePickerCtrl below holds the actual path so the
    // legacy E_FILEPICKERCTRL_Video_Filename serialization key is preserved.
    auto* outer = new wxFlexGridSizer(0, 1, 0, 0);
    outer->AddGrowableCol(0);

    // Top row: Select+Clear buttons + animated preview bitmap.
    auto* topRow = new wxFlexGridSizer(0, 2, 0, 0);
    topRow->AddGrowableCol(1);

    auto* buttonSizer = new wxFlexGridSizer(0, 1, 0, 0);
    auto* selectRow = new wxBoxSizer(wxHORIZONTAL);
    _selectButton = new wxButton(parentWin, wxNewId(), "Select...");
    selectRow->Add(_selectButton, 1, wxRIGHT, 2);
    wxBitmap clearBmp = wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON);
    _clearButton = new wxBitmapButton(parentWin, wxNewId(), clearBmp,
                                       wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    selectRow->Add(_clearButton, 0, 0, 0);
    buttonSizer->Add(selectRow, 0, wxALL | wxEXPAND, 5);
    topRow->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    _previewBitmap = new wxStaticBitmap(parentWin, wxID_ANY, wxNullBitmap);
    _previewBitmap->SetMinSize(wxDLG_UNIT(parentWin, wxSize(0, 50)));
    topRow->Add(_previewBitmap, 1, wxALL | wxEXPAND, 5);

    outer->Add(topRow, 1, wxALL | wxEXPAND, 0);

    _filenameLabel = new wxStaticText(parentWin, wxID_ANY, wxEmptyString,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    outer->Add(_filenameLabel, 0, wxLEFT | wxRIGHT | wxEXPAND, 5);

    // Duration display + match button row.
    auto* durRow = new wxFlexGridSizer(0, 3, 0, 0);
    durRow->AddGrowableCol(1);
    auto* durLabel = new wxStaticText(parentWin, wxID_ANY, "Duration");
    durRow->Add(durLabel, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    _durationDisplay = new wxTextCtrl(parentWin, wxID_ANY, "0:00:00.000",
                                       wxDefaultPosition, wxDefaultSize,
                                       wxTE_READONLY | wxTE_RIGHT, wxDefaultValidator,
                                       _T("ID_TEXTCTRL_Duration"));
    _durationDisplay->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
    durRow->Add(_durationDisplay, 1, wxALL | wxEXPAND, 2);
    _matchDurationButton = new wxButton(parentWin, wxNewId(), "Match Effect To Video Duration");
    durRow->Add(_matchDurationButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    outer->Add(durRow, 0, wxALL | wxEXPAND, 0);

    // Hidden file picker — kept as a child of the panel so
    // GetEffectStringFromWindow finds it under the legacy name and writes
    // E_FILEPICKERCTRL_Video_Filename.
    _hiddenFilePicker = new xlVideoFilePickerCtrl(parentWin, wxNewId(), wxEmptyString,
                                                   wxEmptyString, wxEmptyString,
                                                   wxDefaultPosition, wxDefaultSize,
                                                   wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                   wxDefaultValidator,
                                                   _T("ID_FILEPICKERCTRL_Video_Filename"));
    _hiddenFilePicker->Hide();

    sizer->Add(outer, 1, wxALL | wxEXPAND, 0);

    _selectButton->Bind(wxEVT_BUTTON, &VideoPanel::OnSelectClick, this);
    _clearButton->Bind(wxEVT_BUTTON, &VideoPanel::OnClearClick, this);
    _matchDurationButton->Bind(wxEVT_BUTTON, &VideoPanel::OnMatchVideoDurationClick, this);
    _hiddenFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, &VideoPanel::OnFilePickerChanged, this);

    return _selectButton;
}

wxWindow* VideoPanel::BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer) {
    // Same Faces TransparentBlack pattern: checkbox + slider + text on a
    // single visually-linked row, preserving the legacy IDs so old sequences
    // round-trip via E_CHECKBOX_Video_TransparentBlack /
    // E_TEXTCTRL_Video_TransparentBlack.
    auto* row = new wxFlexGridSizer(0, 3, 0, 0);
    row->AddGrowableCol(1);

    _transparentBlackCheck = new BulkEditCheckBox(parentWin, wxNewId(), "Transparent Black",
                                                    wxDefaultPosition, wxDefaultSize, 0,
                                                    wxDefaultValidator,
                                                    _T("ID_CHECKBOX_Video_TransparentBlack"));
    _transparentBlackCheck->SetValue(false);
    row->Add(_transparentBlackCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _transparentBlackSlider = new BulkEditSlider(parentWin, wxNewId(), 0, 0, 300,
                                                   wxDefaultPosition, wxDefaultSize, 0,
                                                   wxDefaultValidator,
                                                   _T("IDD_SLIDER_Video_TransparentBlack"));
    row->Add(_transparentBlackSlider, 1, wxALL | wxEXPAND, 5);

    _transparentBlackText = new BulkEditTextCtrl(parentWin, wxNewId(), _T("0"),
                                                   wxDefaultPosition,
                                                   wxDLG_UNIT(parentWin, wxSize(40, -1)),
                                                   wxTE_RIGHT, wxDefaultValidator,
                                                   _T("ID_TEXTCTRL_Video_TransparentBlack"));
    row->Add(_transparentBlackText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    sizer->Add(row, 1, wxALL | wxEXPAND, 0);
    return _transparentBlackCheck;
}

void VideoPanel::OnSelectClick(wxCommandEvent& /*event*/) {
    xLightsFrame* xl = nullptr;
    for (wxWindow* w = GetParent(); w; w = w->GetParent()) {
        xl = dynamic_cast<xLightsFrame*>(w);
        if (xl) break;
    }
    if (!xl || !_hiddenFilePicker) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    SequenceElements& elements = xl->GetSequenceElements();
    std::string currentPath = _hiddenFilePicker->GetFileName().GetFullPath().ToStdString();
    SelectMediaDialog dlg(this, &media, &elements,
                          xl->GetShowDirectory(), xl, MediaType::Video, currentPath);
    if (dlg.ShowModal() != wxID_OK) return;
    std::string selected = dlg.GetSelectedPath();
    if (selected.empty()) return;

    _hiddenFilePicker->SetFileName(wxFileName(selected));
    // Manually fire the file picker change to trigger probe + preview update.
    wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, _hiddenFilePicker,
                              _hiddenFilePicker->GetId(), selected);
    _hiddenFilePicker->ProcessWindowEvent(evt);
    FireChangeEvent();
}

void VideoPanel::OnClearClick(wxCommandEvent& /*event*/) {
    if (!_hiddenFilePicker) return;
    _hiddenFilePicker->SetFileName(wxFileName());
    UpdatePreview();
    FireChangeEvent();
}

void VideoPanel::OnFilePickerChanged(wxFileDirPickerEvent& event) {
    std::unique_lock<std::mutex> locker(_videoTimeLock);
    if (_hiddenFilePicker == nullptr) return;
    wxFileName fn = _hiddenFilePicker->GetFileName();
    ObtainAccessToURL(fn.GetFullPath().ToStdString());
    auto cacheIt = _videoTimeCache.find(fn.GetFullPath().ToStdString());
    if (cacheIt != _videoTimeCache.end() && cacheIt->second > 0) {
        if (_startTimeSlider) _startTimeSlider->SetMax(cacheIt->second / 10);
        if (_durationDisplay) _durationDisplay->SetValue(FORMATTIME(cacheIt->second));
    } else {
        // Probe hasn't happened yet — give the slider enough headroom for
        // the user to set a sensible start time before the real max comes in.
        if (_startTimeSlider) _startTimeSlider->SetMax(99999);
        if (_durationDisplay) _durationDisplay->SetValue(FORMATTIME(0));
    }
    locker.unlock();
    UpdatePreview();
    event.Skip();
}

void VideoPanel::OnMatchVideoDurationClick(wxCommandEvent& /*event*/) {
    // Has to be dispatched up to xLightsFrame — the panel can't change its
    // own effect's duration directly.
    if (!_hiddenFilePicker) return;
    wxCommandEvent e(EVT_SET_EFFECT_DURATION);
    wxFileName fn = _hiddenFilePicker->GetFileName();
    ObtainAccessToURL(fn.GetFullPath().ToStdString());
    auto duration = _videoTimeCache[fn.GetFullPath().ToStdString()];
    if (duration > 0) {
        e.SetString("Video");
        e.SetInt(duration);
        wxPostEvent(wxTheApp->GetTopWindow(), e);
    }
}

void VideoPanel::OnVideoDetails(wxCommandEvent& event) {
    AddVideoTime(event.GetString().ToStdString(), event.GetInt());
}

void VideoPanel::AddVideoTime(std::string fn, unsigned long ms) {
    std::unique_lock<std::mutex> locker(_videoTimeLock);
    if (!_hiddenFilePicker) return;
    wxFileName file = _hiddenFilePicker->GetFileName();
    std::string current = file.GetFullPath().ToStdString();
    // Prevent event looping when switching between video effects rapidly.
    if (current != fn) return;

    _videoTimeCache[fn] = ms;

    if (_startTimeSlider && (unsigned long)_startTimeSlider->GetMax() != ms) {
        _startTimeSlider->SetMax(ms / 10);
    }
    if (_durationDisplay) _durationDisplay->SetValue(FORMATTIME(ms));
}

void VideoPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    bool sync = _syncWithAudioCheck && _syncWithAudioCheck->IsChecked();
    if (_hiddenFilePicker) _hiddenFilePicker->Enable(!sync);
    if (_selectButton) _selectButton->Enable(!sync);
    if (_clearButton) _clearButton->Enable(!sync);

    // Background color hint based on file path validity (matches legacy red /
    // yellow tinting). The hidden picker is invisible so the color affects
    // nothing visually, but the tooltip propagates which is the part the user
    // actually sees on hover.
    if (_hiddenFilePicker) {
        wxString file = _hiddenFilePicker->GetFileName().GetFullPath();
        bool fileExists = file.empty() || FileExists(file);
        if (!fileExists) {
            auto* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
            if (xl) {
                fileExists = xl->GetSequenceElements().GetSequenceMedia().HasMedia(file.ToStdString());
            }
        }
        if (!file.empty() && !fileExists) {
            SetToolTip("File " + file + " does not exist.");
        } else if (!file.empty() && !IsXmlSafe(file)) {
            SetToolTip("File " + file + " contains characters in the path or filename that will cause issues in xLights. Please rename it.");
        } else {
            SetToolTip(file);
        }
    }
}

void VideoPanel::UpdatePreview() {
    _previewTimer.Stop();
    _previewFrames.clear();
    _previewFrameTimes.clear();
    _currentPreviewFrame = 0;

    if (!_previewBitmap) return;

    wxFileName fn = _hiddenFilePicker ? _hiddenFilePicker->GetFileName() : wxFileName();
    std::string file = fn.GetFullPath().ToStdString();
    if (_filenameLabel) _filenameLabel->SetLabel(file);
    if (file.empty()) {
        _previewBitmap->SetBitmap(wxNullBitmap);
        return;
    }

    // Look up video through SequenceMedia (handles relative paths).
    SequenceMedia* media = nullptr;
    for (wxWindow* w = GetParent(); w; w = w->GetParent()) {
        if (auto* xl = dynamic_cast<xLightsFrame*>(w)) {
            media = &xl->GetSequenceElements().GetSequenceMedia();
            break;
        }
    }
    if (!media) return;

    auto entry = media->GetVideo(file);
    if (!entry || entry->GetResolvedPath().empty()) {
        _previewBitmap->SetBitmap(wxNullBitmap);
        return;
    }

    // Use a fixed generation size since the widget may not be laid out yet.
    // ShowPreviewFrame will scale to fit the actual widget size.
    int genSize = 300;
    entry->GeneratePreview(genSize, genSize);

    for (size_t i = 0; i < entry->GetPreviewFrameCount(); i++) {
        _previewFrames.push_back(entry->GetPreviewFrame(i));
        _previewFrameTimes.push_back(entry->GetPreviewFrameTime(i));
    }
    if (_previewFrames.empty()) return;

    ShowPreviewFrame(0);
    if (_previewFrames.size() > 1) {
        long interval = (_previewFrameTimes[0] > 0) ? _previewFrameTimes[0] : 50;
        _previewTimer.Start(interval);
    }
}

void VideoPanel::OnPreviewTimer(wxTimerEvent& /*event*/) {
    if (_previewFrames.empty()) {
        _previewTimer.Stop();
        return;
    }
    _currentPreviewFrame = (_currentPreviewFrame + 1) % _previewFrames.size();
    ShowPreviewFrame(_currentPreviewFrame);

    long interval = (_currentPreviewFrame < _previewFrameTimes.size() && _previewFrameTimes[_currentPreviewFrame] > 0)
                        ? _previewFrameTimes[_currentPreviewFrame]
                        : 50;
    _previewTimer.Start(interval);
}

void VideoPanel::ShowPreviewFrame(size_t index) {
    if (index >= _previewFrames.size() || !_previewFrames[index] || !_previewFrames[index]->IsOk()) return;

    const auto& img = _previewFrames[index];
    double scaleFactor = GetContentScaleFactor();

    wxSize widgetSize = _previewBitmap->GetSize();
    int sw = img->GetWidth();
    int sh = img->GetHeight();
    if (widgetSize.x > 0 && widgetSize.y > 0) {
        double pw = widgetSize.x * scaleFactor;
        double ph = widgetSize.y * scaleFactor;
        double scale = std::min(pw / sw, ph / sh);
        sw = std::max(1, (int)(sw * scale));
        sh = std::max(1, (int)(sh * scale));
    }

    xlImage scaled(*img);
    if (sw != img->GetWidth() || sh != img->GetHeight()) {
        scaled.Rescale(sw, sh);
    }

    wxBitmap bmp(xlImageToWxImage(scaled));
    bmp.SetScaleFactor(scaleFactor);
    _previewBitmap->SetBitmap(bmp);
    _previewBitmap->InvalidateBestSize();
    _previewBitmap->Refresh();
    _previewBitmap->Update();
    if (_previewBitmap->GetParent())
        _previewBitmap->GetParent()->Layout();
}
