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
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include "xLightsApp.h" //global app run-time flags
#include "heartbeat.h" //DJ
#include "XmlConversionDialog.h"

// scripting language
#include "xLightsBasic.cpp"

// image files
#include "../include/xlights.xpm"
#include "../include/open.xpm"
#include "../include/save.xpm"
#include "../include/insertrow.xpm"
#include "../include/deleterow.xpm"
#include "../include/padlock16x16-green.xpm" //-DJ
#include "../include/padlock16x16-red.xpm" //-DJ
#include "../include/padlock16x16-blue.xpm" //-DJ
#include "../include/control-pause-blue-icon.xpm"
#include "../include/control-pause-icon.xpm"
#include "../include/control-play-blue-icon.xpm"
#include "../include/control-play-icon.xpm"
#include "../include/control-stop-blue-icon.xpm"
#include "../include/control-stop-icon.xpm"



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
const long xLightsFrame::ID_AUITOOLBAR_OPENSHOW = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_NEWSEQUENCE = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_OPEN = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_SAVE = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_SAVEAS = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_MAIN = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM1 = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PAUSE = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_STOP = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PLAY = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_TAB_INFO = wxNewId();
const long xLightsFrame::ID_BUTTON_STOP_NOW = wxNewId();
const long xLightsFrame::ID_BUTTON_GRACEFUL_STOP = wxNewId();
const long xLightsFrame::ID_BUTTON_LIGHTS_OFF = wxNewId();
const long xLightsFrame::ID_CHECKBOX_LIGHT_OUTPUT = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_OUTPUT = wxNewId();
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
const long xLightsFrame::ID_STATICTEXT68 = wxNewId();
const long xLightsFrame::ID_STATICTEXT15 = wxNewId();
const long xLightsFrame::ID_BUTTON_CHOOSE_FILE = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_FILENAME = wxNewId();
const long xLightsFrame::ID_STATICTEXT16 = wxNewId();
const long xLightsFrame::ID_CHOICE_OUTPUT_FORMAT = wxNewId();
const long xLightsFrame::ID_STATICTEXT20 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_MAP_EMPTY_CHANNELS = wxNewId();
const long xLightsFrame::ID_STATICTEXT33 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_LOR_WITH_NO_CHANNELS = wxNewId();
const long xLightsFrame::ID_STATICTEXT17 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_OFF_AT_END = wxNewId();
const long xLightsFrame::ID_BUTTON_START_CONVERSION = wxNewId();
const long xLightsFrame::ID_STATICTEXT18 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_CONVERSION_STATUS = wxNewId();
const long xLightsFrame::ID_PANEL_CONVERT = wxNewId();
const long xLightsFrame::ID_BUTTON_PREVIEW_OPEN = wxNewId();
const long xLightsFrame::ID_STATICTEXT23 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON5 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON6 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PREVIEW_TIME = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_TIME = wxNewId();
const long xLightsFrame::ID_STATICTEXT_CURRENT_PREVIEW_SIZE = wxNewId();
const long xLightsFrame::ID_BUTTON_SET_PREVIEW_SIZE = wxNewId();
const long xLightsFrame::ID_BUTTON_SET_BACKGROUND_IMAGE = wxNewId();
const long xLightsFrame::ID_STATICTEXT32 = wxNewId();
const long xLightsFrame::ID_SLIDER_BACKGROUND_BRIGHTNESS = wxNewId();
const long xLightsFrame::ID_BUTTON_SELECT_MODEL_GROUPS = wxNewId();
const long xLightsFrame::ID_STATICTEXT21 = wxNewId();
const long xLightsFrame::ID_LISTBOX_ELEMENT_LIST = wxNewId();
const long xLightsFrame::ID_BUTTON_MODELS_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_BUILD_WHOLEHOUSE_MODEL = wxNewId();
const long xLightsFrame::ID_STATICTEXT22 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_SCALE = wxNewId();
const long xLightsFrame::ID_STATICTEXT25 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL2 = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_ROTATE = wxNewId();
const long xLightsFrame::ID_SCROLLEDWINDOW1 = wxNewId();
const long xLightsFrame::ID_PANEL_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON13 = wxNewId();
const long xLightsFrame::ID_BUTTON3 = wxNewId();
const long xLightsFrame::ID_BUTTON58 = wxNewId();
const long xLightsFrame::ID_CHOICE7 = wxNewId();
const long xLightsFrame::ID_BUTTON59 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON11 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON13 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON12 = wxNewId();
const long xLightsFrame::ID_PANEL31 = wxNewId();
const long xLightsFrame::ID_STATICTEXT4 = wxNewId();
const long xLightsFrame::ID_BUTTON_PLAY_RGB_SEQ = wxNewId();
const long xLightsFrame::ID_BUTTON2 = wxNewId();
const long xLightsFrame::ID_BUTTON_SeqExport = wxNewId();
const long xLightsFrame::ID_BUTTON4 = wxNewId();
const long xLightsFrame::ID_BUTTON_CREATE_RANDOM = wxNewId();
const long xLightsFrame::ID_BUTTON_CollapseEffectsButton = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON7 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON9 = wxNewId();
const long xLightsFrame::ID_CHECKBOX1 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON3 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON4 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_GRID_CUT = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_GRID_COPY = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_GRID_PASTE = wxNewId();
const long xLightsFrame::ID_STATICTEXT31 = wxNewId();
const long xLightsFrame::ID_CHOICE_VIEWS = wxNewId();
const long xLightsFrame::ID_BT_EDIT_VIEWS = wxNewId();
const long xLightsFrame::ID_GRID1 = wxNewId();
const long xLightsFrame::ID_PANEL_EFFECTS1_1 = wxNewId();
const long xLightsFrame::ID_PANEL5 = wxNewId();
const long xLightsFrame::ID_PANEL_EFFECTS2_1 = wxNewId();
const long xLightsFrame::ID_PANEL6 = wxNewId();
const long xLightsFrame::ID_NOTEBOOK2 = wxNewId();
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
const long xLightsFrame::ID_STATICTEXT26 = wxNewId();
const long xLightsFrame::ID_SPLITTERWINDOW3 = wxNewId();
const long xLightsFrame::ID_STATICTEXT28 = wxNewId();
const long xLightsFrame::ID_BUTTON5 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL1 = wxNewId();
const long xLightsFrame::ID_BUTTON_PgoStitch = wxNewId();
const long xLightsFrame::ID_STATICTEXT30 = wxNewId();
const long xLightsFrame::ID_BUTTON22 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL67 = wxNewId();
const long xLightsFrame::ID_STATICTEXT71 = wxNewId();
const long xLightsFrame::ID_STATICTEXT36 = wxNewId();
const long xLightsFrame::ID_CHOICE_PgoOutputType = wxNewId();
const long xLightsFrame::ID_STATICTEXT_PgoOutputType = wxNewId();
const long xLightsFrame::ID_STATICTEXT37 = wxNewId();
const long xLightsFrame::ID_STATICTEXT46 = wxNewId();
const long xLightsFrame::ID_STATICTEXT38 = wxNewId();
const long xLightsFrame::ID_CHOICE_PgoGroupName = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_SaveCoroGroup = wxNewId();
const long xLightsFrame::ID_BUTTON_CoroGroupDelete = wxNewId();
const long xLightsFrame::ID_BUTTON_CoroGroupClear = wxNewId();
const long xLightsFrame::ID_BUTTON_PgoCopyVoices = wxNewId();
const long xLightsFrame::ID_GRID_COROFACES = wxNewId();
const long xLightsFrame::ID_CHECKBOX_PgoAutoReset = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PgoMinRest = wxNewId();
const long xLightsFrame::ID_STATICTEXT27 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PgoMaxRest = wxNewId();
const long xLightsFrame::ID_STATICTEXT34 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_CoroEyesRandomBlink = wxNewId();
const long xLightsFrame::ID_CHECKBOX_PgoAutoFade = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PgoAutoFade = wxNewId();
const long xLightsFrame::ID_STATICTEXT35 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_CoroEyesRandomLR = wxNewId();
const long xLightsFrame::ID_CHECKBOX_CoroPictureScaled = wxNewId();
const long xLightsFrame::ID_STATICTEXT70 = wxNewId();
const long xLightsFrame::ID_BUTTON6 = wxNewId();
const long xLightsFrame::ID_PANEL4 = wxNewId();
const long xLightsFrame::ID_PANEL7 = wxNewId();
const long xLightsFrame::ID_NOTEBOOK1 = wxNewId();
const long xLightsFrame::ID_NEW_Non_Music_Seq = wxNewId();
const long xLightsFrame::ID_MENUITEM3 = wxNewId();
const long xLightsFrame::ID_New_Music_Seq = wxNewId();
const long xLightsFrame::ID_OPEN_SEQUENCE = wxNewId();
const long xLightsFrame::IS_SAVE_SEQ = wxNewId();
const long xLightsFrame::ID_MENUITEM2 = wxNewId();
const long xLightsFrame::ID_FILE_BACKUP = wxNewId();
const long xLightsFrame::idMenuQuit = wxNewId();
const long xLightsFrame::idMenuSaveSched = wxNewId();
const long xLightsFrame::idMenuAddList = wxNewId();
const long xLightsFrame::idMenuRenameList = wxNewId();
const long xLightsFrame::idMenuDelList = wxNewId();
const long xLightsFrame::ID_MENUITEM1 = wxNewId();
const long xLightsFrame::idCustomScript = wxNewId();
const long xLightsFrame::ID_EXPORT_ALL = wxNewId();
const long xLightsFrame::ID_EXPORT_MODEL = wxNewId();
const long xLightsFrame::ID_NO_THREADED_SAVE = wxNewId();
const long xLightsFrame::idMenuHelpContent = wxNewId();
const long xLightsFrame::idMenuAbout = wxNewId();
const long xLightsFrame::ID_STATUSBAR1 = wxNewId();
const long xLightsFrame::ID_TIMER1 = wxNewId();
const long xLightsFrame::ID_MESSAGEDIALOG1 = wxNewId();
//*)

// For new sequencer
const long xLightsFrame::ID_PANEL_EFFECTS1 = wxNewId();
const long xLightsFrame::ID_PANEL_EFFECTS = wxNewId();
const long xLightsFrame::ID_NOTEBOOK_EFFECTS = wxNewId();
// End

const long xLightsFrame::ID_PLAYER_DIALOG = wxNewId();
const long xLightsFrame::ID_DELETE_EFFECT = wxNewId();
const long xLightsFrame::ID_IGNORE_CLICK = wxNewId();
const long xLightsFrame::ID_PROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_UNPROTECT_EFFECT = wxNewId();
const long xLightsFrame::ID_RANDOM_EFFECT = wxNewId();
const long xLightsFrame::ID_COPYROW_EFFECT = wxNewId(); //copy random effect across row -DJ
const long xLightsFrame::ID_CLEARROW_EFFECT = wxNewId(); //clear all effects on this row -DJ

const long xLightsFrame::ID_PREVIEW_ALIGN = wxNewId();
const long xLightsFrame::ID_PREVIEW_MODEL_PROPERTIES = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long xLightsFrame::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long xLightsFrame::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long xLightsFrame::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long xLightsFrame::ID_PREVIEW_V_DISTRIBUTE = wxNewId();


wxDEFINE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDEFINE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDEFINE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);

BEGIN_EVENT_TABLE(xLightsFrame,wxFrame)
    //(*EventTable(xLightsFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_TIME_LINE_CHANGED, xLightsFrame::TimelineChanged)
    EVT_COMMAND(wxID_ANY, EVT_ZOOM, xLightsFrame::Zoom)
    EVT_COMMAND(wxID_ANY, EVT_HORIZ_SCROLL, xLightsFrame::HorizontalScrollChanged)
    EVT_COMMAND(wxID_ANY, EVT_TIME_SELECTED, xLightsFrame::TimeSelected)
    EVT_COMMAND(wxID_ANY, EVT_ROW_HEADINGS_CHANGED, xLightsFrame::RowHeadingsChanged)
    EVT_COMMAND(wxID_ANY, EVT_WINDOW_RESIZED, xLightsFrame::WindowResized)
    EVT_COMMAND(wxID_ANY, EVT_SELECTED_EFFECT_CHANGED, xLightsFrame::SelectedEffectChanged)

END_EVENT_TABLE()


class xlAuiToolBar : public wxAuiToolBar {
public:
    xlAuiToolBar(wxWindow* parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxAUI_TB_DEFAULT_STYLE) :
    wxAuiToolBar(parent, id, pos, size, style)
    {
    }
    virtual ~xlAuiToolBar() {

    }

    wxSize &GetAbsoluteMinSize() {
        return m_absoluteMinSize;
    }
    wxSize GetMinSize() {
        return m_absoluteMinSize;
    }
};

xLightsFrame::xLightsFrame(wxWindow* parent,wxWindowID id)
{

    //(*Initialize(xLightsFrame)
    wxBoxSizer* BoxSizer4;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxBoxSizer* BoxSizer6;
    wxMenu* MenuHelp;
    wxFlexGridSizer* FlexGridSizer4;
    wxBoxSizer* BoxSizerModelsPreview;
    wxFlexGridSizer* FlexGridSizer47;
    wxFlexGridSizer* FlexGridSizer54;
    wxFlexGridSizer* FlexGridSizerTest;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizerSetup;
    wxFlexGridSizer* FlexGridSizer24;
    wxMenuItem* MenuItemCustomScript;
    wxStaticBoxSizer* StaticBoxSizerHighlightColor;
    wxBoxSizer* BoxSizer5;
    wxBoxSizer* BoxSizer10;
    wxBoxSizer* BoxSizer7;
    wxFlexGridSizer* FlexGridSizer70;
    wxBoxSizer* BoxSizer8;
    wxFlexGridSizer* FlexGridSizer38;
    wxFlexGridSizer* FlexGridSizer23;
    wxMenuItem* MenuItem5;
    wxFlexGridSizer* FlexGridSizer41;
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItemAddList;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer27;
    wxMenuItem* MenuItem1;
    wxMenuItem* MenuItem4;
    wxFlexGridSizer* FlexGridSizer37;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer25;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer56;
    wxFlexGridSizer* FlexGridSizer9;
    wxBoxSizer* BoxSizerSequencePreview;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizerCal;
    wxStaticText* StaticText21;
    wxFlexGridSizer* FlexGridSizer55;
    wxMenuItem* MenuItemDelList;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizer57;
    wxFlexGridSizer* FlexGridSizer29;
    wxFlexGridSizer* FlexGridSizer34;
    wxFlexGridSizer* FlexGridSizerPreview;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer21;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer20;
    wxBoxSizer* BoxSizer1;
    wxMenuItem* MenuItemRenameList;
    wxFlexGridSizer* FlexGridSizer50;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer35;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer36;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer33;
    wxFlexGridSizer* FlexGridSizerConvert;
    wxFlexGridSizer* FlexGridSizer43;
    wxFlexGridSizer* FlexGridSizer11;
    wxBoxSizer* BoxSizer3;
    wxFlexGridSizer* FlexGridSizer17;
    wxFlexGridSizer* FlexGridSizerPapagayo;
    wxFlexGridSizer* FlexGridSizer32;
    wxFlexGridSizer* FlexGridSizer68;
    wxFlexGridSizer* FlexGridSizer31;
    wxFlexGridSizer* FlexGridSizer40;
    wxFlexGridSizer* FlexGridSizer39;
    wxFlexGridSizer* FlexGridSizer28;
    wxStaticBoxSizer* StaticBoxSizerBackgroundColor;
    wxMenu* MenuPlaylist;
    wxFlexGridSizer* FlexGridSizer26;
    wxFlexGridSizer* FlexGridSizer30;

    Create(parent, wxID_ANY, _("xLights/Nutcracker (Ver 4.0.0)"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(1100,800));
    SetToolTip(_("Export only Channels associated with one model"));
    MainAuiManager = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_ALLOW_ACTIVE_PANE|wxAUI_MGR_DEFAULT);
    MainToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_MAIN, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPENSHOW, _("Open Show Directory"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER_OPEN")),wxART_OTHER), wxNullBitmap, wxITEM_NORMAL, _("Open Show Directory"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_NEWSEQUENCE, _("New Sequence"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NEW")),wxART_OTHER), wxNullBitmap, wxITEM_NORMAL, _("New Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPEN, _("Open Sequence"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_OTHER), wxNullBitmap, wxITEM_NORMAL, _("Open Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVE, _("Save"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE")),wxART_OTHER), wxNullBitmap, wxITEM_NORMAL, _("Save"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVEAS, _("Save As"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE_AS")),wxART_OTHER), wxNullBitmap, wxITEM_NORMAL, _("Save As"), wxEmptyString, NULL);
    MainToolBar->Realize();
    MainAuiManager->AddPane(MainToolBar, wxAuiPaneInfo().Name(_T("Main Tool Bar")).ToolbarPane().Caption(_("Main Tool Bar")).CloseButton(false).Layer(10).Top().Gripper());
    PlayToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_PLAY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    PlayToolBar->AddTool(ID_AUITOOLBARITEM1, _("Play"), control_play_blue_icon, wxNullBitmap, wxITEM_NORMAL, _("Play"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_PAUSE, _("Pause"), control_pause_blue_icon, wxNullBitmap, wxITEM_NORMAL, _("Pause"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_STOP, _("Stop"), control_stop_icon, wxNullBitmap, wxITEM_NORMAL, _("Stop"), wxEmptyString, NULL);
    PlayToolBar->Realize();
    MainAuiManager->AddPane(PlayToolBar, wxAuiPaneInfo().Name(_T("Play Tool Bar")).ToolbarPane().Caption(_("Play Tool Bar")).CloseButton(false).Layer(10).Position(11).Top().Gripper());
    OutputToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_OUTPUT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    BitmapButtonTabInfo = new wxBitmapButton(OutputToolBar, ID_BITMAPBUTTON_TAB_INFO, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_INFORMATION")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxRAISED_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TAB_INFO"));
    BitmapButtonTabInfo->SetToolTip(_("Tips for using the current tab"));
    ButtonStopNow = new wxButton(OutputToolBar, ID_BUTTON_STOP_NOW, _("Stop Now!"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_STOP_NOW"));
    ButtonGracefulStop = new wxButton(OutputToolBar, ID_BUTTON_GRACEFUL_STOP, _("Graceful Stop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_GRACEFUL_STOP"));
    ButtonLightsOff = new wxButton(OutputToolBar, ID_BUTTON_LIGHTS_OFF, _("Lights Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LIGHTS_OFF"));
    CheckBoxLightOutput = new wxCheckBox(OutputToolBar, ID_CHECKBOX_LIGHT_OUTPUT, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LIGHT_OUTPUT"));
    CheckBoxLightOutput->SetValue(false);
    OutputToolBar->AddControl(BitmapButtonTabInfo, wxEmptyString);
    OutputToolBar->AddControl(ButtonStopNow, wxEmptyString);
    OutputToolBar->AddControl(ButtonGracefulStop, wxEmptyString);
    OutputToolBar->AddControl(ButtonLightsOff, wxEmptyString);
    OutputToolBar->AddControl(CheckBoxLightOutput, wxEmptyString);
    OutputToolBar->Realize();
    MainAuiManager->AddPane(OutputToolBar, wxAuiPaneInfo().Name(_T("Output Tool Bar")).ToolbarPane().Caption(_("Output Tool Bar")).CloseButton(false).Layer(10).Position(12).Top().Gripper());
    Notebook1 = new wxAuiNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxAUI_NB_TAB_EXTERNAL_MOVE|wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_TOP|wxNO_BORDER);
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
    StaticText65 = new wxStaticText(PanelConvert, ID_STATICTEXT68, _("For LMS imports, LOR networks should be first followed by DMX starting at universe 1."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT68"));
    FlexGridSizerConvert->Add(StaticText65, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 12);
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
    ChoiceOutputFormat->Append(_("Glediator Record File *.gled"));
    ChoiceOutputFormat->Append(_("Lcb, LOR clipboard *.lcb"));
    ChoiceOutputFormat->Append(_("HLS *.hlsnc"));
    FlexGridSizer26->Add(ChoiceOutputFormat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText20 = new wxStaticText(PanelConvert, ID_STATICTEXT20, _("Map Empty LMS Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    FlexGridSizer26->Add(StaticText20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxMapEmptyChannels = new wxCheckBox(PanelConvert, ID_CHECKBOX_MAP_EMPTY_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MAP_EMPTY_CHANNELS"));
    CheckBoxMapEmptyChannels->SetValue(false);
    FlexGridSizer26->Add(CheckBoxMapEmptyChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(PanelConvert, ID_STATICTEXT33, _("Map LMS Channels with no network"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer26->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MapLORChannelsWithNoNetwork = new wxCheckBox(PanelConvert, ID_CHECKBOX_LOR_WITH_NO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LOR_WITH_NO_CHANNELS"));
    MapLORChannelsWithNoNetwork->SetValue(true);
    FlexGridSizer26->Add(MapLORChannelsWithNoNetwork, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
    FlexGridSizerPreview = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizerPreview->AddGrowableCol(0);
    FlexGridSizerPreview->AddGrowableRow(2);
    FlexGridSizer36 = new wxFlexGridSizer(1, 3, 0, 0);
    FlexGridSizer36->AddGrowableCol(2);
    ButtonPreviewOpen = new wxButton(PanelPreview, ID_BUTTON_PREVIEW_OPEN, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PREVIEW_OPEN"));
    FlexGridSizer36->Add(ButtonPreviewOpen, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(PanelPreview, wxID_ANY, _("Now playing:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer36->Add(StaticText21, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextPreviewFileName = new wxStaticText(PanelPreview, ID_STATICTEXT23, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer36->Add(StaticTextPreviewFileName, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPreview->Add(FlexGridSizer36, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer31 = new wxFlexGridSizer(1, 4, 0, 0);
    FlexGridSizer31->AddGrowableCol(3);
    bbPlayPause = new wxBitmapButton(PanelPreview, ID_BITMAPBUTTON5, control_pause_blue_icon, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
    bbPlayPause->SetBitmapDisabled(control_pause_icon);
    FlexGridSizer31->Add(bbPlayPause, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    bbStop = new wxBitmapButton(PanelPreview, ID_BITMAPBUTTON6, control_stop_blue_icon, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
    bbStop->SetBitmapDisabled(control_stop_icon);
    FlexGridSizer31->Add(bbStop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlPreviewTime = new wxTextCtrl(PanelPreview, ID_TEXTCTRL_PREVIEW_TIME, wxEmptyString, wxDefaultPosition, wxSize(58,21), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_TIME"));
    FlexGridSizer31->Add(TextCtrlPreviewTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer30 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer41 = new wxFlexGridSizer(0, 3, 0, 0);
    SliderPreviewTime = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_TIME, 0, 0, 200, wxDefaultPosition, wxSize(250,30), 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_TIME"));
    FlexGridSizer41->Add(SliderPreviewTime, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30->Add(FlexGridSizer41, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer40 = new wxFlexGridSizer(0, 5, 0, 0);
    StaticTextCurrentPreviewSize = new wxStaticText(PanelPreview, ID_STATICTEXT_CURRENT_PREVIEW_SIZE, _("Size: 1920x1080"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_CURRENT_PREVIEW_SIZE"));
    FlexGridSizer40->Add(StaticTextCurrentPreviewSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSetPreviewSize = new wxButton(PanelPreview, ID_BUTTON_SET_PREVIEW_SIZE, _("Set Size"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SET_PREVIEW_SIZE"));
    FlexGridSizer40->Add(ButtonSetPreviewSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSetBackgroundImage = new wxButton(PanelPreview, ID_BUTTON_SET_BACKGROUND_IMAGE, _("Image"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SET_BACKGROUND_IMAGE"));
    FlexGridSizer40->Add(ButtonSetBackgroundImage, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(PanelPreview, ID_STATICTEXT32, _("Brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer40->Add(StaticText30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_BackgroundBrightness = new wxSlider(PanelPreview, ID_SLIDER_BACKGROUND_BRIGHTNESS, 0, 0, 100, wxDefaultPosition, wxSize(80,20), 0, wxDefaultValidator, _T("ID_SLIDER_BACKGROUND_BRIGHTNESS"));
    FlexGridSizer40->Add(Slider_BackgroundBrightness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30->Add(FlexGridSizer40, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer31->Add(FlexGridSizer30, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPreview->Add(FlexGridSizer31, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer34->AddGrowableCol(1);
    FlexGridSizer34->AddGrowableRow(0);
    FlexGridSizer35 = new wxFlexGridSizer(0, 1, 0, 0);
    ButtonSelectModelGroups = new wxButton(PanelPreview, ID_BUTTON_SELECT_MODEL_GROUPS, _("Select Model Groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_MODEL_GROUPS"));
    FlexGridSizer35->Add(ButtonSelectModelGroups, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(PanelPreview, ID_STATICTEXT21, _("Preview Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer35->Add(StaticText5, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ListBoxElementList = new wxListBox(PanelPreview, ID_LISTBOX_ELEMENT_LIST, wxDefaultPosition, wxSize(132,240), 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX_ELEMENT_LIST"));
    FlexGridSizer35->Add(ListBoxElementList, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonModelsPreview = new wxButton(PanelPreview, ID_BUTTON_MODELS_PREVIEW, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MODELS_PREVIEW"));
    FlexGridSizer35->Add(ButtonModelsPreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSavePreview = new wxButton(PanelPreview, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
    FlexGridSizer35->Add(ButtonSavePreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonBuildWholeHouseModel = new wxButton(PanelPreview, ID_BUTTON_BUILD_WHOLEHOUSE_MODEL, _("Build Whole House Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_BUILD_WHOLEHOUSE_MODEL"));
    FlexGridSizer35->Add(ButtonBuildWholeHouseModel, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer37 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer37->AddGrowableCol(1);
    StaticText23 = new wxStaticText(PanelPreview, ID_STATICTEXT22, _("Model Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer37->Add(StaticText23, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlPreviewElementSize = new wxTextCtrl(PanelPreview, ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE, _("50"), wxDefaultPosition, wxSize(30,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE"));
    FlexGridSizer37->Add(TextCtrlPreviewElementSize, 1, wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer37, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SliderPreviewScale = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_SCALE, 50, 1, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_SCALE"));
    FlexGridSizer35->Add(SliderPreviewScale, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer39 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticTextPreviewRotation = new wxStaticText(PanelPreview, ID_STATICTEXT25, _("Model Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    StaticTextPreviewRotation->Disable();
    FlexGridSizer39->Add(StaticTextPreviewRotation, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlModelRotationDegrees = new wxTextCtrl(PanelPreview, ID_TEXTCTRL2, _("0"), wxDefaultPosition, wxSize(30,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer39->Add(TextCtrlModelRotationDegrees, 1, wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer39, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SliderPreviewRotate = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_ROTATE, 0, -180, 180, wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_ROTATE"));
    SliderPreviewRotate->SetTickFreq(15);
    SliderPreviewRotate->Disable();
    FlexGridSizer35->Add(SliderPreviewRotate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34->Add(FlexGridSizer35, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    ScrolledWindowPreview = new wxScrolledWindow(PanelPreview, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
    BoxSizerModelsPreview = new wxBoxSizer(wxHORIZONTAL);
    ScrolledWindowPreview->SetSizer(BoxSizerModelsPreview);
    BoxSizerModelsPreview->Fit(ScrolledWindowPreview);
    BoxSizerModelsPreview->SetSizeHints(ScrolledWindowPreview);
    FlexGridSizer34->Add(ScrolledWindowPreview, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPreview->Add(FlexGridSizer34, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelPreview->SetSizer(FlexGridSizerPreview);
    FlexGridSizerPreview->Fit(PanelPreview);
    FlexGridSizerPreview->SetSizeHints(PanelPreview);
    PaneNutcracker = new wxPanel(Notebook1, ID_PANEL30, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL30"));
    FlexGridSizer70 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer70->AddGrowableCol(0);
    FlexGridSizer70->AddGrowableRow(0);
    SplitterWindow2 = new wxSplitterWindow(PaneNutcracker, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH|wxSP_NOBORDER|wxSP_NO_XP_THEME, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetMinSize(wxSize(10,10));
    SplitterWindow2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    SplitterWindow2->SetMinimumPaneSize(10);
    SplitterWindow2->SetSashGravity(0.5);
    SeqPanelLeft = new wxPanel(SplitterWindow2, ID_PANEL31, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL31"));
    SeqPanelLeft->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    BoxSizer6 = new wxBoxSizer(wxVERTICAL);
    BoxSizerSequencePreview = new wxBoxSizer(wxHORIZONTAL);
    BoxSizer6->Add(BoxSizerSequencePreview, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, SeqPanelLeft, _("Combined Effect"));
    FlexGridSizer33 = new wxFlexGridSizer(0, 4, 0, 0);
    Button_PlayEffect = new wxButton(SeqPanelLeft, ID_BUTTON13, _("Play (F3)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    Button_PlayEffect->Disable();
    Button_PlayEffect->SetBackgroundColour(wxColour(0,255,0));
    FlexGridSizer33->Add(Button_PlayEffect, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Button_UpdateGrid = new wxButton(SeqPanelLeft, ID_BUTTON3, _("Update Grid (F5)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Button_UpdateGrid->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer33->Add(Button_UpdateGrid, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer33->Add(-1,-1,1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Button_Models = new wxButton(SeqPanelLeft, ID_BUTTON58, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON58"));
    Button_Models->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer33->Add(Button_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Models = new wxChoice(SeqPanelLeft, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE7"));
    FlexGridSizer33->Add(Choice_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Button_Presets = new wxButton(SeqPanelLeft, ID_BUTTON59, _("Effect Presets"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON59"));
    Button_Presets->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_Presets->SetToolTip(_("Click here to load previously saved effects"));
    FlexGridSizer33->Add(Button_Presets, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette = new wxButton(SeqPanelLeft, ID_BUTTON_Palette, _("Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette"));
    Button_Palette->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_Palette->SetToolTip(_("Ckick here to load Palettes of colors."));
    FlexGridSizer33->Add(Button_Palette, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_normal = new wxBitmapButton(SeqPanelLeft, ID_BITMAPBUTTON11, padlock16x16_green_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON11"));
    BitmapButton_normal->SetDefault();
    BitmapButton_normal->Hide();
    FlexGridSizer33->Add(BitmapButton_normal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BitmapButton_locked = new wxBitmapButton(SeqPanelLeft, ID_BITMAPBUTTON13, padlock16x16_red_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON13"));
    BitmapButton_locked->SetDefault();
    BitmapButton_locked->Hide();
    FlexGridSizer33->Add(BitmapButton_locked, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    BitmapButton_random = new wxBitmapButton(SeqPanelLeft, ID_BITMAPBUTTON12, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON12"));
    BitmapButton_random->SetDefault();
    BitmapButton_random->Hide();
    FlexGridSizer33->Add(BitmapButton_random, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    StaticBoxSizer3->Add(FlexGridSizer33, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BoxSizer6->Add(StaticBoxSizer3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    SeqPanelLeft->SetSizer(BoxSizer6);
    BoxSizer6->Fit(SeqPanelLeft);
    BoxSizer6->SetSizeHints(SeqPanelLeft);
    SeqPanelRight = new wxPanel(SplitterWindow2, ID_PANEL32, wxPoint(40,-11), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL32"));
    SeqPanelRight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    BoxSizer4 = new wxBoxSizer(wxVERTICAL);
    StaticBoxSizerSequenceButtons = new wxStaticBoxSizer(wxVERTICAL, SeqPanelRight, _("RGB Sequence"));
    FlexGridSizer32 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer32->AddGrowableCol(0);
    FlexGridSizer32->AddGrowableRow(2);
    StaticTextSequenceFileName = new wxStaticText(SeqPanelRight, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer32->Add(StaticTextSequenceFileName, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
    BoxSizer7 = new wxBoxSizer(wxVERTICAL);
    FlexGridSizer68 = new wxFlexGridSizer(1, 6, 0, 0);
    Button_PlayRgbSeq = new wxButton(SeqPanelRight, ID_BUTTON_PLAY_RGB_SEQ, _("Play (F4)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PLAY_RGB_SEQ"));
    Button_PlayRgbSeq->SetBackgroundColour(wxColour(0,255,0));
    Button_PlayRgbSeq->SetToolTip(_("Play your sequence. Before pressing Play, highlight a grid cell under a model. You will see that model show in the window. If you want to see all models switch over to the PREVIEW window."));
    FlexGridSizer68->Add(Button_PlayRgbSeq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonDisplayElements = new wxButton(SeqPanelRight, ID_BUTTON2, _("Display Elements"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    ButtonDisplayElements->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    ButtonDisplayElements->SetToolTip(_("Display Models that can be attached to your Sequence"));
    FlexGridSizer68->Add(ButtonDisplayElements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSeqExport = new wxButton(SeqPanelRight, ID_BUTTON_SeqExport, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SeqExport"));
    ButtonSeqExport->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    ButtonSeqExport->SetToolTip(_("Export all Channels"));
    FlexGridSizer68->Add(ButtonSeqExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonModelExport = new wxButton(SeqPanelRight, ID_BUTTON4, _("Model Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    ButtonModelExport->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    ButtonModelExport->SetToolTip(_("Export Channels for one model"));
    FlexGridSizer68->Add(ButtonModelExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_CreateRandom = new wxButton(SeqPanelRight, ID_BUTTON_CREATE_RANDOM, _("Create Random Effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CREATE_RANDOM"));
    Button_CreateRandom->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Button_CreateRandom->SetToolTip(_("Create a random effect for every unprotected cell in grid. If you hold down the SHIFT key while clicking \"Create Random Effects\", a random effect will be created and assigned to every model on a row."));
    FlexGridSizer68->Add(Button_CreateRandom, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CollapseEffectsButton = new wxButton(SeqPanelRight, ID_BUTTON_CollapseEffectsButton, _("v"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator, _T("ID_BUTTON_CollapseEffectsButton"));
    FlexGridSizer68->Add(CollapseEffectsButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer7->Add(FlexGridSizer68, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer38 = new wxFlexGridSizer(1, 12, 0, 0);
    BitmapButtonOpenSeq = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON7, open_xpm, wxDefaultPosition, wxSize(23,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    BitmapButtonOpenSeq->SetDefault();
    BitmapButtonOpenSeq->SetToolTip(_("Open Sequence, Create a Sequence"));
    FlexGridSizer38->Add(BitmapButtonOpenSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonSaveSeq = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON9, save_xpm, wxDefaultPosition, wxSize(23,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    BitmapButtonSaveSeq->SetDefault();
    BitmapButtonSaveSeq->SetToolTip(_("Save Sequence. The save will be done for every model you have. The save will start with the leftmost model and finish with the rightmost"));
    FlexGridSizer38->Add(BitmapButtonSaveSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FastSave_CheckBox = new wxCheckBox(SeqPanelRight, ID_CHECKBOX1, _("Fast Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    FastSave_CheckBox->SetValue(false);
    FlexGridSizer38->Add(FastSave_CheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonInsertRow = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON3, insertrow_xpm, wxDefaultPosition, wxSize(23,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    BitmapButtonInsertRow->SetDefault();
    BitmapButtonInsertRow->SetToolTip(_("Insert Row"));
    FlexGridSizer38->Add(BitmapButtonInsertRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonDeleteRow = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON4, deleterow_xpm, wxDefaultPosition, wxSize(23,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    BitmapButtonDeleteRow->SetDefault();
    BitmapButtonDeleteRow->SetToolTip(_("Delete Row"));
    FlexGridSizer38->Add(BitmapButtonDeleteRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonGridCut = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON_GRID_CUT, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CUT")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_GRID_CUT"));
    BitmapButtonGridCut->SetToolTip(_("Cut"));
    FlexGridSizer38->Add(BitmapButtonGridCut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonGridCopy = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON_GRID_COPY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_COPY")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_GRID_COPY"));
    BitmapButtonGridCopy->SetDefault();
    BitmapButtonGridCopy->SetToolTip(_("Copy"));
    FlexGridSizer38->Add(BitmapButtonGridCopy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonGridPaste = new wxBitmapButton(SeqPanelRight, ID_BITMAPBUTTON_GRID_PASTE, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_PASTE")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_GRID_PASTE"));
    BitmapButtonGridPaste->SetDefault();
    BitmapButtonGridPaste->SetToolTip(_("Paste"));
    FlexGridSizer38->Add(BitmapButtonGridPaste, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText28 = new wxStaticText(SeqPanelRight, ID_STATICTEXT31, _("View:"), wxDefaultPosition, wxSize(-1,-1), 0, _T("ID_STATICTEXT31"));
    FlexGridSizer38->Add(StaticText28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Views = new wxChoice(SeqPanelRight, ID_CHOICE_VIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VIEWS"));
    FlexGridSizer38->Add(Choice_Views, 1, wxALL|wxFIXED_MINSIZE|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    btEditViews = new wxButton(SeqPanelRight, ID_BT_EDIT_VIEWS, _("Edit View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BT_EDIT_VIEWS"));
    FlexGridSizer38->Add(btEditViews, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer7->Add(FlexGridSizer38, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer32->Add(BoxSizer7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
    Notebook2 = new wxNotebook(SeqPanelRight, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
    Panel4 = new wxPanel(Notebook2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    EffectsPanel1_1 = new EffectsPanel(Panel4, ID_PANEL_EFFECTS1_1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1_1"));
    Panel5 = new wxPanel(Notebook2, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    EffectsPanel2_1 = new EffectsPanel(Panel5, ID_PANEL_EFFECTS2_1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS2_1"));
    Notebook2->AddPage(Panel4, _("Effect1"), false);
    Notebook2->AddPage(Panel5, _("Effect2"), false);
    BoxSizer5->Add(Notebook2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer4->Add(BoxSizer5, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SeqPanelRight->SetSizer(BoxSizer4);
    BoxSizer4->Fit(SeqPanelRight);
    BoxSizer4->SetSizeHints(SeqPanelRight);
    SplitterWindow2->SplitVertically(SeqPanelLeft, SeqPanelRight);
    FlexGridSizer70->Add(SplitterWindow2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    PaneNutcracker->SetSizer(FlexGridSizer70);
    FlexGridSizer70->Fit(PaneNutcracker);
    FlexGridSizer70->SetSizeHints(PaneNutcracker);
    PanelCal = new wxPanel(Notebook1, ID_PANEL_CAL, wxPoint(49,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_CAL"));
    FlexGridSizerCal = new wxFlexGridSizer(2, 2, 0, 0);
    FlexGridSizerCal->AddGrowableCol(0);
    FlexGridSizerCal->AddGrowableRow(0);
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
    ButtonSaveSchedule->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    BoxSizer2->Add(ButtonSaveSchedule, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonAddShow = new wxButton(Panel3, ID_BUTTON_ADD_SHOW, _("Schedule Playlist"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD_SHOW"));
    ButtonAddShow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    BoxSizer2->Add(ButtonAddShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonUpdateShow = new wxButton(Panel3, ID_BUTTON_UPDATE_SHOW, _("Update Selected Items"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_UPDATE_SHOW"));
    ButtonUpdateShow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    BoxSizer2->Add(ButtonUpdateShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonDeleteShow = new wxButton(Panel3, ID_BUTTON_DELETE_SHOW, _("Delete Selected Items"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_SHOW"));
    ButtonDeleteShow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    BoxSizer2->Add(ButtonDeleteShow, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer27->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText2 = new wxStaticText(Panel3, ID_STATICTEXT2, _("Show Dates"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    wxFont StaticText2Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText2->SetFont(StaticText2Font);
    FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonShowDatesChange = new wxButton(Panel3, ID_BUTTON_SHOW_DATES_CHANGE, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SHOW_DATES_CHANGE"));
    ButtonShowDatesChange->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
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
    ButtonClearLog->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer5->Add(ButtonClearLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSaveLog = new wxButton(Panel2, ID_BUTTON_SAVELOG, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVELOG"));
    ButtonSaveLog->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    FlexGridSizer5->Add(ButtonSaveLog, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer29->Add(FlexGridSizer5, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer28->Add(FlexGridSizer29, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer28);
    FlexGridSizer28->Fit(Panel2);
    FlexGridSizer28->SetSizeHints(Panel2);
    SplitterWindow1->SplitVertically(Panel3, Panel2);
    FlexGridSizerCal->Add(SplitterWindow1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelCal->SetSizer(FlexGridSizerCal);
    FlexGridSizerCal->Fit(PanelCal);
    FlexGridSizerCal->SetSizeHints(PanelCal);
    PanelPapagayo = new wxPanel(Notebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizerPapagayo = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerPapagayo->AddGrowableCol(0);
    FlexGridSizerPapagayo->AddGrowableRow(0);
    FlexGridSizer43 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText24 = new wxStaticText(PanelPapagayo, ID_STATICTEXT26, _(" Convert Papagayo file into an xLights Sequence\n\n Download v1.3.5.3 from nutcracker123.com/nutcracker/releases/papagayoMOD_1.3.5.3_win32.rar"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    wxFont StaticText24Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Arial"),wxFONTENCODING_DEFAULT);
    StaticText24->SetFont(StaticText24Font);
    FlexGridSizer43->Add(StaticText24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    SplitterWindow3 = new wxSplitterWindow(PanelPapagayo, ID_SPLITTERWINDOW3, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW3"));
    SplitterWindow3->SetMinSize(wxSize(10,10));
    SplitterWindow3->SetMinimumPaneSize(10);
    SplitterWindow3->SetSashGravity(0.5);
    FlexGridSizer43->Add(SplitterWindow3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText26 = new wxStaticText(PanelPapagayo, ID_STATICTEXT28, _("1)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    wxFont StaticText26Font(14,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText26->SetFont(StaticText26Font);
    FlexGridSizer56->Add(StaticText26, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer47 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_pgo_filename = new wxButton(PanelPapagayo, ID_BUTTON5, _("Select Papagayo Input File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    Button_pgo_filename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer47->Add(Button_pgo_filename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_pgo_filename = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(236,23), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrl_pgo_filename->SetToolTip(_("Papagayo input file contains voices and phonemes."));
    FlexGridSizer47->Add(TextCtrl_pgo_filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_PgoStitch = new wxButton(PanelPapagayo, ID_BUTTON_PgoStitch, _("+"), wxDefaultPosition, wxSize(29,23), 0, wxDefaultValidator, _T("ID_BUTTON_PgoStitch"));
    Button_PgoStitch->SetToolTip(_("Stitch Papagayo file"));
    FlexGridSizer47->Add(Button_PgoStitch, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer56->Add(FlexGridSizer47, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText27 = new wxStaticText(PanelPapagayo, ID_STATICTEXT30, _("2)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    wxFont StaticText27Font(14,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText27->SetFont(StaticText27Font);
    FlexGridSizer56->Add(StaticText27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer54 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_papagayo_output_sequence = new wxButton(PanelPapagayo, ID_BUTTON22, _("Select Sequencer Output File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON22"));
    Button_papagayo_output_sequence->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer54->Add(Button_papagayo_output_sequence, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_papagayo_output_filename = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL67, wxEmptyString, wxDefaultPosition, wxSize(237,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL67"));
    TextCtrl_papagayo_output_filename->SetToolTip(_("Sequence output will contain the generated effects."));
    FlexGridSizer54->Add(TextCtrl_papagayo_output_filename, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer56->Add(FlexGridSizer54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText68 = new wxStaticText(PanelPapagayo, ID_STATICTEXT71, _("3)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT71"));
    wxFont StaticText68Font(14,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText68->SetFont(StaticText68Font);
    FlexGridSizer56->Add(StaticText68, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer57 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText34 = new wxStaticText(PanelPapagayo, ID_STATICTEXT36, _("Choose output type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer57->Add(StaticText34, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_PgoOutputType = new wxChoice(PanelPapagayo, ID_CHOICE_PgoOutputType, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PgoOutputType"));
    Choice_PgoOutputType->Append(_("(choose)"));
    Choice_PgoOutputType->Append(_("Auto-faces"));
    Choice_PgoOutputType->Append(_("Coro faces"));
    Choice_PgoOutputType->Append(_("Image faces"));
    Choice_PgoOutputType->Append(_("Movie faces"));
    Choice_PgoOutputType->SetToolTip(_("Output type determines the effects used to generate the sequence."));
    FlexGridSizer57->Add(Choice_PgoOutputType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_PgoOutputType = new wxStaticText(PanelPapagayo, ID_STATICTEXT_PgoOutputType, _("(explanation)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PgoOutputType"));
    FlexGridSizer57->Add(StaticText_PgoOutputType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer56->Add(FlexGridSizer57, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText35 = new wxStaticText(PanelPapagayo, ID_STATICTEXT37, _("4)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    wxFont StaticText35Font(14,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText35->SetFont(StaticText35Font);
    FlexGridSizer56->Add(StaticText35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText43 = new wxStaticText(PanelPapagayo, ID_STATICTEXT46, _("In the grid below, configure how phonemes and face parts are rendered.\nPresets allow settings to be saved and reused again later."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    wxFont StaticText43Font(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText43->SetFont(StaticText43Font);
    FlexGridSizer56->Add(StaticText43, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer50 = new wxFlexGridSizer(0, 8, 0, 0);
    StaticText36 = new wxStaticText(PanelPapagayo, ID_STATICTEXT38, _("Preset Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer50->Add(StaticText36, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_PgoGroupName = new wxChoice(PanelPapagayo, ID_CHOICE_PgoGroupName, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_PgoGroupName"));
    Choice_PgoGroupName->SetToolTip(_("Presets allow multiple groups of settings to be saved.  Choose an existing Preset or create a new one."));
    FlexGridSizer50->Add(Choice_PgoGroupName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SaveCoroGroup = new wxBitmapButton(PanelPapagayo, ID_BITMAPBUTTON_SaveCoroGroup, save_xpm, wxDefaultPosition, wxSize(53,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_SaveCoroGroup"));
    BitmapButton_SaveCoroGroup->SetDefault();
    BitmapButton_SaveCoroGroup->SetToolTip(_("Named Presets can be saved and reused again later."));
    FlexGridSizer50->Add(BitmapButton_SaveCoroGroup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_CoroGroupDelete = new wxButton(PanelPapagayo, ID_BUTTON_CoroGroupDelete, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CoroGroupDelete"));
    Button_CoroGroupDelete->SetToolTip(_("Delete current Preset."));
    FlexGridSizer50->Add(Button_CoroGroupDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_CoroGroupClear = new wxButton(PanelPapagayo, ID_BUTTON_CoroGroupClear, _("Clear Grid"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CoroGroupClear"));
    Button_CoroGroupClear->SetToolTip(_("Clear all grid values."));
    FlexGridSizer50->Add(Button_CoroGroupClear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_PgoCopyVoices = new wxButton(PanelPapagayo, ID_BUTTON_PgoCopyVoices, _("Copy First"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PgoCopyVoices"));
    Button_PgoCopyVoices->SetToolTip(_("Copy first Voice values to other columns."));
    FlexGridSizer50->Add(Button_PgoCopyVoices, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(FlexGridSizer50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    GridCoroFaces = new wxGrid(PanelPapagayo, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
    GridCoroFaces->CreateGrid(18,4);
    GridCoroFaces->SetMinSize(wxSize(-1,200));
    GridCoroFaces->EnableEditing(true);
    GridCoroFaces->EnableGridLines(true);
    GridCoroFaces->SetColLabelSize(20);
    GridCoroFaces->SetRowLabelSize(100);
    GridCoroFaces->SetDefaultColSize(110, true);
    GridCoroFaces->SetColLabelValue(0, _("Voice 1"));
    GridCoroFaces->SetColLabelValue(1, _("Voice 2"));
    GridCoroFaces->SetColLabelValue(2, _("Voice 3"));
    GridCoroFaces->SetColLabelValue(3, _("Voice 4"));
    GridCoroFaces->SetRowLabelValue(0, _("Model Name"));
    GridCoroFaces->SetRowLabelValue(1, _("Face Outline"));
    GridCoroFaces->SetRowLabelValue(2, _("Mouth - AI"));
    GridCoroFaces->SetRowLabelValue(3, _("Mouth - E"));
    GridCoroFaces->SetRowLabelValue(4, _("Mouth - etc"));
    GridCoroFaces->SetRowLabelValue(5, _("Mouth - FV"));
    GridCoroFaces->SetRowLabelValue(6, _("Mouth - L"));
    GridCoroFaces->SetRowLabelValue(7, _("Mouth - MBP"));
    GridCoroFaces->SetRowLabelValue(8, _("Mouth - O"));
    GridCoroFaces->SetRowLabelValue(9, _("Mouth - rest"));
    GridCoroFaces->SetRowLabelValue(10, _("Mouth - U"));
    GridCoroFaces->SetRowLabelValue(11, _("Mouth - WQ"));
    GridCoroFaces->SetRowLabelValue(12, _("Eyes - Open"));
    GridCoroFaces->SetRowLabelValue(13, _("Eyes - Closed"));
    GridCoroFaces->SetRowLabelValue(14, _("Eyes - Left"));
    GridCoroFaces->SetRowLabelValue(15, _("Eyes - Right"));
    GridCoroFaces->SetRowLabelValue(16, _("Eyes - Up"));
    GridCoroFaces->SetRowLabelValue(17, _("Eyes - Down"));
    GridCoroFaces->SetDefaultCellFont( GridCoroFaces->GetFont() );
    GridCoroFaces->SetDefaultCellTextColour( GridCoroFaces->GetForegroundColour() );
    BoxSizer8->Add(GridCoroFaces, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(BoxSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer55 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_PgoAutoRest = new wxCheckBox(PanelPapagayo, ID_CHECKBOX_PgoAutoReset, _("Insert Rests. Minimum gap:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PgoAutoReset"));
    CheckBox_PgoAutoRest->SetValue(false);
    FlexGridSizer55->Add(CheckBox_PgoAutoRest, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    TextCtrl_PgoMinRest = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoMinRest, _(".2"), wxDefaultPosition, wxSize(40,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoMinRest"));
    TextCtrl_PgoMinRest->SetToolTip(_("Minimum gap size to insert Rest into."));
    FlexGridSizer3->Add(TextCtrl_PgoMinRest, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText25 = new wxStaticText(PanelPapagayo, ID_STATICTEXT27, _("max:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer3->Add(StaticText25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_PgoMaxRest = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoMaxRest, _(".4"), wxDefaultPosition, wxSize(40,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoMaxRest"));
    TextCtrl_PgoMaxRest->SetToolTip(_("Maximum gap size to insert Rest into."));
    FlexGridSizer3->Add(TextCtrl_PgoMaxRest, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText32 = new wxStaticText(PanelPapagayo, ID_STATICTEXT34, _("sec.        "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer3->Add(StaticText32, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer55->Add(FlexGridSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_CoroEyesRandomBlink = new wxCheckBox(PanelPapagayo, ID_CHECKBOX_CoroEyesRandomBlink, _("Eyes random blink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_CoroEyesRandomBlink"));
    CheckBox_CoroEyesRandomBlink->SetValue(false);
    FlexGridSizer55->Add(CheckBox_CoroEyesRandomBlink, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_PgoAutoFade = new wxCheckBox(PanelPapagayo, ID_CHECKBOX_PgoAutoFade, _("Auto-fade each face after:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PgoAutoFade"));
    CheckBox_PgoAutoFade->SetValue(false);
    FlexGridSizer55->Add(CheckBox_PgoAutoFade, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
    TextCtrl_PgoAutoFade = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoAutoFade, _("1.5"), wxDefaultPosition, wxSize(52,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoAutoFade"));
    TextCtrl_PgoAutoFade->SetToolTip(_("Fade after this length of inactivity."));
    FlexGridSizer8->Add(TextCtrl_PgoAutoFade, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText33 = new wxStaticText(PanelPapagayo, ID_STATICTEXT35, _("sec.        "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer8->Add(StaticText33, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer55->Add(FlexGridSizer8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_CoroEyesRandomLR = new wxCheckBox(PanelPapagayo, ID_CHECKBOX_CoroEyesRandomLR, _("Eyes random left/right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_CoroEyesRandomLR"));
    CheckBox_CoroEyesRandomLR->SetValue(false);
    FlexGridSizer55->Add(CheckBox_CoroEyesRandomLR, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer55->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer55->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_CoroPictureScaled = new wxCheckBox(PanelPapagayo, ID_CHECKBOX_CoroPictureScaled, _("Scaled pictures"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_CoroPictureScaled"));
    CheckBox_CoroPictureScaled->SetValue(false);
    FlexGridSizer55->Add(CheckBox_CoroPictureScaled, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer56->Add(FlexGridSizer55, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText67 = new wxStaticText(PanelPapagayo, ID_STATICTEXT70, _("5)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT70"));
    wxFont StaticText67Font(14,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText67->SetFont(StaticText67Font);
    FlexGridSizer56->Add(StaticText67, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonStartPapagayo = new wxButton(PanelPapagayo, ID_BUTTON6, _("Click to Create new Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    wxFont ButtonStartPapagayoFont(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    ButtonStartPapagayo->SetFont(ButtonStartPapagayoFont);
    FlexGridSizer56->Add(ButtonStartPapagayo, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer43->Add(FlexGridSizer56, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizerPapagayo->Add(FlexGridSizer43, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    PanelPapagayo->SetSizer(FlexGridSizerPapagayo);
    FlexGridSizerPapagayo->Fit(PanelPapagayo);
    FlexGridSizerPapagayo->SetSizeHints(PanelPapagayo);
    PanelSequencer = new wxPanel(Notebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    m_mgr = new wxAuiManager(PanelSequencer, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_ALLOW_ACTIVE_PANE|wxAUI_MGR_DEFAULT);
    Notebook1->AddPage(PanelSetup, _("Setup"), true);
    Notebook1->AddPage(PanelTest, _("Test"));
    Notebook1->AddPage(PanelConvert, _("Convert"));
    Notebook1->AddPage(PanelPreview, _("Preview"));
    Notebook1->AddPage(PaneNutcracker, _("Nutcracker"));
    Notebook1->AddPage(PanelCal, _("Schedule"));
    Notebook1->AddPage(PanelPapagayo, _("Papagayo"));
    Notebook1->AddPage(PanelSequencer, _("Sequencer"));
    MainAuiManager->AddPane(Notebook1, wxAuiPaneInfo().Name(_T("MainPain")).CenterPane().Caption(_("Pane caption")).Floatable().PaneBorder(false));
    MainAuiManager->Update();
    MenuBar1 = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItem3 = new wxMenu();
    MenuItem6 = new wxMenuItem(MenuItem3, ID_NEW_Non_Music_Seq, _("Music Sequence"), wxEmptyString, wxITEM_NORMAL);
    MenuItem6->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NEW")),wxART_OTHER));
    MenuItem3->Append(MenuItem6);
    MenuItem11 = new wxMenuItem(MenuItem3, ID_MENUITEM3, _("Animation Sequence"), wxEmptyString, wxITEM_NORMAL);
    MenuItem11->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NEW")),wxART_OTHER));
    MenuItem3->Append(MenuItem11);
    MenuFile->Append(ID_New_Music_Seq, _("New Sequence"), MenuItem3, wxEmptyString);
    MenuItem10 = new wxMenuItem(MenuFile, ID_OPEN_SEQUENCE, _("Open Sequence\tCtl-o"), wxEmptyString, wxITEM_NORMAL);
    MenuItem10->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_OTHER));
    MenuFile->Append(MenuItem10);
    MenuItem7 = new wxMenuItem(MenuFile, IS_SAVE_SEQ, _("Save Sequence\tCTL-S"), wxEmptyString, wxITEM_NORMAL);
    MenuItem7->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE")),wxART_OTHER));
    MenuFile->Append(MenuItem7);
    MenuFile->AppendSeparator();
    MenuItem5 = new wxMenuItem(MenuFile, ID_MENUITEM2, _("Select Show Folder\tF9"), wxEmptyString, wxITEM_NORMAL);
    MenuItem5->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER")),wxART_OTHER));
    MenuFile->Append(MenuItem5);
    MenuItemBackup = new wxMenuItem(MenuFile, ID_FILE_BACKUP, _("Backup\tF10"), wxEmptyString, wxITEM_NORMAL);
    MenuItemBackup->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_HARDDISK")),wxART_OTHER));
    MenuFile->Append(MenuItemBackup);
    MenuItem1 = new wxMenuItem(MenuFile, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    MenuItem1->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_QUIT")),wxART_OTHER));
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
    Menu2 = new wxMenu();
    MenuItem8 = new wxMenuItem(Menu2, ID_EXPORT_ALL, _("Export All"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem8);
    MenuItem9 = new wxMenuItem(Menu2, ID_EXPORT_MODEL, _("Export Model"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem9);
    MenuBar1->Append(Menu2, _("&Export"));
    Menu1 = new wxMenu();
    ThreadedSaveMenuItem = new wxMenuItem(Menu1, ID_NO_THREADED_SAVE, _("Disable Threaded Save"), wxEmptyString, wxITEM_CHECK);
    Menu1->Append(ThreadedSaveMenuItem);
    MenuBar1->Append(Menu1, _("&Settings"));
    MenuHelp = new wxMenu();
    MenuItem4 = new wxMenuItem(MenuHelp, idMenuHelpContent, _("Content\tF1"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem4);
    MenuItem2 = new wxMenuItem(MenuHelp, idMenuAbout, _("About"), _("Show info about this application"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem2);
    MenuBar1->Append(MenuHelp, _("&Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -50, -35 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    DirDialog1 = new wxDirDialog(this, _("Select directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    Timer1.SetOwner(this, ID_TIMER1);
    FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("xLights Sequences(*.xseq)|*.xseq|\n\n\t\t\tLOR Music Sequences (*.lms)|*.lms|\n\n\t\t\tLOR Animation Sequences (*.las)|*.las|\n\n\t\t\tVixen Sequences (*.vix)|*.vix|\n\n\t\t\tFalcon Pi Player Sequences (*.fseq)|*.fseq|\n\n\t\t\tGlediator Record File (*.gled)|*.gled)|\n\n\t\t\tLynx Conductor Sequences (*.seq)|*.seq|\n\n\t\t\tHLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|\n\n\t\t\txLights 3 XML(*.xml)|*.xml"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    MessageDialog1 = new wxMessageDialog(this, _("Hello"), _("Message"), wxOK|wxCANCEL, wxDefaultPosition);
    FileDialogPgoImage = new wxFileDialog(this, _("Select phoneme image file"), wxEmptyString, wxEmptyString, _("jpeg image(*.jpg)|*.jpg|\npng image(*.png)|*.png"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    DirDialog1 = new wxDirDialog(this, _("Select directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    Timer1.SetOwner(this, ID_TIMER1);
    FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("xLights Sequences(*.xseq)|*.xseq|\n\n\t\t\tLOR Music Sequences (*.lms)|*.lms|\n\n\t\t\tLOR Animation Sequences (*.las)|*.las|\n\n\t\t\tVixen Sequences (*.vix)|*.vix|\n\n\t\t\tFalcon Pi Player Sequences (*.fseq)|*.fseq|\n\n\t\t\tGlediator Record File (*.gled)|*.gled)|\n\n\t\t\tLynx Conductor Sequences (*.seq)|*.seq|\n\n\t\t\tHLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|\n\n\t\t\txLights 3 XML(*.xml)|*.xml"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    MessageDialog1 = new wxMessageDialog(this, _("Hello"), _("Message"), wxOK|wxCANCEL, wxDefaultPosition);
    FileDialogPgoImage = new wxFileDialog(this, _("Select phoneme image file"), wxEmptyString, wxEmptyString, _("jpeg image(*.jpg)|*.jpg|\npng image(*.png)|*.png"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));

    Connect(ID_AUITOOLBAR_OPENSHOW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_AUITOOLBAR_NEWSEQUENCE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_AUITOOLBAR_OPEN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_AUITOOLBAR_SAVE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_AUITOOLBAR_SAVEAS,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonClickSaveAs);
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
    Connect(ID_BITMAPBUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonPlayPreviewClick);
    Connect(ID_BITMAPBUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStopPreviewClick);
    Connect(ID_SLIDER_PREVIEW_TIME,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewTimeCmdScrollThumbTrack);
    Connect(ID_SLIDER_PREVIEW_TIME,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewTimeCmdScrollThumbRelease);
    Connect(ID_SLIDER_PREVIEW_TIME,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewTimeCmdSliderUpdated);
    Connect(ID_BUTTON_SET_PREVIEW_SIZE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSetPreviewSizeClick);
    Connect(ID_BUTTON_SET_BACKGROUND_IMAGE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSetBackgroundImageClick);
    Connect(ID_SLIDER_BACKGROUND_BRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated);
    Connect(ID_BUTTON_SELECT_MODEL_GROUPS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSelectModelGroupsClick);
    Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnListBoxElementListSelect);
    Connect(ID_BUTTON_MODELS_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonModelsPreviewClick);
    Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSavePreviewClick);
    Connect(ID_BUTTON_BUILD_WHOLEHOUSE_MODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonBuildWholeHouseModelClick);
    Connect(ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlPreviewElementSizeText);
    Connect(ID_SLIDER_PREVIEW_SCALE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlModelRotationDegreesText);
    Connect(ID_SLIDER_PREVIEW_ROTATE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewRotateCmdSliderUpdated);
    ScrolledWindowPreview->Connect(wxEVT_PAINT,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewPaint,0,this);
    ScrolledWindowPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftDown,0,this);
    ScrolledWindowPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftUp,0,this);
    ScrolledWindowPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewRightDown,0,this);
    ScrolledWindowPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseMove,0,this);
    ScrolledWindowPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseLeave,0,this);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayEffectClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_UpdateGridClick);
    Connect(ID_BUTTON58,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ModelsClick);
    Connect(ID_BUTTON59,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PresetsClick);
    Connect(ID_BUTTON_Palette,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PaletteClick);
    Connect(ID_BUTTON_PLAY_RGB_SEQ,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayAllClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDisplayElementsClick);
    Connect(ID_BUTTON_SeqExport,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSeqExportClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonModelExportClick);
    Connect(ID_BUTTON_CREATE_RANDOM,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnbtRandomEffectClick);
    Connect(ID_BUTTON_CollapseEffectsButton,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCollapseEffectsButtonClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_BITMAPBUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonInsertRowClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonDeleteRowClick);
    Connect(ID_BITMAPBUTTON_GRID_CUT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonGridCutClick);
    Connect(ID_BITMAPBUTTON_GRID_COPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonGridCopyClick);
    Connect(ID_BITMAPBUTTON_GRID_PASTE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonGridPasteClick);
    Connect(ID_CHOICE_VIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_ViewsSelect);
    Connect(ID_BT_EDIT_VIEWS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnbtEditViewsClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellRightClick);
    Connect(ID_GRID1,wxEVT_GRID_LABEL_RIGHT_CLICK,(wxObjectEventFunction)&xLightsFrame::OnGrid1LabelRightClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellChange);
    Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellLeftClick);
    Grid1->Connect(wxEVT_SET_FOCUS,(wxObjectEventFunction)&xLightsFrame::OnGrid1SetFocus,0,this);
    Grid1->Connect(wxEVT_KILL_FOCUS,(wxObjectEventFunction)&xLightsFrame::OnGrid1KillFocus,0,this);
    EffectsPanel1_1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&xLightsFrame::OnEffectsPanel1Paint,0,this);
    Connect(ID_CHECKBOX_RUN_SCHEDULE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBoxRunScheduleClick);
    Connect(ID_BUTTON_SAVE_SCHEDULE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveScheduleClick);
    Connect(ID_BUTTON_ADD_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddShowClick);
    Connect(ID_BUTTON_UPDATE_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonUpdateShowClick);
    Connect(ID_BUTTON_DELETE_SHOW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDeleteShowClick);
    Connect(ID_BUTTON_SHOW_DATES_CHANGE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonShowDatesChangeClick);
    Connect(ID_BUTTON_CLEARLOG,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonClearLogClick);
    Connect(ID_BUTTON_SAVELOG,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveLogClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_pgo_filenameClick);
    Connect(ID_BUTTON_PgoStitch,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PgoStitchClick);
    Connect(ID_BUTTON22,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_papagayo_output_sequenceClick1);
    Connect(ID_CHOICE_PgoOutputType,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_PgoOutputTypeSelect);
    Connect(ID_CHOICE_PgoGroupName,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_PgoGroupNameSelect);
    Connect(ID_BITMAPBUTTON_SaveCoroGroup,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButton_SaveCoroGroupClick);
    Connect(ID_BUTTON_CoroGroupDelete,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_CoroGroupDeleteClick);
    Connect(ID_BUTTON_CoroGroupClear,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_CoroGroupClearClick);
    Connect(ID_BUTTON_PgoCopyVoices,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PgoCopyVoicesClick);
    Connect(ID_GRID_COROFACES,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&xLightsFrame::OnGridCoroFacesLabelLeftClick);
    Connect(ID_GRID_COROFACES,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&xLightsFrame::OnGridCoroFacesCellSelect);
    Connect(ID_TEXTCTRL_PgoMinRest,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrl_PgoMinRestText);
    Connect(ID_TEXTCTRL_PgoMaxRest,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrl_PgoMaxRestText);
    Connect(ID_TEXTCTRL_PgoAutoFade,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrl_PgoAutoFadeText);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStartPapagayoClick);
    PanelSequencer->Connect(wxEVT_PAINT,(wxObjectEventFunction)&xLightsFrame::OnPanelSequencerPaint,0,this);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged1);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_FILE_BACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBackupSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnQuit);
    Connect(idMenuSaveSched,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemSavePlaylistsSelected);
    Connect(idMenuAddList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemAddListSelected);
    Connect(idMenuRenameList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenameListSelected);
    Connect(idMenuDelList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemDelListSelected);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRefreshSelected);
    Connect(idCustomScript,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemCustomScriptSelected);
    Connect(ID_NO_THREADED_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ToggleThreadedSave);
    Connect(idMenuHelpContent,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnTimer1Trigger);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnTimer1Trigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnClose);
    //*)

    CreateSequencer();

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};
    seqPreview = new SequencePreview( (wxPanel*) SeqPanelLeft, args);
    BoxSizerSequencePreview->Add(seqPreview, 1, wxEXPAND);
    modelPreview = new ModelPreview( (wxPanel*) ScrolledWindowPreview, args);
    BoxSizerModelsPreview->Add(modelPreview, 1, wxEXPAND);

    playIcon = wxBitmap(control_play_blue_icon);
    pauseIcon = wxBitmap(control_pause_blue_icon);

    Grid1HasFocus = false; //set this before grid gets any events -DJ

    SetIcon(wxIcon(xlights_xpm));
    SetName("xLights");
    wxPersistenceManager::Get().RegisterAndRestore(this);
    wxConfigBase* config = wxConfigBase::Get();

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

    itemCol.SetText(_T("Num Channels"));
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    GridNetwork->InsertColumn(3, itemCol);

    itemCol.SetText(_T("xLights/Vixen/FPP Mapping"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(4, itemCol);

    GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(1,100);
    GridNetwork->SetColumnWidth(2,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(3,100);
    GridNetwork->SetColumnWidth(4,170);

    Grid1->SetColFormatFloat(0,7,3);

    // get list of most recently used directories
    wxString dir,mru_name;
    int menuID, idx;
    for (int i=0; i<MRU_LENGTH; i++)
    {
        mru_name=wxString::Format("mru%d",i);
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

    threadedSave = config->ReadBool("ThreadedSave", true);
    ThreadedSaveMenuItem->Check(!threadedSave);

    // initialize all effect wxChoice lists

    BarEffectDirections.Add("up");          // 0
    BarEffectDirections.Add("down");        // 1
    BarEffectDirections.Add("expand");      // 2
    BarEffectDirections.Add("compress");    // 3
    BarEffectDirections.Add("Left");        // 4
    BarEffectDirections.Add("Right");       // 5
    BarEffectDirections.Add("H-expand");    // 6
    BarEffectDirections.Add("H-compress");  // 7
    BarEffectDirections.Add("Alternate Up");// 8
    BarEffectDirections.Add("Alternate Down");  // 9
    BarEffectDirections.Add("Alternate Left");  // 10
    BarEffectDirections.Add("Alternate Right"); // 11


    ButterflyEffectColors.Add("Rainbow");
    ButterflyEffectColors.Add("Palette");
    ButterflyDirection.Add("Normal");
    ButterflyDirection.Add("Reverse");

    FacesPhoneme.Add("AI");
    FacesPhoneme.Add("E");
    FacesPhoneme.Add("FV");
    FacesPhoneme.Add("L");
    FacesPhoneme.Add("MBP");
    FacesPhoneme.Add("O");
    FacesPhoneme.Add("U");
    FacesPhoneme.Add("WQ");
    FacesPhoneme.Add("etc");
    FacesPhoneme.Add("rest");

    CoroFacesPhoneme.Add("AI");
    CoroFacesPhoneme.Add("E");
    CoroFacesPhoneme.Add("FV");
    CoroFacesPhoneme.Add("L");
    CoroFacesPhoneme.Add("MBP");
    CoroFacesPhoneme.Add("O");
    CoroFacesPhoneme.Add("U");
    CoroFacesPhoneme.Add("WQ");
    CoroFacesPhoneme.Add("etc");
    CoroFacesPhoneme.Add("rest");
    CoroFacesPhoneme.Add("(off)"); //allow turn off mouth (ie, eyes only)
    if (xLightsApp::WantDebug) CoroFacesPhoneme.Add("(test)"); //debug/test

    WaveType.Add("Sine");
    WaveType.Add("Triangle");
    WaveType.Add("Square");
    WaveType.Add("Decaying Sine");
    WaveType.Add("Fractal/ivy");
    FillColors.Add("None");
    FillColors.Add("Rainbow");
    FillColors.Add("Palette");
    WaveDirection.Add("Right to Left");
    WaveDirection.Add("Left to Right");



    MeteorsEffectTypes.Add("Rainbow");
    MeteorsEffectTypes.Add("Range");
    MeteorsEffectTypes.Add("Palette");

    MeteorsEffect.Add("Down");
    MeteorsEffect.Add("Up");
    MeteorsEffect.Add("Left");
    MeteorsEffect.Add("Right");
    MeteorsEffect.Add("Implode");
    MeteorsEffect.Add("Explode");
    MeteorsEffect.Add("Icicles"); //random length drip effect -DJ
    MeteorsEffect.Add("Icicles + bkg"); //with bkg (dim) icicles -DJ

    RippleMovement.Add("Explode");
    RippleMovement.Add("Implode");
    RippleObjectToDraw.Add("Circle");
    RippleObjectToDraw.Add("Square");
    RippleObjectToDraw.Add("Triangle");


    TextEffectDirections.Add("left"); //0
    TextEffectDirections.Add("right"); //1
    TextEffectDirections.Add("up"); //2
    TextEffectDirections.Add("down"); //3
    TextEffectDirections.Add("none"); //4
    TextEffectDirections.Add("up-left"); //5
    TextEffectDirections.Add("down-left"); //6
    TextEffectDirections.Add("up-right"); //7
    TextEffectDirections.Add("down-right"); //8
    TextEffectDirections.Add("wavey L-R/up-down"); //9
//    TextEffectDirections.Add("bouncey L-R/up-down"); //TODO
//TODO: should animation movement be factored out and applied to any effect? (at least the line-based effects, probably not for fills) -DJ

    PictureEffectDirections.Add("left"); //0
    PictureEffectDirections.Add("right"); //1
    PictureEffectDirections.Add("up"); //2
    PictureEffectDirections.Add("down"); //3
    PictureEffectDirections.Add("none"); //4
    PictureEffectDirections.Add("up-left"); //5
    PictureEffectDirections.Add("down-left"); //6
    PictureEffectDirections.Add("up-right"); //7
    PictureEffectDirections.Add("down-right"); //8
    PictureEffectDirections.Add("scaled"); //9; for when image size does not match model size -DJ
    PictureEffectDirections.Add("peekaboo"); //10; up+down 1x for hippo peekaboo -DJ
    PictureEffectDirections.Add("wiggle"); //11; move back+forth a little -DJ
    PictureEffectDirections.Add("zoom in"); //12; fast ~= explode -DJ
    PictureEffectDirections.Add("peekaboo 90"); //13; horizontal peekaboo -DJ
    PictureEffectDirections.Add("peekaboo 180"); //14; upside down peekaboo -DJ
    PictureEffectDirections.Add("peekaboo 270"); //15; etc -DJ
    PictureEffectDirections.Add("vix 2 routine"); //16; animated csv or sdv file of pixel values from Vixen 2.x -DJ
    PictureEffectDirections.Add("flag wave"); //17; flag waving in wind -DJ
    PictureEffectDirections.Add("up once"); //18
    PictureEffectDirections.Add("down once"); //19


//  remember to godown to around line 1800 to active ate these add's

//read from choice list instead of hard-coded duplication: -DJ
//    PianoEffectStyles.Add("Color Organ");
//    PianoEffectStyles.Add("Equalizer (bars)");
//    PianoEffectStyles.Add("Keyboard");
//    PianoEffectStyles.Add("Player Piano (scroll)");
//    PianoEffectStyles.Add("RGB Icicles (drip)");
    for (int i = 0; i < EffectsPanel1->Choice_Piano_Style->GetCount(); ++i)
        PianoEffectStyles.Add(EffectsPanel1->Choice_Piano_Style->GetString(i));

//    PianoKeyPlacement.Add("Tile");
//    PianoKeyPlacement.Add("Stretch/shrink");
//    PianoKeyPlacement.Add("Top left");
//    PianoKeyPlacement.Add("Top center");
//    PianoKeyPlacement.Add("Top right");
//    PianoKeyPlacement.Add("Middle left");
//    PianoKeyPlacement.Add("Middle center");
//    PianoKeyPlacement.Add("Middle right");
//    PianoKeyPlacement.Add("Bottom left");
//    PianoKeyPlacement.Add("Bottom middle");
//    PianoKeyPlacement.Add("Bottom right");
    for (int i = 0; i < EffectsPanel1->Choice_Piano_KeyPlacement->GetCount(); ++i)
        PianoKeyPlacement.Add(EffectsPanel1->Choice_Piano_KeyPlacement->GetString(i));

//pre-set Piano style, shapes + map files:
//    wxString mydir;
//    wxFileName::SplitPath(::wxStandardPaths::Get().GetExecutablePath(), NULL, &mydir, NULL, NULL);
    wxFileName myfile(::wxStandardPaths::Get().GetExecutablePath()); //start in folder with .EXXE (there is no "bin" subfolder after an install)
//    myfile.RemoveLastDir();

    myfile.AppendDir("piano"); //piano files moved to separate subfolder

    myfile.SetName("Piano-88KeyShapeMap");
    myfile.SetExt("txt");
    EffectsPanel1->TextCtrl_Piano_MapFilename->SetValue(myfile.GetFullPath());
//~    EffectsPanel2->TextCtrl_Piano_MapFilename->SetValue(myfile.GetFullPath());
    myfile.SetName("Piano-ExampleKeyTopShapes");
    myfile.SetExt("png");
    EffectsPanel1->TextCtrl_Piano_ShapeFilename->SetValue(myfile.GetFullPath());
//~    EffectsPanel2->TextCtrl_Piano_ShapeFilename->SetValue(myfile.GetFullPath());
    EffectsPanel1->Choice_Piano_Style->SetSelection(2); //keyboard
//~    EffectsPanel2->Choice_Piano_Style->SetSelection(2); //keyboard

    //  single strand
//    ButterflyEffectColors.Add("Rainbow");
//    ButterflyEffectColors.Add("Palette");
    SingleStrandColors.Add("Rainbow");
    SingleStrandColors.Add("Palette");
    SingleStrandTypes.Add("Left-Right");  // 0
    SingleStrandTypes.Add("Right-Left");  // 1
    SingleStrandTypes.Add("Auto reverse");  // 2
    SingleStrandTypes.Add("Bounce");  //3
    SingleStrandTypes.Add("Pacman");  //3

    TextEffects.Add("normal");
    TextEffects.Add("vert text up");
    TextEffects.Add("vert text down");
    TextEffects.Add("rotate up 45");
    TextEffects.Add("rotate up 90");
    TextEffects.Add("rotate down 45");
    TextEffects.Add("rotate down 90");

    TextCountDown.Add("none");
    TextCountDown.Add("seconds");
    TextCountDown.Add("to date 'd h m s'");
    TextCountDown.Add("to date 'h:m:s'"); //for smaller grids -DJ
    TextCountDown.Add("to date 'm' or 's'"); //-DJ
    TextCountDown.Add("to date 's'"); //for smallest grids -DJ
    TextCountDown.Add("!to date!%fmt"); //free fmt countdown -DJ

    InitEffectsPanel(EffectsPanel1);
//~    InitEffectsPanel(EffectsPanel2);

    CurtainEdge=EffectsPanel1->Choice_Curtain_Edge->GetStrings();
    CurtainEffect=EffectsPanel1->Choice_Curtain_Effect->GetStrings();

    // Check if schedule should be running
    xout=0;
    long RunFlag=0;

    config->Read(_("RunSchedule"), &RunFlag);
    //delete config;  // close config before calling SetDir, which will open config
    if (RunFlag && xLightsApp::RunPrompt) //give user a chance to edit before running -DJ
        if (wxMessageBox("Auto-run schedule?", "Confirm", wxYES_DEFAULT | wxYES_NO) != wxYES) RunFlag = 0; //, main_frame);

    SetPlayMode(play_off);
    ResetEffectsXml();
    EnableSequenceControls(true);
    UpdateShowDates(wxDateTime::Now(),wxDateTime::Now());
    if (ok && !dir.IsEmpty())
    {
        SetDir(dir);
    }
    EffectsPanel1->PaletteChanged=true;
//~    EffectsPanel2->PaletteChanged=true;
    MixTypeChanged=true;
    HtmlEasyPrint=new wxHtmlEasyPrinting("xLights Printing", this);
    basic.setFrame(this);
    PlayerDlg = new PlayerFrame(this, ID_PLAYER_DIALOG);

    EffectNames=EffectsPanel1->Choicebook1->GetChoiceCtrl()->GetStrings();
//~    EffectLayerOptions=Choice_LayerMethod->GetStrings();

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

//    ConnectOnChar(PaneNutcracker);
//    ConnectOnChar(Panel1); //add hot keys to upper panel as well -DJ
}

xLightsFrame::~xLightsFrame()
{
    //must call these or the app will crash on exit
    m_mgr->UnInit();
    MainAuiManager->UnInit();

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
    panel->Choice_Butterfly_Direction->Set(ButterflyDirection);
    panel->Choice_Butterfly_Direction->SetSelection(0);

    panel->Choice_Wave_Type->Set(WaveType);
    panel->Choice_Wave_Type->SetSelection(0);
    panel->Choice_Fill_Colors->Set(FillColors);
    panel->Choice_Fill_Colors->SetSelection(0);
    panel->Choice_Wave_Direction->Set(WaveDirection);
    panel->Choice_Wave_Direction->SetSelection(0);


    panel->Choice_Meteors_Type->Set(MeteorsEffectTypes);
    panel->Choice_Meteors_Type->SetSelection(0);
    panel->Choice_Meteors_Effect->Set(MeteorsEffect);
    panel->Choice_Meteors_Effect->SetSelection(0);

    panel->Choice_Pictures_Direction->Set(PictureEffectDirections);
    panel->Choice_Pictures_Direction->SetSelection(0);

    panel->Choice_Text_Dir1->Set(TextEffectDirections);
    panel->Choice_Text_Dir1->SetSelection(0);
    panel->Choice_Text_Dir2->Set(TextEffectDirections);
    panel->Choice_Text_Dir2->SetSelection(0);
    panel->Choice_Text_Dir3->Set(TextEffectDirections);
    panel->Choice_Text_Dir3->SetSelection(0);
    panel->Choice_Text_Dir4->Set(TextEffectDirections);
    panel->Choice_Text_Dir4->SetSelection(0);

    panel->Choice_Text_Effect1->Set(TextEffects);
    panel->Choice_Text_Effect1->SetSelection(0);
    panel->Choice_Text_Effect2->Set(TextEffects);
    panel->Choice_Text_Effect2->SetSelection(0);
    panel->Choice_Text_Effect3->Set(TextEffects);
    panel->Choice_Text_Effect3->SetSelection(0);
    panel->Choice_Text_Effect4->Set(TextEffects);
    panel->Choice_Text_Effect4->SetSelection(0);

    panel->Choice_Text_Count1->Set(TextCountDown);
    panel->Choice_Text_Count1->SetSelection(0);
    panel->Choice_Text_Count2->Set(TextCountDown);
    panel->Choice_Text_Count2->SetSelection(0);
    panel->Choice_Text_Count3->Set(TextCountDown);
    panel->Choice_Text_Count3->SetSelection(0);
    panel->Choice_Text_Count4->Set(TextCountDown);
    panel->Choice_Text_Count4->SetSelection(0);

    panel->CurrentDir = &CurrentDir;
    panel->Choice_SingleStrand_Colors->Set(SingleStrandColors);
    panel->Choice_SingleStrand_Colors->SetSelection(1); // Set Rainbow as default
    panel->Choice_Chase_Type1->Set(SingleStrandTypes);
    panel->Choice_Chase_Type1->SetSelection(1); // Set R-L as default

    panel->Choice_Faces_Phoneme->Set(FacesPhoneme);
    panel->Choice_Faces_Phoneme->SetSelection(0);

    panel->Choice_CoroFaces_Phoneme->Set(CoroFacesPhoneme);
    panel->Choice_CoroFaces_Phoneme->SetSelection(0);

    panel->Choice_Ripple_Movement->Set(RippleMovement);
    panel->Choice_Ripple_Movement->SetSelection(0);
    panel->Choice_Ripple_Object_To_Draw->Set(RippleObjectToDraw);
    panel->Choice_Ripple_Object_To_Draw->SetSelection(0);


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
    default:
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
    case PREVIEWTAB:
        caption=_("Preview Tab");
        msg=_("Create display elements by clicking on the Models button. Only models that have 'My Display' checked will be included in the Display Elements list and shown in the preview area.\n\nSelect an item in the Display Elements list and it will turn from gray to yellow (you may not see the yellow if the selected element is hidden behind another one). Once selected, you can drag your cursor across the preview area to move the element. You can also use the Element Size slider to make it bigger or smaller. You can rotate elements that have Display As set to 'Single Line'. Don't forget to click the Save button to save your preview!\n\nClick the Open button to select an xLights sequence to be previewed. Note that any xLights sequence can be previewed, not just those created on the Nutcracker tab. Click Play to start preview playback. Use the Pause button to stop play, and then the Play button to resume. You can drag the slider that appears across the top of the preview area to move playback to any spot in your sequence. The Stop Now button in the upper left will also stop playback.");
        break;
    case SEQUENCETAB:
        caption=_("Nutcracker Tab");
        msg=_("The Nutcracker tab can be used to create RGB sequences. First, create a model of your RGB display element(s) by clicking on the Models button. Then try the different effects and settings until you create something you like. You can save the settings as a preset by clicking the New Preset button. From then on, that preset will be available in the presets drop-down list. You can combine effects by creating a second effect in the Effect 2 area, then choosing a Layering Method. To create a series of effects that will be used in a sequence, click the open file icon to open an xLights (.xseq) sequence. Choose which display elements/models you will use in this sequence. Then click the insert rows icon and type in the start time in seconds when that effect should begin. Rows will automatically sort by start time. To add an effect to the sequence, click on the grid cell in the desired display model column and the desired start time row, then click the Update button. When you are done creating effects for the sequence, click the save icon and the xLights sequence will be updated with the effects you stored in the grid.");
        break;
    case PAPAGAYOTAB:
        caption=_("Papagayo Tab");
        msg=_("The Papagayo tab can be used to create animated faces from a Papagayo file. There are four different types faces that you can create. \n1) Automatic, scaled faces for matrix and megatrees. \n2) You provide 10 images of the Phonemes, they will be selected for you. \n3) You provide moth , eyes and a backgroudn image. A mp4 movie will be created. \n4) You assign channels for each of the 10 Phonemes. This method matches standard coro singing faces ");
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

void xLightsFrame::OnNotebook1PageChanged1(wxAuiNotebookEvent& event)
{
    heartbeat("tab change", true); //tell fido to stop watching -DJ
    int pagenum=event.GetSelection(); //Notebook1->GetSelection();
    if (pagenum == TESTTAB && !xout)
    {
        StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
    }
    else if (pagenum == SEQUENCETAB)
    {
        seqPreview->InitializePreview();
    }
    else if (pagenum == PREVIEWTAB)
    {
        modelPreview->InitializePreview(mBackgroundImage,mBackgroundBrightness);
    }
    else if (pagenum == NEWSEQUENCER)
    {
        InitSequencer();
    }
    else
    {
        StatusBar1->SetStatusText(_(""));
    }

    if (pagenum == PREVIEWTAB)
    {
        UpdatePreview();
        if (SeqPlayerState != NO_SEQ) {
            if(!mediaFilename.IsEmpty())
            {
                StopPreviewPlayback(); //FR. If we have sequence data loaded make sure that media playback is inproper state when returnign to preview tab.
                ResetTimer(PAUSE_SEQ);
            }
            else if (SeqDataLen > 0)
            {
                StopPreviewPlayback();
                ResetTimer(PAUSE_SEQ_ANIM,0);
            }
        }
    }
    if (pagenum == PAPAGAYOTAB)
    {
        InitPapagayoTab(event.GetOldSelection() != PAPAGAYOTAB); //populate choice lists with model names, etc.
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
            if (tagname == "network")
            {
                wxString tempstr=e->GetAttribute("MaxChannels", "0");
                tempstr.ToLong(&MaxChan);
                wxString NetworkType=e->GetAttribute("NetworkType", "");
                wxString ComPort=e->GetAttribute("ComPort", "");
                wxString BaudRate=e->GetAttribute("BaudRate", "");
                int baud = (BaudRate == _("n/a")) ? 115200 : wxAtoi(BaudRate);
                static wxString choices;

#ifdef __WXMSW__ //TODO: enumerate comm ports on all platforms -DJ
                TCHAR valname[32];
                /*byte*/TCHAR portname[32];
                DWORD vallen = sizeof(valname);
                DWORD portlen = sizeof(portname);
                HKEY hkey = NULL;
                DWORD err = 0;

//enum serial comm ports (more user friendly, especially if USB-to-serial ports change):
//logic based on http://www.cplusplus.com/forum/windows/73821/
                if (choices.empty()) //should this be cached?  it's not really that expensive
                {
                    if (!(err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hkey)))
                        for (DWORD inx = 0; !(err = RegEnumValue(hkey, inx, (LPTSTR)valname, &vallen, NULL, NULL, (LPBYTE)portname, &portlen)) || (err == ERROR_MORE_DATA); ++inx)
                        {
                            if (err == ERROR_MORE_DATA) portname[sizeof(portname)/sizeof(portname[0]) - 1] = '\0'; //need to enlarge read buf if this happens; just truncate string for now
//                            debug(3, "found port[%d] %d:'%s' = %d:'%s', err 0x%x", inx, vallen, valname, portlen, portname, err);
                            choices += _(", ") + portname;
                            vallen = sizeof(valname);
                            portlen = sizeof(portname);
                        }
                    if (err && (err != /*ERROR_FILE_NOT_FOUND*/ ERROR_NO_MORE_ITEMS)) choices = wxString::Format(", error %d (can't get serial comm ports from registry)", err);
                    if (hkey) RegCloseKey(hkey);
//                    if (err) SetLastError(err); //tell caller about last real error
                    if (!choices.empty()) choices = "\n(available ports: "+ choices.substr(2) + ")";
                    else choices = "\n(no available ports)";
                }
#endif // __WXMSW__
                wxString msg = _("Error occurred while connecting to ") + NetworkType+ _(" network on ") + ComPort +
                               choices +
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
                    if (wxMessageBox(msg + errmsg + _("\nProceed anyway?"), _("Communication Error"), wxYES_NO | wxNO_DEFAULT) != wxYES)
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


//factored out from below so it can be reused by play/pause button -DJ
void xLightsFrame::StopNow(void)
{
    int actTab = Notebook1->GetSelection();
    if (actTab == PREVIEWTAB)
    {
        StopPreviewPlayback();
        return;
    }
    PlayerDlg->MediaCtrl->Stop();
    if (play_mode == play_sched)
    {
        CheckBoxRunSchedule->SetValue(false);
        CheckRunSchedule();
    }
    heartbeat("playback end", true); //tell fido to stop watching -DJ
    if (basic.IsRunning()) basic.halt();
    SetPlayMode(play_off);
    ResetTimer(NO_SEQ);
    switch (actTab)
    {
    case TESTTAB:
        TestButtonsOff();
        break;
    case SEQUENCETAB:
        EnableSequenceControls(true);
        break;
    }
    Button_PlayEffect->SetLabel(_("Play Effect (F3)")); //toggle label -DJ
    Button_PlayRgbSeq->SetLabel("Play (F4)");
}

void xLightsFrame::OnButtonStopNowClick(wxCommandEvent& event)
{
    StopNow();
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
        heartbeat("exit", true); //tell fido about graceful exit -DJ
    }
    else
    {
        wxMessageBox(_("Graceful Stop is only useful when a schedule or playlist is running"));
    }
}

//make these static so they can be accessed outside of xLightsFrame: -DJ
//NOTE: this assumes there will only be one xLightsMain object
wxString xLightsFrame::CurrentDir = "";
wxString xLightsFrame::PlaybackMarker = "";
wxString xLightsFrame::xlightsFilename = "";
std::vector<ModelClassPtr> xLightsFrame::PreviewModels, xLightsFrame::OtherModels;

void xLightsFrame::OnButtonSaveScheduleClick(wxCommandEvent& event)
{
    SaveScheduleFile();
}

void xLightsFrame::OnMenuItemSavePlaylistsSelected(wxCommandEvent& event)
{
    SaveScheduleFile();
}


#include "TabSchedule.cpp"

void xLightsFrame::OnClose(wxCloseEvent& event)
{
    if (UnsavedChanges && wxNO == wxMessageBox("Quit without saving?",
            "Unsaved Changes", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }

    heartbeat("exit", true); //tell fido about graceful exit -DJ
    // Disconnect the resize events, otherwise they get called as we are shutting down
    ScrolledWindowPreview->Disconnect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewResize,0,this);
    //ScrolledWindow1->Disconnect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindow1Resize,0,this);

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
    wxString newDirBackup = CurrentDir+wxFileName::GetPathSeparator()+"Backup";
    if (!wxDirExists(newDirBackup) && !newDirH.Mkdir(newDirBackup))
    {
        wxMessageBox("Unable to create directory Backup!","Error", wxICON_ERROR|wxOK);
        return;
    }

//if(curTime.ParseFormat("2003-xx-xx yy:yy", "%Y-%m-%d_%H%M%S"))

    wxString newDir = CurrentDir+wxFileName::GetPathSeparator()+wxString::Format(
                          "Backup%c%s-%s",wxFileName::GetPathSeparator(),
                          curTime.FormatISODate(),curTime.Format("%H%M%S"));

    if ( wxNO == wxMessageBox("All xml files under 10MB in your xlights directory will be backed up to \""+
                              newDir+"\". Proceed?","Backup",wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    if (!newDirH.Mkdir(newDir))
    {
        wxMessageBox("Unable to create directory!","Error", wxICON_ERROR|wxOK);
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

    bool cont = srcDir.GetFirst(&fname, "*.xml", wxDIR_FILES);

    while (cont)
    {
        srcFile.SetFullName(fname);

        wxULongLong fsize=srcFile.GetSize();
        if(fsize > 10*1024*1024) // skip any xml files > 10 mbytes, they are something other than xml files
        {
            srcDir.GetNext(&fname);
            continue;
        }
        StatusBar1->SetStatusText("Copying File \""+srcFile.GetFullPath());
        success = wxCopyFile(srcDirName+fname,
                             targetDirName+wxFileName::GetPathSeparator()+fname);
        if (!success)
        {
            wxMessageBox("Unable to copy file \"" + CurrentDir+wxFileName::GetPathSeparator()+fname+"\"",
                         "Error", wxICON_ERROR|wxOK);
        }
        cont = srcDir.GetNext(&fname);
    }
    StatusBar1->SetStatusText("All xml files backed up.");
}

#if 0 //removed
void xLightsFrame::ConnectOnChar(wxWindow* pclComponent)
{
    if(pclComponent)
    {
        pclComponent->Connect(wxID_ANY,
                              wxEVT_CHAR,
                              wxKeyEventHandler(xLightsFrame::OnPaneNutcrackerChar),
                              (wxObject*) NULL,
                              this);

        wxWindowListNode* pclNode = pclComponent->GetChildren().GetFirst();
        while(pclNode)
        {
            wxWindow* pclChild = pclNode->GetData();
            this->ConnectOnChar(pclChild);

            pclNode = pclNode->GetNext();
        }
    }
}
#endif //0


//need to return a "processed" flag here; refactor: -DJ
bool xLightsFrame::HotKey(wxKeyEvent& event)
{
    wxChar uc = event.GetKeyCode();
    bool retval = false;

    if (Notebook1->GetSelection() == SEQUENCETAB) //Nutcracker tab
    {
        retval = true;
        if (event.ControlDown())
        {
            switch (uc)
            {
            case 'o':
            case 'O':
                uc = WXK_CONTROL_O;
                break;
            case 's':
            case 'S':
                uc = WXK_CONTROL_S;
                break;
            case 'c':
            case 'C':
                uc = WXK_CONTROL_C;
                break;
            case 'v':
            case 'V':
                uc = WXK_CONTROL_V;
                break;
            case 'x':
            case 'X':
                uc = WXK_CONTROL_X;
                break;
            }
        }
        switch (uc)
        {
        case WXK_F4:
            if (Button_PlayRgbSeq->IsEnabled())
            {
                PlayRgbSequence();
            }
            else
            {
                StopNow();
            }
            break;
        case WXK_F3:
            switch (SeqPlayerState)
            {
            case PLAYING_EFFECT:
                StopNow();
                break;
            case STARTING_SEQ_ANIM:
            case PLAYING_SEQ_ANIM:
            case STARTING_SEQ:
            case PLAYING_SEQ:
                StopNow();
                PlayEffect();
                break;
            default:
                PlayEffect();
            }
            break;
        case WXK_CONTROL_C:
            if (Grid1HasFocus) CutOrCopyToClipboard(false);
            else retval = false; //allow other text controls to get the event -DJ
            break;
        case WXK_CONTROL_X:
            if (Grid1HasFocus) CutOrCopyToClipboard(true);
            else retval = false; //allow other text controls to get the event -DJ
            break;
        case WXK_CONTROL_V:
            if (Grid1HasFocus) PasteFromClipboard();
            else retval = false; //allow other text controls to get the event -DJ
            break;


        case WXK_F5:
            if (Button_UpdateGrid->IsEnabled())
            {
                UpdateGrid();
            }
            break;
        case WXK_CONTROL_O:
            if (BitmapButtonOpenSeq->IsEnabled())
            {
                OpenSequence();
            }
            break;
        case WXK_CONTROL_S:
            if (BitmapButtonSaveSeq->IsEnabled())
            {
                SaveSequence();
            }
            break;
        case WXK_INSERT:
            if (BitmapButtonInsertRow->IsEnabled())
            {
                InsertRow();
            }
            break;
        default:
            retval = false;
        }
    }
    else if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        switch(uc)
        {
            case '+':
                mainSequencer->PanelTimeLine->ZoomIn();
                break;
            case '-':
                mainSequencer->PanelTimeLine->ZoomOut();
                break;
            default:
                retval = false;
        }

    }
    if (!retval)
    {
        event.Skip();
    }
    return retval;
}

void xLightsFrame::OnPaneNutcrackerChar(wxKeyEvent& event)
{
    HotKey(event);
}

//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
//void djdebug(const char* fmt, ...); //_DJ
//typedef enum { Normal, Locked, Random } EditState;
void xLightsFrame::setlock(wxBitmapButton* button) //, EditState& islocked)
{
    wxString parent = button->GetName();
    if (parent.StartsWith("ID_BITMAPBUTTON_")) parent = "ID_" + parent.substr(16); //map to associated control
    EditState& islocked = buttonState[std::string(parent)]; //creates entry if not there
//    djdebug("ctl %s was %d", (const char*)parent.c_str(), islocked);
    switch (islocked) //cycle thru states
    {
    case Locked:
        islocked = Random;
        button->SetBitmapLabel(BitmapButton_random->GetBitmapLabel());
        break;
//        case Random:
//            islocked = Normal;
//            button->SetBitmapLabel(BitmapButton_normal->GetBitmapLabel());
//            break;
    default:
        islocked = Locked;
        button->SetBitmapLabel(BitmapButton_locked->GetBitmapLabel());
        break;
    }
}
//#define isRandom(ctl)  (buttonState[std::string(ctl->GetName())] == Random)
bool xLightsFrame::isRandom_(wxControl* ctl, const char*debug)
{
//    if (!ctl->GetName().length()) djdebug("NO NAME FOR %s", debug);
    bool retval = (buttonState[std::string(ctl->GetName())] != Locked); //== Random);
//    djdebug("isRandom(%s) = %d", (const char*)ctl->GetName().c_str(), retval);
    return retval;
}

#define showlock(name)  \
/*EditState isLockedFx_##name = Normal;*/ \
void xLightsFrame::OnBitmapButton_##name##Click(wxCommandEvent& event) \
{ \
    setlock(BitmapButton_##name/*, isLockedFx_##name*/); \
}
//#define showlock(name)  \
//EditState isLockedMain_##name = Normal; \
//void xLightsFrame::OnBitmapButton_##name##Click(wxCommandEvent& event) \
//{ \
//    setlock(BitmapButton_##name, BitmapButton_normal, BitmapButton_locked, BitmapButton_random, isLockedMain_##name); \
//}
//~showlock(EffectLayerMix)
//~showlock(SparkleFrequency)
//~showlock(Brightness)
//~showlock(Contrast)
#if 1   // <SCM>
//~showlock(CheckBox_LayerMorph)
#endif

/*void xLightsFrame::OnButtonModelExportClick(wxCommandEvent& event)
{
}
*/

void xLightsFrame::OnEffectsPanel1Paint(wxPaintEvent& event)
{
}

void xLightsFrame::OnGrid1SetFocus(wxFocusEvent& event)
{
    Grid1HasFocus = true;
}

void xLightsFrame::OnGrid1KillFocus(wxFocusEvent& event)
{
    Grid1HasFocus = false;
}


void xLightsFrame::OntxtCtrlSparkleFreqText(wxCommandEvent& event)
{
}


static void AddNonDupAttr(wxXmlNode* node, const wxString& name, const wxString& value)
{
    wxString junk;
    if (node->GetAttribute(name, &junk)) node->DeleteAttribute(name); //kludge: avoid dups
    if (!value.empty()) node->AddAttribute(name, value);
}

//sigh; a function like this should have been built into wxWidgets
wxXmlNode* xLightsFrame::FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create /*= false*/)
{
#if 0
    static struct
    {
        std::unordered_map<const char*, wxXmlNode*> nodes;
        std::string parent, child;
    } cached_names;

    if (parent->GetName() != cached_names.parent) //reload cache
    {
        cached_names.nodes.clear();
        for (wxXmlNode* node = parent->GetChildren(); node != NULL; node = node->GetNext())
            cached_names.nodes[node->GetName()] = node;
        cached_names.parent = parent;
    }
    if (cached_names.nodes.find(tag) == cached_names.nodes.end()) //not found
    {
        if (!create) return 0;
        parent->AddChild(cached_names.nodes[tag] = new wxXmlNode(wxXML_ELEMENT_NODE, tag));
    }
    return cached_names.nodes[tag];
#endif // 0
    for (wxXmlNode* node = parent->GetChildren(); node != NULL; node = node->GetNext())
    {
        if (!tag.empty() && (node->GetName() != tag)) continue;
        if (!value.empty() && (node->GetAttribute(attr) != value)) continue;
        return node;
    }
    if (!create) return 0; //CAUTION: this will give null ptr exc if caller does not check
    wxXmlNode* retnode = new wxXmlNode(wxXML_ELEMENT_NODE, tag); //NOTE: assumes !tag.empty()
    parent->AddChild(retnode);
    if (!value.empty()) AddNonDupAttr(retnode, attr, value);
    return retnode;
}
void xLightsFrame::OnButtonSetPreviewSizeClick(wxCommandEvent& event)
{
    int DlgResult;
    dlgPreviewSize dialog(this);
    dialog.TextCtrl_PreviewWidth->SetValue(wxString::Format("%d",modelPreview->getWidth()));
    dialog.TextCtrl_PreviewHeight->SetValue(wxString::Format("%d",modelPreview->getHeight()));
    dialog.CenterOnParent();
    DlgResult = dialog.ShowModal();
    if (DlgResult == wxID_OK)
    {
        if(!dialog.TextCtrl_PreviewWidth->IsEmpty() && !dialog.TextCtrl_PreviewHeight->IsEmpty())
        {
            int w = wxAtoi(dialog.TextCtrl_PreviewWidth->GetValue());
            int h = wxAtoi(dialog.TextCtrl_PreviewHeight->GetValue());
            if(w > 0 && h > 0)
            {
                SetPreviewSize(w,h);
            }
        }
    }
}
void xLightsFrame::SetPreviewSize(int width,int height)
{
    StaticTextCurrentPreviewSize->SetLabelText(wxString::Format("Size: %d x %d",width,height));
    SetXmlSetting("previewWidth",wxString::Format("%d",width));
    SetXmlSetting("previewHeight",wxString::Format("%d",height));
    SaveEffectsFile();
    modelPreview->SetCanvasSize(width,height);
}
void xLightsFrame::SetXmlSetting(const wxString& settingName,const wxString& value)
{
    wxXmlNode* e;
    // Delete existing setting node
    for(e=SettingsNode->GetChildren(); e!=NULL; e=e->GetNext())
    {
        if(e->GetName() == settingName)
        {
            SettingsNode->RemoveChild(e);
        }
    }
    // Add new one
    wxXmlNode* setting = new wxXmlNode( wxXML_ELEMENT_NODE, settingName );
    setting->AddAttribute("value",value);
    SettingsNode->AddChild(setting);
}
wxString xLightsFrame::GetXmlSetting(const wxString& settingName,const wxString& defaultValue)
{
    wxXmlNode* e;
    // Delete existing setting node
    for(e=SettingsNode->GetChildren(); e!=NULL; e=e->GetNext())
    {
        if(e->GetName() == settingName)
        {
            return e->GetAttribute("value",defaultValue);
        }
    }
    return defaultValue;
}

void xLightsFrame::OnChoicebook1PageChanged(wxChoicebookEvent& event)
{
}


void xLightsFrame::ToggleThreadedSave(wxCommandEvent& event)
{
    threadedSave = !threadedSave;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("ThreadedSave", threadedSave);
    ThreadedSaveMenuItem->Check(!threadedSave);
}

void xLightsFrame::OnCollapseEffectsButtonClick(wxCommandEvent& event)
{
    bool is_expanded = (CollapseEffectsButton->GetLabel() == "v");
    CollapseEffectsButton->SetLabel(is_expanded? "^": "v"); //TODO: make a nicer looking button
    Notebook2->Show(!is_expanded);
    wxSize size = Grid1->GetSize();
    size.y = is_expanded? 470: 120; //TODO: use auto layout
    Grid1->SetSize(size);
    if (Grid1->GetSizer() != NULL) {
        //No Sizer on the Mac
        Grid1->GetSizer()->Layout();
    }
    Grid1->GetParent()->GetSizer()->Layout();
}

void createACrash() {
    int *foo = NULL;
    *foo = 1;
}

void xLightsFrame::OnButtonNewSequenceClick(wxCommandEvent& event)
{
    createACrash();
}

void xLightsFrame::OnButtonClickSaveAs(wxCommandEvent& event)
{
    if (SeqData.size() == 0)
    {
        wxMessageBox("You must open a sequence first!", "Error");
        createACrash();
        return;
    }
    wxString NewFilename;
    wxTextEntryDialog dialog(this,"Enter a name for the sequence:","Save As");
    bool ok;
    do
    {
        if (dialog.ShowModal() != wxID_OK)
        {
            return;
        }
        // validate inputs
        NewFilename=dialog.GetValue();
        NewFilename.Trim();
        ok=true;
        if (NewFilename.IsEmpty())
        {
            ok=false;
            wxMessageBox(_("File name cannot be empty"), _("ERROR"));
        }
    }
    while (!ok);
    wxFileName oName(NewFilename);
    oName.SetPath( CurrentDir );
    oName.SetExt(_(XLIGHTS_SEQUENCE_EXT));
    DisplayXlightsFilename(oName.GetFullPath());

    oName.SetExt("xml");
    SeqXmlFileName=oName.GetFullPath();

    SaveSequence();
}

void xLightsFrame::SetButtonColor(wxButton* btn, const wxColour* c)
{
    btn->SetBackgroundColour(*c);
    int test=c->Red()*0.299 + c->Green()*0.587 + c->Blue()*0.114;
    btn->SetForegroundColour(test < 186 ? *wxWHITE : *wxBLACK);

#ifdef __WXOSX__
    //OSX does NOT allow active buttons to have a color other than the default.
    //We'll use an image of the appropriate color instead
    wxImage image(15, 15);
    image.SetRGB(wxRect(0, 0, 15, 15),
                 c->Red(), c->Green(), c->Blue());
    wxBitmap bmp(image);

    btn->SetBitmap(bmp);
    btn->SetLabelText("");
#endif
}


void xLightsFrame::OnMenuXmlConversionSettings(wxCommandEvent& event)
{
    // populate dialog
    XmlConversionDialog dialog(this);
    dialog.Fit();
    if (dialog.ShowModal() != wxID_OK) return;  // user pressed cancel
}
