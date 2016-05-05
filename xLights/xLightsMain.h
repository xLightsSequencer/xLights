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
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/checkbox.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
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
#include "EffectListDialog.h"
#include "SeqExportDialog.h"
#include "ViewsDialog.h"
#include "SeqElementMismatchDialog.h"
#include "PixelBuffer.h"
#include "NetInfo.h"
#include "PaletteMgmtDialog.h"
#include "ExportModelSelect.h"
#include "ViewsDialog.h"
#include "CurrentPreviewModels.h"
#include "PreviewModels.h"
#include "ModelPreview.h"
#include "EffectAssist.h"
#include "SequenceData.h"
#include "UtilClasses.h"
#include "PhonemeDictionary.h"

#include "sequencer/EffectsGrid.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/RowHeading.h"
#include "sequencer/TimeLine.h"
#include "sequencer/Waveform.h"
#include "TopEffectsPanel.h"
#include "TimingPanel.h"
#include "ColorPanel.h"
#include "PerspectivesPanel.h"
#include "EffectIconPanel.h"
#include "DisplayElementsPanel.h"
#include "effects/EffectManager.h"
#include "models/ModelManager.h"

#include "wx/aui/aui.h"


#include "JobPool.h"

#include <log4cpp/Category.hh>

class EffectTreeDialog;
class ConvertDialog;
class ConvertLogDialog;

// max number of most recently used show directories on the File menu
#define MRU_LENGTH 4

// notebook pages
#define SETUPTAB 0
#define PREVIEWTAB 1
#define SCHEDULETAB 2
#define PAPAGAYOTAB 3
#define NEWSEQUENCER 4

#define FixedPages 5

#define TEXT_ENTRY_TIMING           0
#define TEXT_ENTRY_EFFECT           1
#define TEXT_ENTRY_DIALOG           2

class RenderCommandEvent;

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_MOUSE_POSITION, wxCommandEvent);
wxDECLARE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_UNSELECTED_EFFECT, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);
wxDECLARE_EVENT(EVT_UPDATE_EFFECT, wxCommandEvent);
wxDECLARE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(EVT_LOAD_PERSPECTIVE, wxCommandEvent);
wxDECLARE_EVENT(EVT_SAVE_PERSPECTIVES, wxCommandEvent);
wxDECLARE_EVENT(EVT_PERSPECTIVES_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EXPORT_MODEL, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_MODEL, wxCommandEvent);
wxDECLARE_EVENT(EVT_COPY_MODEL_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PASTE_MODEL_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_MODEL_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAY_SEQUENCE, wxCommandEvent);
wxDECLARE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);
wxDECLARE_EVENT(EVT_PAUSE_SEQUENCE, wxCommandEvent);
wxDECLARE_EVENT(EVT_STOP_SEQUENCE, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_FIRST_FRAME, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_LAST_FRAME, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_REPLAY_SECTION, wxCommandEvent);
wxDECLARE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDECLARE_EVENT(EVT_CONVERT_DATA_TO_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PROMOTE_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_RGBEFFECTS_CHANGED, wxCommandEvent);

static const wxString xlights_base_name       = "xLights";
static const wxString xlights_version_string  = "2016.24";
static const wxString xlights_build_date      = "May 04, 2016";

static const wxString strSupportedFileTypes = "LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|Vixen Sequences (*.vix)|*.vix|Glediator Record File (*.gled)|*.gled)|Lynx Conductor Sequences (*.seq)|*.seq|xLights Sequences(*.xseq)|*.xseq|xLights Imports(*.iseq)|*.iseq|Falcon Pi Player Sequences (*.fseq)|*.fseq";
static const wxString strSequenceSaveAsFileTypes = "xLights Sequences(*.xml)|*.xml";
static wxCriticalSection gs_xoutCriticalSection;



typedef SequenceData SeqDataType;

enum play_modes
{
    play_off,
    play_single,
    play_list,
    play_sched
};

//enum TestFunctions
//{
//    OFF,
//    CHASE,
//    CHASE2,
//    CHASE3,
//    CHASE4,
//    DIM,
//    TWINKLE,
//    SHIMMER
//};

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

class RenderEvent;
class wxDebugReportCompress;
class BufferPanel;
class LayoutPanel;

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

#ifdef __WXOSX__
#define DOUBLE_BUFFER(a)
#else
#define DOUBLE_BUFFER(a) a->SetDoubleBuffered(true);
#endif

class wxAuiToolBar;
class AUIToolbarButtonWrapper {
public:
    AUIToolbarButtonWrapper(wxAuiToolBar *tb, int i) : toolbar(tb), id(i) {}

    bool IsChecked();
    void SetValue(bool b);
    void Enable(bool b);
    void SetBitmap(const wxBitmap &bmp);
private:
    wxAuiToolBar *toolbar;
    int id;
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

    PhonemeDictionary dictionary;

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
    void SetEffectControls(const std::string &modelName, const std::string &name,
                           const SettingsMap &settings, const SettingsMap &palette,
                           bool setDefaults);
    void SetEffectControls(const SettingsMap &settings);
	void ApplySetting(wxString name, wxString value);
	bool SaveEffectsFile();
    void MarkEffectsFileDirty() { UnsavedRgbEffectsChanges=true; }
    void CheckUnsavedChanges();
    void SetStatusText(const wxString &msg);
	std::string GetChannelToControllerMapping(long channel);

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

    enum EFFECT_ASSIST_MODES
    {
        EFFECT_ASSIST_ALWAYS_ON,
        EFFECT_ASSIST_ALWAYS_OFF,
        EFFECT_ASSIST_TOGGLE_MODE,
        EFFECT_ASSIST_NOT_IN_PERSPECTIVE
    };

    static wxString CurrentDir; //expose current folder name -DJ
    static wxString PlaybackMarker; //keep track of where we are within grid -DJ
    static wxString xlightsFilename; //expose current path name -DJ
    static xLightsXmlFile* CurrentSeqXmlFile; // global object for currently opened XML file
    wxString GetShowDirectory() { return showDirectory; }
    NetInfoClass& GetNetInfo() { return NetInfo; }
    void ConversionInit();
    void ConversionError(const wxString& msg);
    void PlayerError(const wxString& msg);
    void SetMediaFilename(const wxString& filename);
    void RenderIseqData(bool bottom_layers, ConvertLogDialog* plog);
    void ClearSequenceData();
    void LoadAudioData(xLightsXmlFile& xml_file);
    void CreateDebugReport(wxDebugReportCompress *report);
    wxString GetThreadStatusReport();
	void SetAudioControls();
    void ImportXLights(const wxFileName &filename);
    void ImportXLights(SequenceElements &se, const std::vector<Element *> &elements,
        bool allowAllModels = false, bool clearSrc = false);
    void ImportVix(const wxFileName &filename);
    void ImportHLS(const wxFileName &filename);
    void ImportLMS(const wxFileName &filename);
    void ImportLSP(const wxFileName &filename);
    void ImportSuperStar(const wxFileName &filename);

    EffectManager &GetEffectManager() { return effectManager; }

private:
    bool ImportSuperStar(Element *el, wxXmlDocument &doc, int x_size, int y_size,
                         int x_offset, int y_offset, bool flip_y,
                         int imageResizeType, const wxSize &modelSize);
    bool ImportLMS(wxXmlDocument &doc);

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
    //void OnButtonStartConversionClick(wxCommandEvent& event);
    //void OnButtonChooseFileClick(wxCommandEvent& event);
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
    void OnButtonSetBackgroundClick(wxCommandEvent& event);
    void OnButtonClearBackgroundClick(wxCommandEvent& event);
    void OnListBoxElementListSelect(wxCommandEvent& event);
    void OnScrolledWindow1Resize(wxSizeEvent& event);
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
    void OnChoice_PgoOutputTypeSelect(wxCommandEvent& event);
    void OnButton_PgoCopyVoicesClick(wxCommandEvent& event);
    void OnTextCtrl_PgoMinRestText(wxCommandEvent& event);
    void OnTextCtrl_PgoMaxRestText(wxCommandEvent& event);
    void OnTextCtrl_PgoAutoFadeText(wxCommandEvent& event);
    void OnbtEditViewsClick(wxCommandEvent& event);
    void OnChoice_ViewsSelect(wxCommandEvent& event);
    void OnButtonBuildCustomModelClick(wxCommandEvent& event);
    void OnGrid1LabelRightClick(wxGridEvent& event);
    void OnGridCoroFacesLabelLeftClick(wxGridEvent& event);
    void OnPanelSequencerPaint(wxPaintEvent& event);
    void OnBitmapButton1Click(wxCommandEvent& event);
    void OnBitmapButton3Click(wxCommandEvent& event);
    void OnMainToolBarDropdown(wxAuiToolBarEvent& event);
    void OnButtonNewSequenceClick(wxCommandEvent& event);
    void OnButtonClickSaveAs(wxCommandEvent& event);
    void OnNotebook1PageChanged1(wxAuiNotebookEvent& event);
    void OnMenuXmlConversionSettings(wxCommandEvent& event);
    void ChangeMediaDirectory(wxCommandEvent& event);
    void OnAuiToolBarItemPlayButtonClick(wxCommandEvent& event);
    void OnAuiToolBarItemPauseButtonClick(wxCommandEvent& event);
    void OnAuiToolBarItemStopClick(wxCommandEvent& event);
    void OnAuiToolBarItemZoominClick(wxCommandEvent& event);
    void OnAuiToolBarItem_ZoomOutClick(wxCommandEvent& event);
    void OnMenuItemLoadEditPerspectiveSelected(wxCommandEvent& event);
    void OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event);
    void OnMenu_Settings_SequenceSelected(wxCommandEvent& event);
    void OnMenuItem_File_Open_SequenceSelected(wxCommandEvent& event);
    void OnMenuItem_File_Save_SequenceSelected(wxCommandEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnAuiToolBarItemRenderAllClick(wxCommandEvent& event);
    void OnMenuItem_File_Close_SequenceSelected(wxCommandEvent& event);
    void OnAuiToolBarFirstFrameClick(wxCommandEvent& event);
    void OnAuiToolBarLastFrameClick(wxCommandEvent& event);
    void OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event);
    void ShowHideEffectSettingsWindow(wxCommandEvent& event);
    void ShowHideColorWindow(wxCommandEvent& event);
    void ShowHideLayerTimingWindow(wxCommandEvent& event);
    void ShowHideModelPreview(wxCommandEvent& event);
    void ShowHideEffectDropper(wxCommandEvent& event);
    void ShowHideHousePreview(wxCommandEvent& event);
    void SetIconSize(wxCommandEvent& event);
    void ResetToolbarLocations(wxCommandEvent& event);
    void SetToolIconSize(wxCommandEvent& event);
    void OnMenuItemRenderEraseModeSelected(wxCommandEvent& event);
    void OnMenuItemRenderCanvasModeSelected(wxCommandEvent& event);
    void OnSetGridIconBackground(wxCommandEvent& event);
    void OnSetGridNodeValues(wxCommandEvent& event);
    void OnMenuItemImportEffects(wxCommandEvent& event);
    void SetPlaySpeed(wxCommandEvent& event);
    void OnBitmapButton_Link_DirsClick(wxCommandEvent& event);
    void OnMenuItemRenderOnSave(wxCommandEvent& event);
    void OnNotebook1PageChanging(wxAuiNotebookEvent& event);
    void OnButtonAddNullClick(wxCommandEvent& event);
    void OnMenuItemEffectAssistAlwaysOnSelected(wxCommandEvent& event);
    void OnMenuItemEffectAssistAlwaysOffSelected(wxCommandEvent& event);
    void OnMenuItemEffectAssistToggleModeSelected(wxCommandEvent& event);
    void ShowHidePerspectivesWindow(wxCommandEvent& event);
    void ShowHideDisplayElementsWindow(wxCommandEvent& event);
    void ShowHideEffectAssistWindow(wxCommandEvent& event);
    void OnMenuItem_File_SaveAs_SequenceSelected(wxCommandEvent& event);
    void OnGridNetworkItemActivated(wxListEvent& event);
    void OnMenuDockAllSelected(wxCommandEvent& event);
    void ShowHideBufferSettingsWindow(wxCommandEvent& event);
    void ResetWindowsToDefaultPositions(wxCommandEvent& event);
    void OnActionTestMenuItemSelected(wxCommandEvent& event);
    void OnAuiToolBarShowHideEffectSettings(wxCommandEvent& event);
    void OnAuiToolBarItemShowHideEffects(wxCommandEvent& event);
    void OnAuiToolBarItemPasteByTimeClick(wxCommandEvent& event);
    void OnAuiToolBarItemPasteByCellClick(wxCommandEvent& event);
    void OnMenuItemConvertSelected(wxCommandEvent& event);
    void OnMenu_GenerateCustomModelSelected(wxCommandEvent& event);
    void OnPaneClose(wxAuiManagerEvent& event);
    void OnMenuItemPackageDebugFiles(wxCommandEvent& event);
    void OnMenuOpenGLSelected(wxCommandEvent& event);
    //*)

    void DoMenuAction(wxMenuEvent &evt);
    void OnPopupClick(wxCommandEvent &evt);
    void DeleteSelectedEffects(wxCommandEvent &evt);
    void InsertRandomEffects(wxCommandEvent &evt);
    void UnprotectSelectedEffects(wxCommandEvent& evt);
    void ProtectSelectedEffects(wxCommandEvent& evt);
    void CopyEffectAcrossRow(wxCommandEvent& evt); //-DJ
    void ClearEffectRow(wxCommandEvent& evt); //-DJ
	void ShowHideAllSequencerWindows(bool show);
	void ResetAllSequencerWindows();
	void SetEffectAssistWindowState(bool show);
    void UpdateEffectAssistWindow(Effect* effect, RenderableEffect* ren_effect);

//    void ConnectOnChar(wxWindow* pclComponent);

    //(*Identifiers(xLightsFrame)
    static const long ID_AUITOOLBAR_OPENSHOW;
    static const long ID_AUITOOLBAR_NEWSEQUENCE;
    static const long ID_AUITOOLBAR_OPEN;
    static const long ID_AUITOOLBAR_SAVE;
    static const long ID_AUITOOLBAR_SAVEAS;
    static const long ID_AUITOOLBAR_RENDERALL;
    static const long ID_AUITOOLBAR_MAIN;
    static const long ID_AUITOOLBAR_PLAY_NOW;
    static const long ID_AUITOOLBAR_PAUSE;
    static const long ID_AUITOOLBAR_STOP;
    static const long ID_AUITOOLBAR_FIRST_FRAME;
    static const long ID_AUITOOLBAR_LAST_FRAME;
    static const long ID_AUITOOLBAR_REPLAY_SECTION;
    static const long ID_AUITOOLBAR_PLAY;
    static const long ID_AUITOOLBARITEM2;
    static const long ID_AUITOOLBARITEM5;
    static const long ID_AUITOOLBARITEM7;
    static const long ID_AUITOOLBARITEM3;
    static const long ID_AUITOOLBARITEM1;
    static const long ID_AUITOOLBARITEM4;
    static const long ID_AUITOOLBARITEM6;
    static const long ID_AUITOOLBARITEM8;
    static const long ID_AUIWINDOWTOOLBAR;
    static const long ID_PASTE_BY_TIME;
    static const long ID_PASTE_BY_CELL;
    static const long ID_AUITOOLBAR_EDIT;
    static const long ID_AUITOOLBARITEM14;
    static const long ID_AUITOOLBAR_VIEW;
    static const long ID_BITMAPBUTTON_TAB_INFO;
    static const long ID_BUTTON_STOP_NOW;
    static const long ID_BUTTON_GRACEFUL_STOP;
    static const long ID_BUTTON_LIGHTS_OFF;
    static const long ID_CHECKBOX_LIGHT_OUTPUT;
    static const long ID_AUITOOLBAR_OUTPUT;
    static const long ID_AUIEFFECTSTOOLBAR;
    static const long ID_BUTTON3;
    static const long ID_STATICTEXT4;
    static const long ID_BUTTON_CHANGE_MEDIA_DIR;
    static const long ID_ANY;
    static const long ID_BITMAPBUTTON_Link_Dirs;
    static const long ID_BUTTON_SAVE_SETUP;
    static const long ID_BUTTON_ADD_DONGLE;
    static const long ID_BUTTON_ADD_E131;
    static const long ID_BUTTON1;
    static const long ID_BUTTON_NETWORK_CHANGE;
    static const long ID_BUTTON_NETWORK_DELETE;
    static const long ID_BUTTON_NETWORK_DELETE_ALL;
    static const long ID_BITMAPBUTTON1;
    static const long ID_BITMAPBUTTON2;
    static const long ID_LISTCTRL_NETWORKS;
    static const long ID_PANEL_SETUP;
    static const long ID_PANEL_PREVIEW;
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
    static const long ID_SPLITTERWINDOW3;
    static const long ID_STATICTEXT28;
    static const long ID_BUTTON5;
    static const long ID_TEXTCTRL1;
    static const long ID_BUTTON_PgoStitch;
    static const long ID_STATICTEXT30;
    static const long ID_BUTTON22;
    static const long ID_TEXTCTRL67;
    static const long ID_STATICTEXT71;
    static const long ID_STATICTEXT36;
    static const long ID_CHOICE_PgoOutputType;
    static const long ID_STATICTEXT_PgoOutputType;
    static const long ID_STATICTEXT37;
    static const long ID_STATICTEXT46;
    static const long ID_STATICTEXT38;
    static const long ID_CHOICE_PgoGroupName;
    static const long ID_BITMAPBUTTON_SaveCoroGroup;
    static const long ID_BUTTON_CoroGroupDelete;
    static const long ID_BUTTON_CoroGroupClear;
    static const long ID_BUTTON_PgoCopyVoices;
    static const long ID_GRID_COROFACES;
    static const long ID_CHECKBOX_PgoAutoReset;
    static const long ID_TEXTCTRL_PgoMinRest;
    static const long ID_STATICTEXT27;
    static const long ID_TEXTCTRL_PgoMaxRest;
    static const long ID_STATICTEXT34;
    static const long ID_CHECKBOX_CoroEyesRandomBlink;
    static const long ID_CHECKBOX_PgoAutoFade;
    static const long ID_TEXTCTRL_PgoAutoFade;
    static const long ID_STATICTEXT35;
    static const long ID_CHECKBOX_CoroEyesRandomLR;
    static const long ID_CHECKBOX_CoroPictureScaled;
    static const long ID_STATICTEXT70;
    static const long ID_BUTTON6;
    static const long ID_PANEL4;
    static const long ID_PANEL7;
    static const long ID_NOTEBOOK1;
    static const long ID_NEW_SEQUENCE;
    static const long ID_OPEN_SEQUENCE;
    static const long IS_SAVE_SEQ;
    static const long ID_SAVE_AS_SEQUENCE;
    static const long ID_CLOSE_SEQ;
    static const long ID_MENUITEM2;
    static const long ID_FILE_BACKUP;
    static const long ID_MENUITEM13;
    static const long ID_MENUITEM_CONVERT;
    static const long ID_MENUITEM_GenerateCustomModel;
    static const long ID_MENUITEM18;
    static const long idMenuSaveSched;
    static const long idMenuAddList;
    static const long idMenuRenameList;
    static const long idMenuDelList;
    static const long ID_MENUITEM1;
    static const long idCustomScript;
    static const long ID_MENUITEM_SAVE_PERSPECTIVE;
    static const long ID_MENUITEM_LOAD_PERSPECTIVE;
    static const long ID_MENUITEM7;
    static const long ID_MENUITEM_DISPLAY_ELEMENTS;
    static const long ID_MENUITEM12;
    static const long ID_MENUITEM3;
    static const long ID_MENUITEM14;
    static const long ID_MENUITEM15;
    static const long ID_MENUITEM16;
    static const long ID_MENUITEM9;
    static const long ID_MENUITEM17;
    static const long ID_MENUITEM_EFFECT_ASSIST_WINDOW;
    static const long ID_MENUITEM_WINDOWS_PERSPECTIVE;
    static const long ID_MENUITEM_WINDOWS_DOCKALL;
    static const long ID_MENUITEM11;
    static const long ID_MENUITEM10;
    static const long ID_PLAY_FULL;
    static const long ID_PLAY_3_4;
    static const long ID_PLAY_1_2;
    static const long ID_PLAY_1_4;
    static const long ID_IMPORT_EFFECTS;
    static const long ID_SEQ_SETTINGS;
    static const long ID_RENDER_ON_SAVE;
    static const long ID_MENUITEM_ICON_SMALL;
    static const long ID_MENUITEM_ICON_MEDIUM;
    static const long ID_MENUITEM_ICON_LARGE;
    static const long ID_MENUITEM_ICON_XLARGE;
    static const long ID_MENUITEM4;
    static const long ID_MENUITEM_GRID_ICON_XSMALL;
    static const long ID_MENUITEM_GRID_ICON_SMALL;
    static const long ID_MENUITEM_GRID_ICON_MEDIUM;
    static const long ID_MENUITEM_GRID_ICON_LARGE;
    static const long ID_MENUITEM_GRID_ICON_XLARGE;
    static const long ID_MENUITEM6;
    static const long ID_MENUITEM_GRID_ICON_BACKGROUND_ON;
    static const long ID_MENUITEM_GRID_ICON_BACKGROUND_OFF;
    static const long ID_MENUITEM_Grid_Icon_Backgrounds;
    static const long ID_MENUITEM_GRID_NODE_VALUES_ON;
    static const long ID_MENUITEM_GRID_NODE_VALUES_OFF;
    static const long ID_MENUITEM8;
    static const long ID_MENU_CANVAS_ERASE_MODE;
    static const long ID_MENU_CANVAS_CANVAS_MODE;
    static const long ID_MENUITEM_RENDER_MODE;
    static const long ID_MENUITEM_EFFECT_ASSIST_ALWAYS_ON;
    static const long ID_MENUITEM_EFFECT_ASSIST_ALWAYS_OFF;
    static const long ID_MENUITEM_EFFECT_ASSIST_TOGGLE;
    static const long ID_MENUITEM_EFFECT_ASSIST;
    static const long ID_MENU_OPENGL_AUTO;
    static const long ID_MENU_OPENGL_3;
    static const long ID_MENU_OPENGL_2;
    static const long ID_MENU_OPENGL_1;
    static const long ID_MENUITEM19;
    static const long ID_MENUITEM5;
    static const long idMenuHelpContent;
    static const long ID_STATUSBAR1;
    static const long ID_TIMER1;
    static const long ID_MESSAGEDIALOG1;
    //*)

    static const long ID_PANEL_EFFECTS1;
    static const long ID_PANEL_EFFECTS;
    static const long ID_NOTEBOOK_EFFECTS;

    static const long ID_PLAYER_DIALOG;
    static const long ID_DELETE_EFFECT;
    static const long ID_IGNORE_CLICK;
    static const long ID_PROTECT_EFFECT;
    static const long ID_UNPROTECT_EFFECT;
    static const long ID_RANDOM_EFFECT;
    static const long ID_COPYROW_EFFECT; //copy random effect across row -DJ
    static const long ID_CLEARROW_EFFECT; //clear all effects on this row -DJ





    //(*Declarations(xLightsFrame)
    xlAuiToolBar* OutputToolBar;
    wxChoice* Choice_PgoOutputType;
    wxButton* Button_pgo_filename;
    wxButton* ButtonAddE131;
    wxMenuItem* MenuItemViewSavePerspective;
    wxMenuItem* QuitMenuItem;
    wxMenuItem* MenuItem33;
    wxMenuItem* MenuItemLoadEditPerspective;
    wxMenuItem* MenuItemGridNodeValuesOff;
    wxCheckBox* CheckBoxRunSchedule;
    wxMenuItem* MenuItem40;
    wxCheckBox* CheckBox_CoroEyesRandomLR;
    wxButton* ButtonClearLog;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxMenu* ToolIconSizeMenu;
    wxMenuItem* MenuItem_File_Open_Sequence;
    wxAuiManager* m_mgr;
    wxStaticText* StaticText33;
    wxTextCtrl* TextCtrl_PgoAutoFade;
    wxStaticText* StaticText2;
    wxTextCtrl* TextCtrl_pgo_filename;
    wxAuiManager* MainAuiManager;
    wxMenuItem* MenuItemRenderCanvasMode;
    wxButton* Button_PgoStitch;
    wxStaticText* StaticTextShowEnd;
    wxButton* Button_papagayo_output_sequence;
    wxMenuItem* MenuItemGridNodeValuesOn;
    wxMenu* MenuItem15;
    wxMenu* Menu3;
    wxStaticText* StaticText26;
    wxTextCtrl* TextCtrlLog;
    wxMenu* MenuItemRenderMode;
    wxButton* Button_Change_Media_Dir;
    wxMenuItem* MenuItemRefresh;
    wxMenuItem* MenuItem_File_Save_Sequence;
    wxMenuItem* MenuItem36;
    wxTextCtrl* TextCtrl_papagayo_output_filename;
    wxStaticText* StaticText32;
    wxButton* ButtonNetworkDeleteAll;
    wxMenuItem* MenuItemGridIconBackgroundOn;
    wxMenuItem* MenuItem_File_Close_Sequence;
    wxStaticText* StaticTextShowStart;
    wxMenu* MenuItem39;
    xlAuiToolBar* ViewToolBar;
    wxMenuItem* MenuItem37;
    wxStaticText* StaticText1;
    wxMenuItem* MenuItem32;
    wxStaticText* StaticText27;
    wxMenuItem* MenuItem_ViewZoomIn;
    wxPanel* PanelPreview;
    wxStaticText* StaticText3;
    xlAuiToolBar* PlayToolBar;
    wxMenu* Menu1;
    wxPanel* PanelSequencer;
    wxButton* ButtonSaveLog;
    wxBitmapButton* BitmapButton_SaveCoroGroup;
    wxMenuItem* MenuItem42;
    wxButton* ButtonStartPapagayo;
    wxStaticText* StaticText67;
    wxChoice* Choice_PgoGroupName;
    wxPanel* Panel3;
    wxMenuItem* MenuItemEffectAssistAlwaysOn;
    wxMenu* MenuItem7;
    wxButton* ButtonAddDongle;
    wxMenuItem* MenuItem_ViewZoomOut;
    wxButton* Button_CoroGroupClear;
    wxCheckBox* CheckBox_CoroPictureScaled;
    wxStaticText* StaticText24;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxMenuItem* MenuItemBackup;
    xlAuiToolBar* WindowMgmtToolbar;
    wxMenuItem* Menu_GenerateCustomModel;
    wxMenuItem* MenuItem38;
    wxMenuItem* MenuItemRenderEraseMode;
    wxMenuItem* MenuItem3;
    wxMenuItem* MenuItemConvert;
    wxButton* ButtonNetworkChange;
    wxButton* ButtonAddNull;
    wxMenuItem* Menu_Settings_Sequence;
    wxMenu* MenuSettings;
    wxButton* ButtonNetworkDelete;
    wxBitmapButton* BitmapButton_Link_Dirs;
    wxMenuItem* MenuItemEffectAssistAlwaysOff;
    wxStaticText* StaticText34;
    wxTreeCtrl* ListBoxSched;
    wxListCtrl* GridNetwork;
    wxStaticText* ShowDirectoryLabel;
    wxPanel* PanelCal;
    wxStatusBar* StatusBar1;
    wxMenu* MenuFile;
    wxDirDialog* DirDialog1;
    wxAuiNotebook* Notebook1;
    wxMenuItem* MenuItem35;
    wxButton* ButtonDeleteShow;
    wxSplitterWindow* SplitterWindow3;
    wxButton* ButtonShowDatesChange;
    wxTextCtrl* TextCtrl_PgoMaxRest;
    wxMenuItem* MenuItem_File_SaveAs_Sequence;
    xlAuiToolBar* MainToolBar;
    wxMenuItem* MenuItemEffectAssistToggleMode;
    wxButton* Button_PgoCopyVoices;
    wxStaticText* StaticText43;
    wxStaticText* MediaDirectoryLabel;
    wxMenuItem* mRenderOnSaveMenuItem;
    wxMenuItem* MenuItem41;
    wxPanel* PanelSetup;
    wxTextCtrl* TextCtrl_PgoMinRest;
    wxStaticText* StaticText68;
    wxButton* ButtonSaveSetup;
    wxStaticText* StaticText35;
    wxStaticText* StaticText_PgoOutputType;
    wxCheckBox* CheckBox_PgoAutoFade;
    wxPanel* Panel2;
    wxMenuItem* MenuItemSavePlaylists;
    wxPanel* PanelPapagayo;
    wxCheckBox* CheckBox_PgoAutoRest;
    wxButton* ButtonUpdateShow;
    wxMessageDialog* MessageDialog1;
    wxMenu* GridSpacingMenu;
    wxStaticText* StaticText25;
    wxMenuItem* MenuItem16;
    wxMenuItem* MenuItem34;
    wxSplitterWindow* SplitterWindow1;
    wxMenuItem* MenuItem43;
    wxMenuItem* MenuItemEffectAssistWindow;
    wxMenuItem* ActionTestMenuItem;
    wxStaticText* StaticText36;
    wxStaticText* StaticText4;
    wxMenu* MenuItem18;
    wxMenu* MenuItem1;
    xlAuiToolBar* EffectsToolBar;
    wxButton* ButtonSaveSchedule;
    wxButton* ButtonAddShow;
    wxButton* Button_CoroGroupDelete;
    wxMenu* AudioMenu;
    wxTimer Timer1;
    wxFileDialog* FileDialogPgoImage;
    xlAuiToolBar* EditToolBar;
    wxMenuItem* MenuItemGridIconBackgroundOff;
    wxMenuBar* MenuBar;
    wxCheckBox* CheckBox_CoroEyesRandomBlink;
    wxGrid* GridCoroFaces;
    //*)

    AUIToolbarButtonWrapper *CheckBoxLightOutput;
    AUIToolbarButtonWrapper *ButtonGracefulStop;
    AUIToolbarButtonWrapper *ButtonPasteByTime;
    AUIToolbarButtonWrapper *ButtonPasteByCell;

    wxBitmap pauseIcon;
    wxBitmap playIcon;
    bool previewLoaded;
    bool previewPlaying;
    wxFileName networkFile;
    wxFileName scheduleFile;
    PlayerFrame* PlayerDlg;
    wxArrayString mru;  // most recently used directories
    wxMenuItem* mru_MenuItem[MRU_LENGTH];
    wxXmlDocument NetworkXML;
    long DragRowIdx;
    wxListCtrl* DragListBox;
    bool UnsavedNetworkChanges;
    bool UnsavedPlaylistChanges;
    int mSavedChangeCount;
    wxDateTime starttime;
    play_modes play_mode;
    NetInfoClass NetInfo;

    ModelPreview* modelPreview;

    EffectManager effectManager;

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
    void PromptForShowDirectory();
    void UpdateNetworkList();
    long GetNetworkSelection();
    void MoveNetworkRow(int fromRow, int toRow);
    void OnGridNetworkDragQuit(wxMouseEvent& event);
    void OnGridNetworkDragEnd(wxMouseEvent& event);
    void SetupDongle(wxXmlNode* e);
    void SetupE131(wxXmlNode* e);
    void SetupNullOutput(wxXmlNode* e);
    bool SaveNetworksFile();
    void SaveFPPUniverses(std::string path);

    // test
    //void SetTestCheckboxes(bool NewValue);
    //void GetCheckedItems(wxArrayInt& chArray);
    //void GetTestPresetNames(wxArrayString& PresetNames);
    //void TestButtonsOff();
    //bool CheckChannelList;
    //int ChaseGrouping;
    //int TwinkleRatio;
    //TestFunctions TestFunc;
    //void OnTimerTest(long curtime);

    wxString mediaFilename;
    wxString showDirectory;
    wxString mediaDirectory;
    SeqDataType SeqData;

    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;
    long seekPoint;

    int mHitTestNextSelectModelIndex;
    wxString mBackgroundImage;
    int mBackgroundBrightness;
    bool mScaleBackgroundImage = false;

    // convert
public:
    bool UnsavedRgbEffectsChanges;

    //void ReadLorFile(const wxString& filename, int LORImportInterval);
    //void WriteLorFile(const wxString& filename);  //      LOR *.lms, *.las

    void ClearLastPeriod();
    void WriteVirFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf); //       Vixen *.vir
    void WriteHLSFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      HLS *.hlsnc
    void WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      LOR *.lcb
    void WriteLSPFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf, int cpn);  //      LSP UserPatterns.xml
    wxString base64_encode();
    int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);
    void ReadXlightsFile(const wxString& FileName, wxString *mediaFilename = NULL);
    void ReadFalconFile(const wxString& FileName, ConvertDialog* convertdlg); // = NULL);
    void WriteFalconPiFile(const wxString& filename); //  Falcon Pi Player *.pseq

private:

    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString &VixChannelNames);
    //void ReadVixFile(const wxString& filename);
    //void ReadHLSFile(const wxString& filename);
    //void ReadGlediatorFile(const wxString& FileName);
    //void ReadConductorFile(const wxString& FileName);
    int GetLorTrack1Length(const char* filename);
    //bool WriteVixenFile(const wxString& filename); //     Vixen *.vix
    //void WriteVirFile(const wxString& filename);
    //void WriteHLSFile(const wxString& filename);  //      HLS *.hlsnc
    //void WriteXLightsFile(const wxString& filename); //   xLights *.xseq
    void WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
                                SeqDataType *dataBuf, int startAddr, int modelSize); //Falcon Pi sub sequence .eseq
    void WriteConductorFile(const wxString& filename); // Conductor *.seq
    void WriteLSPFile(const wxString& filename);  //      LSP UserPatterns.xml
    void WriteLcbFile(const wxString& filename);  //      LOR *.lcb
    void DoConversion(const wxString& FileName, const wxString& OutputFormat);
    bool mapEmptyChannels();
    bool showChannelMapping();
    bool isSetOffAtEnd();


    wxXmlNode* SelectModelToExport();

    JobPool jobPool;

//  papagayo
    void PapagayoError(const wxString& msg);
    //void AutoFace();
    int write_pgo_header(wxFile& f); //, int MaxVoices);
    void write_pgo_footer(wxFile& f); //, int MaxVoices);
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
    void BackupEffectsFile();
    wxString LoadEffectsFileNoCheck();
    void CreateDefaultEffectsXml();
    void UpdateEffectsList();
    void UpdateView();
    void ShowAllModelsView();
    void ShowModelsView();
    void ViewHideAllModels();
    void ChooseColor(wxTextCtrl* TextCtrl);
    void LoadSizerControlsToAttr(wxSizer* sizer,wxXmlNode* x);
    void PlayRgbEffect(int EffectPeriod, SettingsMap &SettingsMap);
    void TimerRgbSeq(long msec);
    void SetChoicebook(wxChoicebook* cb, const wxString& PageName);
    void UpdateGrid();
    void SetXmlSetting(const wxString& settingName,const wxString& value);
    wxString GetXmlSetting(const wxString& settingName,const wxString& defaultValue);

    wxString CreateEffectString();
    void OpenPaletteDialog(const wxString& id1, const wxString& id2, wxSizer* PrimarySizer,wxSizer* SecondarySizer);
    void ChooseModelsForSequence();
    void GetGridColumnLabels(wxArrayString& a);

    void DisplayXlightsFilename(const wxString& filename);
    void CopyRow(int row1, int row2);
    void NumericSort();
    double GetGridStartTime(int row);
    long GetGridStartTimeMSec(int row);
    void UpdateRgbPlaybackStatus(int seconds, long msec, int EffectPeriod, const wxString& seqtype);
    //void SetTextColor(wxWindow* w);
    int ChooseRandomEffect();

    void GridCellChanged(int row, int col);
    void UpdateBufferFadesFromCtrl(PixelBufferClass &buffer);
    int UpdateEffectDuration(bool new_effect_starts, int startRow, PixelBufferClass &buffer, int playCol);
    void ResetEffectDuration(PixelBufferClass &buffer);


public:
    bool InitPixelBuffer(const std::string &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased = false);
    Model *GetModel(const std::string& name);
    void RenderGridToSeqData();
    bool RenderEffectFromMap(Effect *effect, int layer, int period, const SettingsMap& SettingsMap,
                             PixelBufferClass &buffer, bool &ResetEffectState,
                             bool bgThread = false, RenderEvent *event = NULL);
    void RenderEffectOnMainThread(RenderEvent *evt);
    void RenderEffectForModel(const std::string &model, int startms, int endms, bool clear = false);

    void RenderRange(RenderCommandEvent &cmd);
    void RenderDone();

    void EnableSequenceControls(bool enable);
    SequenceElements& GetSequenceElements() { return mSequenceElements; }
    Element* AddTimingElement(const std::string& name);
    void DeleteTimingElement(const std::string& name);
    void RenameTimingElement(const std::string& old_name, const std::string& new_name);
    void ImportTimingElement();
    void ExecuteImportTimingElement(wxCommandEvent &command);
    void ConvertDataRowToEffects(wxCommandEvent &command);
    void ConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime);
    void PromoteEffects(wxCommandEvent &command);
    void PromoteEffects(Element *element);
    wxXmlNode* CreateEffectNode(wxString& name);
    void UpdateEffectNode(wxXmlNode* node);
    void ApplyEffectsPreset(wxString& data);
    void RenameModelInViews(const std::string old_name, const std::string& new_name);
    bool RenameModel(const std::string old_name, const std::string& new_name);
    void UpdateSequenceLength();

    void SetSequenceEnd(int ms);
    void UpdateRenderMode();
    void SetFrequency(int frequency);
    void RenderAll();

protected:
    void ClearEffectWindow();
    bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
    bool SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels);
    void ResetEffectsXml();
    void SeqLoadXlightsXSEQ(const wxString& filename);
    std::string CreateEffectStringRandom(std::string &settings, std::string &palette);
    void BackupDirectory(wxString targetDirName);
    void NewSequence();
    void OpenSequence(wxString passed_filename, ConvertLogDialog* plog);
    void SaveSequence();
    void SaveAsSequence();
    bool CloseSequence();
    void InsertRow();
    void ShowPreviewTime(long ElapsedMSec);
    void PreviewOutput(int period);
    void TimerOutput(int period);
    void GetSeqModelNames(wxArrayString& a);
    void UpdateChannelNames();
    void StopNow(void);
    void PlayRgbSequence(void);
    bool IsValidEffectString(wxString& s);
    void LoadPapagayoFile(const wxString& filename, int frame_offset = 0);
    void InitPapagayoTab(bool tab_changed);
    bool LoadPgoSettings(void);
    bool SavePgoSettings(void);
    bool GetGroupName(wxString& grpname);
    void PgoGridCellSelect(int row, int col, int where);
    void SetSelectedModelToGroupSelected();
    void ShowModelProperties();
    void SetModelAsPartOfDisplay(wxString& model);
    unsigned int GetMaxNumChannels();


    wxXmlDocument pgoXml; //Papagayo settings from xlights_papagayo.xml
    bool Grid1HasFocus; //cut/copy/paste handled differently with grid vs. other text controls -DJ
    wxXmlDocument EffectsXml;
    wxXmlNode* EffectsNode;
    wxXmlNode* PalettesNode;
    wxXmlNode* ViewsNode;
    wxXmlNode* PerspectivesNode;
public:
    wxXmlNode* ModelsNode;
    wxXmlNode* ModelGroupsNode;
private:
    wxXmlNode* SettingsNode;

    bool MixTypeChanged;
    bool FadesChanged;
    long SeqBaseChannel;
    bool SeqChanCtrlBasic;
    bool SeqChanCtrlColor;
	bool mLoopAudio;

    bool mResetToolbars;
    bool mRenderOnSave;
    int mIconSize;
    int mGridSpacing;
    bool mGridIconBackgrounds;
    bool mGridNodeValues;
    int mEffectAssistMode;
	bool mRendering;

    Model *playModel;
    int playType;
    int playStartMS;
    int playStartTime;
    int playOffsetTime;
    int playEndTime;
    bool replaySection;

    std::string selectedEffectName;
    std::string selectedEffectString;
    std::string selectedEffectPalette;
    Effect *selectedEffect;

    std::string lastPlayEffect;
    double mPointSize = 2.0;

    int NextGridRowToPlay;
    int SeqPlayColumn;

	wxArrayString ChNames;
	int _totalChannels;

    wxGridCellCoords *curCell;
    int PlaybackPeriod; //used to be able to record the frame being played in an animation preview

public:
    //stuf used by layoutPanel for now
    void SetPreviewBackgroundScaled(bool scaled);
    void SetPreviewSize(int width, int height);
    void SetPreviewBackgroundImage(const wxString &filename);
    void SetPreviewBackgroundBrightness(int i);
    void UpdatePreview();
    void UpdateModelsList();
    void RowHeadingsChanged( wxCommandEvent& event);
    int GetTotalChannels() { return _totalChannels; };

private:

    wxXmlNode* mCurrentPerpective;
    std::vector<bool> savedPaneShown;

    // New Sequencer variables and methods
    SequenceElements mSequenceElements;

    MainSequencer* mainSequencer;
    ModelPreview * sPreview1;
    ModelPreview * sPreview2;
    LayoutPanel *layoutPanel;
    EffectAssist* sEffectAssist;
    ColorPanel* colorPanel;
    TimingPanel* timingPanel;
    PerspectivesPanel* perspectivePanel;
    EffectIconPanel *effectPalettePanel;
    BufferPanel *bufferPanel;
    DisplayElementsPanel *displayElementsPanel;


    int mMediaLengthMS;
    bool mSequencerInitialize = false;
    // Methods
    void InitSequencer();
    void CreateSequencer();
    void StopSequence();
    // Events
    void Zoom( wxCommandEvent& event);
    void WindowResized( wxCommandEvent& event);
    void TimeSelected( wxCommandEvent& event);
    void MousePositionUpdated( wxCommandEvent& event);
    void SelectedEffectChanged( wxCommandEvent& event);
    void EffectChanged( wxCommandEvent& event);
    void UnselectedEffect( wxCommandEvent& event);
    void EffectDroppedOnGrid(wxCommandEvent& event);
    void PlayModelEffect(wxCommandEvent& event);
    void UpdateEffect(wxCommandEvent& event);
    void UpdateEffectPalette(wxCommandEvent& event);
    void ForceSequencerRefresh(wxCommandEvent& event);
    void LoadPerspective(wxCommandEvent& event);
    void PerspectivesChanged(wxCommandEvent& event);
    void PlayModel(wxCommandEvent& event);
    void CopyModelEffects(wxCommandEvent& event);
    void PasteModelEffects(wxCommandEvent& event);
    void ModelSelected(wxCommandEvent& event);
    void PlaySequence(wxCommandEvent& event);
    void PauseSequence(wxCommandEvent& event);
    void StopSequence(wxCommandEvent& event);
    void SequenceFirstFrame(wxCommandEvent& event);
    void SequenceLastFrame(wxCommandEvent& event);
    void SequenceReplaySection(wxCommandEvent& event);
    void TogglePlay(wxCommandEvent& event);
    void ExportModel(wxCommandEvent& event);
    void ShowDisplayElements(wxCommandEvent& event);

    void CheckForValidModels();

    void LoadSequencer(xLightsXmlFile& xml_file);
    void LoadPerspective(wxXmlNode *p);

    void CheckForAndCreateDefaultPerpective();
    void ZoomIn();
    void ZoomOut();
    void ResizeAndMakeEffectsScroll();
    void ResizeMainSequencer();
    std::string GetEffectTextFromWindows(std::string &palette);

    void EnableToolbarButton(wxAuiToolBar* toolbar,int id, bool enable);
    // Panels
    TopEffectsPanel* effectsPnl;
    EffectsPanel* EffectsPanel1;
    wxFlexGridSizer* FlexGridEffects;
    std::set<int> LorTimingList; // contains a list of period numbers, set by ReadLorFile()

//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
//#include <unordered_map>
    typedef enum { Normal, Locked, Random } EditState;
    std::unordered_map<std::string, EditState> buttonState;
    bool isRandom_(wxControl* ctl, const char*debug);
#define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)

    DECLARE_EVENT_TABLE()
    friend class xLightsApp; //kludge: allow xLightsApp to call OnPaneNutcrackerChar -DJ
public:
    std::vector<Model *> PreviewModels;
    ModelManager AllModels;
    static wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false);

    wxString GetSeqXmlFileName();
	void PlaySequence();

    EffectTreeDialog *EffectTreeDlg;

};

#endif // XLIGHTSMAIN_H
