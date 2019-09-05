#include "ModelStateDialog.h"
#include "xLightsXmlFile.h"
#include "NodesGridCellEditor.h"
#include "ModelPreview.h"
#include "DimmingCurve.h"
#include "SevenSegmentDialog.h"
#include "NodeSelectGrid.h"

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
const long ModelStateDialog::ID_BUTTON1 = wxNewId();
const long ModelStateDialog::ID_GRID_COROSTATES = wxNewId();
const long ModelStateDialog::ID_PANEL2 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX2 = wxNewId();
const long ModelStateDialog::ID_BUTTON2 = wxNewId();
const long ModelStateDialog::ID_GRID3 = wxNewId();
const long ModelStateDialog::ID_PANEL6 = wxNewId();
const long ModelStateDialog::ID_CHOICEBOOK1 = wxNewId();
const long ModelStateDialog::ID_PANEL_PREVIEW = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelStateDialog,wxDialog)
	//(*EventTable(ModelStateDialog)
	//*)
END_EVENT_TABLE()

wxColourData ModelStateDialog::_colorData;

enum {
    SINGLE_NODE_STATE = 0,
    NODE_RANGE_STATE
};

enum {
    //ROWTITLE_COL = 0,
    NAME_COL = 0,
    CHANNEL_COL,
    COLOUR_COL
};


#include "models/Model.h"

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif


ModelStateDialog::ModelStateDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelStateDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxPanel* CoroPanel;
	wxFlexGridSizer* FlexGridSizer3;
	wxPanel* NodeRangePanel;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* PreviewSizer;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxButton* AddButton;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("State Definition"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NameChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	NameChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer7->Add(NameChoice, 1, wxALL|wxEXPAND, 5);
	AddButton = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer7->Add(AddButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteButton = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer7->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	StateTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	CoroPanel = new wxPanel(StateTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CustomColorSingleNode->SetValue(false);
	FlexGridSizer3->Add(CustomColorSingleNode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_7Segment = new wxButton(CoroPanel, ID_BUTTON1, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_7Segment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
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
	NodeRangePanel = new wxPanel(StateTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CustomColorNodeRanges->SetValue(false);
	FlexGridSizer6->Add(CustomColorNodeRanges, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_7Seg = new wxButton(NodeRangePanel, ID_BUTTON2, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer6->Add(Button_7Seg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
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
	StateTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	StateTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
	FlexGridSizer4->Add(StateTypeChoice, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer4->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	ModelPreviewPanelLocation = new wxPanel(this, ID_PANEL_PREVIEW, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_PREVIEW"));
	PreviewSizer = new wxFlexGridSizer(0, 1, 0, 0);
	PreviewSizer->AddGrowableCol(0);
	PreviewSizer->AddGrowableRow(0);
	ModelPreviewPanelLocation->SetSizer(PreviewSizer);
	PreviewSizer->Fit(ModelPreviewPanelLocation);
	PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);
	FlexGridSizer1->Add(ModelPreviewPanelLocation, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelStateDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixDeleteClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButton_7SegmentClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellLeftClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellChange);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellSelect);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButton_7SegmentClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellLeftClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellChange);
	Connect(ID_GRID3,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellSelect);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&ModelStateDialog::OnStateTypeChoicePageChanged);
	//*)

    model = nullptr;

    modelPreview = new ModelPreview(ModelPreviewPanelLocation);
    modelPreview->SetMinSize(wxSize(150, 150));
    PreviewSizer->Add(modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewSizer->Fit(ModelPreviewPanelLocation);
    PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    SetEscapeId(wxID_CANCEL);

    ValidateWindow();
}

ModelStateDialog::~ModelStateDialog()
{
    //(*Destroy(ModelStateDialog)
    //*)
}

void ModelStateDialog::SetStateInfo(Model *cls, std::map< std::string, std::map<std::string, std::string> > &finfo) {
    NodeRangeGrid->SetColSize(COLOUR_COL, 50);
    SingleNodeGrid->SetColSize(COLOUR_COL, 50);
    NameChoice->Clear();
    model = cls;
    modelPreview->SetModel(cls);

    for (std::map< std::string, std::map<std::string, std::string> >::iterator it = finfo.begin();
         it != finfo.end(); ++it) {

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

        stateData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        StateTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        DeleteButton->Disable();
        StateTypeChoice->Disable();
    }

    wxArrayString names;
    names.push_back("");
    for (size_t x = 0; x < cls->GetNodeCount(); x++) {
        wxString nn = cls->GetNodeName(x, true);
        names.push_back(nn);
    }

    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor *neditor = new wxGridCellTextEditor();
        wxString nfilter("abcdefghijklmnopqrstuvwxyz0123456789-_/\\|");
        wxTextValidator nvalidator(wxFILTER_INCLUDE_CHAR_LIST);
        nvalidator.SetCharIncludes(nfilter);
        neditor->SetValidator(nvalidator);

        NodesGridCellEditor *editor = new NodesGridCellEditor();
        editor->names = names;

        SingleNodeGrid->SetCellEditor(x, NAME_COL, neditor);
        SingleNodeGrid->SetCellEditor(x, CHANNEL_COL, editor);
        SingleNodeGrid->SetReadOnly(x, COLOUR_COL);
    }

    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
        wxString filter("0123456789,-");
        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
        validator.SetCharIncludes(filter);
        reditor->SetValidator(validator);

        wxGridCellTextEditor *neditor2 = new wxGridCellTextEditor();
        wxString nfilter2("abcdefghijklmnopqrstuvwxyz0123456789-_/\\|");
        wxTextValidator nvalidator2(wxFILTER_INCLUDE_CHAR_LIST);
        nvalidator2.SetCharIncludes(nfilter2);
        neditor2->SetValidator(nvalidator2);

        NodeRangeGrid->SetCellEditor(x, NAME_COL, neditor2);
        NodeRangeGrid->SetCellEditor(x, CHANNEL_COL, reditor);
        NodeRangeGrid->SetReadOnly(x, COLOUR_COL);
    }

    ValidateWindow();
}

void ModelStateDialog::GetStateInfo(std::map< std::string, std::map<std::string, std::string> > &finfo) {
    finfo.clear();
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = stateData.begin();
         it != stateData.end(); ++it) {
        if (!it->second.empty()) {
            finfo[it->first] = it->second;
        }
    }
}

static bool SetGrid(wxGrid *grid, std::map<std::string, std::string> &info) {
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(COLOUR_COL);
        customColor = true;
    } else {
        grid->HideCol(COLOUR_COL);
    }
    for (int x = 0; x < grid->GetNumberRows(); x++) {
        wxString pname = "s" + grid->GetRowLabelValue(x);
        pname.Replace(" ", "");
        grid->SetCellValue(x, CHANNEL_COL, info[pname.ToStdString()]);

        wxString n = info[pname.ToStdString() + "-Name"];
        grid->SetCellValue(x, NAME_COL, n);

        wxString c = info[pname.ToStdString() + "-Color"];
        if (c == "") {
            c = "#FFFFFF";
        }
        xlColor color(c);
        grid->SetCellBackgroundColour(x, COLOUR_COL, color.asWxColor());
    }
    return customColor;
}

void ModelStateDialog::SelectStateModel(const std::string &name) {
    StateTypeChoice->Enable();
    wxString type = stateData[name]["Type"];
    if (type == "") {
        type = "SingleNode";
        stateData[name]["Type"] = type;
    }
    if (type == "SingleNode") {
        StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
    } else if (type == "NodeRange") {
        StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    }
}

void ModelStateDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    ValidateWindow();
}

void ModelStateDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New State Model", "Enter name for new state model definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);
            NameChoice->SetStringSelection(n);
            SelectStateModel(n);
            NameChoice->Enable();
            StateTypeChoice->Enable();
            DeleteButton->Enable();
        }
    }
    ValidateWindow();
}

void ModelStateDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete state model definion?", "Are you sure you want to delete " + name + "?",
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
            StateTypeChoice->Disable();
            DeleteButton->Disable();
        }
    }
    ValidateWindow();
}

void ModelStateDialog::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "0";
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "1";
        } else {
            NodeRangeGrid->HideCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "0";
        }
    }
}

void ModelStateDialog::GetValue(wxGrid *grid, wxGridEvent &event, std::map<std::string, std::string> &info) {
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = "s" + grid->GetRowLabelValue(r).ToStdString();
    key.Replace(" ", "");
    if (key != "")
    {
        if (c == COLOUR_COL) {
            key += "-Color";
            xlColor color = grid->GetCellBackgroundColour(r, c);
            info[key.ToStdString()] = color;
        }
        else if (c == NAME_COL) {
            key += "-Name";
            info[key.ToStdString()] = grid->GetCellValue(r, c).Lower();
        }
        else {
            info[key.ToStdString()] = grid->GetCellValue(r, c);
        }
    }
    UpdatePreview(grid->GetCellValue(r, CHANNEL_COL).ToStdString(), grid->GetCellBackgroundColour(r, COLOUR_COL));
}

void ModelStateDialog::UpdatePreview(const std::string& channels, wxColor c)
{
    int nn = model->GetNodeCount();
    xlColor cb(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(cb);
    }
    for (int node = 0; node < nn; node++) {
        model->SetNodeColor(node, cb);
    }

    // now highlight selected
    if (channels != "")
    {
        if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens())
            {
                wxString valstr = wtkz.GetNextToken();
                for (size_t n = 0; n < model->GetNodeCount(); n++) {
                    wxString ns = model->GetNodeName(n, true);
                    if (ns == valstr) {
                        model->SetNodeColor(n, c);
                    }
                }
            }
        }
        else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
            std::list<int> ch = model->ParseFaceNodes(channels);
            for (auto it = ch.begin(); it != ch.end(); ++it)
            {
                if (*it < model->GetNodeCount())
                {
                    model->SetNodeColor(*it, c);
                }
            }
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
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

void ModelStateDialog::OnStateTypeChoicePageChanged(wxChoicebookEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    stateData[name].clear();
    switch (StateTypeChoice->GetSelection()) {
        case SINGLE_NODE_STATE:
            stateData[name]["Type"] = "SingleNode";
            break;
        case NODE_RANGE_STATE:
            stateData[name]["Type"] = "NodeRange";
            break;
    }
    SelectStateModel(name);
    UpdatePreview("", *wxWHITE);
}

void ModelStateDialog::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == CHANNEL_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const wxString title = name + " - " + NodeRangeGrid->GetCellValue(event.GetRow(), NAME_COL);
        NodeSelectGrid dialog(title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), this);

        if (dialog.ShowModal() == wxID_OK)
        {
            NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
            GetValue(NodeRangeGrid, event, stateData[name]);
            dialog.Close();
        }
    }
    else if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, dlg.GetColourData().GetColour());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event, stateData[name]);
        }
    }
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
}

void ModelStateDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, dlg.GetColourData().GetColour());
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event, stateData[name]);
        }
    }
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
}

void ModelStateDialog::OnNodeRangeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnNodeRangeGridCellSelect(wxGridEvent& event)
{
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL));
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::AddLabel(wxString label)
{
    int free = -1;
    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    }
    else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    }

    for (int i = 0; i < grid->GetNumberRows(); i++)
    {
        if (grid->GetCellValue(i, NAME_COL) == label)
        {
            free = -1;
            break;
        }

        if (grid->GetCellValue(i,NAME_COL) == "" && free == -1)
        {
            free = i;
        }
    }

    if (free != -1)
    {
        grid->SetCellValue(free, NAME_COL, label);
        wxString key = "s" + grid->GetRowLabelValue(free).ToStdString();
        key.Replace(" ", "");
        if (key != "")
        {
            std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
            key += "-Name";
            stateData[name][key.ToStdString()] = label.Lower();
        }
        UpdatePreview(grid->GetCellValue(free, CHANNEL_COL).ToStdString(), grid->GetCellBackgroundColour(free, COLOUR_COL));
    }
}

void ModelStateDialog::OnButton_7SegmentClick(wxCommandEvent& event)
{
    SevenSegmentDialog dialog(this);

    if (dialog.ShowModal() == wxID_OK)
    {
        if (dialog.CheckBox_Ones->IsChecked())
        {
            for (int i = 0; i < 10; i++)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Tens->IsChecked())
        {
            AddLabel("00");
            for (int i = 10; i < 100; i += 10)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Hundreds->IsChecked())
        {
            AddLabel("000");
            for (int i = 100; i < 1000; i += 100)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Colon->IsChecked())
        {
            AddLabel("colon");
        }
        if (dialog.CheckBox_Decimal->IsChecked())
        {
            AddLabel("dot");
        }
        if (dialog.CheckBox_Thousands->IsChecked())
        {
            AddLabel("0000");
            for (int i = 1000; i < 10000; i += 1000)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
    }
}

void ModelStateDialog::ValidateWindow()
{
    if (NameChoice->GetStringSelection() == "")
    {
        NodeRangeGrid->Disable();
        SingleNodeGrid->Disable();
        Button_7Seg->Disable();
        Button_7Segment->Disable();
        CustomColorSingleNode->Disable();
        CustomColorNodeRanges->Disable();
    }
    else
    {
        NodeRangeGrid->Enable();
        SingleNodeGrid->Enable();
        Button_7Seg->Enable();
        Button_7Segment->Enable();
        CustomColorSingleNode->Enable();
        CustomColorNodeRanges->Enable();
    }
}
