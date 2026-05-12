/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PicturesPanel.h"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/datetime.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "shared/controls/BulkEditControls.h"
#include "shared/utils/wxUtilities.h"
#include "media/ManageMediaPanel.h"
#include "sequencer/MainSequencer.h"
#include "render/SequenceMedia.h"
#include "render/SequenceElements.h"
#include "utils/xlImage.h"
#include "ai/AIImageDialog.h"
#include "ai/aiType.h"
#include "assist/AssistPanel.h"
#include "assist/PicturesAssistPanel.h"
#include "assist/xlGridCanvasPictures.h"
#include "xLightsMain.h"
#include "effects/PicturesEffect.h"
#include "models/Model.h"

PicturesPanel::PicturesPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // Cache pointers to framework-built controls used in ValidateWindow.
    _directionChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_Pictures_Direction", this));
    _loopGifCheck = dynamic_cast<wxCheckBox*>(
        wxWindow::FindWindowByName("ID_CHECKBOX_LoopGIF", this));
    _suppressGifBgCheck = dynamic_cast<wxCheckBox*>(
        wxWindow::FindWindowByName("ID_CHECKBOX_SuppressGIFBackground", this));
    _xcVcButton = wxWindow::FindWindowByName("ID_VALUECURVE_PicturesXC", this);
    _ycVcButton = wxWindow::FindWindowByName("ID_VALUECURVE_PicturesYC", this);

    if (_directionChoice) {
        _directionChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }

    _previewTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &PicturesPanel::OnPreviewTimer, this, _previewTimer.GetId());

    // Pause the preview timer when the panel is hidden (user switches to a
    // different effect) and resume when it's shown again. The effect panels
    // are cached by EffectPanelManager so the timer would otherwise keep
    // decoding / rescaling frames for a hidden widget. Bound as a member
    // function (not a lambda) so the destructor can Unbind it — otherwise
    // a wxEVT_SHOW dispatched from the Win32 HWND teardown after
    // ~PicturesPanel has already destroyed _previewFrames / _previewTimer
    // would access destroyed members and crash on exit.
    Bind(wxEVT_SHOW, &PicturesPanel::OnShowPanel, this);

    ValidateWindow();
}

PicturesPanel::~PicturesPanel() {
    Unbind(wxEVT_SHOW, &PicturesPanel::OnShowPanel, this);
    _previewTimer.Stop();
}

void PicturesPanel::OnShowPanel(wxShowEvent& event) {
    if (event.IsShown()) {
        if (_previewFrames.size() > 1) {
            size_t idx = _currentPreviewFrame < _previewFrameTimes.size() ? _currentPreviewFrame : 0;
            long interval = (_previewFrameTimes[idx] > 0) ? _previewFrameTimes[idx] : 50;
            _previewTimer.Start(interval);
        }
    } else {
        _previewTimer.Stop();
    }
    event.Skip();
}

wxWindow* PicturesPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                              const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Pictures_FilenameBlock") {
        return BuildFilenameBlock(parentWin, sizer);
    }
    if (id == "Pictures_TransparentBlackRow") {
        return BuildTransparentBlackRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* PicturesPanel::BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer) {
    // Layout:
    //   [Select... ] [x]   [animated preview bitmap]
    //   [AI Generate...]
    //   filename label (full width, ellipsized)
    //
    // The hidden _filenameCtrl below holds the actual path so the legacy
    // E_TEXTCTRL_Pictures_Filename serialization key is preserved.
    auto* outer = new wxFlexGridSizer(0, 1, 0, 0);
    outer->AddGrowableCol(0);

    auto* topRow = new wxFlexGridSizer(0, 2, 0, 0);
    topRow->AddGrowableCol(1);

    auto* buttonSizer = new wxFlexGridSizer(0, 1, 0, 0);

    // Select + Clear (X) on one row.
    auto* selectRow = new wxBoxSizer(wxHORIZONTAL);
    _selectButton = new wxButton(parentWin, wxNewId(), "Select...");
    selectRow->Add(_selectButton, 1, wxRIGHT, 2);
    wxBitmap clearBmp = wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON);
    _clearButton = new wxBitmapButton(parentWin, wxNewId(), clearBmp,
                                       wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    _clearButton->SetToolTip("Clear image");
    selectRow->Add(_clearButton, 0, 0, 0);
    buttonSizer->Add(selectRow, 0, wxALL | wxEXPAND, 5);

    _aiGenerateButton = new wxButton(parentWin, wxNewId(), "AI Generate...");
    buttonSizer->Add(_aiGenerateButton, 1, wxALL | wxEXPAND, 5);

    topRow->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    _previewBitmap = new wxStaticBitmap(parentWin, wxID_ANY, wxNullBitmap);
    _previewBitmap->SetMinSize(wxDLG_UNIT(parentWin, wxSize(0, 50)));
    topRow->Add(_previewBitmap, 1, wxALL | wxEXPAND, 5);

    outer->Add(topRow, 1, wxALL | wxEXPAND, 0);

    _filenameLabel = new wxStaticText(parentWin, wxID_ANY, wxEmptyString,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    outer->Add(_filenameLabel, 0, wxLEFT | wxRIGHT | wxEXPAND, 5);

    // Hidden text control holds the path. Named to match legacy
    // ID_TEXTCTRL_Pictures_Filename so GetEffectStringFromWindow writes
    // E_TEXTCTRL_Pictures_Filename — what PicturesEffect::Render reads.
    // Added to the outer sizer so it participates in layout ownership like
    // every other control the builder creates; wxRESERVE_SPACE_EVEN_IF_HIDDEN
    // is omitted so it consumes no visual space.
    _filenameCtrl = new wxTextCtrl(parentWin, wxNewId(), wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize, 0,
                                    wxDefaultValidator, _T("ID_TEXTCTRL_Pictures_Filename"));
    _filenameCtrl->Hide();
    outer->Add(_filenameCtrl, 0, 0, 0);

    sizer->Add(outer, 1, wxALL | wxEXPAND, 0);

    _selectButton->Bind(wxEVT_BUTTON, &PicturesPanel::OnSelectClick, this);
    _aiGenerateButton->Bind(wxEVT_BUTTON, &PicturesPanel::OnAIGenerateClick, this);
    _clearButton->Bind(wxEVT_BUTTON, &PicturesPanel::OnClearClick, this);

    return _selectButton;
}

wxWindow* PicturesPanel::BuildTransparentBlackRow(wxWindow* parentWin, wxSizer* sizer) {
    // Same Faces TransparentBlack pattern: checkbox + slider + text on a
    // single visually-linked row, preserving the legacy IDs so old sequences
    // round-trip via E_CHECKBOX_Pictures_TransparentBlack and
    // E_TEXTCTRL_Pictures_TransparentBlack.
    auto* row = new wxFlexGridSizer(0, 3, 0, 0);
    row->AddGrowableCol(1);

    _transparentBlackCheck = new BulkEditCheckBox(parentWin, wxNewId(), "Transparent Black",
                                                    wxDefaultPosition, wxDefaultSize, 0,
                                                    wxDefaultValidator,
                                                    _T("ID_CHECKBOX_Pictures_TransparentBlack"));
    _transparentBlackCheck->SetValue(false);
    row->Add(_transparentBlackCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

    _transparentBlackSlider = new BulkEditSlider(parentWin, wxNewId(), 0, 0, 300,
                                                   wxDefaultPosition, wxDefaultSize, 0,
                                                   wxDefaultValidator,
                                                   _T("IDD_SLIDER_Pictures_TransparentBlack"));
    row->Add(_transparentBlackSlider, 1, wxALL | wxEXPAND, 5);

    _transparentBlackText = new BulkEditTextCtrl(parentWin, wxNewId(), _T("0"),
                                                   wxDefaultPosition,
                                                   wxDLG_UNIT(parentWin, wxSize(40, -1)),
                                                   wxTE_RIGHT, wxDefaultValidator,
                                                   _T("ID_TEXTCTRL_Pictures_TransparentBlack"));
    row->Add(_transparentBlackText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    sizer->Add(row, 1, wxALL | wxEXPAND, 0);
    return _transparentBlackCheck;
}

void PicturesPanel::OnSelectClick(wxCommandEvent& /*event*/) {
    xLightsFrame* xl = nullptr;
    for (wxWindow* w = this; w; w = w->GetParent()) {
        xl = dynamic_cast<xLightsFrame*>(w);
        if (xl) break;
    }
    if (!xl || !_filenameCtrl) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    SequenceElements& elements = xl->GetSequenceElements();

    std::string currentPath = _filenameCtrl->GetValue().ToStdString();
    SelectMediaDialog dlg(this, &media, &elements,
                          xl->GetShowDirectory(), xl, MediaType::Image, currentPath);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string selected = dlg.GetSelectedPath();
    if (selected.empty()) return;

    _filenameCtrl->SetValue(ToWXString(selected));

    if (_currentModel) {
        auto entry = media.GetImage(selected);
        if (entry && entry->IsOk()) {
            int imgW = entry->GetImageWidth();
            int imgH = entry->GetImageHeight();
            int bufW = _currentModel->GetDefaultBufferWi();
            int bufH = _currentModel->GetDefaultBufferHt();
            if (imgW > bufW && imgH > bufH) {
                auto* scalingChoice = dynamic_cast<wxChoice*>(
                    wxWindow::FindWindowByName("ID_CHOICE_Scaling", this));
                if (scalingChoice && scalingChoice->GetStringSelection() == "No Scaling") {
                    SetChoiceValue(scalingChoice, "Scale To Fit");
                }
            }
        }
    }

    UpdatePreviewBitmap(selected);
    FireChangeEvent();

    if (xl->GetMainSequencer() && xl->GetMainSequencer()->GetSelectedEffectCount("Pictures") > 1) {
        xl->GetMainSequencer()->ApplyEffectSettingToSelected("Pictures", "E_TEXTCTRL_Pictures_Filename", selected, nullptr, "");
    }
}

void PicturesPanel::OnAIGenerateClick(wxCommandEvent& /*event*/) {
    xLightsFrame* xl = nullptr;
    for (wxWindow* w = GetParent(); w; w = w->GetParent()) {
        xl = dynamic_cast<xLightsFrame*>(w);
        if (xl) break;
    }
    if (!xl || !_filenameCtrl) return;

    auto services = xl->GetAIServices(aiType::TYPE::IMAGES);
    if (services.empty()) {
        wxMessageBox("No AI image generation services are registered.\n"
                     "Configure an AI service in the Settings menu.",
                     "AI Generate", wxICON_INFORMATION | wxOK, this);
        return;
    }

    aiBase* serv = services[0];
    if (services.size() > 1) {
        wxArrayString choices;
        for (auto* s : services) {
            choices.push_back(s->GetLLMName());
        }
        wxSingleChoiceDialog pick(this, "Choose AI Image Generator",
                                  "AI Image Generator", choices);
        if (pick.ShowModal() == wxID_CANCEL) return;
        serv = services[pick.GetSelection()];
    }

    AIImageDialog dlg(this, serv);
    dlg.SetEmbeddedMode(true);
    if (dlg.ShowModal() != wxID_OK) return;

    const wxImage& img = dlg.GetCurrentImage();
    if (!img.IsOk()) return;

    // Embed the AI-generated image in the sequence's media so it travels
    // with the .xsq file rather than living as a loose file on disk.
    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    wxString name = wxString::Format("AIImages/ai_generated_%lld.png",
                                     (long long)wxDateTime::Now().GetTicks());
    media.AddEmbeddedImage(name.ToStdString(), wxImageToXlImage(img));

    _filenameCtrl->SetValue(name);
    UpdatePreviewBitmap(name);
    FireChangeEvent();
}

void PicturesPanel::OnClearClick(wxCommandEvent& /*event*/) {
    if (!_filenameCtrl) return;
    _filenameCtrl->SetValue("");
    UpdatePreviewBitmap("");
    FireChangeEvent();
}

void PicturesPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();
    if (_filenameCtrl) {
        _filenameCtrl->ChangeValue(wxEmptyString);
    }
    UpdatePreviewBitmap(wxEmptyString);
}

void PicturesPanel::SetPanelStatus(Model* cls) {
    JsonEffectPanel::SetPanelStatus(cls);
    _currentModel = cls;
}

void PicturesPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    bool isVector = _directionChoice && _directionChoice->GetStringSelection() == "vector";

    // Value curves on the start XC/YC don't make sense in vector mode (the
    // animation runs from start to end position over the effect duration);
    // disable them visually so the user doesn't try to combine the two.
    if (_xcVcButton) {
        if (isVector) {
            _xcVcButton->SetToolTip("Value curve cannot be used on vector movement.");
        } else {
            _xcVcButton->UnsetToolTip();
        }
    }
    if (_ycVcButton) {
        if (isVector) {
            _ycVcButton->SetToolTip("Value curve cannot be used on vector movement.");
        } else {
            _ycVcButton->UnsetToolTip();
        }
    }

    // Refresh the preview from whatever's in the hidden filename control —
    // also re-evaluates LoopGIF / SuppressGIFBackground enable based on
    // whether the current image is an animated GIF.
    if (_filenameCtrl) {
        UpdatePreviewBitmap(_filenameCtrl->GetValue());
    }
}

void PicturesPanel::UpdatePreviewBitmap(const wxString& filename) {
    _previewTimer.Stop();
    _previewFrames.clear();
    _previewFrameTimes.clear();
    _currentPreviewFrame = 0;

    auto refreshPreview = [this](const wxBitmap& bmp, bool loopEnable) {
        if (_previewBitmap) {
            _previewBitmap->SetBitmap(bmp);
            _previewBitmap->InvalidateBestSize();
            _previewBitmap->Refresh();
            _previewBitmap->Update();
            if (_previewBitmap->GetParent()) _previewBitmap->GetParent()->Layout();
        }
        if (_loopGifCheck) _loopGifCheck->Enable(loopEnable);
        if (_suppressGifBgCheck) _suppressGifBgCheck->Enable(loopEnable);
    };

    auto makeRedBitmap = []() {
        wxImage img(64, 64);
        img.SetRGB(wxRect(0, 0, 64, 64), 255, 0, 0);
        return wxBitmap(img);
    };

    if (_filenameLabel) _filenameLabel->SetLabel(filename);

    if (filename.IsEmpty()) {
        refreshPreview(makeRedBitmap(), false);
        return;
    }

    // Use explicitly provided media (test harness) or walk parents to find xLightsFrame.
    SequenceMedia* media = _sequenceMedia;
    if (media == nullptr) {
        for (wxWindow* w = GetParent(); w; w = w->GetParent()) {
            if (auto* xl = dynamic_cast<xLightsFrame*>(w)) {
                media = &xl->GetSequenceElements().GetSequenceMedia();
                break;
            }
        }
    }
    if (media == nullptr) {
        refreshPreview(makeRedBitmap(), false);
        return;
    }

    auto entry = media->GetImage(filename.ToStdString());
    if (!entry || !entry->IsOk()) {
        refreshPreview(makeRedBitmap(), false);
        return;
    }

    auto img = entry->GetFrame(0, false);
    if (!img || !img->IsOk()) {
        refreshPreview(makeRedBitmap(), false);
        return;
    }

    // Fixed generation size — ShowPreviewFrame scales to fit the widget.
    int genSize = 300;
    entry->GeneratePreview(genSize, genSize);

    for (size_t i = 0; i < entry->GetPreviewFrameCount(); i++) {
        _previewFrames.push_back(entry->GetPreviewFrame(i));
        _previewFrameTimes.push_back(entry->GetPreviewFrameTime(i));
    }

    ShowPreviewFrame(0);

    bool isAnimated = entry->IsFrameBasedAnimation();
    if (_loopGifCheck) _loopGifCheck->Enable(!isAnimated);
    if (_suppressGifBgCheck) _suppressGifBgCheck->Enable(!isAnimated);

    if (_previewFrames.size() > 1) {
        long interval = (_previewFrameTimes[0] > 0) ? _previewFrameTimes[0] : 50;
        _previewTimer.Start(interval);
    }
}

void PicturesPanel::OnPreviewTimer(wxTimerEvent& /*event*/) {
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

void PicturesPanel::ShowPreviewFrame(size_t index) {
    if (index >= _previewFrames.size() || !_previewFrames[index] || !_previewFrames[index]->IsOk()) return;
    if (!_previewBitmap) return;

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

AssistPanel* PicturesPanel::GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) {
    auto* assist_panel = new AssistPanel(parent);
    auto* grid = new xlGridCanvasPictures(assist_panel->GetCanvasParent(), wxNewId(),
                                           wxDefaultPosition, wxDefaultSize,
                                           wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE,
                                           _T("PicturesGrid"));
    assist_panel->SetGridCanvas(grid);
    auto* picture_panel = new PicturesAssistPanel(assist_panel->GetCanvasParent());
    picture_panel->SetxLightsFrame(xl_frame);
    assist_panel->AddPanel(picture_panel);
    picture_panel->SetGridCanvas(grid);
    grid->SetMessageParent(picture_panel);
    if (xl_frame != nullptr) {
        grid->SetSequenceMedia(&xl_frame->GetSequenceElements().GetSequenceMedia());
        grid->SetXLightsFrame(xl_frame, &xl_frame->GetSequenceElements());
    }
    return assist_panel;
}
