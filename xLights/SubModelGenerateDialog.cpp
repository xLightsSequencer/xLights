#include "SubModelGenerateDialog.h"

//(*InternalHeaders(SubModelGenerateDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SubModelGenerateDialog)
const long SubModelGenerateDialog::ID_STATICTEXT1 = wxNewId();
const long SubModelGenerateDialog::ID_TEXTCTRL1 = wxNewId();
const long SubModelGenerateDialog::ID_STATICTEXT2 = wxNewId();
const long SubModelGenerateDialog::ID_CHOICE1 = wxNewId();
const long SubModelGenerateDialog::ID_STATICTEXT3 = wxNewId();
const long SubModelGenerateDialog::ID_SPINCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SubModelGenerateDialog,wxDialog)
	//(*EventTable(SubModelGenerateDialog)
	//*)
END_EVENT_TABLE()

SubModelGenerateDialog::SubModelGenerateDialog(wxWindow* parent, int modelWidth, int modelHeight, int modelNodes, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _modelWidth = modelWidth;
    _modelHeight = modelHeight;
    _modelNodes = modelNodes;

	//(*Initialize(SubModelGenerateDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Base Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_BaseName = new wxTextCtrl(this, ID_TEXTCTRL1, _("SubModel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_BaseName, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Count = new wxSpinCtrl(this, ID_SPINCTRL1, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 2, 100, 5, _T("ID_SPINCTRL1"));
	SpinCtrl_Count->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Count, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SubModelGenerateDialog::OnChoice1Select);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&SubModelGenerateDialog::OnSpinCtrl_CountChange);
	//*)

    Choice1->SetSelection(Choice1->Append(_("Vertical Slices")));
    Choice1->Append(_("Horizontal Slices"));
    Choice1->Append(_("Segments 2 Wide"));
    Choice1->Append(_("Segments 2 High"));
    Choice1->Append(_("Segments 3 Wide"));
    Choice1->Append(_("Segments 3 High"));
    Choice1->Append(_("Nodes"));

    _lastCount = SpinCtrl_Count->GetValue();

    ValidateWindow();
}

SubModelGenerateDialog::~SubModelGenerateDialog()
{
	//(*Destroy(SubModelGenerateDialog)
	//*)
}


void SubModelGenerateDialog::OnChoice1Select(wxCommandEvent& event)
{
    ValidateWindow();
}

int SubModelGenerateDialog::GetCount() const
{
    return SpinCtrl_Count->GetValue();
}

wxString SubModelGenerateDialog::GetBaseName() const
{
    return TextCtrl_BaseName->GetValue();
}

wxString SubModelGenerateDialog::GetType() const
{
    return Choice1->GetStringSelection();
}

void SubModelGenerateDialog::ValidateWindow()
{
    if (GetType() == "Vertical Slices")
    {
        SpinCtrl_Count->SetRange(2,_modelWidth);
    }
    else if (GetType() == "Horizontal Slices")
    {
        SpinCtrl_Count->SetRange(2,_modelHeight);
    }
    else if (GetType() == "Nodes")
    {
        SpinCtrl_Count->SetRange(2,_modelNodes);
    }
    else if (GetType() == "Segments 2 Wide" || GetType() == "Segments 2 High")
    {
        SpinCtrl_Count->SetValue(SpinCtrl_Count->GetValue() - SpinCtrl_Count->GetValue() % 2);
    }
    else if (GetType() == "Segments 3 Wide" || GetType() == "Segments 3 High")
    {
        SpinCtrl_Count->SetValue(SpinCtrl_Count->GetValue() - SpinCtrl_Count->GetValue() % 3);
    }

    _lastCount = SpinCtrl_Count->GetValue();
}

int SubModelGenerateDialog::SetSpinValue(int step, bool down)
{
    int newValue = SpinCtrl_Count->GetValue();

    if (SpinCtrl_Count->GetValue() % step != 0)
    {
        if (down)
        {
            newValue -= SpinCtrl_Count->GetValue() % step;
            if (newValue < step) newValue = step;
            SpinCtrl_Count->SetValue(newValue);
        }
        else
        {
            newValue += (step - SpinCtrl_Count->GetValue() % step);
            if (newValue > SpinCtrl_Count->GetMax() - SpinCtrl_Count->GetMax() % step)
            {
                newValue = SpinCtrl_Count->GetMax() - SpinCtrl_Count->GetMax() % step;
            }
            SpinCtrl_Count->SetValue(newValue);
        }
    }

    return newValue;
}

void SubModelGenerateDialog::OnSpinCtrl_CountChange(wxSpinEvent& event)
{
    bool down = false;
    if (SpinCtrl_Count->GetValue() < _lastCount)
    {
        down = true;
    }

    int newValue = SpinCtrl_Count->GetValue();
    if (GetType() == "Segments 2 Wide" || GetType() == "Segments 2 High")
    {
        newValue = SetSpinValue(2, down);
    }
    else if (GetType() == "Segments 3 Wide" || GetType() == "Segments 3 High")
    {
        newValue = SetSpinValue(3, down);
    }

    _lastCount = newValue;
}
