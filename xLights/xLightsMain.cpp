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
#include "SeqSettingsDialog.h"

#include "RenderCommandEvent.h"


// scripting language
#include "xLightsBasic.cpp"

// image files
#include "../include/xLights.xpm"
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
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/font.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/artprov.h>
//*)


#define TOOLBAR_SAVE_VERSION "0001:"

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
const long xLightsFrame::ID_AUITOOLBAR_RENDERALL = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_MAIN = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PLAY_NOW = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PAUSE = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_STOP = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_FIRST_FRAME = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_LAST_FRAME = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_REPLAY_SECTION = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PLAY = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM2 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM5 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM3 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM1 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM4 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM6 = wxNewId();
const long xLightsFrame::ID_AUIWINDOWTOOLBAR = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ZOOM_IN = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ZOOM_OUT = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM14 = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_VIEW = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_TAB_INFO = wxNewId();
const long xLightsFrame::ID_BUTTON_STOP_NOW = wxNewId();
const long xLightsFrame::ID_BUTTON_GRACEFUL_STOP = wxNewId();
const long xLightsFrame::ID_BUTTON_LIGHTS_OFF = wxNewId();
const long xLightsFrame::ID_CHECKBOX_LIGHT_OUTPUT = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_OUTPUT = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON7 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON8 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON3 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON4 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON31 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON32 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON33 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON34 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON9 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON10 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON11 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON12 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON13 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON14 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON15 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON16 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON17 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON18 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON19 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON20 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON21 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON22 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON23 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON24 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON25 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON26 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON27 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON28 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON29 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON30 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON35 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON36 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON37 = wxNewId();
const long xLightsFrame::ID_AUIEFFECTSTOOLBAR = wxNewId();
const long xLightsFrame::ID_BUTTON3 = wxNewId();
const long xLightsFrame::ID_STATICTEXT4 = wxNewId();
const long xLightsFrame::ID_BUTTON_CHANGE_MEDIA_DIR = wxNewId();
const long xLightsFrame::ID_ANY = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON_Link_Dirs = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_SETUP = wxNewId();
const long xLightsFrame::ID_BUTTON_ADD_DONGLE = wxNewId();
const long xLightsFrame::ID_BUTTON_ADD_E131 = wxNewId();
const long xLightsFrame::ID_BUTTON1 = wxNewId();
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
const long xLightsFrame::ID_STATICTEXT17 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_OFF_AT_END = wxNewId();
const long xLightsFrame::ID_STATICTEXT20 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_MAP_EMPTY_CHANNELS = wxNewId();
const long xLightsFrame::ID_STATICTEXT33 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_LOR_WITH_NO_CHANNELS = wxNewId();
const long xLightsFrame::ID_CHOICE1 = wxNewId();
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
const long xLightsFrame::ID_CHECKBOX1 = wxNewId();
const long xLightsFrame::ID_STATICTEXT32 = wxNewId();
const long xLightsFrame::ID_SLIDER_BACKGROUND_BRIGHTNESS = wxNewId();
const long xLightsFrame::ID_BUTTON_SELECT_MODEL_GROUPS = wxNewId();
const long xLightsFrame::ID_STATICTEXT21 = wxNewId();
const long xLightsFrame::ID_LISTBOX_ELEMENT_LIST = wxNewId();
const long xLightsFrame::ID_BUTTON_MODELS_PREVIEW = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long xLightsFrame::ID_STATICTEXT22 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE = wxNewId();
const long xLightsFrame::ID_SLIDER3 = wxNewId();
const long xLightsFrame::ID_STATICTEXT24 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL3 = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_SCALE = wxNewId();
const long xLightsFrame::ID_STATICTEXT25 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL2 = wxNewId();
const long xLightsFrame::ID_SLIDER_PREVIEW_ROTATE = wxNewId();
const long xLightsFrame::ID_STATICTEXT31 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL4 = wxNewId();
const long xLightsFrame::ID_PANEL5 = wxNewId();
const long xLightsFrame::ID_PANEL1 = wxNewId();
const long xLightsFrame::ID_SPLITTERWINDOW2 = wxNewId();
const long xLightsFrame::ID_PANEL_PREVIEW = wxNewId();
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
const long xLightsFrame::ID_NEW_SEQUENCE = wxNewId();
const long xLightsFrame::ID_OPEN_SEQUENCE = wxNewId();
const long xLightsFrame::IS_SAVE_SEQ = wxNewId();
const long xLightsFrame::ID_SAVE_AS_SEQUENCE = wxNewId();
const long xLightsFrame::ID_CLOSE_SEQ = wxNewId();
const long xLightsFrame::ID_MENUITEM2 = wxNewId();
const long xLightsFrame::ID_FILE_BACKUP = wxNewId();
const long xLightsFrame::idMenuSaveSched = wxNewId();
const long xLightsFrame::idMenuAddList = wxNewId();
const long xLightsFrame::idMenuRenameList = wxNewId();
const long xLightsFrame::idMenuDelList = wxNewId();
const long xLightsFrame::ID_MENUITEM1 = wxNewId();
const long xLightsFrame::idCustomScript = wxNewId();
const long xLightsFrame::ID_MENUITEM_VIEW_ZOOM_IN = wxNewId();
const long xLightsFrame::ID_MENUITEM_VIEW_ZOOM_OUT = wxNewId();
const long xLightsFrame::ID_MENUITEM_SAVE_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM_LOAD_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM7 = wxNewId();
const long xLightsFrame::ID_MENUITEM_DISPLAY_ELEMENTS = wxNewId();
const long xLightsFrame::ID_MENUITEM12 = wxNewId();
const long xLightsFrame::ID_MENUITEM3 = wxNewId();
const long xLightsFrame::ID_MENUITEM14 = wxNewId();
const long xLightsFrame::ID_MENUITEM15 = wxNewId();
const long xLightsFrame::ID_MENUITEM16 = wxNewId();
const long xLightsFrame::ID_MENUITEM17 = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_WINDOW = wxNewId();
const long xLightsFrame::ID_MENUITEM_WINDOWS_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM10 = wxNewId();
const long xLightsFrame::ID_PLAY_FULL = wxNewId();
const long xLightsFrame::ID_PLAY_3_4 = wxNewId();
const long xLightsFrame::ID_PLAY_1_2 = wxNewId();
const long xLightsFrame::ID_PLAY_1_4 = wxNewId();
const long xLightsFrame::ID_IMPORT_EFFECTS = wxNewId();
const long xLightsFrame::ID_SEQ_SETTINGS = wxNewId();
const long xLightsFrame::ID_RENDER_ON_SAVE = wxNewId();
const long xLightsFrame::ID_MENUITEM_ICON_SMALL = wxNewId();
const long xLightsFrame::ID_MENUITEM_ICON_MEDIUM = wxNewId();
const long xLightsFrame::ID_MENUITEM_ICON_LARGE = wxNewId();
const long xLightsFrame::ID_MENUITEM_ICON_XLARGE = wxNewId();
const long xLightsFrame::ID_MENUITEM4 = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_SMALL = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_MEDIUM = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_LARGE = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_XLARGE = wxNewId();
const long xLightsFrame::ID_MENUITEM6 = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_BACKGROUND_ON = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_ICON_BACKGROUND_OFF = wxNewId();
const long xLightsFrame::ID_MENUITEM_Grid_Icon_Backgrounds = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_NODE_VALUES_ON = wxNewId();
const long xLightsFrame::ID_MENUITEM_GRID_NODE_VALUES_OFF = wxNewId();
const long xLightsFrame::ID_MENUITEM8 = wxNewId();
const long xLightsFrame::ID_MENU_CANVAS_ERASE_MODE = wxNewId();
const long xLightsFrame::ID_MENU_CANVAS_CANVAS_MODE = wxNewId();
const long xLightsFrame::ID_MENUITEM_RENDER_MODE = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_ALWAYS_ON = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_ALWAYS_OFF = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_TOGGLE = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST = wxNewId();
const long xLightsFrame::ID_MENUITEM5 = wxNewId();
const long xLightsFrame::idMenuHelpContent = wxNewId();
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
wxDEFINE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);
wxDEFINE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_UNSELECTED_EFFECT, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_UPDATED, wxCommandEvent);
wxDEFINE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDEFINE_EVENT(EVT_LOAD_PERSPECTIVE, wxCommandEvent);
wxDEFINE_EVENT(EVT_PERSPECTIVES_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SAVE_PERSPECTIVES, wxCommandEvent);
wxDEFINE_EVENT(EVT_EXPORT_MODEL, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_MODEL, wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_PAUSE_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);
wxDEFINE_EVENT(EVT_STOP_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_FIRST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_LAST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_REPLAY_SECTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDEFINE_EVENT(EVT_RENDER_RANGE, RenderCommandEvent);
wxDEFINE_EVENT(EVT_CONVERT_DATA_TO_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PROMOTE_EFFECTS, wxCommandEvent);

BEGIN_EVENT_TABLE(xLightsFrame,wxFrame)
    //(*EventTable(xLightsFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_TIME_LINE_CHANGED, xLightsFrame::TimelineChanged)
    EVT_COMMAND(wxID_ANY, EVT_ZOOM, xLightsFrame::Zoom)
    EVT_COMMAND(wxID_ANY, EVT_HORIZ_SCROLL, xLightsFrame::HorizontalScrollChanged)
    EVT_COMMAND(wxID_ANY, EVT_SCROLL_RIGHT, xLightsFrame::ScrollRight)
    EVT_COMMAND(wxID_ANY, EVT_TIME_SELECTED, xLightsFrame::TimeSelected)
    EVT_COMMAND(wxID_ANY, EVT_ROW_HEADINGS_CHANGED, xLightsFrame::RowHeadingsChanged)
    EVT_COMMAND(wxID_ANY, EVT_WINDOW_RESIZED, xLightsFrame::WindowResized)
    EVT_COMMAND(wxID_ANY, EVT_SELECTED_EFFECT_CHANGED, xLightsFrame::SelectedEffectChanged)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_CHANGED, xLightsFrame::EffectChanged)
    EVT_COMMAND(wxID_ANY, EVT_UNSELECTED_EFFECT, xLightsFrame::UnselectedEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_DROPPED, xLightsFrame::EffectDroppedOnGrid)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_MODEL_EFFECT, xLightsFrame::PlayModelEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_UPDATED, xLightsFrame::UpdateEffect)
    EVT_COMMAND(wxID_ANY, EVT_FORCE_SEQUENCER_REFRESH, xLightsFrame::ForceSequencerRefresh)
    EVT_COMMAND(wxID_ANY, EVT_LOAD_PERSPECTIVE, xLightsFrame::LoadPerspective)
    EVT_COMMAND(wxID_ANY, EVT_SAVE_PERSPECTIVES, xLightsFrame::OnMenuItemViewSavePerspectiveSelected)
    EVT_COMMAND(wxID_ANY, EVT_PERSPECTIVES_CHANGED, xLightsFrame::PerspectivesChanged)
    EVT_COMMAND(wxID_ANY, EVT_EXPORT_MODEL, xLightsFrame::ExportModel)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_MODEL, xLightsFrame::PlayModel)
    EVT_COMMAND(wxID_ANY, EVT_MODEL_SELECTED, xLightsFrame::ModelSelected)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_SEQUENCE, xLightsFrame::PlaySequence)
    EVT_COMMAND(wxID_ANY, EVT_PAUSE_SEQUENCE, xLightsFrame::PauseSequence)
    EVT_COMMAND(wxID_ANY, EVT_STOP_SEQUENCE, xLightsFrame::StopSequence)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_FIRST_FRAME, xLightsFrame::SequenceFirstFrame)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_LAST_FRAME, xLightsFrame::SequenceLastFrame)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_REPLAY_SECTION, xLightsFrame::SequenceReplaySection)
    EVT_COMMAND(wxID_ANY, EVT_TOGGLE_PLAY, xLightsFrame::TogglePlay)
    EVT_COMMAND(wxID_ANY, EVT_SHOW_DISPLAY_ELEMENTS, xLightsFrame::ShowDisplayElements)
    EVT_COMMAND(wxID_ANY, EVT_IMPORT_TIMING, xLightsFrame::ExecuteImportTimingElement)
    EVT_COMMAND(wxID_ANY, EVT_CONVERT_DATA_TO_EFFECTS, xLightsFrame::ConvertDataRowToEffects)
    EVT_COMMAND(wxID_ANY, EVT_PROMOTE_EFFECTS, xLightsFrame::PromoteEffects)
    wx__DECLARE_EVT1(EVT_RENDER_RANGE, wxID_ANY, &xLightsFrame::RenderRange)
END_EVENT_TABLE()



xLightsFrame::xLightsFrame(wxWindow* parent,wxWindowID id)
{
    Bind(EVT_RENDER_RANGE, &xLightsFrame::RenderRange, this);

    //(*Initialize(xLightsFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem29;
    wxMenuItem* MenuItem23;
    wxFlexGridSizer* FlexGridSizer30;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer21;
    wxStaticText* StaticText22;
    wxFlexGridSizer* FlexGridSizer28;
    wxMenuItem* MenuItem31;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer25;
    wxMenuItem* MenuItem30;
    wxMenuItem* MenuItemDelList;
    wxStaticText* StaticText21;
    wxFlexGridSizer* FlexGridSizer15;
    wxBoxSizer* BoxSizer3;
    wxMenu* Menu5;
    wxFlexGridSizer* FlexGridSizer56;
    wxFlexGridSizer* FlexGridSizer50;
    wxMenuItem* MenuItem12;
    wxMenuItem* MenuItem25;
    wxStaticText* StaticText28;
    wxFlexGridSizer* FlexGridSizer17;
    wxBoxSizer* BoxSizer10;
    wxFlexGridSizer* FlexGridSizer29;
    wxMenuItem* MenuItem19;
    wxFlexGridSizer* FlexGridSizer19;
    wxFlexGridSizer* FlexGridSizer11;
    wxMenuItem* MenuItemAddList;
    wxFlexGridSizer* FlexGridSizerCal;
    wxFlexGridSizer* FlexGridSizer47;
    wxMenuItem* MenuItem20;
    wxFlexGridSizer* FlexGridSizer40;
    wxFlexGridSizer* FlexGridSizerPreview;
    wxFlexGridSizer* FlexGridSizer7;
    wxMenu* MenuItem_Grid_Icon_Backgrounds;
    wxMenuItem* MenuItem24;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer26;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer55;
    wxFlexGridSizer* FlexGridSizer6;
    wxMenuItem* MenuItem17;
    wxButton* Button03;
    wxFlexGridSizer* FlexGridSizer27;
    wxFlexGridSizer* FlexGridSizer37;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer31;
    wxMenuItem* MenuItem9;
    wxFlexGridSizer* FlexGridSizer43;
    wxFlexGridSizer* FlexGridSizerPapagayo;
    wxStaticText* StaticText38;
    wxFlexGridSizer* FlexGridSizer39;
    wxMenuItem* MenuItem11;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxBoxSizer* BoxSizer8;
    wxMenuItem* MenuItem22;
    wxMenuItem* MenuItem5;
    wxFlexGridSizer* FlexGridSizer16;
    wxStaticBoxSizer* StaticBoxSizerHighlightColor;
    wxFlexGridSizer* FlexGridSizer23;
    wxFlexGridSizer* FlexGridSizerConvert;
    wxFlexGridSizer* FlexGridSizerTest;
    wxStaticBoxSizer* StaticBoxSizerBackgroundColor;
    wxFlexGridSizer* FlexGridSizer10;
    wxMenuItem* MenuItemDisplayElements;
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer13;
    wxMenuItem* MenuItem10;
    wxMenuItem* MenuItem27;
    wxGridBagSizer* GridBagSizer1;
    wxFlexGridSizer* FlexGridSizer18;
    wxMenuItem* MenuItem4;
    wxMenuItem* MenuItem6;
    wxMenuItem* MenuItem26;
    wxMenuItem* MenuItem13;
    wxFlexGridSizer* FlexGridSizer54;
    wxFlexGridSizer* FlexGridSizer36;
    wxFlexGridSizer* FlexGridSizer12;
    wxMenu* Menu2;
    wxMenuItem* MenuItemCustomScript;
    wxMenu* MenuPlaylist;
    wxMenuItem* MenuItem28;
    wxFlexGridSizer* FlexGridSizer35;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer24;
    wxMenu* MenuHelp;
    wxMenuItem* MenuItem8;
    wxMenuItem* MenuItem14;
    wxFlexGridSizer* FlexGridSizer57;
    wxFlexGridSizer* FlexGridSizer32;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxMenuItem* MenuItemRenameList;
    wxFlexGridSizer* FlexGridSizer20;
    wxFlexGridSizer* FlexGridSizerSetup;

    Create(parent, wxID_ANY, _("<use variables in xLightsMain.h>"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(1411,1103));
    SetToolTip(_("Export only Channels associated with one model"));
    MainAuiManager = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_ALLOW_ACTIVE_PANE|wxAUI_MGR_DEFAULT);
    MainToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_MAIN, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPENSHOW, _("Open Show Directory"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER_OPEN")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Select Show Directory"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_NEWSEQUENCE, _("New Sequence"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_NEW")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("New Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPEN, _("Open Sequence"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Open Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVE, _("Save"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Save"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVEAS, _("Save As"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE_AS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Save As"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_RENDERALL, _("Render All"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_RENDER_ALL")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Render All"), wxEmptyString, NULL);
    MainToolBar->Realize();
    MainAuiManager->AddPane(MainToolBar, wxAuiPaneInfo().Name(_T("Main Tool Bar")).ToolbarPane().Caption(_("Main Tool Bar")).CloseButton(false).Layer(10).Top().Gripper());
    PlayToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_PLAY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    PlayToolBar->AddTool(ID_AUITOOLBAR_PLAY_NOW, _("Play"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PLAY")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Play"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_PAUSE, _("Pause"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PAUSE")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Pause"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_STOP, _("Stop"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_STOP")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Stop"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_FIRST_FRAME, _("Item label"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_BACKWARD")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("First Frame"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_LAST_FRAME, _("Item label"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_FORWARD")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Last Frame"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_REPLAY_SECTION, _("Item label"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_REPLAY")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Replay Section"), wxEmptyString, NULL);
    PlayToolBar->Realize();
    MainAuiManager->AddPane(PlayToolBar, wxAuiPaneInfo().Name(_T("Play Tool Bar")).ToolbarPane().Caption(_("Play Tool Bar")).CloseButton(false).Layer(10).Position(11).Top().Gripper());
    WindowMgmtToolbar = new xlAuiToolBar(this, ID_AUIWINDOWTOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM2, _("Effects"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_EFFECTS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Effects"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM5, _("Effect Colors"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_COLORS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Effect Colors"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM3, _("Effect Layering"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LAYERS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Effect Layering"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM1, _("Model Preview"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_MODEL_PREVIEW")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Model Preview"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM4, _("House Preview"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_HOUSE_PREVIEW")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("House Preview"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM6, _("Models"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_MODELS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Models"), wxEmptyString, NULL);
    WindowMgmtToolbar->Realize();
    MainAuiManager->AddPane(WindowMgmtToolbar, wxAuiPaneInfo().Name(_T("Windows Tool Bar")).ToolbarPane().Caption(_("Windows Tool Bar")).CloseButton(false).Layer(10).Position(12).Top().Gripper());
    ViewToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_VIEW, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    ViewToolBar->AddTool(ID_AUITOOLBARITEM_ZOOM_IN, _("Zoom In"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_ZOOM_IN")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Zoom In"), wxEmptyString, NULL);
    ViewToolBar->AddTool(ID_AUITOOLBARITEM_ZOOM_OUT, _("Zoom Out"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_ZOOM_OUT")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Zoom Out"), wxEmptyString, NULL);
    ViewToolBar->AddTool(ID_AUITOOLBARITEM14, _("Sequence Settings"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_SETTINGS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Settings"), wxEmptyString, NULL);
    ViewToolBar->Realize();
    MainAuiManager->AddPane(ViewToolBar, wxAuiPaneInfo().Name(_T("View Tool Bar")).ToolbarPane().Caption(_("Pane caption")).CloseButton(false).Layer(10).Position(13).Top().Gripper());
    OutputToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_OUTPUT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    OutputToolBar->AddTool(ID_BITMAPBUTTON_TAB_INFO, _("Information"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_INFORMATION")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Tips for using current tab"), wxEmptyString, NULL);
    OutputToolBar->AddTool(ID_BUTTON_STOP_NOW, _("Stop"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_STOP_NOW")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Stop Now!"), wxEmptyString, NULL);
    OutputToolBar->AddTool(ID_BUTTON_GRACEFUL_STOP, _("Graceful Stop"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_GRACEFUL_STOP")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Graceful Stop"), wxEmptyString, NULL);
    OutputToolBar->AddTool(ID_BUTTON_LIGHTS_OFF, _("Lights Off"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LIGHTS_OFF")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Lights Off"), wxEmptyString, NULL);
    OutputToolBar->AddTool(ID_CHECKBOX_LIGHT_OUTPUT, _("Output To Lights"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_OUTPUT_LIGHTS")),wxART_TOOLBAR), wxNullBitmap, wxITEM_CHECK, _("Output To Lights"), wxEmptyString, NULL);
    OutputToolBar->Realize();
    MainAuiManager->AddPane(OutputToolBar, wxAuiPaneInfo().Name(_T("Output Tool Bar")).ToolbarPane().Caption(_("Output Tool Bar")).CloseButton(false).Layer(10).Position(25).Top().Gripper());
    EffectsToolBar = new xlAuiToolBar(this, ID_AUIEFFECTSTOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    BitmapButton1 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON7, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    BitmapButton1->SetMinSize(wxSize(16,16));
    BitmapButton1->SetMaxSize(wxSize(16,16));
    BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton1->SetEffectIndex(0);
    BitmapButton2 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON8, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON8"));
    BitmapButton2->SetMinSize(wxSize(16,16));
    BitmapButton2->SetMaxSize(wxSize(16,16));
    BitmapButton2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton2->SetEffectIndex(1);
    BitmapButton3 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON3, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    BitmapButton3->SetMinSize(wxSize(16,16));
    BitmapButton3->SetMaxSize(wxSize(16,16));
    BitmapButton3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton3->SetEffectIndex(2);
    BitmapButton4 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON4, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    BitmapButton4->SetMinSize(wxSize(16,16));
    BitmapButton4->SetMaxSize(wxSize(16,16));
    BitmapButton4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton4->SetEffectIndex(3);
    BitmapButton5 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON31, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON31"));
    BitmapButton5->SetMinSize(wxSize(16,16));
    BitmapButton5->SetMaxSize(wxSize(16,16));
    BitmapButton5->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton5->SetEffectIndex(4);
    BitmapButton6 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON32, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON32"));
    BitmapButton6->SetMinSize(wxSize(16,16));
    BitmapButton6->SetMaxSize(wxSize(16,16));
    BitmapButton6->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton6->SetEffectIndex(5);
    BitmapButton7 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON33, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON33"));
    BitmapButton7->SetMinSize(wxSize(16,16));
    BitmapButton7->SetMaxSize(wxSize(16,16));
    BitmapButton7->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton7->SetEffectIndex(6);
    BitmapButton8 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON34, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON34"));
    BitmapButton8->SetMinSize(wxSize(16,16));
    BitmapButton8->SetMaxSize(wxSize(16,16));
    BitmapButton8->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton8->SetEffectIndex(7);
    BitmapButton9 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON9, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    BitmapButton9->SetMinSize(wxSize(16,16));
    BitmapButton9->SetMaxSize(wxSize(16,16));
    BitmapButton9->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton9->SetEffectIndex(8);
    BitmapButton10 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON10, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON10"));
    BitmapButton10->SetMinSize(wxSize(16,16));
    BitmapButton10->SetMaxSize(wxSize(16,16));
    BitmapButton10->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton10->SetEffectIndex(9);
    BitmapButton11 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON11, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON11"));
    BitmapButton11->SetMinSize(wxSize(16,16));
    BitmapButton11->SetMaxSize(wxSize(16,16));
    BitmapButton11->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton11->SetEffectIndex(10);
    BitmapButton12 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON12, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON12"));
    BitmapButton12->SetMinSize(wxSize(16,16));
    BitmapButton12->SetMaxSize(wxSize(16,16));
    BitmapButton12->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton12->SetEffectIndex(11);
    BitmapButton13 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON13, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON13"));
    BitmapButton13->SetMinSize(wxSize(16,16));
    BitmapButton13->SetMaxSize(wxSize(16,16));
    BitmapButton13->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton13->SetEffectIndex(12);
    BitmapButton14 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON14, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON14"));
    BitmapButton14->SetMinSize(wxSize(16,16));
    BitmapButton14->SetMaxSize(wxSize(16,16));
    BitmapButton14->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton14->SetEffectIndex(13);
    BitmapButton15 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON15, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON15"));
    BitmapButton15->SetMinSize(wxSize(16,16));
    BitmapButton15->SetMaxSize(wxSize(16,16));
    BitmapButton15->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton15->SetEffectIndex(14);
    BitmapButton16 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON16, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON16"));
    BitmapButton16->SetMinSize(wxSize(16,16));
    BitmapButton16->SetMaxSize(wxSize(16,16));
    BitmapButton16->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton16->SetEffectIndex(15);
    BitmapButton17 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON17, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON17"));
    BitmapButton17->SetMinSize(wxSize(16,16));
    BitmapButton17->SetMaxSize(wxSize(16,16));
    BitmapButton17->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton17->SetEffectIndex(16);
    BitmapButton18 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON18, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON18"));
    BitmapButton18->SetMinSize(wxSize(16,16));
    BitmapButton18->SetMaxSize(wxSize(16,16));
    BitmapButton18->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton18->SetEffectIndex(17);
    BitmapButton19 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON19, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON19"));
    BitmapButton19->SetMinSize(wxSize(16,16));
    BitmapButton19->SetMaxSize(wxSize(16,16));
    BitmapButton19->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton19->SetEffectIndex(18);
    BitmapButton20 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON20, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON20"));
    BitmapButton20->SetMinSize(wxSize(16,16));
    BitmapButton20->SetMaxSize(wxSize(16,16));
    BitmapButton20->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton20->SetEffectIndex(19);
    BitmapButton21 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON21, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON21"));
    BitmapButton21->SetMinSize(wxSize(16,16));
    BitmapButton21->SetMaxSize(wxSize(16,16));
    BitmapButton21->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton21->SetEffectIndex(20);
    BitmapButton22 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON22, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON22"));
    BitmapButton22->SetMinSize(wxSize(16,16));
    BitmapButton22->SetMaxSize(wxSize(16,16));
    BitmapButton22->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton22->SetEffectIndex(21);
    BitmapButton23 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON23, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON23"));
    BitmapButton23->SetMinSize(wxSize(16,16));
    BitmapButton23->SetMaxSize(wxSize(16,16));
    BitmapButton23->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton23->SetEffectIndex(22);
    BitmapButton24 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON24, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON24"));
    BitmapButton24->SetMinSize(wxSize(16,16));
    BitmapButton24->SetMaxSize(wxSize(16,16));
    BitmapButton24->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton24->SetEffectIndex(23);
    BitmapButton25 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON25, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON25"));
    BitmapButton25->SetMinSize(wxSize(16,16));
    BitmapButton25->SetMaxSize(wxSize(16,16));
    BitmapButton25->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton25->SetEffectIndex(24);
    BitmapButton26 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON26, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON26"));
    BitmapButton26->SetMinSize(wxSize(16,16));
    BitmapButton26->SetMaxSize(wxSize(16,16));
    BitmapButton26->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton26->SetEffectIndex(25);
    BitmapButton27 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON27, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON27"));
    BitmapButton27->SetMinSize(wxSize(16,16));
    BitmapButton27->SetMaxSize(wxSize(16,16));
    BitmapButton27->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton27->SetEffectIndex(26);
    BitmapButton28 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON28, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON28"));
    BitmapButton28->SetMinSize(wxSize(16,16));
    BitmapButton28->SetMaxSize(wxSize(16,16));
    BitmapButton28->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton28->SetEffectIndex(27);
    BitmapButton29 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON29, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON29"));
    BitmapButton29->SetMinSize(wxSize(16,16));
    BitmapButton29->SetMaxSize(wxSize(16,16));
    BitmapButton29->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton29->SetEffectIndex(28);
    BitmapButton30 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON30, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON30"));
    BitmapButton30->SetMinSize(wxSize(16,16));
    BitmapButton30->SetMaxSize(wxSize(16,16));
    BitmapButton30->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton30->SetEffectIndex(29);
    BitmapButton31 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON35, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON35"));
    BitmapButton31->SetMinSize(wxSize(16,16));
    BitmapButton31->SetMaxSize(wxSize(16,16));
    BitmapButton31->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton31->SetEffectIndex(30);
    BitmapButton32 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON36, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON36"));
    BitmapButton32->SetMinSize(wxSize(16,16));
    BitmapButton32->SetMaxSize(wxSize(16,16));
    BitmapButton32->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton32->SetEffectIndex(31);
    BitmapButton33 = new DragEffectBitmapButton(EffectsToolBar, ID_BITMAPBUTTON37, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON37"));
    BitmapButton33->SetMinSize(wxSize(16,16));
    BitmapButton33->SetMaxSize(wxSize(16,16));
    BitmapButton33->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    BitmapButton33->SetEffectIndex(32);
    EffectsToolBar->AddControl(BitmapButton1, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton2, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton3, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton4, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton5, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton6, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton7, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton8, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton9, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton10, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton11, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton12, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton13, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton14, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton15, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton16, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton17, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton18, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton19, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton20, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton21, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton22, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton23, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton24, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton25, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton26, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton27, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton28, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton29, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton30, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton31, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton32, _("Item label"));
    EffectsToolBar->AddControl(BitmapButton33, _("Item label"));
    EffectsToolBar->Realize();
    MainAuiManager->AddPane(EffectsToolBar, wxAuiPaneInfo().Name(_T("EffectsToolBar")).ToolbarPane().Caption(_("Effects")).CloseButton(false).Layer(5).Top().Gripper());
    Notebook1 = new wxAuiNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TAB_SPLIT|wxAUI_NB_TAB_MOVE|wxAUI_NB_TAB_EXTERNAL_MOVE|wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_TOP|wxNO_BORDER);
    PanelSetup = new wxPanel(Notebook1, ID_PANEL_SETUP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_SETUP"));
    FlexGridSizerSetup = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetup->AddGrowableCol(0);
    FlexGridSizerSetup->AddGrowableRow(1);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, PanelSetup, _("Directories"));
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    StaticText38 = new wxStaticText(PanelSetup, wxID_ANY, _("Show Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    GridBagSizer1->Add(StaticText38, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button03 = new wxButton(PanelSetup, ID_BUTTON3, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    GridBagSizer1->Add(Button03, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ShowDirectoryLabel = new wxStaticText(PanelSetup, ID_STATICTEXT4, _("{Show Directory not set}"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    GridBagSizer1->Add(ShowDirectoryLabel, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText28 = new wxStaticText(PanelSetup, wxID_ANY, _("Media Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    GridBagSizer1->Add(StaticText28, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Button_Change_Media_Dir = new wxButton(PanelSetup, ID_BUTTON_CHANGE_MEDIA_DIR, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CHANGE_MEDIA_DIR"));
    GridBagSizer1->Add(Button_Change_Media_Dir, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MediaDirectoryLabel = new wxStaticText(PanelSetup, ID_ANY, _("{Media Directory not set}"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_ANY"));
    GridBagSizer1->Add(MediaDirectoryLabel, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Link_Dirs = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON_Link_Dirs, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_UNLINK")),wxART_OTHER), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Link_Dirs"));
    BitmapButton_Link_Dirs->SetToolTip(_("Link Directories"));
    GridBagSizer1->Add(BitmapButton_Link_Dirs, wxGBPosition(0, 3), wxGBSpan(2, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(GridBagSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    ButtonAddNull = new wxButton(PanelSetup, ID_BUTTON1, _("Add Null"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(ButtonAddNull, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    ChoiceOutputFormat->SetSelection( ChoiceOutputFormat->Append(_("Falcon Pi Player, *.fseq")) );
    ChoiceOutputFormat->Append(_("xLights Sequence, *.xseq"));
    ChoiceOutputFormat->Append(_("Lynx Conductor, *.seq"));
    ChoiceOutputFormat->Append(_("Vix,Vixen 2.1 *.vix sequence file"));
    ChoiceOutputFormat->Append(_("Vir, Vixen 2.1 *.vir routine file"));
    ChoiceOutputFormat->Append(_("LOR Sequence *.las or *.lms"));
    ChoiceOutputFormat->Append(_("Glediator Record File *.gled"));
    ChoiceOutputFormat->Append(_("Lcb, LOR clipboard *.lcb"));
    ChoiceOutputFormat->Append(_("HLS *.hlsnc"));
    FlexGridSizer26->Add(ChoiceOutputFormat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(PanelConvert, ID_STATICTEXT17, _("All channels off at end:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    FlexGridSizer26->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxOffAtEnd = new wxCheckBox(PanelConvert, ID_CHECKBOX_OFF_AT_END, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OFF_AT_END"));
    CheckBoxOffAtEnd->SetValue(false);
    FlexGridSizer26->Add(CheckBoxOffAtEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(FlexGridSizer26, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, PanelConvert, _("LOR Import Options"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText20 = new wxStaticText(PanelConvert, ID_STATICTEXT20, _("Map Empty LMS Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    FlexGridSizer1->Add(StaticText20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxMapEmptyChannels = new wxCheckBox(PanelConvert, ID_CHECKBOX_MAP_EMPTY_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_MAP_EMPTY_CHANNELS"));
    CheckBoxMapEmptyChannels->SetValue(false);
    FlexGridSizer1->Add(CheckBoxMapEmptyChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(PanelConvert, ID_STATICTEXT33, _("Map LMS Channels with no network"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer1->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MapLORChannelsWithNoNetwork = new wxCheckBox(PanelConvert, ID_CHECKBOX_LOR_WITH_NO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LOR_WITH_NO_CHANNELS"));
    MapLORChannelsWithNoNetwork->SetValue(true);
    FlexGridSizer1->Add(MapLORChannelsWithNoNetwork, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(PanelConvert, wxID_ANY, _("Time Resolution"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer1->Add(StaticText22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    LORImportTimeResolution = new wxChoice(PanelConvert, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    LORImportTimeResolution->Append(_("25 ms"));
    LORImportTimeResolution->SetSelection( LORImportTimeResolution->Append(_("50 ms")) );
    LORImportTimeResolution->Append(_("100 ms"));
    FlexGridSizer1->Add(LORImportTimeResolution, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer19 = new wxFlexGridSizer(0, 3, 0, 0);
    ButtonStartConversion = new wxButton(PanelConvert, ID_BUTTON_START_CONVERSION, _("Start Conversion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_START_CONVERSION"));
    FlexGridSizer19->Add(ButtonStartConversion, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(FlexGridSizer19, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    TextCtrlPreviewTime = new wxTextCtrl(PanelPreview, ID_TEXTCTRL_PREVIEW_TIME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_TIME"));
    FlexGridSizer31->Add(TextCtrlPreviewTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer30 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer40 = new wxFlexGridSizer(0, 7, 0, 0);
    FlexGridSizer40->AddGrowableCol(1);
    SliderPreviewTime = new wxSlider(PanelPreview, ID_SLIDER_PREVIEW_TIME, 0, 0, 200, wxDefaultPosition, wxDLG_UNIT(PanelPreview,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_TIME"));
    FlexGridSizer40->Add(SliderPreviewTime, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextCurrentPreviewSize = new wxStaticText(PanelPreview, ID_STATICTEXT_CURRENT_PREVIEW_SIZE, _("Size: 1920x1080"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_CURRENT_PREVIEW_SIZE"));
    FlexGridSizer40->Add(StaticTextCurrentPreviewSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSetPreviewSize = new wxButton(PanelPreview, ID_BUTTON_SET_PREVIEW_SIZE, _("Set Size"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SET_PREVIEW_SIZE"));
    FlexGridSizer40->Add(ButtonSetPreviewSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSetBackgroundImage = new wxButton(PanelPreview, ID_BUTTON_SET_BACKGROUND_IMAGE, _("Image"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SET_BACKGROUND_IMAGE"));
    FlexGridSizer40->Add(ButtonSetBackgroundImage, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ScaleImageCheckbox = new wxCheckBox(PanelPreview, ID_CHECKBOX1, _("Fill Background"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    ScaleImageCheckbox->SetValue(false);
    FlexGridSizer40->Add(ScaleImageCheckbox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(PanelPreview, ID_STATICTEXT32, _("Brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer40->Add(StaticText30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_BackgroundBrightness = new wxSlider(PanelPreview, ID_SLIDER_BACKGROUND_BRIGHTNESS, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_BACKGROUND_BRIGHTNESS"));
    FlexGridSizer40->Add(Slider_BackgroundBrightness, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30->Add(FlexGridSizer40, 1, wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer31->Add(FlexGridSizer30, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerPreview->Add(FlexGridSizer31, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SplitterWindow2 = new wxSplitterWindow(PanelPreview, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetMinSize(wxSize(10,10));
    SplitterWindow2->SetMinimumPaneSize(10);
    SplitterWindow2->SetSashGravity(0.5);
    Panel1 = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer35 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer35->AddGrowableCol(0);
    FlexGridSizer35->AddGrowableRow(2);
    ButtonSelectModelGroups = new wxButton(Panel1, ID_BUTTON_SELECT_MODEL_GROUPS, _("Select Model Groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_MODEL_GROUPS"));
    FlexGridSizer35->Add(ButtonSelectModelGroups, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT21, _("Preview Models:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer35->Add(StaticText5, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ListBoxElementList = new wxListView(Panel1, ID_LISTBOX_ELEMENT_LIST, wxDefaultPosition, wxSize(100,-1), wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTBOX_ELEMENT_LIST"));
    FlexGridSizer35->Add(ListBoxElementList, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonModelsPreview = new wxButton(Panel1, ID_BUTTON_MODELS_PREVIEW, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MODELS_PREVIEW"));
    FlexGridSizer35->Add(ButtonModelsPreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSavePreview = new wxButton(Panel1, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
    FlexGridSizer35->Add(ButtonSavePreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer37 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer37->AddGrowableCol(1);
    StaticText23 = new wxStaticText(Panel1, ID_STATICTEXT22, _("Model Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer37->Add(StaticText23, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlPreviewElementWidth = new wxTextCtrl(Panel1, ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE"));
    FlexGridSizer37->Add(TextCtrlPreviewElementWidth, 1, wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SliderPreviewScaleWidth = new wxSlider(Panel1, ID_SLIDER3, 50, 1, 700, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER3"));
    FlexGridSizer35->Add(SliderPreviewScaleWidth, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText37 = new wxStaticText(Panel1, ID_STATICTEXT24, _("Model Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer2->Add(StaticText37, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlPreviewElementHeight = new wxTextCtrl(Panel1, ID_TEXTCTRL3, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer2->Add(TextCtrlPreviewElementHeight, 1, wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderPreviewScaleHeight = new wxSlider(Panel1, ID_SLIDER_PREVIEW_SCALE, 50, 1, 700, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_SCALE"));
    FlexGridSizer35->Add(SliderPreviewScaleHeight, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer39 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticTextPreviewRotation = new wxStaticText(Panel1, ID_STATICTEXT25, _("Model Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    StaticTextPreviewRotation->Disable();
    FlexGridSizer39->Add(StaticTextPreviewRotation, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlModelRotationDegrees = new wxTextCtrl(Panel1, ID_TEXTCTRL2, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer39->Add(TextCtrlModelRotationDegrees, 1, wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SliderPreviewRotate = new wxSlider(Panel1, ID_SLIDER_PREVIEW_ROTATE, 0, -180, 180, wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW, wxDefaultValidator, _T("ID_SLIDER_PREVIEW_ROTATE"));
    SliderPreviewRotate->SetTickFreq(1);
    SliderPreviewRotate->Disable();
    FlexGridSizer35->Add(SliderPreviewRotate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer32 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText39 = new wxStaticText(Panel1, ID_STATICTEXT31, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer32->Add(StaticText39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlModelStartChannel = new wxTextCtrl(Panel1, ID_TEXTCTRL4, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    FlexGridSizer32->Add(TextCtrlModelStartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer35->Add(FlexGridSizer32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer35);
    FlexGridSizer35->Fit(Panel1);
    FlexGridSizer35->SetSizeHints(Panel1);
    PreviewGLPanel = new wxPanel(SplitterWindow2, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
    PreviewGLSizer = new wxFlexGridSizer(1, 1, 0, 0);
    PreviewGLSizer->AddGrowableCol(0);
    PreviewGLSizer->AddGrowableRow(0);
    PreviewGLPanel->SetSizer(PreviewGLSizer);
    PreviewGLSizer->Fit(PreviewGLPanel);
    PreviewGLSizer->SetSizeHints(PreviewGLPanel);
    SplitterWindow2->SplitVertically(Panel1, PreviewGLPanel);
    SplitterWindow2->SetSashPosition(100);
    FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelPreview->SetSizer(FlexGridSizerPreview);
    FlexGridSizerPreview->Fit(PanelPreview);
    FlexGridSizerPreview->SetSizeHints(PanelPreview);
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
    TextCtrl_papagayo_output_filename = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL67, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(PanelPapagayo,wxSize(150,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL67"));
    TextCtrl_papagayo_output_filename->SetToolTip(_("Sequence output will contain the generated effects."));
    FlexGridSizer54->Add(TextCtrl_papagayo_output_filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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
    TextCtrl_PgoMinRest = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoMinRest, _(".2"), wxDefaultPosition, wxDLG_UNIT(PanelPapagayo,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoMinRest"));
    TextCtrl_PgoMinRest->SetToolTip(_("Minimum gap size to insert Rest into."));
    FlexGridSizer3->Add(TextCtrl_PgoMinRest, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText25 = new wxStaticText(PanelPapagayo, ID_STATICTEXT27, _("max:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer3->Add(StaticText25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_PgoMaxRest = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoMaxRest, _(".4"), wxDefaultPosition, wxDLG_UNIT(PanelPapagayo,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoMaxRest"));
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
    TextCtrl_PgoAutoFade = new wxTextCtrl(PanelPapagayo, ID_TEXTCTRL_PgoAutoFade, _("1.5"), wxDefaultPosition, wxDLG_UNIT(PanelPapagayo,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_PgoAutoFade"));
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
    PanelSequencer = new wxPanel(Notebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("ID_PANEL7"));
    m_mgr = new wxAuiManager(PanelSequencer, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_ALLOW_ACTIVE_PANE|wxAUI_MGR_DEFAULT);
    Notebook1->AddPage(PanelSetup, _("Setup"), true);
    Notebook1->AddPage(PanelTest, _("Test"));
    Notebook1->AddPage(PanelConvert, _("Convert"));
    Notebook1->AddPage(PanelPreview, _("Layout"));
    Notebook1->AddPage(PanelCal, _("Schedule"));
    Notebook1->AddPage(PanelPapagayo, _("Papagayo"));
    Notebook1->AddPage(PanelSequencer, _("Sequencer"));
    MainAuiManager->AddPane(Notebook1, wxAuiPaneInfo().Name(_T("MainPain")).CenterPane().Caption(_("Pane caption")).Floatable().PaneBorder(false));
    MainAuiManager->Update();
    MenuBar = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItem3 = new wxMenuItem(MenuFile, ID_NEW_SEQUENCE, _("New Sequence\tCtrl-n"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem3);
    MenuItem_File_Open_Sequence = new wxMenuItem(MenuFile, ID_OPEN_SEQUENCE, _("Open Sequence\tCTRL-o"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_File_Open_Sequence->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_OPEN")),wxART_OTHER));
    MenuFile->Append(MenuItem_File_Open_Sequence);
    MenuItem_File_Save_Sequence = new wxMenuItem(MenuFile, IS_SAVE_SEQ, _("Save Sequence\tCTRL-S"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_File_Save_Sequence->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FILE_SAVE")),wxART_OTHER));
    MenuFile->Append(MenuItem_File_Save_Sequence);
    MenuItem_File_Save_Sequence->Enable(false);
    MenuItem_File_SaveAs_Sequence = new wxMenuItem(MenuFile, ID_SAVE_AS_SEQUENCE, _("Save As Sequence"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem_File_SaveAs_Sequence);
    MenuItem_File_Close_Sequence = new wxMenuItem(MenuFile, ID_CLOSE_SEQ, _("Close Sequence"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem_File_Close_Sequence);
    MenuItem_File_Close_Sequence->Enable(false);
    MenuFile->AppendSeparator();
    MenuItem5 = new wxMenuItem(MenuFile, ID_MENUITEM2, _("Select Show Folder\tF9"), wxEmptyString, wxITEM_NORMAL);
    MenuItem5->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FOLDER")),wxART_OTHER));
    MenuFile->Append(MenuItem5);
    MenuItemBackup = new wxMenuItem(MenuFile, ID_FILE_BACKUP, _("Backup\tF10"), wxEmptyString, wxITEM_NORMAL);
    MenuItemBackup->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_HARDDISK")),wxART_OTHER));
    MenuFile->Append(MenuItemBackup);
    QuitMenuItem = new wxMenuItem(MenuFile, wxID_EXIT, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    QuitMenuItem->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_QUIT")),wxART_OTHER));
    MenuFile->Append(QuitMenuItem);
    MenuBar->Append(MenuFile, _("&File"));
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
    MenuBar->Append(MenuPlaylist, _("&Playlist"));
    Menu5 = new wxMenu();
    MenuItem_ViewZoomIn = new wxMenuItem(Menu5, ID_MENUITEM_VIEW_ZOOM_IN, _("Zoom In"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_ViewZoomIn);
    MenuItem_ViewZoomOut = new wxMenuItem(Menu5, ID_MENUITEM_VIEW_ZOOM_OUT, _("Zoom Out"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_ViewZoomOut);
    Menu5->AppendSeparator();
    MenuItem15 = new wxMenu();
    MenuItemViewSavePerspective = new wxMenuItem(MenuItem15, ID_MENUITEM_SAVE_PERSPECTIVE, _("Save Current"), wxEmptyString, wxITEM_NORMAL);
    MenuItem15->Append(MenuItemViewSavePerspective);
    MenuItemLoadEditPerspective = new wxMenuItem(MenuItem15, ID_MENUITEM_LOAD_PERSPECTIVE, _("Edit/Load"), wxEmptyString, wxITEM_NORMAL);
    MenuItem15->Append(MenuItemLoadEditPerspective);
    Menu5->Append(ID_MENUITEM7, _("Perspectives"), MenuItem15, wxEmptyString);
    MenuItem18 = new wxMenu();
    MenuItemDisplayElements = new wxMenuItem(MenuItem18, ID_MENUITEM_DISPLAY_ELEMENTS, _("Display Elements"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItemDisplayElements);
    MenuItem20 = new wxMenuItem(MenuItem18, ID_MENUITEM12, _("Model Preview"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem20);
    MenuItem6 = new wxMenuItem(MenuItem18, ID_MENUITEM3, _("House Preview"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem6);
    MenuItem22 = new wxMenuItem(MenuItem18, ID_MENUITEM14, _("Effect Settings"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem22);
    MenuItem23 = new wxMenuItem(MenuItem18, ID_MENUITEM15, _("Colors"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem23);
    MenuItem24 = new wxMenuItem(MenuItem18, ID_MENUITEM16, _("Timing"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem24);
    MenuItem25 = new wxMenuItem(MenuItem18, ID_MENUITEM17, _("Effect Dropper"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem25);
    MenuItemEffectAssistWindow = new wxMenuItem(MenuItem18, ID_MENUITEM_EFFECT_ASSIST_WINDOW, _("Effect Assist"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItemEffectAssistWindow);
    MenuItem18->AppendSeparator();
    MenuItem26 = new wxMenuItem(MenuItem18, ID_MENUITEM_WINDOWS_PERSPECTIVE, _("Perspectives"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem26);
    Menu5->Append(ID_MENUITEM10, _("Windows"), MenuItem18, wxEmptyString);
    MenuBar->Append(Menu5, _("&View"));
    AudioMenu = new wxMenu();
    MenuItem8 = new wxMenuItem(AudioMenu, ID_PLAY_FULL, _("Play Full Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem8);
    MenuItem9 = new wxMenuItem(AudioMenu, ID_PLAY_3_4, _("Play 3/4 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem9);
    MenuItem30 = new wxMenuItem(AudioMenu, ID_PLAY_1_2, _("Play 1/2 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem30);
    MenuItem31 = new wxMenuItem(AudioMenu, ID_PLAY_1_4, _("Play 1/4 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem31);
    MenuBar->Append(AudioMenu, _("&Audio"));
    Menu2 = new wxMenu();
    MenuItem29 = new wxMenuItem(Menu2, ID_IMPORT_EFFECTS, _("Import Effects"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem29);
    MenuBar->Append(Menu2, _("&Import"));
    MenuSettings = new wxMenu();
    Menu_Settings_Sequence = new wxMenuItem(MenuSettings, ID_SEQ_SETTINGS, _("Sequence Settings"), wxEmptyString, wxITEM_NORMAL);
    MenuSettings->Append(Menu_Settings_Sequence);
    mRenderOnSaveMenuItem = new wxMenuItem(MenuSettings, ID_RENDER_ON_SAVE, _("Render On Save"), wxEmptyString, wxITEM_CHECK);
    MenuSettings->Append(mRenderOnSaveMenuItem);
    mRenderOnSaveMenuItem->Check(true);
    ToolIconSizeMenu = new wxMenu();
    MenuItem10 = new wxMenuItem(ToolIconSizeMenu, ID_MENUITEM_ICON_SMALL, _("Small\tALT-1"), wxEmptyString, wxITEM_RADIO);
    ToolIconSizeMenu->Append(MenuItem10);
    MenuItem11 = new wxMenuItem(ToolIconSizeMenu, ID_MENUITEM_ICON_MEDIUM, _("Medium\tALT-2"), wxEmptyString, wxITEM_RADIO);
    ToolIconSizeMenu->Append(MenuItem11);
    MenuItem12 = new wxMenuItem(ToolIconSizeMenu, ID_MENUITEM_ICON_LARGE, _("Large\tALT-3"), wxEmptyString, wxITEM_RADIO);
    ToolIconSizeMenu->Append(MenuItem12);
    MenuItem14 = new wxMenuItem(ToolIconSizeMenu, ID_MENUITEM_ICON_XLARGE, _("Extra Large\tALT-4"), wxEmptyString, wxITEM_RADIO);
    ToolIconSizeMenu->Append(MenuItem14);
    MenuSettings->Append(ID_MENUITEM4, _("Tool Icon Size"), ToolIconSizeMenu, wxEmptyString);
    GridSpacingMenu = new wxMenu();
    MenuItem17 = new wxMenuItem(GridSpacingMenu, ID_MENUITEM_GRID_ICON_SMALL, _("Small\tCTRL-1"), wxEmptyString, wxITEM_RADIO);
    GridSpacingMenu->Append(MenuItem17);
    MenuItem19 = new wxMenuItem(GridSpacingMenu, ID_MENUITEM_GRID_ICON_MEDIUM, _("Medium\tCTRL-2"), wxEmptyString, wxITEM_RADIO);
    GridSpacingMenu->Append(MenuItem19);
    MenuItem27 = new wxMenuItem(GridSpacingMenu, ID_MENUITEM_GRID_ICON_LARGE, _("Large\tCTRL-3"), wxEmptyString, wxITEM_RADIO);
    GridSpacingMenu->Append(MenuItem27);
    MenuItem28 = new wxMenuItem(GridSpacingMenu, ID_MENUITEM_GRID_ICON_XLARGE, _("Extra Large\tCTRL-4"), wxEmptyString, wxITEM_RADIO);
    GridSpacingMenu->Append(MenuItem28);
    MenuSettings->Append(ID_MENUITEM6, _("Grid Spacing"), GridSpacingMenu, wxEmptyString);
    MenuItem_Grid_Icon_Backgrounds = new wxMenu();
    MenuItemGridIconBackgroundOn = new wxMenuItem(MenuItem_Grid_Icon_Backgrounds, ID_MENUITEM_GRID_ICON_BACKGROUND_ON, _("On"), wxEmptyString, wxITEM_CHECK);
    MenuItem_Grid_Icon_Backgrounds->Append(MenuItemGridIconBackgroundOn);
    MenuItemGridIconBackgroundOn->Check(true);
    MenuItemGridIconBackgroundOff = new wxMenuItem(MenuItem_Grid_Icon_Backgrounds, ID_MENUITEM_GRID_ICON_BACKGROUND_OFF, _("Off"), wxEmptyString, wxITEM_CHECK);
    MenuItem_Grid_Icon_Backgrounds->Append(MenuItemGridIconBackgroundOff);
    MenuSettings->Append(ID_MENUITEM_Grid_Icon_Backgrounds, _("Grid Icon Backgrounds"), MenuItem_Grid_Icon_Backgrounds, wxEmptyString);
    MenuItem1 = new wxMenu();
    MenuItemGridNodeValuesOn = new wxMenuItem(MenuItem1, ID_MENUITEM_GRID_NODE_VALUES_ON, _("On"), wxEmptyString, wxITEM_CHECK);
    MenuItem1->Append(MenuItemGridNodeValuesOn);
    MenuItemGridNodeValuesOn->Check(true);
    MenuItemGridNodeValuesOff = new wxMenuItem(MenuItem1, ID_MENUITEM_GRID_NODE_VALUES_OFF, _("Off"), wxEmptyString, wxITEM_CHECK);
    MenuItem1->Append(MenuItemGridNodeValuesOff);
    MenuSettings->Append(ID_MENUITEM8, _("Grid Node Values"), MenuItem1, wxEmptyString);
    MenuItemRenderMode = new wxMenu();
    MenuItemRenderEraseMode = new wxMenuItem(MenuItemRenderMode, ID_MENU_CANVAS_ERASE_MODE, _("Erase Mode"), wxEmptyString, wxITEM_CHECK);
    MenuItemRenderMode->Append(MenuItemRenderEraseMode);
    MenuItemRenderEraseMode->Check(true);
    MenuItemRenderCanvasMode = new wxMenuItem(MenuItemRenderMode, ID_MENU_CANVAS_CANVAS_MODE, _("Canvas Mode"), wxEmptyString, wxITEM_CHECK);
    MenuItemRenderMode->Append(MenuItemRenderCanvasMode);
    MenuSettings->Append(ID_MENUITEM_RENDER_MODE, _("Render Mode"), MenuItemRenderMode, wxEmptyString)->Enable(false);
    MenuItem7 = new wxMenu();
    MenuItemEffectAssistAlwaysOn = new wxMenuItem(MenuItem7, ID_MENUITEM_EFFECT_ASSIST_ALWAYS_ON, _("Always On"), _("Effect Assist Window will always be active if member of perspective."), wxITEM_CHECK);
    MenuItem7->Append(MenuItemEffectAssistAlwaysOn);
    MenuItemEffectAssistAlwaysOff = new wxMenuItem(MenuItem7, ID_MENUITEM_EFFECT_ASSIST_ALWAYS_OFF, _("Always Off"), _("Effect Assist Window will always be inactive."), wxITEM_CHECK);
    MenuItem7->Append(MenuItemEffectAssistAlwaysOff);
    MenuItemEffectAssistToggleMode = new wxMenuItem(MenuItem7, ID_MENUITEM_EFFECT_ASSIST_TOGGLE, _("Toggle Mode"), _("Effect Assist Window will show only when a supported effect is selected."), wxITEM_CHECK);
    MenuItem7->Append(MenuItemEffectAssistToggleMode);
    MenuItemEffectAssistToggleMode->Check(true);
    MenuSettings->Append(ID_MENUITEM_EFFECT_ASSIST, _("Effect Assist Window"), MenuItem7, wxEmptyString);
    MenuItem13 = new wxMenuItem(MenuSettings, ID_MENUITEM5, _("Reset Toolbars"), wxEmptyString, wxITEM_NORMAL);
    MenuSettings->Append(MenuItem13);
    MenuBar->Append(MenuSettings, _("&Settings"));
    MenuHelp = new wxMenu();
    MenuItem4 = new wxMenuItem(MenuHelp, idMenuHelpContent, _("Content\tF1"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem4);
    MenuItem2 = new wxMenuItem(MenuHelp, wxID_ABOUT, _("About"), _("Show info about this application"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem2);
    MenuBar->Append(MenuHelp, _("&Help"));
    SetMenuBar(MenuBar);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -50, -35 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    DirDialog1 = new wxDirDialog(this, _("Select Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    Timer1.SetOwner(this, ID_TIMER1);
    FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("xLights Sequences(*.xseq)|*.xseq|\n\n\t\t\tLOR Music Sequences (*.lms)|*.lms|\n\n\t\t\tLOR Animation Sequences (*.las)|*.las|\n\n\t\t\tVixen Sequences (*.vix)|*.vix|\n\n\t\t\tFalcon Pi Player Sequences (*.fseq)|*.fseq|\n\n\t\t\tGlediator Record File (*.gled)|*.gled)|\n\n\t\t\tLynx Conductor Sequences (*.seq)|*.seq|\n\n\t\t\tHLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    MessageDialog1 = new wxMessageDialog(this, _("Hello"), _("Message"), wxOK|wxCANCEL, wxDefaultPosition);
    FileDialogPgoImage = new wxFileDialog(this, _("Select phoneme image file"), wxEmptyString, wxEmptyString, _("jpeg image(*.jpg)|*.jpg|\npng image(*.png)|*.png"), wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));

    Connect(ID_AUITOOLBAR_OPENSHOW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_AUITOOLBAR_NEWSEQUENCE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_AUITOOLBAR_OPEN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_AUITOOLBAR_SAVE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_AUITOOLBAR_SAVEAS,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonClickSaveAs);
    Connect(ID_AUITOOLBAR_RENDERALL,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemRenderAllClick);
    Connect(ID_AUITOOLBAR_PLAY_NOW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPlayButtonClick);
    Connect(ID_AUITOOLBAR_PAUSE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPauseButtonClick);
    Connect(ID_AUITOOLBAR_STOP,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemStopClick);
    Connect(ID_AUITOOLBAR_FIRST_FRAME,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarFirstFrameClick);
    Connect(ID_AUITOOLBAR_LAST_FRAME,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarLastFrameClick);
    Connect(ID_AUITOOLBAR_REPLAY_SECTION,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemReplaySectionClick);
    Connect(ID_AUITOOLBARITEM2,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_AUITOOLBARITEM5,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_AUITOOLBARITEM3,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideLayerTimingWindow);
    Connect(ID_AUITOOLBARITEM1,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_AUITOOLBARITEM4,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_AUITOOLBARITEM6,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemModelsClick);
    Connect(ID_AUITOOLBARITEM_ZOOM_IN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(ID_AUITOOLBARITEM_ZOOM_OUT,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_AUITOOLBARITEM14,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_BITMAPBUTTON_TAB_INFO,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(ID_BUTTON_STOP_NOW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonStopNowClick);
    Connect(ID_BUTTON_GRACEFUL_STOP,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonGracefulStopClick);
    Connect(ID_BUTTON_LIGHTS_OFF,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonLightsOffClick);
    Connect(ID_CHECKBOX_LIGHT_OUTPUT,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBoxLightOutputClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_BUTTON_CHANGE_MEDIA_DIR,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::ChangeMediaDirectory);
    Connect(ID_BITMAPBUTTON_Link_Dirs,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButton_Link_DirsClick);
    Connect(ID_BUTTON_SAVE_SETUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveSetupClick);
    Connect(ID_BUTTON_ADD_DONGLE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddDongleClick);
    Connect(ID_BUTTON_ADD_E131,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddE131Click);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddNullClick);
    Connect(ID_BUTTON_NETWORK_CHANGE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkChangeClick);
    Connect(ID_BUTTON_NETWORK_DELETE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkDeleteClick);
    Connect(ID_BUTTON_NETWORK_DELETE_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkDeleteAllClick);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveUpClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveDownClick);
    Connect(ID_LISTCTRL_NETWORKS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&xLightsFrame::OnGridNetworkBeginDrag);
    Connect(ID_LISTCTRL_NETWORKS,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xLightsFrame::OnGridNetworkItemActivated);
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
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnScaleImageCheckboxClick);
    Connect(ID_SLIDER_BACKGROUND_BRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated);
    Connect(ID_BUTTON_SELECT_MODEL_GROUPS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSelectModelGroupsClick);
    Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnListBoxElementListItemSelect);
    Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&xLightsFrame::OnListBoxElementListColumnClick);
    Connect(ID_BUTTON_MODELS_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonModelsPreviewClick);
    Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSavePreviewClick);
    Connect(ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlPreviewElementSizeText);
    Connect(ID_SLIDER3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated);
    Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlPreviewElementSizeText);
    Connect(ID_SLIDER_PREVIEW_SCALE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlModelRotationDegreesText);
    Connect(ID_SLIDER_PREVIEW_ROTATE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnSliderPreviewRotateCmdSliderUpdated);
    Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xLightsFrame::OnTextCtrlModelStartChannelText);
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
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanging);
    Connect(ID_NEW_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_OPEN_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Open_SequenceSelected);
    Connect(IS_SAVE_SEQ,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Save_SequenceSelected);
    Connect(ID_SAVE_AS_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected);
    Connect(ID_CLOSE_SEQ,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Close_SequenceSelected);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_FILE_BACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBackupSelected);
    Connect(wxID_EXIT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnQuit);
    Connect(idMenuSaveSched,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemSavePlaylistsSelected);
    Connect(idMenuAddList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemAddListSelected);
    Connect(idMenuRenameList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenameListSelected);
    Connect(idMenuDelList,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemDelListSelected);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRefreshSelected);
    Connect(idCustomScript,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemCustomScriptSelected);
    Connect(ID_MENUITEM_VIEW_ZOOM_IN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(ID_MENUITEM_VIEW_ZOOM_OUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_MENUITEM_SAVE_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemViewSavePerspectiveSelected);
    Connect(ID_MENUITEM_LOAD_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadEditPerspectiveSelected);
    Connect(ID_MENUITEM_DISPLAY_ELEMENTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideDisplayElementsWindow);
    Connect(ID_MENUITEM12,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_MENUITEM14,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_MENUITEM15,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_MENUITEM16,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideLayerTimingWindow);
    Connect(ID_MENUITEM17,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectDropper);
    Connect(ID_MENUITEM_EFFECT_ASSIST_WINDOW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectAssistWindow);
    Connect(ID_MENUITEM_WINDOWS_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHidePerspectivesWindow);
    Connect(ID_PLAY_FULL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_3_4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_IMPORT_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemImportEffects);
    Connect(ID_SEQ_SETTINGS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_RENDER_ON_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenderOnSave);
    Connect(ID_MENUITEM_ICON_SMALL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetToolIconSize);
    Connect(ID_MENUITEM_ICON_MEDIUM,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetToolIconSize);
    Connect(ID_MENUITEM_ICON_LARGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetToolIconSize);
    Connect(ID_MENUITEM_ICON_XLARGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetToolIconSize);
    Connect(ID_MENUITEM_GRID_ICON_SMALL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetIconSize);
    Connect(ID_MENUITEM_GRID_ICON_MEDIUM,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetIconSize);
    Connect(ID_MENUITEM_GRID_ICON_LARGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetIconSize);
    Connect(ID_MENUITEM_GRID_ICON_XLARGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetIconSize);
    Connect(ID_MENUITEM_GRID_ICON_BACKGROUND_ON,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnSetGridIconBackground);
    Connect(ID_MENUITEM_GRID_ICON_BACKGROUND_OFF,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnSetGridIconBackground);
    Connect(ID_MENUITEM_GRID_NODE_VALUES_ON,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnSetGridNodeValues);
    Connect(ID_MENUITEM_GRID_NODE_VALUES_OFF,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnSetGridNodeValues);
    Connect(ID_MENU_CANVAS_ERASE_MODE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenderEraseModeSelected);
    Connect(ID_MENU_CANVAS_CANVAS_MODE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRenderCanvasModeSelected);
    Connect(ID_MENUITEM_EFFECT_ASSIST_ALWAYS_ON,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemEffectAssistAlwaysOnSelected);
    Connect(ID_MENUITEM_EFFECT_ASSIST_ALWAYS_OFF,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemEffectAssistAlwaysOffSelected);
    Connect(ID_MENUITEM_EFFECT_ASSIST_TOGGLE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemEffectAssistToggleModeSelected);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ResetToolbarLocations);
    Connect(idMenuHelpContent,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(wxID_ABOUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnTimer1Trigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnClose);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnResize);
    //*)

    SetTitle( xlights_base_name + " (Ver " + xlights_version_string + ") " + xlights_build_date );

    CheckBoxLightOutput = new AUIToolbarButtonWrapper(OutputToolBar, ID_CHECKBOX_LIGHT_OUTPUT);
    ButtonGracefulStop = new AUIToolbarButtonWrapper(OutputToolBar, ID_BUTTON_GRACEFUL_STOP);

    mResetToolbars = false;
    mRenderOnSave = true;
    mIconSize = 16;

    selectedEffectPalette = "";
    selectedEffect = NULL;
    playStartTime = playEndTime = 0;
    replaySection = false;
    playType = 0;

    UnsavedRgbEffectsChanges = false;
    UnsavedPlaylistChanges = false;
    UnsavedNetworkChanges = false;

    CreateSequencer();

    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, PreviewModels, true);
    PreviewGLSizer->Add(modelPreview, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    modelPreview->Connect(wxEVT_PAINT,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewPaint,0,this);
    modelPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftDown,0,this);
    modelPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewLeftUp,0,this);
    modelPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewRightDown,0,this);
    modelPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseMove,0,this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnScrolledWindowPreviewMouseLeave,0,this);

    playIcon = wxBitmap(control_play_blue_icon);
    pauseIcon = wxBitmap(control_pause_blue_icon);

    Grid1HasFocus = false; //set this before grid gets any events -DJ

    SetIcon(wxIcon(xlights_xpm));
    SetName("xLights");
    wxPersistenceManager::Get().RegisterAndRestore(this);
    wxConfigBase* config = wxConfigBase::Get();

    effGridPrevX = 0;
    effGridPrevY = 0;
    m_creating_bound_rect = false;
    mSavedChangeCount = 0;

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

    itemCol.SetText(_T("Enabled"));
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    GridNetwork->InsertColumn(5, itemCol);

    GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(1,100);
    GridNetwork->SetColumnWidth(2,wxLIST_AUTOSIZE_USEHEADER);
    GridNetwork->SetColumnWidth(3,100);
    GridNetwork->SetColumnWidth(4,170);
    GridNetwork->SetColumnWidth(5,wxLIST_AUTOSIZE_USEHEADER);

    wxListItem elementCol;
	elementCol.SetText(_T("Element Name"));
	elementCol.SetImage(-1);
	elementCol.SetAlign(wxLIST_FORMAT_LEFT);
	ListBoxElementList->InsertColumn(0, elementCol);

	elementCol.SetText(_T("Start Chan"));
	elementCol.SetAlign(wxLIST_FORMAT_LEFT);
	ListBoxElementList->InsertColumn(1, elementCol);

	elementCol.SetText(_T("End Chan"));
	elementCol.SetAlign(wxLIST_FORMAT_LEFT);
	ListBoxElementList->InsertColumn(2, elementCol);
	ListBoxElementList->SetColumnWidth(0,10);
	ListBoxElementList->SetColumnWidth(1,10);
	ListBoxElementList->SetColumnWidth(2,10);

    // get list of most recently used directories
    wxString dir,mru_name;
    int idx;
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
        mru_MenuItem[i] = NULL;
    }
    dir.clear();
    bool ok = config->Read("LastDir", &dir);
    wxString ConvertDir;
    ConvertDir.clear();
    if (ok && !config->Read("ConvertDir", &ConvertDir))
    {
        ConvertDir=dir;
    }
    FileDialogConvert->SetDirectory(ConvertDir);

    if (ok && !config->Read(_("MediaDir"), &mediaDirectory))
    {
        mediaDirectory=dir;
    }
    MediaDirectoryLabel->SetLabel(mediaDirectory);

    wxString tbData = config->Read("ToolbarLocations");
   // wxMessageBox(tbData);
    if (tbData.StartsWith(TOOLBAR_SAVE_VERSION)) {
        MainAuiManager->LoadPerspective(tbData.Right(tbData.size() - 5));
    }
    config->Read("xLightsRenderOnSave", &mRenderOnSave, true);
    mRenderOnSaveMenuItem->Check(mRenderOnSave);
    config->Read("xLightsIconSize", &mIconSize, 16);
    if (mIconSize != 16) {
        int id = ID_MENUITEM_ICON_MEDIUM;
        if (mIconSize == 32) {
            id = ID_MENUITEM_ICON_LARGE;
        } else if (mIconSize == 48) {
            id = ID_MENUITEM_ICON_XLARGE;
        }
        wxCommandEvent event(wxEVT_NULL, id);
        SetToolIconSize(event);
    }
    config->Read("xLightsGridSpacing", &mGridSpacing, 16);
    if (mGridSpacing != 16) {
        int id = ID_MENUITEM_GRID_ICON_MEDIUM;
        if (mGridSpacing == 32) {
            id = ID_MENUITEM_GRID_ICON_LARGE;
        } else if (mGridSpacing == 48) {
            id = ID_MENUITEM_GRID_ICON_XLARGE;
        }
        wxCommandEvent event(wxEVT_NULL, id);
        SetIconSize(event);
    }
    config->Read("xLightsGridIconBackgrounds", &mGridIconBackgrounds, true);
    {
        int id = mGridIconBackgrounds ? ID_MENUITEM_GRID_ICON_BACKGROUND_ON : ID_MENUITEM_GRID_ICON_BACKGROUND_OFF;
        wxCommandEvent event(wxEVT_NULL, id);
        OnSetGridIconBackground(event);
    }
    config->Read("xLightsGridNodeValues", &mGridNodeValues, true);
    {
        int id = mGridNodeValues ? ID_MENUITEM_GRID_NODE_VALUES_ON : ID_MENUITEM_GRID_NODE_VALUES_OFF;
        wxCommandEvent event(wxEVT_NULL, id);
        OnSetGridNodeValues(event);
    }
    config->Read("xLightsEffectAssistMode", &mEffectAssistMode, EFFECT_ASSIST_TOGGLE_MODE);
    MenuItemEffectAssistAlwaysOn->Check(mEffectAssistMode==EFFECT_ASSIST_ALWAYS_ON);
    MenuItemEffectAssistAlwaysOff->Check(mEffectAssistMode==EFFECT_ASSIST_ALWAYS_OFF);
    MenuItemEffectAssistToggleMode->Check(mEffectAssistMode==EFFECT_ASSIST_TOGGLE_MODE);

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
    PictureEffectDirections.Add("vector"); //20

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
//    PianoKeyPlacement.Add("Bottom right");#include "../include/link-48.xpm"


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
    else
    {
        PromptForShowDirectory();
    }
    MixTypeChanged=true;
    HtmlEasyPrint=new wxHtmlEasyPrinting("xLights Printing", this);
    basic.setFrame(this);
    PlayerDlg = new PlayerFrame(this, ID_PLAYER_DIALOG);

    EffectNames=EffectsPanel1->EffectChoicebook->GetChoiceCtrl()->GetStrings();
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

    //start out with 50ms timer, once we load a file or create a new one, we'll reset
    //to whatever the timing that is selected
    Timer1.Start(50, wxTIMER_CONTINUOUS);
    EffectTreeDlg = NULL;

//    ConnectOnChar(PaneNutcracker);
//    ConnectOnChar(Panel1); //add hot keys to upper panel as well -DJ

  jobPool.Start(wxThread::GetCPUCount() * 4);
 //   jobPool.Start(24);
}

xLightsFrame::~xLightsFrame()
{
    Timer1.Stop();
    selectedEffect = NULL;

    wxConfigBase* config = wxConfigBase::Get();
    if (mResetToolbars) {
        config->DeleteEntry("ToolbarLocations");
    } else {
        config->Write("ToolbarLocations", TOOLBAR_SAVE_VERSION + MainAuiManager->SavePerspective());
    }
    config->Write("xLightsIconSize", mIconSize);
    config->Write("xLightsGridSpacing", mGridSpacing);
    config->Write("xLightsGridIconBackgrounds", mGridIconBackgrounds);
    config->Write("xLightsGridNodeValues", mGridNodeValues);
    config->Write("xLightsRenderOnSave", mRenderOnSave);
    config->Write("xLightsEffectAssistMode", mEffectAssistMode);

    config->Flush();

    wxFileName kbf;
    kbf.AssignDir(CurrentDir);
    kbf.SetFullName("xlights_keybindings.xml");
    mainSequencer->keyBindings.Save(kbf);

    //must call these or the app will crash on exit
    m_mgr->UnInit();
    MainAuiManager->UnInit();

    if( CurrentSeqXmlFile )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = NULL;
    }

    delete CheckBoxLightOutput;
    delete ButtonGracefulStop;

    //(*Destroy(xLightsFrame)
    //*)
}

void xLightsFrame::OnQuit(wxCommandEvent& event)
{
    wxCloseEvent evt;
    if (QuitMenuItem->IsEnabled()) {
        OnClose(evt);
    }
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

    panel->Choice_Ripple_Movement->Set(RippleMovement);
    panel->Choice_Ripple_Movement->SetSelection(0);
    panel->Choice_Ripple_Object_To_Draw->Set(RippleObjectToDraw);
    panel->Choice_Ripple_Object_To_Draw->SetSelection(0);


}


void xLightsFrame::OnAbout(wxCommandEvent& event)
{
    wxString hdg = wxString::Format(_("About xLights %s"),xlights_version_string);
    wxMessageBox(XLIGHTS_LICENSE, hdg);
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
    case NEWSEQUENCER:
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
    case NEWSEQUENCER:
        caption=_("Sequencer Tab");
        msg=_("The Sequencer tab can be used to create RGB sequences. First, create a model of your RGB display element(s) by clicking on the Models button. Then try the different effects and settings until you create something you like. You can save the settings as a preset by clicking the New Preset button. From then on, that preset will be available in the presets drop-down list. You can combine effects by creating a second effect in the Effect 2 area, then choosing a Layering Method. To create a series of effects that will be used in a sequence, click the open file icon to open an xLights (.xseq) sequence. Choose which display elements/models you will use in this sequence. Then click the insert rows icon and type in the start time in seconds when that effect should begin. Rows will automatically sort by start time. To add an effect to the sequence, click on the grid cell in the desired display model column and the desired start time row, then click the Update button. When you are done creating effects for the sequence, click the save icon and the xLights sequence will be updated with the effects you stored in the grid.");
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

void xLightsFrame::ShowHideAllSequencerWindows(bool show)
{
    wxAuiPaneInfoArray &info = m_mgr->GetAllPanes();
    bool update = false;
    if (show) {
        for (int x = 0; x < info.size(); x++) {
            if (x < savedPaneShown.size() && info[x].IsFloating() && !info[x].IsShown()
                && savedPaneShown[x]) {
                info[x].Show();
                savedPaneShown[x] = true;
                update = true;
            }
        }
    } else {
        savedPaneShown.resize(info.size());
        for (int x = 0; x < info.size(); x++) {
            savedPaneShown[x] = info[x].IsShown();
            if (info[x].IsFloating() && info[x].IsShown()) {
                info[x].Hide();
                update = true;
            }
        }
    }
    if (update) {
        m_mgr->Update();
    }
}


void xLightsFrame::OnNotebook1PageChanging(wxAuiNotebookEvent& event)
{
    if (event.GetOldSelection() == NEWSEQUENCER) {
        ShowHideAllSequencerWindows(false);
    }
}

void xLightsFrame::OnNotebook1PageChanged1(wxAuiNotebookEvent& event)
{
    heartbeat("tab change", true); //tell fido to stop watching -DJ
    int pagenum=event.GetSelection(); //Notebook1->GetSelection();
    if (pagenum == TESTTAB && !xout)
    {
        StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
    }
    else if (pagenum == PREVIEWTAB)
    {
        modelPreview->InitializePreview(mBackgroundImage,mBackgroundBrightness);
        modelPreview->SetScaleBackgroundImage(mScaleBackgroundImage);
    }
    else if (pagenum == NEWSEQUENCER)
    {
        InitSequencer();
        ShowHideAllSequencerWindows(true);
    }
    else if (pagenum == CONVERTTAB)
    {
        wxArrayString ModelNames;
        GetModelNames(ModelNames);
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
            else if (SeqData.NumFrames() > 0)
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
                bool enabled = e->GetAttribute("Enabled", "Yes") == "Yes";
                static wxString choices;

                int numU = wxAtoi(e->GetAttribute("NumUniverses", "1"));

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
                    xout->addnetwork(NetworkType,MaxChan,ComPort,baud, numU, enabled);
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
    ButtonAddNull->Enable(flag);
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
    PlayerDlg->Stop();
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
    case NEWSEQUENCER:
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
        break;
    }
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
std::vector<ModelClass*> xLightsFrame::PreviewModels;
std::map<wxString, ModelClassPtr> xLightsFrame::AllModels;
xLightsXmlFile* xLightsFrame::CurrentSeqXmlFile = NULL;

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
    wxLogDebug("xLightsFrame::OnClose");

    if (!CloseSequence()) {
        event.Veto();
        return;
    }
    selectedEffect = NULL;

    CheckUnsavedChanges();

    ShowHideAllSequencerWindows(false);

    StopNow();

    heartbeat("exit", true); //tell fido about graceful exit -DJ
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


//#define isRandom(ctl)  (buttonState[std::string(ctl->GetName())] == Random)
bool xLightsFrame::isRandom_(wxControl* ctl, const char*debug)
{
//    if (!ctl->GetName().length()) djdebug("NO NAME FOR %s", debug);
    bool retval = (buttonState[std::string(ctl->GetName())] != Locked); //== Random);
//    djdebug("isRandom(%s) = %d", (const char*)ctl->GetName().c_str(), retval);
    return retval;
}

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
    dialog.TextCtrl_PreviewWidth->SetValue(wxString::Format("%d",modelPreview->GetVirtualCanvasWidth()));
    dialog.TextCtrl_PreviewHeight->SetValue(wxString::Format("%d",modelPreview->GetVirtualCanvasHeight()));
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
                UnsavedRgbEffectsChanges=true;
            }
        }
    }
}
void xLightsFrame::SetPreviewSize(int width,int height)
{
    StaticTextCurrentPreviewSize->SetLabelText(wxString::Format("Size: %d x %d",width,height));
    SetXmlSetting("previewWidth",wxString::Format("%d",width));
    SetXmlSetting("previewHeight",wxString::Format("%d",height));
    modelPreview->SetCanvasSize(width,height);
    modelPreview->Refresh();
    sPreview2->SetVirtualCanvasSize(width, height);
    sPreview2->Refresh();
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

void xLightsFrame::OnButtonClickSaveAs(wxCommandEvent& event)
{
    SaveAsSequence();
}

wxString xLightsFrame::GetSeqXmlFileName() {
    if (CurrentSeqXmlFile == NULL) {
        return "";
    }
    return CurrentSeqXmlFile->GetFullPath();
}

void xLightsFrame::OnMenu_Settings_SequenceSelected(wxCommandEvent& event)
{
    if( xLightsFrame::CurrentSeqXmlFile == NULL ) return;
    // populate dialog
    SeqSettingsDialog dialog(this, xLightsFrame::CurrentSeqXmlFile, mediaDirectory, wxEmptyString);
    dialog.Fit();
    if (dialog.ShowModal() != wxID_OK) return;  // user pressed cancel
}


void xLightsFrame::OnAuiToolBarItemPlayButtonClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::EnableToolbarButton(wxAuiToolBar* toolbar,int id, bool enable)
{
    wxAuiToolBarItem* button = toolbar->FindTool(id);
    int state = enable?wxAUI_BUTTON_STATE_NORMAL:wxAUI_BUTTON_STATE_DISABLED;
    button->SetState(state);
    toolbar->Refresh();
}

void xLightsFrame::OnAuiToolBarItemPauseButtonClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemStopClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        //playStartTime = playEndTime = 0;
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarFirstFrameClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_FIRST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarLastFrameClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_LAST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_REPLAY_SECTION);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemZoominClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        mainSequencer->PanelTimeLine->ZoomIn();
    }
}

void xLightsFrame::OnAuiToolBarItem_ZoomOutClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        mainSequencer->PanelTimeLine->ZoomOut();
    }
}


void xLightsFrame::OnMenuItem_File_Open_SequenceSelected(wxCommandEvent& event)
{
    OpenSequence();
}

void xLightsFrame::OnMenuItem_File_Save_SequenceSelected(wxCommandEvent& event)
{
    SaveSequence();
}

void xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected(wxCommandEvent& event)
{
    SaveAsSequence();
}

void xLightsFrame::OnMenuItem_File_Close_SequenceSelected(wxCommandEvent& event)
{
    CloseSequence();

    // force refreshes since grid has been cleared
    mainSequencer->PanelTimeLine->RaiseChangeTimeline();
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::OnResize(wxSizeEvent& event)
{
}

void xLightsFrame::OnAuiToolBarItemRenderAllClick(wxCommandEvent& event)
{
    RenderAll();
}

bool AUIToolbarButtonWrapper::IsChecked() {
    return toolbar->GetToolToggled(id);
}
void AUIToolbarButtonWrapper::SetValue(bool b) {
    toolbar->ToggleTool(id, b);
}
void AUIToolbarButtonWrapper::Enable(bool b) {
    toolbar->EnableTool(id, b);
}

void xLightsFrame::SetIconSize(wxCommandEvent& event)
{
    int size = 48;
    if (event.GetId() == ID_MENUITEM_GRID_ICON_SMALL) {
        size = 16;
    } else if (event.GetId() == ID_MENUITEM_GRID_ICON_MEDIUM) {
        size = 24;
    } else if (event.GetId() == ID_MENUITEM_GRID_ICON_LARGE) {
        size = 32;
    }
    mGridSpacing = size;

    DEFAULT_ROW_HEADING_HEIGHT = size + 6;
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
    GridSpacingMenu->Check(event.GetId(), true);
}

void xLightsFrame::ResetToolbarLocations(wxCommandEvent& event)
{
    wxMessageBox("Toolbar locations will reset to defaults upon restart.");
    mResetToolbars = true;
}

void xLightsFrame::SetToolIconSize(wxCommandEvent& event)
{
    int size = 48;
    if (event.GetId() == ID_MENUITEM_ICON_SMALL) {
        size = 16;
    } else if (event.GetId() == ID_MENUITEM_ICON_MEDIUM) {
        size = 24;
    } else if (event.GetId() == ID_MENUITEM_ICON_LARGE) {
        size = 32;
    }

    mIconSize = size;
    for (int x = 0; x < EffectsToolBar->GetToolCount(); x++) {
        EffectsToolBar->FindToolByIndex(x)->SetMinSize(wxSize(size, size));
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetSizeHints(size, size, size, size);
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetMinSize(wxSize(size, size));
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetMaxSize(wxSize(size, size));
    }
    EffectsToolBar->Realize();
    wxSize sz = EffectsToolBar->GetSize();
    wxAuiPaneInfo &info = MainAuiManager->GetPane("EffectsToolBar");
    info.BestSize(sz);
    MainAuiManager->Update();

    const wxWindowList& lst =effectPalettePanel->GetChildren();
    for (int x = 0; x < lst.size(); x++) {
        lst[x]->SetSizeHints(size, size, size, size);
    }
    effectPalettePanel->Layout();
    ToolIconSizeMenu->Check(event.GetId(), true);
}

void xLightsFrame::OnMenuItemRenderEraseModeSelected(wxCommandEvent& event)
{
    MenuItemRenderEraseMode->Check(true);
    MenuItemRenderCanvasMode->Check(false);
    CurrentSeqXmlFile->SetRenderMode(xLightsXmlFile::ERASE_MODE);
}

void xLightsFrame::OnMenuItemRenderCanvasModeSelected(wxCommandEvent& event)
{
    MenuItemRenderEraseMode->Check(false);
    MenuItemRenderCanvasMode->Check(true);
    CurrentSeqXmlFile->SetRenderMode(xLightsXmlFile::CANVAS_MODE);
}

void xLightsFrame::UpdateRenderMode()
{
    if( CurrentSeqXmlFile->GetRenderMode() == xLightsXmlFile::CANVAS_MODE )
    {
        MenuItemRenderEraseMode->Check(false);
        MenuItemRenderCanvasMode->Check(true);
    }
    else
    {
        MenuItemRenderEraseMode->Check(true);
        MenuItemRenderCanvasMode->Check(false);
    }
}

void xLightsFrame::SetFrequency(int frequency)
{
    mSequenceElements.SetFrequency(frequency);
    mainSequencer->PanelTimeLine->SetTimeFrequency(frequency);
    mainSequencer->PanelWaveForm->SetTimeFrequency(frequency);
}

void xLightsFrame::OnSetGridIconBackground(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENUITEM_GRID_ICON_BACKGROUND_ON) {
        mGridIconBackgrounds = true;
    } else if (event.GetId() == ID_MENUITEM_GRID_ICON_BACKGROUND_OFF) {
        mGridIconBackgrounds = false;
    }
    MenuItemGridIconBackgroundOn->Check(mGridIconBackgrounds);
    MenuItemGridIconBackgroundOff->Check(!mGridIconBackgrounds);
    mainSequencer->PanelEffectGrid->SetEffectIconBackground(mGridIconBackgrounds);
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::OnSetGridNodeValues(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENUITEM_GRID_NODE_VALUES_ON) {
        mGridNodeValues = true;
    } else if (event.GetId() == ID_MENUITEM_GRID_NODE_VALUES_OFF) {
        mGridNodeValues = false;
    }
    MenuItemGridNodeValuesOn->Check(mGridNodeValues);
    MenuItemGridNodeValuesOff->Check(!mGridNodeValues);
    mainSequencer->PanelEffectGrid->SetEffectNodeValues(mGridNodeValues);
    mainSequencer->PanelEffectGrid->Refresh();
}



void xLightsFrame::SetPlaySpeed(wxCommandEvent& event)
{
    double playSpeed = 1.0;
    if (event.GetId() == ID_PLAY_FULL) {
        playSpeed = 1.0;
    } else if (event.GetId() == ID_PLAY_3_4) {
        playSpeed = 0.75;
    } else if (event.GetId() == ID_PLAY_1_2) {
        playSpeed = 0.5;
    } else if (event.GetId() == ID_PLAY_1_4) {
        playSpeed = 0.25;
    }
    PlayerDlg->SetPlaybackRate(playSpeed);
}

void xLightsFrame::OnBitmapButton_Link_DirsClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    long LinkFlag=0;
    config->Read(_("LinkFlag"), &LinkFlag);
    if( LinkFlag ) {
        LinkFlag = 0;
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_UNLINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(true);
    } else {
        LinkFlag = 1;
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(false);
        mediaDirectory = CurrentDir;
        config->Write(_("MediaDir"), mediaDirectory);
        MediaDirectoryLabel->SetLabel(mediaDirectory);
        MediaDirectoryLabel->GetParent()->Layout();
    }
    config->Write(_("LinkFlag"), LinkFlag);
}

void xLightsFrame::OnAuiToolBarItemModelsClick(wxCommandEvent& event)
{
    ShowModelsDialog();
}

void xLightsFrame::OnMenuItemRenderOnSave(wxCommandEvent& event)
{
    mRenderOnSave = event.IsChecked();
}

void xLightsFrame::OnMenuItemEffectAssistAlwaysOnSelected(wxCommandEvent& event)
{
    MenuItemEffectAssistAlwaysOn->Check(true);
    MenuItemEffectAssistAlwaysOff->Check(false);
    MenuItemEffectAssistToggleMode->Check(false);
    mEffectAssistMode = EFFECT_ASSIST_ALWAYS_ON;
    SetEffectAssistWindowState(true);
}

void xLightsFrame::OnMenuItemEffectAssistAlwaysOffSelected(wxCommandEvent& event)
{
    MenuItemEffectAssistAlwaysOn->Check(false);
    MenuItemEffectAssistAlwaysOff->Check(true);
    MenuItemEffectAssistToggleMode->Check(false);
    mEffectAssistMode = EFFECT_ASSIST_ALWAYS_OFF;
    SetEffectAssistWindowState(false);
}

void xLightsFrame::OnMenuItemEffectAssistToggleModeSelected(wxCommandEvent& event)
{
    MenuItemEffectAssistAlwaysOn->Check(false);
    MenuItemEffectAssistAlwaysOff->Check(false);
    MenuItemEffectAssistToggleMode->Check(true);
    mEffectAssistMode = EFFECT_ASSIST_TOGGLE_MODE;
}

void xLightsFrame::SetEffectAssistWindowState(bool show)
{
    bool visible = m_mgr->GetPane("EffectAssist").IsShown();
    if (visible && !show) {
        m_mgr->GetPane("EffectAssist").Hide();
        m_mgr->Update();
    } else if(!visible && show) {
        m_mgr->GetPane("EffectAssist").Show();
        m_mgr->Update();
    }
}

bool xLightsFrame::EffectAssistSupported(Effect* effect)
{
    wxString name = effect->GetEffectName();
    if( name == "Morph" || name == "Pictures" )
    {
        return true;
    }
    return false;
}

void xLightsFrame::UpdateEffectAssistWindow(Effect* effect)
{
    if( effect == NULL )
    {
        sEffectAssist->SetEffect(NULL);
        return;
    }

    bool effect_is_supported = EffectAssistSupported(effect);

    if( mEffectAssistMode == EFFECT_ASSIST_TOGGLE_MODE )
    {
        if( effect_is_supported )
        {
            SetEffectAssistWindowState(true);
        }
        else
        {
            SetEffectAssistWindowState(false);
        }
    }

    if( effect_is_supported )
    {
        sEffectAssist->SetEffect(effect);
    }
    else
    {
        sEffectAssist->SetEffect(NULL);
    }
}

void xLightsFrame::CheckUnsavedChanges()
{
    if ( UnsavedNetworkChanges && wxYES == wxMessageBox("Save Network Setup changes?",
                                                        "Networks Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT))
    {
        SaveNetworksFile();
    }

    if ( UnsavedPlaylistChanges && wxYES == wxMessageBox("Save Scheduler/Playlist changes?",
                                                         "Scheduler Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT))
    {
        SaveScheduleFile();
    }

    if ( UnsavedRgbEffectsChanges && wxYES == wxMessageBox("Save Models, Views, Perspectives, and Preset changes?",
                                                           "RGB Effects File Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT))
    {
        SaveEffectsFile();
    }
}
