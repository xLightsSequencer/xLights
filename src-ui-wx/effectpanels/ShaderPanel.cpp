/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShaderPanel.h"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/progdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include "shared/controls/BulkEditControls.h"
#include "shared/utils/wxUtilities.h"
#include "../effects/ShaderDownloadDialog.h"
#include "../media/ManageMediaPanel.h"
#include "../sequencer/MainSequencer.h"
#include "../media/ShaderPreviewGenerator.h"
#include "../sequencer/BlendingPanel.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "effects/ShaderEffect.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "utils/ExternalHooks.h"
#include "utils/xlImage.h"
#include "UtilFunctions.h"

#ifndef __WXOSX__
class ShaderPreview : public xlGLCanvas {
public:
    ShaderPreview(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = 0,
                  const wxString& name = wxPanelNameStr,
                  bool coreProfile = true) :
        xlGLCanvas(parent, id, pos, size, style, name, coreProfile) {}
    ~ShaderPreview() override = default;

    void InitializeGLContext() override {
        SetCurrentGLContext();
    }
};

const long ShaderPanel::ID_CANVAS = wxNewId();
#endif

ShaderPanel::ShaderPanel(wxWindow* parent, const nlohmann::json& metadata) :
    JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    _previewTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &ShaderPanel::OnPreviewTimer, this, _previewTimer.GetId());

    // Pause the preview timer when the panel is hidden (user switches to a
    // different effect) and resume when it's shown again. The effect panels
    // are cached by EffectPanelManager so the timer would otherwise keep
    // decoding / rescaling frames for a hidden widget. Bound as a member
    // function (not a lambda) so the destructor can Unbind it — otherwise
    // a wxEVT_SHOW dispatched from the Win32 HWND teardown after
    // ~ShaderPanel has already destroyed _previewFrames / _previewTimer
    // would access destroyed members and crash on exit.
    Bind(wxEVT_SHOW, &ShaderPanel::OnShowPanel, this);

#ifndef __WXOSX__
    // Hidden Linux GL canvas used as a context provider for the shader renderer.
    // Created off-screen (not added to any sizer) — xLightsMain activates its
    // GL context before doing any shader rendering.
    _preview = new ShaderPreview(this, ID_CANVAS);
    _preview->Hide();
#endif

    ValidateWindow();
}

ShaderPanel::~ShaderPanel() {
    Unbind(wxEVT_SHOW, &ShaderPanel::OnShowPanel, this);
    _previewTimer.Stop();
    // _shaderConfig is owned by ShaderMediaCacheEntry; do not delete here.
}

void ShaderPanel::OnShowPanel(wxShowEvent& event) {
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

wxWindow* ShaderPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                            const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Shader_FilenameBlock") {
        return BuildFilenameBlock(parentWin, sizer, cols);
    }
    if (id == "Shader_SpeedRow") {
        return BuildSpeedRow(parentWin, sizer, cols);
    }
    if (id == "Shader_DynamicParams") {
        return BuildDynamicParams(parentWin, sizer, cols);
    }
    return nullptr;
}

wxWindow* ShaderPanel::BuildFilenameBlock(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Two-column block: button stack on the left, preview bitmap on the right,
    // filename label below. Hidden file picker holds the actual path for
    // serialization as E_0FILEPICKERCTRL_IFS (the leading 0 forces it to be
    // loaded first so dynamic uniform controls can bind to an already-loaded
    // shader config).
    auto* outer = new wxFlexGridSizer(0, 1, 0, 0);
    outer->AddGrowableCol(0);

    auto* topRow = new wxFlexGridSizer(0, 2, 0, 0);
    topRow->AddGrowableCol(1);

    auto* buttonSizer = new wxFlexGridSizer(0, 1, 0, 0);
    auto* selectRow = new wxBoxSizer(wxHORIZONTAL);
    _selectButton = new wxButton(parentWin, wxID_ANY, "Select...");
    selectRow->Add(_selectButton, 1, wxRIGHT, 2);
    wxBitmap clearBmp = wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON);
    _clearButton = new wxBitmapButton(parentWin, wxID_ANY, clearBmp, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    selectRow->Add(_clearButton, 0, 0, 0);
    buttonSizer->Add(selectRow, 0, wxALL | wxEXPAND, 5);

    _downloadButton = new wxButton(parentWin, wxID_ANY, "Download");
    buttonSizer->Add(_downloadButton, 0, wxALL | wxEXPAND, 5);
    topRow->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    _previewBitmap = new wxStaticBitmap(parentWin, wxID_ANY, wxNullBitmap);
    _previewBitmap->SetMinSize(wxDLG_UNIT(parentWin, wxSize(0, 50)));
    topRow->Add(_previewBitmap, 1, wxALL | wxEXPAND, 5);

    outer->Add(topRow, 1, wxALL | wxEXPAND, 0);

    _filenameLabel = new wxStaticText(parentWin, wxID_ANY, wxEmptyString,
                                       wxDefaultPosition, wxDefaultSize,
                                       wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE);
    outer->Add(_filenameLabel, 0, wxLEFT | wxRIGHT | wxEXPAND, 5);

    // Hidden file picker holds the actual path. The leading "0" in the control
    // name is intentional: it makes this setting sort first alphabetically in
    // the SettingsMap so the shader config is available when dynamic uniform
    // settings are applied downstream.
    _hiddenFilePicker = new BulkEditFilePickerCtrl(parentWin, wxNewId(),
                                                    wxEmptyString,
                                                    wxEmptyString,
                                                    _T("*.fs"),
                                                    wxDefaultPosition, wxDefaultSize,
                                                    wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                    wxDefaultValidator,
                                                    _T("ID_0FILEPICKERCTRL_IFS"));
    _hiddenFilePicker->Hide();
    outer->Add(_hiddenFilePicker, 0, 0, 0);

    sizer->Add(outer, 1, wxALL | wxEXPAND, 2);

    // Bind events
    _selectButton->Bind(wxEVT_BUTTON, &ShaderPanel::OnSelectClicked, this);
    _clearButton->Bind(wxEVT_BUTTON, &ShaderPanel::OnClearClicked, this);
    _downloadButton->Bind(wxEVT_BUTTON, &ShaderPanel::OnDownloadClicked, this);
    _hiddenFilePicker->Bind(wxEVT_FILEPICKER_CHANGED, &ShaderPanel::OnFilePickerChanged, this);

    return _selectButton;
}

wxWindow* ShaderPanel::BuildSpeedRow(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Custom Time Speed row: BulkEditSliderF2 is the primary serialized control
    // (E_SLIDER_Shader_Speed with divisor 100) with buddy text + value curve,
    // matching the legacy naming so old sequences round-trip. Wrap everything
    // in a 4-col flex grid so the row matches the layout of the framework-
    // built rows (label | slider+VC | text | spacer).
    auto* row = new wxFlexGridSizer(0, 4, 0, 0);
    row->AddGrowableCol(1);

    _speedLabel = new wxStaticText(parentWin, wxID_ANY, "Time Speed");
    row->Add(_speedLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    auto* sliderSizer = new wxFlexGridSizer(0, 2, 0, 0);
    sliderSizer->AddGrowableCol(0);

    _speedSlider = new BulkEditSliderF2(parentWin, wxNewId(), 100, -1000, 1000,
                                         wxDefaultPosition, wxDefaultSize, 0,
                                         wxDefaultValidator,
                                         _T("ID_SLIDER_Shader_Speed"));
    sliderSizer->Add(_speedSlider, 1, wxALL | wxEXPAND, 2);

    _speedVC = new BulkEditValueCurveButton(parentWin, wxNewId(), GetValueCurveNotSelectedBitmap(),
                                             wxDefaultPosition, wxDefaultSize,
                                             wxBU_AUTODRAW | wxBORDER_NONE,
                                             wxDefaultValidator,
                                             _T("ID_VALUECURVE_Shader_Speed"));
    _speedVC->GetValue()->SetLimits(SHADER_SPEED_MIN, SHADER_SPEED_MAX);
    _speedVC->GetValue()->SetDivisor(SHADER_SPEED_DIVISOR);
    sliderSizer->Add(_speedVC, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

    row->Add(sliderSizer, 1, wxALL | wxEXPAND, 0);

    _speedText = new BulkEditTextCtrlF2(parentWin, wxNewId(), _("1.00"),
                                         wxDefaultPosition,
                                         wxDLG_UNIT(parentWin, wxSize(20, -1)),
                                         0, wxDefaultValidator,
                                         _T("IDD_TEXTCTRL_Shader_Speed"));
    _speedText->SetMaxLength(5);
    row->Add(_speedText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

    row->Add(-1, -1, 1, wxALL, 1);

    sizer->Add(row, 1, wxALL | wxEXPAND, 0);

    // Value curve clicks go through the standard panel handler.
    _speedVC->Bind(wxEVT_BUTTON, &ShaderPanel::OnVCButtonClick, this);

    return _speedSlider;
}

wxWindow* ShaderPanel::BuildDynamicParams(wxWindow* parentWin, wxSizer* sizer, int cols) {
    // Container for dynamic shader uniforms. Filled by BuildDynamicUI() on
    // shader load and wiped on shader change. Return parentWin so the
    // framework sees a non-null result and doesn't emit a spurious warning
    // about CreateCustomControl returning null — the row has no single
    // primary control to anchor a tooltip, but this property has no JSON
    // tooltip so parentWin is an inert fallback.
    _dynamicSizer = new wxFlexGridSizer(0, 3, 0, 0);
    _dynamicSizer->AddGrowableCol(1);
    sizer->Add(_dynamicSizer, 1, wxALL | wxEXPAND, 2);
    return parentWin;
}

void ShaderPanel::OnSelectClicked(wxCommandEvent& /*event*/) {
    auto* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
    if (xl == nullptr || _hiddenFilePicker == nullptr) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    SequenceElements& elements = xl->GetSequenceElements();
    std::string currentPath = _hiddenFilePicker->GetFileName().GetFullPath().ToStdString();
    SelectMediaDialog dlg(this, &media, &elements, xl->GetShowDirectory(), xl,
                          MediaType::Shader, currentPath);
    if (dlg.ShowModal() != wxID_OK) return;

    std::string selected = dlg.GetSelectedPath();
    if (selected.empty()) return;

    _hiddenFilePicker->SetFileName(wxFileName(ToWXString(selected)));
    wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, _hiddenFilePicker,
                             _hiddenFilePicker->GetId(), ToWXString(selected));
    // Dispatch on the picker itself — OnFilePickerChanged is Bind()'d to
    // _hiddenFilePicker, so the event must be processed through the picker's
    // handler chain. Processing on `this` walks ShaderPanel's handlers then
    // propagates up to the parent and never reaches the bound handler.
    _hiddenFilePicker->ProcessWindowEvent(evt);

    if (xl->GetMainSequencer() && xl->GetMainSequencer()->GetSelectedEffectCount("Shader") > 1) {
        std::string id = FixIdForPanel("Effect", _hiddenFilePicker->GetName().ToStdString());
        xl->GetMainSequencer()->ApplyEffectSettingToSelected("Shader", id + "_FN", selected, nullptr, "");
    }
}

void ShaderPanel::OnClearClicked(wxCommandEvent& /*event*/) {
    if (_hiddenFilePicker == nullptr) return;
    _hiddenFilePicker->SetFileName(wxFileName());
    wxFileDirPickerEvent evt(wxEVT_FILEPICKER_CHANGED, _hiddenFilePicker,
                             _hiddenFilePicker->GetId(), "");
    _hiddenFilePicker->ProcessWindowEvent(evt);
}

void ShaderPanel::OnDownloadClicked(wxCommandEvent& /*event*/) {
    wxProgressDialog prog("Shader download", "Downloading shaders ...", 100, this,
                          wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();
    ShaderDownloadDialog dlg(this);
    SetCursor(wxCURSOR_WAIT);
    if (dlg.DlgInit(&prog, 0, 100)) {
        prog.Update(100);
        SetCursor(wxCURSOR_DEFAULT);
        if (dlg.ShowModal() == wxID_OK && _hiddenFilePicker) {
            _hiddenFilePicker->SetFileName(wxFileName(dlg.GetShaderFile()));
            wxFileDirPickerEvent e(wxEVT_FILEPICKER_CHANGED, _hiddenFilePicker,
                                   _hiddenFilePicker->GetId(),
                                   _hiddenFilePicker->GetFileName().GetFullPath());
            // Post to the picker, not `this` — see OnSelectClicked above.
            wxPostEvent(_hiddenFilePicker, e);
        }
    } else {
        SetCursor(wxCURSOR_DEFAULT);
    }
}

void ShaderPanel::OnFilePickerChanged(wxFileDirPickerEvent& event) {
    wxString fullPath = event.GetPath();
    ObtainAccessToURL(fullPath.ToStdString());

    ValidateWindow();

    // Reset time / coordinate parameters to defaults on shader change.
    if (_speedVC) _speedVC->SetActive(false);
    if (_speedSlider) _speedSlider->SetValue(100);
    if (_speedText) _speedText->SetValue("1.0");

    auto resetVC = [this](const char* name) {
        if (auto* vc = dynamic_cast<BulkEditValueCurveButton*>(
                wxWindow::FindWindowByName(name, this))) {
            vc->SetActive(false);
        }
    };
    auto resetText = [this](const char* name, const wxString& value) {
        if (auto* txt = dynamic_cast<wxTextCtrl*>(
                wxWindow::FindWindowByName(name, this))) {
            txt->SetValue(value);
        }
    };
    resetVC("ID_VALUECURVE_Shader_Offset_X");
    resetVC("ID_VALUECURVE_Shader_Offset_Y");
    resetVC("ID_VALUECURVE_Shader_Zoom");
    resetText("ID_TEXTCTRL_Shader_LeadIn", "0");
    resetText("ID_TEXTCTRL_Shader_Offset_X", "0");
    resetText("ID_TEXTCTRL_Shader_Offset_Y", "0");
    resetText("ID_TEXTCTRL_Shader_Zoom", "0");

    if (fullPath.empty()) {
        Freeze();
        _shaderCacheEntry.reset();
        _shaderConfig = nullptr;
        ClearDynamicUI();
        if (_hiddenFilePicker) _hiddenFilePicker->UnsetToolTip();
        Thaw();
        CallAfter([this]() { UpdatePreview(); });
        FireChangeEvent();
        return;
    }

    // Resolve shader through SequenceMedia (handles relative paths + embedded shaders).
    // Null out the config pointer first — it's owned by the current cache
    // entry and the reassignment below may drop the last ref, freeing the
    // config we'd otherwise leave dangling if we return early.
    auto* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
    if (xl == nullptr) return;
    _shaderConfig = nullptr;
    auto& media = xl->GetSequenceElements().GetSequenceMedia();
    _shaderCacheEntry = media.GetShader(fullPath.ToStdString());
    if (!_shaderCacheEntry) {
        return;
    }
    _shaderCacheEntry->MarkIsUsed();
    if (!_shaderCacheEntry->GetShaderSource().empty()) {
        ApplyShaderConfig(/*resetParams*/ true);
        if (_hiddenFilePicker) _hiddenFilePicker->Enable(true); // trigger a re-validate
    } else {
        Freeze();
        _shaderCacheEntry.reset();
        _shaderConfig = nullptr;
        ClearDynamicUI();
        if (_hiddenFilePicker) _hiddenFilePicker->UnsetToolTip();
        Thaw();
    }

    // Defer so GenerateShaderPreview's render loop doesn't re-enter event handlers.
    CallAfter([this]() { UpdatePreview(); });
    FireChangeEvent();
}

void ShaderPanel::ClearDynamicUI() {
    if (_dynamicSizer) {
        _dynamicSizer->Clear(/*delete_windows*/ true);
    }
    // _dynamicSizer->Clear(true) destroyed the wx controls but left their
    // PropertyInfo pointers dangling inside the JsonEffectPanel properties_
    // map. Purge them so walkers (SetRenderableEffect, visibility rules, ...)
    // don't dereference destroyed widgets on the next rebuild.
    RemovePropertiesWithPrefix("SHADERXYZZY_");
}

void ShaderPanel::ApplyShaderConfig(bool resetParams) {
    if (_shaderCacheEntry == nullptr) return;

    auto* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
    if (xl == nullptr) return;

    Freeze();

    ClearDynamicUI();
    if (_hiddenFilePicker) _hiddenFilePicker->UnsetToolTip();

    _shaderConfig = _shaderCacheEntry->GetShaderConfig(&xl->GetSequenceElements());

    if (_shaderConfig != nullptr) {
        wxString desc = _shaderConfig->GetDescription();
        if (desc != "") desc += "\n";
        if (_shaderConfig->IsCanvasShader()) {
            desc += "Use Canvas Mode for this shader.";
            BlendingPanel* layerBlendingPanel = xl->GetLayerBlendingPanel();
            layerBlendingPanel->CheckBox_Canvas->SetValue(true);
        }
        wxString const shortName = wxFileName(_shaderConfig->GetFilename()).GetFullName();
        if (_hiddenFilePicker) _hiddenFilePicker->SetToolTip(shortName + "\n\n" + desc);

        ShowHideStaticControls(_shaderConfig->HasTime(), _shaderConfig->HasCoord());

        BuildDynamicUI();
    } else {
        ShowHideStaticControls(false, false);
    }

    Layout();

    if (auto* sw = dynamic_cast<wxScrolledWindow*>(GetParent())) {
        sw->FitInside();
        sw->SetScrollRate(5, 5);
        sw->Refresh();
    }

    Thaw();
}

void ShaderPanel::ShowHideStaticControls(bool hasTime, bool hasCoord) {
    // Helper: show/hide framework-built rows by looking up each of the four
    // controls the framework generates for an int-slider row (label, slider,
    // text, optional value-curve button).
    auto showRow = [this](const char* id, bool show, bool hasVC) {
        auto* label = wxWindow::FindWindowByName(wxString("ID_STATICTEXT_") + id, this);
        auto* slider = wxWindow::FindWindowByName(wxString("IDD_SLIDER_") + id, this);
        auto* text = wxWindow::FindWindowByName(wxString("ID_TEXTCTRL_") + id, this);
        auto* vc = hasVC ? wxWindow::FindWindowByName(wxString("ID_VALUECURVE_") + id, this) : nullptr;
        if (label) label->Show(show);
        if (slider) slider->Show(show);
        if (text) text->Show(show);
        if (vc) vc->Show(show);
    };

    showRow("Shader_LeadIn", hasTime, /*hasVC*/ false);
    if (_speedLabel) _speedLabel->Show(hasTime);
    if (_speedSlider) _speedSlider->Show(hasTime);
    if (_speedText) _speedText->Show(hasTime);
    if (_speedVC) _speedVC->Show(hasTime);

    showRow("Shader_Offset_X", hasCoord, /*hasVC*/ true);
    showRow("Shader_Offset_Y", hasCoord, /*hasVC*/ true);
    showRow("Shader_Zoom", hasCoord, /*hasVC*/ true);
}

void ShaderPanel::BuildDynamicUI() {
    if (_shaderConfig == nullptr || _dynamicSizer == nullptr) return;

    wxWindow* parentWin = _dynamicSizer->GetContainingWindow();
    if (parentWin == nullptr) parentWin = this;

    // Core ShaderConfig emits a schema-shaped JSON array for every visible
    // uniform; the JsonEffectPanel builder then lays out each row exactly
    // like a static property. Using the same builder for dynamic rows means
    // iPad (which has no wxWidgets but speaks the same JSON) can render the
    // panel from an identical description — and any future additions to the
    // schema (new control types, etc.) arrive here for free.
    const int dynamicCols = 3;
    BuildPropertiesIntoSizer(parentWin, _dynamicSizer,
                             _shaderConfig->GetDynamicPropertiesJson(),
                             dynamicCols);
}

void ShaderPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    if (_hiddenFilePicker == nullptr) return;

    wxString file = _hiddenFilePicker->GetFileName().GetFullPath();
    bool fileExists = file.empty() || FileExists(file);
    if (!fileExists) {
        auto* xl = dynamic_cast<xLightsFrame*>(xLightsApp::GetFrame());
        if (xl) {
            fileExists = xl->GetSequenceElements().GetSequenceMedia().HasMedia(file.ToStdString());
        }
    }
    if (!file.empty() && !fileExists) {
        _hiddenFilePicker->SetBackgroundColour(*wxRED);
        SetToolTip("File " + file + " does not exist.");
    } else if (!file.empty() && !IsXmlSafe(file)) {
        _hiddenFilePicker->SetBackgroundColour(*wxYELLOW);
        SetToolTip("File " + file + " contains characters in the path or filename that will cause issues in xLights. Please rename it.");
    } else {
        _hiddenFilePicker->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
        SetToolTip(file);
    }
}

void ShaderPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();

    _shaderCacheEntry.reset();
    _shaderConfig = nullptr;

    if (_speedVC) _speedVC->SetActive(false);
    if (_speedSlider) _speedSlider->SetValue(100);
    if (_speedText) _speedText->SetValue("1.00");

    if (_hiddenFilePicker) _hiddenFilePicker->SetFileName(wxFileName());

    auto resetVC = [this](const char* name) {
        if (auto* vc = dynamic_cast<BulkEditValueCurveButton*>(
                wxWindow::FindWindowByName(name, this))) {
            vc->SetActive(false);
        }
    };
    resetVC("ID_VALUECURVE_Shader_Offset_X");
    resetVC("ID_VALUECURVE_Shader_Offset_Y");
    resetVC("ID_VALUECURVE_Shader_Zoom");

    ClearDynamicUI();
}

void ShaderPanel::UpdatePreview() {
    _previewTimer.Stop();
    _previewFrames.clear();
    _previewFrameTimes.clear();
    _currentPreviewFrame = 0;

    if (!_previewBitmap) return;

    std::string file = _hiddenFilePicker ? _hiddenFilePicker->GetFileName().GetFullPath().ToStdString() : "";
    if (_filenameLabel) _filenameLabel->SetLabel(file);
    if (file.empty()) {
        _previewBitmap->SetBitmap(wxNullBitmap);
        return;
    }

    xLightsFrame* xl = nullptr;
    wxWindow* w = GetParent();
    while (w) {
        xl = dynamic_cast<xLightsFrame*>(w);
        if (xl) break;
        w = w->GetParent();
    }
    if (!xl) return;

    SequenceMedia& media = xl->GetSequenceElements().GetSequenceMedia();
    auto entry = media.GetShader(file);
    if (!entry || entry->GetShaderSource().empty()) {
        _previewBitmap->SetBitmap(wxNullBitmap);
        return;
    }

    GenerateShaderPreview(entry.get(), xl);

    for (size_t i = 0; i < entry->GetPreviewFrameCount(); i++) {
        _previewFrames.push_back(entry->GetPreviewFrame(i));
        _previewFrameTimes.push_back(entry->GetPreviewFrameTime(i));
    }

    if (_previewFrames.empty()) return;

    if (_previewBitmap->GetSize().x <= 0) {
        Layout();
    }

    ShowPreviewFrame(0);

    if (_previewFrames.size() > 1) {
        long interval = (_previewFrameTimes[0] > 0) ? _previewFrameTimes[0] : 50;
        _previewTimer.Start(interval);
    }
}

void ShaderPanel::OnPreviewTimer(wxTimerEvent& /*event*/) {
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

void ShaderPanel::ShowPreviewFrame(size_t index) {
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
    if (_previewBitmap->GetParent()) {
        _previewBitmap->GetParent()->Layout();
    }
}
