/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#define TRACEMOVES

//(*InternalHeaders(ViewsModelsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/xml/xml.h>
#include <wx/dnd.h>

#include "../include/model-16.xpm"
#include "../include/timing-16.xpm"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"

#include "ViewsModelsPanel.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "SequenceViewManager.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

// This event is fired when a model is dropped between lists
wxDEFINE_EVENT(EVT_VMDROP, wxCommandEvent);

class VMDropSource : public wxDropSource
{
    ViewsModelsPanel* _window;
    bool _nonModels;
    bool _models;

public:

    VMDropSource(ViewsModelsPanel* window, bool models, bool nonModels) : wxDropSource(window)
    {
        _window = window;
        _nonModels = nonModels;
        _models = models;
    }

    virtual bool GiveFeedback(wxDragResult effect) override
    {
        wxPoint point = wxGetMousePosition();

        if (_models) {
            if (_window->ListCtrlModels->GetScreenRect().Contains(point) ||
                _window->ListCtrlNonModels->GetScreenRect().Contains(point)) {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else if (_nonModels) {
            if (_window->ListCtrlModels->GetScreenRect().Contains(point)) {
                _window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            else {
                _window->SetCursor(wxCursor(wxCURSOR_NO_ENTRY));
            }
        }
        else {
            return false;
        }

        return true;
    }
};

#define TIMING_IMAGE 2
#define MODEL_IMAGE 3

//(*IdInit(ViewsModelsPanel)
const wxWindowID ViewsModelsPanel::ID_BUTTON3 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON4 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON5 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON6 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON_TOP = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON9 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON10 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON_BOTTOM = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON1 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON2 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON7 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON8 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON_IMPORT = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON11 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON_VIEWUP = wxNewId();
const wxWindowID ViewsModelsPanel::ID_BUTTON_VIEWDOWN = wxNewId();
const wxWindowID ViewsModelsPanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_LISTCTRL_VIEWS = wxNewId();
const wxWindowID ViewsModelsPanel::ID_STATICTEXT2 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_LISTCTRL_MODELS = wxNewId();
const wxWindowID ViewsModelsPanel::ID_LISTCTRL1 = wxNewId();
const wxWindowID ViewsModelsPanel::ID_STATICTEXT3 = wxNewId();
//*)

const long ViewsModelsPanel::ID_MODELS_UNDO = wxNewId();
const long ViewsModelsPanel::ID_MODELS_HIDEALL = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SHOWALL = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SELECTALL = wxNewId();
const long ViewsModelsPanel::ID_MODELS_HIDEUNUSED = wxNewId();
const long ViewsModelsPanel::ID_MODELS_REMOVEUNUSED = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SELECTUNUSED = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SELECTUSED = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORT = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYNAME = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYNAMEGM = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYCPGM = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYSCGM = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYTYPE = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTMODELSUNDERTHISGROUP = wxNewId();
const long ViewsModelsPanel::ID_MODELS_BUBBLEUPGROUPS = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYNAMEGMSIZE = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYNAMEGMCOUNT = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYSCGMSIZE = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYCPGMSIZE = wxNewId();
const long ViewsModelsPanel::ID_MODELS_SORTBYMASTERVIEW = wxNewId();
const long ViewsModelsPanel::VIEWSMODELS_IMPORT_RGBEFFECTS = wxNewId();
const long ViewsModelsPanel::VIEWSMODELS_IMPORT_SEQUENCE = wxNewId();

BEGIN_EVENT_TABLE(ViewsModelsPanel,wxPanel)
	//(*EventTable(ViewsModelsPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_LISTITEM_CHECKED, ViewsModelsPanel::OnListCtrlItemCheck)
    EVT_COMMAND(wxID_ANY, EVT_VMDROP, ViewsModelsPanel::OnDrop)
END_EVENT_TABLE()

ViewsModelsPanel::ViewsModelsPanel(xLightsFrame *frame, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : _xlFrame(frame)
{
	//(*Initialize(ViewsModelsPanel)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer8;
	wxGridBagSizer* GridBagSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer5->AddGrowableRow(0);
	FlexGridSizer5->Add(-1,-1,5, wxALL|wxEXPAND, 2);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	Button_AddAll = new wxButton(this, ID_BUTTON3, _T(">>"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer1->Add(Button_AddAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_AddSelected = new wxButton(this, ID_BUTTON4, _T(">"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(Button_AddSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_RemoveSelected = new wxButton(this, ID_BUTTON5, _T("<"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON5"));
	BoxSizer1->Add(Button_RemoveSelected, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_RemoveAll = new wxButton(this, ID_BUTTON6, _T("<<"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON6"));
	BoxSizer1->Add(Button_RemoveAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Button_Top = new wxButton(this, ID_BUTTON_TOP, _T("^^"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_TOP"));
	Button_Top->SetToolTip(_("Move to Top of List"));
	BoxSizer1->Add(Button_Top, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveUp = new wxButton(this, ID_BUTTON9, _T("^"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON9"));
	BoxSizer1->Add(Button_MoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveDown = new wxButton(this, ID_BUTTON10, _("v"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON10"));
	BoxSizer1->Add(Button_MoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Bottom = new wxButton(this, ID_BUTTON_BOTTOM, _("vv"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_BOTTOM"));
	BoxSizer1->Add(Button_Bottom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(BoxSizer1, 1, wxALL|wxEXPAND, 0);
	GridBagSizer1->Add(FlexGridSizer5, wxGBPosition(1, 1), wxGBSpan(3, 1), wxEXPAND, 0);
	FlexGridSizer8 = new wxFlexGridSizer(7, 1, 0, 0);
	Button_AddView = new wxButton(this, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer8->Add(Button_AddView, 1, wxALL|wxEXPAND, 2);
	Button_DeleteView = new wxButton(this, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer8->Add(Button_DeleteView, 1, wxALL|wxEXPAND, 2);
	ButtonRename = new wxButton(this, ID_BUTTON7, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer8->Add(ButtonRename, 1, wxALL|wxEXPAND, 2);
	ButtonClone = new wxButton(this, ID_BUTTON8, _("Clone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	FlexGridSizer8->Add(ButtonClone, 1, wxALL|wxEXPAND, 2);
	ButtonImport = new wxButton(this, ID_BUTTON_IMPORT, _("Import..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	FlexGridSizer8->Add(ButtonImport, 1, wxALL|wxEXPAND, 2);
	Button_MakeMaster = new wxButton(this, ID_BUTTON11, _("Copy To Master"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
	Button_MakeMaster->SetToolTip(_("Copy elements and ordering to your master render view"));
	FlexGridSizer8->Add(Button_MakeMaster, 1, wxALL|wxEXPAND, 2);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	Button_ViewUp = new wxButton(this, ID_BUTTON_VIEWUP, _T("^"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_VIEWUP"));
	BoxSizer2->Add(Button_ViewUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ViewDown = new wxButton(this, ID_BUTTON_VIEWDOWN, _("v"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_BUTTON_VIEWDOWN"));
	BoxSizer2->Add(Button_ViewDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(BoxSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	GridBagSizer1->Add(FlexGridSizer8, wxGBPosition(1, 3), wxDefaultSpan, wxALL|wxEXPAND, 2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("View:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(0, 2), wxDefaultSpan, wxALL, 2);
	ListCtrlViews = new wxCheckedListCtrl(this, ID_LISTCTRL_VIEWS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTCTRL_VIEWS"));
	ListCtrlViews->SetMinSize(wxDLG_UNIT(this,wxSize(25,30)));
	GridBagSizer1->Add(ListCtrlViews, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxEXPAND, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Added:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(2, 2), wxDefaultSpan, wxALL, 0);
	ListCtrlModels = new wxCheckedListCtrl(this, ID_LISTCTRL_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTCTRL_MODELS"));
	ListCtrlModels->SetMinSize(wxDLG_UNIT(this,wxSize(30,30)));
	GridBagSizer1->Add(ListCtrlModels, wxGBPosition(3, 2), wxGBSpan(1, 2), wxALL|wxEXPAND, 2);
	ListCtrlNonModels = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL1"));
	ListCtrlNonModels->SetMinSize(wxDLG_UNIT(this,wxSize(25,30)));
	GridBagSizer1->Add(ListCtrlNonModels, wxGBPosition(1, 0), wxGBSpan(3, 1), wxALL|wxEXPAND, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Available:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	GridBagSizer1->Add(StaticText3, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxEXPAND, 2);
	SetSizer(GridBagSizer1);

	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddAllClick);
	Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddSelectedClick);
	Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_RemoveSelectedClick);
	Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_RemoveAllClick);
	Connect(ID_BUTTON_TOP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_TopClick);
	Connect(ID_BUTTON9, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_MoveUpClick);
	Connect(ID_BUTTON10, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_MoveDownClick);
	Connect(ID_BUTTON_BOTTOM, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_BottomClick);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_AddViewClick);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_DeleteViewClick);
	Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButtonRenameClick);
	Connect(ID_BUTTON8, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButtonCloneClick);
	Connect(ID_BUTTON_IMPORT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButtonImportClick);
	Connect(ID_BUTTON11, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButton_MakeMasterClick);
	Connect(ID_BUTTON_VIEWUP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButtonView_UpClick);
	Connect(ID_BUTTON_VIEWDOWN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ViewsModelsPanel::OnButtonView_DownClick);
	Connect(ID_LISTCTRL_VIEWS, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlViewsItemSelect);
	Connect(ID_LISTCTRL_VIEWS, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlViewsItemDClick);
	Connect(ID_LISTCTRL_VIEWS, wxEVT_COMMAND_LIST_KEY_DOWN, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlViewsKeyDown);
	Connect(ID_LISTCTRL_MODELS, wxEVT_COMMAND_LIST_BEGIN_DRAG, (wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsBeginDrag);
	Connect(ID_LISTCTRL_MODELS, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsItemSelect);
	Connect(ID_LISTCTRL_MODELS, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlModelsItemActivated);
	Connect(ID_LISTCTRL_MODELS, wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlModelsItemRClick);
	Connect(ID_LISTCTRL_MODELS, wxEVT_COMMAND_LIST_KEY_DOWN, (wxObjectEventFunction)&ViewsModelsPanel::OnListView_ViewItemsKeyDown);
	Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_BEGIN_DRAG, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsBeginDrag);
	Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsItemSelect);
	Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsItemActivated);
	Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_KEY_DOWN, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsKeyDown);
	Connect(ID_LISTCTRL1, wxEVT_COMMAND_LIST_COL_CLICK, (wxObjectEventFunction)&ViewsModelsPanel::OnListCtrlNonModelsColumnClick);
	Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ViewsModelsPanel::OnLeftUp);
	//*)

    Connect(wxID_ANY, EVT_VMDROP, (wxObjectEventFunction)&ViewsModelsPanel::OnDrop);

    GridBagSizer1->AddGrowableCol(0, 2);
    GridBagSizer1->AddGrowableCol(2, 1);
    GridBagSizer1->AddGrowableRow(3);

    ListCtrlViews->SetImages((char**)eye_16, (char**)eye_16_gray);

    ListCtrlModels->SetImages((char**)eye_16, (char**)eye_16_gray);
    ListCtrlModels->AddImage((char**)timing_16);
    ListCtrlModels->AddImage((char**)model_16);

    _imageList = new wxImageList(16, 16, true);
    _imageList->Add(wxIcon((char**)timing_16));
    _imageList->Add(wxIcon((char**)model_16));

    MyTextDropTarget *mdt = new MyTextDropTarget(this, ListCtrlModels, "Model");
    ListCtrlModels->SetDropTarget(mdt);

    mdt = new MyTextDropTarget(this, ListCtrlNonModels, "NonModel");
    ListCtrlNonModels->SetDropTarget(mdt);

    int w, h;
    GetSize(&w, &h);
    SetSize(std::max(600, w), std::max(400, h));

    ValidateWindow();
}

ViewsModelsPanel::~ViewsModelsPanel()
{
    //(*Destroy(ViewsModelsPanel)
    //*)

    //for (int i = 0; i < ListCtrlNonModels->GetItemCount(); ++i)
    //{
    //    Element* e = (Element*)ListCtrlNonModels->GetItemData(i);
    //    if (e != nullptr && e->GetType() == ELEMENT_TYPE_MODEL && e->GetSequenceElements() == nullptr)
    //    {
    //        delete e;
    //        ListCtrlNonModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
    //    }
    //}

    delete _imageList;
}

void ViewsModelsPanel::PopulateModels(const std::string& selectModels)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ListCtrlModels->Freeze();
    ListCtrlNonModels->Freeze();

    int topM = ListCtrlModels->GetTopItem();
    int topN = ListCtrlNonModels->GetTopItem();

    int itemSize;
    if (ListCtrlModels->GetItemCount() > 0) {
        wxRect rect;
        ListCtrlModels->GetItemRect(0, rect);
        itemSize = rect.GetHeight();
    }
    else {
        // need to add something so i can measure it

        ListCtrlModels->AppendColumn("Test");
        ListCtrlModels->InsertItem(0, "Test");

        wxRect rect;
        ListCtrlModels->GetItemRect(0, rect);
        itemSize = rect.GetHeight();
    }

    if (itemSize == 0) {
        logger_base.crit("ViewsModelsPanel::Populate models ... itemSize = 0 ... this is going to crash.");
    }

    int visibileM = ListCtrlModels->GetRect().GetHeight() / itemSize - 1;
    int visibileN = ListCtrlNonModels->GetRect().GetHeight() / itemSize - 1;

    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
        Element* e = (Element*)ListCtrlNonModels->GetItemData(i);
        if (!_sequenceElements->IsValidElement(e)) {
            // This should never happen ... but it does so stop it crashing
            ListCtrlNonModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
        }
        else if (e != nullptr && e->GetType() == ElementType::ELEMENT_TYPE_MODEL && e->GetSequenceElements() == nullptr) {
            delete e;
            ListCtrlNonModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
        }
    }

    ListCtrlNonModels->ClearAll();
    ListCtrlNonModels->SetImageList(_imageList, wxIMAGE_LIST_SMALL);

    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetText(_(""));
    nm0.SetAlign(wxLIST_FORMAT_CENTER);
    ListCtrlNonModels->InsertColumn(0, nm0);

    wxListItem nm1;
    nm1.SetId(1);
    nm1.SetText(_("Timing/Model"));
    ListCtrlNonModels->InsertColumn(1, nm1);

    ListCtrlModels->ClearAll();

    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_(""));
    col0.SetAlign(wxLIST_FORMAT_CENTER);
    ListCtrlModels->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetText(_(""));
    col1.SetAlign(wxLIST_FORMAT_CENTER);
    ListCtrlModels->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText(_("Timing/Model"));
    ListCtrlModels->InsertColumn(2, col2);

    _numModels = 0;
    _numNonModels = 0;
    if (_sequenceElements != nullptr) {
        int current_view = _sequenceElements->GetCurrentView();
        for (int i = 0; i < _sequenceElements->GetElementCount(); i++) {
            Element* elem = _sequenceElements->GetElement(i);
            if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                TimingElement* te = dynamic_cast<TimingElement*>(elem);
                if (current_view == MASTER_VIEW || (te != nullptr && _sequenceElements->TimingIsPartOfView(te, current_view))) {
                    AddTimingToList(elem);
                }
                else {
                    AddTimingToNotList(elem);
                }
            }
        }

        if (current_view > 0) {
            SequenceView* view = _sequenceViewManager->GetSelectedView();
            if (view != nullptr) {
                _sequenceElements->AddMissingModelsToSequence(view->GetModelsString());
                auto models = view->GetModels();
                for (const auto& it : models) {
                    Element* elem = _sequenceElements->GetElement(it);
                    if (elem != nullptr) AddModelToList(elem);
                }

                // add everything that isnt in the view
                for (int i = 0; i < _sequenceElements->GetElementCount(); i++) {
                    Element* elem = _sequenceElements->GetElement(i);
                    if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL && std::find(models.begin(), models.end(), elem->GetName()) == models.end()) {
                        AddModelToNotList(elem);
                    }
                }
            }
        }
        else {
            for (int i = 0; i < _sequenceElements->GetElementCount(); i++) {
                Element* elem = _sequenceElements->GetElement(i);
                if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                    AddModelToList(elem);
                }
            }
        }

        if (_modelGroups != nullptr) {
            for (wxXmlNode* e = _modelGroups->GetChildren(); e != nullptr; e = e->GetNext()) {
                if (e->GetName() == "modelGroup") {
                    wxString name = e->GetAttribute("name");
                    if (!_sequenceElements->ElementExists(name.ToStdString(), 0)) {
                        ModelElement* me = new ModelElement(name.ToStdString());
                        if (me != nullptr) AddModelToNotList(me);
                    }
                }
            }
        }

        if (_models != nullptr) {
            for (wxXmlNode* e = _models->GetChildren(); e != nullptr; e = e->GetNext()) {
                if (e->GetName() == "model") {
                    wxString name = e->GetAttribute("name");
                    if (!_sequenceElements->ElementExists(name.ToStdString(), 0)) {
                        ModelElement* me = new ModelElement(name.ToStdString());
                        if (me != nullptr) AddModelToNotList(me);
                    }
                }
            }
        }

        if (visibileM > 0 && visibileN > 0) {
            if (ListCtrlModels->GetItemCount() > 0) {
                if (topM + visibileM - 1 < ListCtrlModels->GetItemCount()) {
                    ListCtrlModels->EnsureVisible(topM + visibileM - 1);
                }
                ListCtrlModels->EnsureVisible(topM);
            }
            if (ListCtrlNonModels->GetItemCount() > 0) {
                if (topN + visibileN - 1 < ListCtrlNonModels->GetItemCount()) {
                    ListCtrlNonModels->EnsureVisible(topN + visibileN - 1);
                }
                ListCtrlNonModels->EnsureVisible(topN);
            }
        }

        if (selectModels != "") {
            wxArrayString models = wxSplit(selectModels, ',');

            for (const auto& it : models) {
                SelectItem(ListCtrlModels, it.ToStdString(), 2, true);
            }
        }
    }

    ListCtrlNonModels->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrlNonModels->GetColumnWidth(0) < 22) {
        ListCtrlNonModels->SetColumnWidth(0, 22);
    }
    ListCtrlNonModels->SetColumnWidth(1, wxLIST_AUTOSIZE);

    ListCtrlModels->SetColumnWidth(0, wxLIST_AUTOSIZE);
    ListCtrlModels->SetColumnWidth(1, wxLIST_AUTOSIZE);
    if (ListCtrlModels->GetColumnWidth(0) < 28) {
        ListCtrlModels->SetColumnWidth(0, 28);
    }
    if (ListCtrlModels->GetColumnWidth(1) < 22) {
        ListCtrlModels->SetColumnWidth(1, 22);
    }
    ListCtrlModels->SetColumnWidth(2, wxLIST_AUTOSIZE);

    SortNonModels();

    ListCtrlModels->Thaw();
    ListCtrlNonModels->Thaw();
    ListCtrlModels->Refresh();
    ListCtrlNonModels->Refresh();
}

bool ViewsModelsPanel::IsModelAGroup(const std::string& modelname) const
{
    for (auto it = _modelGroups->GetChildren(); it != nullptr; it = it->GetNext()) {
        if (it->GetName() == "modelGroup" && it->GetAttribute("name") == modelname) {
            return true;
        }
    }
    return false;
}

int ViewsModelsPanel::GetPixelCount(const std::string& modelname) {
    int pixelCount = 0;

    if (IsModelAGroup(modelname)) {
        wxArrayString groupModels;
        groupModels = GetGroupModels(modelname);
        for (const auto& m : groupModels) {
            pixelCount += GetPixelCount(m);
        }
    } else {
        int modelPixels = 0;
        auto model = _xlFrame->AllModels.GetModel(modelname);
        if (model != nullptr) {
            modelPixels += model->GetNodeCount();
        }
        pixelCount += modelPixels;
    }
    return pixelCount;
}

wxArrayString ViewsModelsPanel::GetGroupModels(const std::string& group) const
{
    wxArrayString res;

    for (auto it = _modelGroups->GetChildren(); it != nullptr; it = it->GetNext()) {
        if (it->GetName() == "modelGroup" && it->GetAttribute("name") == group) {
            res = wxSplit(it->GetAttribute("models"), ',');
            break;
        }
    }

    return res;
}

std::string ViewsModelsPanel::GetModelType(const std::string& modelname) const
{
    for (auto it = _models->GetChildren(); it != nullptr; it = it->GetNext()) {
        if (it->GetAttribute("name") == modelname) {
            return it->GetAttribute("DisplayAs").ToStdString();
        }
    }
    return "";
}

std::string ViewsModelsPanel::GetModelCPSortString(const std::string& modelname) const
{
    auto& mm = _xlFrame->AllModels;

    auto m = mm.GetModel(modelname);

    if (m != nullptr) {
        return m->GetControllerPortSortString();
    }
    auto c = PadLeft("Z", 'Z', 140);
    return c + ":99999999:99999999:" + modelname;
}

uint32_t ViewsModelsPanel::GetModelSC(const std::string& modelname) const
{
    auto& mm = _xlFrame->AllModels;

    auto m = mm.GetModel(modelname);

    if (m != nullptr) {
        return m->GetFirstChannel();
    }
    return 0;
}

bool ViewsModelsPanel::IsItemSelected(wxListCtrl* ctrl, int item)
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

bool ViewsModelsPanel::SelectItem(wxListCtrl* ctrl, int item, bool select)
{
    return ctrl->SetItemState(item, select ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
}

bool ViewsModelsPanel::SelectItem(wxListCtrl* ctrl, const std::string& item, int col, bool select)
{
    for (int i = 0; i < ctrl->GetItemCount(); ++i) {
        if (ctrl->GetItemText(i, col).ToStdString() == item) {
            return ctrl->SetItemState(i, select ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
        }
    }
    return false;
}

void ViewsModelsPanel::OnListView_ViewItemsBeginDrag(wxListEvent& event)
{
    if (ListCtrlModels->GetSelectedItemCount() == 0) return;

    _dragRowModel = true;
    _dragRowNonModel = false;

    wxString drag = "Model";
    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            drag += "," + ListCtrlModels->GetItemText(i, 1);
        }
    }

    wxTextDataObject my_data(drag);
    VMDropSource dragSource(this, true, false);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ViewsModelsPanel::OnListView_ViewItemsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ViewsModelsPanel::OnListView_ViewItemsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if (key == WXK_LEFT || key == WXK_DELETE || key == WXK_NUMPAD_DELETE) {
        RemoveSelectedModels();
    }
    ValidateWindow();
}

void ViewsModelsPanel::RemoveSelectedModels()
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    SaveUndo();

    // #4134: Abort any render in progress to avoid hanging
    _sequenceElements->GetXLightsFrame()->AbortRender();

    if (_sequenceElements->GetCurrentView() == MASTER_VIEW) {
        bool hasEffects = false;
        for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlModels, i)) {
                Element* e = (Element*)ListCtrlModels->GetItemData(i);

                if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                    if (wxMessageBox("Removing timing track '" + e->GetName() + "' from the Master View will delete the timing track. Are you sure you want to do this?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxNO) {
                        SelectItem(ListCtrlModels, i, false);
                    }
                }
                else if (e->HasEffects()) {
                    hasEffects = true;
                    break;
                }
            }
        }

        if (!hasEffects || wxMessageBox("Delete all effects and layers for the selected model(s)?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {

            // we need to unselect the current effect because we may be about to delete it
            wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
            wxPostEvent(GetParent(), eventUnSelected);

            for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
                if (IsItemSelected(ListCtrlModels, i)) {
                    // Got a selected item so handle it
                    Element* e = (Element*)ListCtrlModels->GetItemData(i);
                    //_sequenceElements->DeleteElementFromView(e->GetName(), MASTER_VIEW);
                    _sequenceElements->DeleteElement(e->GetName());
                }
            }
        }
    } else {
        for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlModels, i)) {
                // Got a selected item so handle it
                Element* e = (Element*)ListCtrlModels->GetItemData(i);
                if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                    _sequenceElements->DeleteElementFromView(e->GetName(), _sequenceElements->GetCurrentView());
                }
                else {
                    _sequenceElements->DeleteTimingFromView(e->GetName(), _sequenceElements->GetCurrentView());
                    _sequenceElements->SetTimingVisibility(_sequenceElements->GetViewName(_sequenceElements->GetCurrentView()));
                }
            }
        }
        UpdateModelsForSelectedView();
    }

    MarkViewsChanged();
    PopulateModels();

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::AddSelectedModels(int pos)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    SaveUndo();

    int p = pos;

    std::string type = "model";
    int currentView = _sequenceElements->GetCurrentView();

    if (p == -1) {
        p = _sequenceElements->GetElementCount();
    } else {
        if (p < 0) p = 0;

        if (currentView != MASTER_VIEW) {
            p -= GetTimingCount();
        }
    }

    wxArrayString addedModels;
    for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlNonModels, i)) {
            addedModels.push_back(ListCtrlNonModels->GetItemText(i, 1));
        }
    }

    if (currentView == MASTER_VIEW) {
        int selcnt = 0;

        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlNonModels, i)) {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
#ifdef TRACEMOVES
                    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                    logger_base.debug("Timing count in models list: %d", GetTimingCount());
                    logger_base.debug("Adding '%s' to %d '%s'", (const char*)ListCtrlNonModels->GetItemText(i, 1).c_str(),
                        p + selcnt, (const char*)(_sequenceElements->GetElement(p + selcnt) == nullptr) ? "N/A" : _sequenceElements->GetElement(p + selcnt)->GetName().c_str());
#endif

                    Element* e = _sequenceElements->AddElement(p + selcnt, ListCtrlNonModels->GetItemText(i, 1).ToStdString(), type, true, false, false, false, false);
                    if (e != nullptr) {
                        e->AddEffectLayer();
                    }
                    selcnt++;
                }
            }
        }
    }
    else {
        int selcnt = 0;
        auto view = _sequenceViewManager->GetSelectedView();
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlNonModels, i)) {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                    view->AddModel(ListCtrlNonModels->GetItemText(i, 1).ToStdString(), p + selcnt);
                    selcnt++;
                }
            }
        }

        _sequenceElements->AddMissingModelsToSequence(view->GetModelsString());
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlNonModels, i)) {
                Element* e = _sequenceElements->GetElement(ListCtrlNonModels->GetItemText(i, 1).ToStdString());
                if (e != nullptr && e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                    e->SetVisible(true);
                }
            }
        }

        std::string modelsString = _sequenceElements->GetViewModels(view->GetName());
        _sequenceElements->PopulateView(modelsString, currentView);

        std::vector<std::string> timings;
        for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
            if (IsItemSelected(ListCtrlNonModels, i)) {
                Element* ee = (Element*)ListCtrlNonModels->GetItemData(i);
                if (ee != nullptr && ee->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                    timings.push_back(ListCtrlNonModels->GetItemText(i, 1).ToStdString());
                }
            }
        }

        if (timings.size() > 0) {
            _sequenceElements->AddViewToTimings(timings, view->GetName());
        }

        _sequenceElements->SetTimingVisibility(view->GetName());
    }

    MarkViewsChanged();
    PopulateModels(wxJoin(addedModels, ',').ToStdString());

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::OnButton_AddAllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); i++) {
        ListCtrlNonModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    AddSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_AddSelectedClick(wxCommandEvent& event)
{
    AddSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_RemoveSelectedClick(wxCommandEvent& event)
{
    RemoveSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::OnButton_RemoveAllClick(wxCommandEvent& event)
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); i++) {
        ListCtrlModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    RemoveSelectedModels();

    ValidateWindow();
}

void ViewsModelsPanel::Clear()
{
    ListCtrlModels->ClearAll();

    for (int i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
        Element* e = (Element*)ListCtrlNonModels->GetItemData(i);
        if (e != nullptr && e->GetType() == ElementType::ELEMENT_TYPE_MODEL && e->GetSequenceElements() == nullptr) {
            delete e;
            ListCtrlNonModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
        }
    }
    ListCtrlNonModels->ClearAll();

    ListCtrlViews->ClearAll();
    ValidateWindow();
}

void ViewsModelsPanel::Initialize()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_seqData == nullptr || _seqData->NumFrames() == 0) {
        Clear();
        return;
    }

    if (_sequenceElements == nullptr) {
        logger_base.crit("ViewsModelsPanel::Initialize _sequenceElements was null ... this is going to crash.");
    }
    if (_sequenceViewManager == nullptr) {
        logger_base.crit("ViewsModelsPanel::Initialize _sequenceViewManager was null ... this is going to crash.");
    }

    _sequenceElements->SetViewsManager(_sequenceViewManager);

    PopulateViews();
    PopulateModels();

    // ensure the selected view exists ... this can happen if the user creates views but doesnt save them
    if (_sequenceViewManager->GetView(_sequenceElements->GetCurrentView()) == nullptr) _sequenceElements->SetCurrentView(0);

    SelectView(_sequenceElements->GetViewName(_sequenceElements->GetCurrentView()));
    ValidateWindow();
}

void ViewsModelsPanel::SetSequenceElementsModelsViews(SequenceData* seqData, SequenceElements* sequenceElements, wxXmlNode* modelsNode, wxXmlNode* modelGroupsNode, SequenceViewManager* sequenceViewManager)
{
    _sequenceElements = sequenceElements;
    _seqData = seqData;
    _models = modelsNode;
    _sequenceViewManager = sequenceViewManager;
    _modelGroups = modelGroupsNode;
    ValidateWindow();
}

void ViewsModelsPanel::ValidateWindow()
{
    if (_sequenceViewManager == nullptr) {
        Enable(false);
        return;
    }
    else {
        Enable(true);
    }

    if (ListCtrlNonModels->GetItemCount() > 0) {
        Button_AddAll->Enable(true);
    }
    else {
        Button_AddAll->Enable(false);
    }

    if (ListCtrlModels->GetItemCount() > 0) {
        Button_RemoveAll->Enable(true);
    }
    else {
        Button_RemoveAll->Enable(false);
    }

    if (ListCtrlNonModels->GetSelectedItemCount() == 0) {
        Button_AddSelected->Enable(false);
    }
    else {
        Button_AddSelected->Enable(true);
    }

    if (ListCtrlModels->GetSelectedItemCount() == 0) {
        Button_RemoveSelected->Enable(false);
    }
    else {
        Button_RemoveSelected->Enable(true);
    }

    if (ListCtrlViews->GetSelectedItemCount() == 0) {
        ButtonClone->Enable(false);
    }
    else {
        ButtonClone->Enable(true);
    }
    if (ListCtrlViews->GetSelectedItemCount() == 0 || _sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        Button_DeleteView->Enable(false);
        ButtonRename->Enable(false);
        Button_MakeMaster->Enable(false);
        Button_ViewUp->Enable(false);
        Button_ViewDown->Enable(false);
    }
    else {
        Button_DeleteView->Enable(true);
        ButtonRename->Enable(true);
        Button_MakeMaster->Enable(true);
        Button_ViewUp->Enable(true);
        Button_ViewDown->Enable(true);
    }

    if (GetSelectedItemCount() > 0) {
        Button_MoveUp->Enable(true);
        Button_MoveDown->Enable(true);
        Button_Bottom->Enable(true);
        Button_Top->Enable(true);
    }
    else {
        Button_MoveUp->Enable(false);
        Button_MoveDown->Enable(false);
        Button_Bottom->Enable(false);
        Button_Top->Enable(false);
    }

    if (_seqData == nullptr || _seqData->NumFrames() == 0) {
        Button_AddView->Enable(false);
    }
    else {
        Button_AddView->Enable(true);
    }
}

void ViewsModelsPanel::UpdateModelsForSelectedView()
{
    if (_sequenceElements == nullptr || _sequenceViewManager == nullptr) return;

    int currentView = _sequenceElements->GetCurrentView();
    if (currentView != MASTER_VIEW) {
        wxString viewName = ListCtrlViews->GetItemText(currentView, 1);
        wxString models = "";
        for (int i = 0; i < _sequenceElements->GetElementCount(currentView); i++) {
            Element* elem = _sequenceElements->GetElement(i, currentView);
            if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                if (models != "") {
                    models += ",";
                }
                models += elem->GetName();
            }
        }

        SequenceView* view = _sequenceViewManager->GetView(viewName.ToStdString());
        if (view != nullptr) view->SetModels(models.ToStdString());
    }
    PopulateModels();
}

#pragma region View List
void ViewsModelsPanel::OnListCtrlViewsItemSelect(wxListEvent& event)
{
    int index = event.m_itemIndex;
    SelectView(ListCtrlViews->GetItemText(index, 1).ToStdString());
    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlItemCheck(wxCommandEvent& event)
{
    Element* e = (Element*)event.GetClientData();
    if (e == nullptr) {
        auto sv = _sequenceViewManager->GetSelectedView();

        int selected = 0;
        const int itemCount = ListCtrlViews->GetItemCount();
        for (int i = 0; i < itemCount; i++) {
            if (ListCtrlViews->IsChecked(i)) {
                selected++;
            }
        }

        if (selected == 0) {
            SelectView(sv->GetName());
        }
        else if (selected == 2) {
            for (int i = 0; i < itemCount; i++) {
                if (ListCtrlViews->IsChecked(i)) {
                    if (sv->GetName() == ListCtrlViews->GetItemText(i, 1)) {
                        ListCtrlViews->SetChecked(i, false);
                    }
                    else {
                        SelectView(ListCtrlViews->GetItemText(i, 1).ToStdString());
                    }
                }
            }
        }
        else {
            bool found = false;
            for (int i = 0; i < itemCount; i++) {
                if (ListCtrlViews->IsChecked(i)) {
                    if (!found) {
                        found = true;
                        SelectView(ListCtrlViews->GetItemText(i, 1).ToStdString());
                    }
                    else {
                        ListCtrlViews->SetChecked(i, false);
                    }
                }
            }
        }
    }
    else {
        e->SetVisible(!e->GetVisible());
        TimingElement* te = dynamic_cast<TimingElement*>(e);
        if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW && te != nullptr) {
            te->SetMasterVisible(e->GetVisible());
        }
    }

    MarkViewsChanged();

    // Update Grid
    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

bool ViewsModelsPanel::HasView(const std::string& view)
{
    return GetViewIndex(view) >= 0;
}

void ViewsModelsPanel::SelectView(const std::string& view)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    if (view != _sequenceViewManager->GetSelectedView()->GetName()) {
        ClearUndo();
    }

    ListCtrlViews->SetChecked(_sequenceElements->GetCurrentView(), false);

    int selected_view = GetViewIndex(view);
    if (selected_view > 0) {
        std::string modelsString = _sequenceElements->GetViewModels(view);
        _sequenceElements->AddMissingModelsToSequence(modelsString);
        _sequenceElements->PopulateView(modelsString, selected_view);
    }
    _sequenceElements->SetCurrentView(selected_view);
    _sequenceElements->SetTimingVisibility(view);
    PopulateModels();
    ListCtrlViews->SetChecked(_sequenceElements->GetCurrentView(), true);
    _mainViewsChoice->SetStringSelection(view);

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

int ViewsModelsPanel::GetViewIndex(const wxString& name)
{
    return _sequenceViewManager->GetViewIndex(name.ToStdString());
}

void ViewsModelsPanel::MarkViewsChanged()
{
    wxCommandEvent eventRgbEffects(EVT_RGBEFFECTS_CHANGED);
    wxPostEvent(GetParent(), eventRgbEffects);
}

// This gives the panel a pointer to the view choice box on the sequencer tab
void ViewsModelsPanel::SetViewChoice(wxChoice* choice)
{
    _mainViewsChoice = choice;

    // this directs events from sequencer tab
    _mainViewsChoice->Connect(wxEVT_CHOICE, (wxObjectEventFunction)&ViewsModelsPanel::OnViewSelect, nullptr, this);
}

void ViewsModelsPanel::OnViewSelect(wxCommandEvent &event) {
    SelectView(_mainViewsChoice->GetString(_mainViewsChoice->GetSelection()).ToStdString());
    ValidateWindow();
}

void ViewsModelsPanel::PopulateViews()
{
    ListCtrlViews->ClearAll();
    if (_mainViewsChoice != nullptr) {
        _mainViewsChoice->Clear();
    }

    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_(""));
    col0.SetWidth(30);
    col0.SetAlign(wxLIST_FORMAT_CENTER);
    ListCtrlViews->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetText(_("View"));
    ListCtrlViews->InsertColumn(1, col1);

    _numViews = 0;
    auto views = _sequenceViewManager->GetViews();
    for (const auto& it : views) {
        bool isChecked = _sequenceViewManager->GetSelectedView() == it;
        AddViewToList(it->GetName(), isChecked);
        if (isChecked) {
            ListCtrlViews->SetChecked(0, false);
        }
    }
    ListCtrlViews->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrlViews->GetColumnWidth(0) < 30) {
        ListCtrlViews->SetColumnWidth(0, 30);
    }
    ListCtrlViews->SetColumnWidth(1, wxLIST_AUTOSIZE);
    _mainViewsChoice->SetSelection(_sequenceViewManager->GetSelectedViewIndex());
}

void ViewsModelsPanel::AddViewToList(const wxString& viewName, bool isChecked)
{
    wxListItem li;
    li.SetId(_numViews);
    ListCtrlViews->InsertItem(li);
    ListCtrlViews->SetItem(_numViews, 1, viewName);
    ListCtrlViews->SetChecked(_numViews, isChecked);
    _numViews++;

    _mainViewsChoice->Append(viewName);
}

void ViewsModelsPanel::OnButton_AddViewClick(wxCommandEvent& event)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    std::string viewName = "";
    int DlgResult = wxID_OK;

    do {
        wxTextEntryDialog dialog(this, _("Enter Name for View"), _("Create View"));
        DlgResult = dialog.ShowModal();
        viewName = dialog.GetValue().Trim().ToStdString();
    } while (DlgResult == wxID_OK && (viewName == "" || _sequenceViewManager->GetView(viewName) != nullptr));

    if (DlgResult != wxID_OK) return;

    _sequenceViewManager->AddView(viewName);
    AddViewToList(viewName, true);
    _sequenceElements->AddView(viewName);
    SelectView(viewName);
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

wxString ViewsModelsPanel::GetMasterViewModels() const
{
    wxArrayString models;
    for (int i = 0; i < _sequenceElements->GetElementCount(); i++) {
        Element* elem = _sequenceElements->GetElement(i);
        if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            models.push_back(elem->GetName());
        }
    }
    return wxJoin(models, ',');
}

void ViewsModelsPanel::OnButtonCloneClick(wxCommandEvent& event)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    int itemIndex = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    std::string oldName = _sequenceElements->GetViewName(itemIndex);

    std::string newName = "Copy Of " + oldName;
    int DlgResult = wxID_OK;

    do {
        wxTextEntryDialog dialog(this, _("Enter Name for View"), _("Create View"), newName);
        DlgResult = dialog.ShowModal();
        newName = dialog.GetValue().Trim().ToStdString();
    } while (DlgResult == wxID_OK && (newName == "" || _sequenceViewManager->GetView(newName) != nullptr));

    if (DlgResult != wxID_OK) return;

    SequenceView* view = _sequenceViewManager->AddView(newName);
    if (itemIndex == MASTER_VIEW) {
        wxArrayString models;
        for (int i = 0; i < _sequenceElements->GetElementCount(); i++) {
            Element* elem = _sequenceElements->GetElement(i);
            if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                models.push_back(elem->GetName());
            }
        }
        view->SetModels(wxJoin(models, ',').ToStdString());
    }
    else {
        view->SetModels(_sequenceViewManager->GetView(oldName)->GetModelsString());
    }

    AddViewToList(newName, true);
    _sequenceElements->AddView(newName);

    std::vector<std::string> timings;
    for (int i = 0; i < _sequenceElements->GetElementCount(itemIndex); i++) {
        Element* elem = _sequenceElements->GetElement(i);
        if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            timings.push_back(elem->GetName());
        }
    }
    _sequenceElements->AddViewToTimings(timings, view->GetName());
    _sequenceElements->SetTimingVisibility(view->GetName());

    SelectView(newName);
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlViewsItemDClick(wxListEvent& event)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;
    if (event.GetIndex() > 0) {
        RenameView(event.GetIndex());
    }
}

void ViewsModelsPanel::RenameView(int itemIndex)
{
    std::string oldName = _sequenceElements->GetViewName(itemIndex);

    std::string newName = oldName;
    int DlgResult = wxID_OK;

    do {
        wxTextEntryDialog dialog(this, _("Enter Name for View"), _("Create View"), newName);
        DlgResult = dialog.ShowModal();
        newName = dialog.GetValue().Trim().ToStdString();
    } while (DlgResult == wxID_OK && (newName == "" || _sequenceViewManager->GetView(newName) != nullptr));

    if (DlgResult != wxID_OK || oldName == newName) return;

    _sequenceViewManager->RenameView(oldName, newName);

    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

void ViewsModelsPanel::OnButtonRenameClick(wxCommandEvent& event)
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    int itemIndex = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    RenameView(itemIndex);
}

void ViewsModelsPanel::DeleteSelectedView()
{
    if (_seqData == nullptr || _seqData->NumFrames() == 0) return;

    int result = wxMessageBox("Are you sure you want to delete this View?", "Confirm Deletion", wxOK | wxCANCEL | wxCENTER);
    if (result != wxOK) return;

    ListCtrlViews->Freeze();
    long itemIndex = -1;

    for (;;) {
        itemIndex = ListCtrlViews->GetNextItem(itemIndex,
            wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);

        if (itemIndex == -1) break;

        // Got a selected item so handle it
        if (itemIndex > 0)  // don't delete master view
        {
            _sequenceElements->RemoveView(itemIndex);
            wxString name = _sequenceElements->GetViewName(itemIndex);
            _sequenceViewManager->DeleteView(name.ToStdString());

            ListCtrlViews->DeleteItem(itemIndex);
            _numViews--;
            break;
        }
        itemIndex = -1; // reset to delete next item which may have same index
    }
    ListCtrlViews->Thaw();
    ListCtrlViews->Refresh();
    _sequenceElements->SetCurrentView(MASTER_VIEW);
    SelectView("Master View");
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

void ViewsModelsPanel::OnButton_DeleteViewClick(wxCommandEvent& event)
{
    DeleteSelectedView();
}
#pragma endregion

void ViewsModelsPanel::OnLeftUp(wxMouseEvent& event)
{
    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

#pragma region Non Models

void ViewsModelsPanel::AddTimingToNotList(Element* timing)
{
    if (timing != nullptr) {
        wxListItem li;
        li.SetId(_numNonModels);
        li.SetText(_(""));
        ListCtrlNonModels->InsertItem(li, 0);
        ListCtrlNonModels->SetItemPtrData(_numNonModels, (wxUIntPtr)timing);
        ListCtrlNonModels->SetItem(_numNonModels, 1, timing->GetName());

        _numNonModels++;
    }
}

void ViewsModelsPanel::AddModelToNotList(Element* model)
{
    if (model != nullptr) {
        wxListItem li;
        li.SetId(_numNonModels);
        li.SetText(_(""));
        if (IsModelAGroup(model->GetName())) {
            ListCtrlNonModels->InsertItem(li, 1);
        }
        else {
            ListCtrlNonModels->InsertItem(li, -1);
        }
        ListCtrlNonModels->SetItemPtrData(_numNonModels, (wxUIntPtr)model);
        ListCtrlNonModels->SetItem(_numNonModels, 1, model->GetName());

        _numNonModels++;
    }
}

void ViewsModelsPanel::OnListCtrlNonModelsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlNonModelsBeginDrag(wxListEvent& event)
{
    if (ListCtrlNonModels->GetSelectedItemCount() == 0) return;

    _dragRowModel = false;
    _dragRowNonModel = true;

    wxString drag = "NonModel";
    for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlNonModels, i)) {
            drag += "," + ListCtrlNonModels->GetItemText(i, 1);
        }
    }

    wxTextDataObject my_data(drag);
    VMDropSource dragSource(this, false, true);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);

    ValidateWindow();
}

void ViewsModelsPanel::OnListCtrlNonModelsKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_RIGHT) {
        AddSelectedModels();
    }
    ValidateWindow();
}

#pragma endregion Non Models

#pragma region Models

void ViewsModelsPanel::OnListCtrlModelsItemRClick(wxListEvent& event)
{
    ListCtrlModels->SetFocus();

    int items = ListCtrlModels->GetItemCount();
    int models = items - GetTimingCount();
    bool isGroup = false;
    if (event.GetIndex() != -1) {
        isGroup = IsModelAGroup(ListCtrlModels->GetItemText(event.GetIndex(), 2).ToStdString());
    }

    wxMenu mnu;
    mnu.Append(ID_MODELS_UNDO, "Undo")->Enable(_undo.size() > 0);
    mnu.Append(ID_MODELS_HIDEALL, "Hide All")->Enable(items > 0);
    mnu.Append(ID_MODELS_HIDEUNUSED, "Hide Unused")->Enable(items > 0);
    mnu.Append(ID_MODELS_SHOWALL, "Show All")->Enable(items > 0);
    mnu.Append(ID_MODELS_REMOVEUNUSED, "Remove Unused")->Enable(items > 0);
    mnu.Append(ID_MODELS_SELECTUNUSED, "Select Unused")->Enable(items > 0);
    mnu.Append(ID_MODELS_SELECTUSED, "Select Used")->Enable(items > 0);
    mnu.Append(ID_MODELS_SELECTALL, "Select All")->Enable(items > 0);

    wxMenu* mnuSort = new wxMenu();

    mnuSort->Append(ID_MODELS_SORTBYNAME, "By Name")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYNAMEGM, "By Name But Groups At Top")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYNAMEGMSIZE, "By Name But Groups At Top by Size")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYNAMEGMCOUNT, "By Name But Groups At Top by Node Count")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYCPGM, "By Controller/Port But Groups At Top")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYCPGMSIZE, "By Controller/Port But Groups At Top by Size")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYSCGM, "By Start Channel But Groups At Top")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTBYSCGMSIZE, "By Start Channel But Groups At Top by Size")->Enable(models > 0);
    if (_sequenceViewManager->GetSelectedViewIndex() != MASTER_VIEW) {
        mnuSort->Append(ID_MODELS_SORTBYMASTERVIEW, "The Same as Current Master View")->Enable(models > 0);
    }
    mnuSort->Append(ID_MODELS_SORTBYTYPE, "By Type")->Enable(models > 0);
    mnuSort->Append(ID_MODELS_SORTMODELSUNDERTHISGROUP, "Models Under This Group")->Enable(isGroup);
    mnuSort->Append(ID_MODELS_BUBBLEUPGROUPS, "Bubble Up Groups")->Enable(models > 0);

    mnuSort->Connect(wxEVT_MENU, (wxObjectEventFunction)&ViewsModelsPanel::OnModelsPopup, nullptr, this);

    mnu.Append(ID_MODELS_SORT, "Sort", mnuSort, "");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ViewsModelsPanel::OnModelsPopup, nullptr, this);

    PopupMenu(&mnu);
    ListCtrlModels->SetFocus();
}

void ViewsModelsPanel::OnModelsPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    int item = ListCtrlModels->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (id == ID_MODELS_HIDEALL) {
        ShowAllModels(false);
    }
    else if (id == ID_MODELS_UNDO) {
        Undo();
    }
    else if (id == ID_MODELS_SHOWALL) {
        ShowAllModels(true);
    }
    else if (id == ID_MODELS_SELECTALL) {
        SelectAllModels();
    }
    else if (id == ID_MODELS_HIDEUNUSED) {
        HideUnusedModels();
    }
    else if (id == ID_MODELS_REMOVEUNUSED) {
        RemoveUnusedModels();
    }
    else if (id == ID_MODELS_SELECTUNUSED) {
        SelectUnusedModels();
    }
    else if (id == ID_MODELS_SELECTUSED) {
        SelectUsedModels();
    }
    else if (id == ID_MODELS_SORTBYNAME) {
        SortModelsByName();
    }
    else if (id == ID_MODELS_SORTBYNAMEGM) {
        SortModelsByNameGM();
    }
    else if (id == ID_MODELS_SORTBYCPGM) {
        SortModelsByCPGM();
    }
    else if (id == ID_MODELS_SORTBYSCGM) {
        SortModelsBySCGM();
    }
    else if (id == ID_MODELS_SORTBYTYPE) {
        SortModelsByType();
    }
    else if (id == ID_MODELS_SORTMODELSUNDERTHISGROUP) {
        SortModelsUnderThisGroup(item);
    }
    else if (id == ID_MODELS_BUBBLEUPGROUPS) {
        SortModelsBubbleUpGroups();
    }
    else if (id == ID_MODELS_SORTBYNAMEGMSIZE) {
        SortModelsByNameGM(true, false);
    }
    else if (id == ID_MODELS_SORTBYNAMEGMCOUNT) {
        SortModelsByNameGM(false, true);
    }
    else if (id == ID_MODELS_SORTBYCPGMSIZE) {
        SortModelsByCPGM(true);
    }
    else if (id == ID_MODELS_SORTBYSCGMSIZE) {
        SortModelsBySCGM(true);
    }
    else if (id == ID_MODELS_SORTBYMASTERVIEW) {
        SortModelsByMasterView();
    }
    ValidateWindow();
}

void ViewsModelsPanel::ShowAllModels(bool show)
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        ListCtrlModels->SetChecked(i, show);
        ((Element*)ListCtrlModels->GetItemData(i))->SetVisible(show);
    }
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::HideUnusedModels()
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        Element* element = (Element*)ListCtrlModels->GetItemData(i);
        if (!element->HasEffects()) {
            ListCtrlModels->SetChecked(i, false);
            ((Element*)ListCtrlModels->GetItemData(i))->SetVisible(false);
        }
    }
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::SelectUnusedModels()
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        Element* element = (Element*)ListCtrlModels->GetItemData(i);
        if (!element->HasEffects()) {
            SelectItem(ListCtrlModels, i, true);
        }
        else {
            SelectItem(ListCtrlModels, i, false);
        }
    }
}

void ViewsModelsPanel::SelectUsedModels()
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        Element* element = (Element*)ListCtrlModels->GetItemData(i);
        if (!element->HasEffects()) {
            SelectItem(ListCtrlModels, i, false);
        }
        else {
            SelectItem(ListCtrlModels, i, true);
        }
    }
}

void ViewsModelsPanel::RemoveUnusedModels()
{
    SelectUnusedModels();
    RemoveSelectedModels();
    _xlFrame->DoForceSequencerRefresh();
}

void ViewsModelsPanel::SelectAllModels()
{
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        SelectItem(ListCtrlModels, i, true);
    }
}

void ViewsModelsPanel::SortModelsByName()
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();
    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');
    modelArray.Sort(wxStringNumberAwareStringCompare);

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

wxArrayString ViewsModelsPanel::MergeStringArrays(const wxArrayString& arr1, const wxArrayString& arr2)
{
    wxArrayString res = arr1;

    for (const auto& it : arr2) {
        res.push_back(it);
    }

    return res;
}

void ViewsModelsPanel::SetMasterViewModels(const wxArrayString& models)
{
    for (size_t i = 0; i < models.size(); ++i) {
        int index = _sequenceElements->GetElementIndex(models[i].ToStdString(), MASTER_VIEW);
        if (index < 0) {
            _sequenceElements->AddElement(i + GetTimingCount(), models[i].ToStdString(), "model", true, false, false, false, false);
        }
        else {
            _sequenceElements->MoveSequenceElement(index, i + GetTimingCount(), MASTER_VIEW);
        }
    }
}

void ViewsModelsPanel::SortModelsByNameGM(bool sortGroupsBySize, bool sortGroupsByCount)
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();

    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    wxArrayString groups;
    wxArrayString modelsOnly;
    std::vector<std::pair<std::string, int>> nodeCounts;

    for (const auto& it : modelArray) {
        if (IsModelAGroup(it.ToStdString())) {
            groups.push_back(it);
            if (sortGroupsByCount) {
                int res = GetPixelCount(it.ToStdString());
                nodeCounts.push_back(std::make_pair(it, res));
            }
        }
        else {
            modelsOnly.push_back(it);
        }
    }

    if (!sortGroupsBySize && !sortGroupsByCount) {
        groups.Sort(wxStringNumberAwareStringCompare);
    } else {
        if (sortGroupsBySize) {
            // groups with more models to the top, i.e whole house models
            auto sortRuleLambda = [this](wxString const& s1, wxString const& s2) -> bool {
                return GetGroupModels(s1).GetCount() > GetGroupModels(s2).GetCount();
            };

            std::sort(groups.begin(), groups.end(), sortRuleLambda);
        } else {
            // groups with more pixels to the top, i.e whole house models regardless of groups in groups
            std::sort(nodeCounts.begin(), nodeCounts.end(), [](const auto& a, const auto& b) {
                return a.second > b.second;
            });
            groups.Clear();
            for (const auto& pair : nodeCounts) {
                groups.Add(pair.first);
            }
        }
    }
    modelsOnly.Sort(wxStringNumberAwareStringCompare);

    modelArray = MergeStringArrays(groups, modelsOnly);

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsByCPGM(bool sortGroupsBySize)
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();

    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    wxArrayString groups;
    wxArrayString modelsOnly;

    for (const auto& it : modelArray) {
        if (IsModelAGroup(it.ToStdString())) {
            groups.push_back(it);
        }
        else {
            modelsOnly.push_back(it);
        }
    }

    if (!sortGroupsBySize) {
        groups.Sort(wxStringNumberAwareStringCompare);
    }
    else {
        //groups with more models to the top, i.e whole house models
        auto sortRuleLambda = [this](wxString const& s1, wxString const& s2) -> bool {
            return GetGroupModels(s1).GetCount() > GetGroupModels(s2).GetCount();
        };

        std::sort(groups.begin(), groups.end(), sortRuleLambda);
    }

    auto cpSortRuleLambda = [this](wxString const& s1, wxString const& s2) -> bool {
        return GetModelCPSortString(s1) < GetModelCPSortString(s2);
    };
    std::sort(modelsOnly.begin(), modelsOnly.end(), cpSortRuleLambda);

    modelArray = MergeStringArrays(groups, modelsOnly);

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsBySCGM(bool sortGroupsBySize)
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();

    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    wxArrayString groups;
    wxArrayString modelsOnly;

    for (const auto& it : modelArray) {
        if (IsModelAGroup(it.ToStdString())) {
            groups.push_back(it);
        }
        else {
            modelsOnly.push_back(it);
        }
    }

    if (!sortGroupsBySize) {
        groups.Sort(wxStringNumberAwareStringCompare);
    }
    else {
        //groups with more models to the top, i.e whole house models
        auto sortRuleLambda = [this](wxString const& s1, wxString const& s2) -> bool {
            return GetGroupModels(s1).GetCount() > GetGroupModels(s2).GetCount();
        };

        std::sort(groups.begin(), groups.end(), sortRuleLambda);
    }

    auto cpSortRuleLambda = [this](wxString const& s1, wxString const& s2) -> bool {
        return GetModelSC(s1) < GetModelSC(s2);
    };
    std::sort(modelsOnly.begin(), modelsOnly.end(), cpSortRuleLambda);

    modelArray = MergeStringArrays(groups, modelsOnly);

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsByMasterView()
{
    SequenceView* view = _sequenceViewManager->GetSelectedView();
    auto v_models = view->GetModelsString();
    wxArrayString v_models_array = wxSplit(v_models, ',');

    auto mv_models = GetMasterViewModels();
    wxArrayString mv_models_array = wxSplit(mv_models, ',');

    wxArrayString modelArray;

    for (const auto& it : mv_models_array) {
        if (std::find(begin(v_models_array), end(v_models_array), it) != end(v_models_array)) {
            modelArray.push_back(it);
        }
    }

    // we should not have dropped any models
    wxASSERT(v_models_array.size() == modelArray.size());

    view->SetModels(wxJoin(modelArray, ',').ToStdString());

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsByType()
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();
    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    wxArrayString groups;
    std::map<std::string, wxArrayString> typeModels;

    for (const auto& it : modelArray) {
        if (IsModelAGroup(it.ToStdString())) {
            groups.push_back(it);
        }
        else {
            std::string type = GetModelType(it.ToStdString());
            if (typeModels.find(type) == typeModels.end()) {
                wxArrayString arr;
                arr.push_back(it);
                typeModels[type] = arr;
            }
            else {
                typeModels[type].push_back(it);
            }
        }
    }

    modelArray = groups;

    for (const auto& it : typeModels) {
        modelArray = MergeStringArrays(modelArray, it.second);
    }

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsUnderThisGroup(int groupIndex)
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();
    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    std::string group = modelArray[groupIndex - GetTimingCount()].ToStdString();

    wxArrayString groupModels = GetGroupModels(group);

    for (auto it = groupModels.rbegin(); it != groupModels.rend(); ++it) {
        if (std::find(modelArray.begin(), modelArray.end(), *it) != modelArray.end()) {
            modelArray.erase(std::find(modelArray.begin(), modelArray.end(), *it));
            auto groupit = std::find(modelArray.begin(), modelArray.end(), group);
            ++groupit;
            modelArray.insert(groupit, *it);
        }
    }

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::SortModelsBubbleUpGroups()
{
    SaveUndo();

    SequenceView* view = _sequenceViewManager->GetSelectedView();
    wxString models;
    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        models = GetMasterViewModels();
    }
    else {
        models = view->GetModelsString();
    }

    wxArrayString modelArray = wxSplit(models, ',');

    wxArrayString groups;
    wxArrayString modelsOnly;

    for (const auto& it : modelArray) {
        if (IsModelAGroup(it.ToStdString())) {
            groups.push_back(it);
        }
        else {
            modelsOnly.push_back(it);
        }
    }

    modelArray = MergeStringArrays(groups, modelsOnly);

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(modelArray);
    }
    else {
        view->SetModels(wxJoin(modelArray, ',').ToStdString());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();

    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

int ViewsModelsPanel::GetTimingCount()
{
    int timings = 0;

    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        Element* t = (Element*)ListCtrlModels->GetItemData(i);
        if (t != nullptr && t->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            timings++;
        }
        else {
            break;
        }
    }

    return timings;
}

void ViewsModelsPanel::AddTimingToList(Element* timing)
{
    if (timing != nullptr) {
        wxListItem li;
        li.SetId(_numModels);
        ListCtrlModels->InsertItem(li);
        ListCtrlModels->SetItemPtrData(_numModels, (wxUIntPtr)timing);
        ListCtrlModels->SetItem(_numModels, 2, timing->GetName());
        ListCtrlModels->SetChecked(_numModels, timing->GetVisible());
        ListCtrlModels->SetItemColumnImage(_numModels, 1, TIMING_IMAGE);
        _numModels++;
    }
}

void ViewsModelsPanel::AddModelToList(Element* model)
{
    if (model != nullptr) {
        wxListItem li;
        li.SetId(_numModels);
        ListCtrlModels->InsertItem(li);
        ListCtrlModels->SetItemPtrData(_numModels, (wxUIntPtr)model);
        ListCtrlModels->SetItem(_numModels, 2, model->GetName());
        ListCtrlModels->SetChecked(_numModels, model->GetVisible());
        // Need to solve this ... I think we only want images for groups.
        if (IsModelAGroup(model->GetName())) {
            ListCtrlModels->SetItemColumnImage(_numModels, 1, MODEL_IMAGE);
        }
        _numModels++;
    }
}

void ViewsModelsPanel::OnListCtrlViewsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if ((key == WXK_DELETE || key == WXK_NUMPAD_DELETE) && Button_DeleteView->IsEnabled()) {
        DeleteSelectedView();
    }
}

#pragma endregion Models

#pragma region Drag and Drop

wxDragResult MyTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    static int MINSCROLLDELAY = 10;
    static int STARTSCROLLDELAY = 300;
    static int scrollDelay = STARTSCROLLDELAY;
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay) {
        // too soon to scroll again
    }
    else {
        if (_type == "Model" && _list->GetItemCount() > 0) {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);

            for (int i = 0; i < _list->GetItemCount(); ++i) {
                if (i == lastItem) {
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                }
                else {
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
                }
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize) {
                // scroll up
                if (_list->GetTopItem() > 0) {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem() - 1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize) {
                // scroll down
                if (lastItem >= 0 && lastItem < _list->GetItemCount()) {
                    _list->EnsureVisible(lastItem + 1);
                    lastTime = wxGetUTCTimeMillis();
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else {
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }

    return wxDragMove;
}

bool MyTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_VMDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "NonModel") {
        if (_type == "Model") {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }
    else if (parms[0] == "Model") {
        if (_type == "Model") {
            event.SetInt(1);
            wxPostEvent(_owner, event);
            return true;
        }
        else if (_type == "NonModel") {
            event.SetInt(2);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

void ViewsModelsPanel::OnDrop(wxCommandEvent& event)
{
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch (event.GetInt()) {
    case 0:
        // Non model dropped into models (an add)
    {
        int flags = wxLIST_HITTEST_ONITEM;
        long index = ListCtrlModels->HitTest(wxPoint(x, y), flags, nullptr);
        AddSelectedModels(index);
    }
    break;
    case 1:
        // Model dropped into models (a reorder)
    {
        int flags = wxLIST_HITTEST_ONITEM;
        long index = ListCtrlModels->HitTest(wxPoint(x, y), flags, nullptr);

        MoveSelectedModelsTo(index);
    }
    break;
    case 2:
        // Model dropped into non model (a remove)
        RemoveSelectedModels();
        break;
    default:
        break;
    }

    // clear any highlight
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        ListCtrlModels->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
    }
}

#pragma endregion Drag and Drop

#pragma region Undo

void ViewsModelsPanel::SaveUndo()
{
    wxArrayString undo;
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        undo.push_back(ListCtrlModels->GetItemText(i, 2).ToStdString());
    }
    std::string to = wxJoin(undo, ',').ToStdString();
    if (undo.size() > 0 && undo.back() != to) {
        _undo.push_back(to);
    }
}

void ViewsModelsPanel::Undo()
{
    if (_undo.size() == 0) return;

    std::string undo = _undo.back();
    _undo.pop_back();

    std::vector<std::string> timings;
    wxArrayString models;

    wxArrayString arr = wxSplit(undo, ',');

    for (const auto& it : arr) {
        Element* e = _sequenceElements->GetElement(it.ToStdString());
        if (e == nullptr) {
            // this cant be undone
        }
        else {
            if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                timings.push_back(it.ToStdString());
            }
            else {
                models.push_back(it);
            }
        }
    }

    if (_sequenceViewManager->GetSelectedViewIndex() == MASTER_VIEW) {
        SetMasterViewModels(models);
    }
    else {
        _sequenceViewManager->GetSelectedView()->SetModels(wxJoin(models, ',').ToStdString());
    }

    _sequenceElements->DeleteTimingsFromView(_sequenceViewManager->GetSelectedViewIndex());
    if (timings.size() > 0) {
        _sequenceElements->AddViewToTimings(timings, _sequenceViewManager->GetSelectedView()->GetName());
    }

    SelectView(_sequenceViewManager->GetSelectedView()->GetName());
    MarkViewsChanged();
    PopulateModels();
    _xlFrame->DoForceSequencerRefresh();
    ValidateWindow();
}

void ViewsModelsPanel::ClearUndo()
{
    _undo.clear();
}

#pragma endregion Undo

bool ViewsModelsPanel::GetSelectionIsMixed() const
{
    size_t model_count = 0;
    size_t timing_count = 0;

    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            if (((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                timing_count++;
            }
            else {
                model_count++;
            }
        }
    }

    return ((model_count > 0) && (timing_count > 0));
}

size_t ViewsModelsPanel::GetSelectedItemCount() const
{
    size_t count = 0;
    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            count++;
        }
    }
    return count;
}

size_t ViewsModelsPanel::GetSelectedModelCount() const
{
    size_t count = 0;
    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i) && ((Element*)ListCtrlModels->GetItemData(i))->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            count++;
        }
    }
    return count;
}

void ViewsModelsPanel::MoveSelectedModelsTo(int indexTo)
{
    if (GetSelectionIsMixed()) return;

    SaveUndo();
    bool itemsMoved = false;
    int currentView = _sequenceViewManager->GetSelectedViewIndex();

    wxArrayString movedModels;
    int selcnt = 0;
    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            movedModels.push_back(ListCtrlModels->GetItemText(i, 2));

            // we are moving this one
            int from = i;
            int to = indexTo + selcnt;
            if (((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                if (currentView != MASTER_VIEW) {
                    return;  // currently can only move timings in master view
                }

                itemsMoved = true;
                from = _sequenceElements->GetElementIndexOfTimingFromListIndex(from);
                to = _sequenceElements->GetElementIndexOfTimingFromListIndex(to);

                if (indexTo == -1) {
                    // moving to the end
                    to = GetTimingCount();
                }
            }
            else {
                itemsMoved = true;
                from -= GetTimingCount();
                to = indexTo + selcnt - GetTimingCount();

                if (currentView == MASTER_VIEW) {
                    from = _sequenceElements->GetIndexOfModelFromModelIndex(from);
                    to = _sequenceElements->GetIndexOfModelFromModelIndex(to);
                }

                if (indexTo == -1) {
                    // moving to the end
                    to = _sequenceElements->GetElementCount(currentView);
                }
            }

            if (to < 0) to = 0;

            if (from < to) {
                from -= selcnt;
                to -= selcnt;
            }

#ifdef TRACEMOVES
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Timing count in models list: %d", GetTimingCount());
            logger_base.debug("Moving from %d '%s' to %d '%s'", from, (const char*)_sequenceElements->GetElement(from, currentView)->GetName().c_str(),
                to, (const char*)(_sequenceElements->GetElement(to, currentView) == nullptr) ? "N/A" : _sequenceElements->GetElement(to, currentView)->GetName().c_str());
#endif

            _sequenceElements->MoveSequenceElement(from, to, currentView);
            SelectItem(ListCtrlModels, i, false);

            selcnt++;
            i--;
        }
    }

    if (itemsMoved) {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        PopulateModels(wxJoin(movedModels, ',').ToStdString());
        _xlFrame->DoForceSequencerRefresh();
    }
}

void ViewsModelsPanel::OnButtonView_DownClick(wxCommandEvent& event) {
    if (_seqData == nullptr || _seqData->NumFrames() == 0)
        return;

    size_t idx = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (idx != -1 && idx < ListCtrlViews->GetItemCount() - 1) {
        _sequenceViewManager->MoveViewDown(idx);
        MarkViewsChanged();
        PopulateViews();
        ListCtrlViews->SetItemState(idx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        ValidateWindow();
    }
}

void ViewsModelsPanel::OnButtonView_UpClick(wxCommandEvent& event) {
    if (_seqData == nullptr || _seqData->NumFrames() == 0)
        return;

    size_t idx = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (idx > 1) {
        _sequenceViewManager->MoveViewUp(idx);
        MarkViewsChanged();
        PopulateViews();
        ListCtrlViews->SetItemState(idx - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    ValidateWindow();
}

void ViewsModelsPanel::OnButton_MoveUpClick(wxCommandEvent& event)
{
    if (GetSelectionIsMixed()) return;

    SaveUndo();
    bool itemsMoved = false;
    int currentView = _sequenceViewManager->GetSelectedViewIndex();

    wxArrayString movedModels;
    int selcnt = 0;
    int firstsel = -1;
    int timing_count = GetTimingCount();

    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            int from = i;
            int to = from - 1;
            if ((((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING)) {
                if (currentView != MASTER_VIEW) {
                    return;  // currently can only move timings in master view
                }
                else {
                    itemsMoved = true;
                    movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                    from = _sequenceElements->GetElementIndexOfTimingFromListIndex(from);
                    to = _sequenceElements->GetElementIndexOfTimingFromListIndex(to);
                }
            }
            else {
                itemsMoved = true;

                movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                from -= timing_count;
                to = from - 1;

                // Only the master view contains the timings
                if (currentView == MASTER_VIEW) {
                    from = _sequenceElements->GetIndexOfModelFromModelIndex(from);
                    to = _sequenceElements->GetIndexOfModelFromModelIndex(to);
                }
            }

            if (to < 0) return;

            if (((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                if (from == 0) return;
            }

            if (firstsel < 0) {
                firstsel = to;
            }
#ifdef TRACEMOVES
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Timing count in models list: %d", GetTimingCount());
            logger_base.debug("Moving from %d '%s' to %d '%s'", from, (const char*)_sequenceElements->GetElement(from, currentView)->GetName().c_str(),
                to, (const char*)(_sequenceElements->GetElement(to, currentView) == nullptr) ? "N/A" : _sequenceElements->GetElement(to, currentView)->GetName().c_str());
#endif

            _sequenceElements->MoveSequenceElement(from, to, currentView);
            SelectItem(ListCtrlModels, i, false);

            selcnt++;
        }
    }

    if (itemsMoved) {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        PopulateModels(wxJoin(movedModels, ',').ToStdString());
        ListCtrlModels->EnsureVisible(firstsel);
        _xlFrame->DoForceSequencerRefresh();
    }
}

void ViewsModelsPanel::OnButton_TopClick(wxCommandEvent& event)
{
    if (GetSelectionIsMixed()) return;

    SaveUndo();
    bool itemsMoved = false;
    int currentView = _sequenceViewManager->GetSelectedViewIndex();

    wxArrayString movedModels;
    int selcnt = 0;
    int firstsel = -1;
    int timing_count = GetTimingCount();

    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            int from = i;
            int to = selcnt;
            if ((((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING)) {
                if (currentView != MASTER_VIEW) {
                    return;  // currently can only move timings in master view
                }
                else {
                    itemsMoved = true;
                    movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                    from = _sequenceElements->GetElementIndexOfTimingFromListIndex(from);
                    to = _sequenceElements->GetElementIndexOfTimingFromListIndex(to);
                }
            }
            else {
                itemsMoved = true;

                movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                from -= timing_count;

                // Only the master view contains the timings
                if (currentView == MASTER_VIEW) {
                    from = _sequenceElements->GetIndexOfModelFromModelIndex(from);
                    to = _sequenceElements->GetIndexOfModelFromModelIndex(to);
                }
            }

            if (to < 0) return;

            if (firstsel < 0) {
                firstsel = to;
            }

#ifdef TRACEMOVES
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Timing count in models list: %d", timing_count);
            logger_base.debug("Moving from %d '%s' to %d '%s'", from, (const char*)_sequenceElements->GetElement(from, currentView)->GetName().c_str(),
                to, (const char*)(_sequenceElements->GetElement(to, currentView) == nullptr) ? "N/A" : _sequenceElements->GetElement(to, currentView)->GetName().c_str());
#endif

            _sequenceElements->MoveSequenceElement(from, to, currentView);
            SelectItem(ListCtrlModels, i, false);

            selcnt++;
        }
    }

    if (itemsMoved) {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        PopulateModels(wxJoin(movedModels, ',').ToStdString());
        ListCtrlModels->EnsureVisible(firstsel);
        _xlFrame->DoForceSequencerRefresh();
    }
}

void ViewsModelsPanel::RemoveModelFromLists(const std::string& modelName)
{
    for (size_t i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (ListCtrlModels->GetItemText(i, 1) == modelName) {
            ListCtrlModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
            break;
        }
    }
    for (size_t i = 0; i < ListCtrlNonModels->GetItemCount(); ++i) {
        if (ListCtrlNonModels->GetItemText(i, 1) == modelName) {
            ListCtrlNonModels->SetItemPtrData(i, (wxUIntPtr)nullptr);
            break;
        }
    }
}

void ViewsModelsPanel::DoMakeMaster()
{
    // this should never happen
    if (_sequenceElements == nullptr || _sequenceViewManager == nullptr) return;

    //swapping out the master will completely change the rendering, abort it
    _sequenceElements->GetXLightsFrame()->AbortRender();

    // get the selected view
    SequenceView* view = _sequenceViewManager->GetView(ListCtrlViews->GetItemText(_sequenceElements->GetCurrentView(), 1).ToStdString());
    if (view != nullptr) {
        auto models = view->GetModels();

        //        bool hadEffects = false;
        std::vector<std::string> hadEffects;
        for (int i = 0; i < _sequenceElements->GetElementCount(MASTER_VIEW); ++i) {
            std::string name = _sequenceElements->GetElement(i)->GetFullName();
            if (std::find(models.begin(), models.end(), name) == models.end()) {
                Element* element = _sequenceElements->GetElement(name);

                if (element != nullptr && element->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                    if (!element->HasEffects()) {
                        //remove all not found elements(models) from the current master view without Effects
                        //They will be added back or move into the correct order with the "SetMasterViewModels" call below if needed
                        _sequenceElements->DeleteElementFromView(name, MASTER_VIEW);
                        //_sequenceElements->DeleteElement(name); //this removes models not found in the "new" master view from ALL Views, not just the master view, causing models to disappear from random Views
                        RemoveModelFromLists(name);
                        --i;
                    }
                    else {
                        //                        hadEffects = true;
                        hadEffects.push_back(name);
                    }
                }
            }
        }

        //        if (hadEffects) {
        if (hadEffects.size()) { //show which one(s)
            //            DisplayWarning("One or more models had effects on them so they were not removed.");
            std::string msg = std::to_string(hadEffects.size()) + " model";
            if (hadEffects.size() != 1) msg += "s"; //OCD/grammar police :P
            msg += " had effects, were not removed: ";
            for (const auto &name: hadEffects) {
                msg += name + ", ";
            }
            msg.pop_back();
            msg.pop_back(); //drop last delimiter
            DisplayWarning(msg);
        }

        // While all models might already be there they are likely not in the right order
        // add the missing models and sort them
        std::string m = view->GetModelsString();
        wxArrayString ms = wxSplit(m, ',');
        SetMasterViewModels(ms);

        // Now select the master view
        SelectView("Master View");

        // Deselect all items
        int itemIndex = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        while (itemIndex != -1) {
            ListCtrlViews->SetItemState(itemIndex, 0, wxLIST_STATE_SELECTED);
            itemIndex = ListCtrlViews->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }

        ValidateWindow();
    }
}

void ViewsModelsPanel::OnButton_MakeMasterClick(wxCommandEvent& event)
{
    DoMakeMaster();
}

int wxCALLBACK MyCompareFunctionVMPAsc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    return *(Element*)item1 == *(Element*)item2 ? 0 : ((*(Element*)item1 < *(Element*)item2) ? -1 : 1);
}

int wxCALLBACK MyCompareFunctionVMPDesc(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    return *(Element*)item1 == *(Element*)item2 ? 0 : ((*(Element*)item1 < *(Element*)item2) ? 1 : -1);
}

void ViewsModelsPanel::SortNonModels()
{
    if (_sortOrder == 0) {
        ListCtrlNonModels->SortItems(MyCompareFunctionVMPAsc, (wxIntPtr)ListCtrlNonModels);
    }
    else {
        ListCtrlNonModels->SortItems(MyCompareFunctionVMPDesc, (wxIntPtr)ListCtrlNonModels);
    }
}

void ViewsModelsPanel::OnListCtrlNonModelsColumnClick(wxListEvent& event)
{
    if (_sortOrder == 0) {
        _sortOrder = 1;
    }
    else {
        _sortOrder = 0;
    }
    SortNonModels();
}

void ViewsModelsPanel::OnListCtrlNonModelsItemActivated(wxListEvent& event)
{
    wxCommandEvent e;
    OnButton_AddSelectedClick(e);
}

void ViewsModelsPanel::OnListCtrlModelsItemActivated(wxListEvent& event)
{
    wxCommandEvent e;
    OnButton_RemoveSelectedClick(e);
}

void ViewsModelsPanel::OnButtonImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(VIEWSMODELS_IMPORT_RGBEFFECTS, "View From Layout File");
    mnu.Append(VIEWSMODELS_IMPORT_SEQUENCE, "Master View From Sequence File");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ViewsModelsPanel::OnImportBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void ViewsModelsPanel::OnImportBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == VIEWSMODELS_IMPORT_RGBEFFECTS) {
        ImportRGBEffectsView();
    }
    else if (event.GetId() == VIEWSMODELS_IMPORT_SEQUENCE) {
        ImportSequenceMasterView();
    }
}

void ViewsModelsPanel::ImportRGBEffectsView()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
#ifdef __WXOSX__
    wxString const wildcard = "*.xml";
#else
    wxString const wildcard = "xlights_rgbeffects.xml";
#endif
    wxString const filename = wxFileSelector(_("Choose RGB Effects file to import from"), wxEmptyString,
        XLIGHTS_RGBEFFECTS_FILE, wxEmptyString,
        "RGB Effects Files (xlights_rgbeffects.xml)|" + wildcard,
        wxFD_FILE_MUST_EXIST | wxFD_OPEN);

    if (filename.IsEmpty()) return;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Importing View From: %s", (const char*)filename.c_str());

    wxXmlDocument doc;
    doc.Load(filename);
    if (doc.IsOk()) {
        wxXmlNode* views = nullptr;
        for (wxXmlNode* node = doc.GetRoot()->GetChildren(); node != nullptr; node = node->GetNext()) {
            if (node->GetName() == "views") {
                views = node;
                break;
            }
        }

        if (views) {
            std::map<wxString, wxString> viewMap;
            wxArrayString viewList;

            for (wxXmlNode* node = views->GetChildren(); node != nullptr; node = node->GetNext()) {
                wxString const view_name = node->GetAttribute("name");
                wxString const view_models = node->GetAttribute("models");
                viewMap.insert({ view_name , view_models });
                viewList.emplace_back(view_name);
            }

            wxMultiChoiceDialog dlg(GetParent(), "Select View(s) to Import", "Select View(s)", viewList);

            if (dlg.ShowModal() == wxID_OK) {
                std::map<wxString, wxArrayString> viewToAddMap;
                for (auto const& idx : dlg.GetSelections()) {
                    auto const sel_view = viewList.at(idx);
                    auto const sel_models = viewMap.at(sel_view);
                    wxArrayString split_model = wxSplit(sel_models, ',');
                    viewToAddMap.insert({ {sel_view, split_model} });
                }
                ImportViewData(viewToAddMap, wxArrayString());
            }
        }
    }
}

void ViewsModelsPanel::ImportSequenceMasterView()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    wxString const wildcards = "xLights Sequence files (*.xsq)|*.xsq|Old xLights Sequence files (*.xml)|*.xml";
    wxString const filename = wxFileSelector(_("Choose Sequence File to Open"),
        _xlFrame->GetShowDirectory(),
        wxEmptyString, "*.xsq", wildcards,
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (filename.IsEmpty()) return;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Importing Master View From: %s", (const char*)filename.c_str());

    wxXmlDocument doc;
    doc.Load(filename);
    if (doc.IsOk()) {

        wxXmlNode* dispElements = nullptr;
        for (wxXmlNode* node = doc.GetRoot()->GetChildren(); node != nullptr; node = node->GetNext()) {
            if (node->GetName() == "DisplayElements") {
                dispElements = node;
                break;
            }
        }

        if (dispElements) {
            wxArrayString modelList;
            wxArrayString timingList;

            for (wxXmlNode* node = dispElements->GetChildren(); node != nullptr; node = node->GetNext()) {
                wxString const elem_name = node->GetAttribute("name");
                wxString const elem_type = node->GetAttribute("type");

                if (elem_type == "timing") {
                    timingList.emplace_back(elem_name);
                }
                else {
                    modelList.emplace_back(elem_name);
                }
            }
			//std::map <wxString, wxArrayString> x{{ "Imported Master", modelList } };

            ImportViewData({ { "Imported Master", modelList } }, timingList);
        }
    }
}

void ViewsModelsPanel::ImportViewData(std::map<wxString, wxArrayString> const& views, wxArrayString const& timings) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (views.empty()) {
        logger_base.debug("No Views Selected to Import");
        return;
    }

	std::vector<std::string> newtimings;
	for (auto const& tim : timings) {
		Element* elem = _sequenceElements->GetElement(tim);
		if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
			newtimings.push_back(elem->GetName());
			logger_base.debug("Timing Found: %s", (const char*)tim.c_str());
		}
		else {
			logger_base.debug("Timing Not Found: %s", (const char*)tim.c_str());
		}
	}

    wxString lastView;

    for (auto const&[name, models] : views)
    {
        auto const sel_view = CreateUniqueName(name);
        lastView = sel_view;
		logger_base.debug("Importing View: %s", (const char*)sel_view.c_str());
		wxArrayString new_models;
		for (auto const& new_mod : models) {//check if models exists
			Model* m = _xlFrame->GetModel(new_mod);
			if (m != nullptr) {
				new_models.Add(new_mod);
				logger_base.debug("Model Found: %s", (const char*)new_mod.c_str());
			}
			else {
				logger_base.debug("Model Not Found: %s", (const char*)new_mod.c_str());
			}
		}
		auto const new_sel_models = wxJoin(new_models, ',');

		logger_base.debug("Models Found: %s", (const char*)new_sel_models.c_str());

		SequenceView* view = _sequenceViewManager->AddView(sel_view);

		view->SetModels(new_sel_models);

		AddViewToList(sel_view, true);
		_sequenceElements->AddView(sel_view);

		if (!newtimings.empty()) {
			_sequenceElements->AddViewToTimings(newtimings, view->GetName());
			_sequenceElements->SetTimingVisibility(view->GetName());
		}
		else {
			logger_base.debug("No Timings Added to View");
		}
    }

    SelectView(lastView);
    MarkViewsChanged();
    PopulateViews();
    ValidateWindow();
}

wxString ViewsModelsPanel::CreateUniqueName(wxString const& prefix)
{
    if (_sequenceViewManager->GetView(prefix) == nullptr) return prefix;

    wxString n;
    int i = 1;
    do {
        n = wxString::Format("%s_%d", prefix, i++);
    } while (_sequenceViewManager->GetView(n) != nullptr);
    return n;
}

void ViewsModelsPanel::OnButton_MoveDownClick(wxCommandEvent& event)
{
    if (GetSelectionIsMixed()) return;

    SaveUndo();
    bool itemsMoved = false;
    int currentView = _sequenceViewManager->GetSelectedViewIndex();

    wxArrayString movedModels;
    int selcnt = 0;
    int lastsel = -1;
    int timing_count = GetTimingCount();

    for (int i = ListCtrlModels->GetItemCount() - 1; i >= 0; --i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            int from = i;
            int to = from + 2;
            if ((((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING)) {
                if (currentView != MASTER_VIEW) {
                    return;  // currently can only move timings in master view
                }
                else {
                    itemsMoved = true;
                    movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                    from = _sequenceElements->GetElementIndexOfTimingFromListIndex(from);
                    to = _sequenceElements->GetElementIndexOfTimingFromListIndex(to);
                }
            }
            else {
                itemsMoved = true;

                movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                from -= timing_count;
                to = from + 2;

                // Only the master view contains the timings
                if (currentView == MASTER_VIEW) {
                    from = _sequenceElements->GetIndexOfModelFromModelIndex(from);
                    to = _sequenceElements->GetIndexOfModelFromModelIndex(to);
                }
            }

            if (to < 0) to = _sequenceElements->GetElementCount(currentView);
            if (to < 0 || to > _sequenceElements->GetElementCount(currentView)) return;

#ifdef TRACEMOVES
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Timing count in models list: %d", GetTimingCount());
            logger_base.debug("Moving from %d '%s' to %d '%s'", from, (const char*)_sequenceElements->GetElement(from, currentView)->GetName().c_str(),
                to, (const char*)(_sequenceElements->GetElement(to, currentView) == nullptr) ? "N/A" : _sequenceElements->GetElement(to, currentView)->GetName().c_str());
#endif
            if (lastsel < 0) {
                lastsel = to;
            }

            _sequenceElements->MoveSequenceElement(from, to, currentView);
            SelectItem(ListCtrlModels, i, false);

            selcnt++;
            i++;
        }
    }

    if (itemsMoved) {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        PopulateModels(wxJoin(movedModels, ',').ToStdString());
        ListCtrlModels->EnsureVisible(lastsel);
        _xlFrame->DoForceSequencerRefresh();
    }
}

void ViewsModelsPanel::OnButton_BottomClick(wxCommandEvent& event)
{
    if (GetSelectionIsMixed()) return;

    SaveUndo();
    bool itemsMoved = false;
    int currentView = _sequenceViewManager->GetSelectedViewIndex();

    wxArrayString movedModels;
    int selcnt = 0;
    int timing_count = GetTimingCount();

    for (int i = 0; i < ListCtrlModels->GetItemCount(); ++i) {
        if (IsItemSelected(ListCtrlModels, i)) {
            int from = i;
            int to = -1;
            if ((((Element*)ListCtrlModels->GetItemData(i))->GetType() == ElementType::ELEMENT_TYPE_TIMING)) {
                if (currentView != MASTER_VIEW) {
                    return;  // currently can only move timings in master view
                }
                else {
                    itemsMoved = true;
                    movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                    from = _sequenceElements->GetElementIndexOfTimingFromListIndex(from - movedModels.size() + 1);
                    to = _sequenceElements->GetElementIndexOfTimingFromListIndex(timing_count);
                }
            }
            else {
                itemsMoved = true;

                movedModels.push_back(ListCtrlModels->GetItemText(i, 2));
                from -= timing_count;

                // Only the master view contains the timings
                if (currentView == MASTER_VIEW) {
                    from = _sequenceElements->GetIndexOfModelFromModelIndex(from - movedModels.size() + 1);
                    to = _sequenceElements->GetIndexOfModelFromModelIndex(ListCtrlModels->GetItemCount() - 1);
                }
            }

            if (to < 0) to = _sequenceElements->GetElementCount(currentView);
            if (to < 0 || to > _sequenceElements->GetElementCount(currentView)) return;

#ifdef TRACEMOVES
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.debug("Timing count in models list: %d", timing_count);
            logger_base.debug("Moving from %d '%s' to %d '%s'", from, (const char*)_sequenceElements->GetElement(from, currentView)->GetName().c_str(),
                to, (const char*)(_sequenceElements->GetElement(to, currentView) == nullptr) ? "N/A" : _sequenceElements->GetElement(to, currentView)->GetName().c_str());
#endif

            _sequenceElements->MoveSequenceElement(from, to, currentView);
            SelectItem(ListCtrlModels, i, false);

            selcnt++;
            i--;
        }
    }

    if (itemsMoved) {
        MarkViewsChanged();
        UpdateModelsForSelectedView();
        PopulateModels(wxJoin(movedModels, ',').ToStdString());
        ListCtrlModels->EnsureVisible(ListCtrlModels->GetItemCount() - 1);
        _xlFrame->DoForceSequencerRefresh();
    }
}
