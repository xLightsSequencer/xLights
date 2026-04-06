#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/grid.h"
#include "wx/laywin.h"
#include <wx/dc.h>

class xlGridCellChoiceRenderer : public wxGridCellStringRenderer
{
public:
	xlGridCellChoiceRenderer(wxLayoutAlignment border = wxLAYOUT_NONE) :
		m_border(border) {}
	virtual void Draw(wxGrid& grid,
		wxGridCellAttr& attr,
		wxDC& dc,
		const wxRect& rect,
		int row, int col,
		bool isSelected);
	virtual wxGridCellRenderer *Clone() const
		{ return new xlGridCellChoiceRenderer; }
private:
	wxLayoutAlignment m_border;
};
