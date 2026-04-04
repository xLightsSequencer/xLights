#include "AboutDialog.h"

//(*InternalHeaders(AboutDialog)
#include <wx/button.h>
#include <wx/hyperlink.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(AboutDialog)
const long AboutDialog::ID_STATICBITMAP1 = wxNewId();
const long AboutDialog::ID_STATICTEXT1 = wxNewId();
const long AboutDialog::ID_STATICTEXT2 = wxNewId();
const long AboutDialog::ID_HYPERLINKCTRL1 = wxNewId();
const long AboutDialog::ID_HYPERLINKCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AboutDialog,wxDialog)
	//(*EventTable(AboutDialog)
	//*)
END_EVENT_TABLE()

AboutDialog::AboutDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(AboutDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticText* StaticText1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("About xLights"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	MainSizer = new wxFlexGridSizer(0, 1, 0, 0);
	MainSizer->AddGrowableCol(0);
	MainSizer->AddGrowableRow(1);
	IconSizer = new wxFlexGridSizer(0, 3, 0, 0);
	IconBitmap = new wxStaticBitmap(this, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_STATICBITMAP1"));
	IconBitmap->SetMinSize(wxSize(128,128));
	IconSizer->Add(IconBitmap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	MainSizer->Add(IconSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("xLights"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	VersionLabel = new wxStaticText(this, ID_STATICTEXT1, _("Version:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(VersionLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LegalTextLabel = new wxStaticText(this, ID_STATICTEXT2, _("LegalText"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	LegalTextLabel->SetMaxSize(wxSize(400,-1));
	FlexGridSizer3->Add(LegalTextLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	PrivacyHyperlinkCtrl = new wxHyperlinkCtrl(this, ID_HYPERLINKCTRL1, _("Privacy Policy"), _("https://xlights.org/privacy-policy/"), wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_CENTRE|wxNO_BORDER, _T("ID_HYPERLINKCTRL1"));
	FlexGridSizer4->Add(PrivacyHyperlinkCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	EULAHyperlinkCtrl = new wxHyperlinkCtrl(this, ID_HYPERLINKCTRL2, _("https://xlights.org/privacy-policy/"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_CENTRE|wxNO_BORDER, _T("ID_HYPERLINKCTRL2"));
	EULAHyperlinkCtrl->Hide();
	FlexGridSizer4->Add(EULAHyperlinkCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	MainSizer->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	MainSizer->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(MainSizer);
	MainSizer->Fit(this);
	MainSizer->SetSizeHints(this);
	//*)
}

AboutDialog::~AboutDialog()
{
	//(*Destroy(AboutDialog)
	//*)
}

