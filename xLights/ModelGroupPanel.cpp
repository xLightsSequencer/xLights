#include "ModelGroupPanel.h"

//(*InternalHeaders(ModelGroupPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include "xLightsMain.h"
#include "models/ModelManager.h"
#include "models/ModelGroup.h"
#include <wx/xml/xml.h>

//(*IdInit(ModelGroupPanel)
const long ModelGroupPanel::ID_STATICTEXT5 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT6 = wxNewId();
const long ModelGroupPanel::ID_CHOICE1 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT4 = wxNewId();
const long ModelGroupPanel::ID_SPINCTRL1 = wxNewId();
const long ModelGroupPanel::ID_CHOICE_PREVIEWS = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT3 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT2 = wxNewId();
const long ModelGroupPanel::ID_LISTBOX_ADD_TO_MODEL_GROUP = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON4 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON3 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON1 = wxNewId();
const long ModelGroupPanel::ID_BITMAPBUTTON2 = wxNewId();
const long ModelGroupPanel::ID_STATICTEXT1 = wxNewId();
const long ModelGroupPanel::ID_LISTBOX_MODELS_IN_GROUP = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelGroupPanel,wxPanel)
	//(*EventTable(ModelGroupPanel)
	//*)
END_EVENT_TABLE()

ModelGroupPanel::ModelGroupPanel(wxWindow* parent,ModelManager &Models,xLightsFrame *xl,wxWindowID id,const wxPoint& pos,const wxSize& size)
:   xlights(xl), mModels(Models)
{
	//(*Initialize(ModelGroupPanel)
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText6;
	wxBitmapButton* BitmapButton2;
	wxBitmapButton* BitmapButton1;
	wxBitmapButton* BitmapButton4;
	wxFlexGridSizer* FlexGridSizer12;
	wxBitmapButton* BitmapButton3;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer11;
	wxStaticText* StaticText4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel_Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Panel_Sizer->AddGrowableCol(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Model Group Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	LabelModelGroupName = new wxStaticText(this, ID_STATICTEXT6, _("<group name>"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer6->Add(LabelModelGroupName, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	ChoicePreviews = new wxChoice(this, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	FlexGridSizer6->Add(ChoicePreviews, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer12 = new wxFlexGridSizer(2, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(0);
	FlexGridSizer12->AddGrowableCol(2);
	FlexGridSizer12->AddGrowableRow(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Add to Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer12->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models in Group:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer12->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ListBoxAddToModelGroup = new wxListBox(this, ID_LISTBOX_ADD_TO_MODEL_GROUP, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTBOX_ADD_TO_MODEL_GROUP"));
	ListBoxAddToModelGroup->SetMinSize(wxDLG_UNIT(this,wxSize(75,65)));
	FlexGridSizer12->Add(ListBoxAddToModelGroup, 3, wxALL|wxEXPAND, 2);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	BitmapButton4 = new wxBitmapButton(this, ID_BITMAPBUTTON4, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_FORWARD")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
	FlexGridSizer11->Add(BitmapButton4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton3 = new wxBitmapButton(this, ID_BITMAPBUTTON3, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_BACK")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	FlexGridSizer11->Add(BitmapButton3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(-1,7));
	FlexGridSizer11->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_UP")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	FlexGridSizer11->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton2 = new wxBitmapButton(this, ID_BITMAPBUTTON2, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_DOWN")),wxART_TOOLBAR), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	FlexGridSizer11->Add(BitmapButton2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer11->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ListBoxModelsInGroup = new wxListBox(this, ID_LISTBOX_MODELS_IN_GROUP, wxDefaultPosition, wxDefaultSize, 0, 0, wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTBOX_MODELS_IN_GROUP"));
	ListBoxModelsInGroup->SetMinSize(wxDLG_UNIT(this,wxSize(75,65)));
	FlexGridSizer12->Add(ListBoxModelsInGroup, 3, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 0);
	Panel_Sizer->Add(FlexGridSizer3, 0, wxEXPAND, 0);
	SetSizer(Panel_Sizer);
	Panel_Sizer->Fit(this);
	Panel_Sizer->SetSizeHints(this);

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnChoiceModelLayoutTypeSelect);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelGroupPanel::OnChoicePreviewsSelect);
	Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonAddToModelGroupClick);
	Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonRemoveFromModelGroupClick);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonUpClick);
	Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelGroupPanel::OnButtonDownClick);
	//*)

    ChoicePreviews->Append("Default");
    ChoicePreviews->Append("All Previews");
    ChoicePreviews->Append("Unassigned");
}

ModelGroupPanel::~ModelGroupPanel()
{
	//(*Destroy(ModelGroupPanel)
	//*)
}

void ModelGroupPanel::AddPreviewChoice(const std::string name)
{
    ChoicePreviews->Append(name);
}

void ModelGroupPanel::UpdatePanel(const std::string group)
{
    mGroup = group;
    LabelModelGroupName->SetLabel(group);
    ListBoxModelsInGroup->Clear();
    ListBoxAddToModelGroup->Clear();
    ChoiceModelLayoutType->SetSelection(1);
    ModelGroup *g = (ModelGroup*)mModels[group];
    wxXmlNode *e = g->GetModelXml();
    for (auto it = g->ModelNames().begin(); it != g->ModelNames().end(); it++) {
        ListBoxModelsInGroup->Append(*it);
    }
    for (auto it = mModels.begin(); it != mModels.end(); it++) {

        if (it->second->GetDisplayAs() != "ModelGroup"
            && std::find(g->ModelNames().begin(), g->ModelNames().end(), it->first) == g->ModelNames().end()) {
             ListBoxAddToModelGroup->Append(it->first);
        }
    }

    wxString v = e->GetAttribute("layout", "grid");
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

    wxString preview = e->GetAttribute("LayoutGroup", "Default");
    ChoicePreviews->SetSelection(0);
    for( int i = 0; i < ChoicePreviews->GetCount(); i++ ) {
        if( ChoicePreviews->GetString(i) == preview )
        {
            ChoicePreviews->SetSelection(i);
        }
    }

    wxCommandEvent evt;
    OnChoiceModelLayoutTypeSelect(evt);

    SizeSpinCtrl->SetValue(wxAtoi(e->GetAttribute("GridSize", "400")));

	Panel_Sizer->Fit(this);
	Panel_Sizer->SetSizeHints(this);
}

void ModelGroupPanel::OnChoiceModelLayoutTypeSelect(wxCommandEvent& event)
{
}

void ModelGroupPanel::OnButtonAddToModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxAddToModelGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        ListBoxModelsInGroup->Append(ListBoxAddToModelGroup->GetString(selectedIndex));
        ListBoxAddToModelGroup->Delete(selectedIndex);
    }
    if(selectedIndex<ListBoxAddToModelGroup->GetCount())
    {
        ListBoxAddToModelGroup->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        ListBoxAddToModelGroup->SetSelection(ListBoxAddToModelGroup->GetCount()-1,TRUE);
    }
    SaveGroupChanges();
}

void ModelGroupPanel::OnButtonRemoveFromModelGroupClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND)
    {
        ListBoxAddToModelGroup->Append(ListBoxModelsInGroup->GetString(selectedIndex));
        ListBoxModelsInGroup->Delete(selectedIndex);
    }
    if(selectedIndex<ListBoxModelsInGroup->GetCount())
    {
        ListBoxModelsInGroup->SetSelection(selectedIndex,TRUE);
    }
    else
    {
        ListBoxModelsInGroup->SetSelection(ListBoxModelsInGroup->GetCount()-1,TRUE);
    }
    SaveGroupChanges();
}

void ModelGroupPanel::OnButtonUpClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND && selectedIndex > 0)
    {
        wxString v = ListBoxModelsInGroup->GetString(selectedIndex);
        ListBoxModelsInGroup->Delete(selectedIndex);
        ListBoxModelsInGroup->Insert(v, selectedIndex - 1);
        ListBoxModelsInGroup->SetSelection(selectedIndex - 1);
        SaveGroupChanges();
    }
}

void ModelGroupPanel::OnButtonDownClick(wxCommandEvent& event)
{
    int selectedIndex = ListBoxModelsInGroup->GetSelection();
    if(selectedIndex !=  wxNOT_FOUND && selectedIndex < (ListBoxModelsInGroup->GetCount() - 1))
    {
        wxString v = ListBoxModelsInGroup->GetString(selectedIndex);
        ListBoxModelsInGroup->Delete(selectedIndex);
        ListBoxModelsInGroup->Insert(v, selectedIndex + 1);
        ListBoxModelsInGroup->SetSelection(selectedIndex + 1);
        SaveGroupChanges();
    }
}

void ModelGroupPanel::SaveGroupChanges()
{
    ModelGroup *g = (ModelGroup*)mModels[mGroup];
    wxXmlNode *e = g->GetModelXml();

    wxString ModelsInGroup="";
    for(int i=0;i<ListBoxModelsInGroup->GetCount();i++)
    {
        if (i<ListBoxModelsInGroup->GetCount()-1)
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
    xlights->UpdateModelsList(false);
    xlights->UnsavedRgbEffectsChanges = true;
    xlights->UpdatePreview();
}

void ModelGroupPanel::OnChoicePreviewsSelect(wxCommandEvent& event)
{
    ModelGroup *g = (ModelGroup*)mModels[mGroup];
    wxXmlNode *e = g->GetModelXml();
    e->DeleteAttribute("LayoutGroup");
    e->AddAttribute("LayoutGroup", ChoicePreviews->GetString(ChoicePreviews->GetCurrentSelection()));
    xlights->UpdateModelsList(true);
    xlights->UnsavedRgbEffectsChanges = true;
    xlights->UpdatePreview();
    xlights->RefreshLayout();
}
