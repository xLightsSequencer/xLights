#include "ModelListDialog.h"
#include "ModelDialog.h"
#include "ChannelLayoutDialog.h"
#include "sequencer/Element.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <wx/xml/xml.h>
#include <wx/file.h>
#include "xLightsMain.h"

//(*InternalHeaders(ModelListDialog)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(ModelListDialog)
const long ModelListDialog::ID_LISTBOX1 = wxNewId();
const long ModelListDialog::ID_BUTTON1 = wxNewId();
const long ModelListDialog::ID_BUTTON3 = wxNewId();
const long ModelListDialog::ID_BUTTON4 = wxNewId();
const long ModelListDialog::ID_BUTTON2 = wxNewId();
const long ModelListDialog::ID_BUTTON5 = wxNewId();
const long ModelListDialog::ID_BUTTON_LAYOUT = wxNewId();
const long ModelListDialog::ID_BUTTON_ExportCsv = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelListDialog,wxDialog)
    //(*EventTable(ModelListDialog)
    //*)
END_EVENT_TABLE()

ModelListDialog::ModelListDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
: mParent((xLightsFrame*)parent),
  mSequenceElements(NULL)
{
    //(*Initialize(ModelListDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Model List"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    ListBox1 = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
    ListBox1->SetMinSize(wxSize(150,150));
    FlexGridSizer2->Add(ListBox1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_New = new wxButton(this, ID_BUTTON1, _("New"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_New, 1, wxALL|wxEXPAND, 5);
    Button_Modify = new wxButton(this, ID_BUTTON3, _("Modify"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(Button_Modify, 1, wxALL|wxEXPAND, 5);
    Button_Delete = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer3->Add(Button_Delete, 1, wxALL|wxEXPAND, 5);
    Button_Rename = new wxButton(this, ID_BUTTON2, _("Rename"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Rename, 1, wxALL|wxEXPAND, 5);
    Button_Copy = new wxButton(this, ID_BUTTON5, _("Copy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer3->Add(Button_Copy, 1, wxALL|wxEXPAND, 5);
    Button_Layout = new wxButton(this, ID_BUTTON_LAYOUT, _("Node Layout"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LAYOUT"));
    FlexGridSizer3->Add(Button_Layout, 1, wxALL|wxEXPAND, 5);
    Button_ExportCsv = new wxButton(this, ID_BUTTON_ExportCsv, _("Export CSV"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ExportCsv"));
    FlexGridSizer3->Add(Button_ExportCsv, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&ModelListDialog::OnListBox_ListBox1);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_NewClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_ModifyClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_DeleteClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_RenameClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_CopyClick);
    Connect(ID_BUTTON_LAYOUT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_LayoutClick);
    Connect(ID_BUTTON_ExportCsv,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelListDialog::OnButton_ExportCsvClick);
    //*)
}

ModelListDialog::~ModelListDialog()
{
    //(*Destroy(ModelListDialog)
    //*)
}

void ModelListDialog::AddModel(const wxString &name, wxXmlNode *nd) {
    models[name] = nd;
    ListBox1->Append(name, nd);
}
wxXmlNode *ModelListDialog::GetXMLForModel(const wxString &name) {
    return models[name];
}


void ModelListDialog::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

// returns true if name is ok
bool ModelListDialog::ValidateModelName(const wxString& name)
{
    if (name.IsEmpty())
    {
        wxMessageBox(_("A model name is required"), _("ERROR"));
        return false;
    }
    if (ListBox1->FindString(name, true) != wxNOT_FOUND)
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
    ModelDialog dialog(this, mParent);
    dialog.RadioButton_BotLeft->SetValue(true);
    dialog.SetNetInfo(netInfo);
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
                e->AddAttribute("offsetXpct","0.5");
                e->AddAttribute("offsetYpct","0.5");

                dialog.UpdateXml(e);
                ListBox1->Append(name,e);
                models[name] = e;
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
    wxXmlNode* e = models[ListBox1->GetString(sel)];
    int DlgResult;
    bool ok;
    ModelDialog *dialog = new ModelDialog(this, mParent);
    dialog->SetFromXml(e, netInfo);
    dialog->TextCtrl_Name->Enable(false); // do not allow name changes; -why? -DJ
    do
    {
        ok=true;
        DlgResult=dialog->ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            if (ok)
            {
                dialog->UpdateXml(e);
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    delete dialog;
}

void ModelListDialog::OnButton_DeleteClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Delete button"));
        return;
    }

    wxXmlNode* e = models[ListBox1->GetString(sel)];
    std::string attr;
    int result = wxNO;
    attr = e->GetAttribute("name");
    Element* elem_to_delete = mSequenceElements->GetElement(attr);
    if( elem_to_delete != NULL )
    {
        result = wxMessageBox("Delete all effects and layers for the selected model(s)?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO);
        if( result == wxYES ) mSequenceElements->DeleteElement(attr);
    }
    if( elem_to_delete == NULL || result == wxYES )
    {
        ListBox1->Delete(sel);
        wxXmlNode* p=e->GetParent();
        if (p) p->RemoveChild(e);
        delete e;
    }
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
    std::string NewName;
    do
    {
        ok=true;
        DlgResult=dialog.ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            NewName=dialog.GetValue().Trim();
            ok=ValidateModelName(NewName);
        }
    }
    while (DlgResult == wxID_OK && !ok);
    if (DlgResult != wxID_OK) return;
    wxXmlNode* e = models[ListBox1->GetString(sel)];

    std::string OldName;
    OldName = e->GetAttribute("name");
    mParent->RenameModel(OldName, NewName);
    

    Element* elem_to_rename = mSequenceElements->GetElement(OldName);
    if( elem_to_rename != NULL )
    {
        elem_to_rename->SetName(NewName);
    }

    e->DeleteAttribute("name");
    e->AddAttribute("name",NewName);

    for (wxXmlNode *grp = modelGroups->GetChildren(); grp != nullptr; grp = grp->GetNext()) {
        wxString groupModels = grp->GetAttribute("models");
        wxArrayString ModelsInGroup=wxSplit(groupModels,',');
        for(int i=0;i<ModelsInGroup.size();i++)
        {
            if (ModelsInGroup[i] == OldName) {
                ModelsInGroup[i] = NewName;
                grp->DeleteAttribute("models");

                groupModels = ModelsInGroup[0];
                for (int x = 1; x < ModelsInGroup.size(); x++) {
                    groupModels += ",";
                    groupModels += ModelsInGroup[x];
                }
                grp->AddAttribute("models", groupModels);
            }
        }
    }

    mParent->RenameModelInViews(OldName, NewName);
    mSequenceElements->RenameModelInViews(OldName, NewName);

    ListBox1->Delete(sel);
    ListBox1->Append(NewName, e);
    models[NewName] = e;
    models[OldName] = nullptr;
}

void ModelListDialog::OnButton_CopyClick(wxCommandEvent& event)
{
    int sel=ListBox1->GetSelection();
    if (sel == wxNOT_FOUND)
    {
        wxMessageBox(_("Select an item before clicking the Copy button"));
        return;
    }
    wxXmlNode* e = models[ListBox1->GetString(sel)];
    wxString name;
    int DlgResult;
    bool ok;
    ModelDialog dialog(this, mParent);
    dialog.SetFromXml(e, netInfo, _(" - Copy"));
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
                e->AddAttribute("offsetXpct","0.5");
                e->AddAttribute("offsetYpct","0.5");
                dialog.UpdateXml(e);
                ListBox1->Append(name,e);
                models[name] = e;
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
    wxXmlNode* ModelNode = models[ListBox1->GetString(sel)];
    std::unique_ptr<Model> md(ModelManager::CreateModel(ModelNode, *netInfo));
    wxString html=md->ChannelLayoutHtml();

    ChannelLayoutDialog dialog(this);
    dialog.HtmlEasyPrint=HtmlEasyPrint;
    dialog.SetHtmlSource(html);
    dialog.ShowModal();
}


#define retmsg(msg)  \
{ \
    wxMessageBox(msg, _("Export Error")); \
    return; \
}

void ModelListDialog::OnButton_ExportCsvClick(wxCommandEvent& event)
{
#if 0
model name
display as
type of strings
#strings
#nodes
start channel
start node = (channel+2)/3;
my display
brightness
#endif // 0
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, wxEmptyString, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
//    if (filename.IsEmpty()) retmsg(wxString("Please choose an output file name."));
    if (filename.IsEmpty()) return;

    wxFile f(filename);
//    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
    f.Write(_("Model_name, Display_as, String_type, String_count, Node_count, Start_channel, Start_node, My_display, Brightness+-\n"));

    int first = 0, last = ListBox1->GetCount();
    if (ListBox1->GetSelection() != wxNOT_FOUND) last = 1 + (first = ListBox1->GetSelection());
    for (int i = first; i < last; ++i)
    {
        wxXmlNode* node = models[ListBox1->GetString(i)];
        std::unique_ptr<Model> model(ModelManager::CreateModel(node, *netInfo));
        wxString stch = node->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
        f.Write(wxString::Format("\"%s\", \"%s\", \"%s\", %d, %d, %s, %d, %d\n", model->name, model->GetDisplayAs(), model->GetStringType(), model->GetNodeCount() / model->NodesPerString(), model->GetNodeCount(), stch, /*WRONG:*/ model->NodeStartChannel(0) / model->NodesPerString() + 1, model->MyDisplay));
//no worky        f.Flush(); //paranoid: write out data in case model loop crashes
    }
    f.Close();
    retmsg(wxString::Format(_("Models exported: %d of %d"), last - first, ListBox1->GetCount()));
}

void ModelListDialog::OnListBox_ListBox1(wxCommandEvent& event)
{
    wxXmlNode* ModelNode = models[ListBox1->GetString(ListBox1->GetSelection())];
    wxString displayAs = ModelNode->GetAttribute("DisplayAs");
    bool enable = displayAs == "WholeHouse"?false:true;
    Button_Modify->Enable(enable);
    Button_Layout->Enable(enable);
    Button_Copy->Enable(enable);
    Button_ExportCsv->Enable(enable);
}
