/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderProgressDialog.h"
#include "UtilFunctions.h"

//(*InternalHeaders(RenderProgressDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
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

	Create(parent, wxID_ANY, _("Rendering Progress"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
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
	ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	ButtonSizer->Realize();
	FlexGridSizer1->Add(ButtonSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	// Restore the size and position of the dialog
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("RenderProgress", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 450)
            sz.SetWidth(450);
        if (sz.GetHeight() < 400)
            sz.SetHeight(400);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);
}

RenderProgressDialog::~RenderProgressDialog()
{
	//(*Destroy(RenderProgressDialog)
	//*)
    SaveWindowPosition("RenderProgress", this);
}
