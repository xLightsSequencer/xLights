/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "OtherSettingsPanel.h"
#include "PrefPanelUtils.h"
#include "color/xlColourData.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include <wx/preferences.h>
#include "xLightsMain.h"

#ifdef __WXOSX__
extern "C" {
extern bool isMetalComputeSupported();
}
#endif

OtherSettingsPanel::OtherSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL), frame(f)
{
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    // Labelled fields, each followed by a greyed description line. The empty
    // first cell on each hint row keeps the hint aligned under the control.
    auto* fields = new wxFlexGridSizer(0, 2, 0, 0);
    fields->AddGrowableCol(1);

    fields->Add(new wxStaticText(this, wxID_ANY, _("eMail Address:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    eMailTextControl = new wxTextCtrl(this, wxID_ANY, _("noone@nowhere.xlights.org"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(180, -1)));
    fields->Add(eMailTextControl, 1, wxALL | wxEXPAND, 5);
    fields->Add(0, 0);
    fields->Add(MakePreferenceHint(this, _("Identifies you when downloading or submitting models to the vendor database.")), 0, wxLEFT | wxBOTTOM, 5);

    fields->Add(new wxStaticText(this, wxID_ANY, _("Link controller upload:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_LinkControllerUpload = new wxChoice(this, wxID_ANY);
    Choice_LinkControllerUpload->SetSelection(Choice_LinkControllerUpload->Append(_("None")));
    Choice_LinkControllerUpload->Append(_("Inputs and Outputs"));
    fields->Add(Choice_LinkControllerUpload, 1, wxALL | wxEXPAND, 5);
    fields->Add(0, 0);
    fields->Add(MakePreferenceHint(this, _("Whether uploading a controller's inputs also uploads its outputs.")), 0, wxLEFT | wxBOTTOM, 5);

    fields->Add(new wxStaticText(this, wxID_ANY, _("Model renaming alias behavior:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    Choice_AliasPromptBehavior = new wxChoice(this, wxID_ANY);
    Choice_AliasPromptBehavior->SetSelection(Choice_AliasPromptBehavior->Append(_("Always Prompt")));
    Choice_AliasPromptBehavior->Append(_("Always Yes"));
    Choice_AliasPromptBehavior->Append(_("Always No"));
    fields->Add(Choice_AliasPromptBehavior, 1, wxALL | wxEXPAND, 5);
    fields->Add(0, 0);
    fields->Add(MakePreferenceHint(this, _("When you rename a model, whether to keep its old name as an alias so existing sequences still find it.")), 0, wxLEFT | wxBOTTOM, 5);

    fields->Add(new wxStaticText(this, wxID_ANY, _("Controller ping interval in seconds (0=Off):")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    CtrlPingInterval = new wxSpinCtrlDouble(this, wxID_ANY, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 300, 0, 10);
    CtrlPingInterval->SetValue(0);
    fields->Add(CtrlPingInterval, 1, wxALL | wxEXPAND, 5);
    fields->Add(0, 0);
    fields->Add(MakePreferenceHint(this, _("How often to check that controllers are online. Any value above 0 adds a Status column to the Controllers screen.")), 0, wxLEFT | wxBOTTOM, 5);

    sizer->Add(fields, 0, wxEXPAND | wxALL, 5);

    // Standalone toggles, each with a description beneath.
    GPURenderCheckbox = new wxCheckBox(this, wxID_ANY, _("GPU Rendering"));
    GPURenderCheckbox->SetValue(true);
    sizer->Add(GPURenderCheckbox, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Render supported effects on the GPU for better performance.")), 0, wxLEFT | wxBOTTOM, 24);

    ShaderCheckbox = new wxCheckBox(this, wxID_ANY, _("Shaders on Background Threads"));
    sizer->Add(ShaderCheckbox, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Render shader effects off the main thread to keep xLights responsive.")), 0, wxLEFT | wxBOTTOM, 24);

    CheckBox_BatchRenderPromptIssues = new wxCheckBox(this, wxID_ANY, _("Prompt issues during batch render"));
    CheckBox_BatchRenderPromptIssues->SetValue(true);
    sizer->Add(CheckBox_BatchRenderPromptIssues, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Warn about problems found while batch rendering sequences.")), 0, wxLEFT | wxBOTTOM, 24);

    CheckBox_PurgeDownloadCache = new wxCheckBox(this, wxID_ANY, _("Purge download cache at startup"));
    sizer->Add(CheckBox_PurgeDownloadCache, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Clear cached downloads each time xLights starts.")), 0, wxLEFT | wxBOTTOM, 24);

    CheckBox_IgnoreVendorModelRecommendations = new wxCheckBox(this, wxID_ANY, _("Ignore vendor model recommendations"));
    sizer->Add(CheckBox_IgnoreVendorModelRecommendations, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Stop warning when a model differs from the vendor's recommended setup.")), 0, wxLEFT | wxBOTTOM, 24);

    CheckBox_UseCustomColorPicker = new wxCheckBox(this, wxID_ANY, _("Use custom color picker (experimental)"));
    sizer->Add(CheckBox_UseCustomColorPicker, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Use the built-in colour picker instead of the operating system's.")), 0, wxLEFT | wxBOTTOM, 24);

    // Packaging Sequences.
    sizer->Add(MakePreferenceSectionHeader(this, _("Packaging Sequences")), 0, wxLEFT | wxTOP, 10);
    sizer->Add(MakePreferenceHint(this, _("Media to leave out when packaging a sequence to share.")), 0, wxLEFT, 16);
    auto* packBox = new wxBoxSizer(wxVERTICAL);
    ExcludeVideosCheckBox = new wxCheckBox(this, wxID_ANY, _("Exclude Videos"));
    packBox->Add(ExcludeVideosCheckBox, 0, wxALL, 5);
    ExcludeAudioCheckBox = new wxCheckBox(this, wxID_ANY, _("Exclude Audio"));
    packBox->Add(ExcludeAudioCheckBox, 0, wxALL, 5);
    sizer->Add(packBox, 0, wxEXPAND | wxLEFT, 16);

    // Tip Of The Day.
    sizer->Add(MakePreferenceSectionHeader(this, _("Tip Of The Day")), 0, wxLEFT | wxTOP, 10);
    sizer->Add(MakePreferenceHint(this, _("Controls the tips shown when xLights starts.")), 0, wxLEFT, 16);
    auto* tipBox = new wxBoxSizer(wxVERTICAL);
    auto* tipRow = new wxBoxSizer(wxHORIZONTAL);
    tipRow->Add(new wxStaticText(this, wxID_ANY, _("Minimum Tip Level")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    Choice_MinTipLevel = new wxChoice(this, wxID_ANY);
    Choice_MinTipLevel->Append(_("Off"));
    Choice_MinTipLevel->SetSelection(Choice_MinTipLevel->Append(_("Beginner")));
    Choice_MinTipLevel->Append(_("Intermediate"));
    Choice_MinTipLevel->Append(_("Advanced"));
    Choice_MinTipLevel->Append(_("Expert"));
    tipRow->Add(Choice_MinTipLevel, 0, wxEXPAND);
    tipBox->Add(tipRow, 0, wxALL, 5);
    CheckBox_RecycleTips = new wxCheckBox(this, wxID_ANY, _("Recycle tips once all seen"));
    tipBox->Add(CheckBox_RecycleTips, 0, wxALL, 5);
    sizer->Add(tipBox, 0, wxEXPAND | wxLEFT, 16);

    // Moving Head Adv - Position Zones.
    sizer->Add(MakePreferenceSectionHeader(this, _("Moving Head Adv - Position Zones")), 0, wxLEFT | wxTOP, 10);
    sizer->Add(MakePreferenceHint(this, _("Named position zones used by Moving Head Advanced effects.")), 0, wxLEFT, 16);
    auto* zoneBox = new wxBoxSizer(wxVERTICAL);
    CheckBox_EnablePositionZones = new wxCheckBox(this, wxID_ANY, _("Enable Position Zones"));
    CheckBox_EnablePositionZones->SetValue(true);
    zoneBox->Add(CheckBox_EnablePositionZones, 0, wxALL, 5);
    CheckBox_ShowZoneIndicator = new wxCheckBox(this, wxID_ANY, _("Show Zone Indicator in Preview"));
    zoneBox->Add(CheckBox_ShowZoneIndicator, 0, wxALL, 5);
    sizer->Add(zoneBox, 0, wxEXPAND | wxLEFT, 16);

    SetSizer(sizer);
    sizer->SetSizeHints(this);

#ifdef __LINUX__
    ShaderCheckbox->Hide();
    GPURenderCheckbox->Hide();
#endif
#ifdef __WXOSX__
    if (!isMetalComputeSupported()) {
        GPURenderCheckbox->Hide();
    }
    ShaderCheckbox->Hide();
#endif
#ifdef __WXMSW__
    GPURenderCheckbox->Hide();
    MSWDisableComposited();
#endif

    eMailTextControl->Bind(wxEVT_TEXT, &OtherSettingsPanel::OnControlChanged, this);
    Choice_LinkControllerUpload->Bind(wxEVT_CHOICE, &OtherSettingsPanel::OnControlChanged, this);
    Choice_AliasPromptBehavior->Bind(wxEVT_CHOICE, &OtherSettingsPanel::OnControlChanged, this);
    CtrlPingInterval->Bind(wxEVT_SPINCTRLDOUBLE, &OtherSettingsPanel::OnSpinCtrlDoubleChange, this);
    GPURenderCheckbox->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    ShaderCheckbox->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_BatchRenderPromptIssues->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_PurgeDownloadCache->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_IgnoreVendorModelRecommendations->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_UseCustomColorPicker->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    ExcludeVideosCheckBox->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    ExcludeAudioCheckBox->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    Choice_MinTipLevel->Bind(wxEVT_CHOICE, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_RecycleTips->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_EnablePositionZones->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);
    CheckBox_ShowZoneIndicator->Bind(wxEVT_CHECKBOX, &OtherSettingsPanel::OnControlChanged, this);

    TransferDataToWindow();
}

OtherSettingsPanel::~OtherSettingsPanel()
{
}

bool OtherSettingsPanel::TransferDataFromWindow() {
    frame->SetExcludeAudioFromPackagedSequences(ExcludeAudioCheckBox->IsChecked());
    frame->SetExcludeVideosFromPackagedSequences(ExcludeVideosCheckBox->IsChecked());
    frame->SetUseGPURendering(GPURenderCheckbox->IsChecked());
    frame->SetShadersOnBackgroundThreads(ShaderCheckbox->IsChecked());
    frame->SetUserEMAIL(eMailTextControl->GetValue());
    frame->SetRenameModelAliasPromptBehavior(Choice_AliasPromptBehavior->GetStringSelection());
    frame->SetPromptBatchRenderIssues(CheckBox_BatchRenderPromptIssues->GetValue());
    frame->SetIgnoreVendorModelRecommendations(CheckBox_IgnoreVendorModelRecommendations->GetValue());
    frame->SetControllerPingInterval(CtrlPingInterval->GetValue());
    frame->SetPurgeDownloadCacheOnStart(CheckBox_PurgeDownloadCache->GetValue());
    frame->SetMinTipLevel(Choice_MinTipLevel->GetStringSelection());
    frame->SetRecycleTips(!CheckBox_RecycleTips->GetValue());
    frame->SetEnablePositionZones(CheckBox_EnablePositionZones->GetValue());
    frame->SetShowZoneIndicator(CheckBox_ShowZoneIndicator->GetValue());
    xlColourData::INSTANCE.SetUseCustomPicker(CheckBox_UseCustomColorPicker->IsChecked());
    return true;
}

bool OtherSettingsPanel::TransferDataToWindow() {
    ExcludeAudioCheckBox->SetValue(frame->ExcludeAudioFromPackagedSequences());
    ExcludeVideosCheckBox->SetValue(frame->ExcludeVideosFromPackagedSequences());
    GPURenderCheckbox->SetValue(frame->UseGPURendering());
    ShaderCheckbox->SetValue(frame->ShadersOnBackgroundThreads());
    eMailTextControl->ChangeValue(frame->UserEMAIL());
    Choice_LinkControllerUpload->SetStringSelection(frame->GetLinkedControllerUpload());
    Choice_AliasPromptBehavior->SetStringSelection(frame->GetRenameModelAliasPromptBehavior());
    CheckBox_BatchRenderPromptIssues->SetValue(frame->GetPromptBatchRenderIssues());
    CheckBox_IgnoreVendorModelRecommendations->SetValue(frame->GetIgnoreVendorModelRecommendations());
    CtrlPingInterval->SetValue(frame->GetControllerPingInterval());
    CheckBox_PurgeDownloadCache->SetValue(frame->GetPurgeDownloadCacheOnStart());
    Choice_MinTipLevel->SetStringSelection(frame->GetMinTipLevel());
    CheckBox_RecycleTips->SetValue(!frame->GetRecycleTips());
    CheckBox_EnablePositionZones->SetValue(frame->GetEnablePositionZones());
    CheckBox_ShowZoneIndicator->SetValue(frame->GetShowZoneIndicator());
    CheckBox_UseCustomColorPicker->SetValue(xlColourData::INSTANCE.UseCustomPicker());

// Remove attempt to sneak functionality into the windows build
#ifndef __WXMSW__
#ifndef IGNORE_VENDORS
    CheckBox_IgnoreVendorModelRecommendations->SetValue(false);
    CheckBox_IgnoreVendorModelRecommendations->Hide();
#endif
#endif
    return true;
}

void OtherSettingsPanel::ApplyIfImmediate() {
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void OtherSettingsPanel::OnControlChanged(wxCommandEvent& event) {
    ApplyIfImmediate();
}

void OtherSettingsPanel::OnSpinCtrlDoubleChange(wxSpinDoubleEvent& event) {
    ApplyIfImmediate();
}
