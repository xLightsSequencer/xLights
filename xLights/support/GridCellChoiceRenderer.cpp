/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "GridCellChoiceRenderer.h"
#include <wx/renderer.h>

void wxGridCellChoiceRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
	const wxRect& rectCell, int row, int col, bool isSelected)
{
    //wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);
    wxRendererNative::Get().DrawComboBox(&grid, dc, rectCell, wxCONTROL_CURRENT);

	// first calculate button size
	// don't draw outside the cell
	int nButtonWidth = 17;
	if (rectCell.height < 2) return;
	wxRect rectButton;
	rectButton.x = rectCell.x + rectCell.width - nButtonWidth;
	rectButton.y = rectCell.y + 1;
	int cell_rows, cell_cols;
	attr.GetSize(&cell_rows, &cell_cols);
	rectButton.width = nButtonWidth;
	if (cell_rows == 1)
		rectButton.height = rectCell.height-2;
	else
		rectButton.height = nButtonWidth;

	SetTextColoursAndFont(grid, attr, dc, isSelected);
	int hAlign, vAlign;
	attr.GetAlignment(&hAlign, &vAlign);
	// leave room for button
	wxRect rect = rectCell;
	rect.SetWidth(rectCell.GetWidth() - rectButton.GetWidth()-2);
	rect.Inflate(-1);
	grid.DrawTextRectangle(dc, grid.GetCellValue(row, col), rect, hAlign, vAlign);

	/*
	// don't bother drawing if the cell is too small
	if (rectButton.height < 4 || rectButton.width < 4) return;
	// draw 3-d button
	wxColour colourBackGround = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
	dc.SetBrush(wxBrush(colourBackGround, wxBRUSHSTYLE_SOLID));
	dc.SetPen(wxPen(colourBackGround, 1, wxPENSTYLE_SOLID));
	dc.DrawRectangle(rectButton);
	dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT), 1, wxPENSTYLE_SOLID));
	dc.DrawLine(rectButton.GetLeft(), rectButton.GetBottom(),
		rectButton.GetRight(), rectButton.GetBottom());
	dc.DrawLine(rectButton.GetRight(), rectButton.GetBottom(),
		rectButton.GetRight(), rectButton.GetTop()-1);
	dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW),
		1, wxPENSTYLE_SOLID));
	dc.DrawLine(rectButton.GetLeft()+1, rectButton.GetBottom()-1,
		rectButton.GetRight()-1, rectButton.GetBottom()-1);
	dc.DrawLine(rectButton.GetRight()-1, rectButton.GetBottom()-1,
		rectButton.GetRight()-1, rectButton.GetTop());
	dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT),
		1, wxPENSTYLE_SOLID));
	dc.DrawLine(rectButton.GetRight()-2, rectButton.GetTop()+1,
		rectButton.GetLeft()+1, rectButton.GetTop()+1);
	dc.DrawLine(rectButton.GetLeft()+1, rectButton.GetTop()+1,
		rectButton.GetLeft()+1, rectButton.GetBottom()-1);
	// Draw little triangle
	int nTriWidth = 7;
	int nTriHeight = 4;
	wxPoint point[3];
	point[0] = wxPoint(rectButton.GetLeft() + (rectButton.GetWidth()-nTriWidth)/2,
		rectButton.GetTop()+(rectButton.GetHeight()-nTriHeight)/2);
	point[1] = wxPoint(point[0].x+nTriWidth-1, point[0].y);
	point[2] = wxPoint(point[0].x+3, point[0].y+nTriHeight-1);
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT), wxBRUSHSTYLE_SOLID));
	dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT), 1, wxPENSTYLE_SOLID));
	dc.DrawPolygon(3, point);
	if (m_border == wxLAYOUT_TOP)
	{
		dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_DOT));
		dc.DrawLine(rectCell.GetRight(), rectCell.GetTop(),
			rectCell.GetLeft(), rectCell.GetTop());
	}*/
}
