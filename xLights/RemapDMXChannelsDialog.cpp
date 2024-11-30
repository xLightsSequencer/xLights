/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RemapDMXChannelsDialog.h"

//(*InternalHeaders(RemapDMXChannelsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>



//(*IdInit(RemapDMXChannelsDialog)
const long RemapDMXChannelsDialog::ID_GRID1 = wxNewId();
const long RemapDMXChannelsDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long RemapDMXChannelsDialog::ID_BUTTON_OK = wxNewId();
const long RemapDMXChannelsDialog::ID_BUTTON_CANCEL = wxNewId();
const long RemapDMXChannelsDialog::ID_BUTTON_SAVE_MAPPING = wxNewId();
const long RemapDMXChannelsDialog::ID_BUTTON_LOAD_MAPPING = wxNewId();
//*)

BEGIN_EVENT_TABLE(RemapDMXChannelsDialog,wxDialog)
	//(*EventTable(RemapDMXChannelsDialog)
	//*)
END_EVENT_TABLE()

RemapDMXChannelsDialog::RemapDMXChannelsDialog(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(RemapDMXChannelsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, _("Remap DMX Channels"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	Grid1 = new wxGrid(ScrolledWindow1, ID_GRID1, wxDefaultPosition, wxSize(609,400), 0, _T("ID_GRID1"));
	Grid1->CreateGrid(48,4);
	Grid1->EnableEditing(true);
	Grid1->EnableGridLines(true);
	Grid1->SetDefaultColSize(150, true);
	Grid1->SetColLabelValue(0, _("Map From"));
	Grid1->SetColLabelValue(1, _("Map To"));
	Grid1->SetColLabelValue(2, _("Scale"));
	Grid1->SetColLabelValue(3, _("Offset"));
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	FlexGridSizer3->Add(Grid1, 0, wxEXPAND, 0);
	ScrolledWindow1->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(ScrolledWindow1);
	FlexGridSizer3->SetSizeHints(ScrolledWindow1);
	FlexGridSizer1->Add(ScrolledWindow1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_SaveMapping = new wxButton(this, ID_BUTTON_SAVE_MAPPING, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_MAPPING"));
	FlexGridSizer2->Add(Button_SaveMapping, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_LoadMapping = new wxButton(this, ID_BUTTON_LOAD_MAPPING, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LOAD_MAPPING"));
	FlexGridSizer2->Add(Button_LoadMapping, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDMXChannelsDialog::OnButton_OkClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDMXChannelsDialog::OnButton_CancelClick);
	Connect(ID_BUTTON_SAVE_MAPPING,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDMXChannelsDialog::OnButton_SaveMappingClick);
	Connect(ID_BUTTON_LOAD_MAPPING,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDMXChannelsDialog::OnButton_LoadMappingClick);
	//*)

    SetEscapeId(ID_BUTTON_CANCEL);

    wxGridCellAttr *ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    Grid1->SetColAttr(0, ca);

    Grid1->SetColFormatFloat(2,3,2);
    Grid1->SetColFormatNumber(3);
    Grid1->SetColSize(2, 75);
    Grid1->SetColSize(3, 75);

    wxArrayString mapTo;
    for (int i = 0; i < 48; i++)
    {
        mapTo.push_back(wxString::Format("Channel %d", i + 1));
        Grid1->SetCellValue(i, DMXMappingColumns::From, mapTo[i]);
        Grid1->SetCellValue(i, DMXMappingColumns::To, mapTo[i]);
        Grid1->SetCellValue(i, DMXMappingColumns::Scale, "1.00");
        Grid1->SetCellValue(i, DMXMappingColumns::Offset, "0");
    }
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(mapTo);
    Grid1->SetDefaultEditor(editor);
}

RemapDMXChannelsDialog::~RemapDMXChannelsDialog()
{
	//(*Destroy(RemapDMXChannelsDialog)
	//*)
}


void RemapDMXChannelsDialog::OnButton_OkClick(wxCommandEvent& event)
{
	EndDialog(wxID_OK);
}

void RemapDMXChannelsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
	EndDialog(wxID_CANCEL);
}

void RemapDMXChannelsDialog::OnButton_SaveMappingClick(wxCommandEvent& event)
{
	wxFileDialog dlg(this, "Save mapping", wxEmptyString, "DMXMapping", "Mapping Files (*.xdmxmap)|*.xdmxmap|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileOutputStream output(dlg.GetPath());
		if (output.IsOk()) {
			wxTextOutputStream text(output);
			for (int i = 0; i < Grid1->GetNumberRows(); i++) {
				text.WriteString(wxString::Format("%s,%s,%s,%s\n", 
					Grid1->GetCellValue(i, DMXMappingColumns::From),
					Grid1->GetCellValue(i, DMXMappingColumns::To),
					Grid1->GetCellValue(i, DMXMappingColumns::Scale), 
					Grid1->GetCellValue(i, DMXMappingColumns::Offset)));
			}
			output.Close();
		}
	}
}

bool RemapDMXChannelsDialog::IsValidRemapping(const wxString& value)
{
	for (int ii = 0; ii < Grid1->GetNumberRows(); ii++) {
		if (Grid1->GetCellValue(ii, DMXMappingColumns::From) == value) {
			return true;
		}
	}
	return false;
}

double RemapDMXChannelsDialog::GetChanScale(int idx)
{
	return wxAtof(Grid1->GetCellValue(idx, DMXMappingColumns::Scale));
}

int RemapDMXChannelsDialog::GetChanOffset(int idx)
{
	return wxAtoi(Grid1->GetCellValue(idx, DMXMappingColumns::Offset));
}

bool RemapDMXChannelsDialog::DoMapping(int idx)
{
	return Grid1->GetCellValue(idx, DMXMappingColumns::From) != Grid1->GetCellValue(idx, DMXMappingColumns::To)
		|| GetChanOffset(idx) != 0 
		|| abs(GetChanScale(idx) - 1.0) > 0.0001;
}

int RemapDMXChannelsDialog::GetToChannel(int idx)
{
	return wxAtoi(Grid1->GetCellValue(idx,  DMXMappingColumns::To).AfterLast(' '));
}

void RemapDMXChannelsDialog::OnButton_LoadMappingClick(wxCommandEvent& event)
{
	wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xdmxmap)|*.xdmxmap|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileInputStream input(dlg.GetPath());
		if (input.IsOk()) {
			// reset the mappings
			for (int ii = 0; ii < Grid1->GetNumberRows(); ii++) {
				Grid1->SetCellValue(ii, DMXMappingColumns::To, Grid1->GetCellValue(ii, DMXMappingColumns::From));
			}

			int i = 0;
			wxTextInputStream text(input, ",");
			auto line = text.ReadLine();
			while (line != "" && i < Grid1->GetNumberRows()) {
				auto const ll = wxSplit(line, ',');
				if (ll.size() == 2 || ll.size() == 4) {
                    if (Grid1->GetCellValue(i, DMXMappingColumns::From) == ll[0]) {
						if (IsValidRemapping(ll[1])) {
                            Grid1->SetCellValue(i, DMXMappingColumns::To, ll[1]);
                            if (ll.size() == 4) {
                                Grid1->SetCellValue(i, DMXMappingColumns::Scale, ll[2]);
                                Grid1->SetCellValue(i, DMXMappingColumns::Offset, ll[3]);
                            }
						}
					}
					else {
						for (int ii = 0; ii < Grid1->GetNumberRows(); ii++) {
                            if (Grid1->GetCellValue(ii, DMXMappingColumns::From) == ll[0]) {
								if (IsValidRemapping(ll[1])) {
                                    Grid1->SetCellValue(ii, DMXMappingColumns::To, ll[1]);
                                    if (ll.size() == 4) {
                                        Grid1->SetCellValue(ii, DMXMappingColumns::Scale, ll[2]);
                                        Grid1->SetCellValue(ii, DMXMappingColumns::Offset, ll[3]);
                                    }
								}
								ii = Grid1->GetNumberRows();
								i = ii;
							}
						}
					}
				}

				line = text.ReadLine();
				i++;
			}
		}
	}
}
