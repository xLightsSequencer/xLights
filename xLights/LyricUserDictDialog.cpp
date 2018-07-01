#include "LyricUserDictDialog.h"

//(*InternalHeaders(LyricUserDictDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LyricUserDictDialog)
const long LyricUserDictDialog::ID_TEXTCTRL_NEW_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_STATICTEXT1 = wxNewId();
const long LyricUserDictDialog::ID_COMBOBOX_OLD_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_ADD_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_GRID1 = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_DELETE_ROW = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_LYRIC_OK = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_LYRIC_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(LyricUserDictDialog,wxDialog)
	//(*EventTable(LyricUserDictDialog)
	//*)
END_EVENT_TABLE()

LyricUserDictDialog::LyricUserDictDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(LyricUserDictDialog)
	wxBoxSizer* BoxSizer1;
	wxGridSizer* GridSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	GridSizer1 = new wxGridSizer(3, 1, 0, 0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Label"));
	TextCtrlNewLyric = new wxTextCtrl(this, ID_TEXTCTRL_NEW_LYRIC, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_NEW_LYRIC"));
	StaticBoxSizer1->Add(TextCtrlNewLyric, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Copy From"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	StaticBoxSizer1->Add(StaticText1, 1, wxALL|wxFIXED_MINSIZE, 5);
	ComboBoxOldLyric = new wxComboBox(this, ID_COMBOBOX_OLD_LYRIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX_OLD_LYRIC"));
	StaticBoxSizer1->Add(ComboBoxOldLyric, 1, wxALL|wxEXPAND, 5);
	ButtonAddLyric = new wxButton(this, ID_BUTTON_ADD_LYRIC, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_LYRIC"));
	StaticBoxSizer1->Add(ButtonAddLyric, 1, wxALL|wxSHAPED|wxFIXED_MINSIZE, 5);
	GridSizer1->Add(StaticBoxSizer1, 1, wxALL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Label"));
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	Grid1->CreateGrid(0,2);
	Grid1->EnableEditing(true);
	Grid1->EnableGridLines(true);
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	StaticBoxSizer2->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeleteRow = new wxButton(this, ID_BUTTON_DELETE_ROW, _("Delete Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_ROW"));
	StaticBoxSizer2->Add(ButtonDeleteRow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED|wxFIXED_MINSIZE, 5);
	GridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonLyricOK = new wxButton(this, ID_BUTTON_LYRIC_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_OK"));
	BoxSizer1->Add(ButtonLyricOK, 1, wxALL, 5);
	ButtonLyricCancel = new wxButton(this, ID_BUTTON_LYRIC_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_CANCEL"));
	BoxSizer1->Add(ButtonLyricCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridSizer1->Add(BoxSizer1, 1, wxALL, 5);
	SetSizer(GridSizer1);
	GridSizer1->Fit(this);
	GridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_LYRIC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonAddLyricClick);
	Connect(ID_BUTTON_DELETE_ROW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonDeleteRowClick);
	Connect(ID_BUTTON_LYRIC_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricOKClick);
	Connect(ID_BUTTON_LYRIC_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricCancelClick);
	//*)
}

LyricUserDictDialog::~LyricUserDictDialog()
{
	//(*Destroy(LyricUserDictDialog)
	//*)
}


void LyricUserDictDialog::OnButtonAddLyricClick(wxCommandEvent& event)
{
}

void LyricUserDictDialog::OnButtonDeleteRowClick(wxCommandEvent& event)
{
}

void LyricUserDictDialog::OnButtonLyricOKClick(wxCommandEvent& event)
{
}

void LyricUserDictDialog::OnButtonLyricCancelClick(wxCommandEvent& event)
{
}
