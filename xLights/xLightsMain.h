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

#ifdef _MSC_VER
    #include <stdlib.h>

    //#define VISUALSTUDIO_MEMORYLEAKDETECTIO
    #ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
        #define _CRTDBG_MAP_ALLOC
        #include <crtdbg.h>
    #endif
#endif

//(*Headers(xLightsFrame)
#include <wx/aui/aui.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/timer.h>
//*)

#include <wx/choicebk.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/dir.h>
#include <wx/treebase.h>
#include <wx/socket.h>

#include <unordered_map>
#include <map>
#include <set>
#include <vector>

#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#inlcude <windows.h>
#endif

#include "outputs/OutputManager.h"
#include "PixelBuffer.h"
#include "SequenceData.h"
#include "effects/EffectManager.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "xLightsTimer.h"
#include "JobPool.h"
#include "SequenceViewManager.h"
#include "ColorManager.h"
#include "ViewpointMgr.h"
#include "PhonemeDictionary.h"
#include "xLightsXmlFile.h"
#include "sequencer/EffectsGrid.h"
#include "RenderCache.h"

class EffectTreeDialog;
class ConvertDialog;
class ConvertLogDialog;
class wxDebugReport;
class RenderTreeData;
class HousePreviewPanel;
class SelectPanel;
class SequenceVideoPanel;
class EffectIconPanel;
class JukeboxPanel;
class TimingPanel;
class ColorPanel;
class EffectsPanel;
class EffectAssist;
class LayoutGroup;
class ViewsModelsPanel;
class PerspectivesPanel;
class TopEffectsPanel;
class MainSequencer;
class ModelPreview;

// max number of most recently used show directories on the File menu
#define MRU_LENGTH 4

// notebook pages
#define SETUPTAB 0
#define LAYOUTTAB 1
#define NEWSEQUENCER 2

#define PLAY_TYPE_STOPPED 0
#define PLAY_TYPE_EFFECT 1
#define PLAY_TYPE_MODEL  2
#define PLAY_TYPE_EFFECT_PAUSED 3
#define PLAY_TYPE_MODEL_PAUSED  4

#define FixedPages 3

#define TEXT_ENTRY_TIMING           0
#define TEXT_ENTRY_EFFECT           1
#define TEXT_ENTRY_DIALOG           2

class RenderCommandEvent;
class SelectedEffectChangedEvent;

wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_GSCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_MOUSE_POSITION, wxCommandEvent);
wxDECLARE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_ROW_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_UPDATED, wxCommandEvent);
wxDECLARE_EVENT(EVT_UNSELECTED_EFFECT, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECTFILE_DROPPED, wxCommandEvent);
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
wxDECLARE_EVENT(EVT_SEQUENCE_REWIND10, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_FFORWARD10, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_SEEKTO, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_REPLAY_SECTION, wxCommandEvent);
wxDECLARE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMPORT_NOTES, wxCommandEvent);
wxDECLARE_EVENT(EVT_CONVERT_DATA_TO_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PROMOTE_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_APPLYLAST, wxCommandEvent);
wxDECLARE_EVENT(EVT_RGBEFFECTS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TURNONOUTPUTTOLIGHTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAYJUKEBOXITEM, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);

static const wxString xlights_base_name       = "xLights";
static const wxString strSupportedFileTypes = "LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|Vixen Sequences (*.vix)|*.vix|Glediator Record File (*.gled)|*.gled)|Lynx Conductor Sequences (*.seq)|*.seq|xLights Sequences(*.xseq)|*.xseq|xLights Imports(*.iseq)|*.iseq|Falcon Pi Player Sequences (*.fseq)|*.fseq";
static const wxString strSequenceSaveAsFileTypes = "xLights Sequences(*.xml)|*.xml";

typedef SequenceData SeqDataType;

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
class RenderProgressDialog;
class RenderProgressInfo;

class xlAuiToolBar : public wxAuiToolBar {
public:
    xlAuiToolBar(wxWindow* parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxAUI_TB_DEFAULT_STYLE);
    virtual ~xlAuiToolBar() {}

    wxSize &GetAbsoluteMinSize() {return m_absoluteMinSize;}
    wxSize GetMinSize() const override {return m_absoluteMinSize;}
};

class FPSEvent
{
public:
    wxDateTime when;
    int frame;

    FPSEvent(int f)
    {
        frame = f;
        when = wxDateTime::UNow();
    }
};

class SchedTreeData : public wxTreeItemData
{
protected:
    wxString eventString;

public:
    SchedTreeData(const wxString& EventData = wxT("")) {eventString = EventData;};
    wxString GetString() const {return eventString;};
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
    wxAuiToolBar *toolbar = nullptr;
    int id;
};

class xLightsFrame: public wxFrame
{
public:

    xLightsFrame(wxWindow* parent,wxWindowID id = -1);
    virtual ~xLightsFrame();

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
        REMOVE_SCRIPT_BUTTON,

        PLAY_LIST_MAX
    };
    long SecondsRemaining, EndTimeSec;
    int TxOverflowCnt, TxOverflowTotal;
    std::mutex saveLock;
    RenderCache _renderCache;
    std::atomic_bool _exiting;

    PhonemeDictionary dictionary;

    bool IsExiting() const { return _exiting; }
    void SetEffectControls(const std::string &modelName, const std::string &name,
                           const SettingsMap &settings, const SettingsMap &palette,
                           bool setDefaults);
    void SetEffectControls(const SettingsMap &settings);
    void ApplyLast(wxCommandEvent& event);
    void SetEffectControlsApplyLast(const SettingsMap &settings);
    bool ApplySetting(wxString name, const wxString &value);
    void LoadPerspectivesMenu(wxXmlNode* perspectivesNode);
    struct PerspectiveId {
        int id;
        wxXmlNode* p = nullptr;
    };

    PerspectiveId perspectives[10];
    void OnMenuItemLoadPerspectiveSelected(wxCommandEvent& event);
	bool SaveEffectsFile(bool backup = false);
    void SaveModelsFile();
    void MarkEffectsFileDirty(bool modelStructureChange);
    void CheckUnsavedChanges();
    void SetStatusText(const wxString &msg, int filename = 0);
    void SetStatusTextColor(const wxString &msg, const wxColor& colour);
	std::string GetChannelToControllerMapping(long channel);
    void GetControllerDetailsForChannel(long channel, std::string& type, std::string& description, long& channeloffset, std::string &ip, std::string& u, std::string& inactive, int& output, std::string& baud);

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
    static wxString FseqDir; //expose current fseq name
    static wxString PlaybackMarker; //keep track of where we are within grid -DJ
    static wxString xlightsFilename; //expose current path name -DJ
    static xLightsXmlFile* CurrentSeqXmlFile; // global object for currently opened XML file
    const wxString &GetShowDirectory() const { return showDirectory; }
    const wxString &GetFseqDirectory() const { return fseqDirectory; }
    static wxString GetFilename() { return xlightsFilename; }
    void ConversionInit();
    void ConversionError(const wxString& msg);
    void SetMediaFilename(const wxString& filename);
    void RenderIseqData(bool bottom_layers, ConvertLogDialog* plog);
    bool IsSequenceDataValid() const
    { return SeqData.IsValidData(); }
    void ClearSequenceData();
    void LoadAudioData(xLightsXmlFile& xml_file);
    void CreateDebugReport(wxDebugReportCompress *report);
    wxString GetThreadStatusReport();
	void SetAudioControls();
    void ImportXLights(const wxFileName &filename);
    void ImportXLights(SequenceElements &se, const std::vector<Element *> &elements, const wxFileName &filename,
        bool allowAllModels = false, bool clearSrc = false);
    void ImportVix(const wxFileName &filename);
    void ImportHLS(const wxFileName &filename);
    void ImportLMS(const wxFileName &filename);
    void ImportLPE(const wxFileName &filename);
    void ImportS5(const wxFileName &filename);
    void ImportLSP(const wxFileName &filename);
    void ImportVsa(const wxFileName &filename);
    void ImportSuperStar(const wxFileName &filename);
    void SaveWorking();
    void SaveWorkingLayout();
    void PlayerError(const wxString& msg);
    void AskCloseSequence();
    void SaveCurrentTab();

    EffectManager &GetEffectManager() { return effectManager; }

    bool ImportSuperStar(Element *el, wxXmlDocument &doc, int x_size, int y_size,
                         int x_offset, int y_offset, bool average_colors,
                         int imageResizeType, const wxSize &modelSize);
    bool ImportLMS(wxXmlDocument &doc, const wxFileName &filename);
    bool ImportLPE(wxXmlDocument &doc, const wxFileName &filename);
    bool ImportS5(wxXmlDocument &doc, const wxFileName &filename);

    //(*Handlers(xLightsFrame)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnMenuOpenFolderSelected(wxCommandEvent& event);
    void OnTimer1Trigger(wxTimerEvent& event);
    void OnButtonNetworkChangeClick(wxCommandEvent& event);
    void OnButtonNetworkDeleteClick(wxCommandEvent& event);
    void OnButtonNetworkMoveUpClick(wxCommandEvent& event);
    void OnButtonNetworkMoveDownClick(wxCommandEvent& event);
    void OnGridNetworkBeginDrag(wxListEvent& event);
    void OnButtonAddE131Click(wxCommandEvent& event);
    void OnButtonAddDongleClick(wxCommandEvent& event);
    void OnButtonSaveSetupClick(wxCommandEvent& event);
    void OnBitmapButtonTabInfoClick(wxCommandEvent& event);
    void OnButtonLightsOffClick(wxCommandEvent& event);
    void OnCheckBoxLightOutputClick(wxCommandEvent& event);
    void OnButtonStopNowClick(wxCommandEvent& event);
    void OnButtonNetworkDeleteAllClick(wxCommandEvent& event);
    void OnBitmapButtonOpenSeqClick(wxCommandEvent& event);
    void OnBitmapButtonSaveSeqClick(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnMenuItemBackupSelected(wxCommandEvent& event);
    void OnEffectsPanel1Paint(wxPaintEvent& event);
    void OnGrid1SetFocus(wxFocusEvent& event);
    void OnGrid1KillFocus(wxFocusEvent& event);
    void OntxtCtrlSparkleFreqText(wxCommandEvent& event);
    void OnPanelSequencerPaint(wxPaintEvent& event);
    void OnButtonNewSequenceClick(wxCommandEvent& event);
    void OnButtonClickSaveAs(wxCommandEvent& event);
    void OnNotebook1PageChanged1(wxAuiNotebookEvent& event);
    void ShowHideModelPreview(wxCommandEvent& event);
    void ShowHideHousePreview(wxCommandEvent& event);
    void OnAuiToolBarItemPlayButtonClick(wxCommandEvent& event);
    void OnAuiToolBarItemPauseButtonClick(wxCommandEvent& event);
    void OnAuiToolBarItemStopClick(wxCommandEvent& event);
    void OnAuiToolBarItemZoominClick(wxCommandEvent& event);
    void OnAuiToolBarItem_ZoomOutClick(wxCommandEvent& event);
    void OnMenuItemLoadEditPerspectiveSelected(wxCommandEvent& event);
    void OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event);
    void OnMenu_Settings_SequenceSelected(wxCommandEvent& event);
    void OnMenuItem_File_Open_SequenceSelected(wxCommandEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnAuiToolBarItemRenderAllClick(wxCommandEvent& event);
    void OnMenuItem_File_Close_SequenceSelected(wxCommandEvent& event);
    void OnMenuItem_File_Export_VideoSelected(wxCommandEvent& event);
    void OnAuiToolBarFirstFrameClick(wxCommandEvent& event);
    void OnAuiToolBarLastFrameClick(wxCommandEvent& event);
    void OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event);
    void ShowHideEffectSettingsWindow(wxCommandEvent& event);
    void ShowHideColorWindow(wxCommandEvent& event);
    void ShowHideLayerTimingWindow(wxCommandEvent& event);
    void ShowHideEffectDropper(wxCommandEvent& event);
    void SetIconSize(wxCommandEvent& event);
    void ResetToolbarLocations(wxCommandEvent& event);
    void SetToolIconSize(wxCommandEvent& event);
    void OnMenuItemRenderEraseModeSelected(wxCommandEvent& event);
    void OnMenuItemRenderCanvasModeSelected(wxCommandEvent& event);
    void OnSetGridIconBackground(wxCommandEvent& event);
    void OnSetGridNodeValues(wxCommandEvent& event);
    void OnMenuItemImportEffects(wxCommandEvent& event);
    void SetPlaySpeed(wxCommandEvent& event);
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
    void OnAuiToolBarItemShowHideEffects(wxCommandEvent& event);
    void OnAuiToolBarItemPasteByTimeClick(wxCommandEvent& event);
    void OnAuiToolBarItemPasteByCellClick(wxCommandEvent& event);
    void OnMenuItemConvertSelected(wxCommandEvent& event);
    void OnMenu_GenerateCustomModelSelected(wxCommandEvent& event);
    void OnPaneClose(wxAuiManagerEvent& event);
    void OnMenuItemPackageDebugFiles(wxCommandEvent& event);
    void OnMenuOpenGLSelected(wxCommandEvent& event);
    void OnTimer_AutoSaveTrigger(wxTimerEvent& event);
    void AutoSaveIntervalSelected(wxCommandEvent& event);
    void OnEffectSettingsTimerTrigger(wxTimerEvent& event);
    void OnMenuItemViewSaveAsPerspectiveSelected(wxCommandEvent& event);
    void OnmBackupOnSaveSelected(wxCommandEvent& event);
    void OnmAltBackupMenuItemSelected(wxCommandEvent& event);
    void OnmExportModelsMenuItemSelected(wxCommandEvent& event);
    void OnMenuItem_BackupOnLaunchSelected(wxCommandEvent& event);
    void OnMenuItem_ViewLogSelected(wxCommandEvent& event);
    void OnMenuItemCheckSequenceSelected(wxCommandEvent& event);
    void OnSpinCtrl_SyncUniverseChange(wxSpinEvent& event);
    void OnMenuItem_e131syncSelected(wxCommandEvent& event);
    void OnButtonArtNETClick(wxCommandEvent& event);
    void OnMenuItem_Help_ForumSelected(wxCommandEvent& event);
    void OnMenuItem_Help_DownloadSelected(wxCommandEvent& event);
    void OnMenuItem_Help_ReleaseNotesSelected(wxCommandEvent& event);
    void OnMenuItem_Help_Isue_TrackerSelected(wxCommandEvent& event);
    void OnMenuItem_Help_FacebookSelected(wxCommandEvent& event);
    void OnMenuItem_ExportEffectsSelected(wxCommandEvent& event);
    void OnGridNetworkItemRClick(wxListEvent& event);
    void OnGridNetworkItemSelect(wxListEvent& event);
    void OnGridNetworkItemDeselect(wxListEvent& event);
    void OnGridNetworkItemFocused(wxListEvent& event);
    void OnGridNetworkKeyDown(wxListEvent& event);
    void OnMenuItem_FPP_ConnectSelected(wxCommandEvent& event);
    void OnMenuItemShiftEffectsSelected(wxCommandEvent& event);
    void OnMenuItem_PackageSequenceSelected(wxCommandEvent& event);
    void OnMenuItem_BackupSubfoldersSelected(wxCommandEvent& event);
    void OnMenuItem_xScheduleSelected(wxCommandEvent& event);
    void OnMenuItem_ForceLocalIPSelected(wxCommandEvent& event);
    void OnMenuItem_VideoTutorialsSelected(wxCommandEvent& event);
    void OnMenuItem_ExcludePresetsFromPackagedSequencesSelected(wxCommandEvent& event);
    void OnMenuItem_ExcludeAudioPackagedSequenceSelected(wxCommandEvent& event);
    void OnMenuItemColorManagerSelected(wxCommandEvent& event);
    void OnMenuItem_DonateSelected(wxCommandEvent& event);
    void OnMenuItemTimingPlayOnDClick(wxCommandEvent& event);
    void OnAC_OnClick(wxCommandEvent& event);
    void OnAC_OffClick(wxCommandEvent& event);
    void OnAC_DisableClick(wxCommandEvent& event);
    void OnMenuItem_ACLIghtsSelected(wxCommandEvent& event);
    void OnACToolbarDropdown(wxAuiToolBarEvent& event);
    void OnAC_ShimmerClick(wxCommandEvent& event);
    void OnAC_TwinkleClick(wxCommandEvent& event);
    void OnAC_BackgroundClick(wxCommandEvent& event);
    void OnAC_ForegroundClick(wxCommandEvent& event);
    void OnAC_CascadeClick(wxCommandEvent& event);
    void OnAC_FillClick(wxCommandEvent& event);
    void OnAC_RampUpDownClick(wxCommandEvent& event);
    void OnAC_RampDownClick(wxCommandEvent& event);
    void OnAC_RampUpClick(wxCommandEvent& event);
    void OnAC_IntensityClick(wxCommandEvent& event);
    void OnChoiceParm1Select(wxCommandEvent& event);
    void OnChoiceParm2Select(wxCommandEvent& event);
    void OnAC_SelectClick(wxCommandEvent& event);
    void OnMenuItem_ShowACRampsSelected(wxCommandEvent& event);
    void OnMenuItem_PerspectiveAutosaveSelected(wxCommandEvent& event);
    void OnMenuItem_SD_10Selected(wxCommandEvent& event);
    void OnMenuItem_SD_20Selected(wxCommandEvent& event);
    void OnMenuItem_SD_40Selected(wxCommandEvent& event);
    void OnMenuItem_SD_NoneSelected(wxCommandEvent& event);
    void OnMenuItem_PlayControlsOnPreviewSelected(wxCommandEvent& event);
    void OnMenuItem_AutoShowHousePreviewSelected(wxCommandEvent& event);
    void OnMenuItem_GenerateLyricsSelected(wxCommandEvent& event);
    void OnMenuItem_CrashXLightsSelected(wxCommandEvent& event);
    void OnMenuItemBatchRenderSelected(wxCommandEvent& event);
    void OnMenuItem_UpdateSelected(wxCommandEvent& event);
    void OnMenuItem_SmallWaveformSelected(wxCommandEvent& event);
    void OnMenuItem_LogRenderStateSelected(wxCommandEvent& event);
    void OnMenuItem_ModelBlendDefaultOffSelected(wxCommandEvent& event);
    void OnMenuItem_File_Save_Selected(wxCommandEvent& event);
    void OnMenuItem_SnapToTimingMarksSelected(wxCommandEvent& event);
    void OnMenuItem_PurgeVendorCacheSelected(wxCommandEvent& event);
    void OnButtonAddLORClick(wxCommandEvent& event);
    void OnMenuItem_LoudVolSelected(wxCommandEvent& event);
    void OnMenuItem_MedVolSelected(wxCommandEvent& event);
    void OnMenuItem_QuietVolSelected(wxCommandEvent& event);
    void OnMenuItem_VQuietVolSelected(wxCommandEvent& event);
    void ShowPresetsPanel();
    void OnMenuItemSelectEffectSelected(wxCommandEvent& event);
    void OnMenuItemShowHideVideoPreview(wxCommandEvent& event);
    void OnButtonAddDDPClick(wxCommandEvent& event);
    void OnButtonOtherFoldersClick(wxCommandEvent& event);
    void OnMenuItem_BackupPurgeIntervalSelected(wxCommandEvent& event);
    void OnMenuItem_DownloadSequencesSelected(wxCommandEvent& event);
    void OnMenuItem_JukeboxSelected(wxCommandEvent& event);
    void OnMenuItemShiftSelectedEffectsSelected(wxCommandEvent& event);
    void OnMenuItem_xFadeDisabledSelected(wxCommandEvent& event);
    void OnMenuItem_XFade_ASelected(wxCommandEvent& event);
    void OnMenuItem_xFade_BSelected(wxCommandEvent& event);
    void OnMenuItemUserDictSelected(wxCommandEvent& event);
    void OnmSaveFseqOnSaveMenuItemSelected(wxCommandEvent& event);
    void OnMenuItem_PurgeRenderCacheSelected(wxCommandEvent& event);
    void OnMenuItem_RenderCache(wxCommandEvent& event);
    void OnMenuItem_ShowKeyBindingsSelected(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMenuItem_ZoomSelected(wxCommandEvent& event);
    void OnButton_DiscoverClick(wxCommandEvent& event);
    void OnMenuItem_CleanupFileLocationsSelected(wxCommandEvent& event);
    void OnMenuItem_Generate2DPathSelected(wxCommandEvent& event);
    void OnMenuItemFSEQV1Selected(wxCommandEvent& event);
    void OnMenuItemFSEQV2Selected(wxCommandEvent& event);
    void OnMenuItem_PrepareAudioSelected(wxCommandEvent& event);
    void OnMenuItemOGLRenderOrder(wxCommandEvent& event);
    void OnMenuItem_UserManualSelected(wxCommandEvent& event);
    void OnMenuItem_MHS_NormalSelected(wxCommandEvent& event);
    void OnMenuItem_MHS_LargeSelected(wxCommandEvent& event);
    void OnMenuItem_MHS_ExtraLargeSelected(wxCommandEvent& event);
    void OnMenuItem61Selected(wxCommandEvent& event);
    //*)
    void OnCharHook(wxKeyEvent& event);
private:

    //void OnMenuItem53Selected(wxCommandEvent& event);

    void OnIdle(wxIdleEvent& event);
    void DoMenuAction(wxMenuEvent &evt);
	void ShowHideAllSequencerWindows(bool show);
	void ResetAllSequencerWindows();
	void SetEffectAssistWindowState(bool show);
    void UpdateEffectAssistWindow(Effect* effect, RenderableEffect* ren_effect);
    void MaybePackageAndSendDebugFiles();
    void SendReport(const wxString &loc, wxDebugReportCompress &report);
    void AddDebugFilesToReport(wxDebugReport &report);

public:

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
    static const long ID_AUITOOLBARITEM_ACDISABLED;
    static const long ID_AUITOOLBARITEM_ACSELECT;
    static const long ID_AUITOOLBARITEM_ACOFF;
    static const long ID_AUITOOLBARITEM_ACON;
    static const long ID_AUITOOLBARITEM_ACSHIMMER;
    static const long ID_AUITOOLBARITEM_ACTWINKLE;
    static const long ID_AUITOOLBARITEM_ACINTENSITY;
    static const long ID_AUITOOLBARITEM_ACRAMPUP;
    static const long ID_AUITOOLBARITEM_ACRAMPDOWN;
    static const long ID_AUITOOLBARITEM_ACRAMPUPDOWN;
    static const long ID_CHOICE_PARM1;
    static const long ID_CHOICE_PARM2;
    static const long ID_AUITOOLBARITEM_ACFILL;
    static const long ID_AUITOOLBARITEM_ACCASCADE;
    static const long ID_AUITOOLBARITEM_ACFOREGROUND;
    static const long ID_AUITOOLBARITEM_ACBACKGROUND;
    static const long ID_AUITOOLBAR_AC;
    static const long ID_AUITOOLBARITEM14;
    static const long ID_AUITOOLBAR_VIEW;
    static const long ID_BITMAPBUTTON_TAB_INFO;
    static const long ID_BUTTON_STOP_NOW;
    static const long ID_BUTTON_LIGHTS_OFF;
    static const long ID_CHECKBOX_LIGHT_OUTPUT;
    static const long ID_AUITOOLBAR_OUTPUT;
    static const long ID_AUIEFFECTSTOOLBAR;
    static const long ID_BUTTON_OTHER_FOLDERS;
    static const long ID_BUTTON3;
    static const long ID_STATICTEXT4;
    static const long ID_BUTTON_SAVE_SETUP;
    static const long ID_BUTTON_ADD_DONGLE;
    static const long ID_BUTTON_ADD_E131;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON_ADD_LOR;
    static const long ID_BUTTON_ADD_DDP;
    static const long ID_BUTTON_NETWORK_CHANGE;
    static const long ID_BUTTON_NETWORK_DELETE;
    static const long ID_BUTTON_NETWORK_DELETE_ALL;
    static const long ID_BUTTON5;
    static const long ID_STATICTEXT8;
    static const long ID_SPINCTRL1;
    static const long ID_BITMAPBUTTON1;
    static const long ID_BITMAPBUTTON2;
    static const long ID_LISTCTRL_NETWORKS;
    static const long ID_PANEL_SETUP;
    static const long ID_PANEL_PREVIEW;
    static const long XLIGHTS_SEQUENCER_TAB;
    static const long ID_NOTEBOOK1;
    static const long ID_STATICTEXT6;
    static const long ID_GAUGE1;
    static const long ID_PANEL5;
    static const long ID_STATICTEXT7;
    static const long ID_PANEL1;
    static const long ID_NEW_SEQUENCE;
    static const long ID_OPEN_SEQUENCE;
    static const long IS_SAVE_SEQ;
    static const long ID_SAVE_AS_SEQUENCE;
    static const long ID_CLOSE_SEQ;
    static const long ID_EXPORT_VIDEO;
    static const long ID_MENUITEM2;
    static const long ID_FILE_BACKUP;
    static const long ID_FILE_ALTBACKUP;
    static const long ID_SHIFT_EFFECTS;
    static const long ID_MNU_SHIFT_SELECTED_EFFECTS;
    static const long ID_MENUITEM13;
    static const long ID_MENUITEM_CONVERT;
    static const long ID_MENUITEM_GenerateCustomModel;
    static const long ID_MNU_GENERATELYRICS;
    static const long ID_MENU_GENERATE2DPATH;
    static const long ID_MNU_PREPAREAUDIO;
    static const long ID_MNU_CHECKSEQ;
    static const long ID_MNU_CLEANUPFILE;
    static const long ID_MENU_VIEW_LOG;
    static const long ID_MENUITEM18;
    static const long ID_EXPORT_MODELS;
    static const long ID_MNU_EXPORT_EFFECTS;
    static const long ID_MENU_FPP_CONNECT;
    static const long ID_MNU_PACKAGESEQUENCE;
    static const long ID_MENU_USER_DICT;
    static const long ID_MNU_DOWNLOADSEQUENCES;
    static const long ID_MENU_BATCH_RENDER;
    static const long ID_MNU_XSCHEDULE;
    static const long iD_MNU_VENDORCACHEPURGE;
    static const long ID_MNU_PURGERENDERCACHE;
    static const long ID_MNU_CRASH;
    static const long ID_MNU_DUMPRENDERSTATE;
    static const long ID_MENUITEM5;
    static const long MNU_ID_ACLIGHTS;
    static const long ID_MNU_SHOWRAMPS;
    static const long ID_MENUITEM_SAVE_PERSPECTIVE;
    static const long ID_MENUITEM_SAVE_AS_PERSPECTIVE;
    static const long ID_MENUITEM_LOAD_PERSPECTIVE;
    static const long ID_MNU_PERSPECTIVES_AUTOSAVE;
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
    static const long ID_MENUITEM_SELECT_EFFECT;
    static const long ID_MENUITEM_VIDEOPREVIEW;
    static const long ID_MNU_JUKEBOX;
    static const long ID_MENUITEM_WINDOWS_PERSPECTIVE;
    static const long ID_MENUITEM_WINDOWS_DOCKALL;
    static const long ID_MENUITEM11;
    static const long ID_MENUITEM10;
    static const long ID_PLAY_FULL;
    static const long ID_MNU_1POINT5SPEED;
    static const long ID_MN_2SPEED;
    static const long ID_MNU_3SPEED;
    static const long ID_MNU_4SPEED;
    static const long ID_PLAY_3_4;
    static const long ID_PLAY_1_2;
    static const long ID_PLAY_1_4;
    static const long ID_MNU_LOUDVOLUME;
    static const long ID_MNU_MEDVOLUME;
    static const long ID_MNU_QUIET;
    static const long ID_MNU_SUPERQUIET;
    static const long ID_IMPORT_EFFECTS;
    static const long ID_SEQ_SETTINGS;
    static const long ID_RENDER_ON_SAVE;
    static const long ID_SAVE_FSEQ_ON_SAVE;
    static const long ID_BACKUP_ON_SAVE;
    static const long ID_MENU_BACKUP_ON_LAUNCH;
    static const long ID_MNU_BKPPURGE_NEVER;
    static const long ID_MNU_BKPPURGE_YEAR;
    static const long ID_MNU_BKPPURGE_QUARTER;
    static const long ID_MNU_BKPPURGE_MONTH;
    static const long ID_MNU_BKPPURGE_WEEK;
    static const long ID_MNU_BKP_PURGE;
    static const long ID_MNU_BACKUP;
    static const long ID_MNU_EXCLUDEPRESETS;
    static const long ID_MNU_EXCLUDEAUDIOPKGSEQ;
    static const long ID_MENUITEM_ICON_SMALL;
    static const long ID_MENUITEM_ICON_MEDIUM;
    static const long ID_MENUITEM_ICON_LARGE;
    static const long ID_MENUITEM_ICON_XLARGE;
    static const long ID_MENUITEM4;
    static const long ID_MNU_SMALLWAVEFORM;
    static const long ID_MENUITEM_GRID_ICON_XSMALL;
    static const long ID_MENUITEM_GRID_ICON_SMALL;
    static const long ID_MENUITEM_GRID_ICON_MEDIUM;
    static const long ID_MENUITEM_GRID_ICON_LARGE;
    static const long ID_MENUITEM_GRID_ICON_XLARGE;
    static const long ID_MENUITEM6;
    static const long ID_MENUITEM24;
    static const long ID_MENUITEM25;
    static const long ID_MENUITEM26;
    static const long ID_MENUITEM23;
    static const long ID_MENUITEM_GRID_ICON_BACKGROUND_ON;
    static const long ID_MENUITEM_GRID_ICON_BACKGROUND_OFF;
    static const long ID_MENUITEM_Grid_Icon_Backgrounds;
    static const long ID_MENUITEM_GRID_NODE_VALUES_ON;
    static const long ID_MENUITEM_GRID_NODE_VALUES_OFF;
    static const long ID_MENUITEM8;
    static const long ID_COLOR_MANAGER;
    static const long ID_MNU_RC_ENABLE;
    static const long ID_MNU_RC_LOCKEDONLY;
    static const long ID_MNU_RC_DISABLED;
    static const long ID_MNU_RENDERCACHE;
    static const long ID_MENU_CANVAS_ERASE_MODE;
    static const long ID_MENU_CANVAS_CANVAS_MODE;
    static const long ID_MENUITEM_RENDER_MODE;
    static const long ID_MENUITEM_EFFECT_ASSIST_ALWAYS_ON;
    static const long ID_MENUITEM_EFFECT_ASSIST_ALWAYS_OFF;
    static const long ID_MENUITEM_EFFECT_ASSIST_TOGGLE;
    static const long ID_MENUITEM_EFFECT_ASSIST;
    static const long ID_MENU_TIMING_EDIT_MODE;
    static const long ID_MENU_TIMING_PLAY_MODE;
    static const long ID_MENUITEM_Timing_DClick_Mode;
    static const long ID_MENU_OPENGL_AUTO;
    static const long ID_MENU_OPENGL_3;
    static const long ID_MENU_OPENGL_1;
    static const long ID_MENUITEM_OGL_RO1;
    static const long ID_MENUITEM_OGL_RO2;
    static const long ID_MENUITEM_OGL_RO3;
    static const long ID_MENUITEM_OGL_RO4;
    static const long ID_MENUITEM_OGL_RO5;
    static const long ID_MENUITEM_OGL_RO6;
    static const long ID_MENUITEM19;
    static const long ID_MNU_PLAYCONTROLSONPREVIEW;
    static const long ID_MNU_AUTOSHOWHOUSEPREVIEW;
    static const long ID_MNU_SUPPRESS_TRANSITION_HINTS;
    static const long ID_MENUITEM_AUTOSAVE_0;
    static const long ID_MENUITEM_AUTOSAVE_3;
    static const long ID_MENUITEM_AUTOSAVE_10;
    static const long ID_MENUITEM_AUTOSAVE_15;
    static const long ID_MENUITEM_AUTOSAVE_30;
    static const long ID_MENUITEM20;
    static const long ID_MNU_XFADE_DISABLED;
    static const long ID_MNU_XFADE_A;
    static const long ID_MNU_XFADE_B;
    static const long ID_MNU_XFADE;
    static const long ID_MNU_SD_None;
    static const long ID_MNU_SD_10;
    static const long ID_MNU_SD_20;
    static const long ID_MNU_SD_40;
    static const long ID_MNU_SUPPRESSDUPLICATES;
    static const long ID_E131_Sync;
    static const long ID_MNU_FORCEIP;
    static const long ID_MNU_DEFAULTMODELBLENDOFF;
    static const long ID_MNU_SNAP_TO_TIMING;
    static const long ID_MENUITEM21;
    static const long ID_MENUITEM22;
    static const long ID_MENUITEM1;
    static const long ID_MNU_MANUAL;
    static const long ID_MNU_ZOOM;
    static const long ID_MNU_KEYBINDINGS;
    static const long idMenuHelpContent;
    static const long ID_MENU_HELP_FORMUM;
    static const long ID_MNU_VIDEOS;
    static const long ID_MENU_HELP_DOWNLOAD;
    static const long ID_MNU_HELP_RELEASE_NOTES;
    static const long ID_MENU_HELP_ISSUE;
    static const long ID_MENU_HELP_FACEBOOK;
    static const long ID_MNU_DONATE;
    static const long ID_MNU_UPDATE;
    static const long ID_TIMER1;
    static const long ID_TIMER2;
    static const long ID_TIMER_EFFECT_SETTINGS;
    //*)

    static const long ID_PANEL_EFFECTS1;
    static const long ID_PANEL_EFFECTS;
    static const long ID_NOTEBOOK_EFFECTS;

    static const long ID_XFADESOCKET;
    static const long ID_XFADESERVER;

    static const long ID_PLAYER_DIALOG;
    static const long ID_DELETE_EFFECT;
    static const long ID_IGNORE_CLICK;
    static const long ID_PROTECT_EFFECT;
    static const long ID_UNPROTECT_EFFECT;
    static const long ID_RANDOM_EFFECT;
    static const long ID_COPYROW_EFFECT; //copy random effect across row -DJ
    static const long ID_CLEARROW_EFFECT; //clear all effects on this row -DJ

    //(*Declarations(xLightsFrame)
    wxAuiManager* MainAuiManager;
    wxAuiManager* m_mgr;
    wxAuiNotebook* Notebook1;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxButton* ButtonAddDDP;
    wxButton* ButtonAddDongle;
    wxButton* ButtonAddE131;
    wxButton* ButtonAddLOR;
    wxButton* ButtonAddNull;
    wxButton* ButtonArtNET;
    wxButton* ButtonNetworkChange;
    wxButton* ButtonNetworkDelete;
    wxButton* ButtonNetworkDeleteAll;
    wxButton* ButtonOtherFolders;
    wxButton* ButtonSaveSetup;
    wxButton* Button_Discover;
    wxChoice* ChoiceParm1;
    wxChoice* ChoiceParm2;
    wxFlexGridSizer* GaugeSizer;
    wxGauge* ProgressBar;
    wxGridBagSizer* StatusBarSizer;
    wxListCtrl* GridNetwork;
    wxMenu* AudioMenu;
    wxMenu* AutoSaveMenu;
    wxMenu* GridSpacingMenu;
    wxMenu* Menu1;
    wxMenu* Menu3;
    wxMenu* MenuFile;
    wxMenu* MenuItem15;
    wxMenu* MenuItem18;
    wxMenu* MenuItem1;
    wxMenu* MenuItem29;
    wxMenu* MenuItem53;
    wxMenu* MenuItem54;
    wxMenu* MenuItem60;
    wxMenu* MenuItem7;
    wxMenu* MenuItemPerspectives;
    wxMenu* MenuItemRenderMode;
    wxMenu* MenuItem_BackupPurge;
    wxMenu* MenuItem_EnableRenderCache;
    wxMenu* MenuSettings;
    wxMenu* MenuView;
    wxMenu* OpenGLMenu;
    wxMenu* ToolIconSizeMenu;
    wxMenuBar* MenuBar;
    wxMenuItem* ActionTestMenuItem;
    wxMenuItem* MenuItem16;
    wxMenuItem* MenuItem32;
    wxMenuItem* MenuItem33;
    wxMenuItem* MenuItem34;
    wxMenuItem* MenuItem35;
    wxMenuItem* MenuItem36;
    wxMenuItem* MenuItem37;
    wxMenuItem* MenuItem38;
    wxMenuItem* MenuItem39;
    wxMenuItem* MenuItem3;
    wxMenuItem* MenuItem49;
    wxMenuItem* MenuItem50;
    wxMenuItem* MenuItem51;
    wxMenuItem* MenuItem52;
    wxMenuItem* MenuItemBackup;
    wxMenuItem* MenuItemCheckSequence;
    wxMenuItem* MenuItemColorManager;
    wxMenuItem* MenuItemConvert;
    wxMenuItem* MenuItemEffectAssistAlwaysOff;
    wxMenuItem* MenuItemEffectAssistAlwaysOn;
    wxMenuItem* MenuItemEffectAssistToggleMode;
    wxMenuItem* MenuItemEffectAssistWindow;
    wxMenuItem* MenuItemFSEQV1;
    wxMenuItem* MenuItemFSEQV2;
    wxMenuItem* MenuItemGridIconBackgroundOff;
    wxMenuItem* MenuItemGridIconBackgroundOn;
    wxMenuItem* MenuItemGridNodeValuesOff;
    wxMenuItem* MenuItemGridNodeValuesOn;
    wxMenuItem* MenuItemLoadEditPerspective;
    wxMenuItem* MenuItemRenderCanvasMode;
    wxMenuItem* MenuItemRenderEraseMode;
    wxMenuItem* MenuItemSelectEffect;
    wxMenuItem* MenuItemShiftEffects;
    wxMenuItem* MenuItemShiftSelectedEffects;
    wxMenuItem* MenuItemTimingEditMode;
    wxMenuItem* MenuItemTimingPlayMode;
    wxMenuItem* MenuItemUserDict;
    wxMenuItem* MenuItemViewSaveAsPerspective;
    wxMenuItem* MenuItemViewSavePerspective;
    wxMenuItem* MenuItem_ACLIghts;
    wxMenuItem* MenuItem_AutoShowHousePreview;
    wxMenuItem* MenuItem_BackupOnLaunch;
    wxMenuItem* MenuItem_BackupSubfolders;
    wxMenuItem* MenuItem_BkpPMonth;
    wxMenuItem* MenuItem_BkpPNever;
    wxMenuItem* MenuItem_BkpPQuarter;
    wxMenuItem* MenuItem_BkpPWeek;
    wxMenuItem* MenuItem_BkpPYear;
    wxMenuItem* MenuItem_CleanupFileLocations;
    wxMenuItem* MenuItem_CrashXLights;
    wxMenuItem* MenuItem_Donate;
    wxMenuItem* MenuItem_DownloadSequences;
    wxMenuItem* MenuItem_ExcludeAudioPackagedSequence;
    wxMenuItem* MenuItem_ExcludePresetsFromPackagedSequences;
    wxMenuItem* MenuItem_ExportEffects;
    wxMenuItem* MenuItem_FPP_Connect;
    wxMenuItem* MenuItem_File_Close_Sequence;
    wxMenuItem* MenuItem_File_Export_Video;
    wxMenuItem* MenuItem_File_Open_Sequence;
    wxMenuItem* MenuItem_File_Save;
    wxMenuItem* MenuItem_File_SaveAs_Sequence;
    wxMenuItem* MenuItem_ForceLocalIP;
    wxMenuItem* MenuItem_Generate2DPath;
    wxMenuItem* MenuItem_GenerateLyrics;
    wxMenuItem* MenuItem_Help_Download;
    wxMenuItem* MenuItem_Help_Facebook;
    wxMenuItem* MenuItem_Help_Forum;
    wxMenuItem* MenuItem_Help_Isue_Tracker;
    wxMenuItem* MenuItem_Help_ReleaseNotes;
    wxMenuItem* MenuItem_ImportEffects;
    wxMenuItem* MenuItem_Jukebox;
    wxMenuItem* MenuItem_LogRenderState;
    wxMenuItem* MenuItem_LoudVol;
    wxMenuItem* MenuItem_MHS_ExtraLarge;
    wxMenuItem* MenuItem_MHS_Large;
    wxMenuItem* MenuItem_MHS_Normal;
    wxMenuItem* MenuItem_MedVol;
    wxMenuItem* MenuItem_ModelBlendDefaultOff;
    wxMenuItem* MenuItem_PackageSequence;
    wxMenuItem* MenuItem_PerspectiveAutosave;
    wxMenuItem* MenuItem_PlayControlsOnPreview;
    wxMenuItem* MenuItem_PrepareAudio;
    wxMenuItem* MenuItem_PurgeRenderCache;
    wxMenuItem* MenuItem_PurgeVendorCache;
    wxMenuItem* MenuItem_QuietVol;
    wxMenuItem* MenuItem_RC_Disable;
    wxMenuItem* MenuItem_RC_Enable;
    wxMenuItem* MenuItem_RC_LockedOnly;
    wxMenuItem* MenuItem_SD_10;
    wxMenuItem* MenuItem_SD_20;
    wxMenuItem* MenuItem_SD_40;
    wxMenuItem* MenuItem_SD_None;
    wxMenuItem* MenuItem_ShowACRamps;
    wxMenuItem* MenuItem_ShowKeyBindings;
    wxMenuItem* MenuItem_SmallWaveform;
    wxMenuItem* MenuItem_SnapToTimingMarks;
    wxMenuItem* MenuItem_SuppressFadeHints;
    wxMenuItem* MenuItem_Update;
    wxMenuItem* MenuItem_UserManual;
    wxMenuItem* MenuItem_VQuietVol;
    wxMenuItem* MenuItem_VideoTutorials;
    wxMenuItem* MenuItem_ViewLog;
    wxMenuItem* MenuItem_ViewZoomIn;
    wxMenuItem* MenuItem_ViewZoomOut;
    wxMenuItem* MenuItem_Zoom;
    wxMenuItem* MenuItem_e131sync;
    wxMenuItem* MenuItem_xFade_A;
    wxMenuItem* MenuItem_xFade_B;
    wxMenuItem* MenuItem_xFade_Disabled;
    wxMenuItem* MenuItem_xSchedule;
    wxMenuItem* Menu_GenerateCustomModel;
    wxMenuItem* Menu_Settings_Sequence;
    wxMenuItem* QuitMenuItem;
    wxMenuItem* mAltBackupMenuItem;
    wxMenuItem* mBackupOnSaveMenuItem;
    wxMenuItem* mExportModelsMenuItem;
    wxMenuItem* mRenderOnSaveMenuItem;
    wxMenuItem* mSaveFseqOnSaveMenuItem;
    wxPanel* AUIStatusBar;
    wxPanel* PanelPreview;
    wxPanel* PanelSequencer;
    wxPanel* PanelSetup;
    wxSpinCtrl* SpinCtrl_SyncUniverse;
    wxStaticText* FileNameText;
    wxStaticText* ShowDirectoryLabel;
    wxStaticText* StaticText5;
    wxStaticText* StatusText;
    wxTimer EffectSettingsTimer;
    wxTimer Timer_AutoSave;
    xLightsTimer Timer1;
    xlAuiToolBar* ACToolbar;
    xlAuiToolBar* EditToolBar;
    xlAuiToolBar* EffectsToolBar;
    xlAuiToolBar* MainToolBar;
    xlAuiToolBar* OutputToolBar;
    xlAuiToolBar* PlayToolBar;
    xlAuiToolBar* ViewToolBar;
    xlAuiToolBar* WindowMgmtToolbar;
    //*)

    AUIToolbarButtonWrapper *CheckBoxLightOutput;
    AUIToolbarButtonWrapper *ButtonPasteByTime;
    AUIToolbarButtonWrapper *ButtonPasteByCell;

    AUIToolbarButtonWrapper *Button_ACDisabled;
    AUIToolbarButtonWrapper *Button_ACSelect;
    AUIToolbarButtonWrapper *Button_ACOn;
    AUIToolbarButtonWrapper *Button_ACOff;
    AUIToolbarButtonWrapper *Button_ACShimmer;
    AUIToolbarButtonWrapper *Button_ACTwinkle;
    AUIToolbarButtonWrapper *Button_ACIntensity;
    AUIToolbarButtonWrapper *Button_ACRampUp;
    AUIToolbarButtonWrapper *Button_ACRampDown;
    AUIToolbarButtonWrapper *Button_ACRampUpDown;
    AUIToolbarButtonWrapper *Button_ACFill;
    AUIToolbarButtonWrapper *Button_ACCascade;
    AUIToolbarButtonWrapper *Button_ACForeground;
    AUIToolbarButtonWrapper *Button_ACBackground;

    wxBitmap pauseIcon;
    wxBitmap playIcon;
    bool previewLoaded = false;
    bool previewPlaying = false;
    wxFileName networkFile;
    wxArrayString mru;  // most recently used directories
    wxMenuItem* mru_MenuItem[MRU_LENGTH];
    OutputManager _outputManager;
    long DragRowIdx;
    wxListCtrl* DragListBox;
    bool UnsavedNetworkChanges = false;
    bool UnsavedPlaylistChanges = false;
    int mSavedChangeCount;
    int mLastAutosaveCount;
    wxDateTime starttime;
    ModelPreview* modelPreview = nullptr;
    EffectManager effectManager;
    int effGridPrevX;
    int effGridPrevY;
    bool _backupSubfolders;
    bool _excludePresetsFromPackagedSequences;
    bool _excludeAudioFromPackagedSequences;
    bool _showACLights;
    bool _showACRamps;
    wxString _enableRenderCache;
    bool _playControlsOnPreview;
    bool _autoShowHousePreview;
    bool _smallWaveform;
    bool _modelBlendDefaultOff;
    bool _snapToTimingMarks;
    bool _autoSavePerspecive;
    int _fseqVersion;
    int _xFadePort;
    bool _wasMaximised = false;
    wxSocketServer* _xFadeSocket = nullptr;

    void OnxFadeSocketEvent(wxSocketEvent & event);
    void OnxFadeServerEvent(wxSocketEvent & event);
    void StartxFadeListener();
    wxString ProcessXFadeMessage(wxString msg);
    void ShowACLights();
    void DoBackup(bool prompt = true, bool startup = false, bool forceallfiles = false);
    int DecodeBackupPurgeDays(std::string s);
    void DoBackupPurge();
    void DoAltBackup(bool prompt = true);
    bool EnableOutputs(bool ignoreCheck = false);
    void EnableNetworkChanges();
    void InitEffectsPanel(EffectsPanel* panel);
    void LogPerspective(const wxString& perspective) const;

    // setup
    void OnMenuMRU(wxCommandEvent& event);
    bool SetDir(const wxString& dirname);
    bool PromptForShowDirectory();
    void UpdateNetworkList(bool updateModels);
    long GetNetworkSelection() const;
    long GetLastNetworkSelection() const;
    int GetNetworkSelectedItemCount() const;
    void MoveNetworkRows(int toRow, bool reverse);
    void OnGridNetworkDragQuit(wxMouseEvent& event);
    void OnGridNetworkDragEnd(wxMouseEvent& event);
    void OnGridNetworkMove(wxMouseEvent& event);
    void OnGridNetworkScrollTimer(wxTimerEvent& event);
    void SetupDongle(Output* e, int after = -1);
    void SetupE131(Output* e, int after = -1);
    void SetupArtNet(Output* e, int after = -1);
    void SetupLOR(Output* e, int after = -1);
    void SetupDDP(Output* e, int after = -1);
    void SetupNullOutput(Output* e, int after = -1);
    bool SaveNetworksFile();
    void NetworkChange();
    std::list<int> GetSelectedOutputs(wxString& ip);
    void UploadFPPBridgeInput();
    void MultiControllerUpload();
    void UploadFPPBridgeOutput();
    void UploadFalconInput();
    void UploadFalconOutput();
    void UploadSanDevicesInput();
    void UploadSanDevicesOutput();
    void UploadJ1SYSOutput();
    void UploadESPixelStickOutput();
    void UploadPixlite16Output();
    void UploadFPPStringOuputs(const std::string &controllers);
	void PingController(Output* e);
	void UploadEasyLightsOutput();

    void VisualiseOutput(Output *e, wxWindow *parent = nullptr);
    void DeleteSelectedNetworks();
    void ActivateSelectedNetworks(bool active);
    void DeactivateUnusedNetworks();
    void ChangeSelectedNetwork();
    bool AllSelectedSupportIP();
    bool AllSelectedSupportChannels();
    void UpdateSelectedIPAddresses();
    void UpdateSelectedChannels();
    void UpdateSelectedDescriptions();
    void UpdateSelectedSuppressDuplicates(bool suppressDuplicates);

    void OnProgressBarDoubleClick(wxMouseEvent& event);
    std::list<RenderProgressInfo *>renderProgressInfo;
    std::queue<RenderEvent*> mainThreadRenderEvents;
    std::mutex renderEventLock;

    wxString mediaFilename;
    wxString showDirectory;
    wxString mediaDirectory;
    wxString fseqDirectory;
    wxString backupDirectory;
    SeqDataType SeqData;
    wxTimer _scrollTimer;

    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;
    long seekPoint;

    wxString mBackgroundImage;
    int mBackgroundBrightness;
    int mBackgroundAlpha;
    bool mScaleBackgroundImage = false;
    std::string mStoredLayoutGroup;
    int _suppressDuplicateFrames;
    bool _suspendAutoSave = false;

    // convert
public:
    bool UnsavedRgbEffectsChanges;
    unsigned int modelsChangeCount;
    bool _renderMode;

    void SuspendAutoSave(bool dosuspend) { _suspendAutoSave = dosuspend; }
    void ClearLastPeriod();
    void WriteVirFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf); //       Vixen *.vir
    void WriteHLSFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      HLS *.hlsnc
    void WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf, int ver, int cpn);  //      LOR *.lcb
    void WriteLSPFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf, int cpn);  //      LSP UserPatterns.xml
    void ReadXlightsFile(const wxString& FileName, wxString *mediaFilename = nullptr);
    void ReadFalconFile(const wxString& FileName, ConvertDialog* convertdlg);
    void WriteFalconPiFile(const wxString& filename); //  Falcon Pi Player *.pseq
    OutputManager* GetOutputManager() { return &_outputManager; };

private:

    void WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
                                SeqDataType *dataBuf, int startAddr, int modelSize,
                                bool v2 = false); //Falcon Pi sub sequence .eseq
    void WriteVideoModelFile(const wxString& filename, long numChans, long numPeriods,
        SeqDataType *dataBuf, int startAddr, int modelSize, Model* model, bool compressed); //.avi file
    void WriteMinleonNECModelFile(const wxString& filename, long numChans, long numPeriods,
        SeqDataType *dataBuf, int startAddr, int modelSize, Model* model); //.bin file


    void OnNetworkPopup(wxCommandEvent &event);

    // sequence
    void LoadEffectsFile();
    wxString LoadEffectsFileNoCheck();
    void CreateDefaultEffectsXml();
    void TimerRgbSeq(long msec);
    void SetChoicebook(wxChoicebook* cb, const wxString& PageName);
    wxString GetXmlSetting(const wxString& settingName,const wxString& defaultValue) const;
    void SetPanelSequencerLabel(const std::string& sequence);

    void DisplayXlightsFilename(const wxString& filename) const;
    int ChooseRandomEffect();

public:
    bool InitPixelBuffer(const std::string &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased = false);
    Model *GetModel(const std::string& name) const;
    void RenderGridToSeqData(std::function<void()>&& callback);
    bool AbortRender();
    std::string GetSelectedLayoutPanelPreview() const;
    void UpdateRenderStatus();
    void LogRenderStatus();
    bool RenderEffectFromMap(Effect *effect, int layer, int period, SettingsMap& SettingsMap,
                             PixelBufferClass &buffer, bool &ResetEffectState,
                             bool bgThread = false, RenderEvent *event = nullptr);
    void RenderMainThreadEffects();
    void RenderEffectOnMainThread(RenderEvent *evt);
    void RenderEffectForModel(const std::string &model, int startms, int endms, bool clear = false);
    void RenderDirtyModels();
    void RenderTimeSlice(int startms, int endms, bool clear);
    void Render(const std::list<Model*> models,
                const std::list<Model *> &restrictToModels,
                int startFrame, int endFrame,
                bool progressDialog, bool clear,
                std::function<void()>&& callback);
    void BuildRenderTree();

    void RenderRange(RenderCommandEvent &cmd);
    void RenderDone();
    bool IsDrawRamps();

    void EnableSequenceControls(bool enable);
    SequenceElements& GetSequenceElements() { return mSequenceElements; }
    TimingElement* AddTimingElement(const std::string& name);
    void DeleteTimingElement(const std::string& name);
    void RenameTimingElement(const std::string& old_name, const std::string& new_name);
    void ImportTimingElement();
    void ExecuteImportTimingElement(wxCommandEvent &command);
    void ExecuteImportNotes(wxCommandEvent &command);
    void ConvertDataRowToEffects(wxCommandEvent &command);
    void DoConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime);
    void PromoteEffects(wxCommandEvent &command);
    void DoPromoteEffects(ModelElement *element);
    wxXmlNode* CreateEffectNode(wxString& name);
    void UpdateEffectNode(wxXmlNode* node);
    void ApplyEffectsPreset(wxString& data, const wxString &pasteDataVersion);
    void ApplyEffectsPreset(const std::string& presetName);
    void RenameModelInViews(const std::string old_name, const std::string& new_name);
    bool RenameModel(const std::string old_name, const std::string& new_name);
    bool RenameObject(const std::string old_name, const std::string& new_name);
    bool EnsureSequenceElementsAreOrderedCorrectly(const std::string ModelName, std::vector<std::string>& submodelOrder);
    void UpdateSequenceLength();

    void SetSequenceEnd(int ms);
    void UpdateRenderMode();
    void SetFrequency(int frequency);
    void RenderAll();

    void SetXmlSetting(const wxString& settingName,const wxString& value);
    unsigned int GetMaxNumChannels();

    bool GetSnapToTimingMarks() const { return _snapToTimingMarks; }

    void UpdateSequenceVideoPanel( const wxString& path );

protected:
    bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
    bool SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels);
    void ResetEffectsXml();
    void SeqLoadXlightsXSEQ(const wxString& filename);
    std::string CreateEffectStringRandom(std::string &settings, std::string &palette);
    bool CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, bool forceallfiles, std::string& errors);
    void BackupDirectory(wxString sourceDir, wxString targetDirName, wxString lastCreatedDirectory, bool forceallfiles, std::string& errors);
    void CreateMissingDirectories(wxString targetDirName, wxString lastCreatedDirectory, std::string& errors);
    void OpenRenderAndSaveSequences(const wxArrayString &filenames, bool exitOnDone);
    void AddAllModelsToSequence();
    void ShowPreviewTime(long ElapsedMSec);
    void PreviewOutput(int period);
    void TimerOutput(int period);
    void UpdateChannelNames();
    void StopNow();
    bool ShowFolderIsInBackup(const std::string showdir);

    bool Grid1HasFocus; //cut/copy/paste handled differently with grid vs. other text controls -DJ
    wxXmlDocument EffectsXml;
	SequenceViewManager _sequenceViewManager;
    wxXmlNode* EffectsNode = nullptr;
    wxXmlNode* PalettesNode = nullptr;
    wxXmlNode* PerspectivesNode = nullptr;
public:
    bool RebuildControllerConfig(OutputManager* outputManager, ModelManager* modelManager);
    wxXmlNode* ModelsNode = nullptr;
    wxXmlNode* ModelGroupsNode = nullptr;
    wxXmlNode* LayoutGroupsNode = nullptr;
    wxXmlNode* ViewObjectsNode = nullptr;
    SequenceViewManager* GetViewsManager() { return &_sequenceViewManager; }
    void OpenSequence(wxString passed_filename, ConvertLogDialog* plog);
    void SaveSequence();
    bool CloseSequence();
    void NewSequence();
    void SaveAsSequence();
    void SetPasteByCell();
    void SetPasteByTime();
    void ShowSequenceSettings();
    bool HandleAllKeyBinding(wxKeyEvent& event);
    int GetModelHandleScale() const { return _modelHandleSize; }
    bool IsSuppressFadeHints() const { return mSuppressFadeHints; }

private:
    wxXmlNode* SettingsNode = nullptr;

    bool MixTypeChanged;
    bool FadesChanged;
    long SeqBaseChannel;
    bool SeqChanCtrlBasic;
    bool SeqChanCtrlColor;
	bool mLoopAudio;
    bool _setupChanged; // set to true if something changes on the setup tab which would require the layout tab to have the model start channels recalculated

    bool mResetToolbars;
    bool mRenderOnSave;
    bool mBackupOnSave;
    bool mBackupOnLaunch;
    bool me131Sync;
    bool mSuppressFadeHints = false;
    wxString mLocalIP;
    wxString mAltBackupDir;
    int mIconSize;
    int mGridSpacing;
    bool mGridIconBackgrounds;
    bool mTimingPlayOnDClick;
    bool mGridNodeValues;
    int mEffectAssistMode;
	bool mRendering;
    bool mSaveFseqOnSave;
    int _modelHandleSize = 1;

    class RenderTree {
    public:
        RenderTree() : renderTreeChangeCount(0) {}
        ~RenderTree() { Clear(); }
        void Clear();
        void Add(Model *el);
        void Print();

        unsigned int renderTreeChangeCount;
        std::list<RenderTreeData*> data;
    } renderTree;
    int AutoSaveInterval;
    int BackupPurgeDays;
    JobPool jobPool;

    Model *playModel;
    int playType;
    int playStartMS;
    std::list<FPSEvent> fpsEvents;
    float _fps;
    int playStartTime;
    int playOffsetTime;
    int playEndTime;
    bool replaySection;
    double playSpeed;
    int playVolume;
    bool playAnimation;

    std::string selectedEffectName;
    std::string selectedEffectString;
    std::string selectedEffectPalette;
    Effect *selectedEffect = nullptr;

    std::string lastPlayEffect;
    double mPointSize = 2.0;

    int NextGridRowToPlay;
    int SeqPlayColumn;

	wxArrayString ChNames;

    int PlaybackPeriod; //used to be able to record the frame being played in an animation preview

public:
    //stuf used by layoutPanel for now
    void SetPreviewBackgroundScaled(bool scaled);
    void SetPreviewSize(int width, int height);
    void SetPreviewBackgroundImage(const wxString &filename);
    void SetDisplay2DBoundingBox(bool bb);
    bool GetDisplay2DBoundingBox() const;
    void SetDisplay2DCenter0(bool bb);
    bool GetDisplay2DCenter0() const;
    const wxString & GetDefaultPreviewBackgroundImage();
    bool GetDefaultPreviewBackgroundScaled();
    int GetDefaultPreviewBackgroundBrightness();
    int GetDefaultPreviewBackgroundAlpha();
    void SetPreviewBackgroundBrightness(int brightness, int alpha);
    void UpdatePreview();
    void UpdateModelsList();
    void RowHeadingsChanged( wxCommandEvent& event);
    void DoForceSequencerRefresh();
    void RefreshLayout();
    void RenderLayout();
    void AddPreviewOption(LayoutGroup* grp);
    void RemovePreviewOption(LayoutGroup* grp);
    ModelPreview* GetLayoutPreview() const {return modelPreview;}
    void SetStoredLayoutGroup(const std::string &group);
    const std::string& GetStoredLayoutGroup() const {return mStoredLayoutGroup;}
    bool IsACActive();
    void GetACSettings(ACTYPE& type, ACSTYLE& style, ACTOOL& tool, ACMODE& mode);
    int GetACIntensity();
    void GetACRampValues(int& a, int& b);
    void UpdateACToolbar(bool forceState = true); // if force state is false then it will force disable the AC toolbar
    void SetACSettings(ACTOOL tool);
    void SetACSettings(ACSTYLE style);
    void SetACSettings(ACMODE mode);
    void SetACSettings(ACTYPE type);
    bool IsPaneDocked(wxWindow* window) const;
    ModelPreview* GetHousePreview() const;

    void UnselectEffect();

private:

    int _acParm1Intensity;
    int _acParm1RampUp;
    int _acParm2RampUp;
    int _acParm1RampDown;
    int _acParm2RampDown;
    int _acParm1RampUpDown;
    int _acParm2RampUpDown;
    wxXmlNode* mCurrentPerpective = nullptr;
    std::map<wxString, bool> savedPaneShown;
    SequenceElements mSequenceElements;
    MainSequencer* mainSequencer = nullptr;
    ModelPreview * _modelPreviewPanel = nullptr;
    HousePreviewPanel *_housePreviewPanel = nullptr;
    LayoutPanel *layoutPanel = nullptr;
    EffectAssist* sEffectAssist = nullptr;
    ColorPanel* colorPanel = nullptr;
    TimingPanel* timingPanel = nullptr;
    PerspectivesPanel* perspectivePanel = nullptr;
    EffectIconPanel *effectPalettePanel = nullptr;
    JukeboxPanel *jukeboxPanel = nullptr;
    BufferPanel *bufferPanel = nullptr;
    ViewsModelsPanel *displayElementsPanel = nullptr;
    TopEffectsPanel* effectsPnl = nullptr;
    EffectsPanel* EffectsPanel1 = nullptr;
    SelectPanel *_selectPanel = nullptr;
    SequenceVideoPanel* sequenceVideoPanel = nullptr;
    int mMediaLengthMS;
    bool mSequencerInitialize = false;
    wxFlexGridSizer* FlexGridEffects = nullptr;
    std::set<int> LorTimingList; // contains a list of period numbers, set by ReadLorFile()
                                 //add lock/unlock/random state flags -DJ
                                 //these could be used to make fields read-only, but initially they are just used for partially random effects
                                 //#include <unordered_map>
    typedef enum { Normal, Locked, Random } EditState;
    std::unordered_map<std::string, EditState> buttonState;

    // Events
    void Zoom( wxCommandEvent& event);
    void Scroll( wxCommandEvent& event);
    void WindowResized( wxCommandEvent& event);
    void TimeSelected( wxCommandEvent& event);
    void MousePositionUpdated( wxCommandEvent& event);
    void SelectedEffectChanged( SelectedEffectChangedEvent& event);
    void SelectedRowChanged( wxCommandEvent& event);
    void EffectChanged( wxCommandEvent& event);
    void EffectUpdated( wxCommandEvent& event);
    void UnselectedEffect( wxCommandEvent& event);
    void EffectDroppedOnGrid(wxCommandEvent& event);
    void EffectFileDroppedOnGrid(wxCommandEvent& event);
    void PlayModelEffect(wxCommandEvent& event);
    void UpdateEffect(wxCommandEvent& event);
    void RandomizeEffect(wxCommandEvent &event);
    void UpdateEffectPalette(wxCommandEvent &event);
    void ForceSequencerRefresh(wxCommandEvent& event);
    void LoadPerspective(wxCommandEvent& event);
    void PerspectivesChanged(wxCommandEvent& event);
    void TurnOnOutputToLights(wxCommandEvent& event);
    void PlayJukeboxItem(wxCommandEvent& event);
    void PlayModel(wxCommandEvent& event);
    void CopyModelEffects(wxCommandEvent& event);
    void PasteModelEffects(wxCommandEvent& event);
    void ModelSelected(wxCommandEvent& event);
    void PlaySequence(wxCommandEvent& event);
    void PauseSequence(wxCommandEvent& event);
    void StopSequence(wxCommandEvent& event);
    void SequenceFirstFrame(wxCommandEvent& event);
    void SequenceLastFrame(wxCommandEvent& event);
    void SequenceRewind10(wxCommandEvent& event);
    void SequenceFForward10(wxCommandEvent& event);
    void SequenceSeekTo(wxCommandEvent& event);
    void SequenceReplaySection(wxCommandEvent& event);
    void TogglePlay(wxCommandEvent& event);
    void ExportModel(wxCommandEvent& event);
    void ShowDisplayElements(wxCommandEvent& event);
    void ShowHidePreviewWindow(wxCommandEvent& event);
    void ShowHideAllPreviewWindows(wxCommandEvent& event);

    bool isRandom_(wxControl* ctl, const char*debug);
    void SetSyncUniverse(int syncUniverse);

    void EnableToolbarButton(wxAuiToolBar* toolbar, int id, bool enable);
    void CheckForAndCreateDefaultPerpective();
    void ResizeAndMakeEffectsScroll();
    void ResizeMainSequencer();
    void LoadSequencer(xLightsXmlFile& xml_file);
    void DoLoadPerspective(wxXmlNode *p);
    void CheckForValidModels();
    void ExportModels(wxString filename);
    void ExportEffects(wxString filename);
    int ExportElement(wxFile& f, Element* e, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles);
    int ExportNodes(wxFile& f, StrandElement* e, NodeLayer* nl, int n, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles);
    std::map<int, std::list<float>> LoadPolyphonicTranscription(AudioManager* audio, int intervalMS);
    std::map<int, std::list<float>> LoadAudacityFile(std::string file, int intervalMS);
    std::map<int, std::list<float>> LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track);
    std::map<int, std::list<float>> LoadMusicXMLFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track);
    void CreateNotes(EffectLayer* el, std::map<int, std::list<float>>& notes, int interval, int frames);
    std::string CreateNotesLabel(const std::list<float>& notes) const;
    void CheckSequence(bool display);
    void CleanupRGBEffectsFileLocations();
    void CleanupSequenceFileLocations();
    void CheckElement(Element* e, wxFile& f, int& errcount, int& warncount, const std::string& name, const std::string& modelName, bool& videoCacheWarning, std::list<std::pair<std::string, std::string>>& faces, std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints, bool& usesShader);
    void CheckEffect(Effect* ef, wxFile& f, int& errcount, int& warncount, const std::string& name, const std::string& modelName, bool node, bool& videoCacheWarning, std::list<std::pair<std::string, std::string>>& faces, std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints);
    bool CheckStart(wxFile& f, const std::string& startmodel, std::list<std::string>& seen, std::string& nextmodel);
    void ShowHideSync();
    void ValidateWindow();
    void DoDonate();
    void AutoShowHouse();
    bool CheckForUpdate(bool force);
    void check32AppOn64Machine();
    void ShiftEffectsOnLayer(EffectLayer* el, int milliseconds);
    void ShiftSelectedEffectsOnLayer(EffectLayer* el, int milliseconds);
    void InitSequencer();
    void CreateSequencer();
    void DoStopSequence();

    wxMenu* MenuItemPreviews = nullptr;
    wxMenuItem* MenuItemPreviewSeparator = nullptr;
    static const long ID_MENU_ITEM_PREVIEWS;
    static const long ID_MENU_ITEM_PREVIEWS_SHOW_ALL;

    static const long ID_NETWORK_ADDUSB;
    static const long ID_NETWORK_ADDNULL;
    static const long ID_NETWORK_ADDE131;
    static const long ID_NETWORK_ADDARTNET;
    static const long ID_NETWORK_ADDLOR;
    static const long ID_NETWORK_ADDDDP;
    static const long ID_NETWORK_BEIPADDR;
    static const long ID_NETWORK_BECHANNELS;
    static const long ID_NETWORK_BEDESCRIPTION;
    static const long ID_NETWORK_BESUPPRESSDUPLICATES;
    static const long ID_NETWORK_BESUPPRESSDUPLICATESYES;
    static const long ID_NETWORK_BESUPPRESSDUPLICATESNO;
    static const long ID_NETWORK_ADD;
    static const long ID_NETWORK_BULKEDIT;
    static const long ID_NETWORK_DELETE;
    static const long ID_NETWORK_ACTIVATE;
    static const long ID_NETWORK_DEACTIVATE;
    static const long ID_NETWORK_DEACTIVATEUNUSED;
    static const long ID_NETWORK_OPENCONTROLLER;
    static const long ID_NETWORK_UPLOADCONTROLLER;
    static const long ID_NETWORK_UCOUTPUT;
    static const long ID_NETWORK_UCINPUT;
    static const long ID_NETWORK_MULTIUPLOAD;
    static const long ID_NETWORK_UCIFPPB;
    static const long ID_NETWORK_UCOFPPB;
    static const long ID_NETWORK_UCIFALCON;
    static const long ID_NETWORK_UCOFALCON;
    static const long ID_NETWORK_UCISANDEVICES;
    static const long ID_NETWORK_UCOSANDEVICES;
    static const long ID_NETWORK_UCOJ1SYS;
    static const long ID_NETWORK_UCOESPIXELSTICK;
    static const long ID_NETWORK_UCOPIXLITE16;
    static const long ID_NETWORK_PINGCONTROLLER;
	static const long ID_NETWORK_UCOEASYLIGHTS;
    static const long ID_NETWORK_UPLOAD_CONTROLLER_CONFIGURED;
    static const long ID_NETWORK_VISUALISE;

    #define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)

    DECLARE_EVENT_TABLE()
    friend class xLightsApp; //kludge: allow xLightsApp to call OnPaneNutcrackerChar -DJ

public:
    std::vector<Model *> PreviewModels;
    std::vector<LayoutGroup *> LayoutGroups;
    std::vector<ModelPreview *> PreviewWindows;
    ModelManager AllModels;
    ViewObjectManager AllObjects;
    ColorManager color_mgr;
    ViewpointMgr viewpoint_mgr;
    EffectTreeDialog *EffectTreeDlg = nullptr;

    void LoadJukebox(wxXmlNode* node);
    static wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false);
    TimingPanel* GetLayerBlendingPanel() const { return timingPanel; }

    int GetPlayStatus() const { return playType; }
    MainSequencer* GetMainSequencer() const { return mainSequencer; }
    wxString GetSeqXmlFileName();

    std::string MoveToShowFolder(const std::string& file, const std::string& subdirectory);
    bool IsInShowFolder(const std::string & file) const;
    bool FilesMatch(const std::string & file1, const std::string & file2) const;

    std::string GetEffectTextFromWindows(std::string &palette) const;

	void DoPlaySequence();
    void RecalcModels(bool force = false);
};
#endif // XLIGHTSMAIN_H
