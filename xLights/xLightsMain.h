/***************************************************************
 * Name:      xLightsMain.h
 * Purpose:   Defines Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#ifndef XLIGHTSMAIN_H
#define XLIGHTSMAIN_H


//(*Headers(xLightsFrame)
#include <wx/listctrl.h>
#include <wx/scrolwin.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/checklst.h>
#include <wx/msgdlg.h>
#include <wx/checkbox.h>
#include <wx/splitter.h>
#include <wx/radiobut.h>
#include <wx/colordlg.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/choicebk.h>
#include <wx/gauge.h>
#include <wx/statusbr.h>
//*)

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/choicdlg.h>
#include <wx/xml/xml.h>
#include <wx/fontdlg.h>
#include <wx/dir.h>

#include <wx/gauge.h>
#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#inlcude <windows.h>
#endif

#include <map>
#include <vector>

#include "../include/globals.h"
#include "xlights_out.h"
#include "PlayerFrame.h"
#include "AddShowDialog.h"
#include "ShowDatesDialog.h"
#include "PlaybackOptionsDialog.h"
#include "ModelListDialog.h"
#include "EffectListDialog.h"
#include "PaletteMgmtDialog.h"
#include "SeqParmsDialog.h"
#include "ChannelMapDialog.h"
#include "SeqOpenDialog.h"
#include "SeqExportDialog.h"
#include "SeqElementMismatchDialog.h"
#include "PixelBuffer.h"
#include "NetInfo.h"

// max number of most recently used show directories on the File menu
#define MRU_LENGTH 4

// notebook pages
#define SETUPTAB 0
#define TESTTAB 1
#define CONVERTTAB 2
#define SEQUENCETAB 3
#define SCHEDULETAB 4

#define FixedPages 5

static wxCriticalSection gs_xoutCriticalSection;

typedef std::map<wxString,wxString> MapStringString;
typedef std::vector<wxUint8> SeqDataType;

enum play_modes
{
    play_off,
    play_single,
    play_list,
    play_sched
};

enum TestFunctions
{
    OFF,
    CHASE,
    CHASE2,
    CHASE3,
    CHASE4,
    DIM,
    TWINKLE,
    SHIMMER
};

enum SeqPlayerStates
{
    NO_SEQ,
    STARTING_MEDIA,
    STARTING_SEQ,
    PLAYING_MEDIA,
    PLAYING_SEQ,
    STARTING_SEQ_ANIM,
    PLAYING_SEQ_ANIM,
    PAUSE_SEQ,
    DELAY_AFTER_PLAY,
    PLAYING_EFFECT
};

class SchedTreeData : public wxTreeItemData
{
protected:
    wxString eventString;

public:
    SchedTreeData(const wxString& EventData = wxT(""))
    {
        eventString = EventData;
    };
    wxString GetString()
    {
        return eventString;
    };
};



class xLightsFrame: public wxFrame
{
public:

    xLightsFrame(wxWindow* parent,wxWindowID id = -1);
    virtual ~xLightsFrame();

    // these are added to 1000*pagenum to get the control id
    enum PlayListIds
    {
        CHKBOX_AUDIO,
        CHKBOX_VIDEO,
        CHKBOX_XLIGHTS,
        CHKBOX_MOVIEMODE,
        UP_BUTTON,
        DOWN_BUTTON,
        INFO_BUTTON,
        PLAY_BUTTON,
        PLAY_ALL_BUTTON,
        DELAY_BUTTON,
        CREATE_SCRIPT,
        PLAYLIST_LISTBOX,
        PLAYLIST_FILES,
        PLAYLIST_ADD,
        PLAYLIST_ADD_ALL,
        PLAYLIST_DELETE,
        PLAYLIST_DELETE_ALL,
        SCRIPT_HELP,
        PLAYLIST_LOGIC,
        REMOVE_SCRIPT_BUTTON
    };
    long SecondsRemaining, EndTimeSec;
    int TxOverflowCnt;
    xOutput* xout;
    void BasicPrompt(char* prompt, char* buff, int size);
    void BasicOutput(char *msg);
    void BasicError(const char *msg);
    char ExtType(const wxString& ext);
    bool Play(wxString& filename, long delay);
    void StopPlayback();
    void StartScript(const char *scriptname);
    void EndScript(const char *scriptname);
    int  FindNotebookPage(wxString& pagename);
    wxWindow* FindNotebookControl(int nbidx, PlayListIds id);

private:

    //(*Handlers(xLightsFrame)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnMenuOpenFolderSelected(wxCommandEvent& event);
    void OnTimer1Trigger(wxTimerEvent& event);
    void OnCheckBoxRunScheduleClick(wxCommandEvent& event);
    void OnButtonAddShowClick(wxCommandEvent& event);
    void OnButtonUpdateShowClick(wxCommandEvent& event);
    void OnButtonDeleteShowClick(wxCommandEvent& event);
    void OnButtonShowDatesChangeClick(wxCommandEvent& event);
    void OnNotebook1PageChanged(wxNotebookEvent& event);
    void OnButtonClearLogClick(wxCommandEvent& event);
    void OnButtonSaveLogClick(wxCommandEvent& event);
    void OnMenuItemRenameListSelected(wxCommandEvent& event);
    void OnMenuItemRefreshSelected(wxCommandEvent& event);
    void OnMenuItemCustomScriptSelected(wxCommandEvent& event);
    void OnButtonNetworkChangeClick(wxCommandEvent& event);
    void OnButtonNetworkDeleteClick(wxCommandEvent& event);
    void OnButtonNetworkMoveUpClick(wxCommandEvent& event);
    void OnButtonNetworkMoveDownClick(wxCommandEvent& event);
    void OnGridNetworkBeginDrag(wxListEvent& event);
    void OnButtonAddE131Click(wxCommandEvent& event);
    void OnButtonAddDongleClick(wxCommandEvent& event);
    void OnButtonTestSelectAllClick(wxCommandEvent& event);
    void OnButtonTestClearClick(wxCommandEvent& event);
    void OnButtonTestLoadClick(wxCommandEvent& event);
    void OnButtonTestSaveClick(wxCommandEvent& event);
    void OnRadioButtonOffSelect(wxCommandEvent& event);
    void OnRadioButtonChaseSelect(wxCommandEvent& event);
    void OnRadioButtonChase3Select(wxCommandEvent& event);
    void OnRadioButtonChase4Select(wxCommandEvent& event);
    void OnRadioButtonChase5Select(wxCommandEvent& event);
    void OnRadioButtonAltSelect(wxCommandEvent& event);
    void OnRadioButtonTwinkle05Select(wxCommandEvent& event);
    void OnRadioButtonTwinkle10Select(wxCommandEvent& event);
    void OnRadioButtonTwinkle25Select(wxCommandEvent& event);
    void OnRadioButtonTwinkle50Select(wxCommandEvent& event);
    void OnRadioButtonShimmerSelect(wxCommandEvent& event);
    void OnRadioButtonDimSelect(wxCommandEvent& event);
    void OnCheckListBoxTestChannelsToggled(wxCommandEvent& event);
    void OnButtonSaveSetupClick(wxCommandEvent& event);
    void OnBitmapButtonTabInfoClick(wxCommandEvent& event);
    void OnMenuItemDelListSelected(wxCommandEvent& event);
    void OnMenuItemAddListSelected(wxCommandEvent& event);
    void OnButtonLightsOffClick(wxCommandEvent& event);
    void OnCheckBoxLightOutputClick(wxCommandEvent& event);
    void OnButtonStartConversionClick(wxCommandEvent& event);
    void OnButtonChooseFileClick(wxCommandEvent& event);
    void OnButtonStopNowClick(wxCommandEvent& event);
    void OnButtonGracefulStopClick(wxCommandEvent& event);
    void OnButtonSaveScheduleClick(wxCommandEvent& event);
    void OnMenuItemSavePlaylistsSelected(wxCommandEvent& event);
    void OnButtonNetworkDeleteAllClick(wxCommandEvent& event);
    void OnButton_PlayAllClick(wxCommandEvent& event);
    void OnButton_PlayEffectClick(wxCommandEvent& event);
    void OnButton_PresetsClick(wxCommandEvent& event);
    void OnChoice_PresetsSelect(wxCommandEvent& event);
    void OnButton_PresetAddClick(wxCommandEvent& event);
    void OnButton_PresetUpdateClick(wxCommandEvent& event);
    void OnChoice_LayerMethodSelect(wxCommandEvent& event);
    void OnButton_ModelsClick(wxCommandEvent& event);
    void OnButton_Palette1Click(wxCommandEvent& event);
    void OnButton_Palette2Click(wxCommandEvent& event);
    void OnButton_Text1_FontClick(wxCommandEvent& event);
    void OnButton_Text2_FontClick(wxCommandEvent& event);
    void OnButton_Pictures1_FilenameClick(wxCommandEvent& event);
    void OnButton_Pictures2_FilenameClick(wxCommandEvent& event);
    void OnButton_UpdateGridClick(wxCommandEvent& event);
    void OnButton_ChannelMapClick(wxCommandEvent& event);
    void OnBitmapButtonOpenSeqClick(wxCommandEvent& event);
    void OnBitmapButtonSaveSeqClick(wxCommandEvent& event);
    void OnBitmapButtonInsertRowClick(wxCommandEvent& event);
    void OnBitmapButtonDeleteRowClick(wxCommandEvent& event);
    void OnButtonDisplayElementsClick(wxCommandEvent& event);
    void OnGrid1CellChange(wxGridEvent& event);
    void OnGrid1CellLeftClick(wxGridEvent& event);
    void OnButton_ColorClick(wxCommandEvent& event);
    void OnChoicebookEffectPageChanged(wxChoicebookEvent& event);
    void OnButtonSeqExportClick(wxCommandEvent& event);
    void OnCheckBox_PaletteClick(wxCommandEvent& event);
    void OnSlider_Spirals1_CountCmdScroll(wxScrollEvent& event);
    void OnPanel1_NonePaint(wxPaintEvent& event);
    void OnTextCtrl_Text1_FontText(wxCommandEvent& event);
    void OnButton_Text1_1_FontClick(wxCommandEvent& event);
    void OnButton_Text1_2_FontClick(wxCommandEvent& event);
    //*)

    //(*Identifiers(xLightsFrame)
    static const long ID_BITMAPBUTTON_TAB_INFO;
    static const long ID_BUTTON_STOP_NOW;
    static const long ID_BUTTON_GRACEFUL_STOP;
    static const long ID_BUTTON_LIGHTS_OFF;
    static const long ID_CHECKBOX_LIGHT_OUTPUT;
    static const long ID_STATICTEXT_SETUP1;
    static const long ID_STATICTEXT_DIRNAME;
    static const long ID_BUTTON_CHANGEDIR;
    static const long ID_BUTTON_SAVE_SETUP;
    static const long ID_BUTTON_ADD_DONGLE;
    static const long ID_BUTTON_ADD_E131;
    static const long ID_BUTTON_NETWORK_CHANGE;
    static const long ID_BUTTON_NETWORK_DELETE;
    static const long ID_BUTTON_NETWORK_DELETE_ALL;
    static const long ID_BITMAPBUTTON1;
    static const long ID_BITMAPBUTTON2;
    static const long ID_LISTCTRL_NETWORKS;
    static const long ID_PANEL_SETUP;
    static const long ID_BUTTON_SELECT_ALL;
    static const long ID_BUTTON_CLEAR_ALL;
    static const long ID_BUTTON_LOAD;
    static const long ID_BUTTON_SAVE;
    static const long ID_STATICTEXT6;
    static const long ID_CHECKLISTBOX_TEST_CHANNELS;
    static const long ID_STATICTEXT8;
    static const long ID_RADIOBUTTON14;
    static const long ID_RADIOBUTTON15;
    static const long ID_RADIOBUTTON16;
    static const long ID_RADIOBUTTON17;
    static const long ID_RADIOBUTTON19;
    static const long ID_RADIOBUTTON18;
    static const long ID_RADIOBUTTON7;
    static const long ID_RADIOBUTTON31;
    static const long ID_RADIOBUTTON30;
    static const long ID_RADIOBUTTON29;
    static const long ID_RADIOBUTTON8;
    static const long ID_RADIOBUTTON6;
    static const long ID_STATICTEXT9;
    static const long ID_SLIDER4;
    static const long ID_STATICTEXT10;
    static const long ID_SLIDER5;
    static const long ID_STATICTEXT11;
    static const long ID_SLIDER6;
    static const long ID_PANEL_TEST_STANDARD;
    static const long ID_STATICTEXT29;
    static const long ID_RADIOBUTTON3;
    static const long ID_RADIOBUTTON4;
    static const long ID_RADIOBUTTON5;
    static const long ID_RADIOBUTTON9;
    static const long ID_RADIOBUTTON25;
    static const long ID_RADIOBUTTON10;
    static const long ID_RADIOBUTTON12;
    static const long ID_RADIOBUTTON28;
    static const long ID_RADIOBUTTON11;
    static const long ID_RADIOBUTTON13;
    static const long ID_RADIOBUTTON20;
    static const long ID_RADIOBUTTON21;
    static const long ID_SLIDER12;
    static const long ID_SLIDER11;
    static const long ID_SLIDER1;
    static const long ID_SLIDER14;
    static const long ID_SLIDER13;
    static const long ID_SLIDER9;
    static const long ID_STATICTEXT7;
    static const long ID_SLIDER2;
    static const long ID_PANEL_TEST_RGB;
    static const long ID_STATICTEXT12;
    static const long ID_RADIOBUTTON22;
    static const long ID_RADIOBUTTON23;
    static const long ID_RADIOBUTTON24;
    static const long ID_RADIOBUTTON26;
    static const long ID_RADIOBUTTON27;
    static const long ID_STATICTEXT13;
    static const long ID_SLIDER7;
    static const long ID_PANEL_RGB_CYCLE;
    static const long ID_NOTEBOOK_TEST;
    static const long ID_PANEL_TEST;
    static const long ID_STATICTEXT14;
    static const long ID_STATICTEXT19;
    static const long ID_STATICTEXT15;
    static const long ID_BUTTON_CHOOSE_FILE;
    static const long ID_TEXTCTRL_FILENAME;
    static const long ID_STATICTEXT16;
    static const long ID_CHOICE_OUTPUT_FORMAT;
    static const long ID_STATICTEXT20;
    static const long ID_CHECKBOX_SAVE_CHANNEL_NAMES;
    static const long ID_STATICTEXT17;
    static const long ID_CHECKBOX_OFF_AT_END;
    static const long ID_BUTTON_START_CONVERSION;
    static const long ID_STATICTEXT18;
    static const long ID_TEXTCTRL_CONVERSION_STATUS;
    static const long ID_PANEL_CONVERT;
    static const long ID_SCROLLEDWINDOW1;
    static const long ID_BUTTON13;
    static const long ID_BUTTON3;
    static const long ID_BUTTON58;
    static const long ID_CHOICE7;
    static const long ID_BUTTON59;
    static const long ID_CHOICE2;
    static const long ID_BUTTON9;
    static const long ID_BUTTON8;
    static const long ID_STATICTEXT23;
    static const long ID_CHOICE_LayerMethod;
    static const long ID_STATICTEXT24;
    static const long ID_SLIDER_SparkleFrequency;
    static const long ID_PANEL31;
    static const long ID_STATICTEXT4;
    static const long ID_BUTTON_PLAY_RGB_SEQ;
    static const long ID_BUTTON2;
    static const long ID_BUTTON1;
    static const long ID_BITMAPBUTTON7;
    static const long ID_BITMAPBUTTON9;
    static const long ID_BITMAPBUTTON3;
    static const long ID_BITMAPBUTTON4;
    static const long ID_BUTTON_SeqExport;
    static const long ID_GAUGE1;
    static const long ID_GRID1;
    static const long ID_PANEL4;
    static const long ID_STATICTEXT25;
    static const long ID_SLIDER_Bars1_BarCount;
    static const long ID_STATICTEXT26;
    static const long ID_CHOICE_Bars1_Direction;
    static const long ID_CHECKBOX_Bars1_Highlight;
    static const long ID_CHECKBOX_Bars1_3D;
    static const long ID_PANEL8;
    static const long ID_STATICTEXT27;
    static const long ID_CHOICE_Butterfly1_Colors;
    static const long ID_STATICTEXT28;
    static const long ID_SLIDER_Butterfly1_Style;
    static const long ID_STATICTEXT30;
    static const long ID_SLIDER_Butterfly1_Chunks;
    static const long ID_STATICTEXT31;
    static const long ID_SLIDER_Butterfly1_Skip;
    static const long ID_PANEL10;
    static const long ID_STATICTEXT21;
    static const long ID_SLIDER_ColorWash1_Count;
    static const long ID_STATICTEXT32;
    static const long ID_CHECKBOX_ColorWash1_HFade;
    static const long ID_STATICTEXT33;
    static const long ID_CHECKBOX_ColorWash1_VFade;
    static const long ID_PANEL5;
    static const long ID_STATICTEXT84;
    static const long ID_SLIDER_Fire1_Height;
    static const long ID_PANEL12;
    static const long ID_STATICTEXT34;
    static const long ID_SLIDER_Garlands1_Type;
    static const long ID_STATICTEXT35;
    static const long ID_SLIDER_Garlands1_Spacing;
    static const long ID_PANEL14;
    static const long ID_STATICTEXT36;
    static const long ID_SLIDER_Life1_Count;
    static const long ID_STATICTEXT37;
    static const long ID_SLIDER_Life1_Seed;
    static const long ID_PANEL16;
    static const long ID_STATICTEXT39;
    static const long ID_CHOICE_Meteors1_Type;
    static const long ID_STATICTEXT41;
    static const long ID_SLIDER_Meteors1_Count;
    static const long ID_STATICTEXT43;
    static const long ID_SLIDER_Meteors1_Length;
    static const long ID_PANEL18;
    static const long ID_BUTTON_PICTURES1_FILENAME;
    static const long ID_TEXTCTRL_Pictures1_Filename;
    static const long ID_STATICTEXT46;
    static const long ID_CHOICE_Pictures1_Direction;
    static const long ID_STATICTEXT97;
    static const long ID_SLIDER_Pictures1_GifSpeed;
    static const long ID_PANEL20;
    static const long ID_STATICTEXT80;
    static const long ID_SLIDER_Snowflakes1_Count;
    static const long ID_STATICTEXT81;
    static const long ID_SLIDER_Snowflakes1_Type;
    static const long ID_PANEL22;
    static const long ID_STATICTEXT45;
    static const long ID_SLIDER_Snowstorm1_Count;
    static const long ID_STATICTEXT51;
    static const long ID_SLIDER_Snowstorm1_Length;
    static const long ID_PANEL24;
    static const long ID_STATICTEXT38;
    static const long ID_SLIDER_Spirals1_Count;
    static const long ID_STATICTEXT40;
    static const long ID_SLIDER_Spirals1_Rotation;
    static const long ID_STATICTEXT42;
    static const long ID_SLIDER_Spirals1_Thickness;
    static const long ID_STATICTEXT44;
    static const long ID_SLIDER_Spirals1_Direction;
    static const long ID_CHECKBOX_Spirals1_Blend;
    static const long ID_CHECKBOX_Spirals1_3D;
    static const long ID_PANEL26;
    static const long ID_STATICTEXT53;
    static const long ID_TEXTCTRL_Text1_Line1;
    static const long ID_STATICTEXT54;
    static const long ID_TEXTCTRL_Text1_Line2;
    static const long ID_BUTTON_TEXT1_1_FONT;
    static const long ID_TEXTCTRL_Text1_1_Font;
    static const long ID_STATICTEXT79;
    static const long ID_CHOICE_Text1_1_Dir;
    static const long ID_STATICTEXT101;
    static const long ID_SLIDER_Text1_1_Position;
    static const long ID_STATICTEXT99;
    static const long ID_SLIDER_Text1_1_TextRotation;
    static const long ID_PANEL38;
    static const long ID_BUTTON_TEXT1_2_FONT;
    static const long ID_TEXTCTRL_Text1_2_Font;
    static const long ID_STATICTEXT57;
    static const long ID_CHOICE_Text1_2_Dir;
    static const long ID_STATICTEXT103;
    static const long ID_SLIDER_Text1_2_Position;
    static const long ID_STATICTEXT104;
    static const long ID_SLIDER_Text1_2_TextRotation;
    static const long ID_PANEL37;
    static const long ID_NOTEBOOK2;
    static const long ID_PANEL7;
    static const long ID_STATICTEXT86;
    static const long ID_SLIDER_Twinkle1_Count;
    static const long ID_PANEL33;
    static const long ID_STATICTEXT87;
    static const long ID_SLIDER_Tree1_Branches;
    static const long ID_PANEL34;
    static const long ID_STATICTEXT88;
    static const long ID_SLIDER_Spirograph1_R;
    static const long ID_STATICTEXT89;
    static const long ID_SLIDER_Spirograph1_r;
    static const long ID_STATICTEXT90;
    static const long ID_SLIDER_Spirograph1_d;
    static const long ID_STATICTEXT93;
    static const long ID_CHECKBOX_Spirograph1_Animate;
    static const long ID_STATICTEXT91;
    static const long ID_PANEL35;
    static const long ID_STATICTEXT95;
    static const long ID_SLIDER_Fireworks1_Number_Explosions;
    static const long ID_STATICTEXT92;
    static const long ID_SLIDER_Fireworks1_Count;
    static const long ID_STATICTEXT94;
    static const long ID_SLIDER_Fireworks1_Velocity;
    static const long ID_STATICTEXT96;
    static const long ID_SLIDER_Fireworks1_Fade;
    static const long ID_PANEL36;
    static const long ID_CHOICEBOOK1;
    static const long ID_BUTTON_Palette1;
    static const long ID_CHECKBOX_Palette1_1;
    static const long ID_BUTTON_Palette1_1;
    static const long ID_CHECKBOX_Palette1_2;
    static const long ID_BUTTON_Palette1_2;
    static const long ID_CHECKBOX_Palette1_3;
    static const long ID_BUTTON_Palette1_3;
    static const long ID_CHECKBOX_Palette1_4;
    static const long ID_BUTTON_Palette1_4;
    static const long ID_CHECKBOX_Palette1_5;
    static const long ID_BUTTON_Palette1_5;
    static const long ID_CHECKBOX_Palette1_6;
    static const long ID_BUTTON_Palette1_6;
    static const long ID_STATICTEXT61;
    static const long ID_SLIDER_Speed1;
    static const long ID_PANEL6;
    static const long ID_STATICTEXT47;
    static const long ID_SLIDER_Bars2_BarCount;
    static const long ID_STATICTEXT48;
    static const long ID_CHOICE_Bars2_Direction;
    static const long ID_CHECKBOX_Bars2_Highlight;
    static const long ID_CHECKBOX_Bars2_3D;
    static const long ID_PANEL9;
    static const long ID_STATICTEXT49;
    static const long ID_CHOICE_Butterfly2_Colors;
    static const long ID_STATICTEXT50;
    static const long ID_SLIDER_Butterfly2_Style;
    static const long ID_STATICTEXT52;
    static const long ID_SLIDER_Butterfly2_Chunks;
    static const long ID_STATICTEXT55;
    static const long ID_SLIDER_Butterfly2_Skip;
    static const long ID_PANEL11;
    static const long ID_STATICTEXT22;
    static const long ID_SLIDER_ColorWash2_Count;
    static const long ID_STATICTEXT56;
    static const long ID_CHECKBOX_ColorWash2_HFade;
    static const long ID_STATICTEXT58;
    static const long ID_CHECKBOX_ColorWash2_VFade;
    static const long ID_PANEL13;
    static const long ID_STATICTEXT85;
    static const long ID_SLIDER_Fire2_Height;
    static const long ID_PANEL15;
    static const long ID_STATICTEXT59;
    static const long ID_SLIDER_Garlands2_Type;
    static const long ID_STATICTEXT60;
    static const long ID_SLIDER_Garlands2_Spacing;
    static const long ID_PANEL17;
    static const long ID_STATICTEXT62;
    static const long ID_SLIDER_Life2_Count;
    static const long ID_STATICTEXT63;
    static const long ID_SLIDER_Life2_Seed;
    static const long ID_PANEL19;
    static const long ID_STATICTEXT64;
    static const long ID_CHOICE_Meteors2_Type;
    static const long ID_STATICTEXT65;
    static const long ID_SLIDER_Meteors2_Count;
    static const long ID_STATICTEXT66;
    static const long ID_SLIDER_Meteors2_Length;
    static const long ID_PANEL21;
    static const long ID_BUTTON_PICTURES2_FILENAME;
    static const long ID_TEXTCTRL_Pictures2_Filename;
    static const long ID_STATICTEXT67;
    static const long ID_CHOICE_Pictures2_Direction;
    static const long ID_STATICTEXT98;
    static const long ID_SLIDER_Pictures2_GifSpeed;
    static const long ID_PANEL23;
    static const long ID_STATICTEXT82;
    static const long ID_SLIDER_Snowflakes2_Count;
    static const long ID_STATICTEXT83;
    static const long ID_SLIDER_Snowflakes2_Type;
    static const long ID_PANEL25;
    static const long ID_STATICTEXT68;
    static const long ID_SLIDER_Snowstorm2_Count;
    static const long ID_STATICTEXT69;
    static const long ID_SLIDER_Snowstorm2_Length;
    static const long ID_PANEL27;
    static const long ID_STATICTEXT70;
    static const long ID_SLIDER_Spirals2_Count;
    static const long ID_STATICTEXT71;
    static const long ID_SLIDER_Spirals2_Rotation;
    static const long ID_STATICTEXT72;
    static const long ID_SLIDER_Spirals2_Thickness;
    static const long ID_STATICTEXT73;
    static const long ID_SLIDER_Spirals2_Direction;
    static const long ID_CHECKBOX_Spirals2_Blend;
    static const long ID_CHECKBOX_Spirals2_3D;
    static const long ID_PANEL28;
    static const long ID_STATICTEXT74;
    static const long ID_TEXTCTRL_Text2_Line1;
    static const long ID_STATICTEXT75;
    static const long ID_TEXTCTRL_Text2_Line2;
    static const long ID_STATICTEXT76;
    static const long ID_SLIDER_Text2_Top;
    static const long ID_STATICTEXT102;
    static const long ID_SLIDER_Text2_Left;
    static const long ID_BUTTON_TEXT2_FONT;
    static const long ID_TEXTCTRL_Text2_Font;
    static const long ID_STATICTEXT78;
    static const long ID_CHOICE_Text2_Dir;
    static const long ID_STATICTEXT100;
    static const long ID_SLIDERText2_TextRotation;
    static const long ID_PANEL29;
    static const long ID_CHOICEBOOK2;
    static const long ID_BUTTON_Palette2;
    static const long ID_CHECKBOX_Palette2_1;
    static const long ID_BUTTON_Palette2_1;
    static const long ID_CHECKBOX_Palette2_2;
    static const long ID_BUTTON_Palette2_2;
    static const long ID_CHECKBOX_Palette2_3;
    static const long ID_BUTTON_Palette2_3;
    static const long ID_CHECKBOX_Palette2_4;
    static const long ID_BUTTON_Palette2_4;
    static const long ID_CHECKBOX_Palette2_5;
    static const long ID_BUTTON_Palette2_5;
    static const long ID_CHECKBOX_Palette2_6;
    static const long ID_BUTTON_Palette2_6;
    static const long ID_STATICTEXT77;
    static const long ID_SLIDER_Speed2;
    static const long ID_PANEL32;
    static const long ID_SPLITTERWINDOW2;
    static const long ID_PANEL30;
    static const long ID_TREECTRL1;
    static const long ID_CHECKBOX_RUN_SCHEDULE;
    static const long ID_BUTTON_SAVE_SCHEDULE;
    static const long ID_BUTTON_ADD_SHOW;
    static const long ID_BUTTON_UPDATE_SHOW;
    static const long ID_BUTTON_DELETE_SHOW;
    static const long ID_STATICTEXT2;
    static const long ID_BUTTON_SHOW_DATES_CHANGE;
    static const long ID_STATICTEXT3;
    static const long ID_STATICTEXT_SHOWSTART;
    static const long ID_STATICTEXT5;
    static const long ID_STATICTEXT_SHOWEND;
    static const long ID_PANEL3;
    static const long ID_STATICTEXT1;
    static const long ID_TEXTCTRL_LOG;
    static const long ID_BUTTON_CLEARLOG;
    static const long ID_BUTTON_SAVELOG;
    static const long ID_PANEL2;
    static const long ID_SPLITTERWINDOW1;
    static const long ID_PANEL_CAL;
    static const long ID_NOTEBOOK1;
    static const long ID_PANEL1;
    static const long ID_MENUITEM2;
    static const long idMenuQuit;
    static const long idMenuSaveSched;
    static const long idMenuAddList;
    static const long idMenuRenameList;
    static const long idMenuDelList;
    static const long ID_MENUITEM1;
    static const long idCustomScript;
    static const long idMenuHelpContent;
    static const long idMenuAbout;
    static const long ID_STATUSBAR1;
    static const long ID_TIMER1;
    static const long ID_MESSAGEDIALOG1;
    //*)

    static const long ID_PLAYER_DIALOG;

    //(*Declarations(xLightsFrame)
    wxStaticText* StaticText6;
    wxCheckBox* CheckBox_Palette1_1;
    wxSlider* Slider_Spirograph1_d;
    wxFlexGridSizer* FlexGridSizer_Palette1;
    wxSlider* Slider_Spirals1_Thickness;
    wxGrid* Grid1;
    wxStaticText* StaticText19;
    wxRadioButton* RadioButtonOff;
    wxPanel* Panel1_Butterfly;
    wxRadioButton* RadioButtonRgbTwinkle25;
    wxCheckBox* CheckBox_Palette2_6;
    wxButton* Button_Palette2_6;
    wxPanel* Panel1_Fireworks;
    wxCheckBox* CheckBox_Spirals2_3D;
    wxButton* ButtonChangeDir;
    wxNotebook* Notebook1;
    wxSlider* Slider_Speed1;
    wxButton* ButtonNetworkChange;
    wxRadioButton* RadioButtonTwinkle50;
    wxButton* ButtonText1_2_Font;
    wxStaticText* StaticText4;
    wxChoicebook* Choicebook1;
    wxStaticText* StaticText38;
    wxPanel* Panel1_Life;
    wxStaticText* StaticText86;
    wxStaticText* StaticText42;
    wxSlider* Slider_Meteors1_Count;
    wxPanel* PanelSequence2;
    wxRadioButton* RadioButtonDim;
    wxSlider* Slider_Spirals1_Rotation;
    wxButton* ButtonLightsOff;
    wxCheckBox* CheckBox_Bars1_3D;
    wxStaticText* StaticText91;
    wxSlider* Slider_Butterfly1_Chunks;
    wxRadioButton* RadioButtonRgbDim;
    wxSplitterWindow* SplitterWindow2;
    wxBitmapButton* BitmapButtonOpenSeq;
    wxCheckBox* CheckBox_Palette1_2;
    wxRadioButton* RadioButtonRgbChase4;
    wxChoice* Choice_Models;
    wxPanel* Panel3;
    wxStaticText* StaticText45;
    wxStaticText* StaticText11;
    wxPanel* PanelTestRgb;
    wxSlider* Slider_Spirals1_Count;
    wxStaticText* StaticText18;
    wxStaticText* StaticText30;
    wxPanel* Panel1;
    wxSlider* Slider_Spirograph1_R;
    wxStaticText* StaticText103;
    wxSlider* Slider_Text1_2_TextRotation;
    wxRadioButton* RadioButtonRgbTwinkle10;
    wxPanel* Panel1_Fire;
    wxStaticText* StaticText48;
    wxSlider* SliderBgColorC;
    wxSlider* Slider_Snowflakes2_Count;
    wxCheckBox* CheckBoxSaveChannelNames;
    wxButton* Button_Pictures1_Filename;
    wxSlider* Slider_Snowflakes1_Count;
    wxStaticText* StaticText37;
    wxSlider* Slider_Spirals2_Direction;
    wxRadioButton* RadioButtonTwinkle05;
    wxStaticText* StaticText1;
    wxBitmapButton* BitmapButtonDeleteRow;
    wxSlider* Slider_Life1_Seed;
    wxStaticText* StaticText22;
    wxStaticText* StaticTextShowStart;
    wxPanel* Panel6;
    wxStaticText* StaticText87;
    wxStaticText* StaticText92;
    wxSlider* Slider_ColorWash1_Count;
    wxPanel* Panel2_Life;
    wxCheckBox* CheckBox_ColorWash2_VFade;
    wxStaticText* StaticTextShowEnd;
    wxStaticText* StaticText52;
    wxStaticText* StaticText96;
    wxPanel* Panel2;
    wxRadioButton* RadioButtonChase4;
    wxStaticText* StaticText10;
    wxSlider* Slider_Spirals1_Direction;
    wxPanel* Panel2_Text;
    wxSlider* Slider_Snowstorm1_Count;
    wxPanel* Panel2_Garlands;
    wxChoice* Choice_Bars1_Direction;
    wxCheckBox* CheckBox_Palette2_3;
    wxSlider* Slider_Life2_Seed;
    wxButton* Button_Text1_1_Font;
    wxCheckBox* CheckBox_Bars2_3D;
    wxRadioButton* RadioButtonChase5;
    wxSlider* Slider_Text1_1_Position;
    wxStaticText* StaticText24;
    wxButton* ButtonDisplayElements;
    wxButton* ButtonTestClear;
    wxChoice* Choice_Meteors1_Type;
    wxSlider* Slider_SparkleFrequency;
    wxRadioButton* RadioButtonAlt;
    wxCheckBox* CheckBox_ColorWash1_VFade;
    wxTextCtrl* TextCtrlLog;
    wxTreeCtrl* ListBoxSched;
    wxTextCtrl* TextCtrl_Text1_Line1;
    wxSlider* Slider_Speed2;
    wxSlider* Slider_Text2_TextRotation;
    wxSlider* Slider_Pictures2_GifSpeed;
    wxCheckBox* CheckBox_Spirals1_Blend;
    wxStaticText* StaticText13;
    wxPanel* Panel2_Fire;
    wxStaticText* StaticText65;
    wxStaticText* StaticText60;
    wxRadioButton* RadioButtonRgbAlt;
    wxRadioButton* RadioButtonTwinkle25;
    wxButton* ButtonTestSave;
    wxStaticText* StaticText49;
    wxButton* ButtonChooseFile;
    wxStaticText* StaticText64;
    wxSlider* Slider_Snowstorm1_Length;
    wxGauge* Gauge1;
    wxCheckBox* CheckBoxRunSchedule;
    wxStaticText* StaticText32;
    wxSlider* SliderFgColorC;
    wxStaticText* StaticText14;
    wxStaticText* StaticText36;
    wxStaticText* StaticText75;
    wxChoice* Choice_Bars2_Direction;
    wxSlider* Slider_Fireworks1_Count;
    wxPanel* Panel5;
    wxRadioButton* RadioButtonRgbChase5;
    wxStaticText* StaticText56;
    wxCheckBox* CheckBoxOffAtEnd;
    wxStaticText* StaticText8;
    wxSlider* Slider_Spirals2_Count;
    wxStaticText* StaticText25;
    wxStaticText* StaticText28;
    wxSlider* Slider_Fireworks1_Fade;
    wxStaticText* StaticText89;
    wxPanel* Panel2_Spirals;
    wxStaticText* StaticText23;
    wxStaticText* StaticText63;
    wxSlider* Slider_Butterfly1_Skip;
    wxRadioButton* RadioButtonRgbShimmer;
    wxStaticText* StaticText55;
    wxStaticText* StaticText15;
    wxChoice* Choice_Butterfly2_Colors;
    wxCheckBox* CheckBoxLightOutput;
    wxPanel* PanelSetup;
    wxCheckBox* CheckBox_Spirals1_3D;
    wxSlider* SliderFgColorB;
    wxPanel* PanelTestStandard;
    wxButton* Button_Palette2_1;
    wxChoice* Choice_LayerMethod;
    wxSlider* Slider_Text1_1_TextRotation;
    wxChoice* ChoiceOutputFormat;
    wxSlider* Slider_Text2_Left;
    wxPanel* PanelConvert;
    wxButton* Button_Palette1_3;
    wxStaticText* StaticText39;
    wxSlider* Slider_Meteors1_Length;
    wxCheckBox* CheckBox_Palette1_6;
    wxStaticText* StaticText78;
    wxBitmapButton* BitmapButtonTabInfo;
    wxStaticText* StaticText67;
    wxButton* ButtonAddDongle;
    wxButton* Button_Palette1_1;
    wxPanel* Panel1_ColorWash;
    wxCheckBox* CheckBox_Bars1_Highlight;
    wxSlider* Slider_Butterfly2_Chunks;
    wxTextCtrl* TextCtrl_Text1_Line2;
    wxSlider* Slider_Garlands1_Type;
    wxSlider* Slider_Bars2_BarCount;
    wxSlider* Slider_Meteors2_Length;
    wxPanel* Panel1_Bars;
    wxTextCtrl* TextCtrl_Text1_1_Font;
    wxPanel* Panel1_Twinkle;
    wxChoice* Choice_Text1_1_Dir;
    wxChoice* Choice_Pictures2_Direction;
    wxStaticText* StaticTextDirName;
    wxButton* Button_ChannelMap;
    wxStaticText* StaticText27;
    wxPanel* PanelTest;
    wxFileDialog* FileDialogConvert;
    wxChoice* Choice_Meteors2_Type;
    wxTimer Timer1;
    wxRadioButton* RadioButtonRgbChase;
    wxButton* ButtonUpdateShow;
    wxStaticText* StaticText70;
    wxStaticText* StaticText35;
    wxButton* ButtonShowDatesChange;
    wxStaticText* StaticText85;
    wxNotebook* Notebook2;
    wxChoice* Choice_Text1_2_Dir;
    wxPanel* Panel1_Garlands;
    wxTextCtrl* TextCtrl_Text2_Font;
    wxSlider* Slider_Fire1_Height;
    wxSlider* SliderRgbCycleSpeed;
    wxStaticText* StaticText76;
    wxButton* Button_Palette1_6;
    wxSlider* Slider_Garlands1_Spacing;
    wxButton* Button_Text2_Font;
    wxStaticText* StaticText72;
    wxRadioButton* RadioButtonChase;
    wxRadioButton* RadioButtonRgbChase3;
    wxPanel* Panel4;
    wxStaticText* StaticText53;
    wxMenuItem* MenuItemSavePlaylists;
    wxButton* ButtonAddE131;
    wxSlider* Slider_Spirals2_Rotation;
    wxButton* ButtonStartConversion;
    wxStatusBar* StatusBar1;
    wxStaticText* StaticText40;
    wxButton* Button_Palette2_5;
    wxStaticText* StaticText21;
    wxListCtrl* GridNetwork;
    wxCheckBox* CheckBox_Palette2_2;
    wxSlider* Slider_Snowflakes1_Type;
    wxRadioButton* RadioButtonRgbCycle4;
    wxButton* Button_Palette1_4;
    wxStaticText* StaticText57;
    wxStaticText* StaticText84;
    wxRadioButton* RadioButtonChase3;
    wxStaticText* StaticText90;
    wxButton* ButtonSaveLog;
    wxStaticText* StaticText93;
    wxCheckBox* CheckBox_Spirograph1_Animate;
    wxCheckListBox* CheckListBoxTestChannels;
    wxStaticText* StaticText61;
    wxStaticText* StaticText62;
    wxPanel* PanelRgbCycle;
    wxButton* Button_PresetUpdate;
    wxStaticText* StaticText54;
    wxCheckBox* CheckBox_Palette1_5;
    wxStaticText* StaticText7;
    wxButton* Button_UpdateGrid;
    wxFlexGridSizer* FlexGridSizer_Palette2;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxButton* Button_Presets;
    wxStaticText* StaticText79;
    wxStaticText* StaticText99;
    wxButton* ButtonDeleteShow;
    wxButton* ButtonNetworkDeleteAll;
    wxButton* ButtonSaveSchedule;
    wxPanel* Panel2_None;
    wxRadioButton* RadioButtonRgbCycle3;
    wxCheckBox* CheckBox_ColorWash2_HFade;
    wxButton* ButtonNetworkDelete;
    wxButton* Button_Palette1_5;
    wxPanel* Panel2_Butterfly;
    wxTextCtrl* TextCtrl_Text1_2_Font;
    wxSlider* Slider_Meteors2_Count;
    wxCheckBox* CheckBox_Palette2_5;
    wxMenuItem* MenuItemRefresh;
    wxSlider* Slider_ColorWash2_Count;
    wxButton* ButtonTestSelectAll;
    wxStaticText* StaticText44;
    wxStaticText* StaticText95;
    wxStaticText* StaticText98;
    wxSlider* Slider_Spirals2_Thickness;
    wxSlider* Slider_Snowstorm2_Length;
    wxStaticText* StaticText88;
    wxNotebook* NotebookTest;
    wxStaticText* StaticText59;
    wxPanel* Panel2_Bars;
    wxRadioButton* RadioButtonShimmer;
    wxSlider* Slider_Tree1_Branches;
    wxStaticText* StaticText68;
    wxPanel* Panel1_Pictures;
    wxStaticText* StaticText97;
    wxStaticText* StaticText71;
    wxButton* ButtonStopNow;
    wxStaticText* StaticText43;
    wxSlider* Slider_Fireworks1_Number_Explosions;
    wxStaticText* StaticText66;
    wxSlider* Slider_Butterfly1_Style;
    wxPanel* PanelCal;
    wxSlider* SliderFgColorA;
    wxPanel* Panel7;
    wxStaticText* StaticText100;
    wxButton* ButtonClearLog;
    wxStaticBoxSizer* StaticBoxSizerSequenceButtons;
    wxCheckBox* CheckBox_Palette2_1;
    wxSlider* Slider_Garlands2_Spacing;
    wxSlider* Slider_Text2_Top;
    wxRadioButton* RadioButtonRgbTwinkle05;
    wxPanel* Panel1_Meteors;
    wxStaticText* StaticText26;
    wxSlider* SliderBgColorA;
    wxTextCtrl* TextCtrlFilename;
    wxSlider* Slider_Snowflakes2_Type;
    wxTextCtrl* TextCtrl_Pictures2_Filename;
    wxButton* ButtonAddShow;
    wxRadioButton* RadioButtonRgbCycleOff;
    wxStaticText* StaticText50;
    wxStaticText* StaticText81;
    wxStaticText* StaticText58;
    wxSlider* Slider_Life2_Count;
    wxStaticText* StaticText17;
    wxCheckBox* CheckBox_ColorWash1_HFade;
    wxCheckBox* CheckBox_Bars2_Highlight;
    wxPanel* Panel1_None;
    wxSlider* Slider_Bars1_BarCount;
    wxSlider* SliderRgbChaseSpeed;
    wxStaticText* StaticText73;
    wxSlider* SliderChaseSpeed;
    wxPanel* Panel1_Snowstorm;
    wxPanel* Panel2_Snowstorm;
    wxTextCtrl* TextCtrl_Text2_Line1;
    wxButton* Button_Pictures2_Filename;
    wxSlider* Slider_Fire2_Height;
    wxColourDialog* ColourDialog1;
    wxChoice* Choice_Text2_Dir;
    wxSlider* Slider_Butterfly2_Skip;
    wxStaticText* StaticText47;
    wxStaticText* StaticText69;
    wxStaticText* StaticText3;
    wxButton* Button_Palette2_3;
    wxPanel* Panel2_Meteors;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxStaticText* StaticText80;
    wxStaticText* StaticText2;
    wxRadioButton* RadioButtonRgbCycleMixed;
    wxTextCtrl* TextCtrl_Pictures1_Filename;
    wxRadioButton* RadioButtonRgbTwinkle50;
    wxButton* ButtonGracefulStop;
    wxDirDialog* DirDialog1;
    wxStaticText* StaticText12;
    wxStaticText* StaticText31;
    wxChoice* Choice_Pictures1_Direction;
    wxCheckBox* CheckBox_Palette1_4;
    wxPanel* Panel2_ColorWash;
    wxSlider* Slider_Life1_Count;
    wxButton* ButtonTestLoad;
    wxMenu* MenuFile;
    wxSlider* SliderFgIntensity;
    wxButton* Button_Palette2_4;
    wxTextCtrl* TextCtrl_Text2_Line2;
    wxSlider* Slider_Garlands2_Type;
    wxSlider* Slider_Twinkle1_Count;
    wxButton* Button_PresetAdd;
    wxRadioButton* RadioButtonRgbChaseOff;
    wxStaticText* StaticText82;
    wxButton* ButtonSeqExport;
    wxStaticText* StaticText51;
    wxSlider* Slider_Spirograph1_r;
    wxStaticText* StaticText33;
    wxPanel* Panel2_Pictures;
    wxChoice* Choice_Butterfly1_Colors;
    wxSlider* SliderBgIntensity;
    wxPanel* Panel1_Text;
    wxCheckBox* CheckBox_Spirals2_Blend;
    wxButton* Button_Palette1_2;
    wxStaticText* StaticTextSetup1;
    wxButton* Button_PlayRgbSeq;
    wxStaticText* StaticText5;
    wxPanel* Panel1_Tree;
    wxSplitterWindow* SplitterWindow1;
    wxCheckBox* CheckBox_Palette2_4;
    wxRadioButton* RadioButtonRgbCycle5;
    wxChoice* Choice_Presets;
    wxButton* Button_Palette1;
    wxStaticText* StaticText16;
    wxSlider* SliderBgColorB;
    wxBitmapButton* BitmapButtonInsertRow;
    wxSlider* Slider_Text1_2_Position;
    wxCheckBox* CheckBox_Palette1_3;
    wxTextCtrl* TextCtrlConversionStatus;
    wxSlider* Slider_Fireworks1_Velocity;
    wxMessageDialog* MessageDialog1;
    wxSlider* Slider_Pictures1_GifSpeed;
    wxStaticText* StaticText20;
    wxButton* Button_Palette2_2;
    wxStaticText* StaticTextSequenceFileName;
    wxButton* Button_Palette2;
    wxStaticText* StaticText94;
    wxStaticText* StaticText46;
    wxStaticText* StaticText77;
    wxStaticText* StaticText83;
    wxStaticText* StaticText74;
    wxStaticText* StaticText41;
    wxSlider* Slider_Snowstorm2_Count;
    wxChoicebook* Choicebook2;
    wxStaticText* StaticText29;
    wxStaticText* StaticText34;
    wxPanel* Panel1_Spirals;
    wxButton* ButtonSaveSetup;
    wxButton* Button_PlayEffect;
    wxStaticText* StaticText101;
    wxPanel* Panel1_Snowflakes;
    wxStaticText* StaticText102;
    wxButton* Button_Models;
    wxBitmapButton* BitmapButtonSaveSeq;
    wxPanel* Panel1_Spirograph;
    wxStaticText* StaticText9;
    wxSlider* Slider_Butterfly2_Style;
    wxScrolledWindow* ScrolledWindow1;
    wxRadioButton* RadioButtonTwinkle10;
    wxPanel* Panel2_Snowflakes;
    //*)

    wxString CurrentDir;
    wxFileName networkFile;
    wxFileName scheduleFile;
    PlayerFrame* PlayerDlg;
    wxArrayString mru;  // most recently used directories
    wxMenuItem* mru_MenuItem[MRU_LENGTH];
    wxXmlDocument NetworkXML;
    long DragRowIdx;
    wxListCtrl* DragListBox;
    bool UnsavedChanges;
    wxDateTime starttime;
    play_modes play_mode;
    NetInfoClass NetInfo;

    void SetPlayMode(play_modes newmode);
    double rand01();
    bool EnableOutputs();
    void EnableNetworkChanges();
    void AllLightsOff();

    // setup
    void OnMenuMRU(wxCommandEvent& event);
    void SetDir(const wxString& dirname);
    void UpdateNetworkList();
    long GetNetworkSelection();
    void MoveNetworkRow(int fromRow, int toRow);
    void OnGridNetworkDragQuit(wxMouseEvent& event);
    void OnGridNetworkDragEnd(wxMouseEvent& event);
    void SetupDongle(wxXmlNode* e);
    void SetupE131(wxXmlNode* e);
    void SetChannelNamesForRgbModel(wxArrayString& ChNames, wxXmlNode* ModelNode);

    // test
    void SetTestCheckboxes(bool NewValue);
    void GetCheckedItems(wxArrayInt& chArray);
    void GetTestPresetNames(wxArrayString& PresetNames);
    void TestButtonsOff();
    bool CheckChannelList;
    int ChaseGrouping;
    int TwinkleRatio;
    TestFunctions TestFunc;
    void OnTimerTest(long curtime);

    // convert
    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels);
    void ReadVixFile(const char* filename);
    void ReadLorFile(const char* filename);
    void ReadXlightsFile(const wxString& FileName);
    void ReadConductorFile(const wxString& FileName);
    void SetMediaFilename(const wxString& filename);
    int GetLorTrack1Length(const char* filename);
    bool WriteVixenFile(const wxString& filename); //     Vixen *.vix
    void WriteVirFile(const wxString& filename); //       Vixen *.vir
    void WriteHLSFile(const wxString& filename);  //      HLS *.hlsnc
    void WriteXLightsFile(const wxString& filename); //   xLights *.xseq
    void WriteFalconPiFile(const wxString& filename); //  Falcon Pi Player *.pseq
    void WriteConductorFile(const wxString& filename); // Conductor *.seq
    void WriteLSPFile(const wxString& filename);  //      LSP UserPatterns.xml
    void WriteLorFile(const wxString& filename);  //      LOR *.lms, *.las
    void WriteLcbFile(const wxString& filename);  //      LOR *.lcb
    void ClearLastPeriod();
    void ConversionInit();
    void DoConversion(const wxString& FileName, const wxString& OutputFormat);
    void ConversionError(const wxString& msg);
    wxString base64_encode();
    std::string base64_decode(const wxString& encoded_string);

    wxString mediaFilename;
    wxString xlightsFilename;
    SeqDataType SeqData;
    long SeqDataLen;
    long SeqNumPeriods;
    long SeqNumChannels;
    wxArrayString FileNames;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    // schedule
    wxDateTime ShowStartDate,ShowEndDate;
    wxArrayString ShowEvents;
    SeqPlayerStates SeqPlayerState;
    wxString LastMoDay;
    long DelayAfterPlayMSEC;

    void AddShow(const wxDateTime& d, const wxString& StartStop, const wxString& Playlist);
    void DisplaySchedule();
    int DisplayScheduleOneDay(const wxDateTime& d, const wxTreeItemId& root);
    void PopulateShowDialog(AddShowDialog& dialog, wxSortedArrayString& SelectedDates);
    void UnpackSchedCode(const wxString& SchedCode, wxString& StartTime, wxString& EndTime, wxString& RepeatOptions, wxString& Playlist);
    void ForceScheduleCheck();
    void CheckRunSchedule();
    long GetSelectedItem(wxListCtrl* ListBoxPlay);
    void UpdateShowDates(const wxDateTime& NewStart, const wxDateTime NewEnd);
    bool DisplayAddShowDialog(AddShowDialog& dialog);
    int DeleteSelectedShows();
    void CheckSchedule();
    void LoadScheduleFile();
    void LoadSchedule(wxXmlNode* n);
    void LoadPlaylists(wxXmlNode* n);
    void LoadPlaylist(wxXmlNode* n);
    void AddPlaylist(const wxString& name);
    wxString OnOffString(bool b);
    wxString CreateScript(wxString ListName, bool Repeat, bool FirstItemOnce, bool LastItemOnce, bool LightsOff, bool Random);
    bool PlayCurrentXlightsFile();
    void RunPlaylist(int nbidx, wxString& script);
    void PlayerError(const wxString& msg);
    void SendToLogAndStatusBar(const wxString& msg);
    void ScanForFiles();
    long DiffSeconds(wxString& strTime, wxTimeSpan& tsCurrent);
    int Time2Seconds(const wxString& hhmm);
    void ResetTimer(SeqPlayerStates newstate, long OffsetMsec=0);
    void SaveScheduleFile();
    void OnButtonPlaylistAddClick(wxCommandEvent& event);
    void OnButtonPlaylistAddAllClick(wxCommandEvent& event);
    void OnButtonPlaylistDeleteClick(wxCommandEvent& event);
    void OnButtonPlaylistDeleteAllClick(wxCommandEvent& event);
    void OnButtonRemoveScriptClick(wxCommandEvent& event);
    void OnButtonRunPlaylistClick(wxCommandEvent& event);
    void OnButtonPlayItemClick(wxCommandEvent& event);
    void OnButtonUpClick(wxCommandEvent& event);
    void OnButtonDownClick(wxCommandEvent& event);
    void OnButtonSetDelayClick(wxCommandEvent& event);
    void OnFileTypeButtonClicked(wxCommandEvent& event);
    void OnPlayListBeginDrag(wxListEvent& event);
    void OnPlayListDragEnd(wxMouseEvent& event);
    void OnPlayListDragQuit(wxMouseEvent& event);
    void OnTimerPlaylist(long msec);

    // sequence
//    void OnCheckBox_PaletteClick(wxCommandEvent& event);
    void PresetsSelect();
    void LoadEffectsFile();
    bool SaveEffectsFile();
    void CreateDefaultEffectsXml();
    void UpdateEffectsList();
    void UpdateModelsList();
    void ChooseColor(wxTextCtrl* TextCtrl);
    wxString PageControlsToString(wxWindow* page);
    wxString SizerControlsToString(wxSizer* sizer);
    void LoadSizerControlsToAttr(wxSizer* sizer,wxXmlNode* x);
    void PlayRgbEffect(int EffectPeriod);
    void TimerRgbSeq(long msec);
    void UpdateBufferPalette();
    void SetChoicebook(wxChoicebook* cb, wxString& PageName);
    void SetEffectControls(wxString settings);
    wxXmlNode* CreateEffectNode(wxString& name);
    wxString CreateEffectString();
    void OpenPaletteDialog(const wxString& id1, const wxString& id2, wxSizer* PrimarySizer,wxSizer* SecondarySizer);
    void ChooseModelsForSequence();
    void GetGridColumnLabels(wxArrayString& a);
    void GetModelNames(wxArrayString& a);
    wxXmlNode* GetModelNode(const wxString& name);
    void DisplayXlightsFilename(const wxString& filename);
    void CopyRow(int row1, int row2);
    void NumericSort();
    double GetGridStartTime(int row);
    long GetGridStartTimeMSec(int row);
    void UpdateRgbPlaybackStatus(int seconds, const wxString& seqtype);
    void SetTextColor(wxWindow* w);
    void LoadEffectFromString(wxString settings, MapStringString& SettingsMap);
    void UpdateBufferPaletteFromMap(int PaletteNum, MapStringString& SettingsMap);
    void RenderEffectFromString(int layer, int period, MapStringString& SettingsMap);
    void ClearEffectWindow();
    void DisplayEffectOnWindow();
    void EnableSequenceControls(bool enable);
    void UpdateFont(wxTextCtrl* FontCtrl);
    void ResetEffectStates();
    void SeqLoadXlightsFile(const wxString& filename);
    void RenderGridToSeqData();
    void ResetEffectsXml();

    wxXmlDocument EffectsXml;
    wxXmlNode* EffectsNode;
    wxXmlNode* ModelsNode;
    wxXmlNode* PalettesNode;
    bool PaletteChanged;
    bool MixTypeChanged;
    long SeqBaseChannel;
    bool SeqChanCtrlBasic;
    bool SeqChanCtrlColor;
    wxString SeqXmlFileName;
    PixelBufferClass buffer;
    wxHtmlEasyPrinting* HtmlEasyPrint;
    int NextGridRowToPlay;
    int SeqPlayColumn;
    bool ResetEffectState[2];
    wxArrayString BarEffectDirections;
    wxArrayString ButterflyEffectColors;
    wxArrayString MeteorsEffectTypes;
    wxArrayString EffectDirections;
    wxArrayString TextEffectDirections;


    DECLARE_EVENT_TABLE()
};

#endif // XLIGHTSMAIN_H
