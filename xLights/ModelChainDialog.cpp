/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelChainDialog.h"
#include <vector>

//(*InternalHeaders(ModelChainDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ModelChainDialog)
const long ModelChainDialog::ID_STATICTEXT1 = wxNewId();
const long ModelChainDialog::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelChainDialog,wxDialog)
	//(*EventTable(ModelChainDialog)
	//*)
END_EVENT_TABLE()

#include "models/ModelManager.h"
#include "models/Model.h"

ModelChainDialog::ModelChainDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelChainDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Chain after model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ModelChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ModelChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer2->Add(ModelChoice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

ModelChainDialog::~ModelChainDialog()
{
	//(*Destroy(ModelChainDialog)
	//*)
}

void ModelChainDialog::Set(Model* m, const ModelManager &models) {

    wxString chain = m->GetModelChain();
    if (chain == "") chain = "Beginning";

    if (chain.StartsWith(">"))
    {
        chain = chain.substr(1);
    }
    int port = m->GetControllerPort();
    std::string protocol = m->GetControllerProtocol();
    std::string controller = m->GetControllerName();
    auto smartRemote = m->GetSmartRemote();

    wxArrayString  list;
    list.push_back("Beginning");
    for (const auto& it : models) {
        if (it.second->GetDisplayAs() != "ModelGroup" && 
            m != it.second && 
            controller == it.second->GetControllerName() &&
            port >= it.second->GetControllerPort() &&
            port <= it.second->GetControllerPort() + it.second->GetNumPhysicalStrings() - 1 &&
            smartRemote == it.second->GetSmartRemote() &&
            protocol == it.second->GetControllerProtocol()) {
            list.push_back(it.first);
        }
    }

    ModelChoice->Append(list);
    ModelChoice->SetStringSelection(chain);
}

std::string ModelChainDialog::Get() {

    if (ModelChoice->GetStringSelection() == "" ||
        ModelChoice->GetStringSelection() == "Beginning")
    {
        return "Beginning";
    }
    else
    {
        return ">" + ModelChoice->GetStringSelection().ToStdString();
    }
}