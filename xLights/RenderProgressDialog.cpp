/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderProgressDialog.h"

//(*InternalHeaders(RenderProgressDialog)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(RenderProgressDialog)
const long RenderProgressDialog::ID_SCROLLEDWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(RenderProgressDialog,wxDialog)
	//(*EventTable(RenderProgressDialog)
	//*)
END_EVENT_TABLE()

RenderProgressDialog::RenderProgressDialog(wxWindow* parent)
{
	//(*Initialize(RenderProgressDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Rendering Progress"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	scrolledWindow = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_SCROLLEDWINDOW1"));
	scrolledWindowSizer = new wxFlexGridSizer(0, 2, 0, 0);
	scrolledWindowSizer->AddGrowableCol(1);
	scrolledWindow->SetSizer(scrolledWindowSizer);
	scrolledWindowSizer->Fit(scrolledWindow);
	scrolledWindowSizer->SetSizeHints(scrolledWindow);
	FlexGridSizer1->Add(scrolledWindow, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

RenderProgressDialog::~RenderProgressDialog()
{
	//(*Destroy(RenderProgressDialog)
	//*)
}

