#ifndef NODESGRIDCELLEDITOR_H
#define NODESGRIDCELLEDITOR_H

#include <wx/wx.h>
#include <wx/grid.h>

class NodesGridCellEditor : public wxGridCellEditor {
public:
    NodesGridCellEditor() : wxGridCellEditor() {

    }
    virtual ~NodesGridCellEditor() {

    }

    wxArrayString names;


    virtual void Create(wxWindow* parent,
        wxWindowID id,
        wxEvtHandler* evtHandler);


    virtual void BeginEdit(int row, int col, wxGrid* grid);
    virtual bool EndEdit(int row, int col, const wxGrid* grid,
        const wxString& oldval, wxString *newval);
    virtual void ApplyEdit(int row, int col, wxGrid* grid);

    virtual void Reset();
    virtual wxGridCellEditor *Clone() const;
    virtual wxString GetValue() const;

    void SelectionChanged(wxCommandEvent& event);
    void EditDone(wxCommandEvent& event);
    void EditDone2(wxMouseEvent& event);

    virtual void SetSize(const wxRect& rect);

protected:
    wxListBox *ListBox() const { return (wxListBox *)m_control; }
    wxStaticText* m_text;
    wxGrid* m_grid;

    wxString        m_value;

    wxDECLARE_NO_COPY_CLASS(NodesGridCellEditor);
};

#endif
