#include "SuperStarImportDialog.h"

//(*InternalHeaders(SuperStarImportDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SuperStarImportDialog)
const long SuperStarImportDialog::ID_STATICTEXT39 = wxNewId();
const long SuperStarImportDialog::ID_CHOICE_SuperStarImportModel = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT24 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_X_Size = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT42 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_X_Offset = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT40 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_Y_Size = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT41 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_Y_Offset = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT1 = wxNewId();
const long SuperStarImportDialog::ID_CHOICE1 = wxNewId();
const long SuperStarImportDialog::ID_SPINCTRL1 = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT2 = wxNewId();
const long SuperStarImportDialog::ID_CHOICE2 = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT31 = wxNewId();
const long SuperStarImportDialog::ID_BUTTON1 = wxNewId();
const long SuperStarImportDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(SuperStarImportDialog,wxDialog)
	//(*EventTable(SuperStarImportDialog)
	//*)
END_EVENT_TABLE()

SuperStarImportDialog::SuperStarImportDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SuperStarImportDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticText* StaticText2;

	Create(parent, id, _("SuperStar Import"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText40 = new wxStaticText(this, ID_STATICTEXT39, _("Select Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
	FlexGridSizer4->Add(StaticText40, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceSuperStarImportModel = new wxChoice(this, ID_CHOICE_SuperStarImportModel, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_SuperStarImportModel"));
	FlexGridSizer4->Add(ChoiceSuperStarImportModel, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText37 = new wxStaticText(this, ID_STATICTEXT24, _("X Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
	FlexGridSizer5->Add(StaticText37, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_X_Size = new wxTextCtrl(this, ID_TEXTCTRL_SS_X_Size, _("12"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_X_Size"));
	FlexGridSizer5->Add(TextCtrl_SS_X_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText44 = new wxStaticText(this, ID_STATICTEXT42, _("X Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
	FlexGridSizer5->Add(StaticText44, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_X_Offset = new wxTextCtrl(this, ID_TEXTCTRL_SS_X_Offset, _("0"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_X_Offset"));
	FlexGridSizer5->Add(TextCtrl_SS_X_Offset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	FlexGridSizer5->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText41 = new wxStaticText(this, ID_STATICTEXT40, _("Y Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
	FlexGridSizer5->Add(StaticText41, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_Y_Size = new wxTextCtrl(this, ID_TEXTCTRL_SS_Y_Size, _("50"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_Y_Size"));
	FlexGridSizer5->Add(TextCtrl_SS_Y_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText42 = new wxStaticText(this, ID_STATICTEXT41, _("Y Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
	FlexGridSizer5->Add(StaticText42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_Y_Offset = new wxTextCtrl(this, ID_TEXTCTRL_SS_Y_Offset, _("0"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_Y_Offset"));
	FlexGridSizer5->Add(TextCtrl_SS_Y_Offset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Image Resizing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer6->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ImageResizeChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ImageResizeChoice->SetSelection( ImageResizeChoice->Append(_("None")) );
	ImageResizeChoice->Append(_("Exact Width"));
	ImageResizeChoice->Append(_("Exact Height"));
	ImageResizeChoice->Append(_("Exact Width or Height"));
	ImageResizeChoice->Append(_("All"));
	FlexGridSizer6->Add(ImageResizeChoice, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Timing Adjustment (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 10000, 0, _T("ID_SPINCTRL1"));
	TimeAdjSpinCtrl->SetValue(_T("0"));
	FlexGridSizer6->Add(TimeAdjSpinCtrl, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Layer Blend:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_LayerBlend = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_LayerBlend->Append(_("Average"));
	Choice_LayerBlend->Append(_("Normal"));
	Choice_LayerBlend->SetSelection( Choice_LayerBlend->Append(_("2 reveals 1")) );
	FlexGridSizer6->Add(Choice_LayerBlend, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	StaticText39 = new wxStaticText(this, ID_STATICTEXT31, _("SuperStar: This is currently designed to work with SuperStar sequences\nfor CCR ribbon trees.  X/Y Size and Offset only apply to SuperStar. \nX/Y Size is ignored unless the sequence type is a Visualization.\n"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
	FlexGridSizer3->Add(StaticText39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_SuperStarImportModel,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SuperStarImportDialog::OnChoiceSuperStarImportModelSelect);
	Connect(ID_TEXTCTRL_SS_X_Size,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SuperStarImportDialog::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_SS_X_Offset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SuperStarImportDialog::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_SS_Y_Size,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SuperStarImportDialog::OnTextCtrl_Text);
	Connect(ID_TEXTCTRL_SS_Y_Offset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SuperStarImportDialog::OnTextCtrl_Text);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SuperStarImportDialog::OnButton_OkClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SuperStarImportDialog::OnButton_CancelClick);
	//*)
    SetEscapeId(ID_BUTTON_CANCEL);
    ValidateWindow();
}

SuperStarImportDialog::~SuperStarImportDialog()
{
	//(*Destroy(SuperStarImportDialog)
	//*)
}

void SuperStarImportDialog::ValidateWindow()
{
    if (ChoiceSuperStarImportModel->GetStringSelection() == "" ||
        wxAtoi(TextCtrl_SS_X_Size->GetValue()) <= 0 ||
        wxAtoi(TextCtrl_SS_Y_Size->GetValue()) <= 0 ||
        !TextCtrl_SS_X_Offset->GetValue().IsNumber() ||
        !TextCtrl_SS_Y_Offset->GetValue().IsNumber()
        )
    {
        Button_Ok->Disable();
    }
    else
    {
        Button_Ok->Enable();
    }
}

void SuperStarImportDialog::OnChoiceSuperStarImportModelSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void SuperStarImportDialog::OnTextCtrl_Text(wxCommandEvent& event)
{
    ValidateWindow();
}

void SuperStarImportDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void SuperStarImportDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
