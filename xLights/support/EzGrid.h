#ifndef _EZGRID_H
#define _EZGRID_H

#include "wx/grid.h"

class EzGrid : public wxGrid
{
public:
    EzGrid( wxWindow *parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxWANTS_CHARS,
        const wxString& name = wxPanelNameStr ) :
            wxGrid(parent, id, pos, size, style, name ) {}
private:
    DECLARE_EVENT_TABLE()
    void OnCellLeftClick( wxGridEvent& );
};


#endif // _EZGRID_H
