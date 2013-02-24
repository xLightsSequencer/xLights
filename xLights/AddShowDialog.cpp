#include "AddShowDialog.h"

//(*InternalHeaders(AddShowDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(AddShowDialog)
const long AddShowDialog::ID_STATICTEXT1 = wxNewId();
const long AddShowDialog::ID_CHOICE_PLAYLIST = wxNewId();
const long AddShowDialog::ID_STATICTEXT5 = wxNewId();
const long AddShowDialog::ID_SPINCTRL1 = wxNewId();
const long AddShowDialog::ID_STATICTEXT8 = wxNewId();
const long AddShowDialog::ID_SPINCTRL2 = wxNewId();
const long AddShowDialog::ID_STATICTEXT6 = wxNewId();
const long AddShowDialog::ID_SPINCTRL3 = wxNewId();
const long AddShowDialog::ID_STATICTEXT9 = wxNewId();
const long AddShowDialog::ID_SPINCTRL4 = wxNewId();
const long AddShowDialog::ID_STATICTEXT2 = wxNewId();
const long AddShowDialog::ID_CHECKBOX_REPEAT = wxNewId();
const long AddShowDialog::ID_STATICTEXT3 = wxNewId();
const long AddShowDialog::ID_CHECKBOX_FIRST_ITEM = wxNewId();
const long AddShowDialog::ID_STATICTEXT4 = wxNewId();
const long AddShowDialog::ID_CHECKBOX_LAST_ITEM = wxNewId();
const long AddShowDialog::ID_STATICTEXT7 = wxNewId();
const long AddShowDialog::ID_CHECKBOX_RANDOM = wxNewId();
const long AddShowDialog::ID_STATICTEXT10 = wxNewId();
const long AddShowDialog::ID_LISTBOX1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AddShowDialog,wxDialog)
    //(*EventTable(AddShowDialog)
    //*)
END_EVENT_TABLE()

AddShowDialog::AddShowDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(AddShowDialog)
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Schedule Playlist"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Playlist"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoicePlayList = new wxChoice(this, ID_CHOICE_PLAYLIST, wxDefaultPosition, wxSize(113,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PLAYLIST"));
    FlexGridSizer4->Add(ChoicePlayList, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Time"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer4->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    SpinCtrlStartHour = new wxSpinCtrl(this, ID_SPINCTRL1, _T("18"), wxDefaultPosition, wxSize(52,21), wxSP_ARROW_KEYS, 0, 23, 18, _T("ID_SPINCTRL1"));
    SpinCtrlStartHour->SetValue(_T("18"));
    SpinCtrlStartHour->SetToolTip(_("Start hour (0-23)"));
    FlexGridSizer2->Add(SpinCtrlStartHour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _(":"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    wxFont StaticText8Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText8->SetFont(StaticText8Font);
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrlStartMinute = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxSize(52,21), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 59, 0, _T("ID_SPINCTRL2"));
    SpinCtrlStartMinute->SetValue(_T("0"));
    SpinCtrlStartMinute->SetToolTip(_("Start minute (0-59)"));
    FlexGridSizer2->Add(SpinCtrlStartMinute, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("End Time"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer4->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    SpinCtrlEndHour = new wxSpinCtrl(this, ID_SPINCTRL3, _T("21"), wxDefaultPosition, wxSize(52,21), wxSP_ARROW_KEYS, 0, 23, 21, _T("ID_SPINCTRL3"));
    SpinCtrlEndHour->SetValue(_T("21"));
    SpinCtrlEndHour->SetToolTip(_("End hour (0-23)"));
    FlexGridSizer3->Add(SpinCtrlEndHour, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _(":"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    wxFont StaticText9Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText9->SetFont(StaticText9Font);
    FlexGridSizer3->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrlEndMinute = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(52,21), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 59, 0, _T("ID_SPINCTRL4"));
    SpinCtrlEndMinute->SetValue(_T("0"));
    SpinCtrlEndMinute->SetToolTip(_("End minute (0-59)"));
    FlexGridSizer3->Add(SpinCtrlEndMinute, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    StaticText2->SetToolTip(_("Repeat playlist until scheduled end time"));
    FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxRepeat = new wxCheckBox(this, ID_CHECKBOX_REPEAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_REPEAT"));
    CheckBoxRepeat->SetValue(false);
    FlexGridSizer4->Add(CheckBoxRepeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Play first\nitem once"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxFirstItem = new wxCheckBox(this, ID_CHECKBOX_FIRST_ITEM, _("  (if repeating)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FIRST_ITEM"));
    CheckBoxFirstItem->SetValue(false);
    FlexGridSizer4->Add(CheckBoxFirstItem, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Play last\nitem once"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxLastItem = new wxCheckBox(this, ID_CHECKBOX_LAST_ITEM, _("  (if repeating)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LAST_ITEM"));
    CheckBoxLastItem->SetValue(false);
    FlexGridSizer4->Add(CheckBoxLastItem, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Random\norder"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer4->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxRandom = new wxCheckBox(this, ID_CHECKBOX_RANDOM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_RANDOM"));
    CheckBoxRandom->SetValue(false);
    FlexGridSizer4->Add(CheckBoxRandom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Apply to these dates\nUse shift-click or ctrl-click to select multiple dates"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer5->Add(StaticText10, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ListBoxDates = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(231,87), 0, 0, wxLB_EXTENDED|wxLB_ALWAYS_SB, wxDefaultValidator, _T("ID_LISTBOX1"));
    FlexGridSizer5->Add(ListBoxDates, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxSHAPED|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer5->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    //*)
}

AddShowDialog::~AddShowDialog()
{
    //(*Destroy(AddShowDialog)
    //*)
}

// return true if start time is before end time
bool AddShowDialog::StartBeforeEnd()
{
    return (SpinCtrlStartHour->GetValue() < SpinCtrlEndHour->GetValue()) ||
           (SpinCtrlStartHour->GetValue() == SpinCtrlEndHour->GetValue() && SpinCtrlStartMinute->GetValue() < SpinCtrlEndMinute->GetValue());
}

// returns true if the user has selected a playlist
bool AddShowDialog::IsPlaylistSelected()
{
    return (ChoicePlayList->GetSelection() != wxNOT_FOUND);
}

wxString AddShowDialog::PartialEventCode()
{
    wxString PartialCode;
    int StartTime = SpinCtrlStartHour->GetValue()*100 + SpinCtrlStartMinute->GetValue();
    int EndTime = SpinCtrlEndHour->GetValue()*100 + SpinCtrlEndMinute->GetValue();
    PartialCode.Printf(wxT("%04d-%04d %c%c%c%c"),StartTime,EndTime,
                       CheckBoxRepeat->IsChecked() ? 'R' : '-',
                       CheckBoxFirstItem->IsChecked() ? 'F' : '-',
                       CheckBoxLastItem->IsChecked() ? 'L' : '-',
                       CheckBoxRandom->IsChecked() ? 'X' : '-');
    return PartialCode;
}
