#include "MhFeatureDialog.h"
#include "MhFeature.h"

//(*InternalHeaders(MhFeatureDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MhFeatureDialog)
const long MhFeatureDialog::ID_GRID1 = wxNewId();
const long MhFeatureDialog::ID_BUTTON_AddFeature = wxNewId();
const long MhFeatureDialog::ID_BUTTON_DeleteFeature = wxNewId();
//*)

BEGIN_EVENT_TABLE(MhFeatureDialog,wxDialog)
	//(*EventTable(MhFeatureDialog)
	//*)
END_EVENT_TABLE()

MhFeatureDialog::MhFeatureDialog(std::vector<std::unique_ptr<MhFeature>>& _features, wxXmlNode* _node_xml, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
: node_xml(_node_xml), features(_features)
{
    //(*Initialize(MhFeatureDialog)
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizerMain = new wxFlexGridSizer(0, 1, 0, 0);
    Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
    Grid1->CreateGrid(0,1);
    Grid1->EnableEditing(true);
    Grid1->EnableGridLines(true);
    Grid1->SetDefaultColSize(200, true);
    Grid1->SetColLabelValue(0, _("Feature"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizerMain->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
    Button_AddFeature = new wxButton(this, ID_BUTTON_AddFeature, _("Add Feature"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_AddFeature"));
    FlexGridSizer2->Add(Button_AddFeature, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_DeleteFeature = new wxButton(this, ID_BUTTON_DeleteFeature, _("Delete Feature"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DeleteFeature"));
    FlexGridSizer2->Add(Button_DeleteFeature, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerMain->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizerMain->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizerMain);
    FlexGridSizerMain->Fit(this);
    FlexGridSizerMain->SetSizeHints(this);

    Connect(ID_BUTTON_AddFeature,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MhFeatureDialog::OnButton_AddFeatureClick);
    Connect(ID_BUTTON_DeleteFeature,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MhFeatureDialog::OnButton_DeleteFeatureClick);
    //*)
    
    for (auto it = features.begin(); it != features.end(); ++it) {
        int num_rows {Grid1->GetNumberRows()};
        Grid1->AppendRows(1);
        Grid1->SetCellValue(num_rows, 0, (*it)->GetBaseName());
    }

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
                if( (*it)->GetBaseName() == new_feature ) {
                    wxMessageBox(wxString::Format("Feature name %s already exists", new_feature), _("ERROR"));
                    return;
                }
            }

            int num_rows {Grid1->GetNumberRows()};
            Grid1->AppendRows(1);
            Grid1->SetCellValue(num_rows, 0, new_feature);
            
            std::string node_name = "MhFeature_" + new_feature;
            wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
            node_xml->AddChild(new_node);
            std::unique_ptr<MhFeature> newFeature(new MhFeature(new_node, new_feature));
            features.push_back(std::move(newFeature));
        }
        this->Fit();
    }
}

void MhFeatureDialog::OnButton_DeleteFeatureClick(wxCommandEvent& event)
{
    wxArrayInt rows = Grid1->GetSelectedRows();
    if( rows.size() == 0 ) return;
    std::string selected_feature = Grid1->GetCellValue(rows[0], 0);
    wxString question = wxString::Format("Are you sure you want to delete Feature %s?", selected_feature);
    if (wxMessageBox(question, "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
    {
        return;
    }
    Grid1->DeleteRows(rows[0]);
    for (auto it = features.begin(); it != features.end(); ++it) {
        std::string feature_name = "MhFeature_" + selected_feature;
        if( (*it)->GetBaseName() == feature_name ) {
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
}
