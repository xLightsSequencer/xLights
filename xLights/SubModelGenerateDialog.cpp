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
	Choice1->SetSelection( Choice1->Append(_("Horizontal Subbuffer")) );
	Choice1->Append(_("Vertical Subbuffer"));
	Choice1->Append(_("Nodes"));
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Count = new wxSpinCtrl(this, ID_SPINCTRL1, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 2, 100, 5, _T("ID_SPINCTRL1"));
	SpinCtrl_Count->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Count, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SubModelGenerateDialog::OnChoice1Select);
	//*)

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
    if (GetType() == "Horizontal Subbuffer")
    {
        SpinCtrl_Count->SetRange(2,_modelWidth);
    }
    else if (GetType() == "Vertical Subbuffer")
    {
        SpinCtrl_Count->SetRange(2,_modelHeight);
    }
    else if (GetType() == "Nodes")
    {
        SpinCtrl_Count->SetRange(2,_modelNodes);
    }
}
