#include "FireworksPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "../sequencer/Effect.h"

//(*InternalHeaders(FireworksPanel)
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

#include "../ValueCurveButton.h"

//(*IdInit(FireworksPanel)
const long FireworksPanel::ID_STATICTEXT_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Explosions = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_CHECKBOX_Fireworks_UseMusic = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_CHECKBOX_FIRETIMING = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_CHECKBOX_FIRETIMING = wxNewId();
const long FireworksPanel::ID_STATICTEXT_FIRETIMINGTRACK = wxNewId();
const long FireworksPanel::ID_CHOICE_FIRETIMINGTRACK = wxNewId();
//*)

BEGIN_EVENT_TABLE(FireworksPanel,wxPanel)
	//(*EventTable(FireworksPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, FireworksPanel::SetTimingTracks)
END_EVENT_TABLE()

FireworksPanel::FireworksPanel(wxWindow* parent)
{
	//(*Initialize(FireworksPanel)
	wxFlexGridSizer* FlexGridSizer73;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer73 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer73->AddGrowableCol(1);
	StaticText94 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Explosions, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Explosions"));
	FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Num_Explosions = new BulkEditSlider(this, ID_SLIDER_Fireworks_Explosions, 16, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Explosions"));
	FlexGridSizer73->Add(Slider_Fireworks_Num_Explosions, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Explosions = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Explosions, _("16"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Explosions"));
	TextCtrl_Fireworks_Explosions->SetMaxLength(2);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Explosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksNumberExplosions = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions"));
	BitmapButton_FireworksNumberExplosions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksNumberExplosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText91 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Count, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Count"));
	FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Count = new BulkEditSlider(this, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
	FlexGridSizer73->Add(Slider_Fireworks_Count, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Count = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Count, _("50"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Count"));
	TextCtrl_Fireworks_Count->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Count"));
	BitmapButton_FireworksCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText93 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Velocity, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Velocity"));
	FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Velocity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
	FlexGridSizer73->Add(Slider_Fireworks_Velocity, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Velocity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Velocity, _("2"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Velocity"));
	TextCtrl_Fireworks_Velocity->SetMaxLength(2);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksVelocity = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity"));
	BitmapButton_FireworksVelocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText95 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Fade, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Fade"));
	FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Fade = new BulkEditSlider(this, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
	FlexGridSizer73->Add(Slider_Fireworks_Fade, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Fade = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Fade, _("50"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Fade"));
	TextCtrl_Fireworks_Fade->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksFade = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Fade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Fade"));
	BitmapButton_FireworksFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fireworks_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_UseMusic, _("Fire with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_UseMusic"));
	CheckBox_Fireworks_UseMusic->SetValue(false);
	FlexGridSizer73->Add(CheckBox_Fireworks_UseMusic, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_UseMusic = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic"));
	BitmapButton_Fireworks_UseMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_Fireworks_UseMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Sensitivity, _("Trigger level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(Slider_Fireworks_Sensitivity, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Sensitivity, _("50"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Sensitivity"));
	TextCtrl_Fireworks_Sensitivity->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_Sensitivity = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity"));
	BitmapButton_Fireworks_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FireTiming = new BulkEditCheckBox(this, ID_CHECKBOX_FIRETIMING, _("Fire with timing track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FIRETIMING"));
	CheckBox_FireTiming->SetValue(false);
	FlexGridSizer73->Add(CheckBox_FireTiming, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_FIRETIMING, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FIRETIMING"));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_FIRETIMINGTRACK, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_TimingTrack = new BulkEditChoice(this, ID_CHOICE_FIRETIMINGTRACK, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(Choice_TimingTrack, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer73);
	FlexGridSizer73->Fit(this);
	FlexGridSizer73->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_Fireworks_UseMusicClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_FIRETIMING,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_FireTimingClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FIRETIMING,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHOICE_FIRETIMINGTRACK,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FireworksPanel::OnChoice_TimingTrackSelect);
	//*)

    SetName("ID_PANEL_FIREWORKS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FireworksPanel::OnVCChanged, 0, this);
    ValidateWindow();
}

FireworksPanel::~FireworksPanel()
{
	//(*Destroy(FireworksPanel)
	//*)
}

void FireworksPanel::ValidateWindow()
{
    if (CheckBox_Fireworks_UseMusic->GetValue())
    {
        Slider_Fireworks_Sensitivity->Enable(true);
    }
    else
    {
        Slider_Fireworks_Sensitivity->Enable(false);
    }
    if (CheckBox_FireTiming->GetValue())
    {
        Choice_TimingTrack->Enable(true);
    }
    else
    {
        Choice_TimingTrack->Enable(false);
    }
}

PANEL_EVENT_HANDLERS(FireworksPanel)

void FireworksPanel::OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnCheckBox_FireTimingClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnChoice_TimingTrackSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_TimingTrack->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (int i = 0; i < Choice_TimingTrack->GetCount(); i++)
    {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
        {
            if (*it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
        {
            if (*it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Append(*it);
        }
    }

    if (removed && Choice_TimingTrack->GetCount() > 0)
    {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
        {
            if (selection == Choice_TimingTrack->GetString(i))
            {
                found = true;
                Choice_TimingTrack->SetSelection(i);
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->SetSelection(0);
        }
    }
    ValidateWindow();
}
