/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsMain.h"
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/valnum.h>

// dialogs
#include "SerialPortWithRate.h"
#include "E131Dialog.h"

// xml
#include "../include/xml-irr-1.2/irrXML.h"
#include "../include/xml-irr-1.2/irrXML.cpp"
using namespace irr;
using namespace io;

// scripting language
#include "xLightsBasic.cpp"

// image files
#include "../include/xlights.xpm"
#include "../include/open.xpm"
#include "../include/save.xpm"
#include "../include/insertrow.xpm"
#include "../include/deleterow.xpm"

//(*InternalHeaders(xLightsFrame)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)


//helper functions
enum wxbuildinfoformat
{
    short_f, long_f
};

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(xLightsFrame)
const long xLightsFrame::ID_BITMAPBUTTON_TAB_INFO = wxNewId();
const long xLightsFrame::ID_BUTTON_STOP_NOW = wxNewId();
const long xLightsFrame::ID_BUTTON_GRACEFUL_STOP = wxNewId();
const long xLightsFrame::ID_BUTTON_LIGHTS_OFF = wxNewId();
const long xLightsFrame::ID_CHECKBOX_LIGHT_OUTPUT = wxNewId();
const long xLightsFrame::ID_STATICTEXT_SETUP1 = wxNewId();
const long xLightsFrame::ID_STATICTEXT_DIRNAME = wxNewId();
const long xLightsFrame::ID_BUTTON_CHANGEDIR = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_SETUP = wxNewId();
const long xLightsFrame::ID_BUTTON_ADD_DONGLE = wxNewId();
const long xLightsFrame::ID_BUTTON_ADD_E131 = wxNewId();
const long xLightsFrame::ID_BUTTON_NETWORK_CHANGE = wxNewId();
const long xLightsFrame::ID_BUTTON_NETWORK_DELETE = wxNewId();
const long xLightsFrame::ID_BUTTON_NETWORK_DELETE_ALL = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON1 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON2 = wxNewId();
const long xLightsFrame::ID_LISTCTRL_NETWORKS = wxNewId();
const long xLightsFrame::ID_PANEL_SETUP = wxNewId();
const long xLightsFrame::ID_BUTTON_SELECT_ALL = wxNewId();
const long xLightsFrame::ID_BUTTON_CLEAR_ALL = wxNewId();
const long xLightsFrame::ID_BUTTON_LOAD = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE = wxNewId();
const long xLightsFrame::ID_STATICTEXT6 = wxNewId();
const long xLightsFrame::ID_CHECKLISTBOX_TEST_CHANNELS = wxNewId();
const long xLightsFrame::ID_STATICTEXT8 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON14 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON15 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON16 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON17 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON19 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON18 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON7 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON31 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON30 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON29 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON8 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON6 = wxNewId();
const long xLightsFrame::ID_STATICTEXT9 = wxNewId();
const long xLightsFrame::ID_SLIDER4 = wxNewId();
const long xLightsFrame::ID_STATICTEXT10 = wxNewId();
const long xLightsFrame::ID_SLIDER5 = wxNewId();
const long xLightsFrame::ID_STATICTEXT11 = wxNewId();
const long xLightsFrame::ID_SLIDER6 = wxNewId();
const long xLightsFrame::ID_PANEL_TEST_STANDARD = wxNewId();
const long xLightsFrame::ID_STATICTEXT29 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON3 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON4 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON5 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON9 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON25 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON10 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON12 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON28 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON11 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON13 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON20 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON21 = wxNewId();
const long xLightsFrame::ID_SLIDER12 = wxNewId();
const long xLightsFrame::ID_SLIDER11 = wxNewId();
const long xLightsFrame::ID_SLIDER1 = wxNewId();
const long xLightsFrame::ID_SLIDER14 = wxNewId();
const long xLightsFrame::ID_SLIDER13 = wxNewId();
const long xLightsFrame::ID_SLIDER9 = wxNewId();
const long xLightsFrame::ID_STATICTEXT7 = wxNewId();
const long xLightsFrame::ID_SLIDER2 = wxNewId();
const long xLightsFrame::ID_PANEL_TEST_RGB = wxNewId();
const long xLightsFrame::ID_STATICTEXT12 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON22 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON23 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON24 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON26 = wxNewId();
const long xLightsFrame::ID_RADIOBUTTON27 = wxNewId();
const long xLightsFrame::ID_STATICTEXT13 = wxNewId();
const long xLightsFrame::ID_SLIDER7 = wxNewId();
const long xLightsFrame::ID_PANEL_RGB_CYCLE = wxNewId();
const long xLightsFrame::ID_NOTEBOOK_TEST = wxNewId();
const long xLightsFrame::ID_PANEL_TEST = wxNewId();
const long xLightsFrame::ID_STATICTEXT14 = wxNewId();
const long xLightsFrame::ID_STATICTEXT19 = wxNewId();
const long xLightsFrame::ID_STATICTEXT15 = wxNewId();
const long xLightsFrame::ID_BUTTON_CHOOSE_FILE = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_FILENAME = wxNewId();
const long xLightsFrame::ID_STATICTEXT16 = wxNewId();
const long xLightsFrame::ID_CHOICE_OUTPUT_FORMAT = wxNewId();
const long xLightsFrame::ID_STATICTEXT20 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_SAVE_CHANNEL_NAMES = wxNewId();
const long xLightsFrame::ID_STATICTEXT17 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_OFF_AT_END = wxNewId();
const long xLightsFrame::ID_BUTTON_START_CONVERSION = wxNewId();
const long xLightsFrame::ID_STATICTEXT18 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_CONVERSION_STATUS = wxNewId();
const long xLightsFrame::ID_PANEL_CONVERT = wxNewId();
const long xLightsFrame::ID_BUTTON_PREVIEW_OPEN = wxNewId();
const long xLightsFrame::ID_STATICTEXT23 = wxNewId();
const long xLightsFrame::ID_BUTTON_PLAY_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_STOP_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_REPEAT_PREVIEW = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PREVIEW_TIME = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_TIME = wxNewId();
const long xLightsFrame::ID_STATICTEXT21 = wxNewId();
const long xLightsFrame::ID_LISTBOX_ELEMENT_LIST = wxNewId();
const long xLightsFrame::ID_BUTTON_MODELS_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long xLightsFrame::ID_STATICTEXT22 = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_SCALE = wxNewId();
const long xLightsFrame::ID_STATICTEXT25 = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_ROTATE = wxNewId();
const long xLightsFrame::ID_SCROLLEDWINDOW_PREVIEW = wxNewId();
const long xLightsFrame::ID_PANEL_PREVIEW = wxNewId();
const long xLightsFrame::ID_SCROLLEDWINDOW1 = wxNewId();
const long xLightsFrame::ID_BUTTON13 = wxNewId();
const long xLightsFrame::ID_BUTTON3 = wxNewId();
const long xLightsFrame::ID_BUTTON58 = wxNewId();
const long xLightsFrame::ID_CHOICE7 = wxNewId();
const long xLightsFrame::ID_BUTTON59 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette = wxNewId();
const long xLightsFrame::ID_CHOICE_LayerMethod = wxNewId();
const long xLightsFrame::ID_SLIDER_EffectLayerMix = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_LayerMix = wxNewId();
const long xLightsFrame::ID_STATICTEXT24 = wxNewId();
const long xLightsFrame::ID_SLIDER_SparkleFrequency = wxNewId();
const long xLightsFrame::ID_TEXTCTRL5 = wxNewId();
const long xLightsFrame::ID_STATICTEXT127 = wxNewId();
const long xLightsFrame::ID_SLIDER_Brightness = wxNewId();
const long xLightsFrame::ID_TEXTCTRL6 = wxNewId();
const long xLightsFrame::ID_STATICTEXT128 = wxNewId();
const long xLightsFrame::ID_SLIDER_Contrast = wxNewId();
const long xLightsFrame::ID_TEXTCTRL7 = wxNewId();
const long xLightsFrame::ID_PANEL31 = wxNewId();
const long xLightsFrame::ID_STATICTEXT4 = wxNewId();
const long xLightsFrame::ID_BUTTON_PLAY_RGB_SEQ = wxNewId();
const long xLightsFrame::ID_BUTTON2 = wxNewId();
const long xLightsFrame::ID_BUTTON1 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON7 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON9 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON3 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON4 = wxNewId();
const long xLightsFrame::ID_BUTTON_SeqExport = wxNewId();
const long xLightsFrame::ID_BUTTON_CREATE_RANDOM = wxNewId();
const long xLightsFrame::ID_GRID1 = wxNewId();
const long xLightsFrame::ID_PANEL_EFFECTS1 = wxNewId();
const long xLightsFrame::ID_PANEL_EFFECTS2 = wxNewId();
const long xLightsFrame::ID_PANEL32 = wxNewId();
const long xLightsFrame::ID_SPLITTERWINDOW2 = wxNewId();
const long xLightsFrame::ID_PANEL30 = wxNewId();
const long xLightsFrame::ID_TREECTRL1 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_RUN_SCHEDULE = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_SCHEDULE = wxNewId();
const long xLightsFrame::ID_BUTTON_ADD_SHOW = wxNewId();
const long xLightsFrame::ID_BUTTON_UPDATE_SHOW = wxNewId();
const long xLightsFrame::ID_BUTTON_DELETE_SHOW = wxNewId();
const long xLightsFrame::ID_STATICTEXT2 = wxNewId();
const long xLightsFrame::ID_BUTTON_SHOW_DATES_CHANGE = wxNewId();
const long xLightsFrame::ID_STATICTEXT3 = wxNewId();
const long xLightsFrame::ID_STATICTEXT_SHOWSTART = wxNewId();
const long xLightsFrame::ID_STATICTEXT5 = wxNewId();
const long xLightsFrame::ID_STATICTEXT_SHOWEND = wxNewId();
const long xLightsFrame::ID_PANEL3 = wxNewId();
const long xLightsFrame::ID_STATICTEXT1 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_LOG = wxNewId();
const long xLightsFrame::ID_BUTTON_CLEARLOG = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVELOG = wxNewId();
const long xLightsFrame::ID_PANEL2 = wxNewId();
const long xLightsFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xLightsFrame::ID_PANEL_CAL = wxNewId();
const long xLightsFrame::ID_NOTEBOOK1 = wxNewId();
const long xLightsFrame::ID_PANEL1 = wxNewId();
const long xLightsFrame::ID_MENUITEM2 = wxNewId();
const long xLightsFrame::ID_FILE_BACKUP = wxNewId();
const long xLightsFrame::idMenuQuit = wxNewId();
const long xLightsFrame::idMenuSaveSched = wxNewId();
const long xLightsFrame::idMenuAddList = wxNewId();
const long xLightsFrame::idMenuRenameList = wxNewId();
const long xLightsFrame::idMenuDelList = wxNewId();
const long xLightsFrame::ID_MENUITEM1 = wxNewId();
const long xLightsFrame::idCustomScript = wxNewId();
const long xLightsFrame::idMenuHelpContent = wxNewId();
const long xLightsFrame::idMenuAbout = wxNewId();
const long xLightsFrame::ID_STATUSBAR1 = wxNewId();
const long xLightsFrame::ID_TIMER1 = wxNewId();
const long xLightsFrame::ID_MESSAGEDIALOG1 = wxNewId();
//*)

const long xLightsFrame::ID_PLAYER_DIALOG = wxNewId();
const long xLightsFrame::ID_DELETE_EFFECT = wxNewId();
const long xLightsFrame::ID_IGNORE_CLICK = wxNewId();
const long xLightsFrame::ID_PROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_UNPROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_RANDOM_EFFECT = wxNewId();


BEGIN_EVENT_TABLE(xLightsFrame,wxFrame)
    //(*EventTable(xLightsFrame)
    //*)
END_EVENT_TABLE()

xLightsFrame::xLightsFrame(wxWindow* parent,wxWindowID id)
{

    //(*Initialize(xLightsFrame)
    wxBoxSizer* BoxSizer4;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxBoxSizer* BoxSizer6;
    wxMenu* MenuHelp;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizerTest;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizerSetup;
    wxFlexGridSizer* FlexGridSizer24;
    wxMenuItem* MenuItemCustomScript;
    wxStaticBoxSizer* StaticBoxSizerHighlightColor;
    wxBoxSizer* BoxSizer5;
    wxFlexGridSizer* FlexGridSizer19;
    wxBoxSizer* BoxSizer10;
    wxFlexGridSizer* FlexGridSizer70;
    wxFlexGridSizer* FlexGridSizer23;
    wxMenuItem* MenuItem5;
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItemAddList;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer27;
    wxMenuItem* MenuItem1;
    wxMenuItem* MenuItem4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer25;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticText* StaticText21;
    wxMenuItem* MenuItemDelList;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizer29;
    wxFlexGridSizer* FlexGridSizer34;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer21;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer20;
    wxBoxSizer* BoxSizer1;
    wxMenuItem* MenuItemRenameList;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer35;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer36;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer33;
    wxFlexGridSizer* FlexGridSizerConvert;
    wxFlexGridSizer* FlexGridSizer11;
    wxBoxSizer* BoxSizer3;
    wxFlexGridSizer* FlexGridSizer17;
    wxStaticBoxSizer* StaticBoxSizer5;
    wxFlexGridSizer* FlexGridSizer32;
    wxFlexGridSizer* FlexGridSizer68;
    wxFlexGridSizer* FlexGridSizer31;
    wxFlexGridSizer* FlexGridSizer28;
    wxStaticBoxSizer* StaticBoxSizerBackgroundColor;
    wxMenu* MenuPlaylist;
    wxFlexGridSizer* FlexGridSizer26;
    wxFlexGridSizer* FlexGridSizer30;

    Create(parent, wxID_ANY, _("xLights/Nutcracker  (Ver 3.1.4)"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_PANEL1"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(1);
    FlexGridSizer19 = new wxFlexGridSizer(0, 6, 0, 0);
    BitmapButtonTabInfo = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_TAB_INFO, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_INFORMATION")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxRAISED_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TAB_INFO"));
    BitmapButtonTabInfo->SetToolTip(_("Tips for using the current tab"));
    FlexGridSizer19->Add(BitmapButtonTabInfo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStopNow = new wxButton(Panel1, ID_BUTTON_STOP_NOW, _("Stop Now!"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_STOP_NOW"));
    FlexGridSizer19->Add(ButtonStopNow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonGracefulStop = new wxButton(Panel1, ID_BUTTON_GRACEFUL_STOP, _("Graceful Stop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GRACEFUL_STOP"));
    FlexGridSizer19->Add(ButtonGracefulStop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonLightsOff = new wxButton(Panel1, ID_BUTTON_LIGHTS_OFF, _("Lights Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LIGHTS_OFF"));
    FlexGridSizer19->Add(ButtonLightsOff, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxLightOutput = new wxCheckBox(Panel1, ID_CHECKBOX_LIGHT_OUTPUT, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LIGHT_OUTPUT"));
    CheckBoxLightOutput->SetValue(false);
    FlexGridSizer19->Add(CheckBoxLightOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer19, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Notebook1 = new wxNotebook(Panel1, ID_NOTEBOOK1, wxDefaultPosition, wxSize(1180,500), 0, _T("ID_NOTEBOOK1"));
    Notebook1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    PanelSetup = new wxPanel(Notebook1, ID_PANEL_SETUP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_SETUP"));
    FlexGridSizerSetup = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetup->AddGrowableCol(0);
    FlexGridSizerSetup->AddGrowableRow(1);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, PanelSetup, _("Show Directory"));
    StaticTextSetup1 = new wxStaticText(PanelSetup, ID_STATICTEXT_SETUP1, _("All sequences and media files must be in this directory"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SETUP1"));
    wxFont StaticTextSetup1Font(10,wxDEFAULT,wxFONTSTYLE_ITALIC,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticTextSetup1->SetFont(StaticTextSetup1Font);
    StaticBoxSizer1->Add(StaticTextSetup1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDirName = new wxStaticText(PanelSetup, ID_STATICTEXT_DIRNAME, _("<No directory selected - SET THIS FIRST>"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DIRNAME"));
    StaticTextDirName->SetMinSize(wxSize(300,0));
    StaticTextDirName->SetForegroundColour(wxColour(0,0,255));
    StaticBoxSizer1->Add(StaticTextDirName, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonChangeDir = new wxButton(PanelSetup, ID_BUTTON_CHANGEDIR, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CHANGEDIR"));
    StaticBoxSizer1->Add(ButtonChangeDir, 1, wxALL|wxFIXED_MINSIZE|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelSetup, _("Lighting Networks"));
    FlexGridSizerNetworks = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizerNetworks->AddGrowableCol(2);
    FlexGridSizerNetworks->AddGrowableRow(0);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    ButtonSaveSetup = new wxButton(PanelSetup, ID_BUTTON_SAVE_SETUP, _("Save Setup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_SETUP"));
    BoxSizer1->Add(ButtonSaveSetup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonAddDongle = new wxButton(PanelSetup, ID_BUTTON_ADD_DONGLE, _("Add USB"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_DONGLE"));
    BoxSizer1->Add(ButtonAddDongle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonAddE131 = new wxButton(PanelSetup, ID_BUTTON_ADD_E131, _("Add E1.31"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_E131"));
    BoxSizer1->Add(ButtonAddE131, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonNetworkChange = new wxButton(PanelSetup, ID_BUTTON_NETWORK_CHANGE, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NETWORK_CHANGE"));
    BoxSizer1->Add(ButtonNetworkChange, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonNetworkDelete = new wxButton(PanelSetup, ID_BUTTON_NETWORK_DELETE, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NETWORK_DELETE"));
    BoxSizer1->Add(ButtonNetworkDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonNetworkDeleteAll = new wxButton(PanelSetup, ID_BUTTON_NETWORK_DELETE_ALL, _("Delete All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NETWORK_DELETE_ALL"));
    BoxSizer1->Add(ButtonNetworkDeleteAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizerNetworks->Add(BoxSizer1, 1, wxALIGN_LEFT|wxALIGN_TOP, 0);
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    BitmapButtonMoveNetworkUp = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_UP")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    BitmapButtonMoveNetworkUp->SetDefault();
    BitmapButtonMoveNetworkUp->SetToolTip(_("Move selected item up"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkUp, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonMoveNetworkDown = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON2, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_GO_DOWN")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    BitmapButtonMoveNetworkDown->SetDefault();
    BitmapButtonMoveNetworkDown->SetToolTip(_("Move selected item down"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkDown, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerNetworks->Add(FlexGridSizer9, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 10);
    GridNetwork = new wxListCtrl(PanelSetup, ID_LISTCTRL_NETWORKS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL, wxDefaultValidator, _T("ID_LISTCTRL_NETWORKS"));
    GridNetwork->SetToolTip(_("Drag an item to reorder the list"));
    FlexGridSizerNetworks->Add(GridNetwork, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    StaticBoxSizer2->Add(FlexGridSizerNetworks, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelSetup->SetSizer(FlexGridSizerSetup);
    FlexGridSizerSetup->Fit(PanelSetup);
    FlexGridSizerSetup->SetSizeHints(PanelSetup);
    PanelTest = new wxPanel(Notebook1, ID_PANEL_TEST, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_TEST"));
    FlexGridSizerTest = new wxFlexGridSizer(2, 2, 0, 0);
    FlexGridSizerTest->AddGrowableCol(1);
    FlexGridSizerTest->AddGrowableRow(1);
    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer10->AddGrowableCol(0);
    FlexGridSizer10->AddGrowableRow(0);
    FlexGridSizer11 = new wxFlexGridSizer(1, 4, 0, 0);
    ButtonTestSelectAll = new wxButton(PanelTest, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
    FlexGridSizer11->Add(ButtonTestSelectAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTestClear = new wxButton(PanelTest, ID_BUTTON_CLEAR_ALL, _("Clear All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLEAR_ALL"));
    FlexGridSizer11->Add(ButtonTestClear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTestLoad = new wxButton(PanelTest, ID_BUTTON_LOAD, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LOAD"));
    ButtonTestLoad->Disable();
    FlexGridSizer11->Add(ButtonTestLoad, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonTestSave = new wxButton(PanelTest, ID_BUTTON_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE"));
    ButtonTestSave->Disable();
    FlexGridSizer11->Add(ButtonTestSave, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(FlexGridSizer11, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(PanelTest, ID_STATICTEXT6, _("Use Shift-click, Ctrl-click, or click && drag to select multiple channels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer10->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerTest->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerTest->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckListBoxTestChannels = new wxCheckListBox(PanelTest, ID_CHECKLISTBOX_TEST_CHANNELS, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_EXTENDED, wxDefaultValidator, _T("ID_CHECKLISTBOX_TEST_CHANNELS"));
    FlexGridSizerTest->Add(CheckListBoxTestChannels, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    NotebookTest = new wxNotebook(PanelTest, ID_NOTEBOOK_TEST, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_TEST"));
    NotebookTest->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    PanelTestStandard = new wxPanel(NotebookTest, ID_PANEL_TEST_STANDARD, wxPoint(232,105), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_TEST_STANDARD"));
    FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer13 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText8 = new wxStaticText(PanelTestStandard, ID_STATICTEXT8, _("Select function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    wxFont StaticText8Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText8->SetFont(StaticText8Font);
    FlexGridSizer13->Add(StaticText8, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonOff = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON14, _("Off"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON14"));
    FlexGridSizer13->Add(RadioButtonOff, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonChase = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON15, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON15"));
    FlexGridSizer13->Add(RadioButtonChase, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonChase3 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON16, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON16"));
    FlexGridSizer13->Add(RadioButtonChase3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonChase4 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON17, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON17"));
    FlexGridSizer13->Add(RadioButtonChase4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonChase5 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON19, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON19"));
    FlexGridSizer13->Add(RadioButtonChase5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonAlt = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON18, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON18"));
    FlexGridSizer13->Add(RadioButtonAlt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonTwinkle05 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON7, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON7"));
    FlexGridSizer13->Add(RadioButtonTwinkle05, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonTwinkle10 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON31, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON31"));
    FlexGridSizer13->Add(RadioButtonTwinkle10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonTwinkle25 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON30, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON30"));
    FlexGridSizer13->Add(RadioButtonTwinkle25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonTwinkle50 = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON29, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON29"));
    FlexGridSizer13->Add(RadioButtonTwinkle50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonShimmer = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON8, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON8"));
    FlexGridSizer13->Add(RadioButtonShimmer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonDim = new wxRadioButton(PanelTestStandard, ID_RADIOBUTTON6, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON6"));
    FlexGridSizer13->Add(RadioButtonDim, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(FlexGridSizer13, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    FlexGridSizer20 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer20->AddGrowableCol(0);
    FlexGridSizer20->AddGrowableRow(1);
    StaticText9 = new wxStaticText(PanelTestStandard, ID_STATICTEXT9, _("Background\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    wxFont StaticText9Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText9->SetFont(StaticText9Font);
    FlexGridSizer20->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderBgIntensity = new wxSlider(PanelTestStandard, ID_SLIDER4, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER4"));
    FlexGridSizer20->Add(SliderBgIntensity, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer3->Add(FlexGridSizer20, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer21 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer21->AddGrowableCol(0);
    FlexGridSizer21->AddGrowableRow(1);
    StaticText10 = new wxStaticText(PanelTestStandard, ID_STATICTEXT10, _("Highlight\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    wxFont StaticText10Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText10->SetFont(StaticText10Font);
    FlexGridSizer21->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderFgIntensity = new wxSlider(PanelTestStandard, ID_SLIDER5, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER5"));
    FlexGridSizer21->Add(SliderFgIntensity, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer3->Add(FlexGridSizer21, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer22 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer22->AddGrowableCol(0);
    FlexGridSizer22->AddGrowableRow(1);
    StaticText11 = new wxStaticText(PanelTestStandard, ID_STATICTEXT11, _("Speed\n"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    wxFont StaticText11Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText11->SetFont(StaticText11Font);
    FlexGridSizer22->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderChaseSpeed = new wxSlider(PanelTestStandard, ID_SLIDER6, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER6"));
    FlexGridSizer22->Add(SliderChaseSpeed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer3->Add(FlexGridSizer22, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer12->Add(BoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    PanelTestStandard->SetSizer(FlexGridSizer12);
    FlexGridSizer12->Fit(PanelTestStandard);
    FlexGridSizer12->SetSizeHints(PanelTestStandard);
    PanelTestRgb = new wxPanel(NotebookTest, ID_PANEL_TEST_RGB, wxPoint(62,20), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_TEST_RGB"));
    FlexGridSizer14 = new wxFlexGridSizer(0, 6, 0, 0);
    FlexGridSizer15 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText29 = new wxStaticText(PanelTestRgb, ID_STATICTEXT29, _("Select function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    wxFont StaticText29Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText29->SetFont(StaticText29Font);
    FlexGridSizer15->Add(StaticText29, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbChaseOff = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON3, _("Off"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
    FlexGridSizer15->Add(RadioButtonRgbChaseOff, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbChase = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON4, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
    FlexGridSizer15->Add(RadioButtonRgbChase, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbChase3 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON5, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON5"));
    FlexGridSizer15->Add(RadioButtonRgbChase3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbChase4 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON9, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON9"));
    FlexGridSizer15->Add(RadioButtonRgbChase4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbChase5 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON25, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON25"));
    FlexGridSizer15->Add(RadioButtonRgbChase5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbAlt = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON10, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON10"));
    FlexGridSizer15->Add(RadioButtonRgbAlt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbTwinkle05 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON12, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON12"));
    FlexGridSizer15->Add(RadioButtonRgbTwinkle05, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbTwinkle10 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON28, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON28"));
    FlexGridSizer15->Add(RadioButtonRgbTwinkle10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbTwinkle25 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON11, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON11"));
    FlexGridSizer15->Add(RadioButtonRgbTwinkle25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbTwinkle50 = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON13, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON13"));
    FlexGridSizer15->Add(RadioButtonRgbTwinkle50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbShimmer = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON20, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON20"));
    FlexGridSizer15->Add(RadioButtonRgbShimmer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbDim = new wxRadioButton(PanelTestRgb, ID_RADIOBUTTON21, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON21"));
    FlexGridSizer15->Add(RadioButtonRgbDim, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14->Add(FlexGridSizer15, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer14->Add(-1,-1,1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    BoxSizer10 = new wxBoxSizer(wxVERTICAL);
    StaticBoxSizerBackgroundColor = new wxStaticBoxSizer(wxHORIZONTAL, PanelTestRgb, _("Background Color"));
    SliderBgColorA = new wxSlider(PanelTestRgb, ID_SLIDER12, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER12"));
    StaticBoxSizerBackgroundColor->Add(SliderBgColorA, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    SliderBgColorB = new wxSlider(PanelTestRgb, ID_SLIDER11, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER11"));
    StaticBoxSizerBackgroundColor->Add(SliderBgColorB, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    SliderBgColorC = new wxSlider(PanelTestRgb, ID_SLIDER1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER1"));
    StaticBoxSizerBackgroundColor->Add(SliderBgColorC, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer10->Add(StaticBoxSizerBackgroundColor, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP, 3);
    StaticBoxSizerHighlightColor = new wxStaticBoxSizer(wxHORIZONTAL, PanelTestRgb, _("Highlight Color"));
    SliderFgColorA = new wxSlider(PanelTestRgb, ID_SLIDER14, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER14"));
    StaticBoxSizerHighlightColor->Add(SliderFgColorA, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    SliderFgColorB = new wxSlider(PanelTestRgb, ID_SLIDER13, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER13"));
    StaticBoxSizerHighlightColor->Add(SliderFgColorB, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    SliderFgColorC = new wxSlider(PanelTestRgb, ID_SLIDER9, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER9"));
    StaticBoxSizerHighlightColor->Add(SliderFgColorC, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    BoxSizer10->Add(StaticBoxSizerHighlightColor, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer14->Add(BoxSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14->Add(-1,-1,1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer16 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer16->AddGrowableCol(0);
    FlexGridSizer16->AddGrowableRow(1);
    StaticText7 = new wxStaticText(PanelTestRgb, ID_STATICTEXT7, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    wxFont StaticText7Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText7->SetFont(StaticText7Font);
    FlexGridSizer16->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderRgbChaseSpeed = new wxSlider(PanelTestRgb, ID_SLIDER2, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER2"));
    FlexGridSizer16->Add(SliderRgbChaseSpeed, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer14->Add(FlexGridSizer16, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    PanelTestRgb->SetSizer(FlexGridSizer14);
    FlexGridSizer14->Fit(PanelTestRgb);
    FlexGridSizer14->SetSizeHints(PanelTestRgb);
    PanelRgbCycle = new wxPanel(NotebookTest, ID_PANEL_RGB_CYCLE, wxPoint(156,13), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_RGB_CYCLE"));
    FlexGridSizer17 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer18 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText12 = new wxStaticText(PanelRgbCycle, ID_STATICTEXT12, _("Select function:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    wxFont StaticText12Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText12->SetFont(StaticText12Font);
    FlexGridSizer18->Add(StaticText12, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbCycleOff = new wxRadioButton(PanelRgbCycle, ID_RADIOBUTTON22, _("Off"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON22"));
    FlexGridSizer18->Add(RadioButtonRgbCycleOff, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbCycle3 = new wxRadioButton(PanelRgbCycle, ID_RADIOBUTTON23, _("A-B-C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON23"));
    FlexGridSizer18->Add(RadioButtonRgbCycle3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbCycle4 = new wxRadioButton(PanelRgbCycle, ID_RADIOBUTTON24, _("A-B-C-All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON24"));
    FlexGridSizer18->Add(RadioButtonRgbCycle4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbCycle5 = new wxRadioButton(PanelRgbCycle, ID_RADIOBUTTON26, _("A-B-C-All-None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON26"));
    FlexGridSizer18->Add(RadioButtonRgbCycle5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    RadioButtonRgbCycleMixed = new wxRadioButton(PanelRgbCycle, ID_RADIOBUTTON27, _("Mixed Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON27"));
    FlexGridSizer18->Add(RadioButtonRgbCycleMixed, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer17->Add(FlexGridSizer18, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer17->Add(-1,-1,1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer24 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer24->AddGrowableRow(1);
    StaticText13 = new wxStaticText(PanelRgbCycle, ID_STATICTEXT13, _("Cycle Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    wxFont StaticText13Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText13->SetFont(StaticText13Font);
    FlexGridSizer24->Add(StaticText13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderRgbCycleSpeed = new wxSlider(PanelRgbCycle, ID_SLIDER7, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxSIMPLE_BORDER, wxDefaultValidator, _T("ID_SLIDER7"));
    SliderRgbCycleSpeed->SetMinSize(wxSize(-1,200));
    FlexGridSizer24->Add(SliderRgbCycleSpeed, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    FlexGridSizer17->Add(FlexGridSizer24, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    PanelRgbCycle->SetSizer(FlexGridSizer17);
    FlexGridSizer17->Fit(PanelRgbCycle);
    FlexGridSizer17->SetSizeHints(PanelRgbCycle);
    NotebookTest->AddPage(PanelTestStandard, _("Standard"), false);
    NotebookTest->AddPage(PanelTestRgb, _("RGB"), false);
    NotebookTest->AddPage(PanelRgbCycle, _("RGB Cycle"), false);
    FlexGridSizerTest->Add(NotebookTest, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    PanelTest->SetSizer(FlexGridSizerTest);
    FlexGridSizerTest->Fit(PanelTest);
    FlexGridSizerTest->SetSizeHints(PanelTest);
    PanelConvert = new wxPanel(Notebook1, ID_PANEL_CONVERT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CONVERT"));
    FlexGridSizerConvert = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerConvert->AddGrowableCol(0);
    StaticText14 = new wxStaticText(PanelConvert, ID_STATICTEXT14, _("xLights File Converter"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    wxFont StaticText14Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText14->SetFont(StaticText14Font);
    FlexGridSizerConvert->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 10);
    StaticText19 = new wxStaticText(PanelConvert, ID_STATICTEXT19, _("Entries on the Setup tab will guide the conversion. Make sure you have your lighting networks defined accurately before you start a conversion."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizerConvert->Add(StaticText19, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 12);
    FlexGridSizer25 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer25->AddGrowableCol(1);
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    StaticText15 = new wxStaticText(PanelConvert, ID_STATICTEXT15, _("Sequence files to convert:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    FlexGridSizer6->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonChooseFile = new wxButton(PanelConvert, ID_BUTTON_CHOOSE_FILE, _("Choose Files"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CHOOSE_FILE"));
    FlexGridSizer6->Add(ButtonChooseFile, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP, 5);
    TextCtrlFilename = new wxTextCtrl(PanelConvert, ID_TEXTCTRL_FILENAME, wxEmptyString, wxDefaultPosition, wxSize(260,80), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_FILENAME"));
    FlexGridSizer6->Add(TextCtrlFilename, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer26 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText16 = new wxStaticText(PanelConvert, ID_STATICTEXT16, _("Output Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer26->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceOutputFormat = new wxChoice(PanelConvert, ID_CHOICE_OUTPUT_FORMAT, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_OUTPUT_FORMAT"));
    ChoiceOutputFormat->SetSelection( ChoiceOutputFormat->Append(_("xLights Sequence, *.xseq")) );
    ChoiceOutputFormat->Append(_("Falcon Pi Player, *.fseq"));
    ChoiceOutputFormat->Append(_("Lynx Conductor, *.seq"));
    ChoiceOutputFormat->Append(_("Vix,Vixen 2.1 *.vix sequence file"));
    ChoiceOutputFormat->Append(_("Vir, Vixen 2.1 *.vir routine file"));
    ChoiceOutputFormat->Append(_("LOR Sequence *.las or *.lms"));
    ChoiceOutputFormat->Append(_("Lcb, LOR clipboard *.lcb"));
    ChoiceOutputFormat->Append(_("HLS *.hlsnc"));
    FlexGridSizer26->Add(ChoiceOutputFormat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText20 = new wxStaticText(PanelConvert, ID_STATICTEXT20, _("Save channel names:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    StaticText20->Hide();
    FlexGridSizer26->Add(StaticText20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxSaveChannelNames = new wxCheckBox(PanelConvert, ID_CHECKBOX_SAVE_CHANNEL_NAMES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_SAVE_CHANNEL_NAMES"));
    CheckBoxSaveChannelNames->SetValue(false);
    CheckBoxSaveChannelNames->Hide();
    FlexGridSizer26->Add(CheckBoxSaveChannelNames, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(PanelConvert, ID_STATICTEXT17, _("All channels off at end:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    FlexGridSizer26->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxOffAtEnd = new wxCheckBox(PanelConvert, ID_CHECKBOX_OFF_AT_END, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OFF_AT_END"));
    CheckBoxOffAtEnd->SetValue(false);
    FlexGridSizer26->Add(CheckBoxOffAtEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(FlexGridSizer26, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStartConversion = new wxButton(PanelConvert, ID_BUTTON_START_CONVERSION, _("Start Conversion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_START_CONVERSION"));
    FlexGridSizer6->Add(ButtonStartConversion, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer25->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer23 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer23->AddGrowableCol(0);
    FlexGridSizer23->AddGrowableRow(1);
    StaticText18 = new wxStaticText(PanelConvert, ID_STATICTEXT18, _("Conversion Messages:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    FlexGridSizer23->Add(StaticText18, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlConversionStatus = new wxTextCtrl(PanelConvert, ID_TEXTCTRL_CONVERSION_STATUS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_CONVERSION_STATUS"));
    TextCtrlConversionStatus->SetMinSize(wxSize(340,-1));
    FlexGridSizer23->Add(TextCtrlConversionStatus, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer25->Add(FlexGridSizer23, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerConvert->Add(FlexGridSizer25, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    PanelConvert->SetSizer(FlexGridSizerConvert);
    FlexGridSizerConvert->Fit(PanelConvert);
    FlexGridSizerConvert->SetSizeHints(PanelConvert);
    PanelPreview = new wxPanel(Notebook1, ID_PANEL_PREVIEW, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_PREVIEW"));
    FlexGridSizer3 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(2);
    FlexGridSizer36 = new wxFlexGridSizer(1, 3, 0, 0);
    FlexGridSizer36->AddGrowableCol(2);
    ButtonPreviewOpen = new wxButton(PanelPreview, ID_BUTTON_PREVIEW_OPEN, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PREVIEW_OPEN"));
    FlexGridSizer36->Add(ButtonPreviewOpen, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(PanelPreview, wxID_ANY, _("Now playing:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer36->Add(StaticText21, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextPreviewFileName = new wxStaticText(PanelPreview, ID_STATICTEXT23, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer36->Add(StaticTextPreviewFileName, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer36, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer31 = new wxFlexGridSizer(1, 5, 0, 0);
    FlexGridSizer31->AddGrowableCol(4);
    ButtonPlayPreview = new wxButton(PanelPreview, ID_BUTTON_PLAY_PREVIEW, _("Play"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PLAY_PREVIEW"));
    FlexGridSizer31->Add(ButtonPlayPreview, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStopPreview = new wxButton(PanelPreview, ID_BUTTON_STOP_PREVIEW, _("Pause"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_STOP_PREVIEW"));
    FlexGridSizer31->Add(ButtonStopPreview, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonRepeatPreview = new wxButton(PanelPreview, ID_BUTTON_REPEAT_PREVIEW, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_REPEAT_PREVIEW"));
    FlexGridSizer31->Add(ButtonRepeatPreview, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlPreviewTime = new wxTextCtrl(PanelPreview, ID_TEXTCTRL_PREVIEW_TIME, wxEmptyString, wxDefaultPosition, wxSize(58,21), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_TIME"));
    FlexGridSizer31->Add(TextCtrlPreviewTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    SliderPreviewTime = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_TIME, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_TIME"));
    FlexGridSizer31->Add(SliderPreviewTime, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer31, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer34->AddGrowableCol(1);
    FlexGridSizer34->AddGrowableRow(0);
    FlexGridSizer35 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText5 = new wxStaticText(PanelPreview, ID_STATICTEXT21, _("Display Elements"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer35->Add(StaticText5, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ListBoxElementList = new wxListBox(PanelPreview, ID_LISTBOX_ELEMENT_LIST, wxDefaultPosition, wxSize(132,102), 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_ELEMENT_LIST"));
    FlexGridSizer35->Add(ListBoxElementList, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonModelsPreview = new wxButton(PanelPreview, ID_BUTTON_MODELS_PREVIEW, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MODELS_PREVIEW"));
    FlexGridSizer35->Add(ButtonModelsPreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSavePreview = new wxButton(PanelPreview, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
    FlexGridSizer35->Add(ButtonSavePreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText23 = new wxStaticText(PanelPreview, ID_STATICTEXT22, _("Element Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer35->Add(StaticText23, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderPreviewScale = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_SCALE, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_SCALE"));
    FlexGridSizer35->Add(SliderPreviewScale, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextPreviewRotation = new wxStaticText(PanelPreview, ID_STATICTEXT25, _("Single String Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    StaticTextPreviewRotation->Disable();
    FlexGridSizer35->Add(StaticTextPreviewRotation, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderPreviewRotate = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_ROTATE, 0, -30, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_ROTATE"));
    SliderPreviewRotate->Disable();
    FlexGridSizer35->Add(SliderPreviewRotate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34->Add(FlexGridSizer35, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    ScrolledWindowPreview = new wxScrolledWindow(PanelPreview, ID_SCROLLEDWINDOW_PREVIEW, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW_PREVIEW"));
    ScrolledWindowPreview->SetBackgroundColour(wxColour(0,0,0));
    FlexGridSizer34->Add(ScrolledWindowPreview, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(FlexGridSizer34, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelPreview->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(PanelPreview);
    FlexGridSizer3->SetSizeHints(PanelPreview);
    PanelSequence2 = new wxPanel(Notebook1, ID_PANEL30, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL30"));
    FlexGridSizer70 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer70->AddGrowableCol(0);
    FlexGridSizer70->AddGrowableRow(0);
    SplitterWindow2 = new wxSplitterWindow(PanelSequence2, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH|wxSP_NOBORDER|wxSP_NO_XP_THEME, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetMinSize(wxSize(10,10));
    SplitterWindow2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    SplitterWindow2->SetMinimumPaneSize(10);
    SplitterWindow2->SetSashGravity(0.5);
    SeqPanelLeft = new wxPanel(SplitterWindow2, ID_PANEL31, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL31"));
    BoxSizer6 = new wxBoxSizer(wxVERTICAL);
    FlexGridSizer30 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer30->AddGrowableCol(0);
    FlexGridSizer30->AddGrowableRow(0);
    ScrolledWindow1 = new wxScrolledWindow(SeqPanelLeft, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxSize(276,225), wxSIMPLE_BORDER|wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
    ScrolledWindow1->SetBackgroundColour(wxColour(0,0,0));
    FlexGridSizer30->Add(ScrolledWindow1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 0);
    BoxSizer6->Add(FlexGridSizer30, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, SeqPanelLeft, _("Combined Effect"));
    FlexGridSizer33 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_PlayEffect = new wxButton(SeqPanelLeft, ID_BUTTON13, _("Play Effect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    Button_PlayEffect->Disable();
    Button_PlayEffect->SetBackgroundColour(wxColour(0,255,0));
    FlexGridSizer33->Add(Button_PlayEffect, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_UpdateGrid = new wxButton(SeqPanelLeft, ID_BUTTON3, _("Update Grid"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Button_UpdateGrid->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_UpdateGrid, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Models = new wxButton(SeqPanelLeft, ID_BUTTON58, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON58"));
    Button_Models->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Models = new wxChoice(SeqPanelLeft, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE7"));
    FlexGridSizer33->Add(Choice_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Presets = new wxButton(SeqPanelLeft, ID_BUTTON59, _("Effect Presets"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON59"));
    Button_Presets->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Presets, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette = new wxButton(SeqPanelLeft, ID_BUTTON_Palette, _("Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette"));
    Button_Palette->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Palette, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_LayerMethod = new wxChoice(SeqPanelLeft, ID_CHOICE_LayerMethod, wxDefaultPosition, wxDefaultSize, 0, 0, wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator, _T("ID_CHOICE_LayerMethod"));
    Choice_LayerMethod->SetSelection( Choice_LayerMethod->Append(_("Effect 1")) );
    Choice_LayerMethod->Append(_("Effect 2"));
    Choice_LayerMethod->Append(_("1 is Mask"));
    Choice_LayerMethod->Append(_("2 is Mask"));
    Choice_LayerMethod->Append(_("1 is Unmask"));
    Choice_LayerMethod->Append(_("2 is Unmask"));
    Choice_LayerMethod->Append(_("Layered"));
    Choice_LayerMethod->Append(_("Average"));
    FlexGridSizer33->Add(Choice_LayerMethod, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_EffectLayerMix = new wxSlider(SeqPanelLeft, ID_SLIDER_EffectLayerMix, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectLayerMix"));
    FlexGridSizer33->Add(Slider_EffectLayerMix, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txtCtlEffectMix = new wxTextCtrl(SeqPanelLeft, ID_TEXTCTRL_LayerMix, _("0"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_LayerMix"));
    FlexGridSizer33->Add(txtCtlEffectMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(SeqPanelLeft, ID_STATICTEXT24, _("Sparkles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer33->Add(StaticText22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_SparkleFrequency = new wxSlider(SeqPanelLeft, ID_SLIDER_SparkleFrequency, 200, 10, 200, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_SparkleFrequency"));
    FlexGridSizer33->Add(Slider_SparkleFrequency, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txtCtrlSparkleFreq = new wxTextCtrl(SeqPanelLeft, ID_TEXTCTRL5, _("200"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL5"));
    FlexGridSizer33->Add(txtCtrlSparkleFreq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText126 = new wxStaticText(SeqPanelLeft, ID_STATICTEXT127, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT127"));
    FlexGridSizer33->Add(StaticText126, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Brightness = new wxSlider(SeqPanelLeft, ID_SLIDER_Brightness, 100, 0, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Brightness"));
    FlexGridSizer33->Add(Slider_Brightness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txtCtlBrightness = new wxTextCtrl(SeqPanelLeft, ID_TEXTCTRL6, _("100"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL6"));
    FlexGridSizer33->Add(txtCtlBrightness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText127 = new wxStaticText(SeqPanelLeft, ID_STATICTEXT128, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT128"));
    FlexGridSizer33->Add(StaticText127, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Contrast = new wxSlider(SeqPanelLeft, ID_SLIDER_Contrast, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Contrast"));
    FlexGridSizer33->Add(Slider_Contrast, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    txtCtlContrast = new wxTextCtrl(SeqPanelLeft, ID_TEXTCTRL7, _("0"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL7"));
    FlexGridSizer33->Add(txtCtlContrast, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer33, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer6->Add(StaticBoxSizer3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SeqPanelLeft->SetSizer(BoxSizer6);
    BoxSizer6->Fit(SeqPanelLeft);
    BoxSizer6->SetSizeHints(SeqPanelLeft);
    SeqPanelRight = new wxPanel(SplitterWindow2, ID_PANEL32, wxPoint(40,-11), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL32"));
    BoxSizer4 = new wxBoxSizer(wxVERTICAL);
    StaticBoxSizerSequenceButtons = new wxStaticBoxSizer(wxVERTICAL, SeqPanelRight, _("RGB Sequence"));
    FlexGridSizer32 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer32->AddGrowableCol(0);
    FlexGridSizer32->AddGrowableRow(2);
    StaticTextSequenceFileName = new wxStaticText(SeqPanelRight, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer32->Add(StaticTextSequenceFileName, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer68 = new wxFlexGridSizer(0, 10, 0, 0);
    Button_PlayRgbSeq = new wxButton(SeqPanelRight, ID_BUTTON_PLAY_RGB_SEQ, _("Play"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PLAY_RGB_SEQ"));
    Button_PlayRgbSeq->SetBackgroundColour(wxColour(0,255,0));
    Button_PlayRgbSeq->SetToolTip(_("Play from current grid cell"));
    FlexGridSizer68->Add(Button_PlayRgbSeq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonDisplayElements = new wxButton(SeqPanelRight, ID_BUTTON2, _("Display Elements"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    ButtonDisplayElements->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(ButtonDisplayElements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ChannelMap = new wxButton(SeqPanelRight, ID_BUTTON1, _("Channel Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button_ChannelMap->Disable();
    Button_ChannelMap->Hide();
    Button_ChannelMap->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(Button_ChannelMap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    BitmapButtonOpenSeq = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON7, open_xpm, wxDefaultPosition, wxSize(48,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    BitmapButtonOpenSeq->SetDefault();
    BitmapButtonOpenSeq->SetToolTip(_("Open Sequence"));
    FlexGridSizer68->Add(BitmapButtonOpenSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonSaveSeq = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON9, save_xpm, wxDefaultPosition, wxSize(53,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    BitmapButtonSaveSeq->SetDefault();
    BitmapButtonSaveSeq->SetToolTip(_("Save Sequence"));
    FlexGridSizer68->Add(BitmapButtonSaveSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonInsertRow = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON3, insertrow_xpm, wxDefaultPosition, wxSize(50,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    BitmapButtonInsertRow->SetDefault();
    BitmapButtonInsertRow->SetToolTip(_("Insert Row"));
    FlexGridSizer68->Add(BitmapButtonInsertRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonDeleteRow = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON4, deleterow_xpm, wxDefaultPosition, wxSize(49,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    BitmapButtonDeleteRow->SetDefault();
    BitmapButtonDeleteRow->SetToolTip(_("Delete Row"));
    FlexGridSizer68->Add(BitmapButtonDeleteRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSeqExport = new wxButton(SeqPanelRight, ID_BUTTON_SeqExport, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SeqExport"));
    ButtonSeqExport->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(ButtonSeqExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_CreateRandom = new wxButton(SeqPanelRight, ID_BUTTON_CREATE_RANDOM, _("Create Random Effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CREATE_RANDOM"));
    Button_CreateRandom->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(Button_CreateRandom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer32->Add(FlexGridSizer68, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Grid1 = new wxGrid(SeqPanelRight, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxFULL_REPAINT_ON_RESIZE, _T("ID_GRID1"));
    Grid1->CreateGrid(0,2);
    Grid1->EnableEditing(true);
    Grid1->EnableGridLines(true);
    Grid1->SetRowLabelSize(35);
    Grid1->SetColLabelValue(0, _("Start Time"));
    Grid1->SetColLabelValue(1, _("Label"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizer32->Add(Grid1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    StaticBoxSizerSequenceButtons->Add(FlexGridSizer32, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer4->Add(StaticBoxSizerSequenceButtons, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxVERTICAL, SeqPanelRight, _("Effect 1"));
    EffectsPanel1 = new EffectsPanel(SeqPanelRight, ID_PANEL_EFFECTS1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1"));
    StaticBoxSizer4->Add(EffectsPanel1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer5->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, SeqPanelRight, _("Effect 2"));
    EffectsPanel2 = new EffectsPanel(SeqPanelRight, ID_PANEL_EFFECTS2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS2"));
    StaticBoxSizer5->Add(EffectsPanel2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer5->Add(StaticBoxSizer5, 1, wxALL|wxFIXED_MINSIZE|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    BoxSizer4->Add(BoxSizer5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SeqPanelRight->SetSizer(BoxSizer4);
    BoxSizer4->Fit(SeqPanelRight);
    BoxSizer4->SetSizeHints(SeqPanelRight);
    SplitterWindow2->SplitVertically(SeqPanelLeft, SeqPanelRight);
    FlexGridSizer70->Add(SplitterWindow2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelSequence2->SetSizer(FlexGridSizer70);
    FlexGridSizer70->Fit(PanelSequence2);
    FlexGridSizer70->SetSizeHints(PanelSequence2);
    PanelCal = new wxPanel(Notebook1, ID_PANEL_CAL, wxPoint(49,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CAL"));
    PanelCal->Hide();
    FlexGridSizer8 = new wxFlexGridSizer(2, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    FlexGridSizer8->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(PanelCal, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH|wxSP_NOBORDER|wxSP_NO_XP_THEME, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinSize(wxSize(10,10));
    SplitterWindow1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    SplitterWindow1->SetMinimumPaneSize(10);
    SplitterWindow1->SetSashGravity(0.5);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    Panel3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    ListBoxSched = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_NO_LINES|wxTR_HIDE_ROOT|wxTR_MULTIPLE|wxTR_EXTENDED|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
    FlexGridSizer4->Add(ListBoxSched, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
    FlexGridSizer27 = new wxFlexGridSizer(0, 1, 0, 0);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    CheckBoxRunSchedule = new wxCheckBox(Panel3, ID_CHECKBOX_RUN_SCHEDULE, _("Run Schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_RUN_SCHEDULE"));
    CheckBoxRunSchedule->SetValue(false);
    BoxSizer2->Add(CheckBoxRunSchedule, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveSchedule = new wxButton(Panel3, ID_BUTTON_SAVE_SCHEDULE, _("Save Schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_SCHEDULE"));
    ButtonSaveSchedule->SetBackgroundColour(wxColour(224,224,224));
    BoxSizer2->Add(ButtonSaveSchedule, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonAddShow = new wxButton(Panel3, ID_BUTTON_ADD_SHOW, _("Schedule Playlist"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_SHOW"));
    ButtonAddShow->SetBackgroundColour(wxColour(224,224,224));
    BoxSizer2->Add(ButtonAddShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonUpdateShow = new wxButton(Panel3, ID_BUTTON_UPDATE_SHOW, _("Update Selected Items"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_UPDATE_SHOW"));
    ButtonUpdateShow->SetBackgroundColour(wxColour(224,224,224));
    BoxSizer2->Add(ButtonUpdateShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonDeleteShow = new wxButton(Panel3, ID_BUTTON_DELETE_SHOW, _("Delete Selected Items"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_SHOW"));
    ButtonDeleteShow->SetBackgroundColour(wxColour(224,224,224));
    BoxSizer2->Add(ButtonDeleteShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer27->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText2 = new wxStaticText(Panel3, ID_STATICTEXT2, _("Show Dates"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    wxFont StaticText2Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText2->SetFont(StaticText2Font);
    FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonShowDatesChange = new wxButton(Panel3, ID_BUTTON_SHOW_DATES_CHANGE, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHOW_DATES_CHANGE"));
    ButtonShowDatesChange->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer7->Add(ButtonShowDatesChange, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(Panel3, ID_STATICTEXT3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShowStart = new wxStaticText(Panel3, ID_STATICTEXT_SHOWSTART, _("xx/xx/xxxx"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SHOWSTART"));
    FlexGridSizer7->Add(StaticTextShowStart, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(Panel3, ID_STATICTEXT5, _("End"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer7->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextShowEnd = new wxStaticText(Panel3, ID_STATICTEXT_SHOWEND, _("xx/xx/xxxx"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SHOWEND"));
    FlexGridSizer7->Add(StaticTextShowEnd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer27->Add(FlexGridSizer7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer27, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    Panel3->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel3);
    FlexGridSizer4->SetSizeHints(Panel3);
    Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxPoint(13,93), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Panel2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    FlexGridSizer28 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer28->AddGrowableCol(0);
    FlexGridSizer28->AddGrowableRow(1);
    StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("While the scheduler is running, each item that is played is logged here"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer28->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer29 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer29->AddGrowableCol(0);
    FlexGridSizer29->AddGrowableRow(0);
    TextCtrlLog = new wxTextCtrl(Panel2, ID_TEXTCTRL_LOG, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL, wxDefaultValidator, _T("ID_TEXTCTRL_LOG"));
    FlexGridSizer29->Add(TextCtrlLog, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    ButtonClearLog = new wxButton(Panel2, ID_BUTTON_CLEARLOG, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLEARLOG"));
    ButtonClearLog->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer5->Add(ButtonClearLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveLog = new wxButton(Panel2, ID_BUTTON_SAVELOG, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVELOG"));
    ButtonSaveLog->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer5->Add(ButtonSaveLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer29->Add(FlexGridSizer5, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer28->Add(FlexGridSizer29, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer28);
    FlexGridSizer28->Fit(Panel2);
    FlexGridSizer28->SetSizeHints(Panel2);
    SplitterWindow1->SplitVertically(Panel3, Panel2);
    FlexGridSizer8->Add(SplitterWindow1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelCal->SetSizer(FlexGridSizer8);
    FlexGridSizer8->Fit(PanelCal);
    FlexGridSizer8->SetSizeHints(PanelCal);
    Notebook1->AddPage(PanelSetup, _("Setup"), true);
    Notebook1->AddPage(PanelTest, _("Test"), false);
    Notebook1->AddPage(PanelConvert, _("Convert"), false);
    Notebook1->AddPage(PanelPreview, _("Preview"), false);
    Notebook1->AddPage(PanelSequence2, _("Nutcracker"), false);
    Notebook1->AddPage(PanelCal, _("Schedule"), false);
    FlexGridSizer2->Add(Notebook1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel1);
    FlexGridSizer2->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItem5 = new wxMenuItem(MenuFile, ID_MENUITEM2, _("Select Show Folder"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem5);
    MenuItemBackup = new wxMenuItem(MenuFile, ID_FILE_BACKUP, _("Backup"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemBackup);
    MenuItem1 = new wxMenuItem(MenuFile, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    MenuFile->Append(MenuItem1);
    MenuBar1->Append(MenuFile, _("&File"));
    MenuPlaylist = new wxMenu();
    MenuItemSavePlaylists = new wxMenuItem(MenuPlaylist, idMenuSaveSched, _("Save Playlists"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemSavePlaylists);
    MenuItemAddList = new wxMenuItem(MenuPlaylist, idMenuAddList, _("Add"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemAddList);
    MenuItemRenameList = new wxMenuItem(MenuPlaylist, idMenuRenameList, _("Rename"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemRenameList);
    MenuItemDelList = new wxMenuItem(MenuPlaylist, idMenuDelList, _("Delete"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemDelList);
    MenuItemRefresh = new wxMenuItem(MenuPlaylist, ID_MENUITEM1, _("Refresh"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemRefresh);
    MenuItemCustomScript = new wxMenuItem(MenuPlaylist, idCustomScript, _("Custom Script"), wxEmptyString, wxITEM_NORMAL);
    MenuPlaylist->Append(MenuItemCustomScript);
    MenuBar1->Append(MenuPlaylist, _("&Playlist"));
    MenuHelp = new wxMenu();
    MenuItem4 = new wxMenuItem(MenuHelp, idMenuHelpContent, _("Content\tF1"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem4);
    MenuItem2 = new wxMenuItem(MenuHelp, idMenuAbout, _("About"), _("Show info about this application"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem2);
    MenuBar1->Append(MenuHelp, _("&Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -1, -1 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    DirDialog1 = new wxDirDialog(this, _("Select directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    Timer1.SetOwner(this, ID_TIMER1);
    FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|Vixen Sequences (*.vix)|*.vix|xLights Sequences(*.xseq)|*.xseq|Falcon Pi Player Sequences (*.fseq)|*.fseq|Lynx Conductor Sequences (*.seq)|*.seq|HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    MessageDialog1 = new wxMessageDialog(this, _("Hello"), _("Message"), wxOK|wxCANCEL, wxDefaultPosition);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BITMAPBUTTON_TAB_INFO,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(ID_BUTTON_STOP_NOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStopNowClick);
    Connect(ID_BUTTON_GRACEFUL_STOP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonGracefulStopClick);
    Connect(ID_BUTTON_LIGHTS_OFF,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonLightsOffClick);
    Connect(ID_CHECKBOX_LIGHT_OUTPUT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBoxLightOutputClick);
    Connect(ID_BUTTON_CHANGEDIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_BUTTON_SAVE_SETUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveSetupClick);
    Connect(ID_BUTTON_ADD_DONGLE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddDongleClick);
    Connect(ID_BUTTON_ADD_E131,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddE131Click);
    Connect(ID_BUTTON_NETWORK_CHANGE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkChangeClick);
    Connect(ID_BUTTON_NETWORK_DELETE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkDeleteClick);
    Connect(ID_BUTTON_NETWORK_DELETE_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkDeleteAllClick);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveUpClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveDownClick);
    Connect(ID_LISTCTRL_NETWORKS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&xLightsFrame::OnGridNetworkBeginDrag);
    Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonTestSelectAllClick);
    Connect(ID_BUTTON_CLEAR_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonTestClearClick);
    Connect(ID_BUTTON_LOAD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonTestLoadClick);
    Connect(ID_BUTTON_SAVE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonTestSaveClick);
    Connect(ID_CHECKLISTBOX_TEST_CHANNELS,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&xLightsFrame::OnCheckListBoxTestChannelsToggled);
    Connect(ID_RADIOBUTTON14,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonOffSelect);
    Connect(ID_RADIOBUTTON15,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChaseSelect);
    Connect(ID_RADIOBUTTON16,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase3Select);
    Connect(ID_RADIOBUTTON17,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase4Select);
    Connect(ID_RADIOBUTTON19,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase5Select);
    Connect(ID_RADIOBUTTON18,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonAltSelect);
    Connect(ID_RADIOBUTTON7,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle05Select);
    Connect(ID_RADIOBUTTON31,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle10Select);
    Connect(ID_RADIOBUTTON30,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle25Select);
    Connect(ID_RADIOBUTTON29,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle50Select);
    Connect(ID_RADIOBUTTON8,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonShimmerSelect);
    Connect(ID_RADIOBUTTON6,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonDimSelect);
    Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonOffSelect);
    Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChaseSelect);
    Connect(ID_RADIOBUTTON5,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase3Select);
    Connect(ID_RADIOBUTTON9,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase4Select);
    Connect(ID_RADIOBUTTON25,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase5Select);
    Connect(ID_RADIOBUTTON10,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonAltSelect);
    Connect(ID_RADIOBUTTON12,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle05Select);
    Connect(ID_RADIOBUTTON28,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle10Select);
    Connect(ID_RADIOBUTTON11,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle25Select);
    Connect(ID_RADIOBUTTON13,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonTwinkle50Select);
    Connect(ID_RADIOBUTTON20,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonShimmerSelect);
    Connect(ID_RADIOBUTTON21,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonDimSelect);
    Connect(ID_RADIOBUTTON22,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonOffSelect);
    Connect(ID_RADIOBUTTON23,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase3Select);
    Connect(ID_RADIOBUTTON24,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase4Select);
    Connect(ID_RADIOBUTTON26,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonChase5Select);
    Connect(ID_RADIOBUTTON27,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnRadioButtonDimSelect);
    Connect(ID_BUTTON_CHOOSE_FILE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonChooseFileClick);
    Connect(ID_BUTTON_START_CONVERSION,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStartConversionClick);
    Connect(ID_BUTTON_PREVIEW_OPEN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonPreviewOpenClick);
    Connect(ID_BUTTON_PLAY_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonPlayPreviewClick);
    Connect(ID_BUTTON_STOP_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStopPreviewClick);
    Connect(ID_BUTTON_REPEAT_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonRepeatPreviewClick);
    Connect(ID_SLIDER_PREVIEW_TIME,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewTimeCmdSliderUpdated);
    Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnListBoxElementListSelect);
    Connect(ID_BUTTON_MODELS_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonModelsPreviewClick);
    Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSavePreviewClick);
    Connect(ID_SLIDER_PREVIEW_SCALE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated);
    Connect(ID_SLIDER_PREVIEW_ROTATE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewRotateCmdSliderUpdated);
    ScrolledWindowPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftDown,0,this);
    ScrolledWindowPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftUp,0,this);
    ScrolledWindowPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseMove,0,this);
    ScrolledWindowPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseLeave,0,this);
    ScrolledWindowPreview->Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewResize,0,this);
    ScrolledWindow1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindow1Resize,0,this);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayEffectClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_UpdateGridClick);
    Connect(ID_BUTTON58,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ModelsClick);
    Connect(ID_BUTTON59,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PresetsClick);
    Connect(ID_BUTTON_Palette,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PaletteClick);
    Connect(ID_CHOICE_LayerMethod,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_LayerMethodSelect);
    Connect(ID_SLIDER_EffectLayerMix,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnSlider_EffectLayerMixCmdScroll);
    Connect(ID_SLIDER_EffectLayerMix,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_EffectLayerMixCmdScroll);
    Connect(ID_SLIDER_SparkleFrequency,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnSlider_SparkleFrequencyCmdScroll);
    Connect(ID_SLIDER_SparkleFrequency,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_SparkleFrequencyCmdScroll);
    Connect(ID_SLIDER_Brightness,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnSlider_BrightnessCmdScroll);
    Connect(ID_SLIDER_Brightness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_BrightnessCmdScroll);
    Connect(ID_SLIDER_Contrast,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnSlider_ContrastCmdScroll);
    Connect(ID_SLIDER_Contrast,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_ContrastCmdScroll);
    Connect(ID_BUTTON_PLAY_RGB_SEQ,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayAllClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDisplayElementsClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ChannelMapClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_BITMAPBUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonInsertRowClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonDeleteRowClick);
    Connect(ID_BUTTON_SeqExport,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSeqExportClick);
    Connect(ID_BUTTON_CREATE_RANDOM,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnbtRandomEffectClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellRightClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellChange);
    Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellLeftClick);
    Connect(ID_CHECKBOX_RUN_SCHEDULE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBoxRunScheduleClick);
    Connect(ID_BUTTON_SAVE_SCHEDULE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveScheduleClick);
    Connect(ID_BUTTON_ADD_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddShowClick);
    Connect(ID_BUTTON_UPDATE_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonUpdateShowClick);
    Connect(ID_BUTTON_DELETE_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDeleteShowClick);
    Connect(ID_BUTTON_SHOW_DATES_CHANGE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonShowDatesChangeClick);
    Connect(ID_BUTTON_CLEARLOG,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonClearLogClick);
    Connect(ID_BUTTON_SAVELOG,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveLogClick);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_FILE_BACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBackupSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnQuit);
    Connect(idMenuSaveSched,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemSavePlaylistsSelected);
    Connect(idMenuAddList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemAddListSelected);
    Connect(idMenuRenameList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenameListSelected);
    Connect(idMenuDelList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemDelListSelected);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRefreshSelected);
    Connect(idCustomScript,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemCustomScriptSelected);
    Connect(idMenuHelpContent,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnTimer1Trigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnClose);
    //*)



    SetIcon(wxIcon(xlights_xpm));
    wxStandardPathsBase& stdp = wxStandardPaths::Get();
    wxConfig* config = new wxConfig(_(XLIGHTS_CONFIG_ID));

    effGridPrevX = 0;
    effGridPrevY = 0;

    // Load headings into network list
    wxListItem itemCol;
    itemCol.SetText(_T("Network Type"));
    itemCol.SetImage(-1);
    GridNetwork->InsertColumn(0, itemCol);

    itemCol.SetText(_T("Port"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(1, itemCol);

    itemCol.SetText(_T("Baud Rate or E1.31 Univ"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    GridNetwork->InsertColumn(2, itemCol);

    itemCol.SetText(_T("Last Channel"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    GridNetwork->InsertColumn(3, itemCol);

    itemCol.SetText(_T("LOR Mapping"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(4, itemCol);

    itemCol.SetText(_T("xLights/Vixen Mapping"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(5, itemCol);

    GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(1,100);
    GridNetwork->SetColumnWidth(2,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(3,100);
    GridNetwork->SetColumnWidth(4,150);
    GridNetwork->SetColumnWidth(5,150);

    Grid1->SetColFormatFloat(0,7,3);

    // get list of most recently used directories
    wxString dir,mru_name;
    int menuID, idx;
    for (int i=0; i<MRU_LENGTH; i++)
    {
        mru_name=wxString::Format(wxT("mru%d"),i);
        dir.clear();
        if ( config->Read(mru_name, &dir) )
        {
            if (!dir.IsEmpty())
            {
                idx=mru.Index(dir);
                if (idx == wxNOT_FOUND) mru.Add(dir);
            }
        }
        menuID = wxNewId();
        mru_MenuItem[i] = new wxMenuItem(MenuFile, menuID, mru_name);
        Connect(menuID,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
    }
    dir.clear();
    bool ok = config->Read(_("LastDir"), &dir);
    wxString ConvertDir;
    if (ok && !config->Read(_("ConvertDir"), &ConvertDir))
    {
        ConvertDir=dir;
    }
    FileDialogConvert->SetDirectory(ConvertDir);

    // initialize all effect wxChoice lists

    BarEffectDirections.Add("up");
    BarEffectDirections.Add("down");
    BarEffectDirections.Add("expand");
    BarEffectDirections.Add("compress");
    BarEffectDirections.Add("Left");
    BarEffectDirections.Add("Right");
    BarEffectDirections.Add("H-expand");
    BarEffectDirections.Add("H-compress");

    ButterflyEffectColors.Add("Rainbow");
    ButterflyEffectColors.Add("Palette");

    MeteorsEffectTypes.Add("Rainbow");
    MeteorsEffectTypes.Add("Range");
    MeteorsEffectTypes.Add("Palette");

    MeteorsEffect.Add("Down");
    MeteorsEffect.Add("Up");
    MeteorsEffect.Add("Left");
    MeteorsEffect.Add("Right");
    MeteorsEffect.Add("Implode");
    MeteorsEffect.Add("Explode");

    EffectDirections.Add("left");
    EffectDirections.Add("right");
    EffectDirections.Add("up");
    EffectDirections.Add("down");
    EffectDirections.Add("none");
    EffectDirections.Add("up-left");
    EffectDirections.Add("down-left");
    EffectDirections.Add("up-right");
    EffectDirections.Add("down-right");

    TextEffects.Add("normal");
    TextEffects.Add("vertical text up");
    TextEffects.Add("vertical dext down");
    TextEffects.Add("rotate up 45");
    TextEffects.Add("rotate up 90");
    TextEffects.Add("rotate down 45");
    TextEffects.Add("rotate down 90");

    TextCountDown.Add("none");
    TextCountDown.Add("count down seconds");
    TextCountDown.Add("count down to date");

    CurtainEdge.Add("left");
    CurtainEdge.Add("center");
    CurtainEdge.Add("right");

    CurtainEffect.Add("open");
    CurtainEffect.Add("close");
    CurtainEffect.Add("open then close");
    CurtainEffect.Add("close then open");

    InitEffectsPanel(EffectsPanel1);
    InitEffectsPanel(EffectsPanel2);

    // Check if schedule should be running
    xout=0;
    long RunFlag=0;
    config->Read(_("RunSchedule"), &RunFlag);
    delete config;  // close config before calling SetDir, which will open config

    SetPlayMode(play_off);
    ResetEffectsXml();
    EnableSequenceControls(true);
    UpdateShowDates(wxDateTime::Now(),wxDateTime::Now());
    if (ok && !dir.IsEmpty())
    {
        SetDir(dir);
    }
    EffectsPanel1->PaletteChanged=true;
    EffectsPanel2->PaletteChanged=true;
    MixTypeChanged=true;
    HtmlEasyPrint=new wxHtmlEasyPrinting(wxT("xLights Printing"), this);
    basic.setFrame(this);
    PlayerDlg = new PlayerFrame(this, ID_PLAYER_DIALOG);

    EffectNames=EffectsPanel1->Choicebook1->GetChoiceCtrl()->GetStrings();
    EffectLayerOptions=Choice_LayerMethod->GetStrings();

    if (RunFlag && !ShowEvents.IsEmpty())
    {
        // open ports
        Notebook1->ChangeSelection(SCHEDULETAB);
        CheckBoxLightOutput->SetValue(true);
        CheckBoxRunSchedule->SetValue(true);
        EnableOutputs();
        CheckRunSchedule();
    }
    else
    {
        Notebook1->ChangeSelection(SETUPTAB);
        EnableNetworkChanges();
    }
    curCell = new wxGridCellCoords(0,0);
    wxImage::AddHandler(new wxGIFHandler);
    Timer1.Start(XTIMER_INTERVAL, wxTIMER_CONTINUOUS);
    EffectTreeDlg = NULL;
}

xLightsFrame::~xLightsFrame()
{
    //(*Destroy(xLightsFrame)
    //*)
}

void xLightsFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xLightsFrame::InitEffectsPanel(EffectsPanel* panel)
{
    panel->Choice_Bars_Direction->Set(BarEffectDirections);
    panel->Choice_Bars_Direction->SetSelection(0);
    panel->Choice_Butterfly_Colors->Set(ButterflyEffectColors);
    panel->Choice_Butterfly_Colors->SetSelection(0);
    panel->Choice_Meteors_Type->Set(MeteorsEffectTypes);
    panel->Choice_Meteors_Type->SetSelection(0);
    panel->Choice_Meteors_Effect->Set(MeteorsEffect);
    panel->Choice_Meteors_Effect->SetSelection(0);
    panel->Choice_Pictures_Direction->Set(EffectDirections);
    panel->Choice_Pictures_Direction->SetSelection(0);
    panel->Choice_Text_Dir1->Set(EffectDirections);
    panel->Choice_Text_Dir1->SetSelection(0);
    panel->Choice_Text_Dir2->Set(EffectDirections);
    panel->Choice_Text_Dir2->SetSelection(0);
    panel->Choice_Text_Effect1->Set(TextEffects);
    panel->Choice_Text_Effect1->SetSelection(0);
    panel->Choice_Text_Effect2->Set(TextEffects);
    panel->Choice_Text_Effect2->SetSelection(0);
    panel->Choice_Text_Count1->Set(TextCountDown);
    panel->Choice_Text_Count1->SetSelection(0);
    panel->Choice_Text_Count2->Set(TextCountDown);
    panel->Choice_Text_Count2->SetSelection(0);
    panel->Choice_Curtain_Edge->Set(CurtainEdge);
    panel->Choice_Curtain_Edge->SetSelection(0);
    panel->Choice_Curtain_Effect->Set(CurtainEffect);
    panel->Choice_Curtain_Effect->SetSelection(0);
    panel->CurrentDir = &CurrentDir;
}

void xLightsFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = _(XLIGHTS_LICENSE);
    wxString hdg = wxString::Format(_("About xLights %s"),_(XLIGHTS_VERSION));
    wxMessageBox(msg, hdg);
}

// return a random number between 0 and 1 inclusive
double xLightsFrame::rand01()
{
    return (double)rand()/(double)RAND_MAX;
}

void xLightsFrame::SetPlayMode(play_modes newmode)
{
    switch (newmode)
    {
    case play_off:
        StatusBar1->SetStatusText(_("Playback: off"));
        break;
    }

    ButtonGracefulStop->Enable(newmode == play_sched || newmode == play_list);
    ButtonChangeDir->Enable(newmode != play_sched && newmode != play_list && newmode != play_single);
    play_mode=newmode;
    starttime = wxDateTime::UNow();
#ifndef NDEBUG
    TextCtrlLog->AppendText(wxString::Format(_("SetPlayMode mode=%d state=%d\n"),play_mode,SeqPlayerState));
#endif
}

void xLightsFrame::OnTimer1Trigger(wxTimerEvent& event)
{
    if (!gs_xoutCriticalSection.TryEnter()) return;
    if (CheckBoxRunSchedule->IsChecked()) CheckSchedule();
    wxTimeSpan ts = wxDateTime::UNow() - starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    if (xout) xout->TimerStart(curtime);
    switch (Notebook1->GetSelection())
    {
    case TESTTAB:
        OnTimerTest(curtime);
        break;
    case SEQUENCETAB:
        TimerRgbSeq(curtime);
        break;
    case PREVIEWTAB:
        TimerPreview(curtime);
        break;
    default:
        OnTimerPlaylist(curtime);
        break;
    }
    if (xout) xout->TimerEnd();
    gs_xoutCriticalSection.Leave();
}

void xLightsFrame::ResetTimer(SeqPlayerStates newstate, long OffsetMsec)
{
    SeqPlayerState = newstate;
#ifndef NDEBUG
    TextCtrlLog->AppendText(wxString::Format(_("ResetTimer mode=%d state=%d\n"),play_mode,SeqPlayerState));
#endif
    //if (newstate == NO_SEQ) SetPlayMode(play_off);
    if (xout) xout->ResetTimer();
    wxTimeSpan offset(0,0,0,OffsetMsec);
    starttime = wxDateTime::UNow() - offset;
}

void xLightsFrame::OnBitmapButtonTabInfoClick(wxCommandEvent& event)
{
    wxString caption,msg,selectmethod;
#ifdef __WXOSX__
    selectmethod=_("cmd-click");
#else
    selectmethod=_("ctrl-click or shift-click");
#endif
    switch (Notebook1->GetSelection())
    {
    case SETUPTAB:
        caption=_("Setup Tab");
        msg=_("Show Directory\n\nThe first thing you need to know about xLights is that it expects you to organize all of your sequence files and associated audio or video files into a single directory. For example, you can have a directory called '2012 Show'. Once you have your show directory created and populated with the relevant files, you are ready to proceed. Tell xLights where your new show directory is by clicking the 'Change' button on the Setup tab, navigate to your show directory, then click 'OK'.\n\nLighting Networks\n\nThe next thing you will need to do is define your lighting network(s). xLights ignores most of the information about your lighting network contained in your LOR or Vixen sequence. Thus this step is very important! Add a row in the lower half of the Setup tab for each network used in your display. xLights can drive a mixture of network types (for example, the first network can be DMX, and the second one LOR, and the third one Renard). When you are finished, do not forget to SAVE YOUR CHANGES by clicking the 'Save Setup' button.");
        break;
    case TESTTAB:
        caption=_("Test Tab");
        msg=_("In order to use the xLights Test module, you must first define your lighting network(s) on the Setup tab. You should also click the 'Output to Lights' checkbox. Next you will need to select which channels you want to test. You can select channels one at a time by clicking on the checkbox next to each channel. You can select multiple channels at a time by using ")+selectmethod+_(", then clicking any checkbox in the selected range. Click the checkbox again to unselect all channels in the range. Once that is done, use the radio buttons and slide bars to control your testing. You can change channel selections while the test is running.");
        break;
    case CONVERTTAB:
        caption=_("Convert Tab");
        msg=_("Use the Convert tab to convert sequence files from one format to another. Available input formats include LOR (.lms or .las), Vixen (.vix), Lynx Conductor (.seq), and xLights (.xseq). Output formats include Vixen (.vix), Lynx Conductor (.seq), and xLights (.xseq). The xLights scheduler will only play xLights sequences and media files (music and video).\n\nThe lighting networks you define on the Setup tab will guide the conversion - make sure you have your lighting networks defined accurately before you start a conversion.\n\nAs the conversion takes place, messages will appear on the right side informing you about the progress and any errors found.");
        break;
    case SEQUENCETAB:
        caption=_("Sequence Tab");
        msg=_("The Sequence tab can be used to create RGB sequences. First, create a model of your RGB display element(s) by clicking on the Models button. Then try the different effects and settings until you create something you like. You can save the settings as a preset by clicking the New Preset button. From then on, that preset will be available in the presets drop-down list. You can combine effects by creating a second effect in the Effect 2 area, then choosing a Layering Method. To create a series of effects that will be used in a sequence, click the open file icon to open an xLights (.xseq) sequence. Choose which display elements/models you will use in this sequence. Then click the insert rows icon and type in the start time in seconds when that effect should begin. Rows will automatically sort by start time. To add an effect to the sequence, click on the grid cell in the desired display model column and the desired start time row, then click the Update button. When you are done creating effects for the sequence, click the save icon and the xLights sequence will be updated with the effects you stored in the grid.");
        break;
    case SCHEDULETAB:
        caption=_("Schedule Tab");
        msg=_("The xLights scheduler works by scheduling playlists. To create a playlist, select Playlist > Add from the menu. Once you have one or more playlists created, you are ready to build your schedule.\n\nStart building your schedule by defining your show start and end dates. Then click 'Schedule Playlist' to add playlists to the schedule. To alter the schedule, you may select multiple items using ")+selectmethod+_(". Don't worry if date headings get selected - they will be ignored. Then click 'Update Selected Items'.\n\nThe schedule is NOT SAVED until you click the 'Save Schedule' button (this also saves all playlists).\n\nThe schedule WILL NOT RUN unless the 'Run Schedule' checkbox is checked. You will also want to ensure 'Output to Lights' is checked.");
        break;
    default:
        // playlist
        caption=_("Playlist Tab");
        msg=_("Files in your show directory are listed on the left. Click the 'Audio', 'Video', or 'xLights' checkbox to see files of the desired type. Files in the playlist are listed on the right. To move files between the left and right lists, use the arrow buttons in the middle. You can rename or delete a list by using the Playlist menu. Selecting Playlist > Save Playlists from the menu will save all playlists as well as the schedule. You can play a single item on the playlist by selecting it and then clicking 'Play Item'. Use 'Play All' to play the entire playlist. You can reorder the playlist by dragging items up or down, or selecting items and using the up/down buttons.");
        break;
    }
    wxMessageBox(msg,caption);
}

void xLightsFrame::AllLightsOff()
{
    TestButtonsOff();
    if (xout) xout->alloff();
}

void xLightsFrame::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    int pagenum=Notebook1->GetSelection();
    if (pagenum == TESTTAB && !xout)
    {
        StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
    }
    else
    {
        StatusBar1->SetStatusText(_(""));
    }

    if (pagenum == PREVIEWTAB)
    {
        UpdatePreview();
    }

    if (event.GetOldSelection() == TESTTAB)
    {
        AllLightsOff();
    }
    m_dragging=false;
}


void xLightsFrame::OnButtonLightsOffClick(wxCommandEvent& event)
{
    AllLightsOff();
}

bool xLightsFrame::EnableOutputs()
{
    wxCriticalSectionLocker locker(gs_xoutCriticalSection);
    long MaxChan;
    bool ok=true;
    if (CheckBoxLightOutput->IsChecked() && xout==0)
    {
        xout = new xOutput();


        for( wxXmlNode* e=NetworkXML.GetRoot()->GetChildren(); e!=NULL && ok; e=e->GetNext() )
        {
            wxString tagname=e->GetName();
            if (tagname == wxT("network"))
            {
                wxString tempstr=e->GetAttribute(wxT("MaxChannels"), wxT("0"));
                tempstr.ToLong(&MaxChan);
                wxString NetworkType=e->GetAttribute(wxT("NetworkType"), wxT(""));
                wxString ComPort=e->GetAttribute(wxT("ComPort"), wxT(""));
                wxString BaudRate=e->GetAttribute(wxT("BaudRate"), wxT(""));
                int baud = (BaudRate == _("n/a")) ? 115200 : wxAtoi(BaudRate);
                wxString msg = _("Error occurred while connecting to ") + NetworkType+ _(" network on ") + ComPort +
                               _("\n\nThings to check:\n1. Are all required cables plugged in?") +
                               _("\n2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.") +
                               _("\n3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n");
                try
                {
                    xout->addnetwork(NetworkType,MaxChan,ComPort,baud);
                    //TextCtrlLog->AppendText(_("Successfully initialized ") + NetworkType + _(" network on ") + ComPort + _("\n"));
                }
                catch (const char *str)
                {
                    wxString errmsg(str,wxConvUTF8);
                    wxMessageBox(msg + errmsg, _("Communication Error"));
                    ok = false;
                }
                catch (char *str)
                {
                    wxString errmsg(str,wxConvUTF8);
                    wxMessageBox(msg + errmsg, _("Communication Error"));
                    ok = false;
                }
            }
        }
        if (!ok)
        {
            // uncheck checkbox since we were not able to initialize the port(s) successfully
            delete xout;
            xout=0;
            CheckBoxLightOutput->SetValue(false);
            //CheckBoxLightOutput->Enable(false);
        }
    }
    else if (!CheckBoxLightOutput->IsChecked() && xout)
    {
        delete xout;
        xout=0;
    }
    EnableNetworkChanges();
    return ok;
}

void xLightsFrame::EnableNetworkChanges()
{
    bool flag=(xout==0 && !CurrentDir.IsEmpty());
    ButtonAddDongle->Enable(flag);
    ButtonAddE131->Enable(flag);
    ButtonNetworkChange->Enable(flag);
    ButtonNetworkDelete->Enable(flag);
    ButtonNetworkDeleteAll->Enable(flag);
    BitmapButtonMoveNetworkUp->Enable(flag);
    BitmapButtonMoveNetworkDown->Enable(flag);
    ButtonSaveSetup->Enable(!CurrentDir.IsEmpty());
    ButtonSaveSchedule->Enable(!CurrentDir.IsEmpty());
    CheckBoxLightOutput->Enable(!CurrentDir.IsEmpty());
    CheckBoxRunSchedule->Enable(!CurrentDir.IsEmpty());
}

void xLightsFrame::OnCheckBoxLightOutputClick(wxCommandEvent& event)
{
    EnableOutputs();
    CheckChannelList=true;  // cause status bar to be updated if in test mode
}


void xLightsFrame::OnButtonStopNowClick(wxCommandEvent& event)
{
    PlayerDlg->MediaCtrl->Stop();
    if (play_mode == play_sched)
    {
        CheckBoxRunSchedule->SetValue(false);
        CheckRunSchedule();
    }
    if (basic.IsRunning()) basic.halt();
    SetPlayMode(play_off);
    ResetTimer(NO_SEQ);
    switch (Notebook1->GetSelection())
    {
    case TESTTAB:
        TestButtonsOff();
        break;
    case SEQUENCETAB:
        EnableSequenceControls(true);
        break;
    }
}

void xLightsFrame::OnButtonGracefulStopClick(wxCommandEvent& event)
{
    if (play_mode == play_sched)
    {
        EndTimeSec = 0;
    }
    else if (basic.IsRunning())
    {
        SecondsRemaining = 0;
        StatusBar1->SetStatusText(_("Finishing playlist"));
    }
    else
    {
        wxMessageBox(_("Graceful Stop is only useful when a schedule or playlist is running"));
    }
}

void xLightsFrame::OnButtonSaveScheduleClick(wxCommandEvent& event)
{
    SaveScheduleFile();
}

void xLightsFrame::OnMenuItemSavePlaylistsSelected(wxCommandEvent& event)
{
    SaveScheduleFile();
}



#include "TabSetup.cpp"
#include "TabTest.cpp"
#include "TabConvert.cpp"
#include "TabSchedule.cpp"
#include "TabPreview.cpp"
#include "TabSequence.cpp"

void xLightsFrame::OnClose(wxCloseEvent& event)
{
    if (UnsavedChanges && wxNO == wxMessageBox("Quit without saving?",
            "Unsaved Changes", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }

    // Disconnect the resize events, otherwise they get called as we are shutting down
    ScrolledWindowPreview->Disconnect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewResize,0,this);
    ScrolledWindow1->Disconnect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindow1Resize,0,this);

    Destroy();
}

void xLightsFrame::OnMenuItemBackupSelected(wxCommandEvent& event)
{
    wxString folderName;
    time_t cur;
    time(&cur);
    wxFileName newDirH;
    wxDateTime curTime(cur);


//  first make sure there is a Backup sub directory
    wxString newDirBackup = CurrentDir+wxFileName::GetPathSeparator()+wxT("Backup");
    if (!wxDirExists(newDirBackup) && !newDirH.Mkdir(newDirBackup))
    {
        wxMessageBox(wxT("Unable to create directory Backup!"),"Error", wxICON_ERROR|wxOK);
        return;
    }

 //if(curTime.ParseFormat("2003-xx-xx yy:yy", "%Y-%m-%d_%H%M%S"))

    wxString newDir = CurrentDir+wxFileName::GetPathSeparator()+wxString::Format(
                                 wxT("Backup%c%s-%s"),wxFileName::GetPathSeparator(),
                                 curTime.FormatISODate(),curTime.Format(wxT("%H%M%S")));

    if ( wxNO == wxMessageBox(wxT("All xml files under 10MB in your xlights directory will be backed up to \"")+
                              newDir+wxT("\". Proceed?"),wxT("Backup"),wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    if (!newDirH.Mkdir(newDir))
    {
        wxMessageBox(wxT("Unable to create directory!"),"Error", wxICON_ERROR|wxOK);
        return;
    }
    BackupDirectory(newDir);

    //CurrentDir
}

void xLightsFrame::BackupDirectory(wxString targetDirName)
{
    wxDir srcDir(CurrentDir);
    wxString fname;
    bool success;
    wxString srcDirName = CurrentDir+wxFileName::GetPathSeparator();
    wxFileName srcFile;
    srcFile.SetPath(CurrentDir);

    if(!srcDir.IsOpened())
    {
        return;
    }

    bool cont = srcDir.GetFirst(&fname, wxT("*.xml"), wxDIR_FILES);

    while (cont)
    {
        srcFile.SetFullName(fname);

        wxULongLong fsize=srcFile.GetSize();
        if(fsize > 10*1024*1024) // skip any xml files > 10 mbytes, they are something other than xml files
        {
            srcDir.GetNext(&fname);
            continue;
        }
        StatusBar1->SetStatusText(wxT("Copying File \"")+srcFile.GetFullPath());
        success = wxCopyFile(srcDirName+fname,
                              targetDirName+wxFileName::GetPathSeparator()+fname);
        if (!success)
        {
            wxMessageBox(wxT("Unable to copy file \"") + CurrentDir+wxFileName::GetPathSeparator()+fname+wxT("\""),
                         "Error", wxICON_ERROR|wxOK);
        }
        cont = srcDir.GetNext(&fname);
    }
    StatusBar1->SetStatusText(wxT("All xml files backed up."));
}

