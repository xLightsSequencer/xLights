#include "LMSImportChannelMapDialog.h"
#include "SequenceElements.h"
#include "xLightsMain.h"
#include "ModelClass.h"

//(*InternalHeaders(LMSImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LMSImportChannelMapDialog)
const long LMSImportChannelMapDialog::ID_CHOICE1 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON_ADDMODEL = wxNewId();
const long LMSImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const long LMSImportChannelMapDialog::ID_GRID1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LMSImportChannelMapDialog,wxDialog)
	//(*EventTable(LMSImportChannelMapDialog)
	//*)
END_EVENT_TABLE()


LMSImportChannelMapDialog::LMSImportChannelMapDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(LMSImportChannelMapDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer->AddGrowableCol(0);
	Sizer->AddGrowableRow(2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	ModelsChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(ModelsChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddModelButton = new wxButton(this, ID_BUTTON_ADDMODEL, _("Add Model For Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADDMODEL"));
	FlexGridSizer1->Add(AddModelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MapByStrand = new wxCheckBox(this, ID_CHECKBOX1, _("Map by Strand/CCR"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	MapByStrand->SetValue(false);
	Sizer->Add(MapByStrand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChannelMapGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	ChannelMapGrid->CreateGrid(18,7);
	ChannelMapGrid->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,440)));
	ChannelMapGrid->EnableEditing(true);
	ChannelMapGrid->EnableGridLines(true);
	ChannelMapGrid->SetColLabelValue(0, _("Model"));
	ChannelMapGrid->SetColLabelValue(1, _("Strand"));
	ChannelMapGrid->SetColLabelValue(2, _("Node"));
	ChannelMapGrid->SetColLabelValue(3, _("Channel"));
	ChannelMapGrid->SetColLabelValue(4, _("Color"));
	ChannelMapGrid->SetDefaultCellFont( ChannelMapGrid->GetFont() );
	ChannelMapGrid->SetDefaultCellTextColour( ChannelMapGrid->GetForegroundColour() );
	Sizer->Add(ChannelMapGrid, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	Sizer->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(Sizer);
	Sizer->Fit(this);
	Sizer->SetSizeHints(this);

	Connect(ID_BUTTON_ADDMODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnAddModelButtonClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnMapByStrandClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridCellChange);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_HIDDEN,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridEditorHidden);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_SHOWN,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridEditorShown);
	//*)
}

LMSImportChannelMapDialog::~LMSImportChannelMapDialog()
{
	//(*Destroy(LMSImportChannelMapDialog)
	//*)
}

void LMSImportChannelMapDialog::Init() {
    for (int i=0;i<mSequenceElements->GetElementCount();i++) {
        if (mSequenceElements->GetElement(i)->GetType() == "model") {
            ModelsChoice->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }
    int sz = ChannelMapGrid->GetColSize(3);
    ChannelMapGrid->DeleteCols(5, 2);
    ChannelMapGrid->SetColSize(0, sz * 1.5);
    ChannelMapGrid->SetColSize(1, sz * 1.5);
    ChannelMapGrid->SetColSize(2, sz * 1.5);
    ChannelMapGrid->SetColSize(3, sz * 2.5);
    ChannelMapGrid->SetColSize(4, sz / 2);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    ChannelMapGrid->SetRowLabelSize(0);
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(MapByStrand->GetValue() ? ccrNames : channelNames);
    ChannelMapGrid->SetDefaultEditor(editor);
}

void LMSImportChannelMapDialog::SetupByNode() {
    MapByStrand->SetValue(false);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    for (int x = 0; x < modelNames.size(); x++) {
        AddModel(xlights->GetModelClass(modelNames[x]));
    }
}
void LMSImportChannelMapDialog::SetupByStrand() {
    MapByStrand->SetValue(true);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    for (int x = 0; x < modelNames.size(); x++) {
        AddModel(xlights->GetModelClass(modelNames[x]));
    }
}



void LMSImportChannelMapDialog::OnMapByStrandClick(wxCommandEvent& event)
{
    ChannelMapGrid->BeginBatch();
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(MapByStrand->GetValue() ? ccrNames : channelNames);
    ChannelMapGrid->SetDefaultEditor(editor);
    if (MapByStrand->GetValue()) {
        SetupByStrand();
    } else {
        SetupByNode();
    }
    ChannelMapGrid->EndBatch();
}
void LMSImportChannelMapDialog::AddModel(ModelClass &cls) {
    ChannelMapGrid->BeginBatch();
    int i = ChannelMapGrid->GetRows();
    int start = i;
    ChannelMapGrid->AppendRows();
    ChannelMapGrid->SetCellValue(i, 0, cls.name);
    for (int s = 0; s < cls.GetNumStrands(); s++) {
        i = ChannelMapGrid->GetRows();
        ChannelMapGrid->AppendRows();
        ChannelMapGrid->SetCellValue(i, 0, cls.name);
        wxString sn = cls.GetStrandName(s);
        if ("" == sn) {
            sn = wxString::Format("Strand %d", s + 1);
        }
        ChannelMapGrid->SetCellValue(i, 1, sn);
        if (!MapByStrand->GetValue()) {
            for (int n = 0; n < cls.GetStrandLength(s); n++) {
                i = ChannelMapGrid->GetRows();
                ChannelMapGrid->AppendRows();
                ChannelMapGrid->SetCellValue(i, 0, cls.name);
                ChannelMapGrid->SetCellValue(i, 1, sn);
                wxString nn = cls.GetNodeName(cls.MapToNodeIndex(s, n));
                if ("" == nn) {
                    nn = wxString::Format("Node %d", n + 1);
                }
                ChannelMapGrid->SetCellValue(i, 2, nn);
            }
        }
    }
    for (int r = start; r < ChannelMapGrid->GetRows(); r++) {
        ChannelMapGrid->SetReadOnly(r, 0);
        ChannelMapGrid->SetReadOnly(r, 1);
        ChannelMapGrid->SetReadOnly(r, 2);
        ChannelMapGrid->SetReadOnly(r, 4);
    }
    ChannelMapGrid->EndBatch();
}

void LMSImportChannelMapDialog::OnAddModelButtonClick(wxCommandEvent& event)
{
    wxString name = ModelsChoice->GetStringSelection();
    ModelsChoice->Delete(ModelsChoice->GetSelection());
    Element * model = nullptr;
    for (int i=0;i<mSequenceElements->GetElementCount();i++) {
        if (mSequenceElements->GetElement(i)->GetType() == "model"
            && name == mSequenceElements->GetElement(i)->GetName()) {
            model = mSequenceElements->GetElement(i);
        }
    }
    modelNames.push_back(name);
    ModelClass &cls = xlights->GetModelClass(name);
    AddModel(cls);
}

void LMSImportChannelMapDialog::OnChannelMapGridCellChange(wxGridEvent& event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    wxString s = ChannelMapGrid->GetCellValue(row, col);
    ChannelMapGrid->SetCellBackgroundColour(channelColors[s].asWxColor(), row, col + 1);
    MapByStrand->Enable(false);
    ChannelMapGrid->Refresh();
}

void LMSImportChannelMapDialog::OnChannelMapGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == 4) {
        wxColor c = ChannelMapGrid->GetCellBackgroundColour(event.GetRow(), 4);
        wxColourData data;
        data.SetColour(c);
        wxColourDialog dlg(this, &data);
        dlg.ShowModal();
        ChannelMapGrid->SetCellBackgroundColour(event.GetRow(), 4, dlg.GetColourData().GetColour());
        ChannelMapGrid->Refresh();
    }
}

void LMSImportChannelMapDialog::OnChannelMapGridEditorShown(wxGridEvent& event)
{
    ModelsChoice->Enable(false);
    AddModelButton->Enable(false);
    MapByStrand->Enable(false);
}

void LMSImportChannelMapDialog::OnChannelMapGridEditorHidden(wxGridEvent& event)
{
    ModelsChoice->Enable(true);
    AddModelButton->Enable(true);
    MapByStrand->Enable(true);
}
