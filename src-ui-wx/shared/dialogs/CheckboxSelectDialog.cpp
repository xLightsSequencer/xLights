/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CheckboxSelectDialog.h"

//(*InternalHeaders(CheckboxSelectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/font.h>
#include <wx/menu.h>
#include <wx/srchctrl.h>
#include <wx/stattext.h>
#include <wx/tokenzr.h>

//(*IdInit(CheckboxSelectDialog)
const wxWindowID CheckboxSelectDialog::ID_CHECKLISTBOXITEMS = wxNewId();
const wxWindowID CheckboxSelectDialog::ID_BUTTONOK = wxNewId();
const wxWindowID CheckboxSelectDialog::ID_BUTTONCANCEL = wxNewId();
//*)

const long CheckboxSelectDialog::ID_MCU_SELECTALL = wxNewId();
const long CheckboxSelectDialog::ID_MCU_SELECTNONE = wxNewId();
const long CheckboxSelectDialog::ID_MCU_SELECT_HIGH = wxNewId();
const long CheckboxSelectDialog::ID_MCU_DESELECT_HIGH = wxNewId();
const long CheckboxSelectDialog::ID_FILTERTIMER = wxNewId();

BEGIN_EVENT_TABLE(CheckboxSelectDialog,wxDialog)
	//(*EventTable(CheckboxSelectDialog)
	//*)
END_EVENT_TABLE()

CheckboxSelectDialog::CheckboxSelectDialog(wxWindow* parent, const wxString &title, const wxArrayString& items, const wxArrayString& itemsSelected, const wxString& header, const wxString& headerBold, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(CheckboxSelectDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Choose Items..."), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	CheckListBox_Items = new wxCheckListBox(this, ID_CHECKLISTBOXITEMS, wxDefaultPosition, wxSize(-1,300), 0, 0, wxLB_EXTENDED, wxDefaultValidator, _T("ID_CHECKLISTBOXITEMS"));
	FlexGridSizer1->Add(CheckListBox_Items, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTONOK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONOK"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTONCANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONCANCEL"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOXITEMS, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, (wxObjectEventFunction)&CheckboxSelectDialog::OnCheckListBox_ItemsToggled);
	Connect(ID_BUTTONOK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&CheckboxSelectDialog::OnButton_OkClick);
	Connect(ID_BUTTONCANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&CheckboxSelectDialog::OnButton_CancelClick);
	//*)

	Connect(ID_CHECKLISTBOXITEMS, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& CheckboxSelectDialog::OnListRClick);

    _allItems = items;
    for (const auto& item : itemsSelected) {
        _checked.insert(item);
    }

    int insertRow = 0;
    if (!header.IsEmpty() || !headerBold.IsEmpty()) {
        wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
        if (!header.IsEmpty()) {
            wxStaticText* normalLabel = new wxStaticText(this, wxID_ANY, header);
            headerSizer->Add(normalLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
        }
        if (!headerBold.IsEmpty()) {
            wxStaticText* boldLabel = new wxStaticText(this, wxID_ANY, headerBold);
            wxFont f = boldLabel->GetFont();
            f.SetWeight(wxFONTWEIGHT_BOLD);
            boldLabel->SetFont(f);
            headerSizer->Add(boldLabel, 0, wxALIGN_CENTER_VERTICAL, 0);
        }
        FlexGridSizer1->Insert(insertRow++, headerSizer, 0, wxALL, 5);
    }

    _filterCtrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    _filterCtrl->ShowCancelButton(true);
    _filterCtrl->SetDescriptiveText(_("Filter"));
    FlexGridSizer1->Insert(insertRow++, _filterCtrl, 0, wxALL | wxEXPAND, 5);

    // The checklist has shifted down by the rows we inserted; keep it the growable one.
    FlexGridSizer1->RemoveGrowableRow(0);
    FlexGridSizer1->AddGrowableRow(insertRow);

    _filterCtrl->Bind(wxEVT_TEXT, &CheckboxSelectDialog::OnFilterText, this);
    _filterCtrl->Bind(wxEVT_TEXT_ENTER, &CheckboxSelectDialog::OnFilterEnter, this);
    _filterCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &CheckboxSelectDialog::OnFilterCancel, this);

    _filterTimer.SetOwner(this, ID_FILTERTIMER);
    Bind(wxEVT_TIMER, &CheckboxSelectDialog::OnFilterTimer, this, ID_FILTERTIMER);
    Bind(wxEVT_CLOSE_WINDOW, &CheckboxSelectDialog::OnCloseWindow, this);

    PopulateList();

	SetTitle(title);

    SetEscapeId(Button_Cancel->GetId());

    // Recompute the layout hints now that the header/filter rows exist, otherwise
    // the min size (set by the generated code before these inserts) clips the buttons.
    Layout();
    FlexGridSizer1->SetSizeHints(this);

    ValidateWindow();
}

CheckboxSelectDialog::~CheckboxSelectDialog()
{
	//(*Destroy(CheckboxSelectDialog)
	//*)
}

wxArrayString CheckboxSelectDialog::GetSelectedItems() const
{
    // _checked is the source of truth so items hidden by an active filter are still returned.
    wxArrayString res;
    for (const auto& item : _allItems)
    {
        if (_checked.find(item) != _checked.end())
        {
            res.Add(item);
        }
    }

    return res;
}

void CheckboxSelectDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    _filterTimer.Stop();
    EndDialog(wxID_CANCEL);
}

void CheckboxSelectDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _filterTimer.Stop();
    SyncCheckedFromList();
    EndDialog(wxID_OK);
}

void CheckboxSelectDialog::OnCheckListBox_ItemsToggled(wxCommandEvent& event)
{
    SyncCheckedFromList();
    ValidateWindow();
}

bool CheckboxSelectDialog::MatchesFilter(const wxString& item) const
{
    if (_filter.IsEmpty())
    {
        return true;
    }

    const wxString itemLower = item.Lower();
    wxStringTokenizer tok(_filter.Lower());
    while (tok.HasMoreTokens())
    {
        if (!itemLower.Contains(tok.GetNextToken()))
        {
            return false;
        }
    }
    return true;
}

void CheckboxSelectDialog::SyncCheckedFromList()
{
    // Merge visible check state into _checked; filtered-out items keep their prior state.
    for (size_t i = 0; i < CheckListBox_Items->GetCount(); ++i)
    {
        const wxString name = CheckListBox_Items->GetString(i);
        if (CheckListBox_Items->IsChecked(i))
        {
            _checked.insert(name);
        }
        else
        {
            _checked.erase(name);
        }
    }
}

void CheckboxSelectDialog::PopulateList()
{
    CheckListBox_Items->Freeze();
    CheckListBox_Items->Clear();
    for (const auto& item : _allItems)
    {
        if (MatchesFilter(item))
        {
            CheckListBox_Items->Append(item);
            if (_checked.find(item) != _checked.end())
            {
                CheckListBox_Items->Check(CheckListBox_Items->GetCount() - 1);
            }
        }
    }
    CheckListBox_Items->Thaw();
}

void CheckboxSelectDialog::OnFilterText(wxCommandEvent& event)
{
    _filterTimer.StartOnce(200);
}

void CheckboxSelectDialog::OnFilterEnter(wxCommandEvent& event)
{
    // Enter applies the pending filter immediately rather than waiting on the debounce.
    _filterTimer.Stop();
    ApplyFilter();
}

void CheckboxSelectDialog::OnFilterCancel(wxCommandEvent& event)
{
    _filterTimer.Stop();
    SyncCheckedFromList();
    _filterCtrl->ChangeValue(wxEmptyString);
    _filter.Clear();
    PopulateList();
    ValidateWindow();
}

void CheckboxSelectDialog::OnCloseWindow(wxCloseEvent& event)
{
    _filterTimer.Stop();
    event.Skip();
}

void CheckboxSelectDialog::OnFilterTimer(wxTimerEvent& event)
{
    ApplyFilter();
}

void CheckboxSelectDialog::ApplyFilter()
{
    SyncCheckedFromList();
    _filter = _filterCtrl->GetValue().Trim(true).Trim(false);
    PopulateList();
    ValidateWindow();
}

void CheckboxSelectDialog::ValidateWindow()
{
    Button_Ok->Enable(true);
}

void CheckboxSelectDialog::OnListRClick(wxContextMenuEvent& event)
{
	wxMenu mnu;
	mnu.Append(ID_MCU_SELECTALL, "Select All");
	mnu.Append(ID_MCU_SELECTNONE, "Deselect All");
    mnu.Append(ID_MCU_SELECT_HIGH, "Select Highlighted");
    mnu.Append(ID_MCU_DESELECT_HIGH, "Deselect Highlighted");

	mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&CheckboxSelectDialog::OnPopup, nullptr, this);
	PopupMenu(&mnu);
}

void CheckboxSelectDialog::OnPopup(wxCommandEvent& event)
{
	if (event.GetId() == ID_MCU_SELECTALL)
	{
		SelectAllLayers();
	}
	else if (event.GetId() == ID_MCU_SELECTNONE)
	{
        SelectAllLayers(false);
	}
    else if (event.GetId() == ID_MCU_SELECT_HIGH)
    {
        SelectHighLightedLayers();
    }
    else if (event.GetId() == ID_MCU_DESELECT_HIGH)
    {
        SelectHighLightedLayers(false);
    }
}

void CheckboxSelectDialog::SelectAllLayers(bool select)
{
    for (size_t i = 0; i < CheckListBox_Items->GetCount(); i++)
    {
        CheckListBox_Items->Check(i, select);
    }
    SyncCheckedFromList();
    ValidateWindow();
}

void CheckboxSelectDialog::SelectHighLightedLayers(bool select)
{
	for (size_t i = 0; i < CheckListBox_Items->GetCount(); i++)
	{
        if (CheckListBox_Items->IsSelected(i))
        {
            CheckListBox_Items->Check(i, select);
        }
	}
    SyncCheckedFromList();
    ValidateWindow();
}
