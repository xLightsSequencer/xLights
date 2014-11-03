#include "dlgPreviewSize.h"

//(*InternalHeaders(dlgPreviewSize)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(dlgPreviewSize)
const long dlgPreviewSize::ID_STATICTEXT1 = wxNewId();
const long dlgPreviewSize::ID_TEXTCTRL_PREVIEW_WIDTH = wxNewId();
const long dlgPreviewSize::ID_STATICTEXT2 = wxNewId();
const long dlgPreviewSize::ID_TEXTCTRL_PREVIEW_HEIGHT = wxNewId();
//*)

BEGIN_EVENT_TABLE(dlgPreviewSize,wxDialog)
	//(*EventTable(dlgPreviewSize)
	//*)
END_EVENT_TABLE()

dlgPreviewSize::dlgPreviewSize(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(dlgPreviewSize)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Set Preview Size"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(191,110));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_PreviewWidth = new wxTextCtrl(this, ID_TEXTCTRL_PREVIEW_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_WIDTH"));
	FlexGridSizer2->Add(TextCtrl_PreviewWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Height:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_PreviewHeight = new wxTextCtrl(this, ID_TEXTCTRL_PREVIEW_HEIGHT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_HEIGHT"));
	FlexGridSizer2->Add(TextCtrl_PreviewHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	//*)
}

dlgPreviewSize::~dlgPreviewSize()
{
	//(*Destroy(dlgPreviewSize)
	//*)
}

