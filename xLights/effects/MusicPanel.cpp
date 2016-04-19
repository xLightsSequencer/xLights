#include "MusicPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(MusicPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MusicPanel)
const long MusicPanel::ID_STATICTEXT1 = wxNewId();
const long MusicPanel::IDD_SLIDER_Music_Bars = wxNewId();
const long MusicPanel::ID_TEXTCTRL_Music_Bars = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHOICE_Music_Bars = wxNewId();
const long MusicPanel::ID_STATICTEXT2 = wxNewId();
const long MusicPanel::ID_CHOICE_Music_Type = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_CHOICE_Music_Type = wxNewId();
const long MusicPanel::ID_STATICTEXT12 = wxNewId();
const long MusicPanel::IDD_SLIDER_Music_StartNote = wxNewId();
const long MusicPanel::ID_TEXTCTRL_Music_StartNote = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_StartNote = wxNewId();
const long MusicPanel::ID_STATICTEXT11 = wxNewId();
const long MusicPanel::IDD_SLIDER_Music_EndNote = wxNewId();
const long MusicPanel::ID_TEXTCTRL_Music_EndNote = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_EndNote = wxNewId();
const long MusicPanel::ID_STATICTEXT4 = wxNewId();
const long MusicPanel::IDD_SLIDER_Music_Sensitivity = wxNewId();
const long MusicPanel::ID_TEXTCTRL_Music_Sensitivity = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_Sensitivty = wxNewId();
const long MusicPanel::ID_STATICTEXT3 = wxNewId();
const long MusicPanel::IDD_SLIDER_Music_Offset = wxNewId();
const long MusicPanel::ID_TEXTCTRL_Music_Offset = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_Offset = wxNewId();
const long MusicPanel::ID_CHECKBOX_Music_Scale = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_Scale = wxNewId();
const long MusicPanel::ID_CHECKBOX_Music_ScaleNotes = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_ScaleNotes = wxNewId();
const long MusicPanel::ID_STATICTEXT6 = wxNewId();
const long MusicPanel::ID_CHOICE_Music_Colour = wxNewId();
const long MusicPanel::ID_BITMAPBUTTON_Music_Colour = wxNewId();
//*)

BEGIN_EVENT_TABLE(MusicPanel,wxPanel)
	//(*EventTable(MusicPanel)
	//*)
END_EVENT_TABLE()

MusicPanel::MusicPanel(wxWindow* parent)
{
	//(*Initialize(MusicPanel)
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer31;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Bars"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_Bars = new wxSlider(this, IDD_SLIDER_Music_Bars, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Music_Bars"));
	FlexGridSizer31->Add(Slider_Music_Bars, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_Bars = new wxTextCtrl(this, ID_TEXTCTRL_Music_Bars, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Music_Bars"));
	FlexGridSizer31->Add(TextCtrl_Music_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_Bars = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Music_Bars, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Music_Bars"));
	BitmapButton_Music_Bars->SetDefault();
	BitmapButton_Music_Bars->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Music_Type = new wxChoice(this, ID_CHOICE_Music_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Music_Type"));
	Choice_Music_Type->SetSelection( Choice_Music_Type->Append(_("Morph In")) );
	Choice_Music_Type->Append(_("Morph Out"));
	Choice_Music_Type->Append(_("Collide"));
	Choice_Music_Type->Append(_("Seperate"));
	FlexGridSizer31->Add(Choice_Music_Type, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Type = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Music_Type, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Music_Type"));
	BitmapButton_Music_Type->SetDefault();
	BitmapButton_Music_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Start Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer31->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_StartNote = new wxSlider(this, IDD_SLIDER_Music_StartNote, 60, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Music_StartNote"));
	FlexGridSizer31->Add(Slider_Music_StartNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_StartNote = new wxTextCtrl(this, ID_TEXTCTRL_Music_StartNote, _("60"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Music_StartNote"));
	FlexGridSizer31->Add(TextCtrl_Music_StartNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_StartNote = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_StartNote, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_StartNote"));
	BitmapButton_Music_StartNote->SetDefault();
	BitmapButton_Music_StartNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_StartNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("End Note"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer31->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_EndNote = new wxSlider(this, IDD_SLIDER_Music_EndNote, 80, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Music_EndNote"));
	FlexGridSizer31->Add(Slider_Music_EndNote, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_EndNote = new wxTextCtrl(this, ID_TEXTCTRL_Music_EndNote, _("80"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Music_EndNote"));
	FlexGridSizer31->Add(TextCtrl_Music_EndNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_EndNote = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_EndNote, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_EndNote"));
	BitmapButton_Music_EndNote->SetDefault();
	BitmapButton_Music_EndNote->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_EndNote, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Sensitivity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_Sensitivity = new wxSlider(this, IDD_SLIDER_Music_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Music_Sensitivity"));
	FlexGridSizer31->Add(Slider_Music_Sensitivity, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_Sensitivity = new wxTextCtrl(this, ID_TEXTCTRL_Music_Sensitivity, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Music_Sensitivity"));
	FlexGridSizer31->Add(TextCtrl_Music_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Music_Sensitivity = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_Sensitivty, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_Sensitivty"));
	BitmapButton_Music_Sensitivity->SetDefault();
	BitmapButton_Music_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Music_Offset = new wxSlider(this, IDD_SLIDER_Music_Offset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Music_Offset"));
	FlexGridSizer31->Add(Slider_Music_Offset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Music_Offset = new wxTextCtrl(this, ID_TEXTCTRL_Music_Offset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Music_Offset"));
	FlexGridSizer31->Add(TextCtrl_Music_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Music_Offset = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_Offset, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_Offset"));
	BitmapButton_Music_Offset->SetDefault();
	BitmapButton_Music_Offset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Music_Scale = new wxCheckBox(this, ID_CHECKBOX_Music_Scale, _("Scale Bars"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Music_Scale"));
	CheckBox_Music_Scale->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Music_Scale, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Scale = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_Scale, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_Scale"));
	BitmapButton_Music_Scale->SetDefault();
	BitmapButton_Music_Scale->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Music_ScaleNotes = new wxCheckBox(this, ID_CHECKBOX_Music_ScaleNotes, _("Scale Notes Independently"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Music_ScaleNotes"));
	CheckBox_Music_ScaleNotes->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Music_ScaleNotes, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_ScaleNotes = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_ScaleNotes, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_ScaleNotes"));
	BitmapButton_Music_ScaleNotes->SetDefault();
	BitmapButton_Music_ScaleNotes->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_ScaleNotes, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Color"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Music_Colour = new wxChoice(this, ID_CHOICE_Music_Colour, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Music_Colour"));
	Choice_Music_Colour->SetSelection( Choice_Music_Colour->Append(_("Distinct")) );
	Choice_Music_Colour->Append(_("Blend"));
	FlexGridSizer31->Add(Choice_Music_Colour, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer31->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Music_Colour = new wxBitmapButton(this, ID_BITMAPBUTTON_Music_Colour, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Music_Colour"));
	BitmapButton_Music_Colour->SetDefault();
	BitmapButton_Music_Colour->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Music_Colour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(IDD_SLIDER_Music_Bars,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Music_Bars,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Music_Bars,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Music_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&MusicPanel::OnChoice_Music_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Music_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Music_StartNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated);
	Connect(ID_TEXTCTRL_Music_StartNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteText);
	Connect(ID_BITMAPBUTTON_Music_StartNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Music_EndNote,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated);
	Connect(ID_TEXTCTRL_Music_EndNote,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::OnTextCtrl_Music_StartNoteText);
	Connect(ID_BITMAPBUTTON_Music_EndNote,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Music_Sensitivity,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Music_Sensitivity,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Music_Sensitivty,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Music_Offset,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Music_Offset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MusicPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Music_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Music_Scale,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Music_ScaleNotes,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Music_Colour,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MusicPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_Music");

	ValidateWindow();
}

MusicPanel::~MusicPanel()
{
	//(*Destroy(MusicPanel)
	//*)
}

PANEL_EVENT_HANDLERS(MusicPanel)

static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}

void MusicPanel::ValidateWindow()
{
}

void MusicPanel::OnChoice_Music_TypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}

void MusicPanel::OnSlider_Music_StartNoteCmdSliderUpdated(wxScrollEvent& event)
{
    int start = Slider_Music_StartNote->GetValue();
    int end = Slider_Music_EndNote->GetValue();
    if (event.GetEventObject() == Slider_Music_StartNote)
    {
        if (end < start)
        {
            end = start;
        }
    }
    else
    {
        if (end < start)
        {
            start = end;
        }
    }

    if (end != Slider_Music_EndNote->GetValue())
    {
        Slider_Music_EndNote->SetValue(end);
    }
    wxString e = wxString::Format("%d", end);
    if (e != TextCtrl_Music_EndNote->GetValue())
    {
        TextCtrl_Music_EndNote->SetValue(e);
    }
    if (start != Slider_Music_StartNote->GetValue())
    {
        Slider_Music_StartNote->SetValue(start);
    }
    wxString s = wxString::Format("%d", start);
    if (s != TextCtrl_Music_StartNote->GetValue())
    {
        TextCtrl_Music_StartNote->SetValue(s);
    }
    ValidateWindow();
}

void MusicPanel::OnTextCtrl_Music_StartNoteText(wxCommandEvent& event)
{
    int start = wxAtoi(TextCtrl_Music_StartNote->GetValue());
    int end = wxAtoi(TextCtrl_Music_EndNote->GetValue());
    if (event.GetEventObject() == TextCtrl_Music_StartNote)
    {
        if (end < start)
        {
            end = start;
        }
    }
    else
    {
        if (end < start)
        {
            start = end;
        }
    }

    if (end != Slider_Music_EndNote->GetValue())
    {
        Slider_Music_EndNote->SetValue(end);
    }
    wxString e = wxString::Format("%d", end);
    if (e != TextCtrl_Music_EndNote->GetValue())
    {
        TextCtrl_Music_EndNote->SetValue(e);
    }
    if (start != Slider_Music_StartNote->GetValue())
    {
        Slider_Music_StartNote->SetValue(start);
    }
    wxString s = wxString::Format("%d", start);
    if (s != TextCtrl_Music_StartNote->GetValue())
    {
        TextCtrl_Music_StartNote->SetValue(s);
    }

    ValidateWindow();
}
