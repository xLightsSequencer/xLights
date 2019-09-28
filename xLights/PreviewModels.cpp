#include "PreviewModels.h"

//(*InternalHeaders(PreviewModels)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/artprov.h>
//*)


//(*IdInit(PreviewModels)
const long PreviewModels::ID_CHOICE1 = wxNewId();
const long PreviewModels::ID_STATICTEXT4 = wxNewId();
const long PreviewModels::ID_SPINCTRL1 = wxNewId();
const long PreviewModels::ID_STATICTEXT3 = wxNewId();
const long PreviewModels::ID_LISTBOX_ADD_TO_MODEL_GROUP = wxNewId();
const long PreviewModels::ID_BITMAPBUTTON4 = wxNewId();
const long PreviewModels::ID_BITMAPBUTTON3 = wxNewId();
const long PreviewModels::ID_BITMAPBUTTON1 = wxNewId();
const long PreviewModels::ID_BITMAPBUTTON2 = wxNewId();
const long PreviewModels::ID_STATICTEXT1 = wxNewId();
const long PreviewModels::ID_STATICTEXT2 = wxNewId();
const long PreviewModels::ID_LISTBOX_MODELS_IN_GROUP = wxNewId();
//*)

BEGIN_EVENT_TABLE(PreviewModels,wxDialog)
	//(*EventTable(PreviewModels)
	//*)
END_EVENT_TABLE()

#include "models/ModelManager.h"
#include "models/ModelGroup.h"


PreviewModels::PreviewModels(wxWindow* parent,const std::string &group, ModelManager &Models, wxWindowID id,const wxPoint& pos,const wxSize& size) :
    mModels(Models), mGroup(group)
{


	//(*Initialize(PreviewModels)
	wxBitmapButton* BitmapButton2;
	wxFlexGridSizer* FlexGridSizer1;
	wxBitmapButton* BitmapButton3;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer3;
	wxBitmapButton* BitmapButton4;
	wxStaticText* StaticText4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer12;
	wxBitmapButton* BitmapButton1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Default Layout Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceModelLayoutType = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ChoiceModelLayoutType->SetSelection( ChoiceModelLayoutType->Append(_("Grid as per preview")) );
	ChoiceModelLayoutType->Append(_("Minimal Grid"));
	ChoiceModelLayoutType->Append(_("Horizontal Per Model"));
	ChoiceModelLayoutType->Append(_("Vertical Per Model"));
	ChoiceModelLayoutType->Append(_("Horizontal Per Model/Strand"));
	ChoiceModelLayoutType->Append(_("Vertical Per Model/Strand"));
	ChoiceModelLayoutType->Append(_("Single Line"));
	ChoiceModelLayoutType->Append(_("Overlay - Centered"));
	ChoiceModelLayoutType->Append(_("Overlay - Scaled"));
	FlexGridSizer6->Add(ChoiceModelLayoutType, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	GridSizeLabel = new wxStaticText(this, ID_STATICTEXT4, _("Max Grid Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(GridSizeLabel, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SizeSpinCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("400"), wxDefaultPosition, wxDefaultSize, 0, 10, 2000, 400, _T("ID_SPINCTRL1"));
	SizeSpinCtrl->SetValue(_T("400"));
	FlexGridSizer6->Add(SizeSpinCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer12 = new wxFlexGridSizer(1, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(0);
	FlexGridSizer12->AddGrowableCol(2);
	FlexGridSizer12->AddGrowableRow(0);
	FlexGridSizer10 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer10->AddGrowableRow(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Add to Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer10->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxAddToModelGroup = new wxListBox(this, ID_LISTBOX_ADD_TO_MODEL_GROUP, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTBOX_ADD_TO_MODEL_GROUP"));
	ListBoxAddToModelGroup->SetMinSize(wxDLG_UNIT(this,wxSize(75,65)));
	FlexGridSizer10->Add(ListBoxAddToModelGroup, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer12->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(-1,10));
	FlexGridSizer11->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton4 = new wxBitmapButton(this, ID_BITMAPBUTTON4, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_FORWARD")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
	FlexGridSizer11->Add(BitmapButton4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton3 = new wxBitmapButton(this, ID_BITMAPBUTTON3, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_BACK")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	FlexGridSizer11->Add(BitmapButton3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	wxSize __SpacerSize_2 = wxDLG_UNIT(this,wxSize(-1,7));
	FlexGridSizer11->Add(__SpacerSize_2.GetWidth(),__SpacerSize_2.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_UP")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	FlexGridSizer11->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton2 = new wxBitmapButton(this, ID_BITMAPBUTTON2, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_DOWN")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	FlexGridSizer11->Add(BitmapButton2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer11->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableRow(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models in Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer9->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListBoxModelsInGroup = new wxListBox(this, ID_LISTBOX_MODELS_IN_GROUP, wxDefaultPosition, wxDefaultSize, 0, 0, wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTBOX_MODELS_IN_GROUP"));
	ListBoxModelsInGroup->SetMinSize(wxDLG_UNIT(this,wxSize(75,65)));
	FlexGridSizer9->Add(ListBoxModelsInGroup, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer12->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer3->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 0, wxEXPAND, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PreviewModels::OnChoiceModelLayoutTypeSelect);
	Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonAddToModelGroupClick);
	Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonRemoveFromModelGroupClick);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonUpClick);
	Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PreviewModels::OnButtonDownClick);
	//*)

    SetTitle(group);

    ChoiceModelLayoutType->SetSelection(1);

    ModelGroup *g = (ModelGroup*)mModels[group];
    wxXmlNode *e = g->GetModelXml();
    for (auto it = g->ModelNames().begin(); it != g->ModelNames().end(); ++it) {
        ListBoxModelsInGroup->Append(*it);
    }
    for (auto it = mModels.begin(); it != mModels.end(); ++it) {

        if (it->second->GetDisplayAs() != "ModelGroup"
            && std::find(g->ModelNames().begin(), g->ModelNames().end(), it->first) == g->ModelNames().end()) {
             ListBoxAddToModelGroup->Append(it->first);
        }
    }

    wxString v = e->GetAttribute("layout", "minimalGrid");
    if (v == "grid") {
        ChoiceModelLayoutType->SetSelection(0);
    }else if (v == "minimalGrid") {
        ChoiceModelLayoutType->SetSelection(1);
    } else if (v == "horizontal") {
        ChoiceModelLayoutType->SetSelection(2);
    } else if (v == "vertical") {
        ChoiceModelLayoutType->SetSelection(3);
    } else {
        int idx = ChoiceModelLayoutType->FindString(v);
        if (idx >= 0) {
            ChoiceModelLayoutType->SetSelection(idx);
        } else {
            ChoiceModelLayoutType->Append(v);
            ChoiceModelLayoutType->SetSelection(ChoiceModelLayoutType->GetCount() - 1);
        }
    }

    wxCommandEvent evt;
    OnChoiceModelLayoutTypeSelect(evt);

    SizeSpinCtrl->SetValue(wxAtoi(e->GetAttribute("GridSize", "400")));

}

PreviewModels::~PreviewModels()
{
	//(*Destroy(PreviewModels)
	//*)
}

void PreviewModels::OnButtonAddToModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxAddToModelGroup->GetSelection();
    if (selectedIndex != wxNOT_FOUND)
    {
        ListBoxModelsInGroup->Append(ListBoxAddToModelGroup->GetString(selectedIndex));
        ListBoxAddToModelGroup->Delete(selectedIndex);
    }
    if (selectedIndex < (int)ListBoxAddToModelGroup->GetCount())
    {
        ListBoxAddToModelGroup->SetSelection(selectedIndex, TRUE);
    }
    else
    {
        ListBoxAddToModelGroup->SetSelection(ListBoxAddToModelGroup->GetCount() - 1, TRUE);
    }
}

void PreviewModels::OnButtonRemoveFromModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if (selectedIndex != wxNOT_FOUND)
    {
        ListBoxAddToModelGroup->Append(ListBoxModelsInGroup->GetString(selectedIndex));
        ListBoxModelsInGroup->Delete(selectedIndex);
    }
    if (selectedIndex < (int)ListBoxModelsInGroup->GetCount())
    {
        ListBoxModelsInGroup->SetSelection(selectedIndex, TRUE);
    }
    else
    {
        ListBoxModelsInGroup->SetSelection(ListBoxModelsInGroup->GetCount() - 1, TRUE);
    }
}

void PreviewModels::UpdateModelGroup()
{
    ModelGroup* g = (ModelGroup*)mModels[mGroup];
    wxXmlNode* e = g->GetModelXml();

    wxString ModelsInGroup = "";
    for (size_t i = 0; i < ListBoxModelsInGroup->GetCount(); i++)
    {
        if (i < ListBoxModelsInGroup->GetCount() - 1)
        {
            ModelsInGroup += ListBoxModelsInGroup->GetString(i) + ",";
        }
        else
        {
            ModelsInGroup += ListBoxModelsInGroup->GetString(i);
        }
    }
    e->DeleteAttribute("models");
    e->AddAttribute("models", ModelsInGroup);

    e->DeleteAttribute("GridSize");
    e->DeleteAttribute("layout");
    e->AddAttribute("GridSize", wxString::Format("%d", SizeSpinCtrl->GetValue()));
    switch (ChoiceModelLayoutType->GetSelection()) {
        case 0:
            e->AddAttribute("layout", "grid");
            break;
        case 1:
            e->AddAttribute("layout", "minimalGrid");
            break;
        case 2:
            e->AddAttribute("layout", "horizontal");
            break;
        case 3:
            e->AddAttribute("layout", "vertical");
            break;
        default:
            e->AddAttribute("layout", ChoiceModelLayoutType->GetStringSelection());
            break;
    }
    g->Reset();
}

void PreviewModels::OnButtonUpClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND && selectedIndex > 0)
    {
        wxString v = ListBoxModelsInGroup->GetString(selectedIndex);
        ListBoxModelsInGroup->Delete(selectedIndex);
        ListBoxModelsInGroup->Insert(v, selectedIndex - 1);
        ListBoxModelsInGroup->SetSelection(selectedIndex - 1);
    }
}

void PreviewModels::OnButtonDownClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND && selectedIndex < ((int)ListBoxModelsInGroup->GetCount() - 1))
    {
        wxString v = ListBoxModelsInGroup->GetString(selectedIndex);
        ListBoxModelsInGroup->Delete(selectedIndex);
        ListBoxModelsInGroup->Insert(v, selectedIndex + 1);
        ListBoxModelsInGroup->SetSelection(selectedIndex + 1);
    }
}

void PreviewModels::OnChoiceModelLayoutTypeSelect(wxCommandEvent& event)
{
}
