#include "ModelListDialog.h"
#include "ModelDialog.h"
#include "ChannelLayoutDialog.h"
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/xml/xml.h>

//(*InternalHeaders(ModelListDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ModelListDialog)
const long ModelListDialog::ID_LISTBOX1 = wxNewId();
const long ModelListDialog::ID_BUTTON1 = wxNewId();
const long ModelListDialog::ID_BUTTON3 = wxNewId();
const long ModelListDialog::ID_BUTTON4 = wxNewId();
const long ModelListDialog::ID_BUTTON2 = wxNewId();
const long ModelListDialog::ID_BUTTON5 = wxNewId();
const long ModelListDialog::ID_BUTTON_LAYOUT = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelListDialog,wxDialog)
    //(*EventTable(ModelListDialog)
    //*)
END_EVENT_TABLE()

ModelListDialog::ModelListDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(ModelListDialog)
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Model List"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    ListBox1 = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
    ListBox1->SetMinSize(wxSize(150,150));
    FlexGridSizer2->Add(ListBox1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_New = new wxButton(this, ID_BUTTON1, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_New, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Modify = new wxButton(this, ID_BUTTON3, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(Button_Modify, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer3->Add(Button_Delete, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Rename = new wxButton(this, ID_BUTTON2, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Rename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Copy = new wxButton(this, ID_BUTTON5, _("Copy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer3->Add(Button_Copy, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Layout = new wxButton(this, ID_BUTTON_LAYOUT, _("Node Layout"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LAYOUT"));
    FlexGridSizer3->Add(Button_Layout, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_NewClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_ModifyClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_DeleteClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_RenameClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_CopyClick);
    Connect(ID_BUTTON_LAYOUT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_LayoutClick);
    //*)
}

ModelListDialog::~ModelListDialog()
{
    //(*Destroy(ModelListDialog)
    //*)
}

// returns true if name is ok
bool ModelListDialog::ValidateModelName(const wxString& name)
{
    if (name.IsEmpty())
    {
        wxMessageBox(_("A model name is required"), _("ERROR"));
        return false;
    }
    if (ListBox1->FindString(name) != wxNOT_FOUND)
    {
        wxMessageBox(_("A model with this name already exists"), _("ERROR"));
        return false;
    }
    return true;
}

void ModelListDialog::OnButton_NewClick(wxCommandEvent& event)
{
    int DlgResult;
    bool ok;
    wxString name;
    ModelDialog dialog(this);
    dialog.RadioButton_BotLeft->SetValue(true);
    dialog.UpdateLabels();
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            name=dialog.TextCtrl_Name->GetValue();
            name.Trim();
            ok=ValidateModelName(name);
            if (ok)
            {
                wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "model");
                e->AddAttribute("name", name);
                dialog.UpdateXml(e);
                ListBox1->Append(name,e);
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void ModelListDialog::OnButton_ModifyClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Modify button"));
        return;
    }
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    int DlgResult;
    bool ok;
    ModelDialog dialog(this);
    dialog.SetFromXml(e);
    dialog.TextCtrl_Name->Enable(false); // do not allow name changes; -why? -DJ
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            if (ok)
            {
                dialog.UpdateXml(e);
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void ModelListDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Delete button"));
        return;
    }
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    ListBox1->Delete(sel);
    wxXmlNode* p=e->GetParent();
    if (p) p->RemoveChild(e);
    delete e;
}

void ModelListDialog::OnButton_RenameClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Rename button"));
        return;
    }
    wxTextEntryDialog dialog(this,_("Enter new name"),_("Rename Model"), ListBox1->GetString(sel));
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
            ok=ValidateModelName(NewName);
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    e->DeleteAttribute("name");
    e->AddAttribute("name",NewName);
    ListBox1->SetString(sel,NewName);
}

void ModelListDialog::OnButton_CopyClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Copy button"));
        return;
    }
    wxXmlNode* e=(wxXmlNode*)ListBox1->GetClientData(sel);
    wxString name;
    int DlgResult;
    bool ok;
    ModelDialog dialog(this);
    dialog.SetFromXml(e,_(" - Copy"));
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            name=dialog.TextCtrl_Name->GetValue();
            name.Trim();
            ok=ValidateModelName(name);
            if (ok)
            {
                wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "model");
                e->AddAttribute("name", name);
                dialog.UpdateXml(e);
                ListBox1->Append(name,e);
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void ModelListDialog::OnButton_LayoutClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Channel Layout button"));
        return;
    }
    wxXmlNode* ModelNode=(wxXmlNode*)ListBox1->GetClientData(sel);
    ModelClass model;
    model.SetFromXml(ModelNode);
    wxString html=model.ChannelLayoutHtml();

    ChannelLayoutDialog dialog(this);
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.SetHtmlSource(html);
    dialog.ShowModal();
}

