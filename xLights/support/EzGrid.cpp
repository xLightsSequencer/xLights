#include "EzGrid.h"

BEGIN_EVENT_TABLE(EzGrid, wxGrid)
EVT_GRID_CELL_LEFT_CLICK( EzGrid::OnCellLeftClick )
END_EVENT_TABLE()


void EzGrid::OnCellLeftClick( wxGridEvent& ev )
{
    m_waitForSlowClick = TRUE;
    SetGridCursor(ev.GetRow(), ev.GetCol());
    ev.Skip();
}
