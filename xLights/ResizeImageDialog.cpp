#include "ResizeImageDialog.h"

//(*InternalHeaders(ResizeImageDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(ResizeImageDialog)
const long ResizeImageDialog::ID_STATICTEXT1 = wxNewId();
const long ResizeImageDialog::ID_SPINCTRL1 = wxNewId();
const long ResizeImageDialog::ID_STATICTEXT2 = wxNewId();
const long ResizeImageDialog::ID_SPINCTRL2 = wxNewId();
const long ResizeImageDialog::ID_STATICTEXT3 = wxNewId();
const long ResizeImageDialog::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ResizeImageDialog,wxDialog)
	//(*EventTable(ResizeImageDialog)
	//*)
END_EVENT_TABLE()

ResizeImageDialog::ResizeImageDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ResizeImageDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	WidthSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	WidthSpinCtrl->SetValue(_T("0"));
	FlexGridSizer2->Add(WidthSpinCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	HeightSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL2"));
	HeightSpinCtrl->SetValue(_T("0"));
	FlexGridSizer2->Add(HeightSpinCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Scaling Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ResizeChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ResizeChoice->SetSelection( ResizeChoice->Append(_("Normal")) );
	ResizeChoice->Append(_("Bilinear"));
	ResizeChoice->Append(_("Bicubic"));
	ResizeChoice->Append(_("Box Average"));
	ResizeChoice->Append(_("Crop/Border"));
	FlexGridSizer2->Add(ResizeChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

ResizeImageDialog::~ResizeImageDialog()
{
	//(*Destroy(ResizeImageDialog)
	//*)
}

