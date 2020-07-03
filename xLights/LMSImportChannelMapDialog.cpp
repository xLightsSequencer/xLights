/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(LMSImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/colordlg.h>

#include "LMSImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"

//(*IdInit(LMSImportChannelMapDialog)
const long LMSImportChannelMapDialog::ID_CHOICE1 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON_ADDMODEL = wxNewId();
const long LMSImportChannelMapDialog::ID_SPINCTRL1 = wxNewId();
const long LMSImportChannelMapDialog::ID_PANEL1 = wxNewId();
const long LMSImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const long LMSImportChannelMapDialog::ID_GRID1 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long LMSImportChannelMapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LMSImportChannelMapDialog,wxDialog)
	//(*EventTable(LMSImportChannelMapDialog)
	//*)
END_EVENT_TABLE()

wxColourData LMSImportChannelMapDialog::_colorData;

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

LMSImportChannelMapDialog::LMSImportChannelMapDialog(wxWindow* parent, const wxFileName &filename,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _filename = filename;

	//(*Initialize(LMSImportChannelMapDialog)
	wxButton* Button01;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText1;
	wxButton* Button02;

	Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer->AddGrowableCol(0);
	Sizer->AddGrowableRow(2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	ModelsChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(ModelsChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddModelButton = new wxButton(this, ID_BUTTON_ADDMODEL, _("Add Model For Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADDMODEL"));
	FlexGridSizer1->Add(AddModelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjustPanel = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	TimeAdjustPanel->Hide();
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(TimeAdjustPanel, wxID_ANY, _("Time Adjust (ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjustSpinCtrl = new wxSpinCtrl(TimeAdjustPanel, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, -10000, 600000, 0, _T("ID_SPINCTRL1"));
	TimeAdjustSpinCtrl->SetValue(_T("0"));
	FlexGridSizer3->Add(TimeAdjustSpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimeAdjustPanel->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(TimeAdjustPanel);
	FlexGridSizer3->SetSizeHints(TimeAdjustPanel);
	FlexGridSizer1->Add(TimeAdjustPanel, 1, wxALL|wxEXPAND, 1);
	Sizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerMap = new wxFlexGridSizer(0, 1, 0, 0);
	MapByStrand = new wxCheckBox(this, ID_CHECKBOX1, _("Map by Strand/CCR"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	MapByStrand->SetValue(false);
	SizerMap->Add(MapByStrand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChannelMapGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	ChannelMapGrid->CreateGrid(18,9);
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
	SizerMap->Add(ChannelMapGrid, 1, wxALL|wxEXPAND, 5);
	Sizer->Add(SizerMap, 0, wxEXPAND, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON3, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON4, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button01 = new wxButton(this, ID_BUTTON1, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button01, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button02 = new wxButton(this, ID_BUTTON2, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button02, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(Sizer);
	Sizer->Fit(this);
	Sizer->SetSizeHints(this);

	Connect(ID_BUTTON_ADDMODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnAddModelButtonClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnMapByStrandClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridCellChange);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_HIDDEN,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridEditorHidden);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_SHOWN,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnChannelMapGridEditorShown);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnButton_CancelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::LoadMapping);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::SaveMapping);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnResize);
	//*)

    if (_filename != "")
    {
        SetLabel(GetLabel() + " - " + _filename.GetFullName());
    }

    _dirty = false;

    SetEscapeId(Button_Cancel->GetId());

    SetSize(1200, 800);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsImportDialogPosition", sz, loc);
    if (loc.x != -1)
    {
        if (sz.GetWidth() < 400) sz.SetWidth(400);
        if (sz.GetHeight() < 300) sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);
}

LMSImportChannelMapDialog::~LMSImportChannelMapDialog()
{
	//(*Destroy(LMSImportChannelMapDialog)
	//*)

    SaveWindowPosition("xLightsImportDialogPosition", this);
}


static wxArrayString Convert(const std::vector<std::string> arr) {
    wxArrayString ret;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        ret.push_back(*it);
    }
    return ret;
}
void LMSImportChannelMapDialog::Init(bool allModels) {
    allowAddModels = allModels;
    if (allModels) {
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
            ModelsChoice->Append(it->first);
        }
    } else {
        for (size_t i=0;i<mSequenceElements->GetElementCount();i++) {
            if (mSequenceElements->GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelsChoice->Append(mSequenceElements->GetElement(i)->GetName());
            }
        }
    }
    int sz = ChannelMapGrid->GetColSize(3);
    ChannelMapGrid->DeleteCols(5, 4);

    wxGridCellAttr *ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(0, ca);
    ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(1, ca);
    ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(2, ca);
    ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(4, ca);

    ChannelMapGrid->SetColSize(0, sz * 1.5);
    ChannelMapGrid->SetColSize(1, sz * 1.5);
    ChannelMapGrid->SetColSize(2, sz * 1.5);
    ChannelMapGrid->SetColSize(3, sz * 4.5);
    ChannelMapGrid->SetColSize(4, sz / 2);

    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    ChannelMapGrid->SetRowLabelSize(0);
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(Convert(MapByStrand->GetValue() ? ccrNames : channelNames));
    ChannelMapGrid->SetDefaultEditor(editor);
}

void LMSImportChannelMapDialog::SetupByNode() {
    MapByStrand->SetValue(false);
    if (ChannelMapGrid->GetNumberRows()) {
        ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    }
    for (size_t x = 0; x < modelNames.size(); x++) {
        Model* m = xlights->GetModel(modelNames[x]);
        if (m != nullptr)
        {
            AddModel(*m);
        }
    }
}
void LMSImportChannelMapDialog::SetupByStrand() {
    MapByStrand->SetValue(true);
    if (ChannelMapGrid->GetNumberRows()) {
        ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    }
    for (size_t x = 0; x < modelNames.size(); x++) {
        Model* m = xlights->GetModel(modelNames[x]);
        if (m != nullptr)
        {
            AddModel(*m);
        }
    }
}


void LMSImportChannelMapDialog::OnMapByStrandClick(wxCommandEvent& event)
{
    ChannelMapGrid->BeginBatch();
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(Convert(MapByStrand->GetValue() ? ccrNames : channelNames));
    ChannelMapGrid->SetDefaultEditor(editor);
    if (MapByStrand->GetValue()) {
        SetupByStrand();
    } else {
        SetupByNode();
    }
    ChannelMapGrid->EndBatch();
}
void LMSImportChannelMapDialog::AddModel(Model &cls) {
    ChannelMapGrid->BeginBatch();
    int i = ChannelMapGrid->GetNumberRows();
    ChannelMapGrid->AppendRows(cls.GetNumStrands() + 1);
    ChannelMapGrid->SetCellValue(i, 0, cls.name);
    i++;
    if (!MapByStrand->GetValue() && cls.GetNumSubModels() > 0) {
        ChannelMapGrid->AppendRows(cls.GetNumSubModels());
        for (int s = 0; s < cls.GetNumSubModels(); s++) {
            ChannelMapGrid->SetCellValue(i, 0, cls.name);
            Model *sm = cls.GetSubModel(s);
            ChannelMapGrid->SetCellValue(i, 1, sm->GetName());
            i++;
        }
    }
    for (int s = 0; s < cls.GetNumStrands(); s++) {
        ChannelMapGrid->SetCellValue(i, 0, cls.name);
        wxString sn = cls.GetStrandName(s);
        if ("" == sn) {
            sn = wxString::Format("Strand %d", s + 1);
        }
        ChannelMapGrid->SetCellValue(i, 1, sn);
        i++;
        if (!MapByStrand->GetValue()) {
            ChannelMapGrid->AppendRows(cls.GetStrandLength(s));
            for (int n = 0; n < cls.GetStrandLength(s); n++) {
                ChannelMapGrid->SetCellValue(i, 0, cls.name);
                ChannelMapGrid->SetCellValue(i, 1, sn);
                wxString nn = cls.GetNodeName(cls.MapToNodeIndex(s, n));
                if ("" == nn) {
                    nn = wxString::Format("Node %d", n + 1);
                }
                ChannelMapGrid->SetCellValue(i, 2, nn);
                i++;
            }
        }
    }
    ChannelMapGrid->EndBatch();
}

void LMSImportChannelMapDialog::OnAddModelButtonClick(wxCommandEvent& event)
{
    std::string name = ModelsChoice->GetStringSelection().ToStdString();
    if (name == "") {
        return;
    }
    ModelsChoice->Delete(ModelsChoice->GetSelection());
    Element * model = nullptr;
    for (size_t i=0;i<mSequenceElements->GetElementCount();i++) {
        if (mSequenceElements->GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_MODEL
            && name == mSequenceElements->GetElement(i)->GetName()) {
            model = mSequenceElements->GetElement(i);
        }
    }
    if (model == nullptr && allowAddModels) {
        model = mSequenceElements->AddElement(name, "model", false, false, false, false);
        model->AddEffectLayer();
    }
    if (model == nullptr) {
        return;
    }
    modelNames.push_back(name);
    Model *cls = xlights->GetModel(name);
    if (cls != nullptr)
    {
        AddModel(*cls);
    }
    Refresh();
    _dirty = true;
}

void LMSImportChannelMapDialog::OnChannelMapGridCellChange(wxGridEvent& event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    std::string s = ChannelMapGrid->GetCellValue(row, col).ToStdString();
    ChannelMapGrid->SetCellBackgroundColour(row, 4, channelColors[s].asWxColor());
    MapByStrand->Enable(false);
    ChannelMapGrid->Refresh();
    _dirty = true;
}

void LMSImportChannelMapDialog::OnChannelMapGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == 4) {
        wxColor c = ChannelMapGrid->GetCellBackgroundColour(event.GetRow(), 4);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK)
        {
            _colorData = dlg.GetColourData();
            ChannelMapGrid->SetCellBackgroundColour(event.GetRow(), 4, dlg.GetColourData().GetColour());
            ChannelMapGrid->Refresh();
            _dirty = true;
        }
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

wxString FindTab(wxString &line) {
    for (size_t x = 0; x < line.size(); x++) {
        if (line[x] == '\t') {
            wxString first = line.SubString(0, x - 1);
            line = line.SubString(x+1, line.size());
            return first;
        }
    }
    return line;
}

void LMSImportChannelMapDialog::LoadMapping(wxCommandEvent& event)
{
    bool strandwarning = false;
    bool modelwarning = false;
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
    }

    wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        for (size_t x = 0; x <  modelNames.size(); x++) {
            ModelsChoice->Append(modelNames[x]);
        }
        ChannelMapGrid->BeginBatch();
        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");
        MapByStrand->SetValue("true" == text.ReadLine());
        int count = wxAtoi(text.ReadLine());
        modelNames.clear();
        for (int x = 0; x < count; x++) {
            std::string mn = text.ReadLine().ToStdString();
            int idx = ModelsChoice->FindString(mn);
            if (idx == wxNOT_FOUND) {
                //wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model.", "", wxICON_WARNING | wxOK , this);
                if (!modelwarning)
                {
                    if (wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model. Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                    {
                        modelwarning = true;
                    }
                }
            } else {
                ModelsChoice->Delete(idx);
                modelNames.push_back(mn);
            }
        }
        if (MapByStrand->GetValue()) {
            SetupByStrand();
        } else {
            SetupByNode();
        }
        wxString line = text.ReadLine();
        int r = 0;
        while (line != "") {

            wxString model = FindTab(line);
            wxString strand = FindTab(line);
            wxString node = FindTab(line);
            wxString mapping = FindTab(line);
            xlColor color(FindTab(line));

            Element *modelEl = mSequenceElements->GetElement(model.ToStdString());
            if (modelEl == nullptr && allowAddModels) {
                modelEl = mSequenceElements->AddElement(model.ToStdString(), "model", false, false, false, false);
                modelEl->AddEffectLayer();
            }
            if (modelEl != nullptr) {
                while (model != ChannelMapGrid->GetCellValue(r, 0)
                       && r < ChannelMapGrid->GetNumberRows()) {
                    r++;
                }

                if (model != ChannelMapGrid->GetCellValue(r, 0)
                    || strand != ChannelMapGrid->GetCellValue(r, 1)
                    || node !=  ChannelMapGrid->GetCellValue(r, 2)) {
                    if (!strandwarning)
                    {
                        if (wxMessageBox(model + "/" + strand + "/" + node + " not found.  Has the models changed? Do you want to see future occurences of this error during this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO)
                        {
                            strandwarning = true;
                        }
                    }
                } else {
                    ChannelMapGrid->SetCellValue(r, 3, mapping);
                    ChannelMapGrid->SetCellBackgroundColour(r, 4, color.asWxColor());
                }
                r++;
            }
            line = text.ReadLine();
        }
        ChannelMapGrid->EndBatch();
        _dirty = false;
    }
}

void LMSImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, "mapping", "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString(MapByStrand->GetValue()?"true\n":"false\n");
        text.WriteString(wxString::Format("%d\n", modelNames.size()));
        for (size_t x = 0; x <  modelNames.size(); x++) {
            text.WriteString(modelNames[x] + "\n");
        }
        for (int x = 0; x < ChannelMapGrid->GetNumberRows(); x++) {
            wxColor wxc = ChannelMapGrid->GetCellBackgroundColour(x, 4);
            xlColor c(wxc);
            text.WriteString(ChannelMapGrid->GetCellValue(x, 0)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 1)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 2)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 3)
                             + "\t" + c + "\n");
        }
        _dirty = false;
    }
}

void LMSImportChannelMapDialog::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()-15);
    s.SetHeight(s.GetHeight()-75);

    wxSize s1 = FlexGridSizer1->GetSize();
    wxSize s2 = FlexGridSizer2->GetSize();

    s.SetHeight(s.GetHeight()-s1.GetHeight()-s2.GetHeight());

    ChannelMapGrid->SetSize(s);
    ChannelMapGrid->SetMinSize(s);
    ChannelMapGrid->SetMaxSize(s);

    ChannelMapGrid->FitInside();
    ChannelMapGrid->Refresh();
    Layout();
}

void LMSImportChannelMapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_OK);
        }

    }
    else
    {
        EndDialog(wxID_OK);
    }
}

void LMSImportChannelMapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_CANCEL);
        }

    }
    else
    {
        EndDialog(wxID_CANCEL);
    }
}
