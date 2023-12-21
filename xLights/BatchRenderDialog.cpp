/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BatchRenderDialog.h"

//(*InternalHeaders(BatchRenderDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include <wx/dir.h>
#include <wx/menu.h>
#include <wx/config.h>
#include "ExternalHooks.h"
#include "UtilFunctions.h"
#include "globals.h"

//(*IdInit(BatchRenderDialog)
const long BatchRenderDialog::ID_CHOICE_FILTER = wxNewId();
const long BatchRenderDialog::ID_CHOICE_FOLDER = wxNewId();
const long BatchRenderDialog::ID_STATICTEXT1 = wxNewId();
const long BatchRenderDialog::ID_TEXTCTRL1 = wxNewId();
const long BatchRenderDialog::ID_CHECKBOX1 = wxNewId();
const long BatchRenderDialog::ID_PANEL_HOLDER = wxNewId();
const long BatchRenderDialog::ID_BUTTON1 = wxNewId();
const long BatchRenderDialog::ID_BUTTON2 = wxNewId();
//*)

const long BatchRenderDialog::ID_MNU_SELECTALL = wxNewId();
const long BatchRenderDialog::ID_MNU_SELECTNONE = wxNewId();
const long BatchRenderDialog::ID_MNU_SELECTHIGH = wxNewId();
const long BatchRenderDialog::ID_MNU_DESELECTHIGH = wxNewId();

#define SORT_SEQ_NAME_COL 0
#define SORT_SEQ_TIME_COL 1

BEGIN_EVENT_TABLE(BatchRenderDialog,wxDialog)
	//(*EventTable(BatchRenderDialog)
	//*)
EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, BatchRenderDialog::SequenceListPopup)

END_EVENT_TABLE()

BatchRenderDialog::BatchRenderDialog(wxWindow* parent)
{
	//(*Initialize(BatchRenderDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;

	Create(parent, wxID_ANY, _("Batch Render"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FilterChoice = new wxChoice(this, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	FilterChoice->Append(_("Recursive Search"));
	FilterChoice->SetSelection( FilterChoice->Append(_("Recursive Search - No Backups")) );
	FilterChoice->Append(_("Only Show Directory"));
	FlexGridSizer2->Add(FilterChoice, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FolderChoice = new wxChoice(this, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	FlexGridSizer2->Add(FolderChoice, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT1, _("Selected:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Selected = new wxTextCtrl(this, ID_TEXTCTRL1, _("Text"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Selected, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ForceHighDefinition = new wxCheckBox(this, ID_CHECKBOX1, _("Force High Definition"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ForceHighDefinition->SetValue(true);
	FlexGridSizer2->Add(CheckBox_ForceHighDefinition, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	CheckListBoxHolder = new wxPanel(this, ID_PANEL_HOLDER, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_HOLDER"));
	CheckListBoxHolder->SetMinSize(wxSize(150,200));
	FlexGridSizer1->Add(CheckListBoxHolder, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_CHOICE_FILTER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BatchRenderDialog::OnFilterChoiceSelect);
	Connect(ID_CHOICE_FOLDER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BatchRenderDialog::OnFolderChoiceSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BatchRenderDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BatchRenderDialog::OnButton_CancelClick);
	Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&BatchRenderDialog::OnInit);
	//*)

    CheckListBox_Sequences = new wxTreeListCtrl(this, wxID_ANY,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxTL_CHECKBOX | wxTL_MULTIPLE, "ID_TREELISTVIEW_SEQUENCES");
    Connect(CheckListBox_Sequences->GetId(), wxEVT_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&BatchRenderDialog::OnSequenceListToggled);
    CheckListBox_Sequences->SetMinSize(wxSize(-1, 100));
    CheckListBox_Sequences->AppendColumn("Sequence", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Modified Date", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);

    wxConfigBase* config = wxConfigBase::Get();
    auto seqSortCol = config->ReadLong("BatchRendererSortCol", SORT_SEQ_NAME_COL);
    auto seqSortOrder = config->ReadBool("BatchRendererSortOrder", true);
    CheckListBox_Sequences->SetSortColumn(seqSortCol, seqSortOrder);
    FlexGridSizer1->Replace(CheckListBoxHolder, CheckListBox_Sequences, true);

    CheckListBoxHolder->Destroy();

    FlexGridSizer1->Layout();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    SetEscapeId(Button_Cancel->GetId());
    ValidateWindow();
}

void BatchRenderDialog::SequenceListPopup(wxTreeListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Select None");
    mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
    mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BatchRenderDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void BatchRenderDialog::OnPopupCommand(wxCommandEvent &event)
{
    int id = event.GetId();
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        bool isSelected = CheckListBox_Sequences->IsSelected(item);
        if (id == ID_MNU_SELECTALL && !isChecked) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_SELECTNONE && isChecked) {
            CheckListBox_Sequences->UncheckItem(item);
        } else if (id == ID_MNU_SELECTHIGH && !isChecked && isSelected) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_DESELECTHIGH && isChecked && isSelected) {
            CheckListBox_Sequences->UncheckItem(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    ValidateWindow();
}

wxArrayString BatchRenderDialog::GetFileList()
{
    wxArrayString lst;
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED) {
            wxString sequences = CheckListBox_Sequences->GetItemText(item);
            lst.push_back(sequences);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    return lst;
}

void BatchRenderDialog::GetSeqList(const wxString& folder)
{
    wxArrayString files;
    GetAllFilesInDir(folder, files, "*.x*", wxDIR_DEFAULT);
    files.Sort();
    for (size_t x = 0; x < files.size(); x++) {
        wxString name = files[x];
        name = name.SubString(folder.length(), files[x].size());
        if (name[0] == '/' || name[0] == '\\') {
            name = name.SubString(1, name.size());
        }
        if (!name.Contains("xlights_") && (name.Lower().EndsWith("xsq")|| name.Lower().EndsWith("xml"))) {
            allFiles.push_back(name);
        }
    }
}

void BatchRenderDialog::GetFolderList(const wxString& folder)
{
    FolderChoice->Append("");
    wxArrayString subfolders;
    wxDir dir(folder);
    if (!dir.IsOpened()) {
        return;
    }
    wxString strFile;

    if (dir.GetFirst(&strFile, "*", wxDIR_HIDDEN | wxDIR_DIRS))
        subfolders.Add(strFile);

    while (dir.GetNext(&strFile)) {
        subfolders.Add(strFile);
    }
    subfolders.Sort();
    for (const auto& subfolder: subfolders) {
        if(subfolder.StartsWith("Backup/") || subfolder.StartsWith("Backup\\"))
            continue;
        if (subfolder.StartsWith("."))
            continue;
        FolderChoice->Append(subfolder);
    }
}

bool BatchRenderDialog::Prepare(const wxString &showDir)
{
    wxProgressDialog prgs("Searching for Sequences",
                          "Searching for Sequences", 100, this);
    prgs.Show();
    showDirectory = showDir;
    prgs.Pulse("Searching for Folder");
    GetFolderList(showDir);
    prgs.Pulse("Searching for Sequences");
    GetSeqList(showDir);

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        int filterSelect = -1;
        wxString folderSelect = "";
        config->Read("BatchRendererFilterSelection", &filterSelect);
        config->Read("BatchRendererFolderSelection", &folderSelect);
        if (filterSelect != wxNOT_FOUND) {
            FilterChoice->SetSelection(filterSelect);
        }
        int ifoldSelect = FolderChoice->FindString(folderSelect);
        if(ifoldSelect != wxNOT_FOUND) {
            FolderChoice->SetSelection(ifoldSelect);
        }

        wxCommandEvent evt;
        OnFilterChoiceSelect(evt);
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);

        wxString itcsv = "";
        config->Read("BatchRendererItemList", &itcsv, "");

        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(item)) != wxNOT_FOUND) {
                    CheckListBox_Sequences->CheckItem(item);
                }
                item = CheckListBox_Sequences->GetNextItem(item);
            }
        }
    }
    prgs.Update(100);
    prgs.Hide();
    ValidateWindow();
    return true;
}

BatchRenderDialog::~BatchRenderDialog()
{
	//(*Destroy(BatchRenderDialog)
	//*)
}

void BatchRenderDialog::OnInit(wxInitDialogEvent& event)
{
    SetSize(600, 400);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("BatchRendererPos", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400)
            sz.SetWidth(400);
        if (sz.GetHeight() < 300)
            sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
    }
    EnsureWindowHeaderIsOnScreen(this);
    Layout();
}

void BatchRenderDialog::OnFilterChoiceSelect(wxCommandEvent& event)
{
    const wxArrayString filelist = GetFileList();

    CheckListBox_Sequences->Freeze();
    CheckListBox_Sequences->DeleteAllItems();

    int type = FilterChoice->GetSelection();
    for (const auto& a : allFiles) {
        const wxString name = a;
        switch (type) {
        case 0:
            FolderChoice->Enable();
            if (isFileInFolder(name)) {
                wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), name);
                DisplayDateModified(a, item);
            }
            break;
        case 1:
            FolderChoice->Enable();
            if (!name.StartsWith("Backup/") && !name.StartsWith("Backup\\") && !name.Contains("\\Backup\\") && !name.Contains("/Backup/") && isFileInFolder(name)) {
                wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), name);
                DisplayDateModified(a, item);
            }
            break;
        case 2:
            FolderChoice->Disable();
            if (!name.Contains("/") && !name.Contains("\\")) {
                wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), name);
                DisplayDateModified(a, item);
            }
            break;
        }
    }

    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        auto seq = CheckListBox_Sequences->GetItemText(item);
        if (filelist.Index(CheckListBox_Sequences->GetItemText(item)) != wxNOT_FOUND) {
            CheckListBox_Sequences->CheckItem(item);
            break;
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    CheckListBox_Sequences->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);

    CheckListBox_Sequences->Thaw();
    ValidateWindow();
}

void BatchRenderDialog::OnFolderChoiceSelect(wxCommandEvent& event)
{
    OnFilterChoiceSelect(event);
}

void BatchRenderDialog::ValidateWindow()
{
    auto count = UpdateCount();

    if (count == 0) {
        Button_Ok->Enable(false);
    }
    else {
        Button_Ok->Enable(true);
    }
}

void BatchRenderDialog::OnButton_OkClick(wxCommandEvent& event)
{
    SaveSettings();
    EndDialog(wxID_OK);
}

void BatchRenderDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    SaveSettings();
    EndDialog(wxID_CANCEL);
}

void BatchRenderDialog::OnSequenceListToggled(wxDataViewEvent& event)
{
    ValidateWindow();
}

bool BatchRenderDialog::isFileInFolder(const wxString &file) const
{
    const wxString folder = FolderChoice->GetString(FolderChoice->GetSelection());
    if (folder.IsEmpty())
        return true;
    if (file.StartsWith( folder + "\\") || file.StartsWith( folder + "/"))
        return true;
    return false;
}

uint32_t BatchRenderDialog::UpdateCount()
{
    uint32_t selected { 0 };

    auto item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED) {
            selected++;
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    TextCtrl_Selected->SetValue(wxString::Format("%d", selected));
    return selected;
}

void BatchRenderDialog::SaveSettings()
{
    wxString selected;
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        if (isChecked) {
            if (selected != "") {
                selected += ",";
            }
            selected += CheckListBox_Sequences->GetItemText(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    unsigned int sortCol { SORT_SEQ_NAME_COL };
    bool ascendingOrder { true };

    CheckListBox_Sequences->GetSortColumn(&sortCol, &ascendingOrder);

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("BatchRendererItemList", selected);
        config->Write("BatchRendererFilterSelection", FilterChoice->GetSelection());
        config->Write("BatchRendererFolderSelection", FolderChoice->GetString(FolderChoice->GetSelection()));
        config->Write("BatchRendererSortCol", sortCol);
        config->Write("BatchRendererSortOrder", ascendingOrder);
    }
    SaveWindowPosition("BatchRendererPos", this);
}

void BatchRenderDialog::DisplayDateModified(std::string const& fileName, wxTreeListItem& item) const
{
    if (FileExists(showDirectory + wxFileName::GetPathSeparator() + fileName)) {
        wxDateTime last_modified_time(wxFileModificationTime(showDirectory + wxFileName::GetPathSeparator() + fileName));
        CheckListBox_Sequences->SetItemText(item, 1, last_modified_time.Format(wxT("%Y-%m-%d %H:%M:%S")));
    }
}