#include "DisplayElementsPanel.h"
#include "../include/model-16.xpm"
#include "../include/timing-16.xpm"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"
#include "ModelViewSelector.h"
#include "sequencer/Element.h"
#include "xLightsMain.h"

#define TIMING_IMAGE 2
#define MODEL_IMAGE 3

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
const long DisplayElementsPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long DisplayElementsPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DisplayElementsPanel,wxPanel)
	//(*EventTable(DisplayElementsPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_LISTITEM_CHECKED, DisplayElementsPanel::ListItemChecked)
END_EVENT_TABLE()

DisplayElementsPanel::DisplayElementsPanel(xLightsFrame *fr,
                                           wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size) : xlFrame(fr)
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
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableRow(0);
	ScrolledWindowDisplayElements = new wxScrolledWindow(Panel_Sizer, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	StaticText4 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT4, _("Views:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ListCtrlViews = new wxCheckedListCtrl(ScrolledWindowDisplayElements, ID_LISTCTRL_VIEWS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_VIEWS"));
	ListCtrlViews->SetMinSize(wxDLG_UNIT(ScrolledWindowDisplayElements,wxSize(100,50)));
	FlexGridSizer6->Add(ListCtrlViews, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddViews = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_ADD_VIEWS, _("Add Views"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_VIEWS"));
	FlexGridSizer3->Add(ButtonAddViews, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteView = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_DELETE_VIEW, _("Delete View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_VIEW"));
	FlexGridSizer3->Add(ButtonDeleteView, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(0);
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer7->AddGrowableRow(1);
	StaticText2 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT2, _("Timings / Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ListCtrlModels = new wxCheckedListCtrl(ScrolledWindowDisplayElements, ID_LISTCTRL_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_MODELS"));
	ListCtrlModels->SetMinSize(wxDLG_UNIT(ScrolledWindowDisplayElements,wxSize(75,85)));
	FlexGridSizer7->Add(ListCtrlModels, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonShowAll = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_SHOW_ALL, _("Show All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHOW_ALL"));
	FlexGridSizer10->Add(ButtonShowAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonHideAll = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_HIDE_ALL, _("Hide All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_HIDE_ALL"));
	FlexGridSizer10->Add(ButtonHideAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer9->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonAddModels = new wxButton(ScrolledWindowDisplayElements, ID_BUTTONADD_MODELS, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONADD_MODELS"));
	FlexGridSizer4->Add(ButtonAddModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteModels = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_DELETE_MODELS, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_MODELS"));
	FlexGridSizer4->Add(ButtonDeleteModels, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(ScrolledWindowDisplayElements, ID_STATICTEXT3, _("Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveUp = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_MOVE_UP, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
	FlexGridSizer4->Add(ButtonMoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonMoveDown = new wxButton(ScrolledWindowDisplayElements, ID_BUTTON_MOVE_DOWN, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_DOWN"));
	FlexGridSizer4->Add(ButtonMoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
	ScrolledWindowDisplayElements->SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(ScrolledWindowDisplayElements);
	FlexGridSizer1->SetSizeHints(ScrolledWindowDisplayElements);
	FlexGridSizer5->Add(ScrolledWindowDisplayElements, 0, wxALIGN_LEFT, 0);
	Panel_Sizer->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel_Sizer);
	FlexGridSizer5->SetSizeHints(Panel_Sizer);
	FlexGridSizer2->Add(Panel_Sizer, 0, wxALIGN_LEFT, 2);
	SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(this);
	FlexGridSizer2->SetSizeHints(this);

	Connect(ID_LISTCTRL_VIEWS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&DisplayElementsPanel::OnListCtrlViewsItemSelect);
	Connect(ID_BUTTON_ADD_VIEWS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddViewsClick);
	Connect(ID_BUTTON_DELETE_VIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonDeleteViewClick);
	Connect(ID_BUTTON_SHOW_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonShowAllClick);
	Connect(ID_BUTTON_HIDE_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonHideAllClick);
	Connect(ID_BUTTONADD_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonAddModelsClick);
	Connect(ID_BUTTON_DELETE_MODELS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonDeleteModelsClick);
	Connect(ID_BUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveUpClick);
	Connect(ID_BUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DisplayElementsPanel::OnButtonMoveDownClick);
	Panel_Sizer->Connect(wxEVT_SIZE,(wxObjectEventFunction)&DisplayElementsPanel::OnResize,0,this);
	Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&DisplayElementsPanel::OnLeftUp);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&DisplayElementsPanel::OnResize);
	//*)

	ListCtrlViews->SetImages((char**)eye_16,(char**)eye_16_gray);
	ListCtrlModels->SetImages((char**)eye_16,(char**)eye_16_gray);
	ListCtrlModels->AddImage((char**)timing_16);
	ListCtrlModels->AddImage((char**)model_16);
	mNumViews = 0;
	mNumModels = 0;
	mSequenceElements = NULL;
    MainViewsChoice = NULL;
}

DisplayElementsPanel::~DisplayElementsPanel()
{
	//(*Destroy(DisplayElementsPanel)
	//*)
}

void DisplayElementsPanel::MarkViewsChanged()
{
    wxCommandEvent eventRgbEffects(EVT_RGBEFFECTS_CHANGED);
    wxPostEvent(GetParent(), eventRgbEffects);
}

void DisplayElementsPanel::SetViewChoice(wxChoice *ch) {
    MainViewsChoice = ch;
    MainViewsChoice->Connect(wxEVT_CHOICE, (wxObjectEventFunction)&DisplayElementsPanel::OnViewSelect, nullptr, this);
}

void DisplayElementsPanel::OnViewSelect(wxCommandEvent &event) {
    SelectView(MainViewsChoice->GetString(MainViewsChoice->GetSelection()).ToStdString());
}

void DisplayElementsPanel::SetSequenceElementsModelsViews(SequenceData* seq_data,
                                                          SequenceElements* elements,
                                                          wxXmlNode* models,
                                                          wxXmlNode* modelGroups,
													  	  SequenceViewManager* sequenceViewManager)
{
}

void DisplayElementsPanel::Initialize()
{
}

void DisplayElementsPanel::AddViewToList(const wxString& viewName, bool isChecked)
{
}

bool DisplayElementsPanel::IsModelAGroup(const std::string& modelname) const
{
    return false;
}

void DisplayElementsPanel::AddModelToList(Element* model)
{
}

void DisplayElementsPanel::AddTimingToList(Element* timing)
{
}

void DisplayElementsPanel::PopulateViews()
{
}

void DisplayElementsPanel::PopulateModels()
{
}

int DisplayElementsPanel::GetViewIndex(const wxString& name)
{
    return 0;
}

void DisplayElementsPanel::OnLeftUp(wxMouseEvent& event)
{
}

void DisplayElementsPanel::OnButtonAddViewsClick(wxCommandEvent& event)
{
}

void DisplayElementsPanel::OnButtonDeleteViewClick(wxCommandEvent& event)
{
}

void DisplayElementsPanel::OnButtonAddModelsClick(wxCommandEvent& event)
{
    if (mSeqData->NumFrames() == 0) return;
    ModelViewSelector dialog(this);
    dialog.SetSequenceElementsModelsViews(mSequenceElements,mModels,mModelGroups,_sequenceViewManager, mSequenceElements->GetCurrentView());
    std::string type = "model";
    int current_view = mSequenceElements->GetCurrentView();
    dialog.Initialize();
    dialog.CenterOnParent();
    int DlgResult = dialog.ShowModal();
    if(DlgResult==wxID_OK)
    {
        if( current_view == MASTER_VIEW )
        {
            for(size_t i=0;i<dialog.ModelsToAdd.size();i++)
            {
                Element* e = mSequenceElements->AddElement(dialog.ModelsToAdd[i],type,true,false,false,false);
				if (e != nullptr)
				{
					e->AddEffectLayer();
				}
            }
        }
        else
        {
			auto view = _sequenceViewManager->GetSelectedView();
            for(int i=0;i<dialog.ModelsToAdd.size();i++)
            {
				view->AddModel(dialog.ModelsToAdd[i]);
            }
            mSequenceElements->AddMissingModelsToSequence(view->GetModelsString());
            for(int i=0;i<dialog.ModelsToAdd.size();i++)
            {
                Element* elem = mSequenceElements->GetElement(dialog.ModelsToAdd[i]);
                if( elem != nullptr )
                {
                    elem->SetVisible(true);
                }
            }
            std::string modelsString = mSequenceElements->GetViewModels(view->GetName());
            mSequenceElements->PopulateView(modelsString, current_view);
            mSequenceElements->AddViewToTimings(dialog.TimingsToAdd, view->GetName());
            mSequenceElements->SetTimingVisibility(view->GetName());
        }
        MarkViewsChanged();

        PopulateModels();

        // Update Grid
        xlFrame->DoForceSequencerRefresh();
    }
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
    MarkViewsChanged();
    // Update Grid
    xlFrame->DoForceSequencerRefresh();
}


void DisplayElementsPanel::OnButtonShowAllClick(wxCommandEvent& event)
{
    if (mSeqData->NumFrames() == 0) return;
    mSequenceElements->SetVisibilityForAllModels(true, mSequenceElements->GetCurrentView());
    MarkViewsChanged();
    PopulateModels();
    xlFrame->DoForceSequencerRefresh();
}

void DisplayElementsPanel::OnButtonHideAllClick(wxCommandEvent& event)
{
    if (mSeqData->NumFrames() == 0) return;
    mSequenceElements->SetVisibilityForAllModels(false, mSequenceElements->GetCurrentView());
    MarkViewsChanged();
    PopulateModels();
    xlFrame->DoForceSequencerRefresh();
}

void DisplayElementsPanel::OnButtonDeleteModelsClick(wxCommandEvent& event)
{
    if (mSeqData->NumFrames() == 0) return;
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
            if( e->GetType() == ELEMENT_TYPE_MODEL)
            {
                mSequenceElements->DeleteElementFromView(e->GetName(), mSequenceElements->GetCurrentView());
            }
            else
            {
                mSequenceElements->DeleteTimingFromView(e->GetName(), mSequenceElements->GetCurrentView());
                mSequenceElements->SetTimingVisibility(mSequenceElements->GetViewName(mSequenceElements->GetCurrentView()));
            }
            ListCtrlModels->DeleteItem(itemIndex);
            itemIndex = -1; // reset to delete next item which may have same index
        }
        ListCtrlModels->Thaw();
        ListCtrlModels->Refresh();
        UpdateModelsForSelectedView();
    }
    MarkViewsChanged();
    xlFrame->DoForceSequencerRefresh();
}

int DisplayElementsPanel::GetFirstModelIndex()
{
    return 0;
}

void DisplayElementsPanel::OnButtonMoveUpClick(wxCommandEvent& event)
{
    if (mSeqData->NumFrames() == 0) return;
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

    int first_model = GetFirstModelIndex();
    if (!selected_list.empty() && selected_list[0] != 0 && selected_list[0] != first_model )  // don't let item or group move up if top item is selected
    {
        if( selected_list[0] > first_model || (selected_list[0] < first_model && selected_list.size() == 1 ) )
        {
            items_moved = true;
            for( long i = 0; i < selected_list.size(); i++ )
            {
                Element* e = (Element*)ListCtrlModels->GetItemData(selected_list[i]);
                mSequenceElements->MoveElementUp(e->GetName(), mSequenceElements->GetCurrentView());
            }
        }
    }
    ListCtrlModels->Thaw();  // free up the list
    ListCtrlModels->Refresh();

    if( items_moved )
    {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        xlFrame->DoForceSequencerRefresh();
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
    if (mSeqData->NumFrames() == 0) return;
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

    int first_model = GetFirstModelIndex();
    if(!selected_list.empty() && selected_list.back() < num_items-1 && selected_list[0] != first_model-1 )  // don't let item or group move up if top item is selected
    {
        if( selected_list[0] >= first_model || (selected_list[0] < first_model && selected_list.size() == 1 ) )
        {
            items_moved = true;
            for( long i = selected_list.size()-1; i >= 0; i-- )
            {
                Element* e = (Element*)ListCtrlModels->GetItemData(selected_list[i]);
                mSequenceElements->MoveElementDown(e->GetName(), mSequenceElements->GetCurrentView());
            }
        }
    }
    ListCtrlModels->Thaw();  // free up the list
    ListCtrlModels->Refresh();

    if( items_moved )
    {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        xlFrame->DoForceSequencerRefresh();
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
}

void DisplayElementsPanel::OnResize(wxSizeEvent& event)
{
}

void DisplayElementsPanel::OnListCtrlViewsItemSelect(wxListEvent& event)
{
}

void DisplayElementsPanel::SelectView(const std::string& name)
{
}
