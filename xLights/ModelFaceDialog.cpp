#include "ModelFaceDialog.h"

//(*InternalHeaders(ModelFaceDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(ModelFaceDialog)
const long ModelFaceDialog::ID_PANEL1 = wxNewId();
const long ModelFaceDialog::ID_GRID_COROFACES = wxNewId();
const long ModelFaceDialog::ID_PANEL2 = wxNewId();
const long ModelFaceDialog::ID_STATICTEXT1 = wxNewId();
const long ModelFaceDialog::ID_PANEL3 = wxNewId();
const long ModelFaceDialog::ID_CHOICEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelFaceDialog,wxDialog)
	//(*EventTable(ModelFaceDialog)
	//*)
END_EVENT_TABLE()


#include "ModelClass.h"

ModelFaceDialog::ModelFaceDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelFaceDialog)
	wxPanel* None;
	wxPanel* Coro;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Face Definition"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FaceTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	None = new wxPanel(FaceTypeChoice, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	Coro = new wxPanel(FaceTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	GridCoroFaces = new wxGrid(Coro, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
	GridCoroFaces->CreateGrid(13,1);
	GridCoroFaces->EnableEditing(true);
	GridCoroFaces->EnableGridLines(true);
	GridCoroFaces->SetColLabelSize(20);
	GridCoroFaces->SetRowLabelSize(100);
	GridCoroFaces->SetDefaultColSize(180, true);
	GridCoroFaces->SetColLabelValue(0, _("Nodes"));
	GridCoroFaces->SetRowLabelValue(0, _("Face Outline"));
	GridCoroFaces->SetRowLabelValue(1, _("Mouth - AI"));
	GridCoroFaces->SetRowLabelValue(2, _("Mouth - E"));
	GridCoroFaces->SetRowLabelValue(3, _("Mouth - etc"));
	GridCoroFaces->SetRowLabelValue(4, _("Mouth - FV"));
	GridCoroFaces->SetRowLabelValue(5, _("Mouth - L"));
	GridCoroFaces->SetRowLabelValue(6, _("Mouth - MBP"));
	GridCoroFaces->SetRowLabelValue(7, _("Mouth - O"));
	GridCoroFaces->SetRowLabelValue(8, _("Mouth - rest"));
	GridCoroFaces->SetRowLabelValue(9, _("Mouth - U"));
	GridCoroFaces->SetRowLabelValue(10, _("Mouth - WQ"));
	GridCoroFaces->SetRowLabelValue(11, _("Eyes - Open"));
	GridCoroFaces->SetRowLabelValue(12, _("Eyes - Closed"));
	GridCoroFaces->SetDefaultCellFont( GridCoroFaces->GetFont() );
	GridCoroFaces->SetDefaultCellTextColour( GridCoroFaces->GetForegroundColour() );
	FlexGridSizer2->Add(GridCoroFaces, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Coro->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Coro);
	FlexGridSizer2->SetSizeHints(Coro);
	Matrix = new wxPanel(FaceTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	StaticText1 = new wxStaticText(Matrix, ID_STATICTEXT1, _("To Be Determined...."), wxPoint(40,32), wxSize(104,32), 0, _T("ID_STATICTEXT1"));
	FaceTypeChoice->AddPage(None, _("None"), false);
	FaceTypeChoice->AddPage(Coro, _("Coro Face"), false);
	FaceTypeChoice->AddPage(Matrix, _("Matrix"), false);
	FlexGridSizer1->Add(FaceTypeChoice, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

ModelFaceDialog::~ModelFaceDialog()
{
	//(*Destroy(ModelFaceDialog)
	//*)
}


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

    wxGridCellEditorEvtHandler* evtHandler = NULL;
    if (m_control)
        evtHandler = wxDynamicCast(m_control->GetEventHandler(), wxGridCellEditorEvtHandler);
    
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
    if ( value == m_value )
        return false;
    
    m_value = value;
    if ( newval )
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
        for (int i = 0; i < ListBox()->GetCount(); ++i) {
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




void ModelFaceDialog::SetFaceInfo(ModelClass *cls, std::map<wxString, wxString> &info) {
    
    if (info["Type"] == "Coro") {
        FaceTypeChoice->SetSelection(1);
        for (int x = 0; x < GridCoroFaces->GetNumberRows(); x++) {
            wxString name = GridCoroFaces->GetRowLabelValue(x);
            name.Replace(" ", "");
            GridCoroFaces->SetCellValue(x, 0, info[name]);
        }
    } else if (info["Type"] == "Matrix") {
        FaceTypeChoice->SetSelection(2);
    } else {
        FaceTypeChoice->SetSelection(0);
    }
    
    wxArrayString names;
    names.push_back("");
    for (int x = 0; x < cls->GetNodeCount(); x++) {
        wxString nn = cls->GetNodeName(x);
        if (nn == "") {
            nn = wxString::Format("Node %d", x);
        }
        names.push_back(nn);
    }

    for (int x = 0; x < GridCoroFaces->GetNumberRows(); x++) {
        NodesGridCellEditor *editor = new NodesGridCellEditor();
        editor->names = names;
        GridCoroFaces->SetCellEditor(x, 0, editor);
    }
}
void ModelFaceDialog::GetFaceInfo(std::map<wxString, wxString> &info) {
    info.clear();
    if (FaceTypeChoice->GetSelection() == 1) {
        //Coro style face
        info["Type"] = "Coro";
        for (int x = 0; x < GridCoroFaces->GetNumberRows(); x++) {
            wxString name = GridCoroFaces->GetRowLabelValue(x);
            name.Replace(" ", "");
            info[name] = GridCoroFaces->GetCellValue(x, 0);
        }
    } else if (FaceTypeChoice->GetSelection() == 2) {
        //matrix style
        info["Type"] = "Matrix";
    }
}

