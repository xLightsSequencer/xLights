/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoSettingsPanel.h"
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include <wx/preferences.h>

#include "xLightsMain.h"

VideoSettingsPanel::VideoSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    HardwareVideoDecodingCheckBox = new wxCheckBox(this, wxID_ANY, _("Hardware Video Decoding"));
    sizer->Add(HardwareVideoDecodingCheckBox, 0, wxALL, 5);

    auto* renderRow = new wxBoxSizer(wxHORIZONTAL);
    renderRow->Add(new wxStaticText(this, wxID_ANY, _("Hardware Video Renderer:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    HardwareVideoRenderChoice = new wxChoice(this, wxID_ANY);
    HardwareVideoRenderChoice->Append(_("DirectX11"));
    HardwareVideoRenderChoice->SetSelection(HardwareVideoRenderChoice->Append(_("FFmpeg Auto")));
    HardwareVideoRenderChoice->Append(_("FFmpeg CUDA"));
    HardwareVideoRenderChoice->Append(_("FFmpeg QSV"));
    HardwareVideoRenderChoice->Append(_("FFmpeg Vulkan"));
    HardwareVideoRenderChoice->Append(_("FFmpeg AMF"));
    HardwareVideoRenderChoice->Append(_("FFmpeg DirectX11"));
    renderRow->Add(HardwareVideoRenderChoice, 1, wxEXPAND);
    sizer->Add(renderRow, 0, wxEXPAND | wxALL, 5);

    sizer->Add(MakePreferenceSectionHeader(this, _("Video Export Settings")), 0, wxLEFT | wxTOP, 10);
    auto* grid = new wxFlexGridSizer(0, 2, 0, 0);
    grid->AddGrowableCol(1);
    grid->Add(new wxStaticText(this, wxID_ANY, _("Video Codec:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    ChoiceCodec = new wxChoice(this, wxID_ANY);
    ChoiceCodec->Append(_("Auto"));
    ChoiceCodec->SetSelection(ChoiceCodec->Append(_("H.264")));
    ChoiceCodec->Append(_("H.265"));
    ChoiceCodec->Append(_("MPEG-4"));
    grid->Add(ChoiceCodec, 1, wxALL | wxEXPAND, 5);
    grid->Add(new wxStaticText(this, wxID_ANY, _("Bitrate(KB/s,0=Auto):")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrlDoubleBitrate = new wxSpinCtrlDouble(this, wxID_ANY, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 90000, 0, 1000);
    SpinCtrlDoubleBitrate->SetValue(0);
    grid->Add(SpinCtrlDoubleBitrate, 1, wxALL | wxEXPAND, 5);
    sizer->Add(grid, 0, wxEXPAND | wxLEFT, 16);

    SetSizer(sizer);
    sizer->SetSizeHints(this);

    // The hardware video renderer choice is only honoured on Windows; other
    // platforms decode without the selectable backend (mirrors the prior panel).
#ifdef __LINUX__
    HardwareVideoDecodingCheckBox->Hide();
    HardwareVideoRenderChoice->Hide();
#endif
#ifdef __WXOSX__
    HardwareVideoRenderChoice->Hide();
#endif

    HardwareVideoDecodingCheckBox->Bind(wxEVT_CHECKBOX, &VideoSettingsPanel::OnControlChanged, this);
    HardwareVideoRenderChoice->Bind(wxEVT_CHOICE, &VideoSettingsPanel::OnControlChanged, this);
    ChoiceCodec->Bind(wxEVT_CHOICE, &VideoSettingsPanel::OnControlChanged, this);
    SpinCtrlDoubleBitrate->Bind(wxEVT_SPINCTRLDOUBLE, &VideoSettingsPanel::OnBitrateChanged, this);

    TransferDataToWindow();
}

bool VideoSettingsPanel::TransferDataFromWindow() {
    frame->SetHardwareVideoAccelerated(HardwareVideoDecodingCheckBox->IsChecked());
#ifdef __WXMSW__
    frame->SetHardwareVideoRenderer(HardwareVideoRenderChoice->GetSelection());
    HardwareVideoRenderChoice->Enable(HardwareVideoDecodingCheckBox->IsChecked());
#endif
    frame->SetVideoExportCodec(ChoiceCodec->GetStringSelection());
    frame->SetVideoExportBitrate(SpinCtrlDoubleBitrate->GetValue());
    return true;
}

bool VideoSettingsPanel::TransferDataToWindow() {
    HardwareVideoDecodingCheckBox->SetValue(frame->HardwareVideoAccelerated());
#ifdef __WXMSW__
    HardwareVideoRenderChoice->SetSelection(frame->HardwareVideoRenderer());
    HardwareVideoRenderChoice->Enable(frame->HardwareVideoAccelerated());
#endif
    ChoiceCodec->SetStringSelection(frame->GetVideoExportCodec());
    SpinCtrlDoubleBitrate->SetValue(frame->GetVideoExportBitrate());
    return true;
}

void VideoSettingsPanel::ApplyIfImmediate() {
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void VideoSettingsPanel::OnControlChanged(wxCommandEvent& event) {
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    } else {
#ifdef __WXMSW__
        frame->SetHardwareVideoRenderer(HardwareVideoRenderChoice->GetSelection());
        HardwareVideoRenderChoice->Enable(HardwareVideoDecodingCheckBox->IsChecked());
#endif
    }
}

void VideoSettingsPanel::OnBitrateChanged(wxSpinDoubleEvent& event) {
    ApplyIfImmediate();
}
