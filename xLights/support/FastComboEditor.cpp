/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FastComboEditor.h"
#include <wx/tokenzr.h>
#include "EzGrid.h"
#include <wx/app.h>

// ----------------------------------------------------------------------------
// wxGridCellChoiceEditor
// ----------------------------------------------------------------------------

wxFastComboEditor::wxFastComboEditor(const wxArrayString& choices,
                                               bool allowOthers)
    : m_choices(choices),
      m_allowOthers(allowOthers) { }

wxFastComboEditor::wxFastComboEditor(size_t count,
                                               const wxString choices[],
                                               bool allowOthers)
                      : m_allowOthers(allowOthers)
{
    SetClientData((void*)&m_pointActivate);

    if ( count )
    {
        m_choices.Alloc(count);
        for ( size_t n = 0; n < count; n++ )
        {
            m_choices.Add(choices[n]);
        }
    }
}

wxGridCellEditor *wxFastComboEditor::Clone() const
{
    wxFastComboEditor *editor = new wxFastComboEditor;
    editor->m_allowOthers = m_allowOthers;
    editor->m_choices = m_choices;

    return editor;
}

void wxFastComboEditor::Create(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler)
{
    int style = wxTE_PROCESS_ENTER |
                wxTE_PROCESS_TAB |
                wxBORDER_NONE;

    if ( !m_allowOthers )
        style |= wxCB_READONLY;

    //*******************  new code ***********************
    size_t count = m_choices.GetCount();
    wxString *choices = new wxString[count];
    for ( size_t n = 0; n < count; n++ )
    {
        choices[n] = m_choices[n];
    }
    m_control = new wxComboBox(parent, id, wxEmptyString,
                                wxDefaultPosition, wxDefaultSize,
                                count, choices,
                                m_allowOthers ? 0 : wxCB_READONLY);
/*    m_control = new wxSComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               m_choices,
                               style);*/
    //*******************  new code ***********************

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxFastComboEditor::SetSize(const wxRect& rect)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxFastComboEditor must be created first!"));

    // Check that the height is not too small to fit the combobox.
    wxRect rectTallEnough = rect;
    const wxSize bestSize = m_control->GetBestSize();
    const wxCoord diffY = bestSize.GetHeight() - rectTallEnough.GetHeight();
    if ( diffY > 0 )
    {
        // Do make it tall enough.
        rectTallEnough.height += diffY;

        // Also centre the effective rectangle vertically with respect to the
        // original one.
        rectTallEnough.y -= diffY/2;
    }
    //else: The rectangle provided is already tall enough.

    wxGridCellEditor::SetSize(rectTallEnough);
}

void wxFastComboEditor::PaintBackground(wxDC& dc,
                                             const wxRect& rectCell,
                                             const wxGridCellAttr& attr)
{
    // as we fill the entire client area, don't do anything here to minimize
    // flicker

    // TODO: It doesn't actually fill the client area since the height of a
    // combo always defaults to the standard.  Until someone has time to
    // figure out the right rectangle to paint, just do it the normal way.
    wxGridCellEditor::PaintBackground(dc, rectCell, attr);
}

void wxFastComboEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be created first!"));

    wxGridCellEditorEvtHandler* evtHandler = NULL;
    if (m_control)
        evtHandler = (wxGridCellEditorEvtHandler*)(m_control->GetEventHandler());

    // Don't immediately end if we get a kill focus event within BeginEdit
    if (evtHandler)
        evtHandler->SetInSetFocus(true);

    m_value = grid->GetTable()->GetValue(row, col);

    Reset(); // this updates combo box to correspond to m_value

    Combo()->SetFocus();

    Combo()->Popup();
/*
#ifdef __WXOSX_COCOA__
    // This is a work around for the combobox being simply dismissed when a
    // choice is made in it under OS X. The bug is almost certainly due to a
    // problem in focus events generation logic but it's not obvious to fix and
    // for now this at least allows to use wxGrid.
    Combo()->Popup();
#endif
*/
    if (evtHandler)
    {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
#if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }

}

bool wxFastComboEditor::EndEdit(int WXUNUSED(row),
                                     int WXUNUSED(col),
                                     const wxGrid* WXUNUSED(grid),
                                     const wxString& WXUNUSED(oldval),
                                     wxString *newval)
{
    const wxString value = Combo()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = value;

    return true;
}

void wxFastComboEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value);
}

void wxFastComboEditor::Reset()
{
    if (m_allowOthers)
    {
        Combo()->SetValue(m_value);
        Combo()->SetInsertionPointEnd();
    }
    else // the combobox is read-only
    {
        // find the right position, or default to the first if not found
        int pos = Combo()->FindString(m_value);
        if (pos == wxNOT_FOUND)
            pos = 0;
        Combo()->SetSelection(pos);
    }
}

void wxFastComboEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
        // what can we do?
        return;
    }

    m_choices.Empty();

    wxStringTokenizer tk(params, wxT(','));
    while ( tk.HasMoreTokens() )
    {
        m_choices.Add(tk.GetNextToken());
    }
}

// return the value in the text control
wxString wxFastComboEditor::GetValue() const
{
  return Combo()->GetValue();
}
