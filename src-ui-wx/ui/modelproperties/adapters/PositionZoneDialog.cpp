#include "PositionZoneDialog.h"

//(*InternalHeaders(PositionZoneDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/msgdlg.h>

//(*IdInit(PositionZoneDialog)
const wxWindowID PositionZoneDialog::ID_GRID_Zones = wxNewId();
const wxWindowID PositionZoneDialog::ID_BUTTON_AddZone = wxNewId();
const wxWindowID PositionZoneDialog::ID_BUTTON_DeleteZone = wxNewId();
//*)

BEGIN_EVENT_TABLE(PositionZoneDialog,wxDialog)
    //(*EventTable(PositionZoneDialog)
    //*)
END_EVENT_TABLE()

PositionZoneDialog::PositionZoneDialog(std::vector<PositionZone>& zones, wxWindow* parent, wxWindowID id) : _zones(zones) {
    //(*Initialize(PositionZoneDialog)
    wxFlexGridSizer* FlexGridSizer2;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Position Zones"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    Grid_Zones = new wxGrid(this, ID_GRID_Zones, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_Zones"));
    Grid_Zones->CreateGrid(0,6);
    Grid_Zones->EnableEditing(true);
    Grid_Zones->EnableGridLines(true);
    Grid_Zones->SetColLabelValue(0, _("Pan Min"));
    Grid_Zones->SetColLabelValue(1, _("Pan Max"));
    Grid_Zones->SetColLabelValue(2, _("Tilt Min"));
    Grid_Zones->SetColLabelValue(3, _("Tilt Max"));
    Grid_Zones->SetColLabelValue(4, _("Channel"));
    Grid_Zones->SetColLabelValue(5, _("Value"));
    Grid_Zones->SetDefaultCellFont( Grid_Zones->GetFont() );
    Grid_Zones->SetDefaultCellTextColour( Grid_Zones->GetForegroundColour() );
    FlexGridSizer1->Add(Grid_Zones, 1, wxALL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_AddZone = new wxButton(this, ID_BUTTON_AddZone, _("Add Zone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_AddZone"));
    FlexGridSizer2->Add(Button_AddZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_DeleteZone = new wxButton(this, ID_BUTTON_DeleteZone, _("Delete Zone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DeleteZone"));
    FlexGridSizer2->Add(Button_DeleteZone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_GRID_Zones, wxEVT_GRID_CELL_CHANGED, (wxObjectEventFunction)&PositionZoneDialog::OnGrid_ZonesCellChanged);
    Connect(ID_BUTTON_AddZone, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PositionZoneDialog::OnButton_AddZoneClick);
    Connect(ID_BUTTON_DeleteZone, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PositionZoneDialog::OnButton_DeleteZoneClick);
    //*)

    for (const auto& zone : _zones) {
        int row = Grid_Zones->GetNumberRows();
        Grid_Zones->AppendRows(1);
        Grid_Zones->SetCellValue(row, 0, wxString::Format("%d", zone.pan_min));
        Grid_Zones->SetCellValue(row, 1, wxString::Format("%d", zone.pan_max));
        Grid_Zones->SetCellValue(row, 2, wxString::Format("%d", zone.tilt_min));
        Grid_Zones->SetCellValue(row, 3, wxString::Format("%d", zone.tilt_max));
        Grid_Zones->SetCellValue(row, 4, wxString::Format("%d", zone.channel));
        Grid_Zones->SetCellValue(row, 5, wxString::Format("%d", zone.value));
    }
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

PositionZoneDialog::~PositionZoneDialog()
{
    //(*Destroy(PositionZoneDialog)
    //*)
}


void PositionZoneDialog::OnButton_AddZoneClick(wxCommandEvent& event)
{
    int row = Grid_Zones->GetNumberRows();
    Grid_Zones->AppendRows(1);
    Grid_Zones->SetCellValue(row, 0, "0");
    Grid_Zones->SetCellValue(row, 1, "255");
    Grid_Zones->SetCellValue(row, 2, "0");
    Grid_Zones->SetCellValue(row, 3, "255");
    Grid_Zones->SetCellValue(row, 4, "1");
    Grid_Zones->SetCellValue(row, 5, "0");

    PositionZone zone;
    _zones.push_back(zone);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

void PositionZoneDialog::OnButton_DeleteZoneClick(wxCommandEvent& event)
{
    int row = Grid_Zones->GetGridCursorRow();
    if (row < 0 || row >= (int)_zones.size())
        return;

    if (wxMessageBox("Are you sure you want to delete this zone?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        return;

    _zones.erase(_zones.begin() + row);
    Grid_Zones->DeleteRows(row);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

void PositionZoneDialog::OnGrid_ZonesCellChanged(wxGridEvent& event)
{
    int row = event.GetRow();
    int col = event.GetCol();

    if (row < 0 || row >= (int)_zones.size())
        return;

    int val = wxAtoi(Grid_Zones->GetCellValue(row, col));
    wxString errMsg;

    switch (col) {
    case 0: // Pan Min
        if (val < 0 || val > 255)
            errMsg = "Pan Min must be 0-255.";
        else if (val > _zones[row].pan_max)
            errMsg = "Pan Min must be <= Pan Max.";
        else
            _zones[row].pan_min = val;
        break;
    case 1: // Pan Max
        if (val < 0 || val > 255)
            errMsg = "Pan Max must be 0-255.";
        else if (val < _zones[row].pan_min)
            errMsg = "Pan Max must be >= Pan Min.";
        else
            _zones[row].pan_max = val;
        break;
    case 2: // Tilt Min
        if (val < 0 || val > 255)
            errMsg = "Tilt Min must be 0-255.";
        else if (val > _zones[row].tilt_max)
            errMsg = "Tilt Min must be <= Tilt Max.";
        else
            _zones[row].tilt_min = val;
        break;
    case 3: // Tilt Max
        if (val < 0 || val > 255)
            errMsg = "Tilt Max must be 0-255.";
        else if (val < _zones[row].tilt_min)
            errMsg = "Tilt Max must be >= Tilt Min.";
        else
            _zones[row].tilt_max = val;
        break;
    case 4: // Channel
        if (val < 1)
            errMsg = "Channel must be >= 1.";
        else
            _zones[row].channel = val;
        break;
    case 5: // Value
        if (val < 0 || val > 255)
            errMsg = "Value must be 0-255.";
        else
            _zones[row].value = (uint8_t)val;
        break;
    }

    if (!errMsg.empty()) {
        wxMessageBox(errMsg, "Invalid Value", wxOK | wxICON_WARNING, this);
        // revert the cell to the current stored value
        switch (col) {
        case 0: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].pan_min)); break;
        case 1: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].pan_max)); break;
        case 2: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].tilt_min)); break;
        case 3: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].tilt_max)); break;
        case 4: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].channel)); break;
        case 5: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].value)); break;
        }
    }
}
