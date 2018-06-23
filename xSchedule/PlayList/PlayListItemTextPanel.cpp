#include "PlayListItemTextPanel.h"
#include "PlayListItemText.h"

//(*InternalHeaders(PlayListItemTextPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/colordlg.h>
#include "../FSEQFile.h"
#include "../xScheduleMain.h"
#include "../ScheduleOptions.h"
#include "../ScheduleManager.h"
#include "../MatrixMapper.h"
#include "PlayListDialog.h"

//(*IdInit(PlayListItemTextPanel)
const long PlayListItemTextPanel::ID_STATICTEXT14 = wxNewId();
const long PlayListItemTextPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT11 = wxNewId();
const long PlayListItemTextPanel::ID_CHOICE5 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemTextPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemTextPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemTextPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemTextPanel::ID_FONTPICKERCTRL1 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemTextPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemTextPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemTextPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemTextPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemTextPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT12 = wxNewId();
const long PlayListItemTextPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT13 = wxNewId();
const long PlayListItemTextPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemTextPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemTextPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemTextPanel::ID_TEXTCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemTextPanel,wxPanel)
	//(*EventTable(PlayListItemTextPanel)
	//*)
END_EVENT_TABLE()

PlayListItemTextPanel::PlayListItemTextPanel(wxWindow* parent, PlayListItemText* text,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _text = text;

	//(*Initialize(PlayListItemTextPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT14, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Matrix:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer1->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Matrices = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	FlexGridSizer1->Add(Choice_Matrices, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Type = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Type->SetSelection( Choice_Type->Append(_("Normal")) );
	Choice_Type->Append(_("Countdown"));
	Choice_Type->Append(_("File Read"));
	FlexGridSizer1->Add(Choice_Type, 1, wxALL|wxEXPAND, 5);
	StaticText_Text = new wxStaticText(this, ID_STATICTEXT1, _("Text:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText_Text, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Text = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Text, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Format = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Format, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Font:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FontPickerCtrl1 = new wxFontPickerCtrl(this, ID_FONTPICKERCTRL1, wxNullFont, wxDefaultPosition, wxDefaultSize, wxFNTP_FONTDESC_AS_LABEL|wxFNTP_USEFONT_FOR_LABEL, wxDefaultValidator, _T("ID_FONTPICKERCTRL1"));
	FlexGridSizer1->Add(FontPickerCtrl1, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Orientation = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_Orientation->SetSelection( Choice_Orientation->Append(_("Normal")) );
	Choice_Orientation->Append(_("Vertical Up"));
	Choice_Orientation->Append(_("Vertical Down"));
	Choice_Orientation->Append(_("Rotate Up 90"));
	Choice_Orientation->Append(_("Rotate Down 90"));
	FlexGridSizer1->Add(Choice_Orientation, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Movement:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Movement = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_Movement->Append(_("None"));
	Choice_Movement->SetSelection( Choice_Movement->Append(_("Right to Left")) );
	Choice_Movement->Append(_("Left to Right"));
	Choice_Movement->Append(_("Bottom to Top"));
	Choice_Movement->Append(_("Top to Bottom"));
	FlexGridSizer1->Add(Choice_Movement, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Speed = new wxSpinCtrl(this, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 10, _T("ID_SPINCTRL1"));
	SpinCtrl_Speed->SetValue(_T("10"));
	FlexGridSizer1->Add(SpinCtrl_Speed, 1, wxALL|wxEXPAND, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_RenderWhenBlank = new wxCheckBox(this, ID_CHECKBOX1, _("Render when text is blank"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_RenderWhenBlank->SetValue(true);
	FlexGridSizer1->Add(CheckBox_RenderWhenBlank, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("X Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer1->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_X = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -1000, 1000, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_X->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_X, 1, wxALL|wxEXPAND, 5);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Y Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer1->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Y = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -1000, 1000, 0, _T("ID_SPINCTRL3"));
	SpinCtrl_Y->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Y, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL4, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL4"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL3, _("0.050"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemTextPanel::OnChoice_TypeSelect);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it = m->begin(); it != m->end(); ++it)
    {
        Choice_Matrices->AppendString((*it)->GetName());
    }
    if (Choice_Matrices->GetCount() > 0) Choice_Matrices->SetSelection(0);

    TextCtrl_Name->SetValue(text->GetRawName());
    TextCtrl_Text->SetValue(text->GetText());
    TextCtrl_Format->SetValue(text->GetFormat());
    SpinCtrl_Speed->SetValue(text->GetSpeed());
    FontPickerCtrl1->SetSelectedFont(*text->GetFont());
    FontPickerCtrl1->SetSelectedColour(text->GetColour());
    Choice_Orientation->SetStringSelection(text->GetOrientation());
    Choice_Movement->SetStringSelection(text->GetMovement());
    Choice_Type->SetStringSelection(text->GetType());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)text->GetDuration() / 1000.0));
    Choice_BlendMode->SetSelection(text->GetBlendMode());
    Choice_Matrices->SetStringSelection(text->GetMatrix());
    SpinCtrl_X->SetValue(_text->GetX());
    SpinCtrl_Y->SetValue(_text->GetY());
    SpinCtrl_Priority->SetValue(_text->GetPriority());
    CheckBox_RenderWhenBlank->SetValue(_text->GetRenderWhenBlank());

    ValidateWindow();
}

PlayListItemTextPanel::~PlayListItemTextPanel()
{
	//(*Destroy(PlayListItemTextPanel)
	//*)

    _text->SetText(TextCtrl_Text->GetValue().ToStdString());
    _text->SetFormat(TextCtrl_Format->GetValue().ToStdString());
    _text->SetSpeed(SpinCtrl_Speed->GetValue());
    _text->SetFont(new wxFont(FontPickerCtrl1->GetSelectedFont()));
    _text->SetColour(FontPickerCtrl1->GetSelectedColour());
    _text->SetOrientation(Choice_Orientation->GetStringSelection().ToStdString());
    _text->SetMovement(Choice_Movement->GetStringSelection().ToStdString());
    _text->SetType(Choice_Type->GetStringSelection().ToStdString());
    _text->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _text->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _text->SetMatrix(Choice_Matrices->GetStringSelection().ToStdString());
    _text->SetX(SpinCtrl_X->GetValue());
    _text->SetY(SpinCtrl_Y->GetValue());
    _text->SetName(TextCtrl_Name->GetValue().ToStdString());
    _text->SetPriority(SpinCtrl_Priority->GetValue());
    _text->SetRenderWhenBlank(CheckBox_RenderWhenBlank->GetValue());
}

void PlayListItemTextPanel::OnChoice_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemTextPanel::ValidateWindow()
{
    TextCtrl_Format->SetToolTip(PlayListItemText::GetTooltip(Choice_Type->GetStringSelection().ToStdString()));
    if (Choice_Type->GetStringSelection() == "Normal")
    {
        StaticText_Text->SetLabel("Text");
    }
    else if (Choice_Type->GetStringSelection() == "File Read")
    {
        StaticText_Text->SetLabel("File Path");
    }
    else
    {
        StaticText_Text->SetLabel("To Date/Time");
    }
}

void PlayListItemTextPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _text->SetName(TextCtrl_Name->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}
