#include "MeteorsPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(MeteorsPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/image.h>
//*)

//(*IdInit(MeteorsPanel)
const long MeteorsPanel::ID_STATICTEXT_Meteors_Type = wxNewId();
const long MeteorsPanel::ID_CHOICE_Meteors_Type = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_CHOICE_Meteors_Type = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_Effect = wxNewId();
const long MeteorsPanel::ID_CHOICE_Meteors_Effect = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_CHOICE_Meteors_Effect = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_Count = wxNewId();
const long MeteorsPanel::ID_SLIDER_Meteors_Count = wxNewId();
const long MeteorsPanel::IDD_TEXTCTRL_Meteors_Count = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Count = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_Length = wxNewId();
const long MeteorsPanel::ID_SLIDER_Meteors_Length = wxNewId();
const long MeteorsPanel::IDD_TEXTCTRL_Meteors_Length = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Length = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_Swirl_Intensity = wxNewId();
const long MeteorsPanel::ID_SLIDER_Meteors_Swirl_Intensity = wxNewId();
const long MeteorsPanel::IDD_TEXTCTRL_Meteors_Swirl_Intensity = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_Speed = wxNewId();
const long MeteorsPanel::ID_SLIDER_Meteors_Speed = wxNewId();
const long MeteorsPanel::IDD_TEXTCTRL_Meteors_Speed = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_XOffset = wxNewId();
const long MeteorsPanel::IDD_SLIDER_Meteors_XOffset = wxNewId();
const long MeteorsPanel::ID_TEXTCTRL_Meteors_XOffset = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_XOffset = wxNewId();
const long MeteorsPanel::ID_STATICTEXT_Meteors_YOffset = wxNewId();
const long MeteorsPanel::IDD_SLIDER_Meteors_YOffset = wxNewId();
const long MeteorsPanel::ID_TEXTCTRL_Meteors_YOffset = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_YOffset = wxNewId();
const long MeteorsPanel::ID_CHECKBOX_Meteors_UseMusic = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_CHECKBOX_Meteors_UseMusic = wxNewId();
const long MeteorsPanel::ID_CHECKBOX_FadeWithDistance = wxNewId();
const long MeteorsPanel::ID_BITMAPBUTTON_CHECKBOX_FadeWithDistance = wxNewId();
//*)

BEGIN_EVENT_TABLE(MeteorsPanel,wxPanel)
	//(*EventTable(MeteorsPanel)
	//*)
END_EVENT_TABLE()

MeteorsPanel::MeteorsPanel(wxWindow* parent)
{
	//(*Initialize(MeteorsPanel)
	BulkEditTextCtrl* TextCtrl52;
	wxFlexGridSizer* FlexGridSizer41;
	BulkEditTextCtrl* TextCtrl49;
	BulkEditTextCtrl* TextCtrl51;
	BulkEditTextCtrl* TextCtrl50;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer41 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer41->AddGrowableCol(1);
	StaticText39 = new wxStaticText(this, ID_STATICTEXT_Meteors_Type, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Type"));
	FlexGridSizer41->Add(StaticText39, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Meteors_Type = new BulkEditChoice(this, ID_CHOICE_Meteors_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Type"));
	Choice_Meteors_Type->SetSelection( Choice_Meteors_Type->Append(_("Rainbow")) );
	Choice_Meteors_Type->Append(_("Range"));
	Choice_Meteors_Type->Append(_("Palette"));
	FlexGridSizer41->Add(Choice_Meteors_Type, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MeteorsType = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Meteors_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Meteors_Type"));
	BitmapButton_MeteorsType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_MeteorsType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText128 = new wxStaticText(this, ID_STATICTEXT_Meteors_Effect, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Effect"));
	FlexGridSizer41->Add(StaticText128, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Meteors_Effect = new BulkEditChoice(this, ID_CHOICE_Meteors_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Effect"));
	Choice_Meteors_Effect->SetSelection( Choice_Meteors_Effect->Append(_("Down")) );
	Choice_Meteors_Effect->Append(_("Up"));
	Choice_Meteors_Effect->Append(_("Left"));
	Choice_Meteors_Effect->Append(_("Right"));
	Choice_Meteors_Effect->Append(_("Implode"));
	Choice_Meteors_Effect->Append(_("Explode"));
	Choice_Meteors_Effect->Append(_("Icicles"));
	Choice_Meteors_Effect->Append(_("Icicles + bkg"));
	FlexGridSizer41->Add(Choice_Meteors_Effect, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MeteorsEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Meteors_Effect, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Meteors_Effect"));
	BitmapButton_MeteorsEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_MeteorsEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText41 = new wxStaticText(this, ID_STATICTEXT_Meteors_Count, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Count"));
	FlexGridSizer41->Add(StaticText41, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Count = new BulkEditSlider(this, ID_SLIDER_Meteors_Count, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Count"));
	FlexGridSizer41->Add(Slider_Meteors_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl49 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Meteors_Count, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Meteors_Count"));
	TextCtrl49->SetMaxLength(4);
	FlexGridSizer41->Add(TextCtrl49, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MeteorsCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Meteors_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Count"));
	BitmapButton_MeteorsCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_MeteorsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText43 = new wxStaticText(this, ID_STATICTEXT_Meteors_Length, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Length"));
	FlexGridSizer41->Add(StaticText43, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Length = new BulkEditSlider(this, ID_SLIDER_Meteors_Length, 25, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Length"));
	FlexGridSizer41->Add(Slider_Meteors_Length, 1, wxALL|wxEXPAND, 2);
	TextCtrl50 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Meteors_Length, _("25"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Meteors_Length"));
	TextCtrl50->SetMaxLength(4);
	FlexGridSizer41->Add(TextCtrl50, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MeteorsLength = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Meteors_Length, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Length"));
	BitmapButton_MeteorsLength->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_MeteorsLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText130 = new wxStaticText(this, ID_STATICTEXT_Meteors_Swirl_Intensity, _("Swirl Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Swirl_Intensity"));
	FlexGridSizer41->Add(StaticText130, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_Swirl_Intensity = new BulkEditSlider(this, ID_SLIDER_Meteors_Swirl_Intensity, 0, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Swirl_Intensity"));
	FlexGridSizer41->Add(Slider_Meteors_Swirl_Intensity, 1, wxALL|wxEXPAND, 2);
	TextCtrl51 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Meteors_Swirl_Intensity, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Meteors_Swirl_Intensity"));
	TextCtrl51->SetMaxLength(3);
	FlexGridSizer41->Add(TextCtrl51, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_MeteorsSwirlIntensity = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity"));
	BitmapButton_MeteorsSwirlIntensity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_MeteorsSwirlIntensity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT_Meteors_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_Speed"));
	FlexGridSizer41->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Meteors_Speed = new BulkEditSlider(this, ID_SLIDER_Meteors_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Speed"));
	FlexGridSizer41->Add(Slider_Meteors_Speed, 1, wxALL|wxEXPAND, 5);
	TextCtrl52 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Meteors_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Meteors_Speed"));
	TextCtrl52->SetMaxLength(3);
	FlexGridSizer41->Add(TextCtrl52, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Meteors_XOffset, _("Horizontal Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_XOffset"));
	FlexGridSizer41->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_XOffset = new BulkEditSlider(this, IDD_SLIDER_Meteors_XOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Meteors_XOffset"));
	FlexGridSizer41->Add(Slider_Meteors_XOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Meteors_XOffset = new BulkEditTextCtrl(this, ID_TEXTCTRL_Meteors_XOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Meteors_XOffset"));
	TextCtrl_Meteors_XOffset->SetMaxLength(4);
	FlexGridSizer41->Add(TextCtrl_Meteors_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Meteors_XOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Meteors_XOffset, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_XOffset"));
	BitmapButton_Meteors_XOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_Meteors_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Meteors_YOffset, _("Vertical Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Meteors_YOffset"));
	FlexGridSizer41->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Meteors_YOffset = new BulkEditSlider(this, IDD_SLIDER_Meteors_YOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Meteors_YOffset"));
	FlexGridSizer41->Add(Slider_Meteors_YOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Meteors_Yoffset = new BulkEditTextCtrl(this, ID_TEXTCTRL_Meteors_YOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Meteors_YOffset"));
	TextCtrl_Meteors_Yoffset->SetMaxLength(4);
	FlexGridSizer41->Add(TextCtrl_Meteors_Yoffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Meteors_YOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Meteors_YOffset, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_YOffset"));
	BitmapButton_Meteors_YOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_Meteors_YOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Meteors_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Meteors_UseMusic, _("Adjust count based on music intensity"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Meteors_UseMusic"));
	CheckBox_Meteors_UseMusic->SetValue(false);
	FlexGridSizer41->Add(CheckBox_Meteors_UseMusic, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Meteors_UseMusic = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Meteors_UseMusic, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Meteors_UseMusic"));
	BitmapButton_Meteors_UseMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_Meteors_UseMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FadeWithDistance = new BulkEditCheckBox(this, ID_CHECKBOX_FadeWithDistance, _("Starfield simulation"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FadeWithDistance"));
	CheckBox_FadeWithDistance->SetValue(false);
	FlexGridSizer41->Add(CheckBox_FadeWithDistance, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer41->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FadeWithDistance = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_FadeWithDistance, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FadeWithDistance"));
	BitmapButton_FadeWithDistance->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer41->Add(BitmapButton_FadeWithDistance, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer41);
	FlexGridSizer41->Fit(this);
	FlexGridSizer41->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_Meteors_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Meteors_Effect,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&MeteorsPanel::OnChoice_Meteors_EffectSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Meteors_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Meteors_XOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Meteors_YOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Meteors_UseMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FadeWithDistance,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MeteorsPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_METEORS");

    ValidateWindow();
}

MeteorsPanel::~MeteorsPanel()
{
	//(*Destroy(MeteorsPanel)
	//*)
}

PANEL_EVENT_HANDLERS(MeteorsPanel)

void MeteorsPanel::ValidateWindow()
{
    if (Choice_Meteors_Effect->GetStringSelection() == "Implode" ||
        Choice_Meteors_Effect->GetStringSelection() == "Explode")
    {
        Slider_Meteors_XOffset->Enable();
        Slider_Meteors_YOffset->Enable();
        CheckBox_FadeWithDistance->Enable();
    }
    else
    {
        Slider_Meteors_XOffset->Disable();
        Slider_Meteors_YOffset->Disable();
        CheckBox_FadeWithDistance->Disable();
    }
}
void MeteorsPanel::OnChoice_Meteors_EffectSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
