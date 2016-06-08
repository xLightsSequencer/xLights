#include "ModelFaceDialog.h"
#include "xLightsXmlFile.h"

//(*InternalHeaders(ModelFaceDialog)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/valtext.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/colordlg.h>
#include <wx/tokenzr.h>
#include <wx/listbox.h>

//(*IdInit(ModelFaceDialog)
const long ModelFaceDialog::ID_STATICTEXT2 = wxNewId();
const long ModelFaceDialog::ID_CHOICE3 = wxNewId();
const long ModelFaceDialog::ID_BUTTON3 = wxNewId();
const long ModelFaceDialog::ID_BUTTON4 = wxNewId();
const long ModelFaceDialog::ID_CHECKBOX1 = wxNewId();
const long ModelFaceDialog::ID_GRID_COROFACES = wxNewId();
const long ModelFaceDialog::ID_PANEL2 = wxNewId();
const long ModelFaceDialog::ID_CHECKBOX2 = wxNewId();
const long ModelFaceDialog::ID_GRID3 = wxNewId();
const long ModelFaceDialog::ID_PANEL6 = wxNewId();
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
    SINGLE_NODE_FACE = 0,
    NODE_RANGE_FACE,
    MATRIX_FACE
};


#include "models/Model.h"

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif


ModelFaceDialog::ModelFaceDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelFaceDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxButton* AddButton;
	wxPanel* CoroPanel;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer6;
	wxPanel* NodeRangePanel;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer5;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Face Definition"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NameChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	NameChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer7->Add(NameChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddButton = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer7->Add(AddButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteButton = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer7->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FaceTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	CoroPanel = new wxPanel(FaceTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CustomColorSingleNode->SetValue(false);
	FlexGridSizer2->Add(CustomColorSingleNode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
	SingleNodeGrid->CreateGrid(13,2);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel,wxSize(-1,200)));
	SingleNodeGrid->EnableEditing(true);
	SingleNodeGrid->EnableGridLines(true);
	SingleNodeGrid->SetColLabelSize(20);
	SingleNodeGrid->SetRowLabelSize(100);
	SingleNodeGrid->SetDefaultColSize(200, true);
	SingleNodeGrid->SetColLabelValue(0, _("Nodes"));
	SingleNodeGrid->SetColLabelValue(1, _("Color"));
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
	FlexGridSizer2->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 5);
	CoroPanel->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(CoroPanel);
	FlexGridSizer2->SetSizeHints(CoroPanel);
	NodeRangePanel = new wxPanel(FaceTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CustomColorNodeRanges->SetValue(false);
	FlexGridSizer5->Add(CustomColorNodeRanges, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
	NodeRangeGrid->CreateGrid(13,2);
	NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel,wxSize(-1,200)));
	NodeRangeGrid->EnableEditing(true);
	NodeRangeGrid->EnableGridLines(true);
	NodeRangeGrid->SetColLabelSize(20);
	NodeRangeGrid->SetRowLabelSize(100);
	NodeRangeGrid->SetDefaultColSize(200, true);
	NodeRangeGrid->SetColLabelValue(0, _("Nodes"));
	NodeRangeGrid->SetColLabelValue(1, _("Color"));
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
	FlexGridSizer5->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 5);
	NodeRangePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(NodeRangePanel);
	FlexGridSizer5->SetSizeHints(NodeRangePanel);
	Matrix = new wxPanel(FaceTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
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
	FlexGridSizer3->Add(MatrixModelsGrid, 1, wxALL|wxEXPAND, 5);
	Matrix->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Matrix);
	FlexGridSizer3->SetSizeHints(Matrix);
	FaceTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	FaceTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
	FaceTypeChoice->AddPage(Matrix, _("Matrix"), false);
	FlexGridSizer1->Add(FaceTypeChoice, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixDeleteClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellChange);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellChange);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatricImagePlacementChoiceSelect);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellChange);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&ModelFaceDialog::OnFaceTypeChoicePageChanged);
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

void ModelFaceDialog::SetFaceInfo(const Model *cls, std::map< std::string, std::map<std::string, std::string> > &finfo) {
    NodeRangeGrid->SetColSize(1, 50);
    SingleNodeGrid->SetColSize(1, 50);
    NameChoice->Clear();
    
    for (std::map< std::string, std::map<std::string, std::string> >::iterator it = finfo.begin();
         it != finfo.end(); it++) {

        std::string name = it->first;
        std::map<std::string, std::string> &info = it->second;

        NameChoice->Append(name);

        std::string type2 = info["Type"];
        if (type2 == "") {
            //old style, map
            if (name == "Coro" || name == "SingleNode") {
                info["Type"] = "SingleNode";
            } else if (name == "NodeRange") {
                info["Type"] = "NodeRange";
            } else {
                info["Type"] = "Matrix";
            }
        }

		for (std::map<std::string, std::string>::iterator it = info.begin(); it != info.end(); ++it)
		{
			if (it->first.substr(0, 5) == "Mouth" || it->first.substr(0, 4) == "Eyes")
			{
				it->second = xLightsXmlFile::FixFile("", it->second);
			}
		}

        faceData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        FaceTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        DeleteButton->Disable();
        FaceTypeChoice->Disable();
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
    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        SingleNodeGrid->SetReadOnly(x, 1);
    }

    wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    wxString filter("0123456789,-");
    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    validator.SetCharIncludes(filter);
    reditor->SetValidator(validator);
    NodeRangeGrid->SetDefaultEditor(reditor);
    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        NodeRangeGrid->SetReadOnly(x, 1);
    }
}


void ModelFaceDialog::GetFaceInfo(std::map< std::string, std::map<std::string, std::string> > &finfo) {
    finfo.clear();
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = faceData.begin();
         it != faceData.end(); it++) {
        if (!it->second.empty()) {
            finfo[it->first] = it->second;
        }
    }
}

static bool SetGrid(wxGrid *grid, std::map<std::string, std::string> &info) {
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(1);
        customColor = true;
    } else {
        grid->HideCol(1);
    }
    for (int x = 0; x < grid->GetNumberRows(); x++) {
        wxString pname = grid->GetRowLabelValue(x);
        pname.Replace(" ", "");
        grid->SetCellValue(x, 0, info[pname.ToStdString()]);

        wxString c = info[pname.ToStdString() + "-Color"];
        if (c == "") {
            c = "#FFFFFF";
        }
        xlColor color(c);
        grid->SetCellBackgroundColour(x, 1, color.asWxColor());
    }
    return customColor;
}

void ModelFaceDialog::SelectFaceModel(const std::string &name) {
    FaceTypeChoice->Enable();
    wxString type = faceData[name]["Type"];
    if (type == "") {
        type = "SingleNode";
        faceData[name]["Type"] = type;
    }
    if (type == "SingleNode") {
        FaceTypeChoice->ChangeSelection(SINGLE_NODE_FACE);
        std::map<std::string, std::string> &info = faceData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
    } else if (type == "NodeRange") {
        FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);
        std::map<std::string, std::string> &info = faceData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    } else {
        FaceTypeChoice->ChangeSelection(MATRIX_FACE);
        for (int r = 0; r < MatrixModelsGrid->GetNumberRows(); r++) {
            for (int c = 0; c < MatrixModelsGrid->GetNumberCols(); c++) {
                wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
                key.Replace(" ", "");
                MatrixModelsGrid->SetCellValue(r, c, faceData[name][key.ToStdString()]);
            }
        }
        std::string w = faceData[name]["ImagePlacement"];
        if (w == "") {
            w = "Centered";
        }
        MatrixImagePlacementChoice->SetStringSelection(w);
    }
}


void ModelFaceDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
}

void ModelFaceDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New Face", "Enter name for new face definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);
            NameChoice->SetStringSelection(n);
            SelectFaceModel(n);
            NameChoice->Enable();
            FaceTypeChoice->Enable();
            DeleteButton->Enable();
        }
    }
}
void ModelFaceDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete face definion?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK , this);
    if (i == wxID_OK || i == wxOK) {

        faceData[name].clear();
        NameChoice->Delete(NameChoice->GetSelection());
        if (NameChoice->GetCount() > 0) {
            NameChoice->SetSelection(0);
            SelectFaceModel(NameChoice->GetString(0).ToStdString());
        } else {
            NameChoice->SetSelection(wxNOT_FOUND);
            NameChoice->Disable();
            FaceTypeChoice->Disable();
            DeleteButton->Disable();
        }
    }
}

void ModelFaceDialog::OnMatrixModelsGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    faceData[name][key.ToStdString()] = MatrixModelsGrid->GetCellValue(r, c);
}


//static const wxString strSupportedImageTypes = "PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPG files(*.jpg)|*.jpg|All files (*.*)|*.*";
static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg|All files (*.*)|*.*";
void ModelFaceDialog::OnMatrixModelsGridCellLeftClick(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
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
        faceData[name][key.ToStdString()] = new_filename;
        MatrixModelsGrid->SetCellValue(r, c, new_filename);
    }
}


void ModelFaceDialog::OnMatricImagePlacementChoiceSelect(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    faceData[name]["ImagePlacement"] = NameChoice->GetString(MatrixImagePlacementChoice->GetSelection());
}

void ModelFaceDialog::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(1);
            faceData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(1);
            faceData[name]["CustomColors"] = "0";
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(1);
            faceData[name]["CustomColors"] = "1";
        } else {
            NodeRangeGrid->HideCol(1);
            faceData[name]["CustomColors"] = "0";
        }
    }
}

static void GetValue(wxGrid *grid, wxGridEvent &event, std::map<std::string, std::string> &info) {
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = grid->GetRowLabelValue(r).ToStdString();
    key.Replace(" ", "");
    if (c == 1) {
        key += "-Color";
        xlColor color = grid->GetCellBackgroundColour(r, c);
        info[key.ToStdString()] = color;
    } else {
        info[key.ToStdString()] = grid->GetCellValue(r, c);
    }
}

void ModelFaceDialog::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event, faceData[name]);
}

void ModelFaceDialog::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event, faceData[name]);
}

void ModelFaceDialog::OnFaceTypeChoicePageChanged(wxChoicebookEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    faceData[name].clear();
    switch (FaceTypeChoice->GetSelection()) {
        case SINGLE_NODE_FACE:
            faceData[name]["Type"] = "SingleNode";
            break;
        case NODE_RANGE_FACE:
            faceData[name]["Type"] = "NodeRange";
            break;
        case MATRIX_FACE:
            faceData[name]["Type"] = "Matrix";
            break;
    }
    SelectFaceModel(name);
}
void ModelFaceDialog::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == 1) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), 1);
        wxColourData data;
        data.SetColour(c);
        wxColourDialog dlg(this, &data);
        if (dlg.ShowModal() == wxID_OK) {
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), 1, dlg.GetColourData().GetColour());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event, faceData[name]);
        }
    }

}

void ModelFaceDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == 1) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), 1);
        wxColourData data;
        data.SetColour(c);
        wxColourDialog dlg(this, &data);
        if (dlg.ShowModal() == wxID_OK) {
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), 1, dlg.GetColourData().GetColour());
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event, faceData[name]);
        }
    }
}
