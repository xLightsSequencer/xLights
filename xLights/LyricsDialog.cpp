/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LyricsDialog.h"

//(*InternalHeaders(LyricsDialog)
#include <wx/button.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(LyricsDialog)
const long LyricsDialog::ID_TEXTCTRL_Lyrics = wxNewId();
const long LyricsDialog::ID_STATICTEXT3 = wxNewId();
const long LyricsDialog::ID_TEXTCTRL_LYRIC_STARTTIME = wxNewId();
const long LyricsDialog::ID_STATICTEXT4 = wxNewId();
const long LyricsDialog::ID_TEXTCTRL_LYRIC_ENDTIME = wxNewId();
const long LyricsDialog::ID_STATICTEXT2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LyricsDialog,wxDialog)
	//(*EventTable(LyricsDialog)
	//*)
END_EVENT_TABLE()

LyricsDialog::LyricsDialog(int endtime, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(LyricsDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Type or Paste Lyrics"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	TextCtrlLyrics = new wxTextCtrl(this, ID_TEXTCTRL_Lyrics, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL_Lyrics"));
	TextCtrlLyrics->SetMinSize(wxDLG_UNIT(this,wxSize(250,150)));
	FlexGridSizer1->Add(TextCtrlLyrics, 1, wxLEFT|wxRIGHT|wxEXPAND, wxDLG_UNIT(this,wxSize(20,0)).GetWidth());
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start Time (sec):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer1->Add(StaticText3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Lyric_StartTime = new wxTextCtrl(this, ID_TEXTCTRL_LYRIC_STARTTIME, _("0.000"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_LYRIC_STARTTIME"));
	TextCtrl_Lyric_StartTime->SetMaxLength(7);
	BoxSizer1->Add(TextCtrl_Lyric_StartTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("End Time (sec):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	BoxSizer1->Add(StaticText4, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	TextCtrl_Lyric_EndTime = new wxTextCtrl(this, ID_TEXTCTRL_LYRIC_ENDTIME, _("0.000"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_LYRIC_ENDTIME"));
	TextCtrl_Lyric_EndTime->SetMaxLength(7);
	BoxSizer1->Add(TextCtrl_Lyric_EndTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, wxDLG_UNIT(this,wxSize(10,0)).GetWidth());
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Caution:  All timings/labels on this track will be replaced."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	wxFont StaticText2Font(11,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText2->SetFont(StaticText2Font);
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 0);
	FlexGridSizer1->Add(-1,20,1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	TextCtrl_Lyric_EndTime->SetValue(std::to_string(double(endtime / 1000.0)).substr(0, 7));

    SetEscapeId(wxID_CANCEL);
}

LyricsDialog::~LyricsDialog()
{
	//(*Destroy(LyricsDialog)
	//*)
}

