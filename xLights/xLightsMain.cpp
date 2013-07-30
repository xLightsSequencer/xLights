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
const long xLightsFrame::ID_SCROLLEDWINDOW1 = wxNewId();
const long xLightsFrame::ID_BUTTON13 = wxNewId();
const long xLightsFrame::ID_BUTTON3 = wxNewId();
const long xLightsFrame::ID_BUTTON58 = wxNewId();
const long xLightsFrame::ID_CHOICE7 = wxNewId();
const long xLightsFrame::ID_BUTTON59 = wxNewId();
const long xLightsFrame::ID_CHOICE2 = wxNewId();
const long xLightsFrame::ID_BUTTON9 = wxNewId();
const long xLightsFrame::ID_BUTTON8 = wxNewId();
const long xLightsFrame::ID_STATICTEXT23 = wxNewId();
const long xLightsFrame::ID_CHOICE_LayerMethod = wxNewId();
const long xLightsFrame::ID_STATICTEXT24 = wxNewId();
const long xLightsFrame::ID_SLIDER_SparkleFrequency = wxNewId();
const long xLightsFrame::ID_STATICTEXT127 = wxNewId();
const long xLightsFrame::ID_SLIDER_Brightness = wxNewId();
const long xLightsFrame::ID_STATICTEXT128 = wxNewId();
const long xLightsFrame::ID_SLIDER_Contrast = wxNewId();
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
const long xLightsFrame::ID_BUTTON4 = wxNewId();
const long xLightsFrame::ID_GRID1 = wxNewId();
const long xLightsFrame::ID_PANEL4 = wxNewId();
const long xLightsFrame::ID_STATICTEXT25 = wxNewId();
const long xLightsFrame::ID_SLIDER_Bars1_BarCount = wxNewId();
const long xLightsFrame::ID_STATICTEXT26 = wxNewId();
const long xLightsFrame::ID_CHOICE_Bars1_Direction = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Bars1_Highlight = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Bars1_3D = wxNewId();
const long xLightsFrame::ID_PANEL8 = wxNewId();
const long xLightsFrame::ID_STATICTEXT27 = wxNewId();
const long xLightsFrame::ID_CHOICE_Butterfly1_Colors = wxNewId();
const long xLightsFrame::ID_STATICTEXT28 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly1_Style = wxNewId();
const long xLightsFrame::ID_STATICTEXT30 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly1_Chunks = wxNewId();
const long xLightsFrame::ID_STATICTEXT31 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly1_Skip = wxNewId();
const long xLightsFrame::ID_PANEL10 = wxNewId();
const long xLightsFrame::ID_STATICTEXT21 = wxNewId();
const long xLightsFrame::ID_SLIDER_ColorWash1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT32 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_ColorWash1_HFade = wxNewId();
const long xLightsFrame::ID_STATICTEXT33 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_ColorWash1_VFade = wxNewId();
const long xLightsFrame::ID_PANEL5 = wxNewId();
const long xLightsFrame::ID_STATICTEXT84 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fire1_Height = wxNewId();
const long xLightsFrame::ID_STATICTEXT133 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fire1_HueShift = wxNewId();
const long xLightsFrame::ID_STATICTEXT135 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Fire1_GrowFire = wxNewId();
const long xLightsFrame::ID_PANEL12 = wxNewId();
const long xLightsFrame::ID_STATICTEXT34 = wxNewId();
const long xLightsFrame::ID_SLIDER_Garlands1_Type = wxNewId();
const long xLightsFrame::ID_STATICTEXT35 = wxNewId();
const long xLightsFrame::ID_SLIDER_Garlands1_Spacing = wxNewId();
const long xLightsFrame::ID_PANEL14 = wxNewId();
const long xLightsFrame::ID_STATICTEXT36 = wxNewId();
const long xLightsFrame::ID_SLIDER_Life1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT37 = wxNewId();
const long xLightsFrame::ID_SLIDER_Life1_Seed = wxNewId();
const long xLightsFrame::ID_PANEL16 = wxNewId();
const long xLightsFrame::ID_STATICTEXT39 = wxNewId();
const long xLightsFrame::ID_CHOICE_Meteors1_Type = wxNewId();
const long xLightsFrame::ID_STATICTEXT129 = wxNewId();
const long xLightsFrame::ID_CHOICE_Meteors1_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT41 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT43 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors1_Length = wxNewId();
const long xLightsFrame::ID_STATICTEXT131 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors1_Swirl_Intensity = wxNewId();
const long xLightsFrame::ID_STATICTEXT106 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Meteors1_FallUp = wxNewId();
const long xLightsFrame::ID_PANEL18 = wxNewId();
const long xLightsFrame::ID_BUTTON_PICTURES1_FILENAME = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Pictures1_Filename = wxNewId();
const long xLightsFrame::ID_STATICTEXT46 = wxNewId();
const long xLightsFrame::ID_CHOICE_Pictures1_Direction = wxNewId();
const long xLightsFrame::ID_STATICTEXT97 = wxNewId();
const long xLightsFrame::ID_SLIDER_Pictures1_GifSpeed = wxNewId();
const long xLightsFrame::ID_PANEL20 = wxNewId();
const long xLightsFrame::ID_STATICTEXT80 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowflakes1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT81 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowflakes1_Type = wxNewId();
const long xLightsFrame::ID_PANEL22 = wxNewId();
const long xLightsFrame::ID_STATICTEXT45 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowstorm1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT51 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowstorm1_Length = wxNewId();
const long xLightsFrame::ID_PANEL24 = wxNewId();
const long xLightsFrame::ID_STATICTEXT38 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT40 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals1_Rotation = wxNewId();
const long xLightsFrame::ID_STATICTEXT42 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals1_Thickness = wxNewId();
const long xLightsFrame::ID_STATICTEXT44 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals1_Direction = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirals1_Blend = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirals1_3D = wxNewId();
const long xLightsFrame::ID_PANEL26 = wxNewId();
const long xLightsFrame::ID_STATICTEXT53 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text1_Line1 = wxNewId();
const long xLightsFrame::ID_STATICTEXT54 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text1_Line2 = wxNewId();
const long xLightsFrame::ID_BUTTON_TEXT1_1_FONT = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text1_1_Font = wxNewId();
const long xLightsFrame::ID_STATICTEXT79 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_1_Dir = wxNewId();
const long xLightsFrame::ID_STATICTEXT108 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_1_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT99 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT101 = wxNewId();
const long xLightsFrame::ID_SLIDER_Text1_1_Position = wxNewId();
const long xLightsFrame::ID_PANEL_Text1_1 = wxNewId();
const long xLightsFrame::ID_BUTTON_TEXT1_2_FONT = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text1_2_Font = wxNewId();
const long xLightsFrame::ID_STATICTEXT57 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_2_Dir = wxNewId();
const long xLightsFrame::ID_STATICTEXT109 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_2_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT104 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text1_2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT103 = wxNewId();
const long xLightsFrame::ID_SLIDER_Text1_2_Position = wxNewId();
const long xLightsFrame::ID_PANEL_Text1_2 = wxNewId();
const long xLightsFrame::ID_NOTEBOOK_Text1 = wxNewId();
const long xLightsFrame::ID_PANEL7 = wxNewId();
const long xLightsFrame::ID_STATICTEXT86 = wxNewId();
const long xLightsFrame::ID_SLIDER_Twinkle1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT105 = wxNewId();
const long xLightsFrame::ID_SLIDER_Twinkle1_Steps = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Twinkle1_Strobe = wxNewId();
const long xLightsFrame::ID_PANEL33 = wxNewId();
const long xLightsFrame::ID_STATICTEXT87 = wxNewId();
const long xLightsFrame::ID_SLIDER_Tree1_Branches = wxNewId();
const long xLightsFrame::ID_PANEL34 = wxNewId();
const long xLightsFrame::ID_STATICTEXT88 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph1_R = wxNewId();
const long xLightsFrame::ID_STATICTEXT89 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph1_r = wxNewId();
const long xLightsFrame::ID_STATICTEXT90 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph1_d = wxNewId();
const long xLightsFrame::ID_STATICTEXT93 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirograph1_Animate = wxNewId();
const long xLightsFrame::ID_STATICTEXT91 = wxNewId();
const long xLightsFrame::ID_PANEL35 = wxNewId();
const long xLightsFrame::ID_STATICTEXT95 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks1_Number_Explosions = wxNewId();
const long xLightsFrame::ID_STATICTEXT92 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT94 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks1_Velocity = wxNewId();
const long xLightsFrame::ID_STATICTEXT96 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks1_Fade = wxNewId();
const long xLightsFrame::ID_PANEL36 = wxNewId();
const long xLightsFrame::ID_STATICTEXT137 = wxNewId();
const long xLightsFrame::ID_SLIDER_Circles1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT138 = wxNewId();
const long xLightsFrame::ID_SLIDER_Circles1_Size = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles1_Bounce = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles1_Collide = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles1_Random_m = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles1_Radial = wxNewId();
const long xLightsFrame::ID_PANEL37 = wxNewId();
const long xLightsFrame::ID_STATICTEXT141 = wxNewId();
const long xLightsFrame::ID_SLIDER_Piano1_Keyboard = wxNewId();
const long xLightsFrame::ID_PANEL39 = wxNewId();
const long xLightsFrame::ID_CHOICEBOOK1 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_1 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_1 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_2 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_2 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_3 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_3 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_4 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_4 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_5 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_5 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette1_6 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette1_6 = wxNewId();
const long xLightsFrame::ID_STATICTEXT61 = wxNewId();
const long xLightsFrame::ID_SLIDER_Speed1 = wxNewId();
const long xLightsFrame::ID_PANEL6 = wxNewId();
const long xLightsFrame::ID_STATICTEXT47 = wxNewId();
const long xLightsFrame::ID_SLIDER_Bars2_BarCount = wxNewId();
const long xLightsFrame::ID_STATICTEXT48 = wxNewId();
const long xLightsFrame::ID_CHOICE_Bars2_Direction = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Bars2_Highlight = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Bars2_3D = wxNewId();
const long xLightsFrame::ID_PANEL9 = wxNewId();
const long xLightsFrame::ID_STATICTEXT49 = wxNewId();
const long xLightsFrame::ID_CHOICE_Butterfly2_Colors = wxNewId();
const long xLightsFrame::ID_STATICTEXT50 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly2_Style = wxNewId();
const long xLightsFrame::ID_STATICTEXT52 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly2_Chunks = wxNewId();
const long xLightsFrame::ID_STATICTEXT55 = wxNewId();
const long xLightsFrame::ID_SLIDER_Butterfly2_Skip = wxNewId();
const long xLightsFrame::ID_PANEL11 = wxNewId();
const long xLightsFrame::ID_STATICTEXT22 = wxNewId();
const long xLightsFrame::ID_SLIDER_ColorWash2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT56 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_ColorWash2_HFade = wxNewId();
const long xLightsFrame::ID_STATICTEXT58 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_ColorWash2_VFade = wxNewId();
const long xLightsFrame::ID_PANEL13 = wxNewId();
const long xLightsFrame::ID_STATICTEXT85 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fire2_Height = wxNewId();
const long xLightsFrame::ID_STATICTEXT134 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fire2_HueShift = wxNewId();
const long xLightsFrame::ID_STATICTEXT136 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Fire2_GrowFire = wxNewId();
const long xLightsFrame::ID_PANEL15 = wxNewId();
const long xLightsFrame::ID_STATICTEXT59 = wxNewId();
const long xLightsFrame::ID_SLIDER_Garlands2_Type = wxNewId();
const long xLightsFrame::ID_STATICTEXT60 = wxNewId();
const long xLightsFrame::ID_SLIDER_Garlands2_Spacing = wxNewId();
const long xLightsFrame::ID_PANEL17 = wxNewId();
const long xLightsFrame::ID_STATICTEXT62 = wxNewId();
const long xLightsFrame::ID_SLIDER_Life2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT63 = wxNewId();
const long xLightsFrame::ID_SLIDER_Life2_Seed = wxNewId();
const long xLightsFrame::ID_PANEL19 = wxNewId();
const long xLightsFrame::ID_STATICTEXT64 = wxNewId();
const long xLightsFrame::ID_CHOICE_Meteors2_Type = wxNewId();
const long xLightsFrame::ID_STATICTEXT130 = wxNewId();
const long xLightsFrame::ID_CHOICE_Meteors2_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT65 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT66 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors2_Length = wxNewId();
const long xLightsFrame::ID_STATICTEXT132 = wxNewId();
const long xLightsFrame::ID_SLIDER_Meteors2_Swirl_Intensity = wxNewId();
const long xLightsFrame::ID_STATICTEXT107 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Meteors2_FallUp = wxNewId();
const long xLightsFrame::ID_PANEL21 = wxNewId();
const long xLightsFrame::ID_BUTTON_PICTURES2_FILENAME = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Pictures2_Filename = wxNewId();
const long xLightsFrame::ID_STATICTEXT67 = wxNewId();
const long xLightsFrame::ID_CHOICE_Pictures2_Direction = wxNewId();
const long xLightsFrame::ID_STATICTEXT98 = wxNewId();
const long xLightsFrame::ID_SLIDER_Pictures2_GifSpeed = wxNewId();
const long xLightsFrame::ID_PANEL23 = wxNewId();
const long xLightsFrame::ID_STATICTEXT82 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowflakes2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT83 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowflakes2_Type = wxNewId();
const long xLightsFrame::ID_PANEL25 = wxNewId();
const long xLightsFrame::ID_STATICTEXT68 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowstorm2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT69 = wxNewId();
const long xLightsFrame::ID_SLIDER_Snowstorm2_Length = wxNewId();
const long xLightsFrame::ID_PANEL27 = wxNewId();
const long xLightsFrame::ID_STATICTEXT70 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT71 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals2_Rotation = wxNewId();
const long xLightsFrame::ID_STATICTEXT72 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals2_Thickness = wxNewId();
const long xLightsFrame::ID_STATICTEXT73 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirals2_Direction = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirals2_Blend = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirals2_3D = wxNewId();
const long xLightsFrame::ID_PANEL28 = wxNewId();
const long xLightsFrame::ID_STATICTEXT74 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text2_Line1 = wxNewId();
const long xLightsFrame::ID_STATICTEXT75 = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text2_Line2 = wxNewId();
const long xLightsFrame::ID_BUTTON_TEXT2_1_FONT = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text2_1_Font = wxNewId();
const long xLightsFrame::ID_STATICTEXT76 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_1_Dir = wxNewId();
const long xLightsFrame::ID_STATICTEXT102 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_1_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT100 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_1_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT78 = wxNewId();
const long xLightsFrame::ID_SLIDER_Text2_1_Position = wxNewId();
const long xLightsFrame::ID_PANEL_Text2_1 = wxNewId();
const long xLightsFrame::ID_BUTTON_TEXT2_2_FONT = wxNewId();
const long xLightsFrame::ID_TEXTCTRL_Text2_2_Font = wxNewId();
const long xLightsFrame::ID_STATICTEXT110 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_2_Dir = wxNewId();
const long xLightsFrame::ID_STATICTEXT113 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_2_Effect = wxNewId();
const long xLightsFrame::ID_STATICTEXT112 = wxNewId();
const long xLightsFrame::ID_CHOICE_Text2_2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT111 = wxNewId();
const long xLightsFrame::ID_SLIDER_Text2_2_Position = wxNewId();
const long xLightsFrame::ID_PANEL_Text2_2 = wxNewId();
const long xLightsFrame::ID_NOTEBOOK2 = wxNewId();
const long xLightsFrame::ID_PANEL29 = wxNewId();
const long xLightsFrame::ID_STATICTEXT114 = wxNewId();
const long xLightsFrame::ID_SLIDER_Twinkle2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT115 = wxNewId();
const long xLightsFrame::ID_SLIDER_Twinkle2_Steps = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Twinkle2_Strobe = wxNewId();
const long xLightsFrame::ID_STATICTEXT116 = wxNewId();
const long xLightsFrame::ID_PANEL40 = wxNewId();
const long xLightsFrame::ID_STATICTEXT117 = wxNewId();
const long xLightsFrame::ID_SLIDER_Tree2_Branches = wxNewId();
const long xLightsFrame::ID_PANEL41 = wxNewId();
const long xLightsFrame::ID_STATICTEXT118 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph2_R = wxNewId();
const long xLightsFrame::ID_STATICTEXT119 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph2_r = wxNewId();
const long xLightsFrame::ID_STATICTEXT120 = wxNewId();
const long xLightsFrame::ID_SLIDER_Spirograph2_d = wxNewId();
const long xLightsFrame::ID_STATICTEXT121 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Spirograph2_Animate = wxNewId();
const long xLightsFrame::ID_STATICTEXT122 = wxNewId();
const long xLightsFrame::ID_PANEL42 = wxNewId();
const long xLightsFrame::ID_STATICTEXT123 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks2_Number_Explosions = wxNewId();
const long xLightsFrame::ID_STATICTEXT124 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks2_Count = wxNewId();
const long xLightsFrame::ID_STATICTEXT125 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks2_Velocity = wxNewId();
const long xLightsFrame::ID_STATICTEXT126 = wxNewId();
const long xLightsFrame::ID_SLIDER_Fireworks2_Fade = wxNewId();
const long xLightsFrame::ID_PANEL43 = wxNewId();
const long xLightsFrame::ID_STATICTEXT139 = wxNewId();
const long xLightsFrame::ID_SLIDER_Circles2_count = wxNewId();
const long xLightsFrame::ID_STATICTEXT140 = wxNewId();
const long xLightsFrame::ID_SLIDER_Circles2 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles2_Bounce = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles2_Collide = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles2_Random_m = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Circles2_Radial = wxNewId();
const long xLightsFrame::ID_PANEL38 = wxNewId();
const long xLightsFrame::ID_STATICTEXT142 = wxNewId();
const long xLightsFrame::ID_SLIDER_Piano2_Keyboard = wxNewId();
const long xLightsFrame::ID_PANEL44 = wxNewId();
const long xLightsFrame::ID_CHOICEBOOK2 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_1 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_1 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_2 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_2 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_3 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_3 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_4 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_4 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_5 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_5 = wxNewId();
const long xLightsFrame::ID_CHECKBOX_Palette2_6 = wxNewId();
const long xLightsFrame::ID_BUTTON_Palette2_6 = wxNewId();
const long xLightsFrame::ID_STATICTEXT77 = wxNewId();
const long xLightsFrame::ID_SLIDER_Speed2 = wxNewId();
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
    wxFlexGridSizer* FlexGridSizer71;
    wxFlexGridSizer* FlexGridSizer45;
    wxFlexGridSizer* FlexGridSizer59;
    wxFlexGridSizer* FlexGridSizer47;
    wxFlexGridSizer* FlexGridSizer54;
    wxFlexGridSizer* FlexGridSizerTest;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizer73;
    wxFlexGridSizer* FlexGridSizerSetup;
    wxFlexGridSizer* FlexGridSizer24;
    wxMenuItem* MenuItemCustomScript;
    wxStaticBoxSizer* StaticBoxSizerHighlightColor;
    wxBoxSizer* BoxSizer5;
    wxFlexGridSizer* FlexGridSizer63;
    wxFlexGridSizer* FlexGridSizer19;
    wxFlexGridSizer* FlexGridSizer76;
    wxBoxSizer* BoxSizer10;
    wxFlexGridSizer* FlexGridSizer70;
    wxFlexGridSizer* FlexGridSizer38;
    wxFlexGridSizer* FlexGridSizer23;
    wxMenuItem* MenuItem5;
    wxFlexGridSizer* FlexGridSizer41;
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItemAddList;
    wxFlexGridSizer* FlexGridSizer77;
    wxFlexGridSizer* FlexGridSizer51;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer80;
    wxFlexGridSizer* FlexGridSizer27;
    wxMenuItem* MenuItem1;
    wxMenuItem* MenuItem4;
    wxFlexGridSizer* FlexGridSizer44;
    wxFlexGridSizer* FlexGridSizer37;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer25;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer62;
    wxFlexGridSizer* FlexGridSizer56;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer66;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer69;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer53;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer55;
    wxMenuItem* MenuItemDelList;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizer74;
    wxFlexGridSizer* FlexGridSizer57;
    wxFlexGridSizer* FlexGridSizer61;
    wxFlexGridSizer* FlexGridSizer52;
    wxFlexGridSizer* FlexGridSizer29;
    wxFlexGridSizer* FlexGridSizer34;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxFlexGridSizer* FlexGridSizer49;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer81;
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
    wxFlexGridSizer* FlexGridSizer78;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer48;
    wxFlexGridSizer* FlexGridSizer46;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer33;
    wxFlexGridSizer* FlexGridSizerConvert;
    wxFlexGridSizer* FlexGridSizer43;
    wxFlexGridSizer* FlexGridSizer11;
    wxBoxSizer* BoxSizer3;
    wxFlexGridSizer* FlexGridSizer17;
    wxStaticBoxSizer* StaticBoxSizer5;
    wxFlexGridSizer* FlexGridSizer32;
    wxFlexGridSizer* FlexGridSizer65;
    wxFlexGridSizer* FlexGridSizer79;
    wxFlexGridSizer* FlexGridSizer42;
    wxFlexGridSizer* FlexGridSizer58;
    wxFlexGridSizer* FlexGridSizer64;
    wxFlexGridSizer* FlexGridSizer75;
    wxFlexGridSizer* FlexGridSizer72;
    wxFlexGridSizer* FlexGridSizer31;
    wxFlexGridSizer* FlexGridSizer68;
    wxFlexGridSizer* FlexGridSizer40;
    wxFlexGridSizer* FlexGridSizer39;
    wxFlexGridSizer* FlexGridSizer28;
    wxStaticBoxSizer* StaticBoxSizerBackgroundColor;
    wxMenu* MenuPlaylist;
    wxFlexGridSizer* FlexGridSizer26;
    wxFlexGridSizer* FlexGridSizer60;
    wxFlexGridSizer* FlexGridSizer30;
    wxFlexGridSizer* FlexGridSizer67;

    Create(parent, wxID_ANY, _("xLights/Nutcracker  (Ver BETA 3.0.24)"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
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
    PanelSequence2 = new wxPanel(Notebook1, ID_PANEL30, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL30"));
    FlexGridSizer70 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer70->AddGrowableCol(0);
    FlexGridSizer70->AddGrowableRow(0);
    SplitterWindow2 = new wxSplitterWindow(PanelSequence2, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH|wxSP_NOBORDER|wxSP_NO_XP_THEME, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetMinSize(wxSize(10,10));
    SplitterWindow2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    SplitterWindow2->SetMinimumPaneSize(10);
    SplitterWindow2->SetSashGravity(0.5);
    Panel5 = new wxPanel(SplitterWindow2, ID_PANEL31, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL31"));
    BoxSizer6 = new wxBoxSizer(wxVERTICAL);
    FlexGridSizer30 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer30->AddGrowableCol(0);
    FlexGridSizer30->AddGrowableRow(0);
    ScrolledWindow1 = new wxScrolledWindow(Panel5, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxSize(276,225), wxSIMPLE_BORDER|wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
    ScrolledWindow1->SetBackgroundColour(wxColour(0,0,0));
    FlexGridSizer30->Add(ScrolledWindow1, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 0);
    BoxSizer6->Add(FlexGridSizer30, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, Panel5, _("Combined Effect"));
    FlexGridSizer33 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_PlayEffect = new wxButton(Panel5, ID_BUTTON13, _("Play Effect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    Button_PlayEffect->Disable();
    Button_PlayEffect->SetBackgroundColour(wxColour(0,255,0));
    FlexGridSizer33->Add(Button_PlayEffect, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_UpdateGrid = new wxButton(Panel5, ID_BUTTON3, _("Update Grid"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    Button_UpdateGrid->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_UpdateGrid, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_Models = new wxButton(Panel5, ID_BUTTON58, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON58"));
    Button_Models->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Models = new wxChoice(Panel5, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE7"));
    FlexGridSizer33->Add(Choice_Models, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_Presets = new wxButton(Panel5, ID_BUTTON59, _("Effect Presets"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON59"));
    Button_Presets->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Presets, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Presets = new wxChoice(Panel5, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer33->Add(Choice_Presets, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_PresetAdd = new wxButton(Panel5, ID_BUTTON9, _("New Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    Button_PresetAdd->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_PresetAdd, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button_PresetUpdate = new wxButton(Panel5, ID_BUTTON8, _("Update Preset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    Button_PresetUpdate->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_PresetUpdate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(Panel5, ID_STATICTEXT23, _("Layer Method"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer33->Add(StaticText21, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_LayerMethod = new wxChoice(Panel5, ID_CHOICE_LayerMethod, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_LayerMethod"));
    Choice_LayerMethod->SetSelection( Choice_LayerMethod->Append(_("Effect 1")) );
    Choice_LayerMethod->Append(_("Effect 2"));
    Choice_LayerMethod->Append(_("1 is Mask"));
    Choice_LayerMethod->Append(_("2 is Mask"));
    Choice_LayerMethod->Append(_("1 is Unmask"));
    Choice_LayerMethod->Append(_("2 is Unmask"));
    Choice_LayerMethod->Append(_("Layered"));
    Choice_LayerMethod->Append(_("Average"));
    FlexGridSizer33->Add(Choice_LayerMethod, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText22 = new wxStaticText(Panel5, ID_STATICTEXT24, _("Sparkles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer33->Add(StaticText22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_SparkleFrequency = new wxSlider(Panel5, ID_SLIDER_SparkleFrequency, 200, 10, 200, wxDefaultPosition, wxDefaultSize, wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_SparkleFrequency"));
    FlexGridSizer33->Add(Slider_SparkleFrequency, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText126 = new wxStaticText(Panel5, ID_STATICTEXT127, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT127"));
    FlexGridSizer33->Add(StaticText126, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Brightness = new wxSlider(Panel5, ID_SLIDER_Brightness, 100, 0, 400, wxDefaultPosition, wxDefaultSize, wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER_Brightness"));
    FlexGridSizer33->Add(Slider_Brightness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText127 = new wxStaticText(Panel5, ID_STATICTEXT128, _("Contrast"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT128"));
    FlexGridSizer33->Add(StaticText127, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Contrast = new wxSlider(Panel5, ID_SLIDER_Contrast, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Contrast"));
    FlexGridSizer33->Add(Slider_Contrast, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer33, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer6->Add(StaticBoxSizer3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel5->SetSizer(BoxSizer6);
    BoxSizer6->Fit(Panel5);
    BoxSizer6->SetSizeHints(Panel5);
    Panel4 = new wxPanel(SplitterWindow2, ID_PANEL32, wxPoint(40,-11), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL32"));
    BoxSizer4 = new wxBoxSizer(wxVERTICAL);
    StaticBoxSizerSequenceButtons = new wxStaticBoxSizer(wxVERTICAL, Panel4, _("RGB Sequence"));
    FlexGridSizer32 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer32->AddGrowableCol(0);
    FlexGridSizer32->AddGrowableRow(2);
    StaticTextSequenceFileName = new wxStaticText(Panel4, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer32->Add(StaticTextSequenceFileName, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer68 = new wxFlexGridSizer(0, 10, 0, 0);
    Button_PlayRgbSeq = new wxButton(Panel4, ID_BUTTON_PLAY_RGB_SEQ, _("Play"), wxDefaultPosition, wxSize(59,23), 0, wxDefaultValidator, _T("ID_BUTTON_PLAY_RGB_SEQ"));
    Button_PlayRgbSeq->SetBackgroundColour(wxColour(0,255,0));
    Button_PlayRgbSeq->SetToolTip(_("Play from current grid cell"));
    FlexGridSizer68->Add(Button_PlayRgbSeq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    ButtonDisplayElements = new wxButton(Panel4, ID_BUTTON2, _("Display Elements"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    ButtonDisplayElements->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(ButtonDisplayElements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ChannelMap = new wxButton(Panel4, ID_BUTTON1, _("Channel Map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button_ChannelMap->Disable();
    Button_ChannelMap->Hide();
    Button_ChannelMap->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(Button_ChannelMap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    BitmapButtonOpenSeq = new wxBitmapButton(Panel4, ID_BITMAPBUTTON7, open_xpm, wxDefaultPosition, wxSize(48,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    BitmapButtonOpenSeq->SetDefault();
    BitmapButtonOpenSeq->SetToolTip(_("Open Sequence"));
    FlexGridSizer68->Add(BitmapButtonOpenSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonSaveSeq = new wxBitmapButton(Panel4, ID_BITMAPBUTTON9, save_xpm, wxDefaultPosition, wxSize(53,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    BitmapButtonSaveSeq->SetDefault();
    BitmapButtonSaveSeq->SetToolTip(_("Save Sequence"));
    FlexGridSizer68->Add(BitmapButtonSaveSeq, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonInsertRow = new wxBitmapButton(Panel4, ID_BITMAPBUTTON3, insertrow_xpm, wxDefaultPosition, wxSize(50,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    BitmapButtonInsertRow->SetDefault();
    BitmapButtonInsertRow->SetToolTip(_("Insert Row"));
    FlexGridSizer68->Add(BitmapButtonInsertRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonDeleteRow = new wxBitmapButton(Panel4, ID_BITMAPBUTTON4, deleterow_xpm, wxDefaultPosition, wxSize(49,23), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    BitmapButtonDeleteRow->SetDefault();
    BitmapButtonDeleteRow->SetToolTip(_("Delete Row"));
    FlexGridSizer68->Add(BitmapButtonDeleteRow, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonSeqExport = new wxButton(Panel4, ID_BUTTON_SeqExport, _("Export"), wxDefaultPosition, wxSize(52,23), 0, wxDefaultValidator, _T("ID_BUTTON_SeqExport"));
    ButtonSeqExport->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer68->Add(ButtonSeqExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(Panel4, ID_BUTTON4, _("Create Random Effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer68->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer32->Add(FlexGridSizer68, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Grid1 = new wxGrid(Panel4, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxFULL_REPAINT_ON_RESIZE, _T("ID_GRID1"));
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
    StaticBoxSizer4 = new wxStaticBoxSizer(wxVERTICAL, Panel4, _("Effect 1"));
    FlexGridSizer34 = new wxFlexGridSizer(0, 3, 0, 0);
    Choicebook1 = new wxChoicebook(Panel4, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
    Panel1_None = new wxPanel(Choicebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    Panel1_Bars = new wxPanel(Choicebook1, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL8"));
    FlexGridSizer35 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer35->AddGrowableCol(1);
    StaticText23 = new wxStaticText(Panel1_Bars, ID_STATICTEXT25, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Bars1_BarCount = new wxSlider(Panel1_Bars, ID_SLIDER_Bars1_BarCount, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars1_BarCount"));
    FlexGridSizer35->Add(Slider_Bars1_BarCount, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText24 = new wxStaticText(Panel1_Bars, ID_STATICTEXT26, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Bars1_Direction = new wxChoice(Panel1_Bars, ID_CHOICE_Bars1_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars1_Direction"));
    FlexGridSizer35->Add(Choice_Bars1_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Bars1_Highlight = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars1_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars1_Highlight"));
    CheckBox_Bars1_Highlight->SetValue(false);
    FlexGridSizer35->Add(CheckBox_Bars1_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Bars1_3D = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars1_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars1_3D"));
    CheckBox_Bars1_3D->SetValue(false);
    FlexGridSizer35->Add(CheckBox_Bars1_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Bars->SetSizer(FlexGridSizer35);
    FlexGridSizer35->Fit(Panel1_Bars);
    FlexGridSizer35->SetSizeHints(Panel1_Bars);
    Panel1_Butterfly = new wxPanel(Choicebook1, ID_PANEL10, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL10"));
    FlexGridSizer36 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText25 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT27, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer36->Add(StaticText25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Butterfly1_Colors = new wxChoice(Panel1_Butterfly, ID_CHOICE_Butterfly1_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly1_Colors"));
    FlexGridSizer36->Add(Choice_Butterfly1_Colors, 1, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText26 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT28, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer36->Add(StaticText26, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly1_Style = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly1_Style, 1, 1, 3, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly1_Style"));
    FlexGridSizer36->Add(Slider_Butterfly1_Style, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText27 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT30, _("Bkgrd Chunks"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer36->Add(StaticText27, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly1_Chunks = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly1_Chunks, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly1_Chunks"));
    FlexGridSizer36->Add(Slider_Butterfly1_Chunks, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText28 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT31, _("Bkgrd Skip"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer36->Add(StaticText28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly1_Skip = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly1_Skip, 2, 2, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly1_Skip"));
    FlexGridSizer36->Add(Slider_Butterfly1_Skip, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Butterfly->SetSizer(FlexGridSizer36);
    FlexGridSizer36->Fit(Panel1_Butterfly);
    FlexGridSizer36->SetSizeHints(Panel1_Butterfly);
    Panel1_ColorWash = new wxPanel(Choicebook1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer37 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer37->AddGrowableCol(1);
    StaticText5 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT21, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer37->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_ColorWash1_Count = new wxSlider(Panel1_ColorWash, ID_SLIDER_ColorWash1_Count, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash1_Count"));
    FlexGridSizer37->Add(Slider_ColorWash1_Count, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT32, _("Horizontal"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer37->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash1_HFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash1_HFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash1_HFade"));
    CheckBox_ColorWash1_HFade->SetValue(false);
    FlexGridSizer37->Add(CheckBox_ColorWash1_HFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT33, _("Vertical"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer37->Add(StaticText31, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash1_VFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash1_VFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash1_VFade"));
    CheckBox_ColorWash1_VFade->SetValue(false);
    FlexGridSizer37->Add(CheckBox_ColorWash1_VFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_ColorWash->SetSizer(FlexGridSizer37);
    FlexGridSizer37->Fit(Panel1_ColorWash);
    FlexGridSizer37->SetSizeHints(Panel1_ColorWash);
    Panel1_Fire = new wxPanel(Choicebook1, ID_PANEL12, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL12"));
    FlexGridSizer38 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer38->AddGrowableCol(1);
    StaticText83 = new wxStaticText(Panel1_Fire, ID_STATICTEXT84, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT84"));
    FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fire1_Height = new wxSlider(Panel1_Fire, ID_SLIDER_Fire1_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire1_Height"));
    FlexGridSizer38->Add(Slider_Fire1_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText132 = new wxStaticText(Panel1_Fire, ID_STATICTEXT133, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT133"));
    FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fire1_HueShift = new wxSlider(Panel1_Fire, ID_SLIDER_Fire1_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire1_HueShift"));
    FlexGridSizer38->Add(Slider_Fire1_HueShift, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText134 = new wxStaticText(Panel1_Fire, ID_STATICTEXT135, _("Grow Fire\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT135"));
    FlexGridSizer38->Add(StaticText134, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Fire1_GrowFire = new wxCheckBox(Panel1_Fire, ID_CHECKBOX_Fire1_GrowFire, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire1_GrowFire"));
    CheckBox_Fire1_GrowFire->SetValue(false);
    FlexGridSizer38->Add(CheckBox_Fire1_GrowFire, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Fire->SetSizer(FlexGridSizer38);
    FlexGridSizer38->Fit(Panel1_Fire);
    FlexGridSizer38->SetSizeHints(Panel1_Fire);
    Panel1_Garlands = new wxPanel(Choicebook1, ID_PANEL14, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL14"));
    FlexGridSizer39 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer39->AddGrowableCol(1);
    StaticText32 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT34, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer39->Add(StaticText32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands1_Type = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands1_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands1_Type"));
    FlexGridSizer39->Add(Slider_Garlands1_Type, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText33 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT35, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer39->Add(StaticText33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands1_Spacing = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands1_Spacing, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands1_Spacing"));
    FlexGridSizer39->Add(Slider_Garlands1_Spacing, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Garlands->SetSizer(FlexGridSizer39);
    FlexGridSizer39->Fit(Panel1_Garlands);
    FlexGridSizer39->SetSizeHints(Panel1_Garlands);
    Panel1_Life = new wxPanel(Choicebook1, ID_PANEL16, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL16"));
    FlexGridSizer40 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer40->AddGrowableCol(1);
    StaticText35 = new wxStaticText(Panel1_Life, ID_STATICTEXT36, _("Cells to start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer40->Add(StaticText35, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life1_Count = new wxSlider(Panel1_Life, ID_SLIDER_Life1_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life1_Count"));
    FlexGridSizer40->Add(Slider_Life1_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText37 = new wxStaticText(Panel1_Life, ID_STATICTEXT37, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer40->Add(StaticText37, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life1_Seed = new wxSlider(Panel1_Life, ID_SLIDER_Life1_Seed, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life1_Seed"));
    FlexGridSizer40->Add(Slider_Life1_Seed, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Life->SetSizer(FlexGridSizer40);
    FlexGridSizer40->Fit(Panel1_Life);
    FlexGridSizer40->SetSizeHints(Panel1_Life);
    Panel1_Meteors = new wxPanel(Choicebook1, ID_PANEL18, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL18"));
    FlexGridSizer41 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer41->AddGrowableCol(1);
    StaticText39 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT39, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer41->Add(StaticText39, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors1_Type = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors1_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors1_Type"));
    FlexGridSizer41->Add(Choice_Meteors1_Type, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText128 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT129, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT129"));
    FlexGridSizer41->Add(StaticText128, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors1_Effect = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors1_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors1_Effect"));
    FlexGridSizer41->Add(Choice_Meteors1_Effect, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText41 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT41, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer41->Add(StaticText41, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors1_Count = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors1_Count, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors1_Count"));
    FlexGridSizer41->Add(Slider_Meteors1_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText43 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT43, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer41->Add(StaticText43, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors1_Length = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors1_Length, 25, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors1_Length"));
    FlexGridSizer41->Add(Slider_Meteors1_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText130 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT131, _("Swirl Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT131"));
    FlexGridSizer41->Add(StaticText130, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors1_Swirl_Intensity = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors1_Swirl_Intensity, 0, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors1_Swirl_Intensity"));
    FlexGridSizer41->Add(Slider_Meteors1_Swirl_Intensity, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText105 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT106, _("Meteors Fall Up\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT106"));
    FlexGridSizer41->Add(StaticText105, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Meteors1_FallUp = new wxCheckBox(Panel1_Meteors, ID_CHECKBOX_Meteors1_FallUp, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Meteors1_FallUp"));
    CheckBox_Meteors1_FallUp->SetValue(false);
    FlexGridSizer41->Add(CheckBox_Meteors1_FallUp, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Meteors->SetSizer(FlexGridSizer41);
    FlexGridSizer41->Fit(Panel1_Meteors);
    FlexGridSizer41->SetSizeHints(Panel1_Meteors);
    Panel1_Pictures = new wxPanel(Choicebook1, ID_PANEL20, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL20"));
    FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer42->AddGrowableCol(0);
    Button_Pictures1_Filename = new wxButton(Panel1_Pictures, ID_BUTTON_PICTURES1_FILENAME, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PICTURES1_FILENAME"));
    Button_Pictures1_Filename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer42->Add(Button_Pictures1_Filename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Pictures1_Filename = new wxTextCtrl(Panel1_Pictures, ID_TEXTCTRL_Pictures1_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Pictures1_Filename"));
    FlexGridSizer42->Add(TextCtrl_Pictures1_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer31 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText46 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT46, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    FlexGridSizer31->Add(StaticText46, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Pictures1_Direction = new wxChoice(Panel1_Pictures, ID_CHOICE_Pictures1_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pictures1_Direction"));
    FlexGridSizer31->Add(Choice_Pictures1_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText96 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT97, _("Animated Gif Speed"), wxDefaultPosition, wxSize(107,13), 0, _T("ID_STATICTEXT97"));
    FlexGridSizer31->Add(StaticText96, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pictures1_GifSpeed = new wxSlider(Panel1_Pictures, ID_SLIDER_Pictures1_GifSpeed, 20, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pictures1_GifSpeed"));
    FlexGridSizer31->Add(Slider_Pictures1_GifSpeed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer42->Add(FlexGridSizer31, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Pictures->SetSizer(FlexGridSizer42);
    FlexGridSizer42->Fit(Panel1_Pictures);
    FlexGridSizer42->SetSizeHints(Panel1_Pictures);
    Panel1_Snowflakes = new wxPanel(Choicebook1, ID_PANEL22, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL22"));
    FlexGridSizer43 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer43->AddGrowableCol(1);
    StaticText79 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT80, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT80"));
    FlexGridSizer43->Add(StaticText79, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes1_Count = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes1_Count, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes1_Count"));
    FlexGridSizer43->Add(Slider_Snowflakes1_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText80 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT81, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT81"));
    FlexGridSizer43->Add(StaticText80, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes1_Type = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes1_Type, 1, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes1_Type"));
    FlexGridSizer43->Add(Slider_Snowflakes1_Type, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Snowflakes->SetSizer(FlexGridSizer43);
    FlexGridSizer43->Fit(Panel1_Snowflakes);
    FlexGridSizer43->SetSizeHints(Panel1_Snowflakes);
    Panel1_Snowstorm = new wxPanel(Choicebook1, ID_PANEL24, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL24"));
    FlexGridSizer44 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer44->AddGrowableCol(1);
    StaticText45 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT45, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
    FlexGridSizer44->Add(StaticText45, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm1_Count = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm1_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm1_Count"));
    FlexGridSizer44->Add(Slider_Snowstorm1_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText51 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT51, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer44->Add(StaticText51, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm1_Length = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm1_Length, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm1_Length"));
    FlexGridSizer44->Add(Slider_Snowstorm1_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Snowstorm->SetSizer(FlexGridSizer44);
    FlexGridSizer44->Fit(Panel1_Snowstorm);
    FlexGridSizer44->SetSizeHints(Panel1_Snowstorm);
    Panel1_Spirals = new wxPanel(Choicebook1, ID_PANEL26, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL26"));
    FlexGridSizer45 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer45->AddGrowableCol(1);
    StaticText34 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT38, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer45->Add(StaticText34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals1_Count = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals1_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals1_Count"));
    FlexGridSizer45->Add(Slider_Spirals1_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText36 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT40, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer45->Add(StaticText36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals1_Rotation = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals1_Rotation, 20, -500, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals1_Rotation"));
    FlexGridSizer45->Add(Slider_Spirals1_Rotation, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText38 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT42, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer45->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals1_Thickness = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals1_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals1_Thickness"));
    FlexGridSizer45->Add(Slider_Spirals1_Thickness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText40 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT44, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer45->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals1_Direction = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals1_Direction, 1, -1, 1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals1_Direction"));
    FlexGridSizer45->Add(Slider_Spirals1_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Spirals1_Blend = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals1_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals1_Blend"));
    CheckBox_Spirals1_Blend->SetValue(false);
    FlexGridSizer45->Add(CheckBox_Spirals1_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Spirals1_3D = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals1_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals1_3D"));
    CheckBox_Spirals1_3D->SetValue(false);
    FlexGridSizer45->Add(CheckBox_Spirals1_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Spirals->SetSizer(FlexGridSizer45);
    FlexGridSizer45->Fit(Panel1_Spirals);
    FlexGridSizer45->SetSizeHints(Panel1_Spirals);
    Panel1_Text = new wxPanel(Choicebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer46 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer46->AddGrowableCol(0);
    FlexGridSizer65 = new wxFlexGridSizer(2, 2, 0, 0);
    FlexGridSizer65->AddGrowableCol(1);
    StaticText53 = new wxStaticText(Panel1_Text, ID_STATICTEXT53, _("Line 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT53"));
    FlexGridSizer65->Add(StaticText53, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text1_Line1 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text1_Line1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text1_Line1"));
    TextCtrl_Text1_Line1->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text1_Line1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText54 = new wxStaticText(Panel1_Text, ID_STATICTEXT54, _("Line 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT54"));
    FlexGridSizer65->Add(StaticText54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text1_Line2 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text1_Line2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text1_Line2"));
    TextCtrl_Text1_Line2->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text1_Line2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer46->Add(FlexGridSizer65, 1, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Notebook_Text1 = new wxNotebook(Panel1_Text, ID_NOTEBOOK_Text1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Text1"));
    Notebook_Text1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    Panel_Text1_1 = new wxPanel(Notebook_Text1, ID_PANEL_Text1_1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text1_1"));
    FlexGridSizer69 = new wxFlexGridSizer(5, 2, 0, 0);
    Button_Text1_1_Font = new wxButton(Panel_Text1_1, ID_BUTTON_TEXT1_1_FONT, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT1_1_FONT"));
    Button_Text1_1_Font->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer69->Add(Button_Text1_1_Font, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text1_1_Font = new wxTextCtrl(Panel_Text1_1, ID_TEXTCTRL_Text1_1_Font, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text1_1_Font"));
    FlexGridSizer69->Add(TextCtrl_Text1_1_Font, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText78 = new wxStaticText(Panel_Text1_1, ID_STATICTEXT79, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT79"));
    FlexGridSizer69->Add(StaticText78, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_1_Dir = new wxChoice(Panel_Text1_1, ID_CHOICE_Text1_1_Dir, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_1_Dir"));
    FlexGridSizer69->Add(Choice_Text1_1_Dir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText107 = new wxStaticText(Panel_Text1_1, ID_STATICTEXT108, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT108"));
    FlexGridSizer69->Add(StaticText107, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_1_Effect = new wxChoice(Panel_Text1_1, ID_CHOICE_Text1_1_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_1_Effect"));
    FlexGridSizer69->Add(Choice_Text1_1_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText98 = new wxStaticText(Panel_Text1_1, ID_STATICTEXT99, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT99"));
    FlexGridSizer69->Add(StaticText98, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_1_Count = new wxChoice(Panel_Text1_1, ID_CHOICE_Text1_1_Count, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_1_Count"));
    FlexGridSizer69->Add(Choice_Text1_1_Count, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText100 = new wxStaticText(Panel_Text1_1, ID_STATICTEXT101, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT101"));
    FlexGridSizer69->Add(StaticText100, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text1_1_Position = new wxSlider(Panel_Text1_1, ID_SLIDER_Text1_1_Position, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text1_1_Position"));
    FlexGridSizer69->Add(Slider_Text1_1_Position, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel_Text1_1->SetSizer(FlexGridSizer69);
    FlexGridSizer69->Fit(Panel_Text1_1);
    FlexGridSizer69->SetSizeHints(Panel_Text1_1);
    Panel_Text1_2 = new wxPanel(Notebook_Text1, ID_PANEL_Text1_2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text1_2"));
    FlexGridSizer74 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Text1_2_Font = new wxButton(Panel_Text1_2, ID_BUTTON_TEXT1_2_FONT, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT1_2_FONT"));
    Button_Text1_2_Font->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer74->Add(Button_Text1_2_Font, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text1_2_Font = new wxTextCtrl(Panel_Text1_2, ID_TEXTCTRL_Text1_2_Font, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text1_2_Font"));
    FlexGridSizer74->Add(TextCtrl_Text1_2_Font, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText57 = new wxStaticText(Panel_Text1_2, ID_STATICTEXT57, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT57"));
    FlexGridSizer74->Add(StaticText57, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_2_Dir = new wxChoice(Panel_Text1_2, ID_CHOICE_Text1_2_Dir, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_2_Dir"));
    FlexGridSizer74->Add(Choice_Text1_2_Dir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText108 = new wxStaticText(Panel_Text1_2, ID_STATICTEXT109, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT109"));
    FlexGridSizer74->Add(StaticText108, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_2_Effect = new wxChoice(Panel_Text1_2, ID_CHOICE_Text1_2_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_2_Effect"));
    FlexGridSizer74->Add(Choice_Text1_2_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText103 = new wxStaticText(Panel_Text1_2, ID_STATICTEXT104, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT104"));
    FlexGridSizer74->Add(StaticText103, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text1_2_Count = new wxChoice(Panel_Text1_2, ID_CHOICE_Text1_2_Count, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text1_2_Count"));
    FlexGridSizer74->Add(Choice_Text1_2_Count, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText102 = new wxStaticText(Panel_Text1_2, ID_STATICTEXT103, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT103"));
    FlexGridSizer74->Add(StaticText102, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text1_2_Position = new wxSlider(Panel_Text1_2, ID_SLIDER_Text1_2_Position, 50, 0, 100, wxDefaultPosition, wxSize(118,24), 0, wxDefaultValidator, _T("ID_SLIDER_Text1_2_Position"));
    FlexGridSizer74->Add(Slider_Text1_2_Position, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel_Text1_2->SetSizer(FlexGridSizer74);
    FlexGridSizer74->Fit(Panel_Text1_2);
    FlexGridSizer74->SetSizeHints(Panel_Text1_2);
    Notebook_Text1->AddPage(Panel_Text1_1, _("Line 1 Details"), false);
    Notebook_Text1->AddPage(Panel_Text1_2, _("Line 2 Details"), false);
    FlexGridSizer46->Add(Notebook_Text1, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
    Panel1_Text->SetSizer(FlexGridSizer46);
    FlexGridSizer46->Fit(Panel1_Text);
    FlexGridSizer46->SetSizeHints(Panel1_Text);
    Panel1_Twinkle = new wxPanel(Choicebook1, ID_PANEL33, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL33"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText85 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT86, _("Number Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT86"));
    FlexGridSizer3->Add(StaticText85, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle1_Count = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle1_Count, 3, 2, 100, wxDefaultPosition, wxSize(117,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle1_Count"));
    FlexGridSizer3->Add(Slider_Twinkle1_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText104 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT105, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT105"));
    FlexGridSizer3->Add(StaticText104, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle1_Steps = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle1_Steps, 30, 2, 200, wxDefaultPosition, wxSize(119,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle1_Steps"));
    FlexGridSizer3->Add(Slider_Twinkle1_Steps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Twinkle1_Strobe = new wxCheckBox(Panel1_Twinkle, ID_CHECKBOX_Twinkle1_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle1_Strobe"));
    CheckBox_Twinkle1_Strobe->SetValue(false);
    FlexGridSizer3->Add(CheckBox_Twinkle1_Strobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Twinkle->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel1_Twinkle);
    FlexGridSizer3->SetSizeHints(Panel1_Twinkle);
    Panel1_Tree = new wxPanel(Choicebook1, ID_PANEL34, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL34"));
    FlexGridSizer71 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText86 = new wxStaticText(Panel1_Tree, ID_STATICTEXT87, _("Number Branches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT87"));
    FlexGridSizer71->Add(StaticText86, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Tree1_Branches = new wxSlider(Panel1_Tree, ID_SLIDER_Tree1_Branches, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree1_Branches"));
    FlexGridSizer71->Add(Slider_Tree1_Branches, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Tree->SetSizer(FlexGridSizer71);
    FlexGridSizer71->Fit(Panel1_Tree);
    FlexGridSizer71->SetSizeHints(Panel1_Tree);
    Panel1_Spirograph = new wxPanel(Choicebook1, ID_PANEL35, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL35"));
    FlexGridSizer72 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText87 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT88, _("R - Radius of outer circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT88"));
    FlexGridSizer72->Add(StaticText87, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph1_R = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph1_R, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph1_R"));
    FlexGridSizer72->Add(Slider_Spirograph1_R, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText88 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT89, _("r - Radius of inner circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT89"));
    FlexGridSizer72->Add(StaticText88, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph1_r = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph1_r, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph1_r"));
    FlexGridSizer72->Add(Slider_Spirograph1_r, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText89 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT90, _("d - Distance"), wxDefaultPosition, wxSize(84,13), 0, _T("ID_STATICTEXT90"));
    FlexGridSizer72->Add(StaticText89, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph1_d = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph1_d, 30, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph1_d"));
    FlexGridSizer72->Add(Slider_Spirograph1_d, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText92 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT93, _("Should distance\nbe animated\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT93"));
    FlexGridSizer72->Add(StaticText92, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Spirograph1_Animate = new wxCheckBox(Panel1_Spirograph, ID_CHECKBOX_Spirograph1_Animate, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirograph1_Animate"));
    CheckBox_Spirograph1_Animate->SetValue(false);
    FlexGridSizer72->Add(CheckBox_Spirograph1_Animate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText90 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT91, _("Note: r should be <= R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT91"));
    FlexGridSizer72->Add(StaticText90, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Spirograph->SetSizer(FlexGridSizer72);
    FlexGridSizer72->Fit(Panel1_Spirograph);
    FlexGridSizer72->SetSizeHints(Panel1_Spirograph);
    Panel1_Fireworks = new wxPanel(Choicebook1, ID_PANEL36, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL36"));
    FlexGridSizer73 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText94 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT95, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT95"));
    FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks1_Number_Explosions = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks1_Number_Explosions, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks1_Number_Explosions"));
    FlexGridSizer73->Add(Slider_Fireworks1_Number_Explosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText91 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT92, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT92"));
    FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks1_Count = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks1_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks1_Count"));
    FlexGridSizer73->Add(Slider_Fireworks1_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText93 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT94, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT94"));
    FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks1_Velocity = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks1_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks1_Velocity"));
    FlexGridSizer73->Add(Slider_Fireworks1_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText95 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT96, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT96"));
    FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks1_Fade = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks1_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks1_Fade"));
    FlexGridSizer73->Add(Slider_Fireworks1_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Fireworks->SetSizer(FlexGridSizer73);
    FlexGridSizer73->Fit(Panel1_Fireworks);
    FlexGridSizer73->SetSizeHints(Panel1_Fireworks);
    Panel1_Circles = new wxPanel(Choicebook1, ID_PANEL37, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL37"));
    FlexGridSizer80 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText136 = new wxStaticText(Panel1_Circles, ID_STATICTEXT137, _("Number of Circles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT137"));
    FlexGridSizer80->Add(StaticText136, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Circles1_Count = new wxSlider(Panel1_Circles, ID_SLIDER_Circles1_Count, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles1_Count"));
    FlexGridSizer80->Add(Slider_Circles1_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText137 = new wxStaticText(Panel1_Circles, ID_STATICTEXT138, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT138"));
    FlexGridSizer80->Add(StaticText137, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Circles1_Size = new wxSlider(Panel1_Circles, ID_SLIDER_Circles1_Size, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles1_Size"));
    FlexGridSizer80->Add(Slider_Circles1_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles1_Bounce = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles1_Bounce, _("Bounce"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles1_Bounce"));
    CheckBox_Circles1_Bounce->SetValue(false);
    FlexGridSizer80->Add(CheckBox_Circles1_Bounce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles1_Collide = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles1_Collide, _("Collide"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles1_Collide"));
    CheckBox_Circles1_Collide->SetValue(false);
    FlexGridSizer80->Add(CheckBox_Circles1_Collide, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles1_Random_m = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles1_Random_m, _("Random Motion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles1_Random_m"));
    CheckBox_Circles1_Random_m->SetValue(false);
    FlexGridSizer80->Add(CheckBox_Circles1_Random_m, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles1_Radial = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles1_Radial, _("Radial"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles1_Radial"));
    CheckBox_Circles1_Radial->SetValue(false);
    FlexGridSizer80->Add(CheckBox_Circles1_Radial, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Circles->SetSizer(FlexGridSizer80);
    FlexGridSizer80->Fit(Panel1_Circles);
    FlexGridSizer80->SetSizeHints(Panel1_Circles);
    Panel1_Piano = new wxPanel(Choicebook1, ID_PANEL39, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL39"));
    StaticText140 = new wxStaticText(Panel1_Piano, ID_STATICTEXT141, _("Keyboard"), wxPoint(32,32), wxDefaultSize, 0, _T("ID_STATICTEXT141"));
    Slider_Piano1_Keyboard = new wxSlider(Panel1_Piano, ID_SLIDER_Piano1_Keyboard, 3, 1, 3, wxPoint(136,32), wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano1_Keyboard"));
    Choicebook1->AddPage(Panel1_None, _("None"), false);
    Choicebook1->AddPage(Panel1_Bars, _("Bars"), false);
    Choicebook1->AddPage(Panel1_Butterfly, _("Butterfly"), false);
    Choicebook1->AddPage(Panel1_ColorWash, _("Color Wash"), false);
    Choicebook1->AddPage(Panel1_Fire, _("Fire"), false);
    Choicebook1->AddPage(Panel1_Garlands, _("Garlands"), false);
    Choicebook1->AddPage(Panel1_Life, _("Life"), false);
    Choicebook1->AddPage(Panel1_Meteors, _("Meteors"), false);
    Choicebook1->AddPage(Panel1_Pictures, _("Pictures"), false);
    Choicebook1->AddPage(Panel1_Snowflakes, _("Snowflakes"), false);
    Choicebook1->AddPage(Panel1_Snowstorm, _("Snowstorm"), false);
    Choicebook1->AddPage(Panel1_Spirals, _("Spirals"), false);
    Choicebook1->AddPage(Panel1_Text, _("Text"), false);
    Choicebook1->AddPage(Panel1_Twinkle, _("Twinkle"), false);
    Choicebook1->AddPage(Panel1_Tree, _("Tree"), false);
    Choicebook1->AddPage(Panel1_Spirograph, _("Spirograph"), false);
    Choicebook1->AddPage(Panel1_Fireworks, _("Fireworks"), false);
    Choicebook1->AddPage(Panel1_Circles, _("Circles (Under Development)"), false);
    Choicebook1->AddPage(Panel1_Piano, _("Piano (Under Development)"), false);
    FlexGridSizer34->Add(Choicebook1, 1, wxTOP|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer47 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Palette1 = new wxButton(Panel4, ID_BUTTON_Palette1, _("Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1"));
    Button_Palette1->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer47->Add(Button_Palette1, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Palette1 = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_Palette1_1 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_1"));
    CheckBox_Palette1_1->SetValue(true);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_1 = new wxButton(Panel4, ID_BUTTON_Palette1_1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_1"));
    Button_Palette1_1->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette1_2 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_2"));
    CheckBox_Palette1_2->SetValue(true);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_2 = new wxButton(Panel4, ID_BUTTON_Palette1_2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_2"));
    Button_Palette1_2->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette1_3 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_3"));
    CheckBox_Palette1_3->SetValue(false);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_3 = new wxButton(Panel4, ID_BUTTON_Palette1_3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_3"));
    Button_Palette1_3->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette1_4 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_4"));
    CheckBox_Palette1_4->SetValue(false);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_4 = new wxButton(Panel4, ID_BUTTON_Palette1_4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_4"));
    Button_Palette1_4->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette1_5 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_5"));
    CheckBox_Palette1_5->SetValue(false);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_5 = new wxButton(Panel4, ID_BUTTON_Palette1_5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_5"));
    Button_Palette1_5->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette1_6 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette1_6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1_6"));
    CheckBox_Palette1_6->SetValue(false);
    FlexGridSizer_Palette1->Add(CheckBox_Palette1_6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette1_6 = new wxButton(Panel4, ID_BUTTON_Palette1_6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1_6"));
    Button_Palette1_6->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette1->Add(Button_Palette1_6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer47->Add(FlexGridSizer_Palette1, 1, wxTOP|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer34->Add(FlexGridSizer47, 1, wxTOP|wxBOTTOM|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer48 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText59 = new wxStaticText(Panel4, ID_STATICTEXT61, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer48->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Speed1 = new wxSlider(Panel4, ID_SLIDER_Speed1, 10, 1, 20, wxDefaultPosition, wxSize(30,150), wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Speed1"));
    FlexGridSizer48->Add(Slider_Speed1, 1, wxTOP|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34->Add(FlexGridSizer48, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer34, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer5->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxVERTICAL, Panel4, _("Effect 2"));
    FlexGridSizer49 = new wxFlexGridSizer(0, 3, 0, 0);
    Choicebook2 = new wxChoicebook(Panel4, ID_CHOICEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK2"));
    Panel2_None = new wxPanel(Choicebook2, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    Panel2_Bars = new wxPanel(Choicebook2, ID_PANEL9, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL9"));
    FlexGridSizer50 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer50->AddGrowableCol(1);
    StaticText42 = new wxStaticText(Panel2_Bars, ID_STATICTEXT47, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT47"));
    FlexGridSizer50->Add(StaticText42, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Bars2_BarCount = new wxSlider(Panel2_Bars, ID_SLIDER_Bars2_BarCount, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars2_BarCount"));
    FlexGridSizer50->Add(Slider_Bars2_BarCount, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText44 = new wxStaticText(Panel2_Bars, ID_STATICTEXT48, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer50->Add(StaticText44, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Bars2_Direction = new wxChoice(Panel2_Bars, ID_CHOICE_Bars2_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars2_Direction"));
    FlexGridSizer50->Add(Choice_Bars2_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Bars2_Highlight = new wxCheckBox(Panel2_Bars, ID_CHECKBOX_Bars2_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars2_Highlight"));
    CheckBox_Bars2_Highlight->SetValue(false);
    FlexGridSizer50->Add(CheckBox_Bars2_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Bars2_3D = new wxCheckBox(Panel2_Bars, ID_CHECKBOX_Bars2_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars2_3D"));
    CheckBox_Bars2_3D->SetValue(false);
    FlexGridSizer50->Add(CheckBox_Bars2_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Bars->SetSizer(FlexGridSizer50);
    FlexGridSizer50->Fit(Panel2_Bars);
    FlexGridSizer50->SetSizeHints(Panel2_Bars);
    Panel2_Butterfly = new wxPanel(Choicebook2, ID_PANEL11, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL11"));
    FlexGridSizer51 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer51->AddGrowableCol(1);
    StaticText47 = new wxStaticText(Panel2_Butterfly, ID_STATICTEXT49, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer51->Add(StaticText47, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Butterfly2_Colors = new wxChoice(Panel2_Butterfly, ID_CHOICE_Butterfly2_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly2_Colors"));
    FlexGridSizer51->Add(Choice_Butterfly2_Colors, 1, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText48 = new wxStaticText(Panel2_Butterfly, ID_STATICTEXT50, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    FlexGridSizer51->Add(StaticText48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly2_Style = new wxSlider(Panel2_Butterfly, ID_SLIDER_Butterfly2_Style, 1, 1, 3, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly2_Style"));
    FlexGridSizer51->Add(Slider_Butterfly2_Style, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText49 = new wxStaticText(Panel2_Butterfly, ID_STATICTEXT52, _("Bkgrd Chunks"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT52"));
    FlexGridSizer51->Add(StaticText49, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly2_Chunks = new wxSlider(Panel2_Butterfly, ID_SLIDER_Butterfly2_Chunks, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly2_Chunks"));
    FlexGridSizer51->Add(Slider_Butterfly2_Chunks, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText50 = new wxStaticText(Panel2_Butterfly, ID_STATICTEXT55, _("Bkgrd Skip"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT55"));
    FlexGridSizer51->Add(StaticText50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly2_Skip = new wxSlider(Panel2_Butterfly, ID_SLIDER_Butterfly2_Skip, 2, 2, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly2_Skip"));
    FlexGridSizer51->Add(Slider_Butterfly2_Skip, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Butterfly->SetSizer(FlexGridSizer51);
    FlexGridSizer51->Fit(Panel2_Butterfly);
    FlexGridSizer51->SetSizeHints(Panel2_Butterfly);
    Panel2_ColorWash = new wxPanel(Choicebook2, ID_PANEL13, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL13"));
    FlexGridSizer52 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer52->AddGrowableCol(1);
    StaticText76 = new wxStaticText(Panel2_ColorWash, ID_STATICTEXT22, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer52->Add(StaticText76, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_ColorWash2_Count = new wxSlider(Panel2_ColorWash, ID_SLIDER_ColorWash2_Count, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash2_Count"));
    FlexGridSizer52->Add(Slider_ColorWash2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText52 = new wxStaticText(Panel2_ColorWash, ID_STATICTEXT56, _("Horizontal"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT56"));
    FlexGridSizer52->Add(StaticText52, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash2_HFade = new wxCheckBox(Panel2_ColorWash, ID_CHECKBOX_ColorWash2_HFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash2_HFade"));
    CheckBox_ColorWash2_HFade->SetValue(false);
    FlexGridSizer52->Add(CheckBox_ColorWash2_HFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText55 = new wxStaticText(Panel2_ColorWash, ID_STATICTEXT58, _("Vertical"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer52->Add(StaticText55, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash2_VFade = new wxCheckBox(Panel2_ColorWash, ID_CHECKBOX_ColorWash2_VFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash2_VFade"));
    CheckBox_ColorWash2_VFade->SetValue(false);
    FlexGridSizer52->Add(CheckBox_ColorWash2_VFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_ColorWash->SetSizer(FlexGridSizer52);
    FlexGridSizer52->Fit(Panel2_ColorWash);
    FlexGridSizer52->SetSizeHints(Panel2_ColorWash);
    Panel2_Fire = new wxPanel(Choicebook2, ID_PANEL15, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL15"));
    FlexGridSizer53 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer53->AddGrowableCol(1);
    StaticText84 = new wxStaticText(Panel2_Fire, ID_STATICTEXT85, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT85"));
    FlexGridSizer53->Add(StaticText84, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fire2_Height = new wxSlider(Panel2_Fire, ID_SLIDER_Fire2_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire2_Height"));
    FlexGridSizer53->Add(Slider_Fire2_Height, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText133 = new wxStaticText(Panel2_Fire, ID_STATICTEXT134, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT134"));
    FlexGridSizer53->Add(StaticText133, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fire2_HueShift = new wxSlider(Panel2_Fire, ID_SLIDER_Fire2_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire2_HueShift"));
    FlexGridSizer53->Add(Slider_Fire2_HueShift, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText135 = new wxStaticText(Panel2_Fire, ID_STATICTEXT136, _("Grow Fire\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT136"));
    FlexGridSizer53->Add(StaticText135, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Fire2_GrowFire = new wxCheckBox(Panel2_Fire, ID_CHECKBOX_Fire2_GrowFire, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire2_GrowFire"));
    CheckBox_Fire2_GrowFire->SetValue(false);
    FlexGridSizer53->Add(CheckBox_Fire2_GrowFire, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Fire->SetSizer(FlexGridSizer53);
    FlexGridSizer53->Fit(Panel2_Fire);
    FlexGridSizer53->SetSizeHints(Panel2_Fire);
    Panel2_Garlands = new wxPanel(Choicebook2, ID_PANEL17, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL17"));
    FlexGridSizer54 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer54->AddGrowableCol(1);
    StaticText56 = new wxStaticText(Panel2_Garlands, ID_STATICTEXT59, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer54->Add(StaticText56, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands2_Type = new wxSlider(Panel2_Garlands, ID_SLIDER_Garlands2_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands2_Type"));
    FlexGridSizer54->Add(Slider_Garlands2_Type, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText58 = new wxStaticText(Panel2_Garlands, ID_STATICTEXT60, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer54->Add(StaticText58, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands2_Spacing = new wxSlider(Panel2_Garlands, ID_SLIDER_Garlands2_Spacing, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands2_Spacing"));
    FlexGridSizer54->Add(Slider_Garlands2_Spacing, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Garlands->SetSizer(FlexGridSizer54);
    FlexGridSizer54->Fit(Panel2_Garlands);
    FlexGridSizer54->SetSizeHints(Panel2_Garlands);
    Panel2_Life = new wxPanel(Choicebook2, ID_PANEL19, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL19"));
    FlexGridSizer55 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer55->AddGrowableCol(1);
    StaticText60 = new wxStaticText(Panel2_Life, ID_STATICTEXT62, _("Cells to start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer55->Add(StaticText60, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life2_Count = new wxSlider(Panel2_Life, ID_SLIDER_Life2_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life2_Count"));
    FlexGridSizer55->Add(Slider_Life2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText61 = new wxStaticText(Panel2_Life, ID_STATICTEXT63, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT63"));
    FlexGridSizer55->Add(StaticText61, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life2_Seed = new wxSlider(Panel2_Life, ID_SLIDER_Life2_Seed, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life2_Seed"));
    FlexGridSizer55->Add(Slider_Life2_Seed, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Life->SetSizer(FlexGridSizer55);
    FlexGridSizer55->Fit(Panel2_Life);
    FlexGridSizer55->SetSizeHints(Panel2_Life);
    Panel2_Meteors = new wxPanel(Choicebook2, ID_PANEL21, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL21"));
    FlexGridSizer56 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer56->AddGrowableCol(1);
    StaticText62 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT64, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT64"));
    FlexGridSizer56->Add(StaticText62, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors2_Type = new wxChoice(Panel2_Meteors, ID_CHOICE_Meteors2_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors2_Type"));
    FlexGridSizer56->Add(Choice_Meteors2_Type, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText129 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT130, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT130"));
    FlexGridSizer56->Add(StaticText129, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors2_Effect = new wxChoice(Panel2_Meteors, ID_CHOICE_Meteors2_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors2_Effect"));
    FlexGridSizer56->Add(Choice_Meteors2_Effect, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText63 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT65, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT65"));
    FlexGridSizer56->Add(StaticText63, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors2_Count = new wxSlider(Panel2_Meteors, ID_SLIDER_Meteors2_Count, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors2_Count"));
    FlexGridSizer56->Add(Slider_Meteors2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText64 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT66, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT66"));
    FlexGridSizer56->Add(StaticText64, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors2_Length = new wxSlider(Panel2_Meteors, ID_SLIDER_Meteors2_Length, 25, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors2_Length"));
    FlexGridSizer56->Add(Slider_Meteors2_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText131 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT132, _("Swirl Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT132"));
    FlexGridSizer56->Add(StaticText131, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors2_Swirl_Intensity = new wxSlider(Panel2_Meteors, ID_SLIDER_Meteors2_Swirl_Intensity, 0, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors2_Swirl_Intensity"));
    FlexGridSizer56->Add(Slider_Meteors2_Swirl_Intensity, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText106 = new wxStaticText(Panel2_Meteors, ID_STATICTEXT107, _("Meteors Fall Up\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT107"));
    FlexGridSizer56->Add(StaticText106, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Meteors2_FallUp = new wxCheckBox(Panel2_Meteors, ID_CHECKBOX_Meteors2_FallUp, _("Yes"), wxDefaultPosition, wxSize(46,13), 0, wxDefaultValidator, _T("ID_CHECKBOX_Meteors2_FallUp"));
    CheckBox_Meteors2_FallUp->SetValue(false);
    FlexGridSizer56->Add(CheckBox_Meteors2_FallUp, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Meteors->SetSizer(FlexGridSizer56);
    FlexGridSizer56->Fit(Panel2_Meteors);
    FlexGridSizer56->SetSizeHints(Panel2_Meteors);
    Panel2_Pictures = new wxPanel(Choicebook2, ID_PANEL23, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL23"));
    FlexGridSizer57 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer57->AddGrowableCol(0);
    Button_Pictures2_Filename = new wxButton(Panel2_Pictures, ID_BUTTON_PICTURES2_FILENAME, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PICTURES2_FILENAME"));
    Button_Pictures2_Filename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer57->Add(Button_Pictures2_Filename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Pictures2_Filename = new wxTextCtrl(Panel2_Pictures, ID_TEXTCTRL_Pictures2_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Pictures2_Filename"));
    FlexGridSizer57->Add(TextCtrl_Pictures2_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer64 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText65 = new wxStaticText(Panel2_Pictures, ID_STATICTEXT67, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT67"));
    FlexGridSizer64->Add(StaticText65, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Pictures2_Direction = new wxChoice(Panel2_Pictures, ID_CHOICE_Pictures2_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pictures2_Direction"));
    FlexGridSizer64->Add(Choice_Pictures2_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText97 = new wxStaticText(Panel2_Pictures, ID_STATICTEXT98, _("Animated Gif Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT98"));
    FlexGridSizer64->Add(StaticText97, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pictures2_GifSpeed = new wxSlider(Panel2_Pictures, ID_SLIDER_Pictures2_GifSpeed, 20, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pictures2_GifSpeed"));
    FlexGridSizer64->Add(Slider_Pictures2_GifSpeed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer57->Add(FlexGridSizer64, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Pictures->SetSizer(FlexGridSizer57);
    FlexGridSizer57->Fit(Panel2_Pictures);
    FlexGridSizer57->SetSizeHints(Panel2_Pictures);
    Panel2_Snowflakes = new wxPanel(Choicebook2, ID_PANEL25, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL25"));
    FlexGridSizer58 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer58->AddGrowableCol(1);
    StaticText81 = new wxStaticText(Panel2_Snowflakes, ID_STATICTEXT82, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT82"));
    FlexGridSizer58->Add(StaticText81, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes2_Count = new wxSlider(Panel2_Snowflakes, ID_SLIDER_Snowflakes2_Count, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes2_Count"));
    FlexGridSizer58->Add(Slider_Snowflakes2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText82 = new wxStaticText(Panel2_Snowflakes, ID_STATICTEXT83, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT83"));
    FlexGridSizer58->Add(StaticText82, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes2_Type = new wxSlider(Panel2_Snowflakes, ID_SLIDER_Snowflakes2_Type, 1, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes2_Type"));
    FlexGridSizer58->Add(Slider_Snowflakes2_Type, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Snowflakes->SetSizer(FlexGridSizer58);
    FlexGridSizer58->Fit(Panel2_Snowflakes);
    FlexGridSizer58->SetSizeHints(Panel2_Snowflakes);
    Panel2_Snowstorm = new wxPanel(Choicebook2, ID_PANEL27, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL27"));
    FlexGridSizer59 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer59->AddGrowableCol(1);
    StaticText66 = new wxStaticText(Panel2_Snowstorm, ID_STATICTEXT68, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT68"));
    FlexGridSizer59->Add(StaticText66, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm2_Count = new wxSlider(Panel2_Snowstorm, ID_SLIDER_Snowstorm2_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm2_Count"));
    FlexGridSizer59->Add(Slider_Snowstorm2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText67 = new wxStaticText(Panel2_Snowstorm, ID_STATICTEXT69, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT69"));
    FlexGridSizer59->Add(StaticText67, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm2_Length = new wxSlider(Panel2_Snowstorm, ID_SLIDER_Snowstorm2_Length, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm2_Length"));
    FlexGridSizer59->Add(Slider_Snowstorm2_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Snowstorm->SetSizer(FlexGridSizer59);
    FlexGridSizer59->Fit(Panel2_Snowstorm);
    FlexGridSizer59->SetSizeHints(Panel2_Snowstorm);
    Panel2_Spirals = new wxPanel(Choicebook2, ID_PANEL28, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL28"));
    FlexGridSizer60 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer60->AddGrowableCol(1);
    StaticText68 = new wxStaticText(Panel2_Spirals, ID_STATICTEXT70, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT70"));
    FlexGridSizer60->Add(StaticText68, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirals2_Count = new wxSlider(Panel2_Spirals, ID_SLIDER_Spirals2_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals2_Count"));
    FlexGridSizer60->Add(Slider_Spirals2_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText69 = new wxStaticText(Panel2_Spirals, ID_STATICTEXT71, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT71"));
    FlexGridSizer60->Add(StaticText69, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirals2_Rotation = new wxSlider(Panel2_Spirals, ID_SLIDER_Spirals2_Rotation, 20, -500, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals2_Rotation"));
    FlexGridSizer60->Add(Slider_Spirals2_Rotation, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText70 = new wxStaticText(Panel2_Spirals, ID_STATICTEXT72, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT72"));
    FlexGridSizer60->Add(StaticText70, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirals2_Thickness = new wxSlider(Panel2_Spirals, ID_SLIDER_Spirals2_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals2_Thickness"));
    FlexGridSizer60->Add(Slider_Spirals2_Thickness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText71 = new wxStaticText(Panel2_Spirals, ID_STATICTEXT73, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT73"));
    FlexGridSizer60->Add(StaticText71, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals2_Direction = new wxSlider(Panel2_Spirals, ID_SLIDER_Spirals2_Direction, 1, -1, 1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals2_Direction"));
    FlexGridSizer60->Add(Slider_Spirals2_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Spirals2_Blend = new wxCheckBox(Panel2_Spirals, ID_CHECKBOX_Spirals2_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals2_Blend"));
    CheckBox_Spirals2_Blend->SetValue(false);
    FlexGridSizer60->Add(CheckBox_Spirals2_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Spirals2_3D = new wxCheckBox(Panel2_Spirals, ID_CHECKBOX_Spirals2_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals2_3D"));
    CheckBox_Spirals2_3D->SetValue(false);
    FlexGridSizer60->Add(CheckBox_Spirals2_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Spirals->SetSizer(FlexGridSizer60);
    FlexGridSizer60->Fit(Panel2_Spirals);
    FlexGridSizer60->SetSizeHints(Panel2_Spirals);
    Panel2_Text = new wxPanel(Choicebook2, ID_PANEL29, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL29"));
    FlexGridSizer61 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer61->AddGrowableCol(0);
    FlexGridSizer66 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer66->AddGrowableCol(1);
    StaticText72 = new wxStaticText(Panel2_Text, ID_STATICTEXT74, _("Line 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT74"));
    FlexGridSizer66->Add(StaticText72, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text2_Line1 = new wxTextCtrl(Panel2_Text, ID_TEXTCTRL_Text2_Line1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text2_Line1"));
    TextCtrl_Text2_Line1->SetMaxLength(256);
    FlexGridSizer66->Add(TextCtrl_Text2_Line1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText73 = new wxStaticText(Panel2_Text, ID_STATICTEXT75, _("Line 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT75"));
    FlexGridSizer66->Add(StaticText73, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text2_Line2 = new wxTextCtrl(Panel2_Text, ID_TEXTCTRL_Text2_Line2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text2_Line2"));
    TextCtrl_Text2_Line2->SetMaxLength(256);
    FlexGridSizer66->Add(TextCtrl_Text2_Line2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer61->Add(FlexGridSizer66, 1, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Notebook2 = new wxNotebook(Panel2_Text, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
    Notebook2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    Panel_Text2_1 = new wxPanel(Notebook2, ID_PANEL_Text2_1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text2_1"));
    FlexGridSizer67 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Text2_1_Font = new wxButton(Panel_Text2_1, ID_BUTTON_TEXT2_1_FONT, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT2_1_FONT"));
    Button_Text2_1_Font->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer67->Add(Button_Text2_1_Font, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text2_1_Font = new wxTextCtrl(Panel_Text2_1, ID_TEXTCTRL_Text2_1_Font, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text2_1_Font"));
    FlexGridSizer67->Add(TextCtrl_Text2_1_Font, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText74 = new wxStaticText(Panel_Text2_1, ID_STATICTEXT76, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT76"));
    FlexGridSizer67->Add(StaticText74, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_1_Dir = new wxChoice(Panel_Text2_1, ID_CHOICE_Text2_1_Dir, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_1_Dir"));
    FlexGridSizer67->Add(Choice_Text2_1_Dir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText101 = new wxStaticText(Panel_Text2_1, ID_STATICTEXT102, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT102"));
    FlexGridSizer67->Add(StaticText101, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_1_Effect = new wxChoice(Panel_Text2_1, ID_CHOICE_Text2_1_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_1_Effect"));
    FlexGridSizer67->Add(Choice_Text2_1_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText99 = new wxStaticText(Panel_Text2_1, ID_STATICTEXT100, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT100"));
    FlexGridSizer67->Add(StaticText99, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_1_Count = new wxChoice(Panel_Text2_1, ID_CHOICE_Text2_1_Count, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_1_Count"));
    FlexGridSizer67->Add(Choice_Text2_1_Count, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText77 = new wxStaticText(Panel_Text2_1, ID_STATICTEXT78, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT78"));
    FlexGridSizer67->Add(StaticText77, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text2_1_Position = new wxSlider(Panel_Text2_1, ID_SLIDER_Text2_1_Position, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text2_1_Position"));
    FlexGridSizer67->Add(Slider_Text2_1_Position, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel_Text2_1->SetSizer(FlexGridSizer67);
    FlexGridSizer67->Fit(Panel_Text2_1);
    FlexGridSizer67->SetSizeHints(Panel_Text2_1);
    Panel_Text2_2 = new wxPanel(Notebook2, ID_PANEL_Text2_2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text2_2"));
    FlexGridSizer75 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Text2_2_Font = new wxButton(Panel_Text2_2, ID_BUTTON_TEXT2_2_FONT, _("Font"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEXT2_2_FONT"));
    Button_Text2_2_Font->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer75->Add(Button_Text2_2_Font, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text2_2_Font = new wxTextCtrl(Panel_Text2_2, ID_TEXTCTRL_Text2_2_Font, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Text2_2_Font"));
    FlexGridSizer75->Add(TextCtrl_Text2_2_Font, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText109 = new wxStaticText(Panel_Text2_2, ID_STATICTEXT110, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT110"));
    FlexGridSizer75->Add(StaticText109, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_2_Dir = new wxChoice(Panel_Text2_2, ID_CHOICE_Text2_2_Dir, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_2_Dir"));
    FlexGridSizer75->Add(Choice_Text2_2_Dir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText112 = new wxStaticText(Panel_Text2_2, ID_STATICTEXT113, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT113"));
    FlexGridSizer75->Add(StaticText112, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_2_Effect = new wxChoice(Panel_Text2_2, ID_CHOICE_Text2_2_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_2_Effect"));
    FlexGridSizer75->Add(Choice_Text2_2_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText111 = new wxStaticText(Panel_Text2_2, ID_STATICTEXT112, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT112"));
    FlexGridSizer75->Add(StaticText111, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text2_2_Count = new wxChoice(Panel_Text2_2, ID_CHOICE_Text2_2_Count, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text2_2_Count"));
    FlexGridSizer75->Add(Choice_Text2_2_Count, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText110 = new wxStaticText(Panel_Text2_2, ID_STATICTEXT111, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT111"));
    FlexGridSizer75->Add(StaticText110, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text2_2_Position = new wxSlider(Panel_Text2_2, ID_SLIDER_Text2_2_Position, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text2_2_Position"));
    FlexGridSizer75->Add(Slider_Text2_2_Position, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel_Text2_2->SetSizer(FlexGridSizer75);
    FlexGridSizer75->Fit(Panel_Text2_2);
    FlexGridSizer75->SetSizeHints(Panel_Text2_2);
    Notebook2->AddPage(Panel_Text2_1, _("Line 1 Details"), false);
    Notebook2->AddPage(Panel_Text2_2, _("Line 2 Details"), false);
    FlexGridSizer61->Add(Notebook2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    Panel2_Text->SetSizer(FlexGridSizer61);
    FlexGridSizer61->Fit(Panel2_Text);
    FlexGridSizer61->SetSizeHints(Panel2_Text);
    Panel2_Twinkle = new wxPanel(Choicebook2, ID_PANEL40, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL40"));
    FlexGridSizer76 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText113 = new wxStaticText(Panel2_Twinkle, ID_STATICTEXT114, _("Number Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT114"));
    FlexGridSizer76->Add(StaticText113, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle2_Count = new wxSlider(Panel2_Twinkle, ID_SLIDER_Twinkle2_Count, 3, 2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle2_Count"));
    FlexGridSizer76->Add(Slider_Twinkle2_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText114 = new wxStaticText(Panel2_Twinkle, ID_STATICTEXT115, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT115"));
    FlexGridSizer76->Add(StaticText114, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle2_Steps = new wxSlider(Panel2_Twinkle, ID_SLIDER_Twinkle2_Steps, 30, 2, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle2_Steps"));
    FlexGridSizer76->Add(Slider_Twinkle2_Steps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Twinkle2_Strobe = new wxCheckBox(Panel2_Twinkle, ID_CHECKBOX_Twinkle2_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle2_Strobe"));
    CheckBox_Twinkle2_Strobe->SetValue(false);
    FlexGridSizer76->Add(CheckBox_Twinkle2_Strobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText115 = new wxStaticText(Panel2_Twinkle, ID_STATICTEXT116, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT116"));
    FlexGridSizer76->Add(StaticText115, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Twinkle->SetSizer(FlexGridSizer76);
    FlexGridSizer76->Fit(Panel2_Twinkle);
    FlexGridSizer76->SetSizeHints(Panel2_Twinkle);
    Panel2_Tree = new wxPanel(Choicebook2, ID_PANEL41, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL41"));
    FlexGridSizer77 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText116 = new wxStaticText(Panel2_Tree, ID_STATICTEXT117, _("Number Branches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT117"));
    FlexGridSizer77->Add(StaticText116, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Tree2_Branches = new wxSlider(Panel2_Tree, ID_SLIDER_Tree2_Branches, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree2_Branches"));
    Slider_Tree2_Branches->SetThumbLength(3);
    Slider_Tree2_Branches->SetTick(1);
    Slider_Tree2_Branches->SetSelection(10, 0);
    FlexGridSizer77->Add(Slider_Tree2_Branches, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Tree->SetSizer(FlexGridSizer77);
    FlexGridSizer77->Fit(Panel2_Tree);
    FlexGridSizer77->SetSizeHints(Panel2_Tree);
    Panel2_Spirograph = new wxPanel(Choicebook2, ID_PANEL42, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL42"));
    FlexGridSizer78 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText117 = new wxStaticText(Panel2_Spirograph, ID_STATICTEXT118, _("R - Radius of outer circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT118"));
    FlexGridSizer78->Add(StaticText117, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph2_R = new wxSlider(Panel2_Spirograph, ID_SLIDER_Spirograph2_R, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph2_R"));
    FlexGridSizer78->Add(Slider_Spirograph2_R, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText118 = new wxStaticText(Panel2_Spirograph, ID_STATICTEXT119, _("r - Radius of inner circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT119"));
    FlexGridSizer78->Add(StaticText118, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph2_r = new wxSlider(Panel2_Spirograph, ID_SLIDER_Spirograph2_r, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph2_r"));
    FlexGridSizer78->Add(Slider_Spirograph2_r, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText119 = new wxStaticText(Panel2_Spirograph, ID_STATICTEXT120, _("d - Distance"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT120"));
    FlexGridSizer78->Add(StaticText119, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph2_d = new wxSlider(Panel2_Spirograph, ID_SLIDER_Spirograph2_d, 30, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph2_d"));
    FlexGridSizer78->Add(Slider_Spirograph2_d, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText120 = new wxStaticText(Panel2_Spirograph, ID_STATICTEXT121, _("Should distance\nbe animated\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT121"));
    FlexGridSizer78->Add(StaticText120, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Spirograph2_Animate = new wxCheckBox(Panel2_Spirograph, ID_CHECKBOX_Spirograph2_Animate, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirograph2_Animate"));
    CheckBox_Spirograph2_Animate->SetValue(false);
    FlexGridSizer78->Add(CheckBox_Spirograph2_Animate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText121 = new wxStaticText(Panel2_Spirograph, ID_STATICTEXT122, _("Note: r should be <= R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT122"));
    FlexGridSizer78->Add(StaticText121, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel2_Spirograph->SetSizer(FlexGridSizer78);
    FlexGridSizer78->Fit(Panel2_Spirograph);
    FlexGridSizer78->SetSizeHints(Panel2_Spirograph);
    Panel2_Fireworks = new wxPanel(Choicebook2, ID_PANEL43, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL43"));
    FlexGridSizer79 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText122 = new wxStaticText(Panel2_Fireworks, ID_STATICTEXT123, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT123"));
    FlexGridSizer79->Add(StaticText122, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks2_Number_Explosions = new wxSlider(Panel2_Fireworks, ID_SLIDER_Fireworks2_Number_Explosions, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks2_Number_Explosions"));
    FlexGridSizer79->Add(Slider_Fireworks2_Number_Explosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText123 = new wxStaticText(Panel2_Fireworks, ID_STATICTEXT124, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT124"));
    FlexGridSizer79->Add(StaticText123, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks2_Count = new wxSlider(Panel2_Fireworks, ID_SLIDER_Fireworks2_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks2_Count"));
    FlexGridSizer79->Add(Slider_Fireworks2_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText124 = new wxStaticText(Panel2_Fireworks, ID_STATICTEXT125, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT125"));
    FlexGridSizer79->Add(StaticText124, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks2_Velocity = new wxSlider(Panel2_Fireworks, ID_SLIDER_Fireworks2_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks2_Velocity"));
    FlexGridSizer79->Add(Slider_Fireworks2_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText125 = new wxStaticText(Panel2_Fireworks, ID_STATICTEXT126, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT126"));
    FlexGridSizer79->Add(StaticText125, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks2_Fade = new wxSlider(Panel2_Fireworks, ID_SLIDER_Fireworks2_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks2_Fade"));
    FlexGridSizer79->Add(Slider_Fireworks2_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Fireworks->SetSizer(FlexGridSizer79);
    FlexGridSizer79->Fit(Panel2_Fireworks);
    FlexGridSizer79->SetSizeHints(Panel2_Fireworks);
    Panel2_Circles = new wxPanel(Choicebook2, ID_PANEL38, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL38"));
    FlexGridSizer81 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText138 = new wxStaticText(Panel2_Circles, ID_STATICTEXT139, _("Number of Circles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT139"));
    FlexGridSizer81->Add(StaticText138, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Circles2_Count = new wxSlider(Panel2_Circles, ID_SLIDER_Circles2_count, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles2_count"));
    FlexGridSizer81->Add(Slider_Circles2_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText139 = new wxStaticText(Panel2_Circles, ID_STATICTEXT140, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT140"));
    FlexGridSizer81->Add(StaticText139, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Circles2_Size = new wxSlider(Panel2_Circles, ID_SLIDER_Circles2, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles2"));
    FlexGridSizer81->Add(Slider_Circles2_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles2_Bounce = new wxCheckBox(Panel2_Circles, ID_CHECKBOX_Circles2_Bounce, _("Bounce"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles2_Bounce"));
    CheckBox_Circles2_Bounce->SetValue(false);
    FlexGridSizer81->Add(CheckBox_Circles2_Bounce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles2_Collide = new wxCheckBox(Panel2_Circles, ID_CHECKBOX_Circles2_Collide, _("Collide"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles2_Collide"));
    CheckBox_Circles2_Collide->SetValue(false);
    FlexGridSizer81->Add(CheckBox_Circles2_Collide, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles2_Random_m = new wxCheckBox(Panel2_Circles, ID_CHECKBOX_Circles2_Random_m, _("Random Motion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles2_Random_m"));
    CheckBox_Circles2_Random_m->SetValue(false);
    FlexGridSizer81->Add(CheckBox_Circles2_Random_m, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Circles2_Radial = new wxCheckBox(Panel2_Circles, ID_CHECKBOX_Circles2_Radial, _("Radial"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles2_Radial"));
    CheckBox_Circles2_Radial->SetValue(false);
    FlexGridSizer81->Add(CheckBox_Circles2_Radial, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel2_Circles->SetSizer(FlexGridSizer81);
    FlexGridSizer81->Fit(Panel2_Circles);
    FlexGridSizer81->SetSizeHints(Panel2_Circles);
    Panel2_Piano = new wxPanel(Choicebook2, ID_PANEL44, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL44"));
    StaticText141 = new wxStaticText(Panel2_Piano, ID_STATICTEXT142, _("Keyboard"), wxPoint(24,32), wxDefaultSize, 0, _T("ID_STATICTEXT142"));
    Slider_Piano2_Keyboard = new wxSlider(Panel2_Piano, ID_SLIDER_Piano2_Keyboard, 3, 1, 3, wxPoint(120,32), wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano2_Keyboard"));
    Choicebook2->AddPage(Panel2_None, _("None"), false);
    Choicebook2->AddPage(Panel2_Bars, _("Bars"), false);
    Choicebook2->AddPage(Panel2_Butterfly, _("Butterfly"), false);
    Choicebook2->AddPage(Panel2_ColorWash, _("Color Wash"), false);
    Choicebook2->AddPage(Panel2_Fire, _("Fire"), false);
    Choicebook2->AddPage(Panel2_Garlands, _("Garlands"), false);
    Choicebook2->AddPage(Panel2_Life, _("Life"), false);
    Choicebook2->AddPage(Panel2_Meteors, _("Meteors"), false);
    Choicebook2->AddPage(Panel2_Pictures, _("Pictures"), false);
    Choicebook2->AddPage(Panel2_Snowflakes, _("Snowflakes"), false);
    Choicebook2->AddPage(Panel2_Snowstorm, _("Snowstorm"), false);
    Choicebook2->AddPage(Panel2_Spirals, _("Spirals"), false);
    Choicebook2->AddPage(Panel2_Text, _("Text"), false);
    Choicebook2->AddPage(Panel2_Twinkle, _("Twinkle"), false);
    Choicebook2->AddPage(Panel2_Tree, _("Tree"), false);
    Choicebook2->AddPage(Panel2_Spirograph, _("Spirograph"), false);
    Choicebook2->AddPage(Panel2_Fireworks, _("Fireworks"), false);
    Choicebook2->AddPage(Panel2_Circles, _("Circles (Under Development)"), false);
    Choicebook2->AddPage(Panel2_Piano, _("Piano"), false);
    FlexGridSizer49->Add(Choicebook2, 1, wxTOP|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer62 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Palette2 = new wxButton(Panel4, ID_BUTTON_Palette2, _("Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2"));
    Button_Palette2->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer62->Add(Button_Palette2, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Palette2 = new wxFlexGridSizer(0, 2, 0, 0);
    CheckBox_Palette2_1 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_1"));
    CheckBox_Palette2_1->SetValue(true);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_1 = new wxButton(Panel4, ID_BUTTON_Palette2_1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_1"));
    Button_Palette2_1->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette2_2 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_2"));
    CheckBox_Palette2_2->SetValue(true);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_2 = new wxButton(Panel4, ID_BUTTON_Palette2_2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_2"));
    Button_Palette2_2->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette2_3 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_3"));
    CheckBox_Palette2_3->SetValue(false);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_3 = new wxButton(Panel4, ID_BUTTON_Palette2_3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_3"));
    Button_Palette2_3->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette2_4 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_4"));
    CheckBox_Palette2_4->SetValue(false);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_4 = new wxButton(Panel4, ID_BUTTON_Palette2_4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_4"));
    Button_Palette2_4->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette2_5 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_5"));
    CheckBox_Palette2_5->SetValue(false);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_5 = new wxButton(Panel4, ID_BUTTON_Palette2_5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_5"));
    Button_Palette2_5->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    CheckBox_Palette2_6 = new wxCheckBox(Panel4, ID_CHECKBOX_Palette2_6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2_6"));
    CheckBox_Palette2_6->SetValue(false);
    FlexGridSizer_Palette2->Add(CheckBox_Palette2_6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Palette2_6 = new wxButton(Panel4, ID_BUTTON_Palette2_6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2_6"));
    Button_Palette2_6->SetMinSize(wxSize(30,20));
    FlexGridSizer_Palette2->Add(Button_Palette2_6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
    FlexGridSizer62->Add(FlexGridSizer_Palette2, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer49->Add(FlexGridSizer62, 1, wxTOP|wxBOTTOM|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer63 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText75 = new wxStaticText(Panel4, ID_STATICTEXT77, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT77"));
    FlexGridSizer63->Add(StaticText75, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Speed2 = new wxSlider(Panel4, ID_SLIDER_Speed2, 10, 1, 20, wxDefaultPosition, wxSize(30,150), wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Speed2"));
    FlexGridSizer63->Add(Slider_Speed2, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer49->Add(FlexGridSizer63, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer49, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer5->Add(StaticBoxSizer5, 1, wxALL|wxFIXED_MINSIZE|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    BoxSizer4->Add(BoxSizer5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(BoxSizer4);
    BoxSizer4->Fit(Panel4);
    BoxSizer4->SetSizeHints(Panel4);
    SplitterWindow2->SplitVertically(Panel5, Panel4);
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
    FileDialogConvert = new wxFileDialog(this, _("Select file"), wxEmptyString, wxEmptyString, _("LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|Vixen Sequences (*.vix)|*.vix|xLights Sequences(*.xseq)|*.xseq|Falcon Pi Player Sequences (*.fseq)|*.fseq|Lynx Conductor Sequences (*.seq)|*.seq"), wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_MULTIPLE, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    ColourDialog1 = new wxColourDialog(this);
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
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayEffectClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_UpdateGridClick);
    Connect(ID_BUTTON58,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ModelsClick);
    Connect(ID_BUTTON59,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PresetsClick);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_PresetsSelect);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PresetAddClick);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PresetUpdateClick);
    Connect(ID_CHOICE_LayerMethod,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoice_LayerMethodSelect);
    Connect(ID_BUTTON_PLAY_RGB_SEQ,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_PlayAllClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDisplayElementsClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ChannelMapClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_BITMAPBUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonInsertRowClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonDeleteRowClick);
    Connect(ID_BUTTON_SeqExport,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSeqExportClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnbtRandomEffectClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellRightClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellChange);
    Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&xLightsFrame::OnGrid1CellLeftClick);
    Connect(ID_BUTTON_PICTURES1_FILENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Pictures1_FilenameClick);
    Connect(ID_BUTTON_TEXT1_1_FONT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Text1_1_FontClick);
    Connect(ID_BUTTON_TEXT1_2_FONT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Text1_2_FontClick);
    Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnChoicebookEffectPageChanged);
    Connect(ID_BUTTON_Palette1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Palette1Click);
    Connect(ID_CHECKBOX_Palette1_1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette1_2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette1_3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette1_4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette1_5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette1_6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1_6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_BUTTON_PICTURES2_FILENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Pictures2_FilenameClick);
    Connect(ID_BUTTON_TEXT2_1_FONT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Text2_1_FontClick);
    Connect(ID_BUTTON_TEXT2_2_FONT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Text2_2_FontClick);
    Connect(ID_CHOICEBOOK2,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnChoicebookEffectPageChanged);
    Connect(ID_BUTTON_Palette2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_Palette2Click);
    Connect(ID_CHECKBOX_Palette2_1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette2_2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette2_3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette2_4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette2_5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
    Connect(ID_CHECKBOX_Palette2_6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2_6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ColorClick);
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
    Choice_Bars1_Direction->Set(BarEffectDirections);
    Choice_Bars1_Direction->SetSelection(0);
    Choice_Bars2_Direction->Set(BarEffectDirections);
    Choice_Bars2_Direction->SetSelection(0);

    ButterflyEffectColors.Add("Rainbow");
    ButterflyEffectColors.Add("Palette");
    Choice_Butterfly1_Colors->Set(ButterflyEffectColors);
    Choice_Butterfly1_Colors->SetSelection(0);
    Choice_Butterfly2_Colors->Set(ButterflyEffectColors);
    Choice_Butterfly2_Colors->SetSelection(0);


    MeteorsEffectTypes.Add("Rainbow");
    MeteorsEffectTypes.Add("Range");
    MeteorsEffectTypes.Add("Palette");
    Choice_Meteors1_Type->Set(MeteorsEffectTypes);
    Choice_Meteors1_Type->SetSelection(0);
    Choice_Meteors2_Type->Set(MeteorsEffectTypes);
    Choice_Meteors2_Type->SetSelection(0);
    MeteorsEffect.Add("Meteor");
    MeteorsEffect.Add("Swirl1");
    MeteorsEffect.Add("Swirl2");
    MeteorsEffect.Add("StarField");
    Choice_Meteors1_Effect->Set(MeteorsEffect);
    Choice_Meteors1_Effect->SetSelection(0);
    Choice_Meteors2_Effect->Set(MeteorsEffect);
    Choice_Meteors2_Effect->SetSelection(0);

    EffectDirections.Add("left");
    EffectDirections.Add("right");
    EffectDirections.Add("up");
    EffectDirections.Add("down");
    EffectDirections.Add("none");
    EffectDirections.Add("up-left");
    EffectDirections.Add("down-left");
    EffectDirections.Add("up-right");
    EffectDirections.Add("down-right");
    Choice_Pictures1_Direction->Set(EffectDirections);
    Choice_Pictures1_Direction->SetSelection(0);
    Choice_Pictures2_Direction->Set(EffectDirections);
    Choice_Pictures2_Direction->SetSelection(0);


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

    // Line 1 of each effect
    Choice_Text1_1_Dir->Set(EffectDirections);
    Choice_Text1_1_Dir->SetSelection(0);
    Choice_Text1_2_Dir->Set(EffectDirections);
    Choice_Text1_2_Dir->SetSelection(0);
    Choice_Text1_1_Effect->Set(TextEffects);
    Choice_Text1_1_Effect->SetSelection(0);
    Choice_Text1_2_Effect->Set(TextEffects);
    Choice_Text1_2_Effect->SetSelection(0);
    Choice_Text1_1_Count->Set(TextCountDown);
    Choice_Text1_1_Count->SetSelection(0);
    Choice_Text1_2_Count->Set(TextCountDown);
    Choice_Text1_2_Count->SetSelection(0);

    // Line 2 of each effect
    Choice_Text2_1_Dir->Set(EffectDirections);
    Choice_Text2_1_Dir->SetSelection(0);
    Choice_Text2_2_Dir->Set(EffectDirections);
    Choice_Text2_2_Dir->SetSelection(0);
    Choice_Text2_1_Effect->Set(TextEffects);
    Choice_Text2_1_Effect->SetSelection(0);
    Choice_Text2_2_Effect->Set(TextEffects);
    Choice_Text2_2_Effect->SetSelection(0);
    Choice_Text2_1_Count->Set(TextCountDown);
    Choice_Text2_1_Count->SetSelection(0);
    Choice_Text2_2_Count->Set(TextCountDown);
    Choice_Text2_2_Count->SetSelection(0);


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
    PaletteChanged=true;
    MixTypeChanged=true;
    HtmlEasyPrint=new wxHtmlEasyPrinting(wxT("xLights Printing"), this);
    basic.setFrame(this);
    PlayerDlg = new PlayerFrame(this, ID_PLAYER_DIALOG);

    /* The whol registration of effects is kind of a kludge but without moving all
     * of the unique panels out of wxSmith there really isn't a better way to do
     * this that I can think of.  This extension is to enable random effect generation
     */
    EffectNames.clear();
    EffectNames.resize(eff_LASTEFFECT); //
    EffectNames[eff_NONE]=wxT("None");
    EffectNames[eff_BARS]=wxT("Bars");
    EffectNames[eff_BUTTERFLY]=wxT("Butterfly");
    EffectNames[eff_COLORWASH]=wxT("Color Wash");
    EffectNames[eff_FIRE]=wxT("Fire");
    EffectNames[eff_GARLANDS]=wxT("Garlands");
    EffectNames[eff_LIFE]=wxT("Life");
    EffectNames[eff_METEORS]=wxT("Meteors");
    EffectNames[eff_PICTURES]=wxT("Pictures");
    EffectNames[eff_SNOWFLAKES]=wxT("Snowflakes");
    EffectNames[eff_SNOWSTORM]=wxT("Snowstorm");
    EffectNames[eff_SPIRALS]=wxT("Spirals");
    EffectNames[eff_TEXT]=wxT("Text");
    EffectNames[eff_TWINKLE]=wxT("Twinkle");
    EffectNames[eff_TREE]=wxT("Tree");
    EffectNames[eff_SPIROGRAPH]=wxT("Spirograph");
    EffectNames[eff_FIREWORKS]=wxT("Fireworks");
    EffectNames[eff_PIANO]=wxT("Piano");
    EffectNames[eff_CIRCLES]=wxT("Circles");

    EffectLayerOptions.clear();
    EffectLayerOptions.resize(LASTLAYER);
    EffectLayerOptions[EFFECT1]=wxT("Effect 1");
    EffectLayerOptions[EFFECT2]=wxT("Effect 2");
    EffectLayerOptions[EFF1MASK]=wxT("1 is Mask");
    EffectLayerOptions[EFF2MASK]=wxT("2 is Mask");
    EffectLayerOptions[EFF1UNMASK]=wxT("1 is Unmask");
    EffectLayerOptions[EFF2UNMASK]=wxT("2 is Unmask");
    EffectLayerOptions[EFFLAYERED]=wxT("Layered");
    EffectLayerOptions[EFFAVERAGED]=wxT("Average");

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
    wxImage::AddHandler(new wxGIFHandler);
    Timer1.Start(XTIMER_INTERVAL, wxTIMER_CONTINUOUS);
//   scm, causes crash if we remove this    Choicebook1->RemovePage(eff_CIRCLES);
//   Choicebook2->RemovePage(eff_CIRCLES);

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
    wxCriticalSectionLocker locker(gs_xoutCriticalSection);
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
    if (Notebook1->GetSelection() == TESTTAB && !xout)
    {
        StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
    }
    if (event.GetOldSelection() == TESTTAB)
    {
        AllLightsOff();
    }
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



// displays color chooser and updates the button's background color with the return value
void xLightsFrame::OnButton_ColorClick(wxCommandEvent& event)
{
    wxWindow* w=(wxWindow*)event.GetEventObject();
    if (ColourDialog1->ShowModal() == wxID_OK)
    {
        wxColourData retData = ColourDialog1->GetColourData();
        wxColour color = retData.GetColour();
        w->SetBackgroundColour(color);
        PaletteChanged=true;
    }
}

#include "TabSetup.cpp"
#include "TabTest.cpp"
#include "TabConvert.cpp"
#include "TabSchedule.cpp"
#include "TabSequence.cpp"

void xLightsFrame::OnChoice_ModelsSelect(wxCommandEvent& event)
{
}



void xLightsFrame::OnClose(wxCloseEvent& event)
{
    if (UnsavedChanges && wxNO == wxMessageBox("Quit without saving?",
            "Unsaved Changes", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    Destroy();
}
