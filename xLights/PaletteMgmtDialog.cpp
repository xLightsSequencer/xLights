#include "PaletteMgmtDialog.h"

//(*InternalHeaders(PaletteMgmtDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PaletteMgmtDialog)
const long PaletteMgmtDialog::ID_STATICTEXT14 = wxNewId();
const long PaletteMgmtDialog::ID_STATICTEXT1 = wxNewId();
const long PaletteMgmtDialog::ID_LISTBOX1 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON5 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON4 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON3 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON1 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON2 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON6 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PaletteMgmtDialog,wxDialog)
    //(*EventTable(PaletteMgmtDialog)
    //*)
END_EVENT_TABLE()

PaletteMgmtDialog::PaletteMgmtDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(PaletteMgmtDialog)
    wxFlexGridSizer* FlexGridSizer3;
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Color Palettes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Palette Management"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    wxFont StaticText14Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText14->SetFont(StaticText14Font);
    FlexGridSizer1->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Saved Palettes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ListBox1 = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
    FlexGridSizer3->Add(ListBox1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    ButtonCopy = new wxButton(this, ID_BUTTON5, _("Copy primary to secondary"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    BoxSizer1->Add(ButtonCopy, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonChangeOrder = new wxButton(this, ID_BUTTON4, _("Change palette order"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    BoxSizer1->Add(ButtonChangeOrder, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSavePalette = new wxButton(this, ID_BUTTON3, _("Save palette as..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    BoxSizer1->Add(ButtonSavePalette, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonLoadPalette1 = new wxButton(this, ID_BUTTON1, _("Load saved palette to primary"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(ButtonLoadPalette1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonLoadPaletteBoth = new wxButton(this, ID_BUTTON2, _("Load saved palette to effects 1 and 2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(ButtonLoadPaletteBoth, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonDelete = new wxButton(this, ID_BUTTON6, _("Delete saved palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    BoxSizer1->Add(ButtonDelete, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(BoxSizer1, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonCopyClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonChangeOrderClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonSavePaletteClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonLoadPalette1Click);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonLoadPaletteBothClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonDeleteClick);
    //*)
}

PaletteMgmtDialog::~PaletteMgmtDialog()
{
    //(*Destroy(PaletteMgmtDialog)
    //*)
}

void PaletteMgmtDialog::initialize(const wxString& id1, const wxString& id2, wxXmlNode* PalNode, wxSizer* PriSizer,wxSizer* SecSizer)
{
    ButtonSavePalette->SetLabel(_("Save palette ")+id1+_(" as..."));
    ButtonLoadPalette1->SetLabel(_("Load saved palette to effect ")+id1);
    ButtonCopy->SetLabel(_("Copy palette for effect ")+id1+_(" to effect ")+id2);
    ButtonChangeOrder->SetLabel(_("Change palette ")+id1+_(" order"));
    PrimarySizer=PriSizer;
    SecondarySizer=SecSizer;
    PrimaryId=id1;
    SecondaryId=id2;
    PalettesNode=PalNode;
    ReloadPaletteList();
}

void PaletteMgmtDialog::ReloadPaletteList()
{
    wxString name;
    ListBox1->Clear();
    for(wxXmlNode* e=PalettesNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("palette"))
        {
            name=e->GetAttribute(wxT("name"));
            if (!name.IsEmpty())
            {
                ListBox1->Append(name,e);
            }
        }
    }
}

void PaletteMgmtDialog::OnButtonSavePaletteClick(wxCommandEvent& event)
{
    wxString ChildName,AttrName;
    wxString name = wxGetTextFromUser(_("Enter name for palette"), _("Save Color Palette"));
    name.Trim(true);
    if (name.IsEmpty()) return; // user pressed cancel
    wxXmlNode* NewPalette = new wxXmlNode(wxXML_ELEMENT_NODE, _("palette"));
    NewPalette->AddAttribute(wxT("name"), name);
    wxSizerItemList &ChildList = PrimarySizer->GetChildren();
    for ( wxSizerItemList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        if (!(*it)->IsWindow()) continue;
        wxWindow *ChildWin = (*it)->GetWindow();
        ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_BUTTON")))
        {
            wxColour color=ChildWin->GetBackgroundColour();
            AttrName=_("color")+ChildName.Right(1);
            NewPalette->AddAttribute(AttrName, color.GetAsString(wxC2S_HTML_SYNTAX));
        }
    }
    PalettesNode->AddChild(NewPalette);
    ReloadPaletteList();
}

void PaletteMgmtDialog::LoadPalette(wxXmlNode* PaletteNode, wxSizer* Sizer)
{
    wxString ChildName,AttrName;
    if (!PaletteNode) return;
    wxSizerItemList &ChildList = Sizer->GetChildren();
    for ( wxSizerItemList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        if (!(*it)->IsWindow()) continue;
        wxWindow *ChildWin = (*it)->GetWindow();
        ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_BUTTON")))
        {
            AttrName=_("color")+ChildName.Right(1);
            ChildWin->SetBackgroundColour(PaletteNode->GetAttribute(AttrName));
        }
    }
}

void PaletteMgmtDialog::OnButtonLoadPalette1Click(wxCommandEvent& event)
{
    int NameIdx=ListBox1->GetSelection();
    if (NameIdx == wxNOT_FOUND)
    {
        wxMessageBox(_("You must select a saved palette first!"));
        return;
    }
    wxXmlNode* PaletteNode=(wxXmlNode*)ListBox1->GetClientData(NameIdx);
    LoadPalette(PaletteNode,PrimarySizer);
}

void PaletteMgmtDialog::OnButtonLoadPaletteBothClick(wxCommandEvent& event)
{
    int NameIdx=ListBox1->GetSelection();
    if (NameIdx == wxNOT_FOUND)
    {
        wxMessageBox(_("You must select a saved palette first!"));
        return;
    }
    wxXmlNode* PaletteNode=(wxXmlNode*)ListBox1->GetClientData(NameIdx);
    LoadPalette(PaletteNode,PrimarySizer);
    LoadPalette(PaletteNode,SecondarySizer);
}

void PaletteMgmtDialog::OnButtonCopyClick(wxCommandEvent& event)
{
    wxString ChildName;
    wxSizerItemList &ChildList = PrimarySizer->GetChildren();
    for ( wxSizerItemList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        if (!(*it)->IsWindow()) continue;
        wxWindow *ChildWin = (*it)->GetWindow();
        ChildName=ChildWin->GetName();
        if (ChildName.StartsWith(wxT("ID_BUTTON")))
        {
            wxColour color=ChildWin->GetBackgroundColour();
            ChildName.Replace(_("_Palette")+PrimaryId, _("_Palette")+SecondaryId);
            wxWindow *CtrlWin=wxWindow::FindWindowByName(ChildName);
            if (CtrlWin != NULL) CtrlWin->SetBackgroundColour(color);
        }
    }
}

void PaletteMgmtDialog::OnButtonChangeOrderClick(wxCommandEvent& event)
{

}

void PaletteMgmtDialog::OnButtonDeleteClick(wxCommandEvent& event)
{
    int NameIdx=ListBox1->GetSelection();
    if (NameIdx == wxNOT_FOUND)
    {
        wxMessageBox(_("You must select a saved palette first!"));
        return;
    }
    wxXmlNode* PaletteNode=(wxXmlNode*)ListBox1->GetClientData(NameIdx);
    PalettesNode->RemoveChild(PaletteNode);
    delete PaletteNode;
    ReloadPaletteList();
}
