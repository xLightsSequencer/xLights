//(*InternalHeaders(BackgroundPlaylistDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "BackgroundPlaylistDialog.h"
#include "PlayList/PlayList.h"

//(*IdInit(BackgroundPlaylistDialog)
const long BackgroundPlaylistDialog::ID_LISTVIEW1 = wxNewId();
const long BackgroundPlaylistDialog::ID_BUTTON1 = wxNewId();
const long BackgroundPlaylistDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BackgroundPlaylistDialog,wxDialog)
	//(*EventTable(BackgroundPlaylistDialog)
	//*)
END_EVENT_TABLE()

BackgroundPlaylistDialog::BackgroundPlaylistDialog(wxWindow* parent, int& plid, std::list<PlayList*> playlists,wxWindowID id,const wxPoint& pos,const wxSize& size) : _plid(plid)
{
	//(*Initialize(BackgroundPlaylistDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Background Playlist"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView_Playlists = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView_Playlists, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&BackgroundPlaylistDialog::OnListView_PlaylistsItemSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BackgroundPlaylistDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BackgroundPlaylistDialog::OnButton_CancelClick);
	//*)

    ListView_Playlists->InsertColumn(0, "Playlists");

    long idd = -1;
    int i = 0;
    for (auto it = playlists.begin(); it != playlists.end(); ++it)
    {
        idd = ListView_Playlists->InsertItem(i, (*it)->GetNameNoTime());
        ListView_Playlists->SetItemData(idd, (*it)->GetId());

        if (_plid == (*it)->GetId())
        {
            ListView_Playlists->Select(idd);
        }

        i++;
    }

    idd = ListView_Playlists->InsertItem(i, "(none)");
    ListView_Playlists->SetItemData(idd, -1);

    if (_plid == -1)
    {
        ListView_Playlists->Select(idd);
    }

    ValidateWindow();
}

BackgroundPlaylistDialog::~BackgroundPlaylistDialog()
{
	//(*Destroy(BackgroundPlaylistDialog)
	//*)
}

void BackgroundPlaylistDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void BackgroundPlaylistDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _plid = ListView_Playlists->GetItemData(ListView_Playlists->GetFirstSelected());
    EndDialog(wxID_OK);
}

void BackgroundPlaylistDialog::OnListView_PlaylistsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void BackgroundPlaylistDialog::ValidateWindow()
{
    if (ListView_Playlists->GetSelectedItemCount() != 1)
    {
        Button_Ok->Disable();
    }
    else
    {
        Button_Ok->Enable();
    }
}