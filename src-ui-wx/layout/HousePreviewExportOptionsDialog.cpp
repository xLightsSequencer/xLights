/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "HousePreviewExportOptionsDialog.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "settings/XLightsConfigAdapter.h"

#include <log.h>

namespace
{
    constexpr int CUSTOM_RESOLUTION_INDEX_SENTINEL = -1;
    const char* const LAST_EXPORT_WIDTH_KEY = "HousePreviewExportLastWidth";
    const char* const LAST_EXPORT_HEIGHT_KEY = "HousePreviewExportLastHeight";
}

HousePreviewExportOptionsDialog::HousePreviewExportOptionsDialog(wxWindow* parent, int currentWidth, int currentHeight) :
    wxDialog(parent, wxID_ANY, _("Export House Preview Video"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
    _currentWidth(currentWidth), _currentHeight(currentHeight)
{
    _presets.push_back({ wxString::Format(_("Current House Preview Size (%d x %d)"), currentWidth, currentHeight), currentWidth, currentHeight });
    _presets.push_back({ "1280 x 720 (HD)", 1280, 720 });
    _presets.push_back({ "1920 x 1080 (Full HD)", 1920, 1080 });
    _presets.push_back({ "2560 x 1440 (QHD)", 2560, 1440 });
    _presets.push_back({ "3840 x 2160 (4K UHD)", 3840, 2160 });
    _presets.push_back({ _("Custom"), CUSTOM_RESOLUTION_INDEX_SENTINEL, CUSTOM_RESOLUTION_INDEX_SENTINEL });

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* grid = new wxFlexGridSizer(0, 2, 5, 10);
    grid->AddGrowableCol(1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Resolution:")), 0, wxALIGN_CENTER_VERTICAL);
    _resolutionChoice = new wxChoice(this, wxID_ANY);
    for (auto const& preset : _presets) {
        _resolutionChoice->Append(preset.label);
    }
    grid->Add(_resolutionChoice, 1, wxEXPAND);

    wxSize spinSize = wxSize(FromDIP(80), -1);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Width:")), 0, wxALIGN_CENTER_VERTICAL);
    _widthSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, spinSize, wxSP_ARROW_KEYS, 16, 7680, currentWidth);
    grid->Add(_widthSpin, 0, wxALIGN_LEFT);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Height:")), 0, wxALIGN_CENTER_VERTICAL);
    _heightSpin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, spinSize, wxSP_ARROW_KEYS, 16, 4320, currentHeight);
    grid->Add(_heightSpin, 0, wxALIGN_LEFT);

    // Default to whatever resolution was chosen the last time this dialog was
    // used, rather than always resetting to the pane's current on-screen size.
    auto* config = GetXLightsConfig();
    int lastWidth = (int)config->Read(LAST_EXPORT_WIDTH_KEY, -1L);
    int lastHeight = (int)config->Read(LAST_EXPORT_HEIGHT_KEY, -1L);
    int initialSelection = 0;
    if (lastWidth > 0 && lastHeight > 0) {
        // currentWidth/currentHeight drive the spin controls below, which is
        // what GetExportWidth()/GetExportHeight() ultimately read -- these
        // must end up holding the *last-used* values whether or not they
        // matched a fixed preset, otherwise the dropdown shows the right
        // label but the spin controls (and thus the actual export) silently
        // fall back to the pane's current on-screen size.
        currentWidth = lastWidth;
        currentHeight = lastHeight;
        bool matchedPreset = false;
        for (size_t i = 0; i < _presets.size(); ++i) {
            if (_presets[i].width == lastWidth && _presets[i].height == lastHeight) {
                initialSelection = (int)i;
                matchedPreset = true;
                break;
            }
        }
        if (!matchedPreset) {
            // Didn't match a fixed preset (or the pane's current size) -- fall
            // back to Custom with the previously chosen dimensions.
            initialSelection = (int)_presets.size() - 1;
        }
    }
    spdlog::info("HousePreviewExportOptionsDialog: last saved {}x{}, selecting preset index {} ('{}'), spin values set to {}x{}",
                 lastWidth, lastHeight, initialSelection, _presets[initialSelection].label.ToStdString(), currentWidth, currentHeight);
    _resolutionChoice->SetSelection(initialSelection);
    _widthSpin->SetValue(currentWidth);
    _heightSpin->SetValue(currentHeight);

    mainSizer->Add(grid, 0, wxALL | wxEXPAND, 10);

    mainSizer->Add(new wxStaticText(this, wxID_ANY,
                                     _("The export is rendered at the chosen resolution, independent of the House\nPreview pane's current on-screen size. "
                                       "The House Preview pane will briefly resize on\nscreen while the export runs, then return to its original size.")),
                    0, wxALL, 10);

    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
    buttonSizer->AddButton(new wxButton(this, wxID_OK));
    buttonSizer->AddButton(new wxButton(this, wxID_CANCEL));
    buttonSizer->Realize();
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 10);

    SetSizerAndFit(mainSizer);
    CentreOnParent();

    bool isCustom = _presets[initialSelection].width == CUSTOM_RESOLUTION_INDEX_SENTINEL;
    _widthSpin->Enable(isCustom);
    _heightSpin->Enable(isCustom);

    _resolutionChoice->Bind(wxEVT_CHOICE, &HousePreviewExportOptionsDialog::OnResolutionChoice, this);
    Bind(wxEVT_BUTTON, &HousePreviewExportOptionsDialog::OnOk, this, wxID_OK);
}

void HousePreviewExportOptionsDialog::OnResolutionChoice(wxCommandEvent& event)
{
    int idx = _resolutionChoice->GetSelection();
    if (idx < 0 || (size_t)idx >= _presets.size()) {
        return;
    }
    auto const& preset = _presets[idx];
    bool isCustom = preset.width == CUSTOM_RESOLUTION_INDEX_SENTINEL;
    _widthSpin->Enable(isCustom);
    _heightSpin->Enable(isCustom);
    if (!isCustom) {
        _widthSpin->SetValue(preset.width);
        _heightSpin->SetValue(preset.height);
    }
}

void HousePreviewExportOptionsDialog::OnOk(wxCommandEvent& event)
{
    auto* config = GetXLightsConfig();
    config->Write(LAST_EXPORT_WIDTH_KEY, GetExportWidth());
    config->Write(LAST_EXPORT_HEIGHT_KEY, GetExportHeight());
    EndModal(wxID_OK);
}

int HousePreviewExportOptionsDialog::GetExportWidth() const
{
    return _widthSpin->GetValue();
}

int HousePreviewExportOptionsDialog::GetExportHeight() const
{
    return _heightSpin->GetValue();
}
