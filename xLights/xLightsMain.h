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
#include <wx/statusbr.h>
//*)

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/choicdlg.h>
#include <wx/xml/xml.h>
#include <wx/fontdlg.h>
#include <wx/dir.h>


#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#inlcude <windows.h>
#endif

#include <map>
#include <vector>

#include "EffectTreeDialog.h"
#include "../include/globals.h"
#include "xlights_out.h"
#include "PlayerFrame.h"
#include "EffectsPanel.h"
#include "AddShowDialog.h"
#include "ShowDatesDialog.h"
#include "PlaybackOptionsDialog.h"
#include "ModelListDialog.h"
#include "EffectListDialog.h"
#include "SeqParmsDialog.h"
#include "ChannelMapDialog.h"
#include "SeqOpenDialog.h"
#include "SeqExportDialog.h"
#include "SeqElementMismatchDialog.h"
#include "PixelBuffer.h"
#include "NetInfo.h"
#include "PaletteMgmtDialog.h"

class EffectTreeDialog;

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
    void SetEffectControls(wxString settings);
    wxXmlNode* CreateEffectNode(wxString& name);
    bool SaveEffectsFile();

    enum RGB_EFFECTS_e
    {
        eff_NONE,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_CURTAIN,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GARLANDS,
        eff_LIFE,
        eff_METEORS,
        eff_PIANO,
        eff_PICTURES,
        eff_SNOWFLAKES,
        eff_SNOWSTORM,
        eff_SPIRALS,
        eff_SPIROGRAPH,
        eff_TEXT,
        eff_TREE,
        eff_TWINKLE,
        eff_LASTEFFECT //Always the last entry
    };

    enum LAYER_OPTIONS_e
    {
        EFFECT1,
        EFFECT2,
        EFF1MASK,
        EFF2MASK,
        EFF1UNMASK,
        EFF2UNMASK,
        EFFLAYERED,
        EFFAVERAGED,
        LASTLAYER
    };
    wxArrayString EffectNames;
    wxArrayString EffectLayerOptions;
    wxString CurrentDir;

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
    void OnButton_UpdateGridClick(wxCommandEvent& event);
    void OnButton_ChannelMapClick(wxCommandEvent& event);
    void OnBitmapButtonOpenSeqClick(wxCommandEvent& event);
    void OnBitmapButtonSaveSeqClick(wxCommandEvent& event);
    void OnBitmapButtonInsertRowClick(wxCommandEvent& event);
    void OnBitmapButtonDeleteRowClick(wxCommandEvent& event);
    void OnButtonDisplayElementsClick(wxCommandEvent& event);
    void OnGrid1CellChange(wxGridEvent& event);
    void OnGrid1CellLeftClick(wxGridEvent& event);
    void OnButtonSeqExportClick(wxCommandEvent& event);
    void OnGrid1CellRightClick(wxGridEvent& event);
    void OnbtRandomEffectClick(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnButton_PaletteClick(wxCommandEvent& event);
    void OnSlider_EffectLayerMixCmdScroll(wxScrollEvent& event);
    void OnSlider_SparkleFrequencyCmdScroll(wxScrollEvent& event);
    void OnSlider_BrightnessCmdScroll(wxScrollEvent& event);
    void OnSlider_ContrastCmdScroll(wxScrollEvent& event);
    void OnMenuItemBackupSelected(wxCommandEvent& event);
    //*)

    void OnPopupClick(wxCommandEvent &evt);
    void DeleteSelectedEffects(wxCommandEvent &evt);
    void InsertRandomEffects(wxCommandEvent &evt);
    void UnprotectSelectedEffects(wxCommandEvent& evt);
    void ProtectSelectedEffects(wxCommandEvent& evt);


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
    static const long ID_BUTTON_Palette;
    static const long ID_CHOICE_LayerMethod;
    static const long ID_SLIDER_EffectLayerMix;
    static const long ID_TEXTCTRL_LayerMix;
    static const long ID_STATICTEXT24;
    static const long ID_SLIDER_SparkleFrequency;
    static const long ID_TEXTCTRL5;
    static const long ID_STATICTEXT127;
    static const long ID_SLIDER_Brightness;
    static const long ID_TEXTCTRL6;
    static const long ID_STATICTEXT128;
    static const long ID_SLIDER_Contrast;
    static const long ID_TEXTCTRL7;
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
    static const long ID_BUTTON4;
    static const long ID_GRID1;
    static const long ID_PANEL_EFFECTS1;
    static const long ID_PANEL_EFFECTS2;
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
    static const long ID_FILE_BACKUP;
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
    static const long ID_DELETE_EFFECT;
    static const long ID_IGNORE_CLICK;
    static const long ID_PROTECT_EFFECT;
    static const long ID_UNPROTECT_EFFECT;
    static const long ID_RANDOM_EFFECT;

    //(*Declarations(xLightsFrame)
    wxButton* Button_UpdateGrid;
    wxStaticText* StaticText10;
    wxRadioButton* RadioButtonChase5;
    wxBitmapButton* BitmapButtonOpenSeq;
    wxStaticText* StaticText22;
    wxStaticText* StaticText9;
    wxRadioButton* RadioButtonRgbCycle4;
    wxPanel* SeqPanelRight;
    wxTextCtrl* TextCtrlFilename;
    wxRadioButton* RadioButtonRgbChaseOff;
    wxStaticText* StaticText20;
    wxRadioButton* RadioButtonRgbCycleOff;
    wxButton* Button_Models;
    wxRadioButton* RadioButtonRgbChase;
    wxButton* ButtonAddE131;
    wxStaticBoxSizer* StaticBoxSizerSequenceButtons;
    wxRadioButton* RadioButtonRgbChase5;
    wxRadioButton* RadioButtonRgbChase3;
    wxStaticText* StaticText29;
    wxRadioButton* RadioButtonShimmer;
    wxCheckBox* CheckBoxRunSchedule;
    wxNotebook* Notebook1;
    wxButton* ButtonClearLog;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxButton* ButtonSeqExport;
    wxStaticText* StaticText13;
    wxStaticText* StaticText2;
    wxBitmapButton* BitmapButtonSaveSeq;
    wxSlider* SliderFgColorC;
    wxFileDialog* FileDialogConvert;
    wxSlider* SliderRgbChaseSpeed;
    wxStaticText* StaticText14;
    wxButton* ButtonLightsOff;
    wxButton* ButtonTestLoad;
    wxStaticText* StaticTextShowEnd;
    wxScrolledWindow* ScrolledWindow1;
    wxButton* ButtonGracefulStop;
    wxButton* Button1;
    wxRadioButton* RadioButtonRgbCycle3;
    wxChoice* ChoiceOutputFormat;
    wxTextCtrl* TextCtrlLog;
    wxStaticText* StaticText6;
    wxSplitterWindow* SplitterWindow2;
    wxMenuItem* MenuItemRefresh;
    wxRadioButton* RadioButtonDim;
    wxRadioButton* RadioButtonChase4;
    wxButton* Button_PlayRgbSeq;
    wxButton* Button_Presets;
    wxRadioButton* RadioButtonRgbTwinkle10;
    wxRadioButton* RadioButtonChase3;
    wxStaticText* StaticText19;
    wxSlider* SliderChaseSpeed;
    wxStaticText* StaticText8;
    wxButton* ButtonNetworkDeleteAll;
    wxButton* ButtonStartConversion;
    wxStaticText* StaticText11;
    wxStaticText* StaticText18;
    EffectsPanel* EffectsPanel1;
    wxSlider* SliderFgColorA;
    wxStaticText* StaticTextShowStart;
    wxPanel* PanelTestRgb;
    wxPanel* Panel1;
    wxStaticText* StaticText1;
    wxSlider* Slider_EffectLayerMix;
    wxButton* ButtonDisplayElements;
    wxStaticText* StaticText3;
    wxGrid* Grid1;
    wxPanel* PanelRgbCycle;
    wxRadioButton* RadioButtonRgbAlt;
    wxButton* ButtonSaveLog;
    wxStaticText* StaticText127;
    wxButton* ButtonChangeDir;
    wxStaticText* StaticTextDirName;
    wxTextCtrl* txtCtlBrightness;
    wxNotebook* NotebookTest;
    wxPanel* Panel3;
    wxTextCtrl* txtCtlContrast;
    wxSlider* SliderRgbCycleSpeed;
    wxButton* ButtonAddDongle;
    wxRadioButton* RadioButtonTwinkle50;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxMenuItem* MenuItemBackup;
    wxButton* Button_Palette;
    wxRadioButton* RadioButtonRgbCycleMixed;
    wxButton* ButtonNetworkChange;
    wxSlider* SliderBgColorA;
    wxRadioButton* RadioButtonRgbTwinkle50;
    wxStaticText* StaticTextSetup1;
    wxPanel* PanelSequence2;
    wxSlider* SliderBgColorC;
    wxRadioButton* RadioButtonRgbTwinkle05;
    wxButton* ButtonNetworkDelete;
    EffectsPanel* EffectsPanel2;
    wxTreeCtrl* ListBoxSched;
    wxStaticText* StaticText7;
    wxSlider* Slider_Brightness;
    wxPanel* PanelTest;
    wxListCtrl* GridNetwork;
    wxBitmapButton* BitmapButtonDeleteRow;
    wxPanel* PanelCal;
    wxStatusBar* StatusBar1;
    wxMenu* MenuFile;
    wxChoice* Choice_LayerMethod;
    wxDirDialog* DirDialog1;
    wxButton* ButtonTestClear;
    wxBitmapButton* BitmapButtonTabInfo;
    wxButton* ButtonDeleteShow;
    wxStaticText* StaticText126;
    wxSlider* Slider_SparkleFrequency;
    wxButton* ButtonShowDatesChange;
    wxRadioButton* RadioButtonRgbTwinkle25;
    wxRadioButton* RadioButtonRgbShimmer;
    wxCheckBox* CheckBoxLightOutput;
    wxStaticText* StaticText15;
    wxStaticText* StaticText12;
    wxCheckListBox* CheckListBoxTestChannels;
    wxPanel* PanelConvert;
    wxRadioButton* RadioButtonTwinkle10;
    wxPanel* PanelSetup;
    wxChoice* Choice_Models;
    wxSlider* SliderBgColorB;
    wxButton* ButtonSaveSetup;
    wxPanel* SeqPanelLeft;
    wxRadioButton* RadioButtonRgbDim;
    wxRadioButton* RadioButtonTwinkle25;
    wxPanel* Panel2;
    wxMenuItem* MenuItemSavePlaylists;
    wxRadioButton* RadioButtonChase;
    wxBitmapButton* BitmapButtonInsertRow;
    wxTextCtrl* txtCtrlSparkleFreq;
    wxButton* ButtonUpdateShow;
    wxSlider* Slider_Contrast;
    wxMessageDialog* MessageDialog1;
    wxPanel* PanelTestStandard;
    wxSlider* SliderBgIntensity;
    wxTextCtrl* TextCtrlConversionStatus;
    wxRadioButton* RadioButtonTwinkle05;
    wxSplitterWindow* SplitterWindow1;
    wxButton* ButtonChooseFile;
    wxRadioButton* RadioButtonOff;
    wxCheckBox* CheckBoxSaveChannelNames;
    wxStaticText* StaticText4;
    wxStaticText* StaticText17;
    wxRadioButton* RadioButtonRgbCycle5;
    wxTextCtrl* txtCtlEffectMix;
    wxRadioButton* RadioButtonRgbChase4;
    wxButton* Button_ChannelMap;
    wxButton* ButtonTestSelectAll;
    wxButton* Button_PlayEffect;
    wxButton* ButtonSaveSchedule;
    wxButton* ButtonAddShow;
    wxButton* ButtonTestSave;
    wxSlider* SliderFgIntensity;
    wxStaticText* StaticText16;
    wxSlider* SliderFgColorB;
    wxTimer Timer1;
    wxCheckBox* CheckBoxOffAtEnd;
    wxRadioButton* RadioButtonAlt;
    wxButton* ButtonStopNow;
    wxStaticText* StaticTextSequenceFileName;
    //*)


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

    int effGridPrevX;
    int effGridPrevY;


    void SetPlayMode(play_modes newmode);
    double rand01();
    bool EnableOutputs();
    void EnableNetworkChanges();
    void AllLightsOff();
    void InitEffectsPanel(EffectsPanel* panel);

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
    void ReadHLSFile(const wxString& filename);
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
    wxString LoadEffectsFileNoCheck();
    void CreateDefaultEffectsXml();
    void UpdateEffectsList();
    void UpdateModelsList();
    void ChooseColor(wxTextCtrl* TextCtrl);
    void LoadSizerControlsToAttr(wxSizer* sizer,wxXmlNode* x);
    void PlayRgbEffect(int EffectPeriod);
    bool PlayRgbEffect1(EffectsPanel* panel, int layer, int EffectPeriod);
    void TimerRgbSeq(long msec);
    void SetChoicebook(wxChoicebook* cb, wxString& PageName);


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
    void LoadSettingsMap(wxString settings, MapStringString& SettingsMap);
    void UpdateBufferFadesFromCtrl();
    void UpdateEffectDuration();
    void ResetEffectDuration();
    void UpdateBufferPalette(EffectsPanel* panel, int layer);
    void UpdateBufferPaletteFromMap(int PaletteNum, MapStringString& SettingsMap);
    bool RenderEffectFromMap(int layer, int period, MapStringString& SettingsMap);
    void UpdateBufferFadesFromMap(int effectNum, MapStringString& SettingsMap);
    void ClearEffectWindow();
    void DisplayEffectOnWindow();
    void EnableSequenceControls(bool enable);
    void ResetEffectStates();
    void SeqLoadXlightsFile(const wxString& filename);
    void RenderGridToSeqData();
    void ResetEffectsXml();
    void ImportAudacityTimings();
    void ProcessAudacityTimingFile(const wxString& filename);
    void ImportxLightsXMLTimings();
    void ProcessxLightsXMLTimingsFile(const wxString& filename);
    void SeqLoadXlightsXSEQ(const wxString& filename);
    wxString CreateEffectStringRandom();
    wxString InsertMissing(wxString str,wxString missing_array,bool INSERT);
    void FixVersionDifferences(wxString file);
    void BackupDirectory(wxString targetDirName);

    wxXmlDocument EffectsXml;
    wxXmlNode* EffectsNode;
    wxXmlNode* ModelsNode;
    wxXmlNode* PalettesNode;
    bool MixTypeChanged;
    bool FadesChanged;
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
    wxArrayString MeteorsEffect;
    wxArrayString EffectDirections;
    wxArrayString TextEffects;
    wxArrayString TextCountDown;
    wxArrayString CurtainEdge;
    wxArrayString CurtainEffect;
    wxGridCellCoords *curCell;
    EffectTreeDialog *EffectTreeDlg;

    DECLARE_EVENT_TABLE()
};

#endif // XLIGHTSMAIN_H
