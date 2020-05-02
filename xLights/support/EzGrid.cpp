/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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
