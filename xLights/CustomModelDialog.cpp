#include "CustomModelDialog.h"
#include <wx/clipbrd.h>

//(*InternalHeaders(CustomModel)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(CustomModel)
const long CustomModelDialog::ID_GRID1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(CustomModelDialog,wxDialog)
	//(*EventTable(CustomModel)
	//*)
END_EVENT_TABLE()

CustomModelDialog::CustomModelDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(CustomModel)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	gdModelChans = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxSize(407,430), wxVSCROLL|wxHSCROLL|wxFULL_REPAINT_ON_RESIZE, _T("ID_GRID1"));
	gdModelChans->CreateGrid(1,1);
	gdModelChans->EnableEditing(true);
	gdModelChans->EnableGridLines(true);
	gdModelChans->SetColLabelSize(20);
	gdModelChans->SetRowLabelSize(30);
	gdModelChans->SetDefaultColSize(30, true);
	gdModelChans->SetDefaultCellFont( gdModelChans->GetFont() );
	gdModelChans->SetDefaultCellTextColour( gdModelChans->GetForegroundColour() );
	FlexGridSizer1->Add(gdModelChans, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	gdModelChans->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(CustomModelDialog::onKeyDown_gdModelChans),
                          (wxObject*) NULL, this);
}

CustomModelDialog::~CustomModelDialog()
{
	//(*Destroy(CustomModel)
	//*)
}

void CustomModelDialog::onKeyDown_gdModelChans(wxKeyEvent& event)
{

    if ((event.GetUnicodeKey() == 'C') && (event.ControlDown() == true))
    {
        CopyData( (wxCommandEvent&) event);
    }
    else if ((event.GetUnicodeKey() == 'V') && (event.ControlDown() == true))
    {
        PasteData( (wxCommandEvent&) event );
    }
    event.Skip();
}



void CustomModelDialog::CopyData( wxCommandEvent& WXUNUSED(ev) )
{
int i,k;
wxString copy_data;
bool something_in_this_line;

    copy_data.Clear();

    for (i=0; i< gdModelChans->GetRows(); i++) {     // step through all lines
        something_in_this_line = false;     // nothing found yet
        for (k=0; k<gdModelChans->GetCols(); k++) { // step through all colums
            if (gdModelChans->IsInSelection(i,k)) { // this field is selected!!!
                if (something_in_this_line == false) {  // first field in this line => may need a linefeed
                    if (copy_data.IsEmpty() == false) {     // ... if it is not the very first field
                        copy_data = copy_data + wxT("\n");  // next LINE
                    }
                    something_in_this_line = true;
                } else {                                // if not the first field in this line we need a field seperator (TAB)
                    copy_data = copy_data + wxT("\t");  // next COLUMN
                }
                copy_data = copy_data + gdModelChans->GetCellValue(i,k);    // finally we need the field value :-)
            }
        }
    }
#ifndef __WXOSX__
    wxOpenClipboard();          // now copy all these things into the clipbord
    wxEmptyClipboard();
    wxSetClipboardData(wxDF_TEXT,copy_data.c_str(),0,0);
    wxCloseClipboard();
#endif
}

void CustomModelDialog::PasteData( wxCommandEvent& WXUNUSED(ev) )
{
    wxString copy_data;
    wxString cur_field;
    wxString cur_line;
    int i,k,k2;

#ifndef __WXOSX__
    wxOpenClipboard();          // now copy all these things into the clipbord
    copy_data = (char *)wxGetClipboardData(wxDF_TEXT);
    wxCloseClipboard();
#endif

    i = gdModelChans->GetGridCursorRow();
    k = gdModelChans->GetGridCursorCol();
    k2= k;

    do {
        cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        do {
            cur_field = cur_line.BeforeFirst('\t');
            cur_line  = cur_line.AfterFirst ('\t');
            gdModelChans->SetCellValue(i,k,cur_field);
            k++;
        } while(cur_line.IsEmpty() == false);
        i++;
        k = k2;
    } while (copy_data.IsEmpty() == false);
}
