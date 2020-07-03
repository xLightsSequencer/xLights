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

#include "wx/grid.h"
#include <wx/combobox.h>

// the editor for string data allowing to choose from the list of strings
class wxFastComboEditor : public wxGridCellEditor
{
public:
    // if !allowOthers, user can't type a string not in choices array
    wxFastComboEditor(size_t count = 0,
                           const wxString choices[] = NULL,
                           bool allowOthers = false);
    wxFastComboEditor(const wxArrayString& choices,
                           bool allowOthers = false);

    virtual void Create(wxWindow* parent,
                        wxWindowID id,
                        wxEvtHandler* evtHandler) wxOVERRIDE;

    virtual void SetSize(const wxRect& rect) wxOVERRIDE;

    virtual void PaintBackground(wxDC& dc,
                                 const wxRect& rectCell,
                                 const wxGridCellAttr& attr) wxOVERRIDE;

    virtual void BeginEdit(int row, int col, wxGrid* grid) wxOVERRIDE;
    virtual bool EndEdit(int row, int col, const wxGrid* grid,
                         const wxString& oldval, wxString *newval) wxOVERRIDE;
    virtual void ApplyEdit(int row, int col, wxGrid* grid) wxOVERRIDE;

    virtual void Reset() wxOVERRIDE;

    // parameters string format is "item1[,item2[...,itemN]]"
    virtual void SetParameters(const wxString& params) wxOVERRIDE;

    virtual wxGridCellEditor *Clone() const wxOVERRIDE;

    // added GetValue so we can get the value which is in the control
    virtual wxString GetValue() const wxOVERRIDE;

protected:
    wxComboBox *Combo() const { return (wxComboBox *)m_control; }
    wxPoint m_pointActivate;
    wxString        m_value;
    wxArrayString   m_choices;
    bool            m_allowOthers;

    wxDECLARE_NO_COPY_CLASS(wxFastComboEditor);
};
