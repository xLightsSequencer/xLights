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

        PositionZoneDialog(std::vector<PositionZone>& zones, wxWindow* parent, wxWindowID id = wxID_ANY);
        virtual ~PositionZoneDialog();

        //(*Declarations(PositionZoneDialog)
        wxButton* Button_AddZone;
        wxButton* Button_DeleteZone;
        wxFlexGridSizer* FlexGridSizer1;
        wxGrid* Grid_Zones;
        //*)

    protected:

        //(*Identifiers(PositionZoneDialog)
        static const wxWindowID ID_GRID_Zones;
        static const wxWindowID ID_BUTTON_AddZone;
        static const wxWindowID ID_BUTTON_DeleteZone;
        //*)

    private:

        //(*Handlers(PositionZoneDialog)
        void OnButton_AddZoneClick(wxCommandEvent& event);
        void OnButton_DeleteZoneClick(wxCommandEvent& event);
        void OnGrid_ZonesCellChanged(wxGridEvent& event);
        //*)

        std::vector<PositionZone>& _zones;

        DECLARE_EVENT_TABLE()
};

#endif
