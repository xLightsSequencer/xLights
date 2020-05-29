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
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>
#include <wx/menu.h>
#include <wx/config.h>

#include "globals.h"

//(*IdInit(BatchRenderDialog)
const long BatchRenderDialog::ID_CHOICE_FILTER = wxNewId();
const long BatchRenderDialog::ID_CHOICE_FOLDER = wxNewId();
const long BatchRenderDialog::ID_CHECKLISTBOX_SEQUENCES = wxNewId();
const long BatchRenderDialog::ID_BUTTON1 = wxNewId();
const long BatchRenderDialog::ID_BUTTON2 = wxNewId();
//*)

const long BatchRenderDialog::ID_MNU_SELECTALL = wxNewId();
const long BatchRenderDialog::ID_MNU_SELECTNONE = wxNewId();
const long BatchRenderDialog::ID_MNU_SELECTHIGH = wxNewId();
const long BatchRenderDialog::ID_MNU_DESELECTHIGH = wxNewId();

BEGIN_EVENT_TABLE(BatchRenderDialog,wxDialog)
	//(*EventTable(BatchRenderDialog)
	//*)
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
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FilterChoice = new wxChoice(this, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	FilterChoice->Append(_("Recursive Search"));
	FilterChoice->SetSelection( FilterChoice->Append(_("Recursive Search - No Backups")) );
	FilterChoice->Append(_("Only Show Directory"));
	FlexGridSizer2->Add(FilterChoice, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FolderChoice = new wxChoice(this, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	FlexGridSizer2->Add(FolderChoice, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	SequenceList = new wxCheckListBox(this, ID_CHECKLISTBOX_SEQUENCES, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED, wxDefaultValidator, _T("ID_CHECKLISTBOX_SEQUENCES"));
	SequenceList->SetMinSize(wxDLG_UNIT(this,wxSize(150,200)));
	FlexGridSizer1->Add(SequenceList, 1, wxALL|wxEXPAND, 5);
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
	Connect(ID_CHECKLISTBOX_SEQUENCES,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&BatchRenderDialog::OnSequenceListToggled);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BatchRenderDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BatchRenderDialog::OnButton_CancelClick);
	//*)

    Connect(ID_CHECKLISTBOX_SEQUENCES, wxEVT_CONTEXT_MENU,(wxObjectEventFunction)&BatchRenderDialog::OnPreviewRightDown);

    SetEscapeId(Button_Cancel->GetId());
    ValidateWindow();
}

void BatchRenderDialog::OnPreviewRightDown(wxMouseEvent& event)
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
    if (event.GetId() == ID_MNU_SELECTALL || event.GetId() == ID_MNU_SELECTNONE ) {
        for (size_t x = 0; x < SequenceList->GetCount(); x++) {
            SequenceList->Check(x, event.GetId() == ID_MNU_SELECTALL);
        }
    } else if (event.GetId() == ID_MNU_SELECTHIGH || event.GetId() == ID_MNU_DESELECTHIGH ) {
        for (size_t x = 0; x < SequenceList->GetCount(); x++) {
            if (SequenceList->IsSelected(x)) {
                SequenceList->Check(x, event.GetId() == ID_MNU_SELECTHIGH);
            }
        }
    }
    ValidateWindow();
}

wxArrayString BatchRenderDialog::GetFileList()
{
    wxArrayString lst;
    for (size_t x = 0; x < SequenceList->GetCount(); x++) {
        if (SequenceList->IsChecked(x)) {
            lst.push_back(SequenceList->GetString(x));
        }
    }

    return lst;
}

void BatchRenderDialog::GetSeqList(const wxString& folder)
{
    wxArrayString files;
    wxDir::GetAllFiles(folder, &files, "*.x*");
    files.Sort();
    for (size_t x = 0; x < files.size(); x++) {
        wxString name = files[x].SubString(folder.length(), files[x].size());
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
        if(subfolder.StartsWith("Backup/") && subfolder.StartsWith("Backup\\"))
            continue;
        if (subfolder.StartsWith("."))
            continue;
        FolderChoice->Append(subfolder);
    }
}

bool BatchRenderDialog::Prepare(const wxString &showDir)
{
    GetFolderList(showDir);
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
            wxArrayString items = wxSplit(itcsv, ',');

            for (const auto& it : items) {
                int index = SequenceList->FindString(it);
                if (index != wxNOT_FOUND) {
                    SequenceList->Check(index, true);
                }
            }
        }
    }

    ValidateWindow();
    return true;
}

BatchRenderDialog::~BatchRenderDialog()
{
	//(*Destroy(BatchRenderDialog)
	//*)
}

void BatchRenderDialog::OnFilterChoiceSelect(wxCommandEvent& event)
{
    const wxArrayString filelist = GetFileList();

    SequenceList->Freeze();
    SequenceList->Clear();

    int type = FilterChoice->GetSelection();
    for (const auto& a : allFiles) {
        const wxString name = a;
        switch (type) {
        case 0:
            FolderChoice->Enable();
            if (isFileInFolder(name)) {
                SequenceList->Append(name);
            }
            break;
        case 1:
            FolderChoice->Enable();
            if (!name.StartsWith("Backup/") && !name.StartsWith("Backup\\") && isFileInFolder(name)) {
                SequenceList->Append(name);
            }
            break;
        case 2:
            FolderChoice->Disable();
            if (!name.Contains("/") && !name.Contains("\\")) {
                SequenceList->Append(name);
            }
            break;
        }
    }

    for (const auto& it : filelist)
    {
        int index = SequenceList->FindString(it);
        if (index != wxNOT_FOUND) {
            SequenceList->Check(index, true);
        }
    }

    SequenceList->Thaw();

    ValidateWindow();
}

void BatchRenderDialog::OnFolderChoiceSelect(wxCommandEvent& event)
{
    OnFilterChoiceSelect(event);
}

void BatchRenderDialog::ValidateWindow()
{
    wxArrayInt sel;
    SequenceList->GetCheckedItems(sel);
    if (sel.size() == 0) {
        Button_Ok->Enable(false);
    }
    else {
        Button_Ok->Enable(true);
    }
}

void BatchRenderDialog::OnButton_OkClick(wxCommandEvent& event)
{
    wxString selected = "";
    for (size_t x = 0; x < SequenceList->GetCount(); x++) {
        if (SequenceList->IsChecked(x)) {
            if (selected != "") {
                selected += ",";
            }
            selected += SequenceList->GetString(x);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("BatchRendererItemList", selected);
        config->Write("BatchRendererFilterSelection", FilterChoice->GetSelection());
        config->Write("BatchRendererFolderSelection", FolderChoice->GetString(FolderChoice->GetSelection()));
    }

    EndDialog(wxID_OK);
}

void BatchRenderDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void BatchRenderDialog::OnSequenceListToggled(wxCommandEvent& event)
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
