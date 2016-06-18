#include "ModelStateDialog.h"
#include "xLightsXmlFile.h"

//(*InternalHeaders(ModelStateDialog)
#include <wx/intl.h>
#include <wx/string.h>
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

//(*IdInit(ModelStateDialog)
const long ModelStateDialog::ID_STATICTEXT2 = wxNewId();
const long ModelStateDialog::ID_CHOICE3 = wxNewId();
const long ModelStateDialog::ID_BUTTON3 = wxNewId();
const long ModelStateDialog::ID_BUTTON4 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX1 = wxNewId();
const long ModelStateDialog::ID_GRID_COROSTATES = wxNewId();
const long ModelStateDialog::ID_PANEL2 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX2 = wxNewId();
const long ModelStateDialog::ID_GRID3 = wxNewId();
const long ModelStateDialog::ID_PANEL6 = wxNewId();
const long ModelStateDialog::ID_CHOICEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelStateDialog,wxDialog)
	//(*EventTable(ModelStateDialog)
	//*)
END_EVENT_TABLE()


enum {
    SINGLE_NODE_STATE = 0,
    NODE_RANGE_STATE
};


#include "models/Model.h"

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif


ModelStateDialog::ModelStateDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelStateDialog)
	wxPanel* CoroPanel;
	wxPanel* NodeRangePanel;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxButton* AddButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("State Definition"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
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
	SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROSTATES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROSTATES"));
	SingleNodeGrid->CreateGrid(40,3);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel,wxSize(-1,200)));
	SingleNodeGrid->EnableEditing(true);
	SingleNodeGrid->EnableGridLines(true);
	SingleNodeGrid->SetColLabelSize(20);
	SingleNodeGrid->SetDefaultColSize(150, true);
	SingleNodeGrid->SetColLabelValue(0, _("State"));
	SingleNodeGrid->SetColLabelValue(1, _("Nodes"));
	SingleNodeGrid->SetColLabelValue(2, _("Color"));
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
	NodeRangeGrid->CreateGrid(40,3);
	NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel,wxSize(-1,200)));
	NodeRangeGrid->EnableEditing(true);
	NodeRangeGrid->EnableGridLines(true);
	NodeRangeGrid->SetColLabelSize(20);
	NodeRangeGrid->SetDefaultColSize(150, true);
	NodeRangeGrid->SetColLabelValue(0, _("State"));
	NodeRangeGrid->SetColLabelValue(1, _("Nodes"));
	NodeRangeGrid->SetColLabelValue(2, _("Color"));
	NodeRangeGrid->SetDefaultCellFont( NodeRangeGrid->GetFont() );
	NodeRangeGrid->SetDefaultCellTextColour( NodeRangeGrid->GetForegroundColour() );
	FlexGridSizer5->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 5);
	NodeRangePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(NodeRangePanel);
	FlexGridSizer5->SetSizeHints(NodeRangePanel);
	FaceTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	FaceTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
	FlexGridSizer1->Add(FaceTypeChoice, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelStateDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixAddClicked);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellChange);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellChange);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&ModelStateDialog::OnFaceTypeChoicePageChanged);
	//*)
}

ModelStateDialog::~ModelStateDialog()
{
	//(*Destroy(ModelStateDialog)
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

void ModelStateDialog::SetStateInfo(const Model *cls, std::map< std::string, std::map<std::string, std::string> > &finfo) {
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
            }
        }

		for (std::map<std::string, std::string>::iterator it = info.begin(); it != info.end(); ++it)
		{
			if (it->first.substr(0, 5) == "Mouth" || it->first.substr(0, 4) == "Eyes")
			{
				it->second = xLightsXmlFile::FixFile("", it->second);
			}
		}

        stateData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        FaceTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
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


void ModelStateDialog::GetStateInfo(std::map< std::string, std::map<std::string, std::string> > &finfo) {
    finfo.clear();
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = stateData.begin();
         it != stateData.end(); it++) {
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

void ModelStateDialog::SelectStateModel(const std::string &name) {
    FaceTypeChoice->Enable();
    wxString type = stateData[name]["Type"];
    if (type == "") {
        type = "SingleNode";
        stateData[name]["Type"] = type;
    }
    if (type == "SingleNode") {
        FaceTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
    } else if (type == "NodeRange") {
        FaceTypeChoice->ChangeSelection(NODE_RANGE_STATE);
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    }
}


void ModelStateDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
}

void ModelStateDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New Face", "Enter name for new face definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);
            NameChoice->SetStringSelection(n);
            SelectStateModel(n);
            NameChoice->Enable();
            FaceTypeChoice->Enable();
            DeleteButton->Enable();
        }
    }
}
void ModelStateDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete face definion?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK , this);
    if (i == wxID_OK || i == wxOK) {

        stateData[name].clear();
        NameChoice->Delete(NameChoice->GetSelection());
        if (NameChoice->GetCount() > 0) {
            NameChoice->SetSelection(0);
            SelectStateModel(NameChoice->GetString(0).ToStdString());
        } else {
            NameChoice->SetSelection(wxNOT_FOUND);
            NameChoice->Disable();
            FaceTypeChoice->Disable();
            DeleteButton->Disable();
        }
    }
}

void ModelStateDialog::OnMatrixModelsGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    stateData[name][key.ToStdString()] = MatrixModelsGrid->GetCellValue(r, c);
}


//static const wxString strSupportedImageTypes = "PNG files (*.png)|*.png|BMP files (*.bmp)|*.bmp|JPG files(*.jpg)|*.jpg|All files (*.*)|*.*";
static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg|All files (*.*)|*.*";
void ModelStateDialog::OnMatrixModelsGridCellLeftClick(wxGridEvent& event)
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
        stateData[name][key.ToStdString()] = new_filename;
        MatrixModelsGrid->SetCellValue(r, c, new_filename);
    }
}


void ModelStateDialog::OnMatricImagePlacementChoiceSelect(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    stateData[name]["ImagePlacement"] = NameChoice->GetString(MatrixImagePlacementChoice->GetSelection());
}

void ModelStateDialog::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(1);
            stateData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(1);
            stateData[name]["CustomColors"] = "0";
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(1);
            stateData[name]["CustomColors"] = "1";
        } else {
            NodeRangeGrid->HideCol(1);
            stateData[name]["CustomColors"] = "0";
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

void ModelStateDialog::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event, stateData[name]);
}

void ModelStateDialog::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event, stateData[name]);
}

void ModelStateDialog::OnFaceTypeChoicePageChanged(wxChoicebookEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    stateData[name].clear();
    switch (FaceTypeChoice->GetSelection()) {
        case SINGLE_NODE_STATE:
            stateData[name]["Type"] = "SingleNode";
            break;
        case NODE_RANGE_STATE:
            stateData[name]["Type"] = "NodeRange";
            break;
    }
    SelectStateModel(name);
}
void ModelStateDialog::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
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
            GetValue(NodeRangeGrid, event, stateData[name]);
        }
    }

}

void ModelStateDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
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
            GetValue(SingleNodeGrid, event, stateData[name]);
        }
    }
}
