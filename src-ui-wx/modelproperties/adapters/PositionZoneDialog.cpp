#include "PositionZoneDialog.h"

//(*InternalHeaders(PositionZoneDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>

#include <spdlog/spdlog.h>

#include "models/ModelManager.h"
#include "models/OutputModelManager.h"
#include "shared/dialogs/CheckboxSelectDialog.h"

//(*IdInit(PositionZoneDialog)
const wxWindowID PositionZoneDialog::ID_GRID_Zones = wxNewId();
const wxWindowID PositionZoneDialog::ID_BUTTON_AddZone = wxNewId();
const wxWindowID PositionZoneDialog::ID_BUTTON_DeleteZone = wxNewId();
//*)
const wxWindowID PositionZoneDialog::ID_BUTTON_ExportZones = wxNewId();
const wxWindowID PositionZoneDialog::ID_BUTTON_ImportZones = wxNewId();

BEGIN_EVENT_TABLE(PositionZoneDialog,wxDialog)
    //(*EventTable(PositionZoneDialog)
    //*)
END_EVENT_TABLE()

PositionZoneDialog::PositionZoneDialog(DmxMovingHeadAdv& model, std::vector<PositionZone>& zones, wxWindow* parent) : _model(model), _zones(zones) {
    //(*Initialize(PositionZoneDialog)
    wxFlexGridSizer* FlexGridSizer2;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Position Zones"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    Grid_Zones = new wxGrid(this, ID_GRID_Zones, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_Zones"));
    Grid_Zones->CreateGrid(0,7);
    Grid_Zones->EnableEditing(true);
    Grid_Zones->EnableGridLines(true);
    Grid_Zones->SetColLabelValue(0, _("Label"));
    Grid_Zones->SetColLabelValue(1, _("Pan Min"));
    Grid_Zones->SetColLabelValue(2, _("Pan Max"));
    Grid_Zones->SetColLabelValue(3, _("Tilt Min"));
    Grid_Zones->SetColLabelValue(4, _("Tilt Max"));
    Grid_Zones->SetColLabelValue(5, _("Channel"));
    Grid_Zones->SetColLabelValue(6, _("Value"));
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

    // export/import zones to/from other moving head props in the layout
    Button_ExportZones = new wxButton(this, ID_BUTTON_ExportZones, _("Export Zones..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ExportZones"));
    FlexGridSizer2->Add(Button_ExportZones, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ImportZones = new wxButton(this, ID_BUTTON_ImportZones, _("Import Zones..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ImportZones"));
    FlexGridSizer2->Add(Button_ImportZones, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Connect(ID_BUTTON_ExportZones, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PositionZoneDialog::OnButton_ExportZonesClick);
    Connect(ID_BUTTON_ImportZones, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&PositionZoneDialog::OnButton_ImportZonesClick);

    // instruction text above the grid
    const wxString bullet(wxUniChar(0x2022));
    const wxString helpTextLabel = wxString::Format(
        _("Define zones that trigger a DMX channel output when the moving head enters that pan/tilt range.\n"
          "  %s  Label: optional word or two to describe this zone\n"
          "  %s  Pan Min/Max: pan channel value range (0-255) that defines this zone\n"
          "  %s  Tilt Min/Max: tilt channel value range (0-255) that defines this zone\n"
          "  %s  Channel: DMX channel number to set when the head is inside this zone\n"
          "  %s  Value: DMX value (0-255) to send on that channel"),
        bullet, bullet, bullet, bullet, bullet);
    wxStaticText* helpText = new wxStaticText(this, wxID_ANY, helpTextLabel);
    FlexGridSizer1->Prepend(helpText, 0, wxALL | wxEXPAND, 8);
    FlexGridSizer1->RemoveGrowableRow(0);
    FlexGridSizer1->AddGrowableRow(1);

    // per-column spin editors with min/max ranges
    auto makeAttr = [](int min, int max) {
        wxGridCellAttr* attr = new wxGridCellAttr();
        attr->SetEditor(new wxGridCellNumberEditor(min, max));
        attr->SetRenderer(new wxGridCellNumberRenderer());
        return attr;
    };
    Grid_Zones->SetColAttr(1, makeAttr(0, 255));   // Pan Min
    Grid_Zones->SetColAttr(2, makeAttr(0, 255));   // Pan Max
    Grid_Zones->SetColAttr(3, makeAttr(0, 255));   // Tilt Min
    Grid_Zones->SetColAttr(4, makeAttr(0, 255));   // Tilt Max
    Grid_Zones->SetColAttr(5, makeAttr(1, 512));   // Channel
    Grid_Zones->SetColAttr(6, makeAttr(0, 255));   // Value

    for (const auto& zone : _zones) {
        AppendZoneRow(zone);
    }
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

PositionZoneDialog::~PositionZoneDialog()
{
    //(*Destroy(PositionZoneDialog)
    //*)

    // Safe to notify now: ShowModal() has already returned by this point (this
    // dialog's destructor cannot run while its own modal loop is still active),
    // so the deferred rerender/save work can no longer land mid-dialog.
    FlushPendingExportNotifications();
}

void PositionZoneDialog::FlushPendingExportNotifications()
{
    if (_pendingExportNotifyTargets.empty())
        return;

    spdlog::debug("PositionZoneDialog: flushing export notifications for {} target(s) after modal close", _pendingExportNotifyTargets.size());
    for (auto* target : _pendingExportNotifyTargets) {
        target->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PositionZoneDialog::Export");
        target->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PositionZoneDialog::Export");
        target->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PositionZoneDialog::Export");
    }
    _pendingExportNotifyTargets.clear();

    // The AddASAPWork calls above each overwrite the OutputModelManager's "selected model"
    // with the export target's name; put it back on the model whose editor is actually
    // still open so a subsequent property grid reload doesn't jump to the wrong prop.
    _model.AddASAPWork(0, "PositionZoneDialog::RestoreSelection");
}


void PositionZoneDialog::AppendZoneRow(const PositionZone& zone)
{
    int row = Grid_Zones->GetNumberRows();
    Grid_Zones->AppendRows(1);
    Grid_Zones->SetCellValue(row, 0, zone.label);
    Grid_Zones->SetCellValue(row, 1, wxString::Format("%d", zone.pan_min));
    Grid_Zones->SetCellValue(row, 2, wxString::Format("%d", zone.pan_max));
    Grid_Zones->SetCellValue(row, 3, wxString::Format("%d", zone.tilt_min));
    Grid_Zones->SetCellValue(row, 4, wxString::Format("%d", zone.tilt_max));
    Grid_Zones->SetCellValue(row, 5, wxString::Format("%d", zone.channel));
    Grid_Zones->SetCellValue(row, 6, wxString::Format("%d", zone.value));
}

wxString PositionZoneDialog::ZoneDisplayName(const PositionZone& zone, size_t index)
{
    if (!zone.label.empty()) {
        return wxString::Format("%s (Ch %d = %d)", wxString(zone.label), zone.channel, zone.value);
    }
    return wxString::Format("Zone %d (Ch %d = %d)", (int)index + 1, zone.channel, zone.value);
}

void PositionZoneDialog::OnButton_AddZoneClick(wxCommandEvent& event)
{
    PositionZone zone;
    AppendZoneRow(zone);
    _zones.push_back(zone);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

void PositionZoneDialog::OnButton_ExportZonesClick(wxCommandEvent& event)
{
    spdlog::debug("PositionZoneDialog::OnButton_ExportZonesClick - {} zone(s) defined on '{}'", _zones.size(), _model.GetName());

    if (_zones.empty()) {
        wxMessageBox("There are no zones to export.", "Export Zones", wxOK | wxICON_INFORMATION, this);
        return;
    }

    wxArrayString zoneItems;
    for (size_t i = 0; i < _zones.size(); ++i) {
        zoneItems.Add(ZoneDisplayName(_zones[i], i));
    }

    CheckboxSelectDialog zoneDlg(this, "Export Zones", zoneItems, zoneItems, "Select the zones to export:");
    if (zoneDlg.ShowModal() != wxID_OK)
        return;
    wxArrayString selectedZones = zoneDlg.GetSelectedItems();
    if (selectedZones.IsEmpty())
        return;

    std::vector<PositionZone> zonesToExport;
    for (size_t i = 0; i < _zones.size(); ++i) {
        if (selectedZones.Index(zoneItems[i]) != wxNOT_FOUND) {
            zonesToExport.push_back(_zones[i]);
        }
    }

    std::vector<DmxMovingHeadAdv*> otherHeads;
    wxArrayString propItems;
    const ModelManager& mm = _model.GetModelManager();
    for (auto it = mm.begin(); it != mm.end(); ++it) {
        Model* m = it->second;
        if (m == &_model)
            continue;
        DmxMovingHeadAdv* other = dynamic_cast<DmxMovingHeadAdv*>(m);
        if (other != nullptr) {
            otherHeads.push_back(other);
            propItems.Add(m->GetName());
        }
    }

    if (otherHeads.empty()) {
        wxMessageBox("There are no other moving head props to export zones to.", "Export Zones", wxOK | wxICON_INFORMATION, this);
        return;
    }

    CheckboxSelectDialog propDlg(this, "Export Zones", propItems, wxArrayString(), "Select the props to export the selected zones to:");
    if (propDlg.ShowModal() != wxID_OK)
        return;
    wxArrayString selectedProps = propDlg.GetSelectedItems();
    if (selectedProps.IsEmpty())
        return;

    int propCount = 0;
    for (size_t i = 0; i < otherHeads.size(); ++i) {
        if (selectedProps.Index(propItems[i]) == wxNOT_FOUND)
            continue;

        DmxMovingHeadAdv* target = otherHeads[i];
        auto targetZones = target->GetPositionZones();
        for (const auto& zone : zonesToExport) {
            targetZones.push_back(zone);
        }
        target->SetPositionZones(targetZones);
        // Notification (AddASAPWork) is deferred to the destructor -- see the comment
        // on _pendingExportNotifyTargets. Only the plain data mutation happens here.
        _pendingExportNotifyTargets.push_back(target);
        ++propCount;
    }

    spdlog::debug("PositionZoneDialog::OnButton_ExportZonesClick - exported {} zone(s) to {} prop(s), notifications deferred until dialog close",
                  (int)zonesToExport.size(), propCount);

    wxMessageBox(wxString::Format("Exported %d zone(s) to %d prop(s).", (int)zonesToExport.size(), propCount),
                 "Export Zones", wxOK | wxICON_INFORMATION, this);
}

void PositionZoneDialog::OnButton_ImportZonesClick(wxCommandEvent& event)
{
    std::vector<DmxMovingHeadAdv*> otherHeads;
    wxArrayString propItems;
    const ModelManager& mm = _model.GetModelManager();
    for (auto it = mm.begin(); it != mm.end(); ++it) {
        Model* m = it->second;
        if (m == &_model)
            continue;
        DmxMovingHeadAdv* other = dynamic_cast<DmxMovingHeadAdv*>(m);
        if (other != nullptr && !other->GetPositionZones().empty()) {
            otherHeads.push_back(other);
            propItems.Add(m->GetName());
        }
    }

    if (otherHeads.empty()) {
        wxMessageBox("There are no other moving head props with zones to import from.", "Import Zones", wxOK | wxICON_INFORMATION, this);
        return;
    }

    int propIndex = wxGetSingleChoiceIndex("Select the prop to import zones from:", "Import Zones", propItems, this);
    if (propIndex == wxNOT_FOUND)
        return;

    DmxMovingHeadAdv* source = otherHeads[propIndex];
    const std::vector<PositionZone>& sourceZones = source->GetPositionZones();

    wxArrayString zoneItems;
    for (size_t i = 0; i < sourceZones.size(); ++i) {
        zoneItems.Add(ZoneDisplayName(sourceZones[i], i));
    }

    CheckboxSelectDialog zoneDlg(this, "Import Zones", zoneItems, zoneItems, wxString::Format("Select the zones to import from '%s':", propItems[propIndex]));
    if (zoneDlg.ShowModal() != wxID_OK)
        return;
    wxArrayString selectedZones = zoneDlg.GetSelectedItems();
    if (selectedZones.IsEmpty())
        return;

    int importCount = 0;
    for (size_t i = 0; i < sourceZones.size(); ++i) {
        if (selectedZones.Index(zoneItems[i]) == wxNOT_FOUND)
            continue;
        AppendZoneRow(sourceZones[i]);
        _zones.push_back(sourceZones[i]);
        ++importCount;
    }

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    wxMessageBox(wxString::Format("Imported %d zone(s) from '%s'.", importCount, propItems[propIndex]),
                 "Import Zones", wxOK | wxICON_INFORMATION, this);
}

void PositionZoneDialog::OnButton_DeleteZoneClick(wxCommandEvent& event)
{
    int row = Grid_Zones->GetGridCursorRow();
    if (row < 0 || row >= (int)_zones.size())
        return;

    if (wxMessageBox("Are you sure you want to delete this zone?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        return;

    // The confirmation pumps the event loop, so the row captured before it can
    // no longer be in range by the time the answer comes back.
    if (row >= (int)_zones.size() || row >= Grid_Zones->GetNumberRows())
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

    if (col == 0) { // Label
        _zones[row].label = Grid_Zones->GetCellValue(row, col).ToStdString();
        return;
    }

    int val = wxAtoi(Grid_Zones->GetCellValue(row, col));
    wxString errMsg;

    switch (col) {
    case 1: // Pan Min
        if (val < 0 || val > 255)
            errMsg = "Pan Min must be 0-255.";
        else if (val > _zones[row].pan_max)
            errMsg = "Pan Min must be <= Pan Max.";
        else
            _zones[row].pan_min = val;
        break;
    case 2: // Pan Max
        if (val < 0 || val > 255)
            errMsg = "Pan Max must be 0-255.";
        else if (val < _zones[row].pan_min)
            errMsg = "Pan Max must be >= Pan Min.";
        else
            _zones[row].pan_max = val;
        break;
    case 3: // Tilt Min
        if (val < 0 || val > 255)
            errMsg = "Tilt Min must be 0-255.";
        else if (val > _zones[row].tilt_max)
            errMsg = "Tilt Min must be <= Tilt Max.";
        else
            _zones[row].tilt_min = val;
        break;
    case 4: // Tilt Max
        if (val < 0 || val > 255)
            errMsg = "Tilt Max must be 0-255.";
        else if (val < _zones[row].tilt_min)
            errMsg = "Tilt Max must be >= Tilt Min.";
        else
            _zones[row].tilt_max = val;
        break;
    case 5: // Channel
        if (val < 1)
            errMsg = "Channel must be >= 1.";
        else
            _zones[row].channel = val;
        break;
    case 6: // Value
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
        case 1: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].pan_min)); break;
        case 2: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].pan_max)); break;
        case 3: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].tilt_min)); break;
        case 4: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].tilt_max)); break;
        case 5: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].channel)); break;
        case 6: Grid_Zones->SetCellValue(row, col, wxString::Format("%d", _zones[row].value)); break;
        }
    }
}
