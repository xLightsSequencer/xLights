/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListDialog.h"
#include "PlayList.h"
#include "PlayListStep.h"
#include "PlayListItem.h"
#include "../MyTreeItemData.h"
#include "PlayListPanel.h"
#include "PlayListStepPanel.h"
#include "PlayListItemVideo.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemSetColour.h"
#include "PlayListItemRunCommand.h"
#include "PlayListItemMQTT.h"
#include "PlayListItemOSC.h"
#include "PlayListItemRunProcess.h"
#include "PlayListItemCURL.h"
#include "PlayListItemSerial.h"
#include "PlayListItemMIDI.h"
#include "PlayListItemFPPEvent.h"
#include "PlayListItemFile.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemText.h"
#include "PlayListItemScreenMap.h"
#include "PlayListItemFSEQVideo.h"
#include "PlayListItemTest.h"
#include "PlayListItemMicrophone.h"
#include "PlayListItemColourOrgan.h"
#include "PlayListItemRDS.h"
#include "PlayListItemAudio.h"
#include "PlayListItemESEQ.h"
#include "PlayListItemFade.h"
#include "PlayListItemImage.h"
#include "PlayListItemJukebox.h"
#include "PlayListItemDelay.h"
#include "PlayListItemDim.h"
#include "PlayListItemProjector.h"
#include "PlayListItemARTNetTrigger.h"

#include "../xLights/osxMacUtils.h"

#include <wx/xml/xml.h>
#include <wx/menu.h>
#include <wx/notebook.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/confbase.h>
#include <wx/wupdlock.h>

//(*InternalHeaders(PlayListDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListDialog)
const long PlayListDialog::ID_TREECTRL1 = wxNewId();
const long PlayListDialog::ID_STATICTEXT2 = wxNewId();
const long PlayListDialog::ID_BUTTON3 = wxNewId();
const long PlayListDialog::ID_BUTTON4 = wxNewId();
const long PlayListDialog::ID_BUTTON5 = wxNewId();
const long PlayListDialog::ID_BUTTON7 = wxNewId();
const long PlayListDialog::ID_BUTTON6 = wxNewId();
const long PlayListDialog::ID_BUTTON1 = wxNewId();
const long PlayListDialog::ID_BUTTON2 = wxNewId();
const long PlayListDialog::ID_PANEL1 = wxNewId();
const long PlayListDialog::ID_NOTEBOOK1 = wxNewId();
const long PlayListDialog::ID_PANEL2 = wxNewId();
const long PlayListDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long PlayListDialog::ID_MNU_CLONE = wxNewId();
const long PlayListDialog::ID_MNU_ADDSTEP = wxNewId();
const long PlayListDialog::ID_MNU_ADDVIDEO = wxNewId();
const long PlayListDialog::ID_MNU_ADDARTNETTRIGGER = wxNewId();
const long PlayListDialog::ID_MNU_ADDAUDIO = wxNewId();
const long PlayListDialog::ID_MNU_ADDESEQ = wxNewId();
const long PlayListDialog::ID_MNU_ADDFADE = wxNewId();
const long PlayListDialog::ID_MNU_ADDFSEQ = wxNewId();
const long PlayListDialog::ID_MNU_ADDTEXT = wxNewId();
const long PlayListDialog::ID_MNU_ADDSCREENMAP = wxNewId();
const long PlayListDialog::ID_MNU_ADDFILE = wxNewId();
const long PlayListDialog::ID_MNU_ADDFSEQVIDEO = wxNewId();
const long PlayListDialog::ID_MNU_ADDTEST = wxNewId();
const long PlayListDialog::ID_MNU_ADDMICROPHONE = wxNewId();
const long PlayListDialog::ID_MNU_ADDCOLOURORGAN = wxNewId();
const long PlayListDialog::ID_MNU_ADDRDS = wxNewId();
const long PlayListDialog::ID_MNU_ADDPROJECTOR = wxNewId();
const long PlayListDialog::ID_MNU_ADDALLOFF = wxNewId();
const long PlayListDialog::ID_MNU_ADDSETCOLOUR = wxNewId();
const long PlayListDialog::ID_MNU_ADDIMAGE = wxNewId();
const long PlayListDialog::ID_MNU_ADDJUKEBOX = wxNewId();
const long PlayListDialog::ID_MNU_ADDDELAY = wxNewId();
const long PlayListDialog::ID_MNU_ADDDIM = wxNewId();
const long PlayListDialog::ID_MNU_ADDCOMMAND = wxNewId();
const long PlayListDialog::ID_MNU_ADDOSC = wxNewId();
const long PlayListDialog::ID_MNU_ADDMQTT = wxNewId();
const long PlayListDialog::ID_MNU_ADDPROCESS = wxNewId();
const long PlayListDialog::ID_MNU_ADDCURL = wxNewId();
const long PlayListDialog::ID_MNU_ADDSERIAL = wxNewId();
const long PlayListDialog::ID_MNU_ADDMIDI = wxNewId();
const long PlayListDialog::ID_MNU_ADDFPPEVENT = wxNewId();
const long PlayListDialog::ID_MNU_DELETE = wxNewId();
const long PlayListDialog::ID_MNU_REMOVEEMPTYSTEPS = wxNewId();

wxDEFINE_EVENT(EVT_UPDATEITEMNAME, wxCommandEvent);

BEGIN_EVENT_TABLE(PlayListDialog,wxDialog)
	//(*EventTable(PlayListDialog)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_UPDATEITEMNAME, PlayListDialog::UpdateItemName)
END_EVENT_TABLE()

PlayListDialog::PlayListDialog(wxWindow* parent, OutputManager* outputManager, PlayList* playlist, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _playlist = playlist;

	//(*Initialize(PlayListDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, id, _("Play List"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetSashGravity(0.5);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	TreeCtrl_PlayList = new wxTreeCtrl(Panel1, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	TreeCtrl_PlayList->SetMinSize(wxSize(300,300));
	FlexGridSizer3->Add(TreeCtrl_PlayList, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Right click on tree to add steps and advanced items."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_AddFSEQ = new wxButton(Panel1, ID_BUTTON3, _("Add FSEQ"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer1->Add(Button_AddFSEQ, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_FSEQVideo = new wxButton(Panel1, ID_BUTTON4, _("Add FSEQ + Video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(Button_FSEQVideo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_AddAudio = new wxButton(Panel1, ID_BUTTON5, _("Add Audio Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	BoxSizer1->Add(Button_AddAudio, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Clone = new wxButton(Panel1, ID_BUTTON7, _("Clone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	BoxSizer1->Add(Button_Clone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Delete = new wxButton(Panel1, ID_BUTTON6, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	BoxSizer1->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(Panel1, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer5->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(Panel1, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxALIGN_RIGHT, 5);
	Panel1->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel1);
	FlexGridSizer3->SetSizeHints(Panel1);
	Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	Notebook1 = new wxNotebook(Panel2, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	FlexGridSizer4->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel2);
	FlexGridSizer4->SetSizeHints(Panel2);
	SplitterWindow1->SplitVertically(Panel1, Panel2);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FileDialog1 = new wxFileDialog(this, _("Select files"), wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_DEFAULT_STYLE|wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_BEGIN_DRAG,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListBeginDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_END_DRAG,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListEndDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListSelectionChanged);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListKeyDown);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListItemMenu);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_AddFSEQClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_FSEQVideoClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_AddAudioClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_CloneClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_DeleteClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_CancelClick);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&PlayListDialog::OnNotebook1PageChanged);
	//*)

    int x, y, w, h;
    wxConfigBase* config = wxConfigBase::Get();
    x = config->ReadLong(_("xsPLWindowPosX"), 50);
    y = config->ReadLong(_("xsPLWindowPosY"), 50);
    w = config->ReadLong(_("xsPLWindowPosW"), 800);
    h = config->ReadLong(_("xsPLWindowPosH"), 600);
    SetPosition(wxPoint(x, y));
    SetSize(w, h);

    // save the current state in case the user cancels
    _savedState = new PlayList(*playlist);

    _playlist->ConsolidateEveryDay();

    PopulateTree(playlist, nullptr, nullptr);

    TreeCtrl_PlayList->DragAcceptFiles(true);
    TreeCtrl_PlayList->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(PlayListDialog::OnDropFiles), nullptr, this);

    SetAffirmativeId(Button_Ok->GetId());
    SetEscapeId(Button_Cancel->GetId());

    ValidateWindow();
}

PlayListDialog::~PlayListDialog()
{
	//(*Destroy(PlayListDialog)
	//*)

    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("xsPLWindowPosX"), x);
    config->Write(_("xsPLWindowPosY"), y);
    config->Write(_("xsPLWindowPosW"), w);
    config->Write(_("xsPLWindowPosH"), h);
    config->Flush();

    // manually remove the notebook page to force updating now rather than during deletion
    SwapPage(nullptr);
}

void PlayListDialog::SwapPage(wxNotebookPage* newpage, const std::string& text)
{
    if (newpage == nullptr && Notebook1->GetPageCount() == 0) {
        return;
    }

    WINDOW_LOCKER(Panel2, lockPanel);
    WINDOW_LOCKER(Notebook1, lockNotebook);

    if (Notebook1->GetPageCount() > 0)
    {
        wxNotebookPage* p = Notebook1->GetPage(0);

        if (p != nullptr)
        {
             Notebook1->RemovePage(0);
             delete p;
        }
    }

    if (newpage != nullptr)
    {
        Notebook1->AddPage(newpage, text, true);
    }
}

void PlayListDialog::PopulateTree(PlayList* selplaylist, PlayListStep* selstep, PlayListItem* selitem)
{
    TreeCtrl_PlayList->Freeze();

    wxTreeItemId select = nullptr;
    TreeCtrl_PlayList->DeleteAllItems();

    wxTreeItemId id = TreeCtrl_PlayList->AddRoot(_playlist->GetName());
    TreeCtrl_PlayList->SetItemData(id, new MyTreeItemData(_playlist));
    if (selstep == nullptr && selitem == nullptr) select = id;

    auto steps = _playlist->GetSteps();
    for (const auto& it : steps)
    {
        wxTreeItemId step = TreeCtrl_PlayList->AppendItem(TreeCtrl_PlayList->GetRootItem(), it->GetName(_playlist));
        TreeCtrl_PlayList->SetItemData(step, new MyTreeItemData(it));
        if (selitem == nullptr && selstep != nullptr && it->GetId() == selstep->GetId())
        {
            select = step;
        }

        size_t ms;
        PlayListItem* ts = it->GetTimeSource(ms);

        for (const auto& it2 : it->GetItems())
        {
            id = TreeCtrl_PlayList->AppendItem(step, it2->GetName());
            TreeCtrl_PlayList->SetItemData(id, new MyTreeItemData(it2));

            if (selitem != nullptr && it2->GetId() == selitem->GetId()) select = id;

            if (ts != nullptr)
            {
                if (ts->GetId() == it2->GetId())
                {
                    TreeCtrl_PlayList->SetItemTextColour(id, *wxBLUE);
                }
            }
        }
    }

    if (select == nullptr) select = TreeCtrl_PlayList->GetRootItem();
    TreeCtrl_PlayList->ExpandAll();
    TreeCtrl_PlayList->EnsureVisible(select);
    TreeCtrl_PlayList->UnselectAll();
    TreeCtrl_PlayList->SetIndent(8);
    TreeCtrl_PlayList->Thaw();
    TreeCtrl_PlayList->SelectItem(select);
    TreeCtrl_PlayList->Refresh();
}

void PlayListDialog::OnTextCtrl_PlayListNameText(wxCommandEvent& event)
{
    ValidateWindow();
}

bool PlayListDialog::IsPlayList(wxTreeItemId id)
{
    return (id.IsOk() && id == TreeCtrl_PlayList->GetRootItem());
}

bool PlayListDialog::IsPlayListStep(wxTreeItemId id)
{
    return (id.IsOk() && TreeCtrl_PlayList->GetItemParent(id) == TreeCtrl_PlayList->GetRootItem());
}

void PlayListDialog::OnTreeCtrl_PlayListSelectionChanged(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (!treeitem.IsOk()) return;

    if (IsPlayList(treeitem))
    {
        PlayList* pl = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        SwapPage(new PlayListPanel(Notebook1, pl), "Playlist");
    }
    else if (IsPlayListStep(treeitem))
    {
        PlayListStep* pls = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        SwapPage(new PlayListStepPanel(Notebook1, pls), "Playlist Step");
    }
    else
    {
        // must be a playlist entry
        PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        WINDOW_LOCKER(Panel2, lockPanel);
        WINDOW_LOCKER(Notebook1, lockNotebook);
        SwapPage(nullptr);
        pli->Configure(Notebook1);
    }
    ValidateWindow();
}

void PlayListDialog::OnTreeMouseMove(wxMouseEvent& event)
{
    wxTreeItemId dropitem = TreeCtrl_PlayList->HitTest(event.GetPosition());
    wxTreeItemId dragitem = TreeCtrl_PlayList->GetSelection();

    if (!dragitem.IsOk() || !dropitem.IsOk())
    {
        SetCursor(wxCURSOR_NO_ENTRY);
        HighlightDropItem(nullptr);
        return;
    }

    bool canbedropped = true;

    if (dragitem == dropitem)
    {
        // cant be dropped on myself
        canbedropped = false;
    }
    else if (IsPlayListStep(dragitem))
    {
        // can't be dropped on my children
        if (dragitem == TreeCtrl_PlayList->GetItemParent(dropitem))
        {
            canbedropped = false;
        }
    }
    else if (!IsPlayList(dragitem))
    {
        // cant be dropped on a playlist
        if (IsPlayList(dropitem))
        {
            canbedropped = false;
        }
        else if (IsPlayListStep(dropitem))
        {
            // cant be dropped on my parent
            if (TreeCtrl_PlayList->GetItemParent(dragitem) == dropitem)
            {
                canbedropped = false;
            }
        }
        else
        {
            // cant be dropped on my siblings
            if (TreeCtrl_PlayList->GetItemParent(dragitem) == TreeCtrl_PlayList->GetItemParent(dropitem))
            {
                canbedropped = false;
            }
        }
    }

    if (canbedropped)
    {
        SetCursor(wxCURSOR_HAND);
        HighlightDropItem(&dropitem);
    }
    else
    {
        SetCursor(wxCURSOR_NO_ENTRY);
        HighlightDropItem(nullptr);
    }
}

void PlayListDialog::HighlightDropItem(wxTreeItemId* id)
{
    wxTreeItemId root = TreeCtrl_PlayList->GetRootItem();
    TreeCtrl_PlayList->SetItemDropHighlight(root, id != nullptr && *id == root);

    wxTreeItemIdValue tid;
    for (wxTreeItemId it = TreeCtrl_PlayList->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(root, tid))
    {
        TreeCtrl_PlayList->SetItemDropHighlight(it, id != nullptr && *id == it);
        wxTreeItemIdValue tid2;
        for (wxTreeItemId it2 = TreeCtrl_PlayList->GetFirstChild(it, tid2); it2 != nullptr; it2 = TreeCtrl_PlayList->GetNextChild(it, tid2))
        {
            TreeCtrl_PlayList->SetItemDropHighlight(it2, id != nullptr && *id == it2);
        }
    }
}

void PlayListDialog::OnTreeCtrl_PlayListBeginDrag(wxTreeEvent& event)
{
    wxTreeItemId dragitem = TreeCtrl_PlayList->HitTest(event.GetPoint());
    TreeCtrl_PlayList->SelectItem(dragitem);

    if (IsPlayList(dragitem))
    {
        //event.Skip();
        return;
    }

    TreeCtrl_PlayList->Connect(wxEVT_LEFT_UP,
        wxMouseEventHandler(PlayListDialog::OnTreeDragEnd), nullptr, this);
    // trigger when user leaves window to abort drag
    TreeCtrl_PlayList->Connect(wxEVT_LEAVE_WINDOW,
        wxMouseEventHandler(PlayListDialog::OnTreeDragQuit), nullptr, this);
    // trigger when mouse moves
    TreeCtrl_PlayList->Connect(wxEVT_MOTION,
        wxMouseEventHandler(PlayListDialog::OnTreeMouseMove), nullptr, this);

    _dragging = true;
    SetCursor(wxCURSOR_HAND);
}

// abort dragging a list item because user has left window
void PlayListDialog::OnTreeDragQuit(wxMouseEvent& event)
{
    // restore cursor and disconnect unconditionally
    SetCursor(wxCURSOR_ARROW);
    TreeCtrl_PlayList->Disconnect(wxEVT_LEFT_UP,
        wxMouseEventHandler(PlayListDialog::OnTreeDragEnd));
    TreeCtrl_PlayList->Disconnect(wxEVT_LEAVE_WINDOW,
        wxMouseEventHandler(PlayListDialog::OnTreeDragQuit));
    TreeCtrl_PlayList->Disconnect(wxEVT_MOTION,
        wxMouseEventHandler(PlayListDialog::OnTreeMouseMove));
    HighlightDropItem(nullptr);
}

void PlayListDialog::OnTreeDragEnd(wxMouseEvent& event)
{
    wxTreeItemId dropitem = TreeCtrl_PlayList->HitTest(event.GetPosition());
    if (!dropitem.IsOk())
    {
        dropitem = TreeCtrl_PlayList->GetRootItem();
    }
    wxTreeItemId dragitem = TreeCtrl_PlayList->GetSelection();

    if (IsPlayListStep(dragitem))
    {
        PlayListStep* dragstep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(dragitem))->GetData();

        if (!IsPlayList(dropitem) && !IsPlayListStep(dropitem))
        {
            // drop on an item so act like it was dropped on parent
            dropitem = TreeCtrl_PlayList->GetItemParent(dropitem);
        }

        // if dropped on playlist make it the first step
        if (IsPlayList(dropitem))
        {
            _playlist->MoveStepBeforeStep(dragstep, nullptr);
            PopulateTree(_playlist, dragstep, nullptr);
        }
        // if dropped on a step make it the step after the dropped step
        else if (IsPlayListStep(dropitem))
        {
            PlayListStep* dropstep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(dropitem))->GetData();
            _playlist->MoveStepBeforeStep(dragstep, dropstep);
            PopulateTree(_playlist, dragstep, nullptr);
        }
    }
    else if (!IsPlayList(dragitem))
    {
        PlayListItem* dragpli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(dragitem))->GetData();
        PlayListStep* dragstep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(TreeCtrl_PlayList->GetItemParent(dragitem)))->GetData();

        if (!IsPlayListStep(dropitem) && !IsPlayList(dropitem))
        {
            // drop on an item so act like it was dropped on parent
            dropitem = TreeCtrl_PlayList->GetItemParent(dropitem);
        }

        // dragging a play list item
        if (IsPlayList(dropitem))
        {
            // this doesnt make any sense ... do nothing
        }
        else
        {
            PlayListStep* dropstep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(dropitem))->GetData();

            if (dragstep == dropstep)
            {
                // nothing to do
            }
            else
            {
                // remove it from the drag step
                dragstep->RemoveItem(dragpli);

                // add it to the drop step
                dropstep->AddItem(dragpli);
                PopulateTree(_playlist, dropstep, dragpli);
            }
        }
    }

    _dragging = false;
    SetCursor(wxCURSOR_ARROW);
    HighlightDropItem(nullptr);

    // disconnect both functions
    TreeCtrl_PlayList->Disconnect(wxEVT_LEFT_UP,
        wxMouseEventHandler(PlayListDialog::OnTreeDragEnd));
    TreeCtrl_PlayList->Disconnect(wxEVT_LEAVE_WINDOW,
        wxMouseEventHandler(PlayListDialog::OnTreeDragQuit));
    TreeCtrl_PlayList->Disconnect(wxEVT_MOTION,
        wxMouseEventHandler(PlayListDialog::OnTreeMouseMove));
}

void PlayListDialog::OnTreeCtrl_PlayListEndDrag(wxTreeEvent& event)
{
}

void PlayListDialog::OnTreeCtrl_PlayListItemMenu(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->HitTest(event.GetPoint());
    TreeCtrl_PlayList->SelectItem(treeitem);

    wxMenu mnu;
    if (!IsPlayList(treeitem))
    {
        if (IsPlayListStep(treeitem))
        {
            mnu.Append(ID_MNU_CLONE, "Clone Step");
        }
        else
        {
            mnu.Append(ID_MNU_CLONE, "Clone Item");
        }
    }
    mnu.AppendSeparator();
    mnu.Append(ID_MNU_ADDALLOFF, "Add All Set");
    mnu.Append(ID_MNU_ADDARTNETTRIGGER, "Add ARTNet Trigger");
    mnu.Append(ID_MNU_ADDAUDIO, "Add Audio");
    mnu.Append(ID_MNU_ADDCOLOURORGAN, "Add Colour Organ");
    mnu.Append(ID_MNU_ADDCOMMAND, "Add Command");
    mnu.Append(ID_MNU_ADDCURL, "Add CURL");
    mnu.Append(ID_MNU_ADDDELAY, "Add Delay");
    mnu.Append(ID_MNU_ADDDIM, "Add Dim");
    mnu.Append(ID_MNU_ADDESEQ, "Add ESEQ");
    mnu.Append(ID_MNU_ADDFADE, "Add Fade");
    mnu.Append(ID_MNU_ADDFILE, "Add File");
    mnu.Append(ID_MNU_ADDFPPEVENT, "Add FPP Event");
    mnu.Append(ID_MNU_ADDFSEQ, "Add FSEQ");
    mnu.Append(ID_MNU_ADDFSEQVIDEO, "Add FSEQ & Video");
    mnu.Append(ID_MNU_ADDIMAGE, "Add Image");
    mnu.Append(ID_MNU_ADDJUKEBOX, "Add xLights Jukebox");
    mnu.Append(ID_MNU_ADDMICROPHONE, "Add Microphone");
    mnu.Append(ID_MNU_ADDMIDI, "Add MIDI");
    mnu.Append(ID_MNU_ADDMQTT, "Add MQTT");
    mnu.Append(ID_MNU_ADDOSC, "Add OSC");
    mnu.Append(ID_MNU_ADDPROCESS, "Add Process");
    mnu.Append(ID_MNU_ADDPROJECTOR, "Add Projector");
    mnu.Append(ID_MNU_ADDRDS, "Add RDS");
    mnu.Append(ID_MNU_ADDSCREENMAP, "Add Screen Map");
    mnu.Append(ID_MNU_ADDSERIAL, "Add Serial");
    mnu.Append(ID_MNU_ADDSETCOLOUR, "Add Set Colour");
    mnu.Append(ID_MNU_ADDTEST, "Add Test");
    mnu.Append(ID_MNU_ADDTEXT, "Add Text");
    mnu.Append(ID_MNU_ADDVIDEO, "Add Video");

    wxMenuItem* mi = mnu.Append(ID_MNU_ADDSTEP, "Add Step");
    if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_DELETE, "Delete");
    if (IsPlayList(treeitem))
    {
        mi->Enable(false);
    }
    mnu.Append(ID_MNU_REMOVEEMPTYSTEPS, "Remove Empty Steps");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&PlayListDialog::OnTreeCtrlMenu, nullptr, this);
    ModalPopup(this, mnu);
}

int PlayListDialog::GetPos(const wxTreeItemId& item)
{
    wxTreeItemId parent = TreeCtrl_PlayList->GetItemParent(item);

    int i = 0;
    wxTreeItemIdValue tid;
    wxTreeItemId curr = TreeCtrl_PlayList->GetFirstChild(parent, tid);
    while (curr != item)
    {
        curr = TreeCtrl_PlayList->GetNextChild(parent, tid);
        i++;
    }

    return i;
}

void PlayListDialog::OnTreeCtrlMenu(wxCommandEvent &event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    PlayListStep* step = nullptr;
    if (IsPlayListStep(treeitem))
    {
        step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
    }

    if (event.GetId() == ID_MNU_ADDVIDEO)
    {
        PlayListItemVideo* pli = new PlayListItemVideo();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDIMAGE)
    {
        PlayListItemImage* pli = new PlayListItemImage();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDJUKEBOX)
    {
        PlayListItemJukebox* pli = new PlayListItemJukebox();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDALLOFF)
    {
        PlayListItemAllOff* pli = new PlayListItemAllOff(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDSETCOLOUR)
    {
        PlayListItemSetColour* pli = new PlayListItemSetColour(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDDELAY)
    {
        PlayListItemDelay* pli = new PlayListItemDelay();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDDIM)
    {
        PlayListItemDim* pli = new PlayListItemDim(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDPROCESS)
    {
        PlayListItemRunProcess* pli = new PlayListItemRunProcess();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDCURL)
    {
        PlayListItemCURL* pli = new PlayListItemCURL();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDSERIAL)
    {
        PlayListItemSerial* pli = new PlayListItemSerial();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDMIDI)
    {
        PlayListItemMIDI* pli = new PlayListItemMIDI();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDFPPEVENT)
    {
        PlayListItemFPPEvent* pli = new PlayListItemFPPEvent();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDCOMMAND)
    {
        PlayListItemRunCommand* pli = new PlayListItemRunCommand();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDOSC)
    {
        PlayListItemOSC* pli = new PlayListItemOSC();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDMQTT)
    {
        PlayListItemMQTT* pli = new PlayListItemMQTT();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDTEXT)
    {
        PlayListItemText* pli = new PlayListItemText();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDSCREENMAP)
    {
        PlayListItemScreenMap* pli = new PlayListItemScreenMap();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDFILE)
    {
        PlayListItemFile* pli = new PlayListItemFile();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDFSEQ)
    {
        PlayListItemFSEQ* pli = new PlayListItemFSEQ(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDFSEQVIDEO)
    {
        PlayListItemFSEQVideo* pli = new PlayListItemFSEQVideo(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDTEST)
    {
        PlayListItemTest* pli = new PlayListItemTest(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDMICROPHONE)
    {
        PlayListItemMicrophone* pli = new PlayListItemMicrophone(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDCOLOURORGAN)
    {
        PlayListItemColourOrgan* pli = new PlayListItemColourOrgan(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDRDS)
    {
        PlayListItemRDS* pli = new PlayListItemRDS();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDAUDIO)
    {
        PlayListItemAudio* pli = new PlayListItemAudio();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDPROJECTOR)
    {
        PlayListItemProjector* pli = new PlayListItemProjector();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDARTNETTRIGGER)
    {
        PlayListItemARTNetTrigger* pli = new PlayListItemARTNetTrigger();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDESEQ)
    {
        PlayListItemESEQ* pli = new PlayListItemESEQ();
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_ADDFADE)
    {
        PlayListItemFade* pli = new PlayListItemFade(_outputManager);
        AddItem(_playlist, step, pli);
    }
    else if (event.GetId() == ID_MNU_CLONE)
    {
        Clone();
    }
    else if (event.GetId() == ID_MNU_ADDSTEP)
    {
        PlayListStep* pls = new PlayListStep();
        if (IsPlayList(treeitem))
        {
            _playlist->AddStep(pls, 0);
        }
        else
        {
            _playlist->AddStep(pls, GetPos(treeitem)+1);
        }
        PopulateTree(_playlist, pls, nullptr);
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        DeleteSelectedItem();
    }
    else if (event.GetId() == ID_MNU_REMOVEEMPTYSTEPS)
    {
        _playlist->RemoveEmptySteps();
        PopulateTree(_playlist, nullptr, nullptr);
    }
}

void PlayListDialog::DeleteSelectedItem()
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (treeitem.IsOk() && !IsPlayList(treeitem))
    {
        if (wxMessageBox(wxString::Format ("Are you sure you want to delete '%s'?", TreeCtrl_PlayList->GetItemText(treeitem)),
            "Are you sure?", wxYES_NO) == wxYES)
        {
            SwapPage(nullptr);
            if (IsPlayListStep(treeitem))
            {
                PlayListStep* playliststep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(TreeCtrl_PlayList->GetItemParent(treeitem)))->GetData();
                playlist->RemoveStep(playliststep);
                PopulateTree(_playlist, nullptr, nullptr);
            }
            else if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
            {
                PlayListItem* playlistitem = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
                PlayListStep* playliststep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(TreeCtrl_PlayList->GetItemParent(treeitem)))->GetData();
                playliststep->RemoveItem(playlistitem);
                auto items = playliststep->GetItems();
                for (const auto& it : items)
                {
                    it->SetStepLength(playliststep->GetLengthMS());
                }

                PopulateTree(_playlist, playliststep, nullptr);
            }
        }
    }
}

wxTreeItemId PlayListDialog::FindStepTreeItem(PlayListStep* step)
{
    wxTreeItemId root = TreeCtrl_PlayList->GetRootItem();

    wxTreeItemIdValue tid;
    for (wxTreeItemId it = TreeCtrl_PlayList->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(root, tid))
    {
        PlayListStep* pls = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it))->GetData();
        if (pls->GetId() == step->GetId()) return it;
    }

    return wxTreeItemId();
}

void PlayListDialog::Clone()
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (treeitem.IsOk())
    {
        if (IsPlayListStep(treeitem))
        {
            PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
            if (step != nullptr)
            {
                PlayListStep* pls = new PlayListStep(*step);
                pls->SetDirty();
                _playlist->AddStep(pls, GetPos(treeitem) + 1);
                PopulateTree(_playlist, step, nullptr);
            }
        }
        else if (!IsPlayList(treeitem))
        {
            PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
            if (pli != nullptr)
            {
                wxTreeItemId treeitemStep = TreeCtrl_PlayList->GetItemParent(treeitem);
                if (treeitemStep.IsOk())
                {
                    PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitemStep))->GetData();
                    if (step != nullptr)
                    {
                        PlayListItem* newItem = pli->Copy();
                        newItem->SetDirty();
                        step->AddItem(newItem);
                        PopulateTree(_playlist, step, pli);
                    }
                }
            }
        }
    }
}

void PlayListDialog::ValidateWindow()
{
    if (TreeCtrl_PlayList->GetItemText(TreeCtrl_PlayList->GetRootItem()).Trim(true).Trim(false) != "")
    {
        Button_Ok->Enable(true);
    }
    else
    {
        Button_Ok->Enable(false);
    }

    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (treeitem.IsOk() && !IsPlayList(treeitem))
    {
        Button_Delete->Enable();
        Button_Clone->Enable();
    }
    else
    {
        Button_Delete->Enable(false);
        Button_Clone->Enable(false);
    }
}

void PlayListDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    SwapPage(nullptr);
    *_playlist = *_savedState;
    delete _savedState;
    EndDialog(wxID_CANCEL);
}

void PlayListDialog::OnButton_OkClick(wxCommandEvent& event)
{
    SwapPage(nullptr);
    delete _savedState;
    _playlist->SeparateEveryDay();
    EndDialog(wxID_OK);
}

void PlayListDialog::OnTreeCtrl_PlayListKeyDown(wxTreeEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DeleteSelectedItem();
    }
}

void PlayListDialog::OnNotebook1PageChanged(wxNotebookEvent& event)
{
}

void PlayListDialog::UpdateTree()
{
    wxTreeItemId root = TreeCtrl_PlayList->GetRootItem();
    PlayList* pl = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(root))->GetData();
    TreeCtrl_PlayList->SetItemText(root, pl->GetName());

    wxTreeItemIdValue tid;
    for (wxTreeItemId it = TreeCtrl_PlayList->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(root, tid))
    {
        PlayListStep* pls = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it))->GetData();
        if (pls != nullptr)
        {
            TreeCtrl_PlayList->SetItemText(it, pls->GetName(pl));
        }

        wxTreeItemIdValue tid2;
        for (wxTreeItemId it2 = TreeCtrl_PlayList->GetFirstChild(it, tid2); it2 != nullptr; it2 = TreeCtrl_PlayList->GetNextChild(it, tid2))
        {
            PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it2))->GetData();
            if (pli != nullptr)
            {
                TreeCtrl_PlayList->SetItemText(it2, pli->GetName());
            }
        }
    }
    ValidateWindow();
}

void PlayListDialog::OnDropFiles(wxDropFilesEvent& event)
{
    wxPoint pt = event.GetPosition();
    int pos = -1;

    // I would like this to work but the position returned I cannot make sense of
    wxTreeItemId treeitem = TreeCtrl_PlayList->HitTest(pt);
    if (treeitem.IsOk() && !IsPlayListStep(treeitem) && !IsPlayList(treeitem))
    {
        treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
    }

    if (treeitem.IsOk())
    {
        if (IsPlayList(treeitem))
        {
            pos = -1;
        }
        else
        {
            pos = ((PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData())->GetPosition();
        }
    }

    if (event.GetNumberOfFiles() > 0) {

        wxString* dropped = event.GetFiles();
        wxASSERT(dropped);

        wxString name;
        wxArrayString files;

        for (int i = 0; i < event.GetNumberOfFiles(); i++) {
            name = dropped[i];
            if (wxFileExists(name))
                files.push_back(name);
            else if (wxDirExists(name))
                wxDir::GetAllFiles(name, &files);
        }

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            wxFileName fn(*it);
            if (fn.GetExt().Lower() == "fseq")
            {
                PlayListItemFSEQ* fseq = new PlayListItemFSEQ(_outputManager);
                fseq->SetFSEQFileName(fn.GetFullPath().ToStdString());
                PlayListStep* step = new PlayListStep();
                step->AddItem(fseq);
                _playlist->AddStep(step, pos);
            }
            else if (PlayListItemVideo::IsVideo(fn.GetExt().Lower().ToStdString()))
            {
                PlayListItemVideo* video = new PlayListItemVideo();
                video->SetVideoFile(fn.GetFullPath().ToStdString());
                PlayListStep* step = new PlayListStep();
                step->AddItem(video);
                _playlist->AddStep(step, pos);
            }
            else if (PlayListItemAudio::IsAudio(fn.GetExt().Lower().ToStdString()))
            {
                PlayListItemAudio* audio = new PlayListItemAudio();
                audio->SetAudioFile(fn.GetFullPath().ToStdString());
                PlayListStep* step = new PlayListStep();
                step->AddItem(audio);
                _playlist->AddStep(step, pos);
            }
        }
        PopulateTree(_playlist, nullptr, nullptr);
    }
}

void PlayListDialog::AddItem(PlayList* playlist, PlayListStep* step, PlayListItem* newitem)
{
    PlayListStep* pls = step;

    if (pls == nullptr)
    {
        pls = new PlayListStep();
        playlist->AddStep(pls, 0);
    }
    pls->AddItem(newitem);

    auto items = pls->GetItems();
    for (const auto& it : items)
    {
        it->SetStepLength(pls->GetLengthMS());
    }

    PopulateTree(_playlist, pls, newitem);
}

void PlayListDialog::OnButton_AddFSEQClick(wxCommandEvent& event)
{
    FileDialog1->SetWildcard(FSEQFILES);

    if (FileDialog1->ShowModal() == wxID_OK)
    {
        wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
        PlayListStep* step = nullptr;
        if (IsPlayListStep(treeitem))
        {
            step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        }

        wxArrayString files;
        FileDialog1->GetPaths(files);

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            PlayListItemFSEQ* pli = new PlayListItemFSEQ(_outputManager);
            pli->SetFSEQFileName(it->ToStdString());
            PlayListStep* pls = new PlayListStep();
            pls->AddItem(pli);

            if (!treeitem.IsOk() || IsPlayList(treeitem))
            {
                _playlist->AddStep(pls, 0);
            }
            else
            {
                _playlist->AddStep(pls, GetPos(treeitem) + 1);
            }
        }
        PopulateTree(_playlist, step, nullptr);
    }
}

void PlayListDialog::OnButton_FSEQVideoClick(wxCommandEvent& event)
{
    FileDialog1->SetWildcard(FSEQFILES);

    if (FileDialog1->ShowModal() == wxID_OK)
    {
        wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
        PlayListStep* step = nullptr;
        if (IsPlayListStep(treeitem))
        {
            step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        }

        wxArrayString files;
        FileDialog1->GetPaths(files);

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            PlayListItemFSEQVideo* pli = new PlayListItemFSEQVideo(_outputManager);
            pli->SetFSEQFileName(it->ToStdString());
            PlayListStep* pls = new PlayListStep();
            pls->AddItem(pli);

            if (!treeitem.IsOk() || IsPlayList(treeitem))
            {
                _playlist->AddStep(pls, 0);
            }
            else
            {
                _playlist->AddStep(pls, GetPos(treeitem) + 1);
            }
        }
        PopulateTree(_playlist, step, nullptr);
    }
}

void PlayListDialog::OnButton_AddAudioClick(wxCommandEvent& event)
{
    FileDialog1->SetWildcard(AUDIOFILES);

    if (FileDialog1->ShowModal() == wxID_OK)
    {
        wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
        PlayListStep* step = nullptr;
        if (IsPlayListStep(treeitem))
        {
            step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        }

        wxArrayString files;
        FileDialog1->GetPaths(files);

        for (auto it = files.begin(); it != files.end(); ++it)
        {
            PlayListItemAudio* pli = new PlayListItemAudio();
            pli->SetAudioFile(it->ToStdString());
            PlayListStep* pls = new PlayListStep();
            pls->AddItem(pli);

            if (!treeitem.IsOk() || IsPlayList(treeitem))
            {
                _playlist->AddStep(pls, 0);
            }
            else
            {
                _playlist->AddStep(pls, GetPos(treeitem) + 1);
            }
        }
        PopulateTree(_playlist, step, nullptr);
    }
}

void PlayListDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    DeleteSelectedItem();
    PopulateTree(_playlist, nullptr, nullptr);
}

void PlayListDialog::OnButton_CloneClick(wxCommandEvent& event)
{
    Clone();
}
