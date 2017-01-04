#include "PlayListDialog.h"

#include "PlayList.h"
#include "PlayListStep.h"
#include "PlayListItem.h"
#include "../MyTreeItemData.h"
#include "PlayListPanel.h"
#include "PlayListStepPanel.h"
#include "PlayListItemVideo.h"
#include "PlayListItemAllOff.h"
#include "PlayListItemRunProcess.h"
#include "PlayListItemFSEQ.h"
#include "PlayListItemESEQ.h"
#include "PlayListItemImage.h"
#include "PlayListItemDelay.h"

#include <wx/xml/xml.h>
#include <wx/menu.h>
#include <wx/dnd.h>

//(*InternalHeaders(PlayListDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListDialog)
const long PlayListDialog::ID_TREECTRL1 = wxNewId();
const long PlayListDialog::ID_STATICTEXT2 = wxNewId();
const long PlayListDialog::ID_BUTTON1 = wxNewId();
const long PlayListDialog::ID_BUTTON2 = wxNewId();
const long PlayListDialog::ID_PANEL1 = wxNewId();
const long PlayListDialog::ID_NOTEBOOK1 = wxNewId();
const long PlayListDialog::ID_PANEL2 = wxNewId();
const long PlayListDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long PlayListDialog::ID_MNU_ADDSTEP = wxNewId();
const long PlayListDialog::ID_MNU_ADDVIDEO = wxNewId();
const long PlayListDialog::ID_MNU_ADDESEQ = wxNewId();
const long PlayListDialog::ID_MNU_ADDFSEQ = wxNewId();
const long PlayListDialog::ID_MNU_ADDALLOFF = wxNewId();
const long PlayListDialog::ID_MNU_ADDIMAGE = wxNewId();
const long PlayListDialog::ID_MNU_ADDDELAY = wxNewId();
const long PlayListDialog::ID_MNU_ADDPROCESS = wxNewId();
const long PlayListDialog::ID_MNU_DELETE = wxNewId();

BEGIN_EVENT_TABLE(PlayListDialog,wxDialog)
	//(*EventTable(PlayListDialog)
	//*)
END_EVENT_TABLE()

PlayListDialog::PlayListDialog(wxWindow* parent, PlayList* playlist, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _playlist = playlist;

	//(*Initialize(PlayListDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Play List"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	TreeCtrl_PlayList = new wxTreeCtrl(Panel1, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	TreeCtrl_PlayList->SetMinSize(wxSize(300,300));
	FlexGridSizer3->Add(TreeCtrl_PlayList, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Right click on tree to add/remove steps and items."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(Panel1, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
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
	Notebook1->SetMinSize(wxSize(300,300));
	FlexGridSizer4->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	Panel2->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel2);
	FlexGridSizer4->SetSizeHints(Panel2);
	SplitterWindow1->SplitVertically(Panel1, Panel2);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_BEGIN_DRAG,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListBeginDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_END_DRAG,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListEndDrag);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListSelectionChanged);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListKeyDown);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&PlayListDialog::OnTreeCtrl_PlayListItemMenu);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlayListDialog::OnButton_CancelClick);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&PlayListDialog::OnNotebook1PageChanged);
	//*)

    SetSize(800, 500);

    // save the current state in case the user cancels
    _savedState = playlist->Save();

    PopulateTree();

    TreeCtrl_PlayList->SelectItem(TreeCtrl_PlayList->GetRootItem());

    ValidateWindow();
}

PlayListDialog::~PlayListDialog()
{
	//(*Destroy(PlayListDialog)
	//*)
}

void PlayListDialog::PopulateTree()
{
    TreeCtrl_PlayList->DeleteAllItems();

    wxTreeItemId id = TreeCtrl_PlayList->AddRoot(_playlist->GetName());
    TreeCtrl_PlayList->SetItemData(id, new MyTreeItemData(_playlist));

    auto steps = _playlist->GetSteps();
    for (auto it = steps.begin(); it != steps.end(); ++it)
    {
        wxTreeItemId step = TreeCtrl_PlayList->AppendItem(TreeCtrl_PlayList->GetRootItem(), (*it)->GetName());
        TreeCtrl_PlayList->SetItemData(step, new MyTreeItemData(*it));

        auto items = (*it)->GetItems();
        for (auto it2 = items.begin(); it2 != items.end(); ++it2)
        {
            id = TreeCtrl_PlayList->AppendItem(step, (*it2)->GetName());
            TreeCtrl_PlayList->SetItemData(id, new MyTreeItemData(*it2));
        }
        TreeCtrl_PlayList->Expand(step);
    }
    TreeCtrl_PlayList->Expand(TreeCtrl_PlayList->GetRootItem());
}

void PlayListDialog::OnTextCtrl_PlayListNameText(wxCommandEvent& event)
{
    ValidateWindow();
}

bool PlayListDialog::IsPlayList(wxTreeItemId id)
{
    return (id == TreeCtrl_PlayList->GetRootItem());
}

bool PlayListDialog::IsPlayListStep(wxTreeItemId id)
{
    return (TreeCtrl_PlayList->GetItemParent(id) == TreeCtrl_PlayList->GetRootItem());
}

void PlayListDialog::OnTreeCtrl_PlayListSelectionChanged(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (IsPlayList(treeitem))
    {
        PlayList* pl = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        if (Notebook1->GetPageCount() > 0) Notebook1->RemovePage(0);
        Notebook1->AddPage(new PlayListPanel(Notebook1, pl), "Playlist", true);
    }
    else if (IsPlayListStep(treeitem))
    {
        PlayListStep* pls = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        if (Notebook1->GetPageCount() > 0) Notebook1->RemovePage(0);
        Notebook1->AddPage(new PlayListStepPanel(Notebook1, pls), "Playlist Step", true);
    }
    else
    {
        // must be a playlist entry
        PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        if (Notebook1->GetPageCount() > 0) Notebook1->RemovePage(0);
        pli->Configure(Notebook1);
    }
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
        event.Skip();
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
            _playlist->MoveStepAfterStep(dragstep, nullptr);
            PopulateTree();
        }
        // if dropped on a step make it the step after the dropped step
        else if (IsPlayListStep(dropitem))
        {
            PlayListStep* dropstep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(dropitem))->GetData();
            _playlist->MoveStepAfterStep(dragstep, dropstep);
            PopulateTree();
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
                TreeCtrl_PlayList->Delete(dragitem);

                // add it to the drop step
                dropstep->AddItem(dragpli);
                wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(dropitem, dragpli->GetName(), -1, -1, new MyTreeItemData(dragpli));
                TreeCtrl_PlayList->Expand(newitem);
                TreeCtrl_PlayList->EnsureVisible(newitem);
                TreeCtrl_PlayList->SelectItem(newitem);
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
    wxMenuItem* mi = mnu.Append(ID_MNU_ADDFSEQ, "Add FSEQ");
    mi = mnu.Append(ID_MNU_ADDESEQ, "Add ESEQ");
    mi = mnu.Append(ID_MNU_ADDVIDEO, "Add Video");
    mi = mnu.Append(ID_MNU_ADDIMAGE, "Add Image");
    mi = mnu.Append(ID_MNU_ADDALLOFF, "Add All Off");
    mi = mnu.Append(ID_MNU_ADDDELAY, "Add Delay");
    mi = mnu.Append(ID_MNU_ADDPROCESS, "Add Process");

    mi = mnu.Append(ID_MNU_ADDSTEP, "Add Step");
    if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_DELETE, "Delete");
    if (IsPlayList(treeitem))
    {
        mi->Enable(false);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&PlayListDialog::OnTreeCtrlMenu, nullptr, this);
    PopupMenu(&mnu);
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
    if (event.GetId() == ID_MNU_ADDVIDEO)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemVideo* pli = new PlayListItemVideo();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDIMAGE)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemImage* pli = new PlayListItemImage();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDALLOFF)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemAllOff* pli = new PlayListItemAllOff();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDDELAY)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemDelay* pli = new PlayListItemDelay();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDPROCESS)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemRunProcess* pli = new PlayListItemRunProcess();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDFSEQ)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemFSEQ* pli = new PlayListItemFSEQ();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDESEQ)
    {
        if (IsPlayList(treeitem))
        {
            // seemlessly add a step
            PlayListStep* pls = new PlayListStep();
            wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
            _playlist->AddStep(pls, 0);
            treeitem = newitem;
        }
        else if (!IsPlayListStep(treeitem))
        {
            treeitem = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        PlayListStep* step = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
        PlayListItemESEQ* pli = new PlayListItemESEQ();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        step->AddItem(pli);
    }
    else if (event.GetId() == ID_MNU_ADDSTEP)
    {
        wxTreeItemId parent;
        if (IsPlayListStep(treeitem))
        {
            parent = TreeCtrl_PlayList->GetItemParent(treeitem);
        }
        else
        {
            parent = treeitem;
        }

        PlayListStep* pls = new PlayListStep();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(parent, pls->GetName(), -1, -1, new MyTreeItemData(pls));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
        if (IsPlayList(treeitem))
        {
            _playlist->AddStep(pls, 0);
        }
        else
        {
            _playlist->AddStep(pls, GetPos(treeitem)+1);
        }
        PopulateTree();
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        DeleteSelectedItem();
    }
}

void PlayListDialog::DeleteSelectedItem()
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();
    if (treeitem.IsOk() && !IsPlayList(treeitem))
    {
        if (wxMessageBox("Are you sure?", "Are you sure?", wxYES_NO) == wxYES)
        {
            if (Notebook1->GetPageCount() > 0) Notebook1->RemovePage(0);
            wxTreeItemId parent = TreeCtrl_PlayList->GetItemParent(treeitem);
            if (IsPlayListStep(treeitem))
            {
                PlayListStep* playliststep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();

                wxTreeItemIdValue tid;
                for (auto it = TreeCtrl_PlayList->GetFirstChild(treeitem, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(treeitem, tid))
                {
                    PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it))->GetData();
                    delete pli;
                }
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(TreeCtrl_PlayList->GetItemParent(treeitem)))->GetData();
                playlist->RemoveStep(playliststep);
                delete playliststep;

                TreeCtrl_PlayList->Delete(treeitem);
            }
            else if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
            {
                PlayListItem* playlistitem = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
                PlayListStep* playliststep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(TreeCtrl_PlayList->GetItemParent(treeitem)))->GetData();
                playliststep->RemoveItem(playlistitem);
                delete playlistitem;
                TreeCtrl_PlayList->Delete(treeitem);
            }
            TreeCtrl_PlayList->SelectItem(parent);
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
}

void PlayListDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    _playlist->Load(_savedState);
    delete _savedState;
    EndDialog(wxID_CANCEL);
}

void PlayListDialog::OnButton_OkClick(wxCommandEvent& event)
{
    delete _savedState;
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
    TreeCtrl_PlayList->SetItemText(root, ((PlayList*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(root))->GetData())->GetName());

    wxTreeItemIdValue tid;
    for (wxTreeItemId it = TreeCtrl_PlayList->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(root, tid))
    {
        PlayListStep* pls = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it))->GetData();
        TreeCtrl_PlayList->SetItemText(it, pls->GetName());

        wxTreeItemIdValue tid2;
        for (wxTreeItemId it2 = TreeCtrl_PlayList->GetFirstChild(it, tid2); it2 != nullptr; it2 = TreeCtrl_PlayList->GetNextChild(it, tid2))
        {
            PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it2))->GetData();
            TreeCtrl_PlayList->SetItemText(it2, pli->GetName());
        }
    }
    ValidateWindow();
}
