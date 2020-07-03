/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PaletteMgmtDialog.h"

//(*InternalHeaders(PaletteMgmtDialog)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/font.h>
//*)

#include "UtilFunctions.h"

//(*IdInit(PaletteMgmtDialog)
const long PaletteMgmtDialog::ID_STATICTEXT14 = wxNewId();
const long PaletteMgmtDialog::ID_STATICTEXT1 = wxNewId();
const long PaletteMgmtDialog::ID_LISTBOX1 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON5 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON3 = wxNewId();
const long PaletteMgmtDialog::ID_BUTTON4 = wxNewId();
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
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer3;
    wxBoxSizer* BoxSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Color Palettes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Palette Management"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    wxFont StaticText14Font(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText14->SetFont(StaticText14Font);
    FlexGridSizer1->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Saved Palettes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ListBox1 = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
    FlexGridSizer3->Add(ListBox1, 1, wxALL|wxEXPAND, 5);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    ButtonCopy12 = new wxButton(this, ID_BUTTON5, _("Copy palette 1 to palette 2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    BoxSizer1->Add(ButtonCopy12, 1, wxALL|wxEXPAND, 5);
    ButtonSavePalette1 = new wxButton(this, ID_BUTTON3, _("Save palette 1 as..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    BoxSizer1->Add(ButtonSavePalette1, 1, wxALL|wxEXPAND, 5);
    ButtonSavePalette2 = new wxButton(this, ID_BUTTON4, _("Save palette 2 as..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    BoxSizer1->Add(ButtonSavePalette2, 1, wxALL|wxEXPAND, 5);
    ButtonLoadPalette1 = new wxButton(this, ID_BUTTON1, _("Load saved palette to palette 1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(ButtonLoadPalette1, 1, wxALL|wxEXPAND, 5);
    ButtonLoadPalette2 = new wxButton(this, ID_BUTTON2, _("Load saved palette to palette 2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(ButtonLoadPalette2, 1, wxALL|wxEXPAND, 5);
    ButtonDelete = new wxButton(this, ID_BUTTON6, _("Delete saved palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    BoxSizer1->Add(ButtonDelete, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3->Add(BoxSizer1, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonCopyClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonSavePalette1Click);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonSavePalette2Click);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonLoadPalette1Click);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonLoadPalette2Click);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PaletteMgmtDialog::OnButtonDeleteClick);
    //*)
}

PaletteMgmtDialog::~PaletteMgmtDialog()
{
    //(*Destroy(PaletteMgmtDialog)
    //*)
}

void PaletteMgmtDialog::initialize(wxXmlNode* PalNode, EffectsPanel* p1,EffectsPanel* p2)
{
    panel1=p1;
    panel2=p2;
    PalettesNode=PalNode;
    ReloadPaletteList();
}

void PaletteMgmtDialog::ReloadPaletteList()
{
    wxString name;
    ListBox1->Clear();
    for(wxXmlNode* e=PalettesNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "palette")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                ListBox1->Append(name,e);
            }
        }
    }
}

void PaletteMgmtDialog::LoadPalette(wxXmlNode* PaletteNode, EffectsPanel* panel)
{
    wxString AttrName;
    wxColour color;
    if (!PaletteNode) return;
    for (int i=1; i<=6; i++) {
        AttrName.Printf("color%d",i);
        color.Set(PaletteNode->GetAttribute(AttrName));
//~        panel->SetPaletteColor(i,&color);
    }
}

int PaletteMgmtDialog::GetSelectedIndex()
{
    int NameIdx=ListBox1->GetSelection();
    if (NameIdx == wxNOT_FOUND)
    {
        DisplayError(_("You must select a saved palette first!"), this);
    }
    return NameIdx;
}

void PaletteMgmtDialog::OnButtonLoadPalette1Click(wxCommandEvent& event)
{
    int NameIdx=GetSelectedIndex();
    if (NameIdx != wxNOT_FOUND) {
        LoadPalette((wxXmlNode*)ListBox1->GetClientData(NameIdx),panel1);
    }
}

void PaletteMgmtDialog::OnButtonLoadPalette2Click(wxCommandEvent& event)
{
    int NameIdx=GetSelectedIndex();
    if (NameIdx != wxNOT_FOUND) {
        LoadPalette((wxXmlNode*)ListBox1->GetClientData(NameIdx),panel2);
    }
}

void PaletteMgmtDialog::OnButtonCopyClick(wxCommandEvent& event)
{
    wxColour c;
    for (int i=1; i<=6; i++) {
//~        c=panel1->GetPaletteColor(i);
//~        panel2->SetPaletteColor(i,&c);
    }
}

void PaletteMgmtDialog::OnButtonDeleteClick(wxCommandEvent& event)
{
    int NameIdx=GetSelectedIndex();
    if (NameIdx == wxNOT_FOUND) return;
    wxXmlNode* PaletteNode=(wxXmlNode*)ListBox1->GetClientData(NameIdx);
    PalettesNode->RemoveChild(PaletteNode);
    delete PaletteNode;
    ReloadPaletteList();
}


void PaletteMgmtDialog::SavePalette(EffectsPanel* panel)
{
    wxString ChildName,AttrName;
    wxColour color;
    wxString name = wxGetTextFromUser(_("Enter name for palette"), _("Save Color Palette"));
    name.Trim(true);
    if (name.IsEmpty()) return; // user pressed cancel
    wxXmlNode* NewPalette = new wxXmlNode(wxXML_ELEMENT_NODE, _("palette"));
    NewPalette->AddAttribute("name", name);
    for (int i=1; i<=6; i++) {
//~        color=panel->GetPaletteColor(i);
        AttrName.Printf("color%d",i);
        NewPalette->AddAttribute(AttrName, color.GetAsString(wxC2S_HTML_SYNTAX));
    }
    PalettesNode->AddChild(NewPalette);
    ReloadPaletteList();
}

void PaletteMgmtDialog::OnButtonSavePalette1Click(wxCommandEvent& event)
{
    SavePalette(panel1);
}

void PaletteMgmtDialog::OnButtonSavePalette2Click(wxCommandEvent& event)
{
    SavePalette(panel2);
}
