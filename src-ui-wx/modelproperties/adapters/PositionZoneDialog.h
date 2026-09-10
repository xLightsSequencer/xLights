#ifndef POSITIONZONEDIALOG_H
#define POSITIONZONEDIALOG_H

//(*Headers(PositionZoneDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
//*)
#include "models/DMX/DmxMovingHeadAdv.h"

class PositionZoneDialog: public wxDialog
{
    public:

        PositionZoneDialog(DmxMovingHeadAdv& model, std::vector<PositionZone>& zones, wxWindow* parent);
        virtual ~PositionZoneDialog();

        //(*Declarations(PositionZoneDialog)
        wxButton* Button_AddZone;
        wxButton* Button_DeleteZone;
        wxFlexGridSizer* FlexGridSizer1;
        wxGrid* Grid_Zones;
        //*)

        wxButton* Button_ExportZones = nullptr;
        wxButton* Button_ImportZones = nullptr;

    protected:

        //(*Identifiers(PositionZoneDialog)
        static const wxWindowID ID_GRID_Zones;
        static const wxWindowID ID_BUTTON_AddZone;
        static const wxWindowID ID_BUTTON_DeleteZone;
        //*)

        static const wxWindowID ID_BUTTON_ExportZones;
        static const wxWindowID ID_BUTTON_ImportZones;

    private:

        //(*Handlers(PositionZoneDialog)
        void OnButton_AddZoneClick(wxCommandEvent& event);
        void OnButton_DeleteZoneClick(wxCommandEvent& event);
        void OnGrid_ZonesCellChanged(wxGridEvent& event);
        //*)

        void OnButton_ExportZonesClick(wxCommandEvent& event);
        void OnButton_ImportZonesClick(wxCommandEvent& event);

        void AppendZoneRow(const PositionZone& zone);
        static wxString ZoneDisplayName(const PositionZone& zone, size_t index);
        void FlushPendingExportNotifications();

        DmxMovingHeadAdv& _model;
        std::vector<PositionZone>& _zones;

        // Notifying other models' AddASAPWork while this dialog is still modal lets the
        // queued CallAfter(DoASAPWork) fire mid-dialog (wx dispatches CallAfter events even
        // inside a nested modal loop) and rebuild the layout/property grid out from under this
        // dialog's own parent grid, so these are collected during Export and only flushed once
        // this dialog's modal session has fully ended (see the destructor).
        std::vector<DmxMovingHeadAdv*> _pendingExportNotifyTargets;

        DECLARE_EVENT_TABLE()
};

#endif
