#include "PlayListItemScreenMapPanel.h"
#include "PlayListItemScreenMap.h"

//(*InternalHeaders(PlayListItemScreenMapPanel)
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

//(*IdInit(PlayListItemScreenMapPanel)
const long PlayListItemScreenMapPanel::ID_STATICTEXT14 = wxNewId();
const long PlayListItemScreenMapPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT11 = wxNewId();
const long PlayListItemScreenMapPanel::ID_CHOICE5 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemScreenMapPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemScreenMapPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemScreenMapPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT12 = wxNewId();
const long PlayListItemScreenMapPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT13 = wxNewId();
const long PlayListItemScreenMapPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemScreenMapPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemScreenMapPanel::ID_SPINCTRL5 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemScreenMapPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemScreenMapPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemScreenMapPanel::ID_TEXTCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemScreenMapPanel,wxPanel)
	//(*EventTable(PlayListItemScreenMapPanel)
	//*)
END_EVENT_TABLE()

PlayListItemScreenMapPanel::PlayListItemScreenMapPanel(wxWindow* parent, PlayListItemScreenMap* screenMap, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _screenMap = screenMap;

	//(*Initialize(PlayListItemScreenMapPanel)
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
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Rescale = new wxCheckBox(this, ID_CHECKBOX1, _("Rescale"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Rescale->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Rescale, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT3, _("Rescale Quality:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Quality = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Quality->Append(_("Normal"));
	Choice_Quality->SetSelection( Choice_Quality->Append(_("High")) );
	Choice_Quality->Append(_("Bicubic"));
	Choice_Quality->Append(_("Bilinear"));
	Choice_Quality->Append(_("Box Average"));
	FlexGridSizer1->Add(Choice_Quality, 1, wxALL|wxEXPAND, 5);
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
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Width = new wxSpinCtrl(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 100, _T("ID_SPINCTRL1"));
	SpinCtrl_Width->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Width, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Height:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Height = new wxSpinCtrl(this, ID_SPINCTRL5, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 100, _T("ID_SPINCTRL5"));
	SpinCtrl_Height->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Height, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL4, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL4"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL3, _("0.050"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemScreenMapPanel::OnCheckBox_RescaleClick);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it = m->begin(); it != m->end(); ++it)
    {
        Choice_Matrices->AppendString((*it)->GetName());
    }
    if (Choice_Matrices->GetCount() > 0) Choice_Matrices->SetSelection(0);

    TextCtrl_Name->SetValue(screenMap->GetRawName());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)screenMap->GetDuration() / 1000.0));
    Choice_BlendMode->SetSelection(screenMap->GetBlendMode());
    Choice_Matrices->SetStringSelection(screenMap->GetMatrix());
    Choice_Quality->SetStringSelection(screenMap->GetQuality());
    SpinCtrl_X->SetValue(_screenMap->GetX());
    SpinCtrl_Y->SetValue(_screenMap->GetY());
    SpinCtrl_Width->SetValue(_screenMap->GetWidth());
    SpinCtrl_Height->SetValue(_screenMap->GetHeight());
    SpinCtrl_Priority->SetValue(_screenMap->GetPriority());
    CheckBox_Rescale->SetValue(_screenMap->GetRescale());

    ValidateWindow();
}

PlayListItemScreenMapPanel::~PlayListItemScreenMapPanel()
{
	//(*Destroy(PlayListItemScreenMapPanel)
	//*)

    _screenMap->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _screenMap->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _screenMap->SetMatrix(Choice_Matrices->GetStringSelection().ToStdString());
    _screenMap->SetX(SpinCtrl_X->GetValue());
    _screenMap->SetY(SpinCtrl_Y->GetValue());
    _screenMap->SetWidth(SpinCtrl_Width->GetValue());
    _screenMap->SetHeight(SpinCtrl_Height->GetValue());
    _screenMap->SetName(TextCtrl_Name->GetValue().ToStdString());
    _screenMap->SetPriority(SpinCtrl_Priority->GetValue());
    _screenMap->SetRescale(CheckBox_Rescale->GetValue());
    _screenMap->SetQuality(Choice_Quality->GetStringSelection().ToStdString());
}

void PlayListItemScreenMapPanel::ValidateWindow()
{
    if (CheckBox_Rescale->GetValue())
    {
        SpinCtrl_Height->Enable(true);
        SpinCtrl_Width->Enable(true);
        Choice_Quality->Enable(true);
    }
    else
    {
        SpinCtrl_Height->Enable(false);
        SpinCtrl_Width->Enable(false);
        Choice_Quality->Enable(false);
    }
}

void PlayListItemScreenMapPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _screenMap->SetName(TextCtrl_Name->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemScreenMapPanel::OnCheckBox_RescaleClick(wxCommandEvent& event)
{
    ValidateWindow();
}

