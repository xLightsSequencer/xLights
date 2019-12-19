#include "NodesGridCellEditor.h"
#include <wx/tokenzr.h>

wxGridCellEditor *NodesGridCellEditor::Clone() const
{
    NodesGridCellEditor *editor = new NodesGridCellEditor;
    editor->names = names;
    return editor;
}
void NodesGridCellEditor::SetSize(const wxRect& rect) {
    // Check that the height is not too small to fit the combobox.
    wxRect rectTallEnough = rect;

    rectTallEnough.y += rectTallEnough.height; //put list below cell so it looks like a combo box and user can see current selection(s)
    rectTallEnough.height *= 5; //std::min(5, (int)m_choices.GetCount()); //rect was set to cell size so x, y, and width were correct; just adjust height to show listbox
    rectTallEnough.height += 4; //FUD; maybe borders?
    rectTallEnough.width += 2; //kludge: not quite wide enough, due to border?

    wxRect txtrect = rect;
    //TODO: spacing needs a little tweaking yet
    ++txtrect.x;
    ++txtrect.y; //FUD
    --txtrect.height;
    --txtrect.width;
    //    txtrect.height -= 2;
    m_text->SetSize(txtrect);
    wxGridCellEditor::SetSize(rectTallEnough);
}

void NodesGridCellEditor::Create(wxWindow* parent,
    wxWindowID id,
    wxEvtHandler* evtHandler)
{
    int style = wxBORDER_DEFAULT;
    style |= wxLB_EXTENDED; //wxLB_MULTIPLE;

    m_control = new wxListBox(parent, id,
        wxDefaultPosition, wxDefaultSize,
        names,
        style);
    m_text = new wxStaticText(parent, wxID_ANY /*wxNewId()*/, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT | wxST_NO_AUTORESIZE);
    m_text->SetForegroundColour(*wxBLUE); //make it easier to see which cell will be changing
    wxColor ltblue(200, 200, 255);
    m_text->SetBackgroundColour(ltblue);

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void NodesGridCellEditor::SelectionChanged(wxCommandEvent& event)
{
    m_text->SetLabel(GetValue()); //show intermediate results
}

void NodesGridCellEditor::EditDone(wxCommandEvent& event)
{
    if (m_grid) m_grid->DisableCellEditControl();
}

void NodesGridCellEditor::EditDone2(wxMouseEvent& event)
{
    if (m_grid) m_grid->DisableCellEditControl();
}


void NodesGridCellEditor::BeginEdit(int row, int col, wxGrid* grid) {

#ifdef __linux__
    // wxGridCellEditorEvtHandler is undefined on Linux, need a workaround
    return;
#endif

    wxGridCellEditorEvtHandler* evtHandler = NULL;
    if (m_control)
        evtHandler = (wxGridCellEditorEvtHandler*)m_control->GetEventHandler();

    // Don't immediately end if we get a kill focus event within BeginEdit
    if (evtHandler) {
        evtHandler->SetInSetFocus(true);
        evtHandler->Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &NodesGridCellEditor::SelectionChanged, this);
        evtHandler->Bind(wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, &NodesGridCellEditor::EditDone, this);
        m_grid = grid; //allow custom events to access grid
    }

    //no worky; use grid event handler instead
    wxEvtHandler* evtHandler2 = NULL;
    if (m_text) evtHandler2 = m_text->GetEventHandler();
    if (evtHandler2) {
        evtHandler2->Bind(wxEVT_LEFT_DOWN, &NodesGridCellEditor::EditDone2, this);
    }

    m_value = grid->GetTable()->GetValue(row, col);

    Reset(); // this updates combo box to correspond to m_value
    m_text->SetLabel(GetValue()); //show intermediate results
    m_text->Show();

    ListBox()->SetFocus();


    if (evtHandler) {
        // When dropping down the menu, a kill focus event
        // happens after this point, so we can't reset the flag yet.
        evtHandler->SetInSetFocus(false);
    }
}

#define notWXUNUSED(thing)  thing
bool NodesGridCellEditor::EndEdit(int notWXUNUSED(row),
    int notWXUNUSED(col),
    const wxGrid* notWXUNUSED(grid),
    const wxString& notWXUNUSED(oldval),
    wxString *newval)
{
    m_text->Hide();
    const wxString value = GetValue();
    if (value == m_value)
        return false;

    m_value = value;
    if (newval)
        *newval = value;
    return true;
}

void NodesGridCellEditor::ApplyEdit(int row, int col, wxGrid* grid) {
    grid->GetTable()->SetValue(row, col, m_value);
}

void NodesGridCellEditor::Reset()
{
    ListBox()->SetSelection(wxNOT_FOUND);

    wxStringTokenizer wtkz(m_value, ",");
    while (wtkz.HasMoreTokens()) //single iteration for model name, maybe multiple for node#s
    {
        wxString valstr = wtkz.GetNextToken();
        for (size_t i = 0; i < ListBox()->GetCount(); ++i) {
            if (ListBox()->GetString(i) == valstr) {
                ListBox()->SetSelection(i);
            }
        }
    }
}

// return the value in the text control
wxString NodesGridCellEditor::GetValue() const
{
    wxString retval;
    for (int i = 0; i < ListBox()->GetCount(); ++i) {
        if (!ListBox()->IsSelected(i)) continue;
        if (!retval.empty()) retval += wxT(",");
        retval += ListBox()->GetString(i);
    }
    return retval;
}