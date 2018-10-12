#include "SuperStarImportDialog.h"

//(*InternalHeaders(SuperStarImportDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(SuperStarImportDialog)
const long SuperStarImportDialog::ID_CHOICE_SuperStarImportModel = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT1 = wxNewId();
const long SuperStarImportDialog::ID_CHOICE1 = wxNewId();
const long SuperStarImportDialog::ID_SPINCTRL1 = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT39 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_X_Offset = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT24 = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT40 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_Y_Offset = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT42 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_X_Size = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT41 = wxNewId();
const long SuperStarImportDialog::ID_TEXTCTRL_SS_Y_Size = wxNewId();
const long SuperStarImportDialog::ID_STATICTEXT31 = wxNewId();
const long SuperStarImportDialog::ID_CHECKBOX_AverageColors = wxNewId();
//*)

BEGIN_EVENT_TABLE(SuperStarImportDialog,wxDialog)
	//(*EventTable(SuperStarImportDialog)
	//*)
END_EVENT_TABLE()

SuperStarImportDialog::SuperStarImportDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SuperStarImportDialog)
	wxStaticText* StaticText2;
	wxGridBagSizer* GridBagSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("SuperStar Import"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	ChoiceSuperStarImportModel = new wxChoice(this, ID_CHOICE_SuperStarImportModel, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_SuperStarImportModel"));
	GridBagSizer1->Add(ChoiceSuperStarImportModel, wxGBPosition(0, 1), wxGBSpan(1, 3), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Image Resizing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ImageResizeChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ImageResizeChoice->SetSelection( ImageResizeChoice->Append(_("None")) );
	ImageResizeChoice->Append(_("Exact Width"));
	ImageResizeChoice->Append(_("Exact Height"));
	ImageResizeChoice->Append(_("Exact Width or Height"));
	ImageResizeChoice->Append(_("All"));
	GridBagSizer1->Add(ImageResizeChoice, wxGBPosition(4, 1), wxGBSpan(1, 3), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Timing Adjustment (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 10000, 0, _T("ID_SPINCTRL1"));
	TimeAdjSpinCtrl->SetValue(_T("0"));
	GridBagSizer1->Add(TimeAdjSpinCtrl, wxGBPosition(5, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText40 = new wxStaticText(this, ID_STATICTEXT39, _("Select Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
	GridBagSizer1->Add(StaticText40, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_X_Offset = new wxTextCtrl(this, ID_TEXTCTRL_SS_X_Offset, _("0"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_X_Offset"));
	GridBagSizer1->Add(TextCtrl_SS_X_Offset, wxGBPosition(1, 3), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	StaticText37 = new wxStaticText(this, ID_STATICTEXT24, _("X Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
	GridBagSizer1->Add(StaticText37, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText41 = new wxStaticText(this, ID_STATICTEXT40, _("Y Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
	GridBagSizer1->Add(StaticText41, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_Y_Offset = new wxTextCtrl(this, ID_TEXTCTRL_SS_Y_Offset, _("0"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_Y_Offset"));
	GridBagSizer1->Add(TextCtrl_SS_Y_Offset, wxGBPosition(2, 3), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
	StaticText44 = new wxStaticText(this, ID_STATICTEXT42, _("X Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
	GridBagSizer1->Add(StaticText44, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_X_Size = new wxTextCtrl(this, ID_TEXTCTRL_SS_X_Size, _("12"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_X_Size"));
	GridBagSizer1->Add(TextCtrl_SS_X_Size, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText42 = new wxStaticText(this, ID_STATICTEXT41, _("Y Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
	GridBagSizer1->Add(StaticText42, wxGBPosition(2, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SS_Y_Size = new wxTextCtrl(this, ID_TEXTCTRL_SS_Y_Size, _("50"), wxDefaultPosition, wxSize(50,-1), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_SS_Y_Size"));
	GridBagSizer1->Add(TextCtrl_SS_Y_Size, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText39 = new wxStaticText(this, ID_STATICTEXT31, _("SuperStar: This is currently designed to work with SuperStar sequences\nfor CCR ribbon trees.  X/Y Size and Offset only apply to SuperStar. \nX/Y Size is ignored unless the sequence type is a Visualization.\n"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
	GridBagSizer1->Add(StaticText39, wxGBPosition(8, 0), wxGBSpan(1, 5), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_AverageColors = new wxCheckBox(this, ID_CHECKBOX_AverageColors, _("Average Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AverageColors"));
	CheckBox_AverageColors->SetValue(true);
	GridBagSizer1->Add(CheckBox_AverageColors, wxGBPosition(6, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(GridBagSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
    SetEscapeId(wxID_CANCEL);
}

SuperStarImportDialog::~SuperStarImportDialog()
{
	//(*Destroy(SuperStarImportDialog)
	//*)
}

