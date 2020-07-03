/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "tmGridCell.h"

wxGridCellButtonRenderer::
wxGridCellButtonRenderer(wxString label)
{
   m_strLabel = label;
}

wxGridCellButtonRenderer::
~wxGridCellButtonRenderer(void)
{
}

const long tmGrid::ID_GRID_RENDERER = wxNewId();

void wxGridCellButtonRenderer::
Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected)
{
#ifndef __linux__
   // DrawTitleBarBitmap() is only available on MSW and OS X according to wx/renderer.h
   wxRendererNative::Get().DrawTitleBarBitmap(&grid, dc, rect, wxTITLEBAR_BUTTON_CLOSE, wxCONTROL_CURRENT);
   //wxRendererNative::Get().DrawPushButton(&grid, dc, rect, wxCONTROL_CURRENT);
#endif
   wxFont font = grid.GetFont();
   dc.SetFont(font);
   int x = rect.x + 10;
   int y = rect.y + 1;
   dc.DrawText(m_strLabel, x, y);
}

wxSize wxGridCellButtonRenderer::
GetBestSize(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, int row, int col)
{
   wxString text = grid.GetCellValue(row, col);
   dc.SetFont(attr.GetFont());
   return dc.GetTextExtent(text);
}

wxGridCellRenderer *wxGridCellButtonRenderer::
Clone() const
{
   return new wxGridCellButtonRenderer(m_strLabel);
}

////////////////////////////////////////////////////////////////////////////////////////////
wxGridCellButtonEditor::
wxGridCellButtonEditor(wxString label)
{
   m_strLabel = label;
}

wxGridCellButtonEditor::
~wxGridCellButtonEditor(void)
{
}

void wxGridCellButtonEditor::
Create(wxWindow *parent, wxWindowID id, wxEvtHandler* pEvtHandler)
{
   m_pButton = new wxButton(parent, id, m_strLabel);
   SetControl(m_pButton);
   m_pButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(wxGridCellButtonEditor::OnButton));
}

void wxGridCellButtonEditor::
OnButton(wxCommandEvent &evt)
{

   evt.Skip();
}

void wxGridCellButtonEditor::
SetSize(const wxRect &rect)
{
   m_pButton->SetSize(rect.x, rect.y, rect.width+2, rect.height+2, wxSIZE_ALLOW_MINUS_ONE);
}


void wxGridCellButtonEditor::
BeginEdit(int row, int col, wxGrid *pGrid)
{
   wxGridEvent evt(wxID_ANY, wxEVT_GRID_CELL_LEFT_CLICK, pGrid, row, col);
   wxPostEvent(m_pButton, wxCommandEvent(wxEVT_COMMAND_BUTTON_CLICKED));
}

bool wxGridCellButtonEditor::
EndEdit(int row, int col, const wxGrid *grid,
        const wxString& oldval, wxString *newval)
{
   return false;
}

void wxGridCellButtonEditor::
ApplyEdit(int row, int col, wxGrid* grid)
{
}

void wxGridCellButtonEditor::
Reset()
{
}

wxString wxGridCellButtonEditor::
GetValue() const
{
   return wxEmptyString;
}

wxGridCellEditor *wxGridCellButtonEditor::
Clone() const
{
   return new wxGridCellButtonEditor(m_strLabel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(tmGrid, wxGrid)
   EVT_GRID_CELL_LEFT_CLICK(tmGrid::OnLeftClick)
   EVT_GRID_CELL_CHANGED(tmGrid::OnCellChanged)
END_EVENT_TABLE()

tmGrid::
tmGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
      : wxGrid(parent, id, pos, size, style, name)
{
}

tmGrid::
~tmGrid()
{
}

void tmGrid::
OnLeftClick(wxGridEvent &evt)
{
    SetGridCursor(evt.GetRow(), evt.GetCol());
    //wxCommandEvent myevent(wxEVT_COMMAND_BUTTON_CLICKED, evt.GetRow());
    wxCommandEvent myevent(EVT_DELETE_ROW, evt.GetRow());
    wxPostEvent(this, myevent);
    //evt.Skip();
    //evt.StopPropagation();
}

void tmGrid::
OnCellChanged(wxGridEvent &evt)
{
    //SetGridCursor(evt.GetRow(), evt.GetCol());
    wxCommandEvent myevent(EVT_NAME_CHANGE, evt.GetRow());
    wxPostEvent(this, myevent);
}
