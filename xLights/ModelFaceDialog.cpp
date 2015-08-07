#include "ModelFaceDialog.h"

//(*InternalHeaders(ModelFaceDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/filefn.h>
#include <wx/filename.h>

//(*IdInit(ModelFaceDialog)
const long ModelFaceDialog::ID_PANEL1 = wxNewId();
const long ModelFaceDialog::ID_GRID_COROFACES = wxNewId();
const long ModelFaceDialog::ID_PANEL2 = wxNewId();
const long ModelFaceDialog::ID_GRID3 = wxNewId();
const long ModelFaceDialog::ID_PANEL6 = wxNewId();
const long ModelFaceDialog::ID_STATICTEXT1 = wxNewId();
const long ModelFaceDialog::ID_CHOICE1 = wxNewId();
const long ModelFaceDialog::ID_BUTTON1 = wxNewId();
const long ModelFaceDialog::ID_BUTTON2 = wxNewId();
const long ModelFaceDialog::ID_CHOICE2 = wxNewId();
const long ModelFaceDialog::ID_GRID1 = wxNewId();
const long ModelFaceDialog::ID_PANEL3 = wxNewId();
const long ModelFaceDialog::ID_CHOICEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelFaceDialog,wxDialog)
	//(*EventTable(ModelFaceDialog)
	//*)
END_EVENT_TABLE()


enum {
    NO_FACE = 0,
    SINGLE_NODE_FACE,
    NODE_RANGE_FACE,
    MATRIX_FACE
};


#include "ModelClass.h"

ModelFaceDialog::ModelFaceDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelFaceDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxPanel* None;
	wxPanel* CoroPanel;
	wxStaticText* StaticText2;
	wxButton* Button1;
	wxFlexGridSizer* FlexGridSizer3;
	wxPanel* NodeRangePanel;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer6;
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
	CoroPanel = new wxPanel(FaceTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
	SingleNodeGrid->CreateGrid(13,1);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel,wxSize(-1,200)));
	SingleNodeGrid->EnableEditing(true);
	SingleNodeGrid->EnableGridLines(true);
	SingleNodeGrid->SetColLabelSize(20);
	SingleNodeGrid->SetRowLabelSize(100);
	SingleNodeGrid->SetDefaultColSize(200, true);
	SingleNodeGrid->SetColLabelValue(0, _("Nodes"));
	SingleNodeGrid->SetRowLabelValue(0, _("Face Outline"));
	SingleNodeGrid->SetRowLabelValue(1, _("Mouth - AI"));
	SingleNodeGrid->SetRowLabelValue(2, _("Mouth - E"));
	SingleNodeGrid->SetRowLabelValue(3, _("Mouth - etc"));
	SingleNodeGrid->SetRowLabelValue(4, _("Mouth - FV"));
	SingleNodeGrid->SetRowLabelValue(5, _("Mouth - L"));
	SingleNodeGrid->SetRowLabelValue(6, _("Mouth - MBP"));
	SingleNodeGrid->SetRowLabelValue(7, _("Mouth - O"));
	SingleNodeGrid->SetRowLabelValue(8, _("Mouth - rest"));
	SingleNodeGrid->SetRowLabelValue(9, _("Mouth - U"));
	SingleNodeGrid->SetRowLabelValue(10, _("Mouth - WQ"));
	SingleNodeGrid->SetRowLabelValue(11, _("Eyes - Open"));
	SingleNodeGrid->SetRowLabelValue(12, _("Eyes - Closed"));
	SingleNodeGrid->SetDefaultCellFont( SingleNodeGrid->GetFont() );
	SingleNodeGrid->SetDefaultCellTextColour( SingleNodeGrid->GetForegroundColour() );
	FlexGridSizer2->Add(SingleNodeGrid, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CoroPanel->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(CoroPanel);
	FlexGridSizer2->SetSizeHints(CoroPanel);
	NodeRangePanel = new wxPanel(FaceTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
	NodeRangeGrid->CreateGrid(13,1);
	NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel,wxSize(-1,200)));
	NodeRangeGrid->EnableEditing(true);
	NodeRangeGrid->EnableGridLines(true);
	NodeRangeGrid->SetColLabelSize(20);
	NodeRangeGrid->SetRowLabelSize(100);
	NodeRangeGrid->SetDefaultColSize(200, true);
	NodeRangeGrid->SetColLabelValue(0, _("Nodes"));
	NodeRangeGrid->SetRowLabelValue(0, _("Face Outline"));
	NodeRangeGrid->SetRowLabelValue(1, _("Mouth - AI"));
	NodeRangeGrid->SetRowLabelValue(2, _("Mouth - E"));
	NodeRangeGrid->SetRowLabelValue(3, _("Mouth - etc"));
	NodeRangeGrid->SetRowLabelValue(4, _("Mouth - FV"));
	NodeRangeGrid->SetRowLabelValue(5, _("Mouth - L"));
	NodeRangeGrid->SetRowLabelValue(6, _("Mouth - MBP"));
	NodeRangeGrid->SetRowLabelValue(7, _("Mouth - O"));
	NodeRangeGrid->SetRowLabelValue(8, _("Mouth - rest"));
	NodeRangeGrid->SetRowLabelValue(9, _("Mouth - U"));
	NodeRangeGrid->SetRowLabelValue(10, _("Mouth - WQ"));
	NodeRangeGrid->SetRowLabelValue(11, _("Eyes - Open"));
	NodeRangeGrid->SetRowLabelValue(12, _("Eyes - Closed"));
	NodeRangeGrid->SetDefaultCellFont( NodeRangeGrid->GetFont() );
	NodeRangeGrid->SetDefaultCellTextColour( NodeRangeGrid->GetForegroundColour() );
	FlexGridSizer5->Add(NodeRangeGrid, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NodeRangePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(NodeRangePanel);
	FlexGridSizer5->SetSizeHints(NodeRangePanel);
	Matrix = new wxPanel(FaceTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText1 = new wxStaticText(Matrix, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixNameChoice = new wxChoice(Matrix, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer4->Add(MatrixNameChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button1 = new wxButton(Matrix, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixDeleteButton = new wxButton(Matrix, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(MatrixDeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(Matrix, wxID_ANY, _("Image Placement:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixImagePlacementChoice = new wxChoice(Matrix, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	MatrixImagePlacementChoice->SetSelection( MatrixImagePlacementChoice->Append(_("Centered")) );
	MatrixImagePlacementChoice->Append(_("Scaled"));
	FlexGridSizer6->Add(MatrixImagePlacementChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixModelsGrid = new wxGrid(Matrix, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	MatrixModelsGrid->CreateGrid(10,2);
	MatrixModelsGrid->SetMinSize(wxDLG_UNIT(Matrix,wxSize(-1,200)));
	MatrixModelsGrid->EnableEditing(true);
	MatrixModelsGrid->EnableGridLines(true);
	MatrixModelsGrid->SetColLabelSize(20);
	MatrixModelsGrid->SetRowLabelSize(100);
	MatrixModelsGrid->SetDefaultColSize(200, true);
	MatrixModelsGrid->SetColLabelValue(0, _("Eyes Open"));
	MatrixModelsGrid->SetColLabelValue(1, _("Eyes Closed"));
	MatrixModelsGrid->SetRowLabelValue(0, _("Mouth - AI"));
	MatrixModelsGrid->SetRowLabelValue(1, _("Mouth - E"));
	MatrixModelsGrid->SetRowLabelValue(2, _("Mouth - etc"));
	MatrixModelsGrid->SetRowLabelValue(3, _("Mouth - FV"));
	MatrixModelsGrid->SetRowLabelValue(4, _("Mouth - L"));
	MatrixModelsGrid->SetRowLabelValue(5, _("Mouth - MBP"));
	MatrixModelsGrid->SetRowLabelValue(6, _("Mouth - O"));
	MatrixModelsGrid->SetRowLabelValue(7, _("Mouth - rest"));
	MatrixModelsGrid->SetRowLabelValue(8, _("Mouth - U"));
	MatrixModelsGrid->SetRowLabelValue(9, _("Mouth - WQ"));
	MatrixModelsGrid->SetDefaultCellFont( MatrixModelsGrid->GetFont() );
	MatrixModelsGrid->SetDefaultCellTextColour( MatrixModelsGrid->GetForegroundColour() );
	FlexGridSizer3->Add(MatrixModelsGrid, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Matrix->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Matrix);
	FlexGridSizer3->SetSizeHints(Matrix);
	FaceTypeChoice->AddPage(None, _("None"), false);
	FaceTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	FaceTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
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

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixDeleteClick);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatricImagePlacementChoiceSelect);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellChange);
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

void ModelFaceDialog::SetFaceInfo(ModelClass *cls, std::map< wxString, std::map<wxString, wxString> > &finfo) {
    FaceTypeChoice->SetSelection(NO_FACE);
    for (std::map< wxString, std::map<wxString, wxString> >::iterator it = finfo.begin();
         it != finfo.end(); it++) {

        wxString type = it->first;
        std::map<wxString, wxString> &info = it->second;
        if (type == "Coro" || type == "SingleNode") {
            FaceTypeChoice->SetSelection(SINGLE_NODE_FACE);
            for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
                wxString name = SingleNodeGrid->GetRowLabelValue(x);
                name.Replace(" ", "");
                SingleNodeGrid->SetCellValue(x, 0, info[name]);
            }
        } else if (type == "NodeRange") {
            FaceTypeChoice->SetSelection(NODE_RANGE_FACE);
            for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
                wxString name = NodeRangeGrid->GetRowLabelValue(x);
                name.Replace(" ", "");
                NodeRangeGrid->SetCellValue(x, 0, info[name]);
            }
        } else {
            FaceTypeChoice->SetSelection(MATRIX_FACE);
            matrixData[type] = info;
            MatrixNameChoice->Append(type);
        }
    }

    wxArrayString names;
    names.push_back("");
    for (int x = 0; x < cls->GetNodeCount(); x++) {
        wxString nn = cls->GetNodeName(x, true);
        names.push_back(nn);
    }

    NodesGridCellEditor *editor = new NodesGridCellEditor();
    editor->names = names;
    SingleNodeGrid->SetDefaultEditor(editor);

    wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    wxString filter("0123456789,-");
    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    validator.SetCharIncludes(filter);
    reditor->SetValidator(validator);
    NodeRangeGrid->SetDefaultEditor(reditor);

    if (matrixData.empty()) {
        MatrixNameChoice->Disable();
        MatrixModelsGrid->Disable();
        MatrixDeleteButton->Disable();
        MatrixImagePlacementChoice->Disable();
    } else {
        MatrixNameChoice->SetSelection(0);
        FillMatrix(MatrixNameChoice->GetString(MatrixNameChoice->GetSelection()));
    }
}
void ModelFaceDialog::GetFaceInfo(std::map< wxString, std::map<wxString, wxString> > &finfo) {
    finfo.clear();
    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
        //Coro style face
        std::map<wxString, wxString> &info = finfo["SingleNode"];
        for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
            wxString name = SingleNodeGrid->GetRowLabelValue(x);
            name.Replace(" ", "");
            info[name] = SingleNodeGrid->GetCellValue(x, 0);
        }
    } else if (FaceTypeChoice->GetSelection() == NODE_RANGE_FACE) {
        //Coro style face using pixels
        std::map<wxString, wxString> &info = finfo["NodeRange"];
        for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
            wxString name = NodeRangeGrid->GetRowLabelValue(x);
            name.Replace(" ", "");
            info[name] = NodeRangeGrid->GetCellValue(x, 0);
        }
    } else if (FaceTypeChoice->GetSelection() == MATRIX_FACE) {
        //matrix style
        for (std::map<wxString, std::map<wxString, wxString> >::iterator it = matrixData.begin();
             it != matrixData.end(); it++) {
            if (!it->second.empty()) {
                finfo[it->first] = it->second;
            }
        }
    }
}
void ModelFaceDialog::FillMatrix(const wxString &name) {
    for (int r = 0; r < MatrixModelsGrid->GetNumberRows(); r++) {
        for (int c = 0; c < MatrixModelsGrid->GetNumberCols(); c++) {
            wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
            key.Replace(" ", "");
            MatrixModelsGrid->SetCellValue(r, c, matrixData[name][key]);
        }
    }
    wxString w = matrixData[name]["ImagePlacement"];
    if (w == "") {
        w = "Centered";
    }
    MatrixImagePlacementChoice->SetStringSelection(w);
}


void ModelFaceDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    FillMatrix(MatrixNameChoice->GetString(MatrixNameChoice->GetSelection()));
}

void ModelFaceDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New Face", "Enter name for new face definition");
    if (dlg.ShowModal() == wxID_OK) {
        wxString n = dlg.GetValue();
        if (MatrixNameChoice->FindString(n) == wxNOT_FOUND) {
            MatrixNameChoice->Append(n);
            MatrixNameChoice->SetStringSelection(n);
            FillMatrix(n);
            MatrixNameChoice->Enable();
            MatrixModelsGrid->Enable();
            MatrixDeleteButton->Enable();
            MatrixImagePlacementChoice->Enable();
        }
    }
}
void ModelFaceDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    wxString name = MatrixNameChoice->GetString(MatrixNameChoice->GetSelection());
    if (wxMessageBox("Delete face definion?", "Are you sure you want to delete " + name + "?",
                     wxICON_WARNING | wxOK , this) == wxID_OK) {

        matrixData[name].clear();
        FillMatrix(name);
        MatrixNameChoice->Delete(MatrixNameChoice->GetSelection());
        if (MatrixNameChoice->GetCount() > 0) {
            MatrixNameChoice->SetSelection(0);
            FillMatrix(MatrixNameChoice->GetString(0));
        } else {
            MatrixNameChoice->SetSelection(wxNOT_FOUND);
            MatrixNameChoice->Disable();
            MatrixDeleteButton->Disable();
            MatrixModelsGrid->Disable();
            MatrixImagePlacementChoice->Disable();
        }
    }
}

void ModelFaceDialog::OnMatrixModelsGridCellChange(wxGridEvent& event)
{
    wxString name = MatrixNameChoice->GetString(MatrixNameChoice->GetSelection());
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    matrixData[name][key] = MatrixModelsGrid->GetCellValue(r, c);
}


static const wxString strSupportedImageTypes = "PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPG files(*.jpg)|*.jpg|All files (*.*)|*.*";
void ModelFaceDialog::OnMatrixModelsGridCellLeftClick(wxGridEvent& event)
{
    wxString name = MatrixNameChoice->GetString(MatrixNameChoice->GetSelection());
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    wxFileName fname(MatrixModelsGrid->GetCellValue(r, c));
    wxFileDialog dlg(this, "Choose Image File for " + key, fname.GetPath(),
                     wxEmptyString,
                     strSupportedImageTypes,
                     wxFD_OPEN);
    if (dlg.ShowModal() == wxID_OK) {
        wxString new_filename = dlg.GetPath();
        key.Replace(" ", "");
        matrixData[name][key] = new_filename;
        MatrixModelsGrid->SetCellValue(r, c, new_filename);
    }
}


void ModelFaceDialog::OnMatricImagePlacementChoiceSelect(wxCommandEvent& event)
{
    wxString name = MatrixNameChoice->GetString(MatrixNameChoice->GetSelection());
    matrixData[name]["ImagePlacement"] = MatrixNameChoice->GetString(MatrixImagePlacementChoice->GetSelection());
}
