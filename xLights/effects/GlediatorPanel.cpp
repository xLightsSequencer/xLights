#include "GlediatorPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(GlediatorPanel)
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include "wx/filedlg.h"


//(*IdInit(GlediatorPanel)
const long GlediatorPanel::ID_BUTTON_GLED_FILE = wxNewId();
const long GlediatorPanel::ID_BITMAPBUTTON10 = wxNewId();
const long GlediatorPanel::ID_TEXTCTRL_Glediator_Filename = wxNewId();
//*)

BEGIN_EVENT_TABLE(GlediatorPanel,wxPanel)
	//(*EventTable(GlediatorPanel)
	//*)
END_EVENT_TABLE()

GlediatorPanel::GlediatorPanel(wxWindow* parent)
{
	//(*Initialize(GlediatorPanel)
	wxFlexGridSizer* FlexGridSizer54;
	wxFlexGridSizer* FlexGridSizer53;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer53 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer54 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonGledFile = new wxButton(this, ID_BUTTON_GLED_FILE, _("Glediator *.gled File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GLED_FILE"));
	FlexGridSizer54->Add(ButtonGledFile, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Glediator_Filename = new wxBitmapButton(this, ID_BITMAPBUTTON10, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,17), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON10"));
	BitmapButton_Glediator_Filename->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer54->Add(BitmapButton_Glediator_Filename, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer53->Add(FlexGridSizer54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer53->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Glediator_Filename = new wxTextCtrl(this, ID_TEXTCTRL_Glediator_Filename, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(200,-1)), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Glediator_Filename"));
	FlexGridSizer53->Add(TextCtrl_Glediator_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer53->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer53);
	FlexGridSizer53->Fit(this);
	FlexGridSizer53->SetSizeHints(this);

	Connect(ID_BUTTON_GLED_FILE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GlediatorPanel::OnButton_Glediator_FilenameClick);
	Connect(ID_BITMAPBUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GlediatorPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_GLEDIATOR");
}

GlediatorPanel::~GlediatorPanel()
{
	//(*Destroy(GlediatorPanel)
	//*)
}

PANEL_EVENT_HANDLERS(GlediatorPanel)

void GlediatorPanel::OnButton_Glediator_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Glediator *.gled", defaultDir, "", "",
                                       "Glediator files (*.gled)|*.gled|Jinx! Glediator files (*.out)|*.out", wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Glediator_Filename->SetValue(filename);
}
