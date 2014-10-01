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
#include <wx/listbox.h>
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
#include <unordered_map> //-DJ


#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#inlcude <windows.h>
#endif

#include <map>
#include <set>
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
#include "ExportModelSelect.h"

class EffectTreeDialog;

// max number of most recently used show directories on the File menu
#define MRU_LENGTH 4

// notebook pages
#define SETUPTAB 0
#define TESTTAB 1
#define CONVERTTAB 2
#define PREVIEWTAB 3
#define SEQUENCETAB 4
#define SCHEDULETAB 5
#define PAPAGAYOTAB 6

#define FixedPages 7

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
    PAUSE_SEQ_ANIM,
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
    int TxOverflowCnt, TxOverflowTotal;
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
    void SetEffectControls(wxString settings, const wxString& model_name);
    wxXmlNode* CreateEffectNode(wxString& name);
    bool SaveEffectsFile();

    enum RGB_EFFECTS_e
    {
        eff_NONE,
        eff_BARS,
        eff_BUTTERFLY,
        eff_CIRCLES,
        eff_COLORWASH,
        eff_COROFACES,
        eff_CURTAIN,
        eff_FACES,
        eff_FIRE,
        eff_FIREWORKS,
        eff_GARLANDS,
        eff_GLEDIATOR,
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
        eff_SINGLESTRAND,
        eff_WAVE,
        eff_LASTEFFECT //Always the last entry
    };

    /* copy from sep 29
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
        eff_SINGLESTRAND,
        eff_FACES,
        eff_WAVE,
        eff_GLEDIATOR,
        eff_COROFACES,
        eff_LASTEFFECT //Always the last entry
    };
    */

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
    static wxString CurrentDir; //expose current folder name -DJ
    static wxString PlaybackMarker; //keep track of where we are within grid -DJ
    static wxString xlightsFilename; //expose current path name -DJ

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
    void OnButtonAddElementClick(wxCommandEvent& event);
    void OnButtonChangeElementClick(wxCommandEvent& event);
    void OnButtonRenameElementClick(wxCommandEvent& event);
    void OnButtonDeleteElementClick(wxCommandEvent& event);
    void OnButtonNodeLayoutClick(wxCommandEvent& event);
    void OnButtonSavePreviewClick(wxCommandEvent& event);
    void OnButtonSetBackgroundClick(wxCommandEvent& event);
    void OnButtonClearBackgroundClick(wxCommandEvent& event);
    void OnButtonPreviewOpenClick(wxCommandEvent& event);
    void OnListBoxElementListSelect(wxCommandEvent& event);
    void OnScrolledWindow1Resize(wxSizeEvent& event);
    void OnScrolledWindowPreviewLeftUp(wxMouseEvent& event);
    void OnScrolledWindowPreviewMouseLeave(wxMouseEvent& event);
    void OnScrolledWindowPreviewLeftDown(wxMouseEvent& event);
    void OnScrolledWindowPreviewResize(wxSizeEvent& event);
    void OnScrolledWindowPreviewMouseMove(wxMouseEvent& event);
    void OnSliderPreviewScaleCmdSliderUpdated(wxScrollEvent& event);
    void OnButtonModelsPreviewClick(wxCommandEvent& event);
    void OnButtonPlayPreviewClick(wxCommandEvent& event);
    void OnButtonStopPreviewClick(wxCommandEvent& event);
    void OnSliderPreviewTimeCmdSliderUpdated(wxScrollEvent& event);
    void OnSliderPreviewRotateCmdSliderUpdated(wxScrollEvent& event);
    void OnPaneNutcrackerChar(wxKeyEvent& event);
    void OnBitmapButton_EffectLayerMixClick(wxCommandEvent& event);
    void OnBitmapButton_SparkleFrequencyClick(wxCommandEvent& event);
    void OnBitmapButton_BrightnessClick(wxCommandEvent& event);
    void OnBitmapButton_ContrastClick(wxCommandEvent& event);
    void OnButtonModelExportClick(wxCommandEvent& event);
    void OnBitmapButtonGridCutClick(wxCommandEvent& event);
    void OnBitmapButtonGridCopyClick(wxCommandEvent& event);
    void OnBitmapButtonGridPasteClick(wxCommandEvent& event);
    void OnEffectsPanel1Paint(wxPaintEvent& event);
    void OnGrid1SetFocus(wxFocusEvent& event);
    void OnGrid1KillFocus(wxFocusEvent& event);
    void OnBitmapButton_CheckBox_LayerMorphClick(wxCommandEvent& event);
    void OnBitmapButton_LayerMorphClick(wxCommandEvent& event);
    void OnTextCtrlPreviewElementSizeText(wxCommandEvent& event);
    void OnNotebook2PageChanged(wxNotebookEvent& event);
    void OnButtonStartPapagayoClick(wxCommandEvent& event);
    void OnButton_pgo_filenameClick(wxCommandEvent& event);
    //    void OnButton_papagayo_output_sequenceClick(wxCommandEvent& event);
    void OnButton_papagayo_output_sequenceClick1(wxCommandEvent& event);
    void OnTextCtrl1Text(wxCommandEvent& event);
    void OnTextCtrl20Text(wxCommandEvent& event);
    void OnButtonPgoImageClick(wxCommandEvent& event);
    void OntxtCtrlSparkleFreqText(wxCommandEvent& event);
    void OnBitmapButton_SaveCoroGroupClick(wxCommandEvent& event);
    void OnBitmapButton_OpenCoroGroupClick(wxCommandEvent& event);
    void OnButton_CoroGroupDeleteClick(wxCommandEvent& event);
    void OnChoice_PgoGroupNameSelect(wxCommandEvent& event);
    void OnButton_CoroGroupClearClick(wxCommandEvent& event);
    void OnGridCoroFacesCellSelect(wxGridEvent& event);
    void OnNotebookPgoParmsPageChanged(wxNotebookEvent& event);
    void OnTimer2Trigger(wxTimerEvent& event);
    void OnButton_PgoStitchClick(wxCommandEvent& event);
    void OnCheckBox_AutoFadePgoAllClick(wxCommandEvent& event);
    //*)

    void OnPopupClick(wxCommandEvent &evt);
    void DeleteSelectedEffects(wxCommandEvent &evt);
    void InsertRandomEffects(wxCommandEvent &evt);
    void UnprotectSelectedEffects(wxCommandEvent& evt);
    void ProtectSelectedEffects(wxCommandEvent& evt);
    void CopyEffectAcrossRow(wxCommandEvent& evt); //-DJ
    void ClearEffectRow(wxCommandEvent& evt); //-DJ
//    void ConnectOnChar(wxWindow* pclComponent);

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
    static const long ID_STATICTEXT68;
    static const long ID_STATICTEXT15;
    static const long ID_BUTTON_CHOOSE_FILE;
    static const long ID_TEXTCTRL_FILENAME;
    static const long ID_STATICTEXT16;
    static const long ID_CHOICE_OUTPUT_FORMAT;
    static const long ID_STATICTEXT20;
    static const long ID_CHECKBOX_MAP_EMPTY_CHANNELS;
    static const long ID_STATICTEXT17;
    static const long ID_CHECKBOX_OFF_AT_END;
    static const long ID_BUTTON_START_CONVERSION;
    static const long ID_STATICTEXT18;
    static const long ID_TEXTCTRL_CONVERSION_STATUS;
    static const long ID_PANEL_CONVERT;
    static const long ID_BUTTON_PREVIEW_OPEN;
    static const long ID_STATICTEXT23;
    static const long ID_BUTTON_PLAY_PREVIEW;
    static const long ID_BUTTON_STOP_PREVIEW;
    static const long ID_TEXTCTRL_PREVIEW_TIME;
    static const long ID_SLIDER_PREVIEW_TIME;
    static const long ID_STATICTEXT21;
    static const long ID_LISTBOX_ELEMENT_LIST;
    static const long ID_BUTTON_MODELS_PREVIEW;
    static const long ID_BUTTON_SAVE_PREVIEW;
    static const long ID_STATICTEXT22;
    static const long ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE;
    static const long ID_SLIDER_PREVIEW_SCALE;
    static const long ID_STATICTEXT25;
    static const long ID_SLIDER_PREVIEW_ROTATE;
    static const long ID_SCROLLEDWINDOW_PREVIEW;
    static const long ID_PANEL_PREVIEW;
    static const long ID_SCROLLEDWINDOW1;
    static const long ID_BUTTON13;
    static const long ID_BUTTON3;
    static const long ID_BUTTON58;
    static const long ID_CHOICE7;
    static const long ID_BUTTON59;
    static const long ID_BUTTON_Palette;
    static const long ID_CHECKBOX_LayerMorph;
    static const long ID_BITMAPBUTTON_CHECKBOX_LayerMorph;
    static const long ID_CHOICE_LayerMethod;
    static const long ID_SLIDER_EffectLayerMix;
    static const long ID_TEXTCTRL_LayerMix;
    static const long ID_BITMAPBUTTON_SLIDER_EffectLayerMix;
    static const long ID_STATICTEXT24;
    static const long ID_SLIDER_SparkleFrequency;
    static const long ID_TEXTCTRL5;
    static const long ID_BITMAPBUTTON_SLIDER_SparkleFrequency;
    static const long ID_STATICTEXT127;
    static const long ID_SLIDER_Brightness;
    static const long ID_TEXTCTRL6;
    static const long ID_BITMAPBUTTON_SLIDER_Brightness;
    static const long ID_STATICTEXT128;
    static const long ID_SLIDER_Contrast;
    static const long ID_TEXTCTRL7;
    static const long ID_BITMAPBUTTON_SLIDER_Contrast;
    static const long ID_BITMAPBUTTON11;
    static const long ID_BITMAPBUTTON13;
    static const long ID_BITMAPBUTTON12;
    static const long ID_PANEL31;
    static const long ID_STATICTEXT4;
    static const long ID_BUTTON_PLAY_RGB_SEQ;
    static const long ID_BUTTON2;
    static const long ID_BUTTON_SeqExport;
    static const long ID_BUTTON4;
    static const long ID_BUTTON_CREATE_RANDOM;
    static const long ID_BITMAPBUTTON7;
    static const long ID_BITMAPBUTTON9;
    static const long ID_BITMAPBUTTON3;
    static const long ID_BITMAPBUTTON4;
    static const long ID_BITMAPBUTTON_GRID_CUT;
    static const long ID_BITMAPBUTTON_GRID_COPY;
    static const long ID_BITMAPBUTTON_GRID_PASTE;
    static const long ID_BUTTON1;
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
    static const long ID_STATICTEXT26;
    static const long ID_STATICTEXT28;
    static const long ID_BUTTON5;
    static const long ID_TEXTCTRL1;
    static const long ID_BUTTON_PgoStitch;
    static const long ID_STATICTEXT30;
    static const long ID_BUTTON22;
    static const long ID_TEXTCTRL67;
    static const long ID_STATICTEXT71;
    static const long ID_STATICTEXT27;
    static const long ID_STATICTEXT64;
    static const long ID_STATICTEXT31;
    static const long ID_RADIOBUTTON_PgoFaceRound;
    static const long ID_RADIOBUTTON_PgoFaceRect;
    static const long ID_STATICTEXT32;
    static const long ID_CHECKBOX_PgoFaceOutline;
    static const long ID_PGO_AUTOFACE;
    static const long ID_STATICTEXT46;
    static const long ID_STATICTEXT38;
    static const long ID_CHOICE_PgoGroupName;
    static const long ID_BITMAPBUTTON_SaveCoroGroup;
    static const long ID_BUTTON_CoroGroupDelete;
    static const long ID_BUTTON_CoroGroupClear;
    static const long ID_GRID_COROFACES;
    static const long ID_CHOICE_PgoModelVoiceEdit;
    static const long ID_CHOICE_RelativeNodes;
    static const long ID_CHECKLISTBOX_CheckListBox_RelativeNodes;
    static const long ID_LISTBOX_RelativeNodes;
    static const long ID_CHECKBOX_AutoFadePgoElement;
    static const long ID_TEXTCTRL_AutoFadePgoElement;
    static const long ID_STATICTEXT34;
    static const long ID_CHECKBOX_CoroEyesRandomBlink;
    static const long ID_CHECKBOX_AutoFadePgoAll;
    static const long ID_TEXTCTRL_AutoFadePgoAll;
    static const long ID_STATICTEXT35;
    static const long ID_CHECKBOX_CoroEyesRandomLR;
    static const long ID_PGO_COROFACES;
    static const long ID_STATICTEXT66;
    static const long ID_STATICTEXT33;
    static const long ID_BUTTON7;
    static const long ID_TEXTCTRL3;
    static const long ID_BUTTON8;
    static const long ID_TEXTCTRL4;
    static const long ID_BUTTON9;
    static const long ID_TEXTCTRL8;
    static const long ID_BUTTON_AI_image;
    static const long ID_TEXTCTRL_AI_image;
    static const long ID_BUTTON_E_image;
    static const long ID_TEXTCTRL_E_image;
    static const long ID_BUTTON_etc_image;
    static const long ID_TEXTCTRL_etc_image;
    static const long ID_BUTTON_FV_image;
    static const long ID_TEXTCTRL_FV_image;
    static const long ID_BUTTON_L_image;
    static const long ID_TEXTCTRL_L_image;
    static const long ID_BUTTON_MBP_image;
    static const long ID_TEXTCTRL_MBP_image;
    static const long ID_BUTTON_O_image;
    static const long ID_TEXTCTRL_O_image;
    static const long ID_BUTTON_rest_image;
    static const long ID_TEXTCTRL_rest_image;
    static const long ID_BUTTON18;
    static const long ID_TEXTCTRL_U_image;
    static const long ID_BUTTON_WQ_image;
    static const long ID_TEXTCTRL_WQ_image;
    static const long ID_BITMAPBUTTON8;
    static const long ID_BITMAPBUTTON10;
    static const long ID_PGO_IMAGES;
    static const long ID_STATICTEXT65;
    static const long ID_BUTTON_background_image;
    static const long ID_TEXTCTRL_background_image;
    static const long ID_BUTTON_eyesopen_image;
    static const long ID_TEXTCTRL_eyesopen_image;
    static const long ID_BUTTON_eyesclosed_image;
    static const long ID_TEXTCTRL_eyesclosed_image;
    static const long ID_PGO_MOVIE;
    static const long ID_NOTEBOOK_PGO_PARMS;
    static const long ID_STATICTEXT70;
    static const long ID_BUTTON6;
    static const long ID_SPLITTERWINDOW3;
    static const long ID_PANEL4;
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
    static const long ID_TIMER2;
    //*)

    static const long ID_PLAYER_DIALOG;
    static const long ID_DELETE_EFFECT;
    static const long ID_IGNORE_CLICK;
    static const long ID_PROTECT_EFFECT;
    static const long ID_UNPROTECT_EFFECT;
    static const long ID_RANDOM_EFFECT;
    static const long ID_COPYROW_EFFECT; //copy random effect across row -DJ
    static const long ID_CLEARROW_EFFECT; //clear all effects on this row -DJ

    //(*Declarations(xLightsFrame)
    wxRadioButton* RadioButtonRgbChase;
    wxSlider* Slider_EffectLayerMix;
    wxSlider* SliderFgColorC;
    wxRadioButton* RadioButtonRgbCycleMixed;
    wxBitmapButton* BitmapButton_normal;
    wxButton* Button_Presets;
    wxCheckBox* CheckBox_AutoFadePgoAll;
    wxBitmapButton* BitmapButton4;
    wxChoice* Choice_Models;
    wxButton* Button_papagayo_output_sequence;
    wxTextCtrl* TextCtrl_background_image;
    wxButton* ButtonClearLog;
    EffectsPanel* EffectsPanel1;
    wxStaticBoxSizer* StaticBoxSizerSequenceButtons;
    wxCheckBox* CheckBox_PgoFaceOutline;
    wxStaticText* StaticText68;
    wxSlider* SliderBgIntensity;
    wxButton* Button_CoroGroupDelete;
    wxPanel* SeqPanelLeft;
    wxStaticText* StaticText32;
    wxStaticText* StaticText36;
    wxButton* Button_PlayRgbSeq;
    wxPanel* Panel2;
    wxTextCtrl* TextCtrl_L_image;
    wxButton* ButtonTestSelectAll;
    wxButton* ButtonSaveSchedule;
    wxRadioButton* RadioButtonRgbTwinkle50;
    wxStaticText* StaticText27;
    wxButton* ButtonAddE131;
    wxTextCtrl* TextCtrlFilename;
    wxBitmapButton* BitmapButtonDeleteRow;
    wxFileDialog* FileDialogConvert;
    wxTimer Timer1;
    wxCheckBox* CheckBoxLightOutput;
    wxPanel* Panel1;
    wxRadioButton* RadioButtonTwinkle50;
    wxRadioButton* RadioButtonRgbTwinkle10;
    wxPanel* PanelPapagayo;
    wxPanel* PGO_AUTOFACE;
    wxStaticText* StaticTextDirName;
    wxRadioButton* RadioButtonChase3;
    wxRadioButton* RadioButtonChase4;
    wxTimer Timer2;
    wxTextCtrl* TextCtrl_U_image;
    wxButton* Button_pgo_filename;
    wxButton* Button2;
    wxRadioButton* RadioButton_PgoFaceRect;
    wxButton* Button_PlayEffect;
    wxRadioButton* RadioButtonRgbChase4;
    wxButton* ButtonModelsPreview;
    wxButton* ButtonPreviewOpen;
    wxPanel* PanelTest;
    wxButton* ButtonStopPreview;
    wxChoice* Choice_PgoModelVoiceEdit;
    wxButton* ButtonModelExport;
    wxTextCtrl* TextCtrl_MBP_image;
    wxButton* Button_FV_image;
    wxMenuItem* MenuItemBackup;
    wxBitmapButton* BitmapButton_Brightness;
    wxButton* Button_Palette;
    wxButton* ButtonChooseFile;
    wxTextCtrl* TextCtrl_AI_image;
    wxRadioButton* RadioButtonRgbDim;
    wxStaticText* StaticText5;
    wxStaticText* StaticText25;
    wxBitmapButton* BitmapButton_locked;
    wxPanel* PanelPreview;
    wxStaticText* StaticText6;
    wxButton* ButtonTestClear;
    wxBitmapButton* BitmapButtonGridCopy;
    wxButton* Button_background_image;
    wxButton* ButtonStopNow;
    wxStaticText* StaticText126;
    wxPanel* PanelConvert;
    wxTextCtrl* TextCtrl3;
    wxNotebook* Notebook1;
    wxStaticText* StaticText127;
    wxButton* ButtonSaveSetup;
    wxButton* ButtonStartConversion;
    wxBitmapButton* BitmapButtonInsertRow;
    wxStaticText* StaticText19;
    wxButton* ButtonDeleteShow;
    wxButton* Button_CoroGroupClear;
    wxStaticText* StaticTextSequenceFileName;
    wxBitmapButton* BitmapButton_SparkleFrequency;
    wxRadioButton* RadioButtonRgbShimmer;
    wxButton* ButtonDisplayElements;
    wxListBox* ListBoxElementList;
    wxStaticText* StaticText10;
    wxTextCtrl* txtCtlBrightness;
    wxTextCtrl* TextCtrlPreviewElementSize;
    wxButton* Button_eyesopen_image;
    wxTextCtrl* TextCtrl_WQ_image;
    wxChoice* ChoiceOutputFormat;
    wxDirDialog* DirDialog1;
    wxStaticText* StaticTextSetup1;
    wxButton* Button_CreateRandom;
    wxCheckListBox* CheckListBoxTestChannels;
    wxTextCtrl* TextCtrl_E_image;
    wxStaticText* StaticText33;
    wxStaticText* StaticText23;
    wxRadioButton* RadioButtonTwinkle10;
    wxStaticText* StaticText12;
    wxStaticText* StaticText22;
    wxSlider* SliderPreviewRotate;
    wxButton* Button_etc_image;
    wxTreeCtrl* ListBoxSched;
    wxNotebook* NotebookTest;
    wxRadioButton* RadioButtonTwinkle25;
    wxTextCtrl* txtCtlEffectMix;
    wxButton* Button_ChannelMap;
    wxButton* Button_L_image;
    wxButton* Button_AI_image;
    wxBitmapButton* BitmapButtonSaveSeq;
    wxSlider* SliderChaseSpeed;
    wxPanel* PGO_IMAGES;
    wxCheckBox* CheckBoxMapEmptyChannels;
    wxNotebook* NotebookPgoParms;
    wxCheckBox* CheckBox_AutoFadePgoElement;
    wxPanel* PaneNutcracker;
    wxButton* ButtonShowDatesChange;
    wxButton* ButtonAddShow;
    wxScrolledWindow* ScrolledWindow1;
    wxButton* Button_UpdateGrid;
    wxRadioButton* RadioButtonDim;
    wxButton* ButtonUpdateShow;
    wxStaticText* StaticText61;
    wxBitmapButton* BitmapButtonGridPaste;
    wxStaticText* StaticText7;
    wxMenu* MenuFile;
    wxTextCtrl* TextCtrl_rest_image;
    wxTextCtrl* TextCtrl_O_image;
    wxStaticText* StaticText16;
    wxSlider* Slider_SparkleFrequency;
    wxButton* ButtonChangeDir;
    wxGrid* Grid1;
    wxButton* ButtonSaveLog;
    wxPanel* PanelSetup;
    wxBitmapButton* BitmapButtonGridCut;
    wxStaticText* StaticText43;
    wxCheckListBox* xCheckListBox_RelativeNodes;
    wxStaticText* StaticText24;
    wxSlider* SliderFgColorB;
    wxStaticText* StaticText65;
    wxRadioButton* RadioButtonShimmer;
    wxMenuItem* MenuItemSavePlaylists;
    wxTextCtrl* txtCtrlSparkleFreq;
    wxStaticText* StaticText14;
    wxSlider* SliderBgColorC;
    wxStatusBar* StatusBar1;
    wxButton* ButtonPlayPreview;
    wxPanel* Panel3;
    wxStaticText* StaticText18;
    wxListCtrl* GridNetwork;
    wxBitmapButton* BitmapButton_random;
    wxSlider* SliderRgbChaseSpeed;
    wxButton* ButtonSavePreview;
    wxStaticText* StaticText13;
    wxStaticText* StaticTextPreviewRotation;
    wxTextCtrl* TextCtrl_eyesopen_image;
    wxSplitterWindow* SplitterWindow1;
    wxSlider* SliderFgIntensity;
    wxButton* ButtonLightsOff;
    wxPanel* PanelTestStandard;
    wxPanel* SeqPanelRight;
    wxStaticText* StaticText20;
    wxButton* ButtonStartPapagayo;
    wxTextCtrl* TextCtrl_eyesclosed_image;
    wxButton* ButtonSeqExport;
    wxTextCtrl* TextCtrl_pgo_filename;
    wxCheckBox* CheckBox_CoroEyesRandomBlink;
    wxRadioButton* RadioButtonRgbCycle4;
    wxStaticText* StaticText31;
    wxBitmapButton* BitmapButton_Contrast;
    wxChoice* Choice_PgoGroupName;
    wxRadioButton* RadioButtonRgbTwinkle05;
    wxButton* Button_eyesclosed_image;
    wxPanel* PGO_COROFACES;
    wxSlider* Slider_Contrast;
    wxBitmapButton* BitmapButton_SaveCoroGroup;
    wxStaticText* StaticText67;
    wxButton* ButtonTestSave;
    wxButton* Button_U_image;
    wxChoice* Choice_RelativeNodes;
    wxButton* Button_PgoStitch;
    wxButton* Button_E_image;
    wxBitmapButton* BitmapButton3;
    wxStaticText* StaticTextShowEnd;
    wxFileDialog* FileDialogPgoImage;
    wxButton* Button_rest_image;
    wxTextCtrl* TextCtrl_papagayo_output_filename;
    wxStaticText* StaticText62;
    wxStaticText* StaticText4;
    wxRadioButton* RadioButtonAlt;
    wxChoice* Choice_LayerMethod;
    wxTextCtrl* TextCtrl2;
    wxRadioButton* RadioButton_PgoFaceRound;
    wxTextCtrl* txtCtlContrast;
    wxPanel* PanelRgbCycle;
    wxRadioButton* RadioButtonRgbChase3;
    wxCheckBox* CheckBoxRunSchedule;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxButton* Button_Models;
    wxCheckBox* CheckBox_LayerMorph;
    wxButton* Button3;
    wxRadioButton* RadioButtonChase5;
    wxRadioButton* RadioButtonRgbCycleOff;
    wxPanel* PanelCal;
    wxBitmapButton* BitmapButtonOpenSeq;
    wxStaticText* StaticText26;
    wxStaticText* StaticText15;
    wxStaticText* StaticText8;
    wxBitmapButton* BitmapButton_EffectLayerMix;
    wxMenuItem* MenuItemRefresh;
    wxStaticText* StaticText30;
    wxStaticText* StaticText28;
    wxRadioButton* RadioButtonRgbAlt;
    wxStaticText* StaticText29;
    wxSlider* SliderFgColorA;
    wxTextCtrl* TextCtrl_FV_image;
    wxCheckBox* CheckBoxOffAtEnd;
    wxSlider* SliderRgbCycleSpeed;
    wxButton* Button1;
    wxButton* ButtonNetworkDelete;
    wxTextCtrl* TextCtrl_etc_image;
    wxStaticText* StaticText63;
    wxTextCtrl* TextCtrl4;
    wxSlider* SliderPreviewScale;
    wxButton* ButtonTestLoad;
    wxListBox* xListBox_RelativeNodes;
    wxRadioButton* RadioButtonOff;
    wxRadioButton* RadioButtonRgbTwinkle25;
    wxRadioButton* RadioButtonRgbChase5;
    wxTextCtrl* TextCtrlLog;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxTextCtrl* TextCtrl_AutoFadePgoAll;
    wxTextCtrl* TextCtrlConversionStatus;
    wxSlider* SliderBgColorB;
    wxPanel* PanelTestRgb;
    wxButton* ButtonAddDongle;
    wxSplitterWindow* SplitterWindow3;
    wxRadioButton* RadioButtonChase;
    wxStaticText* StaticText17;
    wxBitmapButton* BitmapButtonTabInfo;
    wxBitmapButton* BitmapButton_CheckBox_LayerMorph;
    wxStaticText* StaticText11;
    wxPanel* PGO_MOVIE;
    wxScrolledWindow* ScrolledWindowPreview;
    wxButton* Button_MBP_image;
    wxButton* Button_O_image;
    wxRadioButton* RadioButtonRgbCycle5;
    wxGrid* GridCoroFaces;
    wxStaticText* StaticTextShowStart;
    wxButton* ButtonGracefulStop;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxStaticText* StaticTextPreviewFileName;
    wxSlider* SliderPreviewTime;
    wxStaticText* StaticText9;
    wxMessageDialog* MessageDialog1;
    wxSlider* Slider_Brightness;
    wxButton* ButtonNetworkDeleteAll;
    wxRadioButton* RadioButtonRgbChaseOff;
    wxButton* ButtonNetworkChange;
    wxCheckBox* CheckBox_CoroEyesRandomLR;
    wxTextCtrl* TextCtrlPreviewTime;
    wxTextCtrl* TextCtrl_AutoFadePgoElement;
    wxRadioButton* RadioButtonRgbCycle3;
    wxButton* Button_WQ_image;
    EffectsPanel* EffectsPanel2;
    wxSlider* SliderBgColorA;
    wxRadioButton* RadioButtonTwinkle05;
    wxStaticText* StaticText3;
    wxSplitterWindow* SplitterWindow2;
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

    wxString mediaFilename;
//    wxString xlightsFilename;
    SeqDataType SeqData;
    long SeqDataLen;
    long SeqNumPeriods;
    long SeqNumChannels;
    wxArrayString FileNames;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;

    // convert
    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString &VixChannelNames);
    void ReadVixFile(const char* filename);
    void ReadLorFile(const char* filename);
    void ReadHLSFile(const wxString& filename);
    void ReadXlightsFile(const wxString& FileName);
    void ReadFalconFile(const wxString& FileName);
    void ReadGlediatorFile(const wxString& FileName);
    void ReadConductorFile(const wxString& FileName);
    void SetMediaFilename(const wxString& filename);
    int GetLorTrack1Length(const char* filename);
    bool WriteVixenFile(const wxString& filename); //     Vixen *.vix
    void WriteVirFile(const wxString& filename);
    void WriteVirFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf); //       Vixen *.vir
    void WriteHLSFile(const wxString& filename);  //      HLS *.hlsnc
    void WriteHLSFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      HLS *.hlsnc
    void WriteXLightsFile(const wxString& filename); //   xLights *.xseq
    void WriteFalconPiFile(const wxString& filename); //  Falcon Pi Player *.pseq
    void WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
                                SeqDataType *dataBuf, int startAddr, int modelSize); //Falcon Pi sub sequence .eseq
    void WriteConductorFile(const wxString& filename); // Conductor *.seq
    void WriteLSPFile(const wxString& filename);  //      LSP UserPatterns.xml
    void WriteLSPFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf,int cpn);  //      LSP UserPatterns.xml
    void WriteLorFile(const wxString& filename);  //      LOR *.lms, *.las
    void WriteLcbFile(const wxString& filename);  //      LOR *.lcb
    void WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      LOR *.lcb
    void ClearLastPeriod();
    void ConversionInit();
    void DoConversion(const wxString& FileName, const wxString& OutputFormat);
    void ConversionError(const wxString& msg);

    wxString base64_encode();
    std::string base64_decode(const wxString& encoded_string);

    SeqDataType* RenderModelToData(wxXmlNode *modelNode);
    wxXmlNode* SelectModelToExport();


//  papagayo
    void PapagayoError(const wxString& msg);
    //void AutoFace();
    int write_pgo_header(wxFile& f, int MaxVoices);
    void write_pgo_footer(wxFile& f, int MaxVoices);
//    void AutoFace(wxFile& f, int MaxVoices,int start_frame,int end_frame,const wxString& phoneme, const wxString& word);
    void AutoFace(wxFile& f, int start_frame, void* voice_ptr, void* phrase_ptr, void* word_ptr, void* phoneme_ptr);


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
    void UpdateGrid();

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
    void UpdateRgbPlaybackStatus(int seconds, long msec, int EffectPeriod, const wxString& seqtype);
    void SetTextColor(wxWindow* w);
    void LoadSettingsMap(wxString settings, MapStringString& SettingsMap);
    void UpdateBufferFadesFromCtrl();
    void UpdateEffectDuration();
    void ResetEffectDuration();
    void UpdateBufferPalette(EffectsPanel* panel, int layer);
    void UpdateBufferPaletteFromMap(int PaletteNum, MapStringString& SettingsMap);
    bool RenderEffectFromMap(int layer, int period, MapStringString& SettingsMap);
    void UpdateBufferFadesFromMap(int effectNum, MapStringString& SettingsMap);
    void UpdateFitToTimeFromMap(int effectNum, MapStringString& SettingsMap);
    void ClearEffectWindow();
    void EnableSequenceControls(bool enable);
    void ResetEffectStates();
    bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
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
    void OpenSequence();
    void SaveSequence();
    void InsertRow();
    void UpdatePreview();
    void ShowModelsDialog();
    void ShowPreviewTime(long ElapsedMSec);
    void PreviewOutput(int period);
    void TimerOutput(int period);
    void ResetSequenceGrid();
    void CompareMyDisplayToSeq();
    void GetSeqModelNames(wxArrayString& a);
    void UpdateChannelNames();
    void StopNow(void);
    void PlayRgbSequence(void);
    void PlayEffect(void);
    bool HotKey(wxKeyEvent& event);
    void CutOrCopyToClipboard(bool IsCut);
    void PasteFromClipboard(void);
    bool IsValidEffectString(wxString& s);
    void PreviewScaleUpdated(int newscale);
    void LoadPapagayoFile(const wxString& filename, int frame_offset = 0);
    void InitPapagayoTab(bool tab_changed);
    bool LoadPgoSettings(void);
    bool SavePgoSettings(void);
    bool GetGroupName(wxString& grpname);
    void PgoGridCellSelect(int row, int col, int where);
    void GetMouthNodes(const wxString& model_name);

    wxXmlDocument pgoXml; //Papagayo settings from xlights_papagayo.xml
    bool Grid1HasFocus; //cut/copy/paste handled differently with grid vs. other text controls -DJ
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
//    std::vector<ModelClassPtr> PreviewModels;
    wxHtmlEasyPrinting* HtmlEasyPrint;
    int NextGridRowToPlay;
    int SeqPlayColumn;
    bool ResetEffectState[2];

    wxArrayString BarEffectDirections;
    wxArrayString ButterflyEffectColors;
    wxArrayString ButterflyDirection;
    wxArrayString FacesPhoneme;
    wxArrayString CoroFacesPhoneme;

    wxArrayString MeteorsEffectTypes;
    wxArrayString MeteorsEffect;
    wxArrayString TextEffectDirections;
    wxArrayString PictureEffectDirections;
    wxArrayString PianoEffectStyles;
    wxArrayString PianoKeyPlacement;
    wxArrayString SingleStrandTypes;
    wxArrayString SingleStrandColors;
    wxArrayString TextEffects;
    wxArrayString TextCountDown;
    wxArrayString WaveType;
    wxArrayString FillColors;
    wxArrayString WaveDirection;


    wxArrayString CurtainEdge;
    wxArrayString CurtainEffect;
    wxGridCellCoords *curCell;
    EffectTreeDialog *EffectTreeDlg;

    bool m_dragging;
    int m_previous_mouse_x, m_previous_mouse_y;
    std::string LastIntensity;
    std::set<int> LorTimingList; // contains a list of period numbers, set by ReadLorFile()

//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
//#include <unordered_map>
    typedef enum { Normal, Locked, Random } EditState;
    std::unordered_map<std::string, EditState> buttonState;
    bool isRandom_(wxControl* ctl, const char*debug);
#define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)
    void setlock(wxBitmapButton* button); //, EditState& islocked);

    DECLARE_EVENT_TABLE()
    friend class xLightsApp; //kludge: allow xLightsApp to call OnPaneNutcrackerChar -DJ
public:
    static std::vector<ModelClassPtr> PreviewModels, OtherModels; //make public and static for easier access -DJ
    static wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false);
};

#endif // XLIGHTSMAIN_H
