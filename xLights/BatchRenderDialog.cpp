#include "BatchRenderDialog.h"

//(*InternalHeaders(BatchRenderDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>
#include <wx/menu.h>
#include <wx/config.h>

#include "globals.h"

//(*IdInit(BatchRenderDialog)
const long BatchRenderDialog::ID_CHOICE_FILTER = wxNewId();
const long BatchRenderDialog::ID_CHECKLISTBOX_SEQUENCES = wxNewId();
//*)

BEGIN_EVENT_TABLE(BatchRenderDialog,wxDialog)
	//(*EventTable(BatchRenderDialog)
	//*)
END_EVENT_TABLE()

BatchRenderDialog::BatchRenderDialog(wxWindow* parent)
{
	//(*Initialize(BatchRenderDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

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
	FlexGridSizer2->Add(FilterChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	SequenceList = new wxCheckListBox(this, ID_CHECKLISTBOX_SEQUENCES, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX_SEQUENCES"));
	SequenceList->SetMinSize(wxDLG_UNIT(this,wxSize(150,200)));
	FlexGridSizer1->Add(SequenceList, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_CHOICE_FILTER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BatchRenderDialog::OnFilterChoiceSelect);
	//*)

    Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&BatchRenderDialog::OnPreviewRightDown, nullptr,this);
}

void BatchRenderDialog::OnPreviewRightDown(wxMouseEvent& event) {
    wxMenu mnu;
    mnu.Append(1, "Select All");
    mnu.Append(2, "Select None");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BatchRenderDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void BatchRenderDialog::OnPopupCommand(wxCommandEvent &event) {
    for (int x = 0; x < SequenceList->GetCount(); x++) {
        SequenceList->Check(x, event.GetId() == 1);
    }
}

wxArrayString BatchRenderDialog::GetFileList() {
    wxArrayString lst;
    wxString selected = "";
    for (int x = 0; x < SequenceList->GetCount(); x++) {
        if (SequenceList->IsChecked(x)) {
            lst.push_back(SequenceList->GetString(x));
            if (selected != "")
            {
                selected += ",";
            }
            selected += SequenceList->GetString(x);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr)
    {
        config->Write("BatchRendererItemList", selected);
    }

    return lst;
}

bool BatchRenderDialog::Prepare(const wxString &showDir) {
    wxArrayString files;
    wxDir::GetAllFiles(showDir, &files, "*.xml");
    files.Sort();
    for (int x = 0; x < files.size(); x++) {
        wxString name = files[x].SubString(showDir.length(), files[x].size());
        if (name[0] == '/' || name[0] == '\\') {
            name = name.SubString(1, name.size());
        }
        if (!name.Contains("xlights_")) {
            allFiles.push_back(name);
        }
    }
    wxCommandEvent evt;
    OnFilterChoiceSelect(evt);
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr)
    {
        wxString itcsv = "";
        config->Read("BatchRendererItemList", &itcsv);

        if (itcsv != "")
        {
            wxArrayString items = wxSplit(itcsv, ',');

            for (auto it = items.begin(); it != items.end(); ++it)
            {
                int index = SequenceList->FindString(*it);
                if (index != wxNOT_FOUND)
                {
                    SequenceList->Check(index, true);
                }
            }
        }
    }

    return SequenceList->GetCount() > 0;
}

BatchRenderDialog::~BatchRenderDialog()
{
	//(*Destroy(BatchRenderDialog)
	//*)
}

void BatchRenderDialog::OnFilterChoiceSelect(wxCommandEvent& event)
{
    wxArrayString filelist = GetFileList();

    SequenceList->Freeze();
    SequenceList->Clear();

    int type = FilterChoice->GetSelection();
    for (auto a = allFiles.begin(); a != allFiles.end(); ++a) {
        wxString name = *a;
        switch (type) {
        case 0:
            SequenceList->Append(name);
            break;
        case 1:
            if (!name.StartsWith("Backup/") && !name.StartsWith("Backup\\")) {
                SequenceList->Append(name);
            }
            break;
        case 2:
            if (!name.Contains("/") && !name.Contains("\\")) {
                SequenceList->Append(name);
            }
            break;
        }
    }

    for (auto it = filelist.begin(); it != filelist.end(); ++it)
    {
        int index = SequenceList->FindString(*it);
        if (index != wxNOT_FOUND)
        {
            SequenceList->Check(index, true);
        }
    }

    SequenceList->Thaw();
}
