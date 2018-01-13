#include "SubModelsDialog.h"

#include <wx/dnd.h>
#include <wx/xml/xml.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/tokenzr.h>

#include "models/Model.h"
#include "SubBufferPanel.h"
#include "SubModelGenerateDialog.h"
#include "UtilFunctions.h"

//(*InternalHeaders(SubModelsDialog)
#include <wx/grid.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)


//(*IdInit(SubModelsDialog)
const long SubModelsDialog::ID_STATICTEXT1 = wxNewId();
const long SubModelsDialog::ID_LISTCTRL_SUB_MODELS = wxNewId();
const long SubModelsDialog::ID_BUTTON3 = wxNewId();
const long SubModelsDialog::ID_BUTTON4 = wxNewId();
const long SubModelsDialog::ID_BUTTON5 = wxNewId();
const long SubModelsDialog::ID_PANEL4 = wxNewId();
const long SubModelsDialog::ID_STATICTEXT_NAME = wxNewId();
const long SubModelsDialog::ID_TEXTCTRL_NAME = wxNewId();
const long SubModelsDialog::ID_CHECKBOX1 = wxNewId();
const long SubModelsDialog::ID_BUTTON6 = wxNewId();
const long SubModelsDialog::ID_GRID1 = wxNewId();
const long SubModelsDialog::ID_BUTTON1 = wxNewId();
const long SubModelsDialog::ID_BUTTON2 = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_UP = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_DOWN = wxNewId();
const long SubModelsDialog::ID_PANEL2 = wxNewId();
const long SubModelsDialog::ID_PANEL3 = wxNewId();
const long SubModelsDialog::ID_NOTEBOOK1 = wxNewId();
const long SubModelsDialog::ID_PANEL5 = wxNewId();
const long SubModelsDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long SubModelsDialog::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SubModelsDialog,wxDialog)
	//(*EventTable(SubModelsDialog)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_VMDROP, SubModelsDialog::OnDrop)
END_EVENT_TABLE()

#include "ModelPreview.h"
#include "DimmingCurve.h"


SubModelsDialog::SubModelsDialog(wxWindow* parent)
{
	//(*Initialize(SubModelsDialog)
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxPanel* Panel1;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer5;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Sub Models"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetClientSize(wxSize(778,368));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel2 = new wxPanel(SplitterWindow1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(1);
	StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("SubModels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer9->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl_SubModels = new wxListCtrl(Panel2, ID_LISTCTRL_SUB_MODELS, wxDefaultPosition, wxSize(150,-1), wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_SUB_MODELS"));
	ListCtrl_SubModels->SetMinSize(wxSize(150,-1));
	FlexGridSizer9->Add(ListCtrl_SubModels, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10 = new wxFlexGridSizer(1, 3, 0, 0);
	AddButton = new wxButton(Panel2, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer10->Add(AddButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteButton = new wxButton(Panel2, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer10->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Generate = new wxButton(Panel2, ID_BUTTON5, _("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer10->Add(Button_Generate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	Panel2->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(Panel2);
	FlexGridSizer9->SetSizeHints(Panel2);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticTextName = new wxStaticText(Panel3, ID_STATICTEXT_NAME, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_NAME"));
	FlexGridSizer7->Add(StaticTextName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(Panel3, ID_TEXTCTRL_NAME, _("Submodel Name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_NAME"));
	FlexGridSizer7->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TypeNotebook = new wxNotebook(Panel3, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel1 = new wxPanel(TypeNotebook, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	FlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer8->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	LayoutCheckbox = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Vertical Buffer Layout"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	LayoutCheckbox->SetValue(false);
	FlexGridSizer6->Add(LayoutCheckbox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button_ReverseNodes = new wxButton(Panel1, ID_BUTTON6, _("Reverse Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer6->Add(Button_ReverseNodes, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	NodesGrid = new wxGrid(Panel1, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
	NodesGrid->CreateGrid(5,1);
	NodesGrid->EnableEditing(true);
	NodesGrid->EnableGridLines(true);
	NodesGrid->SetColLabelSize(18);
	NodesGrid->SetDefaultColSize(160, true);
	NodesGrid->SetColLabelValue(0, _("Node Ranges"));
	NodesGrid->SetRowLabelValue(0, _("Strand   1"));
	NodesGrid->SetDefaultCellFont( NodesGrid->GetFont() );
	NodesGrid->SetDefaultCellTextColour( NodesGrid->GetForegroundColour() );
	FlexGridSizer8->Add(NodesGrid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	AddRowButton = new wxButton(Panel1, ID_BUTTON1, _("Add Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(AddRowButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteRowButton = new wxButton(Panel1, ID_BUTTON2, _("Delete Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(DeleteRowButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveUp = new wxButton(Panel1, ID_BUTTON_MOVE_UP, _("^"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
	FlexGridSizer5->Add(Button_MoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveDown = new wxButton(Panel1, ID_BUTTON_MOVE_DOWN, _("v"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_DOWN"));
	FlexGridSizer5->Add(Button_MoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	Panel1->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel1);
	FlexGridSizer4->SetSizeHints(Panel1);
	SubBufferPanelHolder = new wxPanel(TypeNotebook, ID_PANEL3, wxPoint(90,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	SubBufferSizer = new wxFlexGridSizer(1, 1, 0, 0);
	SubBufferSizer->AddGrowableCol(0);
	SubBufferSizer->AddGrowableRow(0);
	SubBufferPanelHolder->SetSizer(SubBufferSizer);
	SubBufferSizer->Fit(SubBufferPanelHolder);
	SubBufferSizer->SetSizeHints(SubBufferPanelHolder);
	TypeNotebook->AddPage(Panel1, _("Node Ranges"), false);
	TypeNotebook->AddPage(SubBufferPanelHolder, _("SubBuffer"), false);
	FlexGridSizer3->Add(TypeNotebook, 1, wxALL|wxEXPAND, 0);
	Panel3->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel3);
	FlexGridSizer3->SetSizeHints(Panel3);
	SplitterWindow1->SplitVertically(Panel2, Panel3);
	FlexGridSizer2->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	ModelPreviewPanelLocation = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	ModelPreviewPanelLocation->SetMinSize(wxDLG_UNIT(this,wxSize(100,100)));
	PreviewSizer = new wxFlexGridSizer(1, 1, 0, 0);
	PreviewSizer->AddGrowableCol(0);
	PreviewSizer->AddGrowableRow(0);
	ModelPreviewPanelLocation->SetSizer(PreviewSizer);
	PreviewSizer->Fit(ModelPreviewPanelLocation);
	PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);
	FlexGridSizer2->Add(ModelPreviewPanelLocation, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	Center();

	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsBeginDrag);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsItemSelect);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsKeyDown);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsColumnClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddButtonClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteButtonClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_GenerateClick);
	Connect(ID_TEXTCTRL_NAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SubModelsDialog::OnTextCtrl_NameText_Change);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnLayoutCheckboxClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseNodesClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddRowButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteRowButtonClick);
	//*)

    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellChange);

    SetSize(1200, 800);
    _numSubModels = 0;
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsSubModelDialogPosition", sz, loc);
    if (loc.x != -1)
    {
        SetPosition(loc);
    }

    modelPreview = new ModelPreview(ModelPreviewPanelLocation);
    modelPreview->SetMinSize(wxSize(150, 150));
    PreviewSizer->Add(modelPreview, 1, wxALL|wxEXPAND, 0);
    PreviewSizer->Fit(ModelPreviewPanelLocation);
    PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);

    subBufferPanel = new SubBufferPanel(SubBufferPanelHolder, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    wxSize s(50,50);
    subBufferPanel->SetMinSize(s);
    SubBufferSizer->Insert(0, subBufferPanel,1, wxALL|wxEXPAND, 2);
    SubBufferSizer->Fit(SubBufferPanelHolder);
    Connect(subBufferPanel->GetId(),SUBBUFFER_RANGE_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnSubBufferRangeChange);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    SplitterWindow1->SetMinSize(wxSize(1000,400));

    SubModelTextDropTarget *mdt = new SubModelTextDropTarget(this, ListCtrl_SubModels, "SubModel");
    ListCtrl_SubModels->SetDropTarget(mdt);

    NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
}

SubModelsDialog::~SubModelsDialog()
{
	//(*Destroy(SubModelsDialog)
	//*)
    SaveWindowPosition("xLightsSubModelDialogPosition", this);
}

SubModelsDialog::SubModelInfo &SubModelsDialog::GetSubModelInfo(const wxString &str) {
    for (int x = 0; x < subModels.size(); x++) {
        if (subModels[x].name == str) {
            return subModels[x];
        }
    }
    subModels.push_back(SubModelInfo(str));
    return subModels.back();
}


void SubModelsDialog::Setup(Model *m)
{
    ListCtrl_SubModels->Freeze();

    ListCtrl_SubModels->ClearAll();
    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetAlign(wxLIST_FORMAT_LEFT);
    nm0.SetText(_("SubModel"));
    ListCtrl_SubModels->InsertColumn(1, nm0);


    model = m;
    modelPreview->SetModel(m);
    wxXmlNode * root = m->GetModelXml();
    wxXmlNode * child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxString name = child->GetAttribute("name");
            SubModelInfo &sm = GetSubModelInfo(name);
            sm.name = name;
            sm.isRanges = child->GetAttribute("type", "ranges") == "ranges";
            sm.vertical = child->GetAttribute("layout") == "vertical";
            sm.subBuffer = child->GetAttribute("subBuffer");
            sm.strands.resize(1);
            sm.strands[0] = "";
            int x = 0;
            while (child->HasAttribute(wxString::Format("line%d", x))) {
                if (x >= sm.strands.size()) {
                    sm.strands.resize(x + 1);
                }
                sm.strands[x] = child->GetAttribute(wxString::Format("line%d", x));
                x++;
            }
            AddSubModelToList(&sm, -1, true);
        }
        child = child->GetNext();
    }

    ListCtrl_SubModels->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrl_SubModels->GetColumnWidth(0) < 100) {
        ListCtrl_SubModels->SetColumnWidth(0, 100);
    }

    // use a general validateDialog() here?
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        Select(ListCtrl_SubModels->GetItemText(0));
        ListCtrl_SubModels->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    } else {
        ListCtrl_SubModels->Disable();
        TextCtrl_Name->Disable();
        DeleteButton->Disable();
        NodesGrid->Disable();
        LayoutCheckbox->Disable();
        AddRowButton->Disable();
        DeleteRowButton->Disable();
        subBufferPanel->Disable();
        TypeNotebook->Disable();
    }

    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsSubModelDialogPosition", sz, loc);
    if (loc.x != -1)
    {
        SetPosition(loc);
        SetSize(sz);
    }

    // SortSubModels();
    ListCtrl_SubModels->Thaw();
    ListCtrl_SubModels->Refresh();
}

bool SubModelsDialog::IsItemSelected(wxListCtrl* ctrl, int item)
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

void SubModelsDialog::AddSubModelToList(SubModelInfo *submodel, int index, bool load)
{
    int idx = 0;
    if (index < 0)
    {
        idx = ListCtrl_SubModels->InsertItem(_numSubModels, submodel->name);
    } else {
        idx = ListCtrl_SubModels->InsertItem(index, submodel->name);
    }
    ListCtrl_SubModels->SetItemPtrData(idx, (wxUIntPtr)submodel);
    if (!load)
    {
        subModels.insert(subModels.begin(), *submodel);
    } else {
         ++_numSubModels;
    }
}

void SubModelsDialog::RemoveSubModelFromList(wxString name)
{
    subModels.erase(subModels.begin() + GetSubModelInfoIndex(name));
    long id = ListCtrl_SubModels->FindItem(-1, name);
    ListCtrl_SubModels->DeleteItem(id);
    --_numSubModels;
}

wxString SubModelsDialog::GetSelectedName()
{
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i) )
        {
            return ListCtrl_SubModels->GetItemText(i);
        }
    }
    return "";
}

void SubModelsDialog::Save()
{
    wxXmlNode * root = model->GetModelXml();
    wxXmlNode * child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxXmlNode *n = child;
            child = child->GetNext();
            root->RemoveChild(n);
            delete n;
        } else {
            child = child->GetNext();
        }
    }
    for (auto a = subModels.begin(); a != subModels.end(); ++a) {
        child = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
        child->AddAttribute("name", a->name);
        child->AddAttribute("layout", a->vertical ? "vertical" : "horizontal");
        child->AddAttribute("type", a->isRanges ? "ranges" : "subbuffer");
        if (a->isRanges) {
            for (int x = 0; x < a->strands.size(); x++) {
                child->AddAttribute(wxString::Format("line%d", x), a->strands[x]);
            }
        } else {
            child->AddAttribute("subBuffer", a->subBuffer);
        }
        root->AddChild(child);
    }
}

int SubModelsDialog::GetSubModelInfoIndex(const wxString &name) {
    for (int x = 0; x < subModels.size(); x++) {
        if (subModels[x].name == name) {
            return x;
        }
    }
    return -1;
}

void SubModelsDialog::OnAddButtonClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Enter name for sub model");
    wxString name = "";
    int id = wxID_OK;
    while (name == "" && GetSubModelInfoIndex(name) == -1 && id == wxID_OK) {
        id = dlg.ShowModal();
        name= dlg.GetValue();
    }
    if (id == wxID_OK) {
        SubModelInfo &sm = GetSubModelInfo(name);

        sm.name = name;
        sm.vertical = false;
        sm.isRanges = true;
        sm.strands.clear();
        sm.strands.push_back("");
        AddSubModelToList(&sm);
        Select(name);
    }
}

void SubModelsDialog::OnDeleteButtonClick(wxCommandEvent& event)
{
    // get the current selected submodel
    wxString name = GetSelectedName();
    // ask user if they really want to delete it
    int answer = wxMessageBox("Are you sure you want to delete sub model " + name + "?",
                              "Delete Model",
                              wxYES_NO, this);
    // no: return
    if (answer == wxNO) {
        return;
    }
    // delete selected submodel
    RemoveSubModelFromList(name);
    ValidateWindow();
}

void SubModelsDialog::ValidateWindow()
{
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        Select(ListCtrl_SubModels->GetItemText(0));
        ListCtrl_SubModels->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    } else {
        ListCtrl_SubModels->Disable();
        TextCtrl_Name->Disable();
        DeleteButton->Disable();
        NodesGrid->Disable();
        LayoutCheckbox->Disable();
        AddRowButton->Disable();
        DeleteRowButton->Disable();
        subBufferPanel->Disable();
        TypeNotebook->Disable();
    }
}

void SubModelsDialog::OnNameChoiceSelect(wxCommandEvent& event)
{
    Select(event.GetString());
}

void SubModelsDialog::Select(const wxString &name) {
    if (name == "") {
        TextCtrl_Name->Disable();
        DeleteButton->Disable();
        NodesGrid->Disable();
        LayoutCheckbox->Disable();
        AddRowButton->Disable();
        DeleteRowButton->Disable();
        subBufferPanel->Disable();
        TypeNotebook->Disable();
        return;
    }
    TextCtrl_Name->Enable();
    DeleteButton->Enable();
    NodesGrid->Enable();
    LayoutCheckbox->Enable();
    AddRowButton->Enable();
    subBufferPanel->Enable();
    TypeNotebook->Enable();

    SubModelInfo &sm = GetSubModelInfo(name);
    DeleteRowButton->Enable(sm.strands.size() > 1);

    TextCtrl_Name->SetValue(name);
    int idx = ListCtrl_SubModels->FindItem(-1, name);
    ListCtrl_SubModels->SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

    if (sm.isRanges) {
        TypeNotebook->SetSelection(0);
        LayoutCheckbox->SetValue(sm.vertical);
        NodesGrid->BeginBatch();
        if (NodesGrid->GetNumberRows() > 0) {
            NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
        }
        for (int x = 0; x < sm.strands.size(); x++) {
            NodesGrid->AppendRows(1);
            if (x == 0)
            {
                NodesGrid->SetRowLabelValue(x, "Bottom");
            }
            else if (x == sm.strands.size() - 1)
            {
                NodesGrid->SetRowLabelValue(x, "Top");
            }
            else
            {
                NodesGrid->SetRowLabelValue(x, wxString::Format("Line %d", (x + 1)));
            }
            NodesGrid->SetCellValue(x, 0, sm.strands[x]);
        }
        NodesGrid->EndBatch();
        NodesGrid->GoToCell(0, 0);
        SelectRow(0);
    } else {
        TypeNotebook->SetSelection(1);
        subBufferPanel->SetValue(sm.subBuffer.ToStdString());
        DisplayRange(sm.subBuffer);
    }

//    NodesGrid->SetGridCursor(0,0);
}

void SubModelsDialog::OnNodesGridCellChange(wxGridEvent& event)
{
    int r = event.GetRow();
    SubModelInfo &sm = GetSubModelInfo(GetSelectedName());
    sm.strands[r] = NodesGrid->GetCellValue(r, 0);
    SelectRow(r);
}

void SubModelsDialog::OnNodesGridCellSelect(wxGridEvent& event)
{
    SelectRow(event.GetRow());
}

void SubModelsDialog::DisplayRange(const wxString &range) {
    float x1 = 0;
    float x2 = 100;
    float y1 = 0;
    float y2 = 100;
    if (range != "") {
        wxArrayString v = wxSplit(range, 'x');
        x1 = v.size() > 0 ? wxAtof(v[0]) : 0.0;
        y1 = v.size() > 1 ? wxAtof(v[1]) : 0.0;
        x2 = v.size() > 2 ? wxAtof(v[2]) : 100.0;
        y2 = v.size() > 3 ? wxAtof(v[3]) : 100.0;
    }

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    x1 *= (float) model->GetDefaultBufferWi();
    x2 *= (float) model->GetDefaultBufferWi();
    y1 *= (float) model->GetDefaultBufferHt();
    y2 *= (float) model->GetDefaultBufferHt();
    x1 /= 100.0;
    x2 /= 100.0;
    y1 /= 100.0;
    y2 /= 100.0;
    x1 = std::ceil(x1);
    y1 = std::ceil(y1);

    int nn = model->GetNodeCount();
    xlColor c(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        if (model->IsNodeInBufferRange(node, x1, y1, x2, y2)) {
            model->SetNodeColor(node, xlWHITE);
        } else {
            model->SetNodeColor(node, c);
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
}

void SubModelsDialog::SelectRow(int r) {
    int nn = model->GetNodeCount();
    xlColor c(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        model->SetNodeColor(node, c);
    }

    int start = r;
    int end = r;
    if (r == -1) {
        start = 0;
        end = NodesGrid->GetNumberRows() - 1;
    }
    for (int cur = start; cur <= end; cur++) {
        wxString v = NodesGrid->GetCellValue(cur, 0);
        wxStringTokenizer wtkz(v, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            int start2, end2;
            if (valstr.Contains("-")) {
                int idx = valstr.Index('-');
                start2 = wxAtoi(valstr.Left(idx));
                end2 = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            } else {
                start2 = end2 = wxAtoi(valstr);
            }
            start2--;
            end2--;
            bool done = false;
            int n = start2;
            while (!done) {
                if (n < model->GetNodeCount()) {
                    model->SetNodeColor(n, xlWHITE);
                }
                if (start2 > end2) {
                    n--;
                    done = n < end2;
                } else {
                    n++;
                    done = n > end2;
                }
            }
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
}

void SubModelsDialog::OnLayoutCheckboxClick(wxCommandEvent& event)
{
    SubModelInfo &sm = GetSubModelInfo(GetSelectedName());
    sm.vertical = LayoutCheckbox->GetValue();
}

void SubModelsDialog::OnAddRowButtonClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo &sm = GetSubModelInfo(name);
    sm.strands.push_back("");
    DeleteRowButton->Enable();
    Select(name);
}

void SubModelsDialog::OnDeleteRowButtonClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo &sm = GetSubModelInfo(name);
    int row = NodesGrid->GetGridCursorRow();
    int sz = sm.strands.size();
    sm.strands.erase(sm.strands.begin()+row);
    sm.strands.resize(sz - 1);
    Select(name);
    DeleteRowButton->Enable(sm.strands.size() > 1);
}

void SubModelsDialog::OnTypeNotebookPageChanged(wxBookCtrlEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo &sm = GetSubModelInfo(name);
    sm.isRanges = TypeNotebook->GetSelection() == 0;
    Select(name);
}

void SubModelsDialog::OnSubBufferRangeChange(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo &sm = GetSubModelInfo(name);
    sm.isRanges = false;
    sm.subBuffer = event.GetString();
    DisplayRange(sm.subBuffer);
}


void SubModelsDialog::OnNodesGridLabelLeftClick(wxGridEvent& event)
{
    SelectRow(event.GetRow());
    if (event.GetRow() != -1) {
        NodesGrid->GoToCell(event.GetRow(), 0);
    }
}

// Value must be 0-1
// When zero then we will always get generated segments slightly overlapping
// When one then they should never overlap but you may also get some gaps
#define GENERATE_GAP 0.25

void SubModelsDialog::GenerateSegment(SubModelsDialog::SubModelInfo& sm, int segments, int segment, bool horizontal, int count)
{
    if (horizontal)
    {
        float perx = 100.0 / segments;
        int offset = segment % segments;
        float startx = offset * perx;
        float endx = startx + perx - GENERATE_GAP;
        if ((segment + 1) % segments == 0) endx = 100;

        float per = 100.0 / (count / segments);
        float start = segment / segments * per;
        float end = start + per - GENERATE_GAP;

        if ((segment + 1) / segments == count / segments) end = 100;

        sm.isRanges = false;
        sm.subBuffer = wxString::Format("%.2fx%.2fx%.2fx%.2f", startx, start, endx, end);
    }
    else
    {
        float pery = 100.0 / segments;
        int offset = segment % segments;
        float starty = offset * pery;
        float endy = starty + pery - GENERATE_GAP;
        if ((segment + 1) % segments == 0) endy = 100;

        float per = 100.0 / (count / segments);
        float start = segment / segments * per;
        float end = start + per - GENERATE_GAP;

        if ((segment + 1) / segments == count /segments) end = 100;

        sm.isRanges = false;
        sm.subBuffer = wxString::Format("%.2fx%.2fx%.2fx%.2f", start, starty, end, endy);
    }
}

void SubModelsDialog::OnButton_GenerateClick(wxCommandEvent& event)
{
    SubModelGenerateDialog dialog(this, model->GetDefaultBufferWi(), model->GetDefaultBufferHt(), model->GetNodeCount());

    if (dialog.ShowModal() == wxID_OK)
    {
        int last = 0;
        for (int i = 0; i < dialog.GetCount(); i++)
        {
            wxString name = wxString::Format("%s-%i", dialog.GetBaseName(), i + 1);

            if (GetSubModelInfoIndex(name) != -1)
            {
                // this name clashes ... so I cant create it
            }
            else
            {
                SubModelInfo &sm = GetSubModelInfo(name);

                sm.name = name;
                sm.vertical = false;
                sm.strands.clear();
                sm.strands.push_back("");
                AddSubModelToList(&sm);

                if (dialog.GetType() == "Vertical Slices")
                {
                    GenerateSegment(sm, 1, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Horizontal Slices")
                {
                    GenerateSegment(sm, 1, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 2 Wide")
                {
                    GenerateSegment(sm, 2, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 2 High")
                {
                    GenerateSegment(sm, 2, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 3 Wide")
                {
                    GenerateSegment(sm, 3, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 3 High")
                {
                    GenerateSegment(sm, 3, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Nodes")
                {
                    sm.isRanges = true;
                    float per = (float)model->GetNodeCount() / (float)dialog.GetCount();
                    int start = last + 1;
                    int end = (i+1) * per;

                    if (i == dialog.GetCount() - 1)
                    {
                        end = model->GetNodeCount();
                    }

                    last = end;
                    if (start == end)
                    {
                        sm.strands[0] = wxString::Format("%i", start);
                    }
                    else
                    {
                        sm.strands[0] = wxString::Format("%i-%i", start, end);
                    }
                }
                Select(name);
            }
        }
    }
}

void SubModelsDialog::OnButton_ReverseNodesClick(wxCommandEvent& event)
{
    SubModelInfo &sm = GetSubModelInfo(GetSelectedName());

    if (sm.isRanges)
    {
        std::vector<wxString> newStrands;
        for (auto it = sm.strands.begin(); it != sm.strands.end(); ++it)
        {
            wxString newStrand = "";
            auto nodes = wxSplit(*it, ',');
            for (auto nit = nodes.rbegin(); nit != nodes.rend(); ++nit)
            {
                if (newStrand != "") newStrand += ",";
                if (nit->Contains('-'))
                {
                    auto range = wxSplit(*nit, '-');
                    if (range.size() == 2)
                    {
                        newStrand += range[1] + "-" + range[0];
                    }
                    else
                    {
                        // not valid so just copy
                        newStrand += *nit;
                    }
                }
                else
                {
                    newStrand += *nit;
                }
            }
            newStrands.push_back(newStrand);
        }
        Select(GetSelectedName());
    }
}

void SubModelsDialog::OnListCtrl_SubModelsItemSelect(wxListEvent& event)
{
    Select(GetSelectedName());
    // ValidateWindow();
}

void SubModelsDialog::OnListCtrl_SubModelsBeginDrag(wxListEvent& event)
{
    if (ListCtrl_SubModels->GetSelectedItemCount() == 0) return;

    wxString drag = "SubModel";
    for (size_t i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i))
        {
            drag += "," + ListCtrl_SubModels->GetItemText(i);
        }
    }

    wxTextDataObject my_data(drag);
    wxDropSource dragSource(this);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);
}

void SubModelsDialog::OnListCtrl_SubModelsColumnClick(wxListEvent& event)
{
}

void SubModelsDialog::OnListCtrl_SubModelsKeyDown(wxListEvent& event)
{
}

void SubModelsDialog::MoveSelectedModelsTo(int indexTo)
{
    wxString name = GetSelectedName();
    int indexFrom = GetSubModelInfoIndex(name);
    if (indexTo == indexFrom) { return; }

    SubModelInfo sm = subModels.at(indexFrom);
    RemoveSubModelFromList(sm.name);
    AddSubModelToList(&sm, indexTo);
    Select(name);
    ListCtrl_SubModels->SetItemState(indexTo, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void SubModelsDialog::OnDrop(wxCommandEvent& event)
{
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch(event.GetInt())
    {
        case 0:
            // an add
            break;
        case 1:
            // Model dropped into models (a reorder)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            long index = ListCtrl_SubModels->HitTest(wxPoint(x, y), flags, nullptr);

            MoveSelectedModelsTo(index);
        }
            break;
        case 2:
            // a remove
           break;
        default:
            break;
    }
}

#pragma region Drag and Drop

wxDragResult SubModelTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    static int MINSCROLLDELAY = 10;
    static int STARTSCROLLDELAY = 300;
    static int scrollDelay = STARTSCROLLDELAY;
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay)
    {
        // too soon to scroll again
    }
    else
    {
        if (_type == "SubModel" && _list->GetItemCount() > 0)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);

            for (int i = 0; i < _list->GetItemCount(); ++i)
            {
                if (i == lastItem)
                {
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                }
                else
                {
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
                }
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                // scroll up
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem()-1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                // scroll down
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem+1);
                    lastTime = wxGetUTCTimeMillis();
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else
            {
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }

    return wxDragMove;
}

bool SubModelTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_VMDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "SubModel")
    {
        if (_type == "SubModel")
        {
            event.SetInt(1);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

#pragma endregion Drag and Drop


void SubModelsDialog::OnTextCtrl_NameText_Change(wxCommandEvent& event)
{
    wxString s = TextCtrl_Name->GetValue();
    int idx = GetSubModelInfoIndex(GetSelectedName());
//    SubModelInfo sm = subModels.at(idx);
//    sm.name = s;
//    SubModelInfo *smi = (*SubModelInfo)ListCtrl_SubModels->GetItemData(idx);
//    smi->name = s;
//    ListCtrl_SubModels->SetItemText(idx, s);
}
