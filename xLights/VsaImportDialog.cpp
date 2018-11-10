//(*InternalHeaders(VsaImportDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>

#include "VsaImportDialog.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "VSAFile.h"
#include "support/FastComboEditor.h"
#include "support/GridCellChoiceRenderer.h"
#include "support/EzGrid.h"
#include "UtilFunctions.h"

//(*IdInit(VsaImportDialog)
const long VsaImportDialog::ID_STATICTEXT39 = wxNewId();
const long VsaImportDialog::ID_GRID1 = wxNewId();
const long VsaImportDialog::ID_STATICTEXT1 = wxNewId();
const long VsaImportDialog::ID_CHOICE_MODELS = wxNewId();
const long VsaImportDialog::ID_BUTTON3 = wxNewId();
const long VsaImportDialog::ID_BUTTON4 = wxNewId();
const long VsaImportDialog::ID_BUTTON1 = wxNewId();
const long VsaImportDialog::ID_BUTTON2 = wxNewId();
//*)

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

BEGIN_EVENT_TABLE(VsaImportDialog,wxDialog)
	//(*EventTable(VsaImportDialog)
	//*)
END_EVENT_TABLE()

static wxArrayString Convert(const std::vector<std::string> arr) {
    wxArrayString ret;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        ret.push_back(*it);
    }
    return ret;
}

VsaImportDialog::VsaImportDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
: allowAddModels(false), _dirty(false), _file(nullptr), _num_tracks(0)
{
	//(*Initialize(VsaImportDialog)
	wxButton* Button01;
	wxButton* Button02;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText40 = new wxStaticText(this, ID_STATICTEXT39, _("Assign VSA Tracks to xLights Models/Layers"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
	wxFont StaticText40Font(14,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText40->SetFont(StaticText40Font);
	GridBagSizer1->Add(StaticText40, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(GridBagSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SizerMap = new wxFlexGridSizer(0, 1, 0, 0);
	ChannelMapGrid = new EzGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	ChannelMapGrid->CreateGrid(18,4);
	ChannelMapGrid->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,440)));
	ChannelMapGrid->EnableEditing(true);
	ChannelMapGrid->EnableGridLines(true);
	ChannelMapGrid->SetColLabelValue(0, _("Track"));
	ChannelMapGrid->SetColLabelValue(1, _("Model"));
	ChannelMapGrid->SetColLabelValue(2, _("Channel"));
	ChannelMapGrid->SetColLabelValue(3, _("Layer"));
	ChannelMapGrid->SetDefaultCellFont( ChannelMapGrid->GetFont() );
	ChannelMapGrid->SetDefaultCellTextColour( ChannelMapGrid->GetForegroundColour() );
	SizerMap->Add(ChannelMapGrid, 1, wxALL, 5);
	FlexGridSizer1->Add(SizerMap, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Set All Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceSetModels = new wxChoice(this, ID_CHOICE_MODELS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_MODELS"));
	FlexGridSizer3->Add(ChoiceSetModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
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
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&VsaImportDialog::OnChannelMapGridCellChange);
	Connect(ID_CHOICE_MODELS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VsaImportDialog::OnChoiceSetModelsSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VsaImportDialog::OnButton_OkClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VsaImportDialog::OnButton_CancelClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VsaImportDialog::LoadMapping);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VsaImportDialog::SaveMapping);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&VsaImportDialog::OnResize);
	//*)

    SetEscapeId(Button_Cancel->GetId());
}

VsaImportDialog::~VsaImportDialog()
{
	//(*Destroy(VsaImportDialog)
	//*)
}

void VsaImportDialog::Init(VSAFile* file, bool allModels) {
    _file = file;
    const std::vector< VSAFile::vsaTrackRecord > &tracks = _file->GetTrackInfo();

    ChannelMapGrid->DisableDragRowSize();
    int sz = ChannelMapGrid->GetColSize(0);

    wxGridCellAttr *ca = new wxGridCellAttr();
    ca->SetKind(wxGridCellAttr::wxAttrKind::Col);
    ca->SetReadOnly();
    ChannelMapGrid->SetColAttr(0, ca);

    ChannelMapGrid->SetColSize(0, sz * 1.5);
    ChannelMapGrid->SetColSize(1, sz * 2);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());

    allowAddModels = allModels;

    modelNames.push_back("");
    if (allModels) {
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); ++it) {
            modelNames.push_back(it->first);
        }
    } else {
        for (size_t i=0;i<mSequenceElements->GetElementCount();i++) {
            if (mSequenceElements->GetElement(i)->GetType() == ELEMENT_TYPE_MODEL) {
                modelNames.push_back(mSequenceElements->GetElement(i)->GetName());
            }
        }
    }

    for (size_t x = 0; x <  modelNames.size(); x++) {
        ChoiceSetModels->Append(modelNames[x]);
    }

    // populate the grid data
    _num_tracks = 0;
    for( size_t i = 0; i < tracks.size(); ++i ) {
        if( tracks[i].enable ) {
            ChannelMapGrid->AppendRows(1);
            ChannelMapGrid->SetCellValue(_num_tracks, 0, tracks[i].name);
            ChannelMapGrid->SetCellValue(_num_tracks, 3, wxString::Format("%d", (int)_num_tracks));
            ChannelMapGrid->SetCellRenderer(_num_tracks, 1, new wxGridCellChoiceRenderer);
            ChannelMapGrid->SetCellEditor(_num_tracks, 1, new wxFastComboEditor(Convert(modelNames)));
            ChannelMapGrid->SetCellValue(_num_tracks, 1, modelNames[0]);
            ChannelMapGrid->SetCellRenderer(_num_tracks, 2, new wxGridCellChoiceRenderer);
            trackNames.push_back(tracks[i].name);
            trackIndex.push_back(i);
            selectedModels.push_back(modelNames[0]);
            selectedLayers.push_back(_num_tracks);
            selectedChannels.push_back("");
            UpdateChannels(_num_tracks);
            _num_tracks++;
        }
    }

    // enlarge dialog width
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()*1.3);
    SetSize(s);
    Layout();
}

void VsaImportDialog::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    s.SetWidth(s.GetWidth()-15);
    s.SetHeight(s.GetHeight()-85);

    wxSize s1 = GridBagSizer1->GetSize();
    wxSize s2 = FlexGridSizer2->GetSize();
    wxSize s3 = FlexGridSizer3->GetSize();

    s.SetHeight(s.GetHeight()-s1.GetHeight()-s2.GetHeight()-s3.GetHeight());

    ChannelMapGrid->SetSize(s);
    ChannelMapGrid->SetMinSize(s);
    ChannelMapGrid->SetMaxSize(s);

    ChannelMapGrid->FitInside();
    ChannelMapGrid->Refresh();
    Layout();
}

extern wxString FindTab(wxString &line);  // defined in LMSImportChannelMappingDialog.cpp

void VsaImportDialog::LoadMapping(wxCommandEvent& event)
{
    if (_dirty)
    {
        if (wxMessageBox("Are you sure you dont want to save your changes for future imports?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
    }

    wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        ChannelMapGrid->BeginBatch();
        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");
        int count = wxAtoi(text.ReadLine());
        if( count != _num_tracks ) {
            if (wxMessageBox("Number of tracks in mapping file does not match number of tracks imported. Do you want to continue with this import?", "", wxICON_WARNING | wxYES_NO, this) == wxNO) {
                return;
            }
        }
        for( size_t i = 0; i < _num_tracks; ++i ) {
            selectedModels[i] = modelNames[0];
            selectedLayers[i] = i;
            selectedChannels[i] = "";
            UpdateChannels(i);
        }

        wxString line = text.ReadLine();
        while (line != "") {
            wxString track = FindTab(line);
            wxString model = FindTab(line);
            wxString channel = FindTab(line);
            wxString layer = FindTab(line);

            int row = -1;
            for( int x = 0; x < trackNames.size(); ++x ) {
                if( trackNames[x] == track ) {
                    row = x;
                    break;
                }
            }
            if( row != -1 ) {
                if (std::find(modelNames.begin(), modelNames.end(), model) != modelNames.end())
                {
                    ChannelMapGrid->SetCellValue(row, 1, model);
                } else {
                    DisplayWarning("Model: " + model + "not found!", this);
                    ChannelMapGrid->SetCellValue(row, 1, "");
                }
                UpdateChannels(row, channel);
                ChannelMapGrid->SetCellValue(row, 3, layer);
                selectedModels[row] = ChannelMapGrid->GetCellValue(row, 1).ToStdString();
                selectedChannels[row] = ChannelMapGrid->GetCellValue(row, 2).ToStdString();
                selectedLayers[row] = wxAtoi(ChannelMapGrid->GetCellValue(row, 3));
            } else {
                DisplayWarning("Track: " + track + "not found!", this);
            }

            line = text.ReadLine();
        }
        ChannelMapGrid->EndBatch();
        _dirty = false;
    }
}

void VsaImportDialog::SaveMapping(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, "mapping", "Mapping Files (*.xmap)|*.xmap|All Files (*.)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        text.WriteString(wxString::Format("%d\n", trackNames.size()));
        for (int x = 0; x < ChannelMapGrid->GetNumberRows(); x++) {
            text.WriteString(ChannelMapGrid->GetCellValue(x, 0)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 1)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 2)
                             + "\t" + ChannelMapGrid->GetCellValue(x, 3) + "\n");
        }
        _dirty = false;
    }
}

void VsaImportDialog::OnButton_OkClick(wxCommandEvent& event)
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

void VsaImportDialog::OnButton_CancelClick(wxCommandEvent& event)
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

void VsaImportDialog::OnChoiceSetModelsSelect(wxCommandEvent& event)
{
    ChannelMapGrid->BeginBatch();
    wxString selection = event.GetString();
    for( size_t i = 0; i < ChannelMapGrid->GetNumberRows(); ++i ) {
        ChannelMapGrid->SetCellValue(i, 1, selection);
        selectedModels[i] = selection;
        UpdateChannels(i);
    }
    ChannelMapGrid->EndBatch();
    _dirty = true;
}

void VsaImportDialog::OnChannelMapGridCellChange(wxGridEvent& event)
{
    int row = event.GetRow();
    int col = event.GetCol();
    ChannelMapGrid->BeginBatch();
    if( col == 1 ) {
        selectedModels[row] = ChannelMapGrid->GetCellValue(row, col);
        UpdateChannels(row);
    } else if( col == 2 ) {
        selectedChannels[row] = ChannelMapGrid->GetCellValue(row, col);
    } else if( col == 3 ) {
        selectedLayers[row] = wxAtoi(ChannelMapGrid->GetCellValue(row, col));
    }
    ChannelMapGrid->EndBatch();
    _dirty = true;
}

void VsaImportDialog::UpdateChannels(int row, const wxString& match_channel )
{
    Model *cls = xlights->GetModel(selectedModels[row]);
    wxArrayString nodes;
    wxString match = match_channel;
    nodes.push_back("");
    if( cls != nullptr ) {
        for (size_t x = 0; x < cls->GetNodeCount(); x++) {
            std::string name = cls->GetNodeName(x);
            if( name != "" && name[0] != '-' ) {
                nodes.push_back(name);
                if( (match_channel == "") && (name == trackNames[row]) ) {
                    match = name;
                }
            }
        }
    }
    ChannelMapGrid->SetCellEditor(row, 2, new wxFastComboEditor(nodes));
    selectedChannels[row] = match;
    ChannelMapGrid->SetCellValue(row, 2, match);
}
