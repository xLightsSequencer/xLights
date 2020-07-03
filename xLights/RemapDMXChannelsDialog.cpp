/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RemapDMXChannelsDialog.h"

//(*InternalHeaders(RemapDMXChannelsDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(RemapDMXChannelsDialog)
const long RemapDMXChannelsDialog::ID_GRID1 = wxNewId();
const long RemapDMXChannelsDialog::ID_SCROLLEDWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(RemapDMXChannelsDialog,wxDialog)
	//(*EventTable(RemapDMXChannelsDialog)
	//*)
END_EVENT_TABLE()

RemapDMXChannelsDialog::RemapDMXChannelsDialog(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(RemapDMXChannelsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Remap DMX Channels"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	Grid1 = new wxGrid(ScrolledWindow1, ID_GRID1, wxDefaultPosition, wxSize(400,400), 0, _T("ID_GRID1"));
	Grid1->CreateGrid(40,2);
	Grid1->EnableEditing(true);
	Grid1->EnableGridLines(true);
	Grid1->SetDefaultColSize(150, true);
	Grid1->SetColLabelValue(0, _("Map From"));
	Grid1->SetColLabelValue(1, _("Map To"));
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	FlexGridSizer3->Add(Grid1, 0, wxEXPAND, 0);
	ScrolledWindow1->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(ScrolledWindow1);
	FlexGridSizer3->SetSizeHints(ScrolledWindow1);
	FlexGridSizer1->Add(ScrolledWindow1, 1, wxALL|wxEXPAND, 5);
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

    wxGridCellAttr *ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    Grid1->SetColAttr(0, ca);

    wxArrayString mapTo;
    for (int i = 0; i < 40; i++)
    {
        mapTo.push_back(wxString::Format("Channel %d", i + 1));
        Grid1->SetCellValue(i, 0, mapTo[i]);
        Grid1->SetCellValue(i, 1, mapTo[i]);
    }
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(mapTo);
    Grid1->SetDefaultEditor(editor);
}

RemapDMXChannelsDialog::~RemapDMXChannelsDialog()
{
	//(*Destroy(RemapDMXChannelsDialog)
	//*)
}

