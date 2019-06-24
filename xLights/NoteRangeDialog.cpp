#include "NoteRangeDialog.h"
#include "AudioManager.h"

//(*InternalHeaders(NoteRangeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(NoteRangeDialog)
const long NoteRangeDialog::ID_STATICTEXT1 = wxNewId();
const long NoteRangeDialog::ID_SLIDER1 = wxNewId();
const long NoteRangeDialog::ID_TEXTCTRL1 = wxNewId();
const long NoteRangeDialog::ID_STATICTEXT3 = wxNewId();
const long NoteRangeDialog::ID_STATICTEXT2 = wxNewId();
const long NoteRangeDialog::ID_SLIDER2 = wxNewId();
const long NoteRangeDialog::ID_TEXTCTRL2 = wxNewId();
const long NoteRangeDialog::ID_STATICTEXT4 = wxNewId();
const long NoteRangeDialog::ID_BUTTON_Ok = wxNewId();
const long NoteRangeDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(NoteRangeDialog,wxDialog)
	//(*EventTable(NoteRangeDialog)
	//*)
END_EVENT_TABLE()

NoteRangeDialog::NoteRangeDialog(wxWindow* parent,int& lowNote, int& highNote, wxWindowID id,const wxPoint& pos,const wxSize& size) : _lowNote(lowNote), _highNote(highNote)
{
	//(*Initialize(NoteRangeDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Midi Note Range"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("LowNote:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Low = new wxSlider(this, ID_SLIDER1, 0, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
	FlexGridSizer1->Add(Slider_Low, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Low = new wxTextCtrl(this, ID_TEXTCTRL1, _("0"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_Low->SetMaxLength(3);
	FlexGridSizer1->Add(TextCtrl_Low, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_LowHz = new wxStaticText(this, ID_STATICTEXT3, _("Label"), wxDefaultPosition, wxSize(65,-1), 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText_LowHz, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("High Note:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_High = new wxSlider(this, ID_SLIDER2, 127, 0, 127, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER2"));
	FlexGridSizer1->Add(Slider_High, 1, wxALL|wxEXPAND, 5);
	TextCtrl_High = new wxTextCtrl(this, ID_TEXTCTRL2, _("127"), wxDefaultPosition, wxSize(40,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl_High->SetMaxLength(3);
	FlexGridSizer1->Add(TextCtrl_High, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_HighHz = new wxStaticText(this, ID_STATICTEXT4, _("Label"), wxDefaultPosition, wxSize(65,-1), 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText_HighHz, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON_Ok, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Ok"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NoteRangeDialog::OnSlider_LowCmdScroll);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteRangeDialog::OnTextCtrl_LowText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&NoteRangeDialog::OnTextCtrl_LowTextEnter);
	Connect(ID_SLIDER2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NoteRangeDialog::OnSlider_HighCmdScroll);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NoteRangeDialog::OnTextCtrl_HighText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&NoteRangeDialog::OnTextCtrl_HighTextEnter);
	Connect(ID_BUTTON_Ok,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NoteRangeDialog::OnButton_OkClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NoteRangeDialog::OnButton_CancelClick);
	//*)

    Slider_Low->SetValue(_lowNote);
    TextCtrl_Low->SetValue(wxString::Format("%d", _lowNote));
    Slider_High->SetValue(_highNote);
    TextCtrl_High->SetValue(wxString::Format("%d", _highNote));
    SetEscapeId(ID_BUTTON_CANCEL);
    UpdateNotes();
}

NoteRangeDialog::~NoteRangeDialog()
{
	//(*Destroy(NoteRangeDialog)
	//*)
}

void NoteRangeDialog::UpdateNotes()
{
    StaticText_LowHz->SetLabel(wxString::Format("%s %dHz", AudioManager::MidiToNote(Slider_Low->GetValue()), (int)AudioManager::MidiToFrequency(Slider_Low->GetValue())));
    StaticText_HighHz->SetLabel(wxString::Format("%s %dHz", AudioManager::MidiToNote(Slider_High->GetValue()), (int)AudioManager::MidiToFrequency(Slider_High->GetValue())));
}

void NoteRangeDialog::OnSlider_LowCmdScroll(wxScrollEvent& event)
{
    if (Slider_Low->GetValue() > Slider_High->GetValue())
    {
        Slider_High->SetValue(Slider_Low->GetValue());
        TextCtrl_High->SetValue(wxString::Format("%d", Slider_High->GetValue()));
    }
    TextCtrl_Low->SetValue(wxString::Format("%d", Slider_Low->GetValue()));
    UpdateNotes();
}

void NoteRangeDialog::OnSlider_HighCmdScroll(wxScrollEvent& event)
{
    if (Slider_High->GetValue() < Slider_Low->GetValue())
    {
        Slider_Low->SetValue(Slider_High->GetValue());
        TextCtrl_Low->SetValue(wxString::Format("%d", Slider_Low->GetValue()));
    }
    TextCtrl_High->SetValue(wxString::Format("%d", Slider_High->GetValue()));
    UpdateNotes();
}

void NoteRangeDialog::OnTextCtrl_HighText(wxCommandEvent& event)
{
    int newHigh = wxAtoi(TextCtrl_High->GetValue());
    Slider_High->SetValue(newHigh);
    if (Slider_High->GetValue() < Slider_Low->GetValue())
    {
        Slider_Low->SetValue(Slider_High->GetValue());
        TextCtrl_Low->SetValue(wxString::Format("%d", Slider_Low->GetValue()));
    }
    UpdateNotes();
}

void NoteRangeDialog::OnTextCtrl_LowText(wxCommandEvent& event)
{
    int newLow = wxAtoi(TextCtrl_Low->GetValue());
    Slider_Low->SetValue(newLow);
    if (Slider_Low->GetValue() > Slider_High->GetValue())
    {
        Slider_High->SetValue(Slider_Low->GetValue());
        TextCtrl_High->SetValue(wxString::Format("%d", Slider_High->GetValue()));
    }
    UpdateNotes();
}

void NoteRangeDialog::OnTextCtrl_LowTextEnter(wxCommandEvent& event)
{
}

void NoteRangeDialog::OnTextCtrl_HighTextEnter(wxCommandEvent& event)
{
}

void NoteRangeDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _lowNote = Slider_Low->GetValue();
    _highNote = Slider_High->GetValue();
    EndDialog(wxID_OK);
}

void NoteRangeDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
