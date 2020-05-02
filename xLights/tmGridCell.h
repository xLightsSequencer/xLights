#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>
#include <wx/event.h>

wxDECLARE_EVENT(EVT_DELETE_ROW, wxCommandEvent);
wxDECLARE_EVENT(EVT_NAME_CHANGE, wxCommandEvent);

class wxGridCellButtonRenderer : public wxGridCellRenderer
{
public:
   wxGridCellButtonRenderer(wxString label);
   virtual ~wxGridCellButtonRenderer(void);

    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected);
    virtual wxSize GetBestSize(wxGrid &grid, wxGridCellAttr& attr, wxDC &dc, int row, int col);
    virtual wxGridCellRenderer *Clone() const;
private:
    wxString m_strLabel;
};


class wxGridCellButtonEditor : public wxEvtHandler, public wxGridCellEditor
{
public:
    wxGridCellButtonEditor(wxString label);
   virtual ~wxGridCellButtonEditor(void);

    virtual void Create(wxWindow *parent, wxWindowID id, wxEvtHandler* pEvtHandler);
   void OnButton(wxCommandEvent &evt);

    virtual void SetSize(const wxRect &rect);
    virtual void BeginEdit(int row, int col, wxGrid *pGrid);
    virtual bool EndEdit(int row, int col, const wxGrid *grid,
                         const wxString& oldval, wxString *newval);
    virtual void ApplyEdit(int row, int col, wxGrid* grid);
    virtual void Reset();
   virtual wxString GetValue() const;
    virtual wxGridCellEditor *Clone() const;

private:
    wxString m_strLabel;
   wxButton *m_pButton;

    DECLARE_NO_COPY_CLASS(wxGridCellButtonEditor)
};

class tmGrid : public wxGrid
{
public:
   tmGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
      long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
   virtual ~tmGrid();

   void OnLeftClick(wxGridEvent &evt);
    void OnCellChanged(wxGridEvent &evt);

   static const long ID_GRID_RENDERER;

   DECLARE_EVENT_TABLE()
};
