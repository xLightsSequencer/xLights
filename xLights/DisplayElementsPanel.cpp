#include "DisplayElementsPanel.h"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"
#include "ModelViewSelector.h"
#include "sequencer/Element.h"


//(*InternalHeaders(DisplayElementsPanel)
#include <wx/settings.h>
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
const long DisplayElementsPanel::ID_SCROLLED_Display_Elements = wxNewId();
const long DisplayElementsPanel::ID_PANEL1 = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	Panel_Sizer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	ScrolledWindowDisplayElements = new wxScrolledWindow(Panel_Sizer, ID_SCROLLED_Display_Elements, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLED_Display_Elements"));
	ScrolledWindowDisplayElements->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	StaticText4 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT4, _("Views:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ListCtrlViews = new wxCheckedListCtrl(ScrolledWindowDisplayElements, ID_LISTCTRL_VIEWS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_VIEWS"));
	ListCtrlViews->SetMinSize(wxDLG_UNIT(ScrolledWindowDisplayElements,wxSize(100,50)));
	FlexGridSizer6->Add(ListCtrlViews, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddViews = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_ADD_VIEWS, _("Add Views"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_VIEWS"));
	FlexGridSizer3->Add(ButtonAddViews, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteView = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_DELETE_VIEW, _("Delete View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_VIEW"));
	ButtonDeleteView->Disable();
	FlexGridSizer3->Add(ButtonDeleteView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(1);
	StaticText2 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT2, _("Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlModels = new wxCheckedListCtrl(ScrolledWindowDisplayElements, ID_LISTCTRL_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_MODELS"));
	ListCtrlModels->SetMinSize(wxDLG_UNIT(ScrolledWindowDisplayElements,wxSize(75,85)));
	FlexGridSizer7->Add(ListCtrlModels, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonShowAll = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_SHOW_ALL, _("Show All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHOW_ALL"));
	FlexGridSizer10->Add(ButtonShowAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonHideAll = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_HIDE_ALL, _("Hide All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_HIDE_ALL"));
	FlexGridSizer10->Add(ButtonHideAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddModels = new wxButton(ScrolledWindowDisplayElements, ID_BUTTONADD_MODELS, _("Add Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONADD_MODELS"));
	FlexGridSizer4->Add(ButtonAddModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteModels = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_DELETE_MODELS, _("Delete Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_MODELS"));
	FlexGridSizer4->Add(ButtonDeleteModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT3, _("Render Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveUp = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_MOVE_UP, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
	FlexGridSizer4->Add(ButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveDown = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_MOVE_DOWN, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_DOWN"));
	FlexGridSizer4->Add(ButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrolledWindowDisplayElements->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(ScrolledWindowDisplayElements);
	FlexGridSizer5->SetSizeHints(ScrolledWindowDisplayElements);
	FlexGridSizer2->Add(ScrolledWindowDisplayElements, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTCTRL_VIEWS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&DisplayElementsPanel::OnListCtrlViewsItemSelect);
	Connect(ID_BUTTON_ADD_VIEWS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddViewsClick);
	Connect(ID_BUTTON_SHOW_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonShowAllClick);
	Connect(ID_BUTTON_HIDE_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonHideAllClick);
	Connect(ID_BUTTONADD_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddModelsClick);
	Connect(ID_BUTTON_DELETE_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonDeleteModelsClick);
	Connect(ID_BUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveUpClick);
	Connect(ID_BUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveDownClick);
	Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&DisplayElementsPanel::OnLeftUp);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&DisplayElementsPanel::OnResize);
	//*)

	ListCtrlViews->SetImages((char**)eye_16,(char**)eye_16_gray);
	ListCtrlModels->SetImages((char**)eye_16,(char**)eye_16_gray);
	mNumViews = 0;
	mNumModels = 0;
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

void DisplayElementsPanel::AddViewToList(const wxString& viewName, bool isChecked)
{
    wxListItem li;
    li.SetId(mNumViews);
    ListCtrlViews->InsertItem(li);
    ListCtrlViews->SetItem(mNumViews,1,viewName);
    ListCtrlViews->SetChecked(mNumViews,isChecked);
    mNumViews++;
}

void DisplayElementsPanel::AddModelToList(Element* model)
{
    if( model != nullptr )
    {
        wxListItem li;
        li.SetId(mNumModels);
        ListCtrlModels->InsertItem(li);
        ListCtrlModels->SetItemPtrData(mNumModels,(wxUIntPtr)model);
        ListCtrlModels->SetItem(mNumModels,1,model->GetName());
        ListCtrlModels->SetChecked(mNumModels,model->GetVisible());
        mNumModels++;
    }
}

void DisplayElementsPanel::PopulateViews()
{
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

    mNumViews = 0;
    AddViewToList("Master View", true);
    for(wxXmlNode* view=mViews->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        wxString viewName = view->GetAttribute("name");
        bool isChecked = view->GetAttribute("selected")=="1"?true:false;
        AddViewToList(viewName, isChecked);
        if( isChecked )
        {
            ListCtrlViews->SetChecked(0,false);
        }
    }
}

void DisplayElementsPanel::PopulateModels()
{
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

    mNumModels=0;
    if( mSequenceElements->GetCurrentView() > 0 )
    {
        for(wxXmlNode* view=mViews->GetChildren(); view!=NULL; view=view->GetNext() )
        {
            wxString viewName = view->GetAttribute("name");
            bool isChecked = view->GetAttribute("selected")=="1"?true:false;
            if( isChecked )
            {
                wxString models = view->GetAttribute("models");
                mSequenceElements->AddMissingModelsToSequence(models);

                if(models.length()> 0)
                {
                    wxArrayString model=wxSplit(models,',');
                    for(int m=0;m<model.size();m++)
                    {
                        wxString modelName = model[m];
                        Element* elem = mSequenceElements->GetElement(modelName);
                        AddModelToList(elem);
                    }
                }
            }
        }
    }
    else
    {
        for(int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* elem = mSequenceElements->GetElement(i);
            AddModelToList(elem);
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
    wxTextEntryDialog dialog(this,_("Enter Name for View"),_("Create View"));
    int DlgResult=dialog.ShowModal();;
    if (DlgResult != wxID_OK) return;
    wxString viewName=dialog.GetValue();
    viewName.Trim();

    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, "view");
    new_node->AddAttribute("name", viewName);
    new_node->AddAttribute("selected", "1");
    new_node->AddAttribute("models", "");
    mViews->AddChild(new_node);

    AddViewToList(viewName, true);
    mSequenceElements->AddView(viewName);
    SelectView(viewName);
    PopulateViews();
}

void DisplayElementsPanel::OnButtonAddModelsClick(wxCommandEvent& event)
{
    ModelViewSelector dialog(this);
    dialog.SetSequenceElementsModelsViews(mSequenceElements,mModels,mModelGroups,mViews, mSequenceElements->GetCurrentView());
    wxString type = "model";
    int current_view = mSequenceElements->GetCurrentView();
    dialog.Initialize(type);
    dialog.CenterOnParent();
    int DlgResult = dialog.ShowModal();
    if(DlgResult==wxID_OK)
    {
        if( current_view == MASTER_VIEW )
        {
            for(int i=0;i<dialog.ElementsToAdd.size();i++)
            {
                Element* e = mSequenceElements->AddElement(dialog.ElementsToAdd[i],type,true,false,false,false);
                e->AddEffectLayer();
            }
        }
        else
        {
            for(wxXmlNode* view=mViews->GetChildren(); view!=NULL; view=view->GetNext() )
            {
                wxString viewName = view->GetAttribute("name");
                bool isChecked = view->GetAttribute("selected")=="1"?true:false;
                if( isChecked )
                {
                    wxString models = view->GetAttribute("models");
                    for(int i=0;i<dialog.ElementsToAdd.size();i++)
                    {
                        if( models != "" )
                        {
                            models += ",";
                        }
                        models += dialog.ElementsToAdd[i];
                    }
                    mSequenceElements->AddMissingModelsToSequence(models);
                    for(int i=0;i<dialog.ElementsToAdd.size();i++)
                    {
                        wxString modelName = dialog.ElementsToAdd[i];
                        Element* elem = mSequenceElements->GetElement(modelName);
                        if( elem != nullptr )
                        {
                            elem->SetVisible(true);
                        }
                    }
                    view->DeleteAttribute("models");
                    view->AddAttribute("models", models);
                    wxString modelsString = mSequenceElements->GetViewModels(viewName);
                    mSequenceElements->PopulateView(modelsString, current_view);
                    break;
                }
            }
        }

        PopulateModels();

        // Update Grid
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    }
}

void DisplayElementsPanel::AddMissingModelsOfView(wxString view)
{
    wxString modelsString = mSequenceElements->GetViewModels(view);
    mSequenceElements->AddMissingModelsToSequence(modelsString);
}

void DisplayElementsPanel::ListItemChecked(wxCommandEvent& event)
{
    Element* e = (Element*)event.GetClientData();
    if( e == nullptr)
    {
        /*const int itemCount = ListCtrlViews->GetItemCount();
        for( int i=0; i<itemCount; i++ )
        {
            ListCtrlViews->SetChecked(i,false);
        }*/
    }
    else
    {
        e->SetVisible(!e->GetVisible());
    }
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
    if( mSequenceElements->GetCurrentView() == MASTER_VIEW )
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
        }
    }
    else
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
            mSequenceElements->DeleteElementFromView(e->GetName(), mSequenceElements->GetCurrentView());
            ListCtrlModels->DeleteItem(itemIndex);
            itemIndex = -1; // reset to delete next item which may have same index
        }
        ListCtrlModels->Thaw();
        ListCtrlModels->Refresh();
        UpdateModelsForSelectedView();
    }
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
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
            mSequenceElements->MoveElementUp(e->GetName(), mSequenceElements->GetCurrentView());
        }
    }
    ListCtrlModels->Thaw();  // free up the list

    if( items_moved )
    {
        UpdateModelsForSelectedView();
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
            mSequenceElements->MoveElementDown(e->GetName(), mSequenceElements->GetCurrentView());
        }
    }
    ListCtrlModels->Thaw();  // free up the list

    if( items_moved )
    {
        UpdateModelsForSelectedView();
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

void DisplayElementsPanel::UpdateModelsForSelectedView()
{
    int current_view = mSequenceElements->GetCurrentView();
    if( current_view != MASTER_VIEW )
    {
        wxString viewName = ListCtrlViews->GetItemText(current_view, 1);
        wxString models = "";
        for(int i = 0; i < mSequenceElements->GetElementCount(current_view); i++)
        {
            Element* elem = mSequenceElements->GetElement(i, current_view);
            if( elem->GetType() == "model" )
            {
                if( models != "" )
                {
                    models += ",";
                }
                models += elem->GetName();
            }
        }

        for(wxXmlNode* view=mViews->GetChildren(); view!=NULL; view=view->GetNext() )
        {
            wxString name = view->GetAttribute("name");
            if( name == viewName )
            {
                view->DeleteAttribute("models");
                view->AddAttribute("models", models);
                break;
            }

        }
    }
    PopulateModels();
}

void DisplayElementsPanel::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    ScrolledWindowDisplayElements->SetSize(s);
    ScrolledWindowDisplayElements->SetMinSize(s);
    ScrolledWindowDisplayElements->SetMaxSize(s);

    ScrolledWindowDisplayElements->FitInside();
    ScrolledWindowDisplayElements->SetScrollRate(5, 5);
    ScrolledWindowDisplayElements->Refresh();
}

void DisplayElementsPanel::OnListCtrlViewsItemSelect(wxListEvent& event)
{
    int index = event.m_itemIndex;
    wxString view_name = ListCtrlViews->GetItemText(index, 1);
    SelectView(view_name);
}

void DisplayElementsPanel::SelectView(const wxString& name)
{
    ListCtrlViews->SetChecked(mSequenceElements->GetCurrentView(),false);
    int j = 0;
    int selected_view = 0;
    for(wxXmlNode* view=mViews->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        j++;
        view->DeleteAttribute("selected");
        if( view->GetAttribute("name") == name )
        {
            view->AddAttribute("selected", "1");
            selected_view = j;
        }
        else
        {
            view->AddAttribute("selected", "0");
        }
    }
    if( selected_view > 0 )
    {
        wxString modelsString = mSequenceElements->GetViewModels(name);
        mSequenceElements->AddMissingModelsToSequence(modelsString);
        mSequenceElements->PopulateView(modelsString, selected_view);
    }
    mSequenceElements->SetCurrentView(selected_view);
    PopulateModels();
    ListCtrlViews->SetChecked(mSequenceElements->GetCurrentView(),true);
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}
