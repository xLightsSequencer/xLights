#include "xLightsImportChannelMapDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

//(*InternalHeaders(xLightsImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(xLightsImportChannelMapDialog)
const long xLightsImportChannelMapDialog::ID_CHOICE1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON_ADDMODEL = wxNewId();
const long xLightsImportChannelMapDialog::ID_GRID1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON3 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON4 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON1 = wxNewId();
const long xLightsImportChannelMapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(xLightsImportChannelMapDialog,wxDialog)
	//(*EventTable(xLightsImportChannelMapDialog)
	//*)
END_EVENT_TABLE()


#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

xLightsImportChannelMapDialog::xLightsImportChannelMapDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(xLightsImportChannelMapDialog)
	wxButton* Button01;
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
	Sizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerMap = new wxFlexGridSizer(0, 1, 0, 0);
	ChannelMapGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	ChannelMapGrid->CreateGrid(18,2);
	ChannelMapGrid->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,440)));
	ChannelMapGrid->EnableEditing(true);
	ChannelMapGrid->EnableGridLines(true);
	ChannelMapGrid->SetColLabelValue(0, _("Model"));
	ChannelMapGrid->SetColLabelValue(1, _("Map To"));
	ChannelMapGrid->SetDefaultCellFont( ChannelMapGrid->GetFont() );
	ChannelMapGrid->SetDefaultCellTextColour( ChannelMapGrid->GetForegroundColour() );
	SizerMap->Add(ChannelMapGrid, 1, wxALL|wxEXPAND, 5);
	Sizer->Add(SizerMap, 0, wxEXPAND, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer2->AddGrowableCol(2);
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

	Connect(ID_BUTTON_ADDMODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnAddModelButtonClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnChannelMapGridCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnChannelMapGridCellChange);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_HIDDEN,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnChannelMapGridEditorHidden);
	Connect(ID_GRID1,wxEVT_GRID_EDITOR_SHOWN,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnChannelMapGridEditorShown);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnButton_CancelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::LoadMapping);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsImportChannelMapDialog::SaveMapping);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsImportChannelMapDialog::OnResize);
	//*)

    _dirty = false;
}

xLightsImportChannelMapDialog::~xLightsImportChannelMapDialog()
{
	//(*Destroy(xLightsImportChannelMapDialog)
	//*)
}

static wxArrayString Convert(const std::vector<std::string> arr) {
    wxArrayString ret;
    for (auto it = arr.begin(); it != arr.end(); it++) {
        ret.push_back(*it);
    }
    return ret;
}

bool xLightsImportChannelMapDialog::Init(bool allModels) {
    allowAddModels = allModels;
    if (allModels) {
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            ModelsChoice->Append(it->first);
        }
    } else {
        for (size_t i=0;i<mSequenceElements->GetElementCount();i++) {
            if (mSequenceElements->GetElement(i)->GetType() == "model") {
                ModelsChoice->Append(mSequenceElements->GetElement(i)->GetName());
            }
        }
    }

    if (ModelsChoice->GetCount() == 0)
    {
        wxMessageBox("No models to import to. Add some models to the rows of the effects grid.");
        return false;
    }

    wxGridCellAttr *ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(0, ca);
    ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    //ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(1, ca);

    int sz = ChannelMapGrid->GetColSize(0);
    ChannelMapGrid->SetColSize(0, sz * 2.5);
    ChannelMapGrid->SetColSize(1, sz * 2.5);

    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    ChannelMapGrid->SetRowLabelSize(0);
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(Convert(channelNames));
    ChannelMapGrid->SetDefaultEditor(editor);
    return true;
}

void xLightsImportChannelMapDialog::AddModel(Model &cls) {
    ChannelMapGrid->BeginBatch();
    int i = ChannelMapGrid->GetNumberRows();
    ChannelMapGrid->AppendRows(1);
    ChannelMapGrid->SetCellValue(i, 0, cls.name);
    i++;
    ChannelMapGrid->EndBatch();
}

void xLightsImportChannelMapDialog::OnAddModelButtonClick(wxCommandEvent& event)
{
    std::string name = ModelsChoice->GetStringSelection().ToStdString();
    if (name == "") {
        return;
    }
    ModelsChoice->Delete(ModelsChoice->GetSelection());
    Element * model = nullptr;
    for (size_t i=0;i<mSequenceElements->GetElementCount();i++) {
        if (mSequenceElements->GetElement(i)->GetType() == "model"
            && name == mSequenceElements->GetElement(i)->GetName()) {
            model = mSequenceElements->GetElement(i);
            break;
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
    AddModel(*cls);
    Refresh();
}

void xLightsImportChannelMapDialog::OnChannelMapGridCellChange(wxGridEvent& event)
{
    _dirty = true;
    int row = event.GetRow();
    int col = event.GetCol();
    std::string s = ChannelMapGrid->GetCellValue(row, col).ToStdString();
    ChannelMapGrid->Refresh();
}

void xLightsImportChannelMapDialog::OnChannelMapGridEditorShown(wxGridEvent& event)
{
    ModelsChoice->Enable(false);
    AddModelButton->Enable(false);
}

void xLightsImportChannelMapDialog::OnChannelMapGridEditorHidden(wxGridEvent& event)
{
    ModelsChoice->Enable(true);
    AddModelButton->Enable(true);
}

wxString xLightsImportChannelMapDialog::FindTab(wxString &line) {
    for (size_t x = 0; x < line.size(); x++) {
        if (line[x] == '\t') {
            wxString first = line.SubString(0, x - 1);
            line = line.SubString(x+1, line.size());
            return first;
        }
    }
    return line;
}
void xLightsImportChannelMapDialog::LoadMapping(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
    }

    wxFileDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        for (size_t x = 0; x <  modelNames.size(); x++) {
            ModelsChoice->Append(modelNames[x]);
        }
        ChannelMapGrid->BeginBatch();
        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");
        text.ReadLine(); // map by strand ... ignore this
        int count = wxAtoi(text.ReadLine());
        modelNames.clear();
        for (int x = 0; x < count; x++) {
            std::string mn = text.ReadLine().ToStdString();
            int idx = ModelsChoice->FindString(mn);
            if (idx == wxNOT_FOUND) {
                wxMessageBox("Model " + mn + " not part of sequence.  Not mapping channels to this model.", "", wxICON_WARNING | wxOK , this);
            } else {
                ModelsChoice->Delete(idx);
                modelNames.push_back(mn);
            }
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
                    wxMessageBox(model + "/"+strand+"/"+node+ " not found.  Has the models changed?", "", wxICON_WARNING | wxOK , this);
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

void xLightsImportChannelMapDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString("false\n");
        text.WriteString(wxString::Format("%d\n", modelNames.size()));
        for (size_t x = 0; x <  modelNames.size(); x++) {
            text.WriteString(modelNames[x] + "\n");
        }
        for (int x = 0; x < ChannelMapGrid->GetNumberRows(); x++) {
            wxColor wxc = ChannelMapGrid->GetCellBackgroundColour(x, 4);
            xlColor c(wxc);
            text.WriteString(ChannelMapGrid->GetCellValue(x, 0)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 1)
                             + "\n");
        }
        _dirty = false;
    }
}

void xLightsImportChannelMapDialog::OnResize(wxSizeEvent& event)
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

void xLightsImportChannelMapDialog::OnChannelMapGridCellLeftClick(wxGridEvent& event)
{
    //ChannelMapGrid->SetCellEditor(event.GetRow(), event.GetCol(), ChannelMapGrid->GetDefaultEditor());
    ChannelMapGrid->SetGridCursor(event.GetRow(), event.GetCol());
    event.Skip();
}

void xLightsImportChannelMapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_OK);
        }

    }
    else
    {
            EndDialog(wxID_OK);
    }
}

void xLightsImportChannelMapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you want to lose your changes?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxYES)
        {
            EndDialog(wxID_CANCEL);
        }

    }
    else
    {
            EndDialog(wxID_CANCEL);
    }
}
