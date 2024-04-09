/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MhFeatureDialog.h"
#include "MhFeature.h"
#include "MhChannelDialog.h"
#include "tmGridCell.h"

#include "../../../UtilFunctions.h"

//(*InternalHeaders(MhFeatureDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MhFeatureDialog)
const long MhFeatureDialog::ID_BUTTON_AddFeature = wxNewId();
//*)

const long MhFeatureDialog::ID_GRID_FEATURES = wxNewId();

BEGIN_EVENT_TABLE(MhFeatureDialog,wxDialog)
	//(*EventTable(MhFeatureDialog)
	//*)
EVT_COMMAND(wxID_ANY, EVT_GRID_ROW_CLICKED, MhFeatureDialog::OnButton_FeatureClick)
EVT_COMMAND(wxID_ANY, EVT_NAME_CHANGE, MhFeatureDialog::OnButton_RenameFeatureClick)
END_EVENT_TABLE()

MhFeatureDialog::MhFeatureDialog(std::vector<std::unique_ptr<MhFeature>>& _features, wxXmlNode* _node_xml, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
: node_xml(_node_xml), features(_features)
{
    //(*Initialize(MhFeatureDialog)
    wxFlexGridSizer* FlexGridSizerGrid;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizerMain = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerGrid = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerMain->Add(FlexGridSizerGrid, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
    Button_AddFeature = new wxButton(this, ID_BUTTON_AddFeature, _("Add Feature"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_AddFeature"));
    FlexGridSizer2->Add(Button_AddFeature, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerMain->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizerMain->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizerMain);
    FlexGridSizerMain->Fit(this);
    FlexGridSizerMain->SetSizeHints(this);

    Connect(ID_BUTTON_AddFeature,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MhFeatureDialog::OnButton_AddFeatureClick);
    //*)

    // Setup Grid
    Grid_Features = new tmGrid(this, ID_GRID_FEATURES, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE, _T("ID_GRID_FEATURES"));
    FlexGridSizerGrid->Add(Grid_Features, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 15);
    Grid_Features->DisableDragGridSize();
    Grid_Features->DisableDragRowSize();
    Grid_Features->DisableDragColSize();
    Grid_Features->CreateGrid(0, 3);
    Grid_Features->HideRowLabels();

    Grid_Features->SetColSize(0, 200);
    Grid_Features->SetColSize(1, 60);
    Grid_Features->SetColSize(2, 23);
    Grid_Features->SetColLabelValue(0, "Features");
    Grid_Features->SetColLabelValue(1, "");
    Grid_Features->SetColLabelValue(2, "");

    for (auto it = features.begin(); it != features.end(); ++it) {
        int num_rows {Grid_Features->GetNumberRows()};
        Grid_Features->AppendRows(1);
        Grid_Features->SetCellValue(num_rows, 0, (*it)->GetName());
        // Add buttons
        wxGridCellButtonRenderer* new_renderer = new wxGridCellButtonRenderer("Config");
        Grid_Features->SetCellRenderer(num_rows, 1, new_renderer);
        new_renderer = new wxGridCellButtonRenderer("");
        Grid_Features->SetCellRenderer(num_rows, 2, new_renderer);
    }
    FlexGridSizerMain->Fit(this);
    FlexGridSizerMain->SetSizeHints(this);
}

MhFeatureDialog::~MhFeatureDialog()
{
	//(*Destroy(MhFeatureDialog)
	//*)
}

void MhFeatureDialog::OnButton_AddFeatureClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New Moving Head Feature", "Enter Feature Name:");
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal()) {
        if (dlg.GetValue().ToStdString() != "") {

            std::string new_feature = dlg.GetValue().ToStdString();

            // Protect against duplicate feature names
            for (auto it = features.begin(); it != features.end(); ++it) {
                if( (*it)->GetName() == new_feature ) {
                    wxMessageBox(wxString::Format("Feature name %s already exists", new_feature), _("ERROR"));
                    return;
                }
            }

            int num_rows {Grid_Features->GetNumberRows()};
            Grid_Features->AppendRows(1);
            Grid_Features->SetCellValue(num_rows, 0, new_feature);
            
            // Add a config button
            wxGridCellButtonRenderer* new_renderer = new wxGridCellButtonRenderer("Config");
            Grid_Features->SetCellRenderer(num_rows, 1, new_renderer);
            new_renderer = new wxGridCellButtonRenderer("");
            Grid_Features->SetCellRenderer(num_rows, 2, new_renderer);

            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "feature");
            new_node->AddAttribute("Name", new_feature);
            node_xml->AddChild(new_node);
            std::unique_ptr<MhFeature> newFeature(new MhFeature(new_node, "feature", new_feature));
            features.push_back(std::move(newFeature));
        }
        this->Fit();
    }
}

void MhFeatureDialog::OnButton_FeatureClick(wxCommandEvent& event)
{
    int row = Grid_Features->GetGridCursorRow();
    std::string selected_feature = Grid_Features->GetCellValue(row, 0);

    if( Grid_Features->GetGridCursorCol() == 2 )
    {
        wxString question = wxString::Format("Are you sure you want to delete Feature %s?", selected_feature);
        if (wxMessageBox(question, "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
            return;
        }
        Grid_Features->DeleteRows(row);
        for (auto it = features.begin(); it != features.end(); ++it) {
            if( (*it)->GetName() == selected_feature ) {
                wxXmlNode* node = (*it)->GetXmlNode();
                if( node != nullptr ) {
                    node_xml->RemoveChild(node);
                    delete node;
                }
                features.erase(it);
                break;
            }
        }
        FlexGridSizerMain->Layout();
        this->Fit();
    } else if( Grid_Features->GetGridCursorCol() == 1 ) {
        for (auto it = features.begin(); it != features.end(); ++it) {
            if( (*it)->GetName() == selected_feature ) {
                MhChannelDialog dlg(*it, this);
                dlg.SetFeatureName(selected_feature);
                if (dlg.ShowModal() == wxID_OK) {
                }
                break;
            }
        }
    }
}

void MhFeatureDialog::OnButton_RenameFeatureClick(wxCommandEvent& event)
{
    if( Grid_Features->GetGridCursorCol() == 0 )
    {
        int row = Grid_Features->GetGridCursorRow();
        std::string feature_name = wxString::Format("MhFeature%i", row);

        std::string selected_feature = Grid_Features->GetCellValue(row, 0);
        //Grid_Features->DeleteRows(row);
        for (auto it = features.begin(); it != features.end(); ++it) {
            wxXmlNode* node = (*it)->GetXmlNode();
            std::string feature_name = node->GetAttribute("name");
            if( (*it)->GetName() == feature_name ) {
                if( node != nullptr ) {
                    node->DeleteAttribute("name");
                    node->AddAttribute("name", feature_name);
                }
                break;
            }
        }
    }
}
