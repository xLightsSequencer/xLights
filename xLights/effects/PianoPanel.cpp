#include "PianoPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(PianoPanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)


#include "wx/filedlg.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

//(*IdInit(PianoPanel)
const long PianoPanel::ID_STATICTEXT1 = wxNewId();
const long PianoPanel::ID_CHOICE_Piano_Style = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_SLIDER_Piano_Style = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_NumKeys = wxNewId();
const long PianoPanel::ID_STATICTEXT_Piano_NumRows = wxNewId();
const long PianoPanel::ID_SLIDER_Piano_KeyWidth = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_NumRows = wxNewId();
const long PianoPanel::ID_STATICTEXT20 = wxNewId();
const long PianoPanel::ID_CHOICE_PianoPlacement = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_KeyPlacement = wxNewId();
const long PianoPanel::ID_BUTTON_Piano_CueFilename = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_Filename = wxNewId();
const long PianoPanel::ID_STATICTEXT14 = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_CueFilename = wxNewId();
const long PianoPanel::ID_PANEL9 = wxNewId();
const long PianoPanel::ID_BUTTON1 = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_MapFilename = wxNewId();
const long PianoPanel::ID_STATICTEXT13 = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_MapFilename = wxNewId();
const long PianoPanel::ID_PANEL11 = wxNewId();
const long PianoPanel::ID_BUTTON2 = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_ShapeFilename = wxNewId();
const long PianoPanel::ID_CHECKBOX1 = wxNewId();
const long PianoPanel::ID_BITMAPBUTTON_Piano_Clipping = wxNewId();
const long PianoPanel::ID_TEXTCTRL_Piano_ShapeFilename = wxNewId();
const long PianoPanel::ID_PANEL13 = wxNewId();
const long PianoPanel::ID_NOTEBOOK2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PianoPanel,wxPanel)
	//(*EventTable(PianoPanel)
	//*)
END_EVENT_TABLE()

PianoPanel::PianoPanel(wxWindow* parent)
{
	//(*Initialize(PianoPanel)
	wxFlexGridSizer* FlexGridSizer27;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer34;
	wxFlexGridSizer* FlexGridSizer33;
	wxFlexGridSizer* FlexGridSizer32;
	wxFlexGridSizer* FlexGridSizer28;
	wxFlexGridSizer* FlexGridSizer26;
	wxFlexGridSizer* FlexGridSizer30;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer27->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Style:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Piano_Style = new wxChoice(this, ID_CHOICE_Piano_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Style"));
	Choice_Piano_Style->Append(_("Anim Image (face)"));
	Choice_Piano_Style->Append(_("Equalizer (bars)"));
	Choice_Piano_Style->SetSelection( Choice_Piano_Style->Append(_("Piano Keys (top/edge)")) );
	Choice_Piano_Style->Append(_("Player Piano (scroll)"));
	FlexGridSizer27->Add(Choice_Piano_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_Style = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Piano_Style, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Piano_Style"));
	BitmapButton_Piano_Style->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText_Piano_NumKeys = new wxStaticText(this, ID_STATICTEXT_Piano_NumKeys, _("# Keys:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumKeys"));
	FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Piano_NumKeys = new wxSlider(this, ID_SLIDER_Piano_NumKeys, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_NumKeys"));
	FlexGridSizer27->Add(Slider_Piano_NumKeys, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_NumKeys = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_NumKeys, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumKeys"));
	BitmapButton_Piano_NumKeys->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_NumKeys, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText_Piano_NumRows = new wxStaticText(this, ID_STATICTEXT_Piano_NumRows, _("# Rows:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumRows"));
	FlexGridSizer27->Add(StaticText_Piano_NumRows, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Piano_NumRows = new wxSlider(this, ID_SLIDER_Piano_KeyWidth, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_KeyWidth"));
	FlexGridSizer27->Add(Slider_Piano_NumRows, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_NumRows = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_NumRows, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumRows"));
	BitmapButton_Piano_NumRows->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_NumRows, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText21 = new wxStaticText(this, ID_STATICTEXT20, _("Placement:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
	FlexGridSizer27->Add(StaticText21, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Piano_KeyPlacement = new wxChoice(this, ID_CHOICE_PianoPlacement, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PianoPlacement"));
	Choice_Piano_KeyPlacement->SetSelection( Choice_Piano_KeyPlacement->Append(_("Tile")) );
	Choice_Piano_KeyPlacement->Append(_("Stretch/shrink"));
	Choice_Piano_KeyPlacement->Append(_("Top left"));
	Choice_Piano_KeyPlacement->Append(_("Top center"));
	Choice_Piano_KeyPlacement->Append(_("Top right"));
	Choice_Piano_KeyPlacement->Append(_("Middle left"));
	Choice_Piano_KeyPlacement->Append(_("Middle center"));
	Choice_Piano_KeyPlacement->Append(_("Middle right"));
	Choice_Piano_KeyPlacement->Append(_("Bottom left"));
	Choice_Piano_KeyPlacement->Append(_("Bottom middle"));
	Choice_Piano_KeyPlacement->Append(_("Bottom right"));
	FlexGridSizer27->Add(Choice_Piano_KeyPlacement, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_KeyPlacement = new wxBitmapButton(this, ID_BITMAPBUTTON_Piano_KeyPlacement, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_KeyPlacement"));
	BitmapButton_Piano_KeyPlacement->SetDefault();
	FlexGridSizer27->Add(BitmapButton_Piano_KeyPlacement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer27, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Notebook2 = new wxNotebook(this, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
	Notebook2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
	Panel3 = new wxPanel(Notebook2, ID_PANEL9, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL9"));
	FlexGridSizer28 = new wxFlexGridSizer(5, 1, 0, 0);
	FlexGridSizer26 = new wxFlexGridSizer(0, 6, 0, 0);
	Button_Piano_CueFilename = new wxButton(Panel3, ID_BUTTON_Piano_CueFilename, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Piano_CueFilename"));
	Button_Piano_CueFilename->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer26->Add(Button_Piano_CueFilename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_CueFilename = new wxBitmapButton(Panel3, ID_BITMAPBUTTON_Piano_Filename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Filename"));
	BitmapButton_Piano_CueFilename->SetDefault();
	FlexGridSizer26->Add(BitmapButton_Piano_CueFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer26->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText15_layout_kludge_1 = new wxStaticText(Panel3, ID_STATICTEXT14, _("                                    x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	StaticText15_layout_kludge_1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	FlexGridSizer26->Add(StaticText15_layout_kludge_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer28->Add(FlexGridSizer26, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Piano_CueFilename = new wxTextCtrl(Panel3, ID_TEXTCTRL_Piano_CueFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_CueFilename"));
	FlexGridSizer28->Add(TextCtrl_Piano_CueFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(FlexGridSizer28);
	FlexGridSizer28->Fit(Panel3);
	FlexGridSizer28->SetSizeHints(Panel3);
	Panel4 = new wxPanel(Notebook2, ID_PANEL11, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL11"));
	FlexGridSizer30 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer33 = new wxFlexGridSizer(0, 6, 0, 0);
	Button_Piano_MapFilename = new wxButton(Panel4, ID_BUTTON1, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Piano_MapFilename->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer33->Add(Button_Piano_MapFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_MapFilename = new wxBitmapButton(Panel4, ID_BITMAPBUTTON_Piano_MapFilename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_MapFilename"));
	BitmapButton_Piano_MapFilename->SetDefault();
	FlexGridSizer33->Add(BitmapButton_Piano_MapFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText14_layout_kludge2 = new wxStaticText(Panel4, ID_STATICTEXT13, _("                                    x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	StaticText14_layout_kludge2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	FlexGridSizer33->Add(StaticText14_layout_kludge2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer30->Add(FlexGridSizer33, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Piano_MapFilename = new wxTextCtrl(Panel4, ID_TEXTCTRL_Piano_MapFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_MapFilename"));
	FlexGridSizer30->Add(TextCtrl_Piano_MapFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel4->SetSizer(FlexGridSizer30);
	FlexGridSizer30->Fit(Panel4);
	FlexGridSizer30->SetSizeHints(Panel4);
	Panel5 = new wxPanel(Notebook2, ID_PANEL13, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL13"));
	FlexGridSizer32 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer34 = new wxFlexGridSizer(0, 5, 0, 0);
	Button_Piano_ShapeFilename = new wxButton(Panel5, ID_BUTTON2, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	Button_Piano_ShapeFilename->SetBackgroundColour(wxColour(224,224,224));
	FlexGridSizer34->Add(Button_Piano_ShapeFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Piano_ShapeFilename = new wxBitmapButton(Panel5, ID_BITMAPBUTTON_Piano_ShapeFilename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_ShapeFilename"));
	BitmapButton_Piano_ShapeFilename->SetDefault();
	FlexGridSizer34->Add(BitmapButton_Piano_ShapeFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer34->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Piano_Clipping = new wxCheckBox(Panel5, ID_CHECKBOX1, _("Clip shapes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Piano_Clipping->SetValue(false);
	FlexGridSizer34->Add(CheckBox_Piano_Clipping, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Piano_Clipping = new wxBitmapButton(Panel5, ID_BITMAPBUTTON_Piano_Clipping, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Clipping"));
	BitmapButton_Piano_Clipping->SetDefault();
	FlexGridSizer34->Add(BitmapButton_Piano_Clipping, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer32->Add(FlexGridSizer34, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Piano_ShapeFilename = new wxTextCtrl(Panel5, ID_TEXTCTRL_Piano_ShapeFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_ShapeFilename"));
	FlexGridSizer32->Add(TextCtrl_Piano_ShapeFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel5->SetSizer(FlexGridSizer32);
	FlexGridSizer32->Fit(Panel5);
	FlexGridSizer32->SetSizeHints(Panel5);
	Notebook2->AddPage(Panel3, _("Cues"), false);
	Notebook2->AddPage(Panel4, _("Cue-Shape Map"), false);
	Notebook2->AddPage(Panel5, _("Shapes"), false);
	FlexGridSizer5->Add(Notebook2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(this);
	FlexGridSizer5->SetSizeHints(this);

	Connect(ID_CHOICE_Piano_Style,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PianoPanel::OnPiano_StyleSelect);
	Connect(ID_BITMAPBUTTON_SLIDER_Piano_Style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Piano_NumKeys,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PianoPanel::OnSlider_Piano_NumKeysCmdScroll);
	Connect(ID_BITMAPBUTTON_Piano_NumKeys,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Piano_KeyWidth,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PianoPanel::OnSlider_Piano_NumRowsCmdScroll);
	Connect(ID_BITMAPBUTTON_Piano_NumRows,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_KeyPlacement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BUTTON_Piano_CueFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnButton_Piano_CueFilenameClick);
	Connect(ID_BITMAPBUTTON_Piano_Filename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnButton_Piano_MapFilenameClick);
	Connect(ID_BITMAPBUTTON_Piano_MapFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnButton_Piano_ShapeFilenameClick);
	Connect(ID_BITMAPBUTTON_Piano_ShapeFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_Piano_Clipping,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PianoPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_PIANO");
    
    //pre-set Piano style, shapes + map files:
    wxFileName myfile(::wxStandardPaths::Get().GetExecutablePath()); //start in folder with .EXXE (there is no "bin" subfolder after an install)
    myfile.AppendDir("piano"); //piano files moved to separate subfolder
    
    myfile.SetName("Piano-88KeyShapeMap");
    myfile.SetExt("txt");
    TextCtrl_Piano_MapFilename->SetValue(myfile.GetFullPath());
    myfile.SetName("Piano-ExampleKeyTopShapes");
    myfile.SetExt("png");
    TextCtrl_Piano_ShapeFilename->SetValue(myfile.GetFullPath());

}

PianoPanel::~PianoPanel()
{
	//(*Destroy(PianoPanel)
	//*)
}

PANEL_EVENT_HANDLERS(PianoPanel)




#if defined(__WXMAC__)
//OSX doesn't allow wildcards in the "name" part
static const wxString NOTES_WILDCARD("Notes cue files (*notes.txt;*.not)|*.txt;*.not");
static const wxString CUEMAP_WILDCARD("Cue map files (*map.txt;*.map)|*.txt;*.map");
#else
static const wxString NOTES_WILDCARD("Notes cue files (*notes.txt;*.not)|*notes.txt;*.not");
static const wxString CUEMAP_WILDCARD("Cue map files (*map.txt;*.map)|*map.txt;*.map");
#endif

void PianoPanel::OnButton_Piano_CueFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Notes Cue File", defaultDir, "", "",
                                       NOTES_WILDCARD, wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_CueFilename->SetValue(filename);
}

void PianoPanel::OnButton_Piano_MapFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Cue Map File", defaultDir, "", "",
                                       CUEMAP_WILDCARD, wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_MapFilename->SetValue(filename);
}

void PianoPanel::OnButton_Piano_ShapeFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Shapes File", defaultDir, "", "",
                                       "Key shapes files " + wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_ShapeFilename->SetValue(filename);
}

//new Piano effect UI handlers: -DJ
void PianoPanel::OnPiano_StyleSelect(wxCommandEvent& event)
{
    //TODO?
}

//#define WANT_DEBUG 99
//#include "djdebug.cpp"
void PianoPanel::OnSlider_Piano_NumKeysCmdScroll(wxScrollEvent& event)
{
    //    debug(1, "#keys %d", Slider_Piano_NumKeys->GetValue());
    //    TextCtrl_Speed->SetValue(wxString::Format("%d",Slider_Speed->GetValue()));
    //    txtCtrlSparkleFreq->SetValue(wxString::Format("%d",Slider_SparkleFrequency->GetValue()));
    //wxMessageBox(wxString::Format("# Keys (%d):", Slider_Piano_NumKeys->GetValue()));
    //broken, use wxsl_lables instead for now:
    StaticText_Piano_NumKeys->SetLabel(wxString::Format("# Keys (%d):", Slider_Piano_NumKeys->GetValue())); //show slider value for user friendliness
    //    Layout();
}

void PianoPanel::OnSlider_Piano_NumRowsCmdScroll(wxScrollEvent& event)
{
    //    debug(1, "#rows %d", Slider_Piano_NumRows->GetValue());
    //broken, use wxsl_lables instead for now:
    StaticText_Piano_NumRows->SetLabel(wxString::Format("# Rows (%d):", Slider_Piano_NumRows->GetValue())); //show slider value for user friendliness
    //    Layout();
    //wxMessageBox(wxString::Format("# Rows (%d):", Slider_Piano_NumRows->GetValue()));
}
