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

#include <wx/xml/xml.h>
#include <wx/menu.h>

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
const long PlayListDialog::ID_MNU_ADDFSEQ = wxNewId();
const long PlayListDialog::ID_MNU_ADDALLOFF = wxNewId();
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

    FlexGridSizer1->Fit(this);

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
    }
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

void PlayListDialog::OnTreeCtrl_PlayListBeginDrag(wxTreeEvent& event)
{
}

void PlayListDialog::OnTreeCtrl_PlayListEndDrag(wxTreeEvent& event)
{
}

void PlayListDialog::OnTreeCtrl_PlayListItemMenu(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayList->GetSelection();

    wxMenu mnu;
    wxMenuItem* mi = mnu.Append(ID_MNU_ADDFSEQ, "Add FSEQ");
    if (!IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_ADDVIDEO, "Add Video");
    if (!IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_ADDALLOFF, "Add All Off");
    if (!IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_ADDPROCESS, "Add Process");
    if (!IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_ADDSTEP, "Add Step");
    if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
    {
        mi->Enable(false);
    }

    mi = mnu.Append(ID_MNU_DELETE, "Add Delete");
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
        PlayListItemVideo* pli = new PlayListItemVideo();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
            TreeCtrl_PlayList->Expand(newitem);
            TreeCtrl_PlayList->EnsureVisible(newitem);
            TreeCtrl_PlayList->SelectItem(newitem);
    }
    else if (event.GetId() == ID_MNU_ADDALLOFF)
    {
        PlayListItemAllOff* pli = new PlayListItemAllOff();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
    }
    else if (event.GetId() == ID_MNU_ADDPROCESS)
    {
        PlayListItemRunProcess* pli = new PlayListItemRunProcess();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
    }
    else if (event.GetId() == ID_MNU_ADDFSEQ)
    {
        PlayListItemFSEQ* pli = new PlayListItemFSEQ();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pli->GetName(), -1, -1, new MyTreeItemData(pli));
        TreeCtrl_PlayList->Expand(newitem);
        TreeCtrl_PlayList->EnsureVisible(newitem);
        TreeCtrl_PlayList->SelectItem(newitem);
    }
    else if (event.GetId() == ID_MNU_ADDSTEP)
    {
        PlayListStep* pls = new PlayListStep();
        wxTreeItemId  newitem = TreeCtrl_PlayList->AppendItem(treeitem, pls->GetName(), -1, -1, new MyTreeItemData(pls));
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
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        if (wxMessageBox("Are you sure?", "Are you sure?", wxYES_NO) == wxYES)
        {
            if (Notebook1->GetPageCount() > 0) Notebook1->RemovePage(0);
            if (IsPlayListStep(treeitem))
            {
                PlayListStep* playliststep = (PlayListStep*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();

                wxTreeItemIdValue tid;
                for (auto it = TreeCtrl_PlayList->GetFirstChild(treeitem, tid); it != nullptr; it = TreeCtrl_PlayList->GetNextChild(treeitem, tid))
                {
                    PlayListItem* pli = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(it))->GetData();
                    delete pli;
                }

                delete playliststep;

                TreeCtrl_PlayList->Delete(treeitem);
            }
            else if (!IsPlayList(treeitem) && !IsPlayListStep(treeitem))
            {
                PlayListItem* playlistitem = (PlayListItem*)((MyTreeItemData*)TreeCtrl_PlayList->GetItemData(treeitem))->GetData();
                delete playlistitem;
                TreeCtrl_PlayList->Delete(treeitem);
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
