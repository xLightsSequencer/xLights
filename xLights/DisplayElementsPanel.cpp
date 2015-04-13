#include "DisplayElementsPanel.h"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"
#include "ModelViewSelector.h"
#include "sequencer/Element.h"


//(*InternalHeaders(DisplayElementsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(DisplayElementsPanel)
const long DisplayElementsPanel::ID_STATICTEXT4 = wxNewId();
const long DisplayElementsPanel::ID_LISTCTRL_VIEWS = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_ADD_VIEWS = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_DELETE_VIEW = wxNewId();
const long DisplayElementsPanel::ID_STATICTEXT2 = wxNewId();
const long DisplayElementsPanel::ID_LISTCTRL_MODELS = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_SHOW_ALL = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_HIDE_ALL = wxNewId();
const long DisplayElementsPanel::ID_BUTTONADD_MODELS = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_DELETE_MODELS = wxNewId();
const long DisplayElementsPanel::ID_STATICTEXT3 = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_MOVE_UP = wxNewId();
const long DisplayElementsPanel::ID_BUTTON_MOVE_DOWN = wxNewId();
//*)

BEGIN_EVENT_TABLE(DisplayElementsPanel,wxPanel)
	//(*EventTable(DisplayElementsPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_LISTITEM_CHECKED, DisplayElementsPanel::ListItemChecked)
END_EVENT_TABLE()

DisplayElementsPanel::DisplayElementsPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(DisplayElementsPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, wxEmptyString);
	FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Views:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ListCtrlViews = new wxCheckedListCtrl(this, ID_LISTCTRL_VIEWS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_VIEWS"));
	ListCtrlViews->SetMinSize(wxSize(175,125));
	FlexGridSizer6->Add(ListCtrlViews, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddViews = new wxButton(this, ID_BUTTON_ADD_VIEWS, _("Add Views"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_VIEWS"));
	ButtonAddViews->Disable();
	FlexGridSizer3->Add(ButtonAddViews, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteView = new wxButton(this, ID_BUTTON_DELETE_VIEW, _("Delete View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_VIEW"));
	ButtonDeleteView->Disable();
	FlexGridSizer3->Add(ButtonDeleteView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer11->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlModels = new wxCheckedListCtrl(this, ID_LISTCTRL_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_MODELS"));
	ListCtrlModels->SetMinSize(wxSize(175,175));
	FlexGridSizer7->Add(ListCtrlModels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonShowAll = new wxButton(this, ID_BUTTON_SHOW_ALL, _("Show All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHOW_ALL"));
	FlexGridSizer10->Add(ButtonShowAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonHideAll = new wxButton(this, ID_BUTTON_HIDE_ALL, _("Hide All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_HIDE_ALL"));
	FlexGridSizer10->Add(ButtonHideAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddModels = new wxButton(this, ID_BUTTONADD_MODELS, _("Add Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONADD_MODELS"));
	FlexGridSizer4->Add(ButtonAddModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteModels = new wxButton(this, ID_BUTTON_DELETE_MODELS, _("Delete Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_MODELS"));
	FlexGridSizer4->Add(ButtonDeleteModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Render Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveUp = new wxButton(this, ID_BUTTON_MOVE_UP, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
	FlexGridSizer4->Add(ButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveDown = new wxButton(this, ID_BUTTON_MOVE_DOWN, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_DOWN"));
	FlexGridSizer4->Add(ButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer11->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticBoxSizer1->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_VIEWS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddViewsClick);
	Connect(ID_BUTTON_SHOW_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonShowAllClick);
	Connect(ID_BUTTON_HIDE_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonHideAllClick);
	Connect(ID_BUTTONADD_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddModelsClick);
	Connect(ID_BUTTON_DELETE_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonDeleteModelsClick);
	Connect(ID_BUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveUpClick);
	Connect(ID_BUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveDownClick);
	Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&DisplayElementsPanel::OnLeftUp);
	//*)

	ListCtrlViews->SetImages((char**)eye_16,(char**)eye_16_gray);
	ListCtrlModels->SetImages((char**)eye_16,(char**)eye_16_gray);
}

DisplayElementsPanel::~DisplayElementsPanel()
{
	//(*Destroy(DisplayElementsPanel)
	//*)
}

void DisplayElementsPanel::SetSequenceElementsModelsViews(SequenceElements* elements,
                                                          wxXmlNode* models,
                                                          wxXmlNode* modelGroups,
                                                          wxXmlNode* views)
{
    mSequenceElements = elements;
    mModels = models;
    mViews = views;
    mModelGroups = modelGroups;
}

void DisplayElementsPanel::Initialize()
{
    PopulateViews();
    PopulateModels();
}

void DisplayElementsPanel::PopulateViews()
{
// Add first column
    ListCtrlViews->ClearAll();

	wxListItem col0;
	col0.SetId(0);
	col0.SetText( _("") );
	col0.SetWidth(30);
	ListCtrlViews->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( _("View") );
	col1.SetWidth(130);
	ListCtrlViews->InsertColumn(1, col1);

    int j=0;
    for(int i=0;i<mSequenceElements->GetElementCount();i++)
    {
        if(mSequenceElements->GetElement(i)->GetType()== "view")
        {
            wxListItem li;
            li.SetId(j);
            li.SetData((void*)mSequenceElements->GetElement(i));
    //        li.SetText(mSequenceElements->GetElement(i)->GetName());
            ListCtrlViews->SetItem(j,1,mSequenceElements->GetElement(i)->GetName());
            ListCtrlViews->SetChecked(j,mSequenceElements->GetElement(i)->GetVisible());
            j++;
        }
    }
}

void DisplayElementsPanel::PopulateModels()
{
// Add first column
    ListCtrlModels->ClearAll();

	wxListItem col0;
	col0.SetId(0);
	col0.SetText( _("") );
	col0.SetWidth(30);
	ListCtrlModels->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( _("Model") );
	col1.SetWidth(130);
	ListCtrlModels->InsertColumn(1, col1);

    int j=0;
    for(int i=0;i<mSequenceElements->GetElementCount();i++)
    {
        if(mSequenceElements->GetElement(i)->GetType()== "model"
           || mSequenceElements->GetElement(i)->GetType()== "modelGroup")
        {
            wxListItem li;
            li.SetId(j);
            ListCtrlModels->InsertItem(li);
            ListCtrlModels->SetItemPtrData(j,(wxUIntPtr)mSequenceElements->GetElement(i));
            ListCtrlModels->SetItem(j,1,mSequenceElements->GetElement(i)->GetName());
            ListCtrlModels->SetChecked(j,mSequenceElements->GetElement(i)->GetVisible());
            j++;
        }
    }
}

void DisplayElementsPanel::OnLeftUp(wxMouseEvent& event)
{
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

void DisplayElementsPanel::OnButtonAddViewsClick(wxCommandEvent& event)
{
    AddElements("view");
    PopulateViews();
}

void DisplayElementsPanel::OnButtonAddModelsClick(wxCommandEvent& event)
{
    AddElements("model");
    PopulateModels();
}

void DisplayElementsPanel::AddElements(wxString type)
{
    ModelViewSelector dialog(this);
    dialog.SetSequenceElementsModelsViews(mSequenceElements,mModels,mModelGroups,mViews);
    dialog.Initialize(type);
    dialog.CenterOnParent();
    int DlgResult = dialog.ShowModal();
    if(DlgResult==wxID_OK)
    {
        for(int i=0;i<dialog.ElementsToAdd.size();i++)
        {
            int index = type=="view"?mSequenceElements->GetLastViewIndex():mSequenceElements->GetElementCount();
            Element* e = mSequenceElements->AddElement(index,dialog.ElementsToAdd[i],type,true,false,false,false);
            e->AddEffectLayer();
            if(type == "view")
            {
                AddMissingModelsOfView(dialog.ElementsToAdd[i]);
            }
        }
        // Update Grid
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    }
}

void DisplayElementsPanel::AddMissingModelsOfView(wxString view)
{
    wxString modelsString = mSequenceElements->GetViewModels(view);
    if(modelsString.length()> 0)
    {
        wxArrayString modelsArr=wxSplit(modelsString,',');
        for(int i=0;i<modelsArr.size();i++)
        {
            wxString modelName =  modelsArr[i];
            if(!mSequenceElements->ElementExists(modelName))
            {
               wxString elementType = "model";
               Element* e = mSequenceElements->AddElement(modelName,elementType,false,false,false,false);
               e->AddEffectLayer();
            }
        }
    }
}

void DisplayElementsPanel::ListItemChecked(wxCommandEvent& event)
{
    Element* e = (Element*)event.GetClientData();
    e->SetVisible(!e->GetVisible());
    // Update Grid
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}


void DisplayElementsPanel::OnButtonShowAllClick(wxCommandEvent& event)
{
    mSequenceElements->SetVisibilityForAllModels(true);
    PopulateModels();
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

void DisplayElementsPanel::OnButtonHideAllClick(wxCommandEvent& event)
{
    mSequenceElements->SetVisibilityForAllModels(false);
    PopulateModels();
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

void DisplayElementsPanel::OnButtonDeleteModelsClick(wxCommandEvent& event)
{
    if( wxMessageBox("Delete all effects and layers for the selected model(s)?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES )
    {
        ListCtrlModels->Freeze();
        long itemIndex = -1;

        for (;;) {
            itemIndex = ListCtrlModels->GetNextItem(itemIndex,
                                                    wxLIST_NEXT_ALL,
                                                    wxLIST_STATE_SELECTED);

            if (itemIndex == -1) break;

            // Got a selected item so handle it
            Element* e = (Element*)ListCtrlModels->GetItemData(itemIndex);
            mSequenceElements->DeleteElement(e->GetName());
            ListCtrlModels->DeleteItem(itemIndex);
            itemIndex = -1; // reset to delete next item which may have same index
        }
        ListCtrlModels->Thaw();
        ListCtrlModels->Refresh();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    }
}

void DisplayElementsPanel::OnButtonMoveUpClick(wxCommandEvent& event)
{
    ListCtrlModels->Freeze();  // prevent list changes while we work on it
    bool items_moved = false;
    std::vector<long> selected_list;
    long itemIndex = -1;
    long firstItemIndex = -1;

    for (;;) {
        itemIndex = ListCtrlModels->GetNextItem(itemIndex,
                                                wxLIST_NEXT_ALL,
                                                wxLIST_STATE_SELECTED);
        if (itemIndex == -1) break;

        if(firstItemIndex==-1)
        {
            firstItemIndex = itemIndex;
        }
        // Got a selected item so handle it
        selected_list.push_back(itemIndex);
    }

    if( selected_list[0] != 0 )  // don't let item or group move up if top item is selected
    {
        items_moved = true;
        for( long i = 0; i < selected_list.size(); i++ )
        {
            Element* e = (Element*)ListCtrlModels->GetItemData(selected_list[i]);
            mSequenceElements->MoveElementUp(e->GetName());
        }
    }
    ListCtrlModels->Thaw();  // free up the list

    if( items_moved )
    {
        PopulateModels();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
        for( long i = 0; i < selected_list.size(); i++ )
        {
            ListCtrlModels->SetItemState(selected_list[i]-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }
    selected_list.clear();
    if(firstItemIndex!=-1 && firstItemIndex!=0)
    {
        ListCtrlModels->EnsureVisible(firstItemIndex-1);
    }
}

void DisplayElementsPanel::OnButtonMoveDownClick(wxCommandEvent& event)
{
    ListCtrlModels->Freeze();  // prevent list changes while we work on it
    bool items_moved = false;
    std::vector<long> selected_list;
    long itemIndex = -1;
    long lastItemIndex = -1;

    long num_items = ListCtrlModels->GetItemCount();

    for (;;) {
        itemIndex = ListCtrlModels->GetNextItem(itemIndex,
                                                wxLIST_NEXT_ALL,
                                                wxLIST_STATE_SELECTED);

        if (itemIndex == -1) break;

        lastItemIndex = itemIndex;


        // Got a selected item so handle it
        selected_list.push_back(itemIndex);
    }

    if( selected_list.back() < num_items-1 )  // don't let item or group move down if bottom item is selected
    {
        items_moved = true;
        for( long i = selected_list.size()-1; i >= 0; i-- )
        {
            Element* e = (Element*)ListCtrlModels->GetItemData(selected_list[i]);
            mSequenceElements->MoveElementDown(e->GetName());
        }
    }
    ListCtrlModels->Thaw();  // free up the list

    if( items_moved )
    {
        PopulateModels();
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
        for( long i = 0; i < selected_list.size(); i++ )
        {
            ListCtrlModels->SetItemState(selected_list[i]+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }
    selected_list.clear();
    if(lastItemIndex!=-1 && lastItemIndex!=ListCtrlModels->GetItemCount()-1)
    {
        ListCtrlModels->EnsureVisible(lastItemIndex+1);
    }
}
