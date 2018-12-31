#include "EffectListDialog.h"
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/xml/xml.h>

//(*InternalHeaders(EffectListDialog)
#include <wx/string.h>
#include <wx/intl.h>
#include "UtilFunctions.h"
//*)

//(*IdInit(EffectListDialog)
const long EffectListDialog::ID_LISTBOX1 = wxNewId();
const long EffectListDialog::ID_BUTTON3 = wxNewId();
const long EffectListDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectListDialog,wxDialog)
    //(*EventTable(EffectListDialog)
    //*)
END_EVENT_TABLE()

EffectListDialog::EffectListDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(EffectListDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Presets List"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    ListBox1 = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
    ListBox1->SetMinSize(wxSize(150,200));
    FlexGridSizer2->Add(ListBox1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Delete = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Rename = new wxButton(this, ID_BUTTON2, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Rename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectListDialog::OnButton_DeleteClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectListDialog::OnButton_RenameClick);
    //*)
}

EffectListDialog::~EffectListDialog()
{
    //(*Destroy(EffectListDialog)
    //*)
}


void EffectListDialog::OnButton_RenameClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        DisplayError(_("Select an item before clicking the Rename button"), this);
        return;
    }
    wxTextEntryDialog dialog(this,_("Enter new name"),_("Rename Preset"),ListBox1->GetString(sel));
    int DlgResult;
    bool ok;
    wxString NewName;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            NewName=dialog.GetValue();
            NewName.Trim();
            if (NewName.IsEmpty())
            {
                ok=false;
                DisplayError(_("A preset name cannot be empty"), this);
            }
            else
            {
                int FindIdx=ListBox1->FindString(NewName);
                if (FindIdx != wxNOT_FOUND && FindIdx != sel)
                {
                    ok=false;
                    DisplayError(_("That name is already in use"), this);
                }
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    e->DeleteAttribute("name");
    e->AddAttribute("name",NewName);
    ListBox1->Delete(sel);
    ListBox1->Append(NewName, e);
}

void EffectListDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        DisplayError(_("Select an item before clicking the Delete button"), this);
        return;
    }
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    ListBox1->Delete(sel);
    wxXmlNode* p=e->GetParent();
    if (p) p->RemoveChild(e);
    delete e;
}
