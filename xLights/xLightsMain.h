#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef _MSC_VER
    #include <stdlib.h>

    //#define VISUALSTUDIO_MEMORYLEAKDETECTIO
    #ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
        #define _CRTDBG_MAP_ALLOC
        #include <crtdbg.h>
    #endif
#endif

// Every time this regenerates from code blocks you will need to remove wx/led.h

//(*Headers(xLightsFrame)
#include <wx/aui/aui.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/timer.h>
//*)

#include <wx/preferences.h>
#include <wx/choicebk.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/dir.h>
#include <wx/treebase.h>
#include <wx/socket.h>
#include <wx/listctrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/appprogress.h>

#include <unordered_map>
#include <map>
#include <set>
#include <vector>

#ifdef LINUX
#include <unistd.h>
#endif
#ifdef WINDOWS
#include <windows.h>
#endif

#include "../common/xlBaseApp.h"
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
#include "outputs/ZCPP.h"
#include "OutputModelManager.h"
#include "models/Model.h"
#include "SequencePackage.h"
#include "ScriptsDialog.h"
#include "TipOfTheDayDialog.h"

class wxDebugReport;

class ControllerCaps;
class EffectTreeDialog;
class ConvertDialog;
class ConvertLogDialog;
class RenderTreeData;
class HousePreviewPanel;
class SelectPanel;
class SearchPanel;
class SequenceVideoPanel;
class EffectIconPanel;
class ValueCurvesPanel;
class ColoursPanel;
class JukeboxPanel;
class FindDataPanel;
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
class ZCPPOutput;
class UDControllerPort;
class Model;
class ControllerEthernet;
class HttpServer;
class HttpConnection;
class HttpRequest;
class wxTaskBarIcon;

// max number of most recently used show directories on the File menu
#define MRUD_LENGTH 4
// max number of most recently used files on the File menu
#define MRUF_LENGTH 8

// notebook pages
#define SETUPTAB 0
#define LAYOUTTAB 1
#define NEWSEQUENCER 2

#define PLAY_TYPE_STOPPED 0
#define PLAY_TYPE_EFFECT 1
#define PLAY_TYPE_MODEL  2
#define PLAY_TYPE_EFFECT_PAUSED 3
#define PLAY_TYPE_MODEL_PAUSED  4

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
wxDECLARE_EVENT(EVT_CUT_MODEL_EFFECTS, wxCommandEvent);
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
wxDECLARE_EVENT(EVT_SHOW_SELECT_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDECLARE_EVENT(EVT_IMPORT_NOTES, wxCommandEvent);
wxDECLARE_EVENT(EVT_CONVERT_DATA_TO_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PROMOTE_EFFECTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_APPLYLAST, wxCommandEvent);
wxDECLARE_EVENT(EVT_RGBEFFECTS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TURNONOUTPUTTOLIGHTS, wxCommandEvent);
wxDECLARE_EVENT(EVT_PLAYJUKEBOXITEM, wxCommandEvent);
wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);
wxDECLARE_EVENT(EVT_COLOUR_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SETEFFECTCHOICE, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIPOFDAY_READY, wxCommandEvent);
wxDECLARE_EVENT(EVT_SET_EFFECT_DURATION, wxCommandEvent);

static const wxString xlights_base_name       = "xLights";
static const wxString strSupportedFileTypes = "LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|Vixen Sequences (*.vix)|*.vix|Glediator Record File (*.gled)|*.gled)|Lynx Conductor Sequences (*.seq)|*.seq|xLights/FPP Sequences(*.fseq)|*.fseq|xLights Imports(*.iseq)|*.iseq";
static const wxString strSequenceSaveAsFileTypes = "xLights Sequences(*.xsq)|*.xsq|Old xLights Sequences(*.xml)|*.xml";

#define PRESET_MODEL_NAME "PRESET_Matrix_XYZZY"

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
class wxLed;

class xlAuiToolBar : public wxAuiToolBar {
public:
    xlAuiToolBar(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxAUI_TB_DEFAULT_STYLE) : wxAuiToolBar(parent, id, pos, size, style)
    {
    }
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
    void SetBitmap(const wxBitmapBundle &bmp);
private:
    wxAuiToolBar *toolbar = nullptr;
    int id;
};

class xLightsFrame: public xlFrame
{
public:

    xLightsFrame(wxWindow* parent, int ab, wxWindowID id = -1, bool renderOnlyMode = false);
    virtual ~xLightsFrame();

    static bool IsCheckSequenceOptionDisabled(const std::string& option);
    static void SetCheckSequenceOptionDisable(const std::string& option, bool value);

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
    long SecondsRemaining = 0;
    long EndTimeSec = 0;
    int TxOverflowCnt = 0;
    int TxOverflowTotal = 0;
    std::mutex saveLock;
    RenderCache _renderCache;
    std::atomic_bool _exiting;
    #ifdef __WXMSW__
    // windows has issues if we create it later
    TipOfTheDayDialog _tod;
    #else
    TipOfTheDayDialog* _tod = nullptr;
    #endif

    PhonemeDictionary dictionary;

    bool IsExiting() const { return _exiting; }
    void SetEffectControls(const std::string &modelName, const std::string &name,
                           const SettingsMap &settings, const SettingsMap &palette,
                           int startTimeMs, int endTimeMs, bool setDefaults);
    void SetEffectControls(const SettingsMap &settings);
    void ApplyLast(wxCommandEvent& event);
    void SetEffectControlsApplyLast(const SettingsMap &settings);
    bool ApplySetting(wxString name, const wxString &value, int count = 0);
    void LoadPerspectivesMenu(wxXmlNode* perspectivesNode);
    struct PerspectiveId {
        int id = 0;
        wxXmlNode* p = nullptr;
    };

    PerspectiveId perspectives[10];
    void OnMenuItemLoadPerspectiveSelected(wxCommandEvent& event);
	bool SaveEffectsFile(bool backup = false);
    void SaveModelsFile();
    void MarkEffectsFileDirty();
    void MarkModelsAsNeedingRender();
    void CheckUnsavedChanges();
    void SetStatusText(const wxString &msg, int filename = 0) override;
    void SetStatusTextColor(const wxString &msg, const wxColor& colour);
	std::string GetChannelToControllerMapping(int32_t channel);
    void GetControllerDetailsForChannel(int32_t channel, std::string& controllername, std::string& type, std::string& protocol, std::string& description, int32_t& channeloffset, std::string &ip, std::string& u, std::string& inactive, std::string& baud, int& start_universe, int& start_universe_channel);

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

    wxString _userEmail;
    wxString _linkedSave = "None";
    wxString _linkedControllerUpload = "None";
    static wxString CurrentDir; //expose current folder name -DJ
    static wxString FseqDir; //expose current fseq name
    static wxString PlaybackMarker; //keep track of where we are within grid -DJ
    static wxString xlightsFilename; //expose current path name -DJ
    static xLightsXmlFile* CurrentSeqXmlFile; // global object for currently opened XML file
    const std::string &GetShowDirectory() const { return showDirectory; }
    const std::string &GetFseqDirectory() const { return fseqDirectory; }
    static wxString GetFilename() { return xlightsFilename; }
    void ConversionInit();
    void ConversionError(const wxString& msg);
    void SetMediaFilename(const wxString& filename);
    void RenderIseqData(bool bottom_layers, ConvertLogDialog* plog);
    bool IsSequenceDataValid() const
    { return _seqData.IsValidData(); }
    std::string GetPresetIconFilename(const std::string& preset) const;
    void CreatePresetIcons();
    void ClearSequenceData();
    void LoadAudioData(xLightsXmlFile& xml_file);
    virtual void CreateDebugReport(xlCrashHandler* crashHandler) override;
    virtual std::string GetCurrentDir() const override { return CurrentDir.ToStdString(); }
    wxString GetThreadStatusReport();
    void PushTraceContext();
    void PopTraceContext();
    void AddTraceMessage(const std::string &msg);
    void ClearTraceMessages();
    bool ExportVideoPreview(wxString const& path);

	void SetAudioControls();
    void ImportXLights(const wxFileName &filename, std::string const& mapFile = std::string());
    void ImportXLights(SequenceElements &se, const std::vector<Element *> &elements, const wxFileName &filename,
        bool modelBlendig = false, bool showModelBlending = false, bool allowAllModels = false, bool clearSrc = false);
    void ImportXLights(SequenceElements &se, const std::vector<Element *> &elements, SequencePackage &xsqPkg,
        bool modelBlendig = false, bool showModelBlending = false, bool allowAllModels = false, bool clearSrc = false, std::string const& mapFile = std::string());
    void ImportVix(const wxFileName &filename);
    void ImportHLS(const wxFileName &filename);
    void ImportLMS(const wxFileName &filename);
    void ImportLPE(const wxFileName &filename);
    void ImportS5(const wxFileName &filename);
    void ImportLSP(const wxFileName &filename);
    void ImportVsa(const wxFileName &filename);
    void ImportSuperStar(const wxFileName &filename);
    void CloneXLightsEffects(EffectLayer* target, EffectLayer* src, bool eraseExisting);
    bool CloneXLightsEffects(const std::string& target,
                             const std::string& source,
                             SequenceElements& seqEl,
                             bool eraseExisting);
    void SaveWorking();
    void SaveWorkingLayout();
    void PlayerError(const wxString& msg);
    void AskCloseSequence();
    void SaveCurrentTab();
    void DoWork(uint32_t work, const std::string& type, BaseObject* model = nullptr, const std::string& selected = "");
    void DoASAPWork();
    void DoSetupWork();
    void DoLayoutWork();
    bool DoAllWork();

    bool readOnlyMode = false;
    void SetReadOnlyMode(bool mode) {
    	readOnlyMode = mode;
        UpdateReadOnlyState();
	}
    bool IsReadOnlyMode() const {
		return readOnlyMode;
	}
    void UpdateReadOnlyState();

    EffectManager &GetEffectManager() { return effectManager; }

    bool ImportSuperStar(Element *el, wxXmlDocument &doc, int x_size, int y_size,
                         int x_offset, int y_offset,
                         int imageResizeType, const wxSize &modelSize, const wxString& layerBlend);
    bool ImportLMS(wxXmlDocument &doc, const wxFileName &filename);
    bool ImportLPE(wxXmlDocument &doc, const wxFileName &filename);
    bool ImportVixen3(const wxFileName &filename);
    bool ImportS5(wxXmlDocument &doc, const wxFileName &filename);

    void SuspendRender(bool suspend) { _suspendRender = suspend; }
    bool IsRenderSuspended() const { return _suspendRender; }
    void SetPlaySpeedTo(float speed);

    //(*Handlers(xLightsFrame)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnMenuOpenFolderSelected(wxCommandEvent& event);
    void OnOutputTimerTrigger(wxTimerEvent& event);
    void OnButtonNetworkMoveUpClick(wxCommandEvent& event);
    void OnButtonNetworkMoveDownClick(wxCommandEvent& event);
    void OnButtonSaveSetupClick(wxCommandEvent& event);
    void OnBitmapButtonTabInfoClick(wxCommandEvent& event);
    void OnCheckBoxLightOutputClick(wxCommandEvent& event);
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
    void OnMenuItem_File_NewXLightsInstance(wxCommandEvent& event);
    void OnAuiToolBarFirstFrameClick(wxCommandEvent& event);
    void OnAuiToolBarLastFrameClick(wxCommandEvent& event);
    void OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event);
    void ShowHideEffectSettingsWindow(wxCommandEvent& event);
    void ShowHideColorWindow(wxCommandEvent& event);
    void ShowHideLayerTimingWindow(wxCommandEvent& event);
    void ShowHideEffectDropper(wxCommandEvent& event);
    void ResetToolbarLocations(wxCommandEvent& event);
    void OnMenuItemImportEffects(wxCommandEvent& event);
    void SetPlaySpeed(wxCommandEvent& event);
    void OnNotebook1PageChanging(wxAuiNotebookEvent& event);
    void ShowHidePerspectivesWindow(wxCommandEvent& event);
    void ShowHideDisplayElementsWindow(wxCommandEvent& event);
    void ShowHideEffectAssistWindow(wxCommandEvent& event);
    void OnMenuItem_File_SaveAs_SequenceSelected(wxCommandEvent& event);
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
    void OnTimer_AutoSaveTrigger(wxTimerEvent& event);
    void OnEffectSettingsTimerTrigger(wxTimerEvent& event);
    void OnRenderStatusTimerTrigger(wxTimerEvent& event);
    void OnMenuItemViewSaveAsPerspectiveSelected(wxCommandEvent& event);
    void OnmAltBackupMenuItemSelected(wxCommandEvent& event);
    void OnmExportModelsMenuItemSelected(wxCommandEvent& event);
    void OnMenuItem_ViewLogSelected(wxCommandEvent& event);
    void OnMenuItemCheckSequenceSelected(wxCommandEvent& event);
    void OnMenuItem_Help_ForumSelected(wxCommandEvent& event);
    void OnMenuItem_Help_DownloadSelected(wxCommandEvent& event);
    void OnMenuItem_Help_ReleaseNotesSelected(wxCommandEvent& event);
    void OnMenuItem_Help_Isue_TrackerSelected(wxCommandEvent& event);
    void OnMenuItem_Help_FacebookSelected(wxCommandEvent& event);
    void OnMenuItem_ExportEffectsSelected(wxCommandEvent& event);
    void OnMenuItem_FPP_ConnectSelected(wxCommandEvent& event);
    void OnMenuItemShiftEffectsSelected(wxCommandEvent& event);
    void OnMenuItem_PackageSequenceSelected(wxCommandEvent& event);
    void OnMenuItem_xScheduleSelected(wxCommandEvent& event);
    void OnMenuItem_xCaptureSelected(wxCommandEvent& event);
    void OnMenuItem_VideoTutorialsSelected(wxCommandEvent& event);
    void OnMenuItem_DonateSelected(wxCommandEvent& event);
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
    void OnMenuItem_GenerateLyricsSelected(wxCommandEvent& event);
    void OnMenuItem_CrashXLightsSelected(wxCommandEvent& event);
    void OnMenuItemBatchRenderSelected(wxCommandEvent& event);
    void OnMenuItem_UpdateSelected(wxCommandEvent& event);
    void OnMenuItem_LogRenderStateSelected(wxCommandEvent& event);
    void OnMenuItem_File_Save_Selected(wxCommandEvent& event);
    void OnMenuItem_PurgeVendorCacheSelected(wxCommandEvent& event);
    void OnMenuItem_LoudVolSelected(wxCommandEvent& event);
    void OnMenuItem_MedVolSelected(wxCommandEvent& event);
    void OnMenuItem_QuietVolSelected(wxCommandEvent& event);
    void OnMenuItem_VQuietVolSelected(wxCommandEvent& event);
    void TogglePresetsPanel();
    void ShowPresetsPanel();
    uint64_t BadDriveAccess(const std::list<std::string>& files, std::list<std::pair<std::string, uint64_t>>& slow, uint64_t thresholdUS);
    void OnMenuItemSelectEffectSelected(wxCommandEvent& event);
    void OnMenuItemShowHideVideoPreview(wxCommandEvent& event);
    void OnMenuItem_DownloadSequencesSelected(wxCommandEvent& event);
    void OnMenuItem_JukeboxSelected(wxCommandEvent& event);
    void OnMenuItemShiftSelectedEffectsSelected(wxCommandEvent& event);
    void OnMenuItemUserDictSelected(wxCommandEvent& event);
    void OnMenuItem_PurgeRenderCacheSelected(wxCommandEvent& event);
    void OnMenuItem_ShowKeyBindingsSelected(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnMenuItem_ZoomSelected(wxCommandEvent& event);
    void OnMenuItem_CleanupFileLocationsSelected(wxCommandEvent& event);
    void OnMenuItem_Generate2DPathSelected(wxCommandEvent& event);
    void OnMenuItem_PrepareAudioSelected(wxCommandEvent& event);
    void OnMenuItem_UserManualSelected(wxCommandEvent& event);
    void OnMenuItem_ValueCurvesSelected(wxCommandEvent& event);
    void OnMenuItem_ColourDropperSelected(wxCommandEvent& event);
    void OnMenuItemHinksPixExportSelected(wxCommandEvent& event);
    void OnMenuItemPreferencesSelected(wxCommandEvent& event);
    void OnButtonDiscoverClick(wxCommandEvent& event);
    void OnButtonDeleteAllControllersClick(wxCommandEvent& event);
    void OnButtonVisualiseClick(wxCommandEvent& event);
    void OnButtonUploadInputClick(wxCommandEvent& event);
    void OnButtonUploadOutputClick(wxCommandEvent& event);
    void OnButtonOpenClick(wxCommandEvent& event);
    void OnButtonControllerDeleteClick(wxCommandEvent& event);
    void OnMenuItemBulkControllerUploadSelected(wxCommandEvent& event);
    void OnButtonAddControllerSerialClick(wxCommandEvent& event);
    void OnButtonAddControllerEthernetClick(wxCommandEvent& event);
    void OnButtonAddControllerNullClick(wxCommandEvent& event);
    void OnMenuItem_KeyBindingsSelected(wxCommandEvent& event);
    void OnButton_ChangeShowFolderTemporarily(wxCommandEvent& event);
    void OnSysColourChanged(wxSysColourChangedEvent& event);
    void OnMenuItem_ExportControllerConnectionsSelected(wxCommandEvent& event);
    void OnMenuItem_xScannerSelected(wxCommandEvent& event);
    void OnButton_OpenProxyClick(wxCommandEvent& event);
    void OnMenuItemRunScriptSelected(wxCommandEvent& event);
    void OnButton_ChangeTemporarilyAgainClick(wxCommandEvent& event);
    void OnMenuItem_ColorReplaceSelected(wxCommandEvent& event);
    void OnMenuItemFindDataSelected(wxCommandEvent& event);
    void OnMenuItemSearchEffectsSelected(wxCommandEvent& event);
    void OnMenuItem_SilentVolSelected(wxCommandEvent& event);
    void OnMenuItem_TODSelected(wxCommandEvent& event);
    void OnMenuItem_RemapCustomSelected(wxCommandEvent& event);
    void OnMenuItemRestoreBackupSelected(wxCommandEvent& event);
    void OnMenuItem_SuppressDock(wxCommandEvent& event);
    void OnButton_ChangeBaseShowDirClick(wxCommandEvent& event);
    void OnButton_ClearBaseShowDirClick(wxCommandEvent& event);
    void OnCheckBox_AutoUpdateBaseClick(wxCommandEvent& event);
    void OnButton_UpdateBaseClick(wxCommandEvent& event);
    void ShowHideSelectEffectsWindow(wxCommandEvent& event);
    bool OnButton_OpenBaseShowDirClick(wxCommandEvent& event);
    void OnButtonFPPConnectClick(wxCommandEvent& event);
    //*)
    void OnCharHook(wxKeyEvent& event);
    void OnHelp(wxHelpEvent& event);

private :

    //void OnMenuItem53Selected(wxCommandEvent& event);

    void DoMenuAction(wxMenuEvent &evt);
	void ShowHideAllSequencerWindows(bool show);
	void ResetAllSequencerWindows();
	void SetEffectAssistWindowState(bool show);
    void UpdateEffectAssistWindow(Effect* effect, RenderableEffect* ren_effect);
    void AddDebugFilesToReport(wxDebugReport &report);

public:

    //(*Identifiers(xLightsFrame)
    static const wxWindowID ID_AUITOOLBAR_OPENSHOW;
    static const wxWindowID ID_AUITOOLBAR_NEWSEQUENCE;
    static const wxWindowID ID_AUITOOLBAR_OPEN;
    static const wxWindowID ID_AUITOOLBAR_SAVE;
    static const wxWindowID ID_AUITOOLBAR_SAVEAS;
    static const wxWindowID ID_AUITOOLBAR_RENDERALL;
    static const wxWindowID ID_AUITOOLBAR_MAIN;
    static const wxWindowID ID_AUITOOLBAR_PLAY_NOW;
    static const wxWindowID ID_AUITOOLBAR_PAUSE;
    static const wxWindowID ID_AUITOOLBAR_STOP;
    static const wxWindowID ID_AUITOOLBAR_FIRST_FRAME;
    static const wxWindowID ID_AUITOOLBAR_LAST_FRAME;
    static const wxWindowID ID_AUITOOLBAR_REPLAY_SECTION;
    static const wxWindowID ID_CHECKBOX_LIGHT_OUTPUT;
    static const wxWindowID ID_AUITOOLBAR_PLAY;
    static const wxWindowID ID_AUITOOLBARITEM2;
    static const wxWindowID ID_AUITOOLBARITEM5;
    static const wxWindowID ID_AUITOOLBARITEM7;
    static const wxWindowID ID_AUITOOLBARITEM3;
    static const wxWindowID ID_TOGGLE_MODEL_PREVIEW;
    static const wxWindowID ID_TOGGLE_HOUSE_PREVIEW;
    static const wxWindowID ID_AUITOOLBARITEM6;
    static const wxWindowID ID_AUITOOLBARITEM8;
    static const wxWindowID ID_AUITOOLBARITEM9;
    static const wxWindowID ID_AUITOOLBARITEM10;
    static const wxWindowID ID_AUIWINDOWTOOLBAR;
    static const wxWindowID ID_PASTE_BY_TIME;
    static const wxWindowID ID_PASTE_BY_CELL;
    static const wxWindowID ID_AUITOOLBAR_EDIT;
    static const wxWindowID ID_AUITOOLBARITEM_ACDISABLED;
    static const wxWindowID ID_AUITOOLBARITEM_ACSELECT;
    static const wxWindowID ID_AUITOOLBARITEM_ACOFF;
    static const wxWindowID ID_AUITOOLBARITEM_ACON;
    static const wxWindowID ID_AUITOOLBARITEM_ACSHIMMER;
    static const wxWindowID ID_AUITOOLBARITEM_ACTWINKLE;
    static const wxWindowID ID_AUITOOLBARITEM_ACINTENSITY;
    static const wxWindowID ID_AUITOOLBARITEM_ACRAMPUP;
    static const wxWindowID ID_AUITOOLBARITEM_ACRAMPDOWN;
    static const wxWindowID ID_AUITOOLBARITEM_ACRAMPUPDOWN;
    static const wxWindowID ID_CHOICE_PARM1;
    static const wxWindowID ID_CHOICE_PARM2;
    static const wxWindowID ID_AUITOOLBARITEM_ACFILL;
    static const wxWindowID ID_AUITOOLBARITEM_ACCASCADE;
    static const wxWindowID ID_AUITOOLBARITEM_ACFOREGROUND;
    static const wxWindowID ID_AUITOOLBARITEM_ACBACKGROUND;
    static const wxWindowID ID_AUITOOLBAR_AC;
    static const wxWindowID ID_AUITOOLBARITEM14;
    static const wxWindowID ID_AUITOOLBAR_VIEW;
    static const wxWindowID ID_AUIEFFECTSTOOLBAR;
    static const wxWindowID ID_BUTTON3;
    static const wxWindowID ID_BUTTON11;
    static const wxWindowID ID_BUTTON13;
    static const wxWindowID ID_STATICTEXT4;
    static const wxWindowID ID_STATICTEXT2;
    static const wxWindowID ID_BUTTON14;
    static const wxWindowID ID_BUTTON17;
    static const wxWindowID ID_BUTTON15;
    static const wxWindowID ID_STATICTEXT3;
    static const wxWindowID ID_CHECKBOX1;
    static const wxWindowID ID_BUTTON16;
    static const wxWindowID ID_BUTTON_SAVE_SETUP;
    static const wxWindowID ID_BUTTON9;
    static const wxWindowID ID_BUTTON6;
    static const wxWindowID ID_BUTTON10;
    static const wxWindowID ID_BUTTON5;
    static const wxWindowID ID_BUTTON18;
    static const wxWindowID ID_BITMAPBUTTON1;
    static const wxWindowID ID_BITMAPBUTTON2;
    static const wxWindowID ID_PANEL2;
    static const wxWindowID ID_BUTTON1;
    static const wxWindowID ID_BUTTON2;
    static const wxWindowID ID_BUTTON7;
    static const wxWindowID ID_STATICTEXT1;
    static const wxWindowID ID_BUTTON8;
    static const wxWindowID ID_BUTTON4;
    static const wxWindowID ID_BUTTON12;
    static const wxWindowID ID_PANEL3;
    static const wxWindowID ID_PANEL_SETUP;
    static const wxWindowID ID_PANEL_PREVIEW;
    static const wxWindowID XLIGHTS_SEQUENCER_TAB;
    static const wxWindowID ID_NOTEBOOK1;
    static const wxWindowID ID_STATICTEXT6;
    static const wxWindowID ID_GAUGE1;
    static const wxWindowID ID_PANEL5;
    static const wxWindowID ID_STATICTEXT7;
    static const wxWindowID ID_PANEL1;
    static const wxWindowID ID_NEW_SEQUENCE;
    static const wxWindowID ID_OPEN_SEQUENCE;
    static const wxWindowID ID_MENUITEM4;
    static const wxWindowID ID_MENUITEM_OPENRECENTSEQUENCE;
    static const wxWindowID IS_SAVE_SEQ;
    static const wxWindowID ID_SAVE_AS_SEQUENCE;
    static const wxWindowID ID_CLOSE_SEQ;
    static const wxWindowID ID_SEQ_SETTINGS;
    static const wxWindowID ID_MNU_KEYBINDINGS;
    static const wxWindowID ID_EXPORT_VIDEO;
    static const wxWindowID ID_MENUITEM2;
    static const wxWindowID ID_MENUITEM8;
    static const wxWindowID ID_MENUITEM_RECENTFOLDERS;
    static const wxWindowID ID_FILE_BACKUP;
    static const wxWindowID ID_FILE_RESTOREBACKUP;
    static const wxWindowID ID_FILE_ALTBACKUP;
    static const wxWindowID ID_SHIFT_EFFECTS;
    static const wxWindowID ID_MNU_SHIFT_SELECTED_EFFECTS;
    static const wxWindowID ID_MNU_COLOURREPLACE;
    static const wxWindowID ID_MENUITEM13;
    static const wxWindowID ID_MNU_CHECKSEQ;
    static const wxWindowID ID_MNU_CLEANUPFILE;
    static const wxWindowID ID_MNU_PACKAGESEQUENCE;
    static const wxWindowID ID_MNU_DOWNLOADSEQUENCES;
    static const wxWindowID ID_MENU_BATCH_RENDER;
    static const wxWindowID ID_MENU_FPP_CONNECT;
    static const wxWindowID ID_MNU_BULKUPLOAD;
    static const wxWindowID ID_MENU_HINKSPIX_EXPORT;
    static const wxWindowID ID_MENU_RUN_SCRIPT;
    static const wxWindowID ID_EXPORT_MODELS;
    static const wxWindowID ID_MNU_EXPORT_EFFECTS;
    static const wxWindowID ID_MNU_EXPORT_CONTROLLER_CONNECTIONS;
    static const wxWindowID ID_MENU_VIEW_LOG;
    static const wxWindowID ID_MENUITEM18;
    static const wxWindowID iD_MNU_VENDORCACHEPURGE;
    static const wxWindowID ID_MNU_PURGERENDERCACHE;
    static const wxWindowID ID_MNU_CRASH;
    static const wxWindowID ID_MNU_DUMPRENDERSTATE;
    static const wxWindowID ID_MENU_GENERATE2DPATH;
    static const wxWindowID ID_MENUITEM_GenerateCustomModel;
    static const wxWindowID ID_MNU_REMAPCUSTOM;
    static const wxWindowID ID_MNU_GENERATELYRICS;
    static const wxWindowID ID_MENUITEM_CONVERT;
    static const wxWindowID ID_MNU_PREPAREAUDIO;
    static const wxWindowID ID_MENU_USER_DICT;
    static const wxWindowID ID_MNU_XSCHEDULE;
    static const wxWindowID ID_MENU_XCAPTURE;
    static const wxWindowID ID_MNU_XSCANNER;
    static const wxWindowID ID_MENUITEM5;
    static const wxWindowID MNU_ID_ACLIGHTS;
    static const wxWindowID ID_MNU_SHOWRAMPS;
    static const wxWindowID ID_MENUITEM_SAVE_PERSPECTIVE;
    static const wxWindowID ID_MENUITEM_SAVE_AS_PERSPECTIVE;
    static const wxWindowID ID_MENUITEM_LOAD_PERSPECTIVE;
    static const wxWindowID ID_MNU_PERSPECTIVES_AUTOSAVE;
    static const wxWindowID ID_MENUITEM7;
    static const wxWindowID ID_MENUITEM_DISPLAY_ELEMENTS;
    static const wxWindowID ID_MENU_TOGGLE_MODEL_PREVIEW;
    static const wxWindowID ID_MENU_TOGGLE_HOUSE_PREVIEW;
    static const wxWindowID ID_MENUITEM14;
    static const wxWindowID ID_MENUITEM15;
    static const wxWindowID ID_MENUITEM16;
    static const wxWindowID ID_MENUITEM9;
    static const wxWindowID ID_MENUITEM17;
    static const wxWindowID ID_MNU_VALUECURVES;
    static const wxWindowID ID_MNU_COLOURDROPPER;
    static const wxWindowID ID_MENUITEM_EFFECT_ASSIST_WINDOW;
    static const wxWindowID ID_MENUITEM_SELECT_EFFECT;
    static const wxWindowID ID_MENUITEM_SEARCH_EFFECTS;
    static const wxWindowID ID_MENUITEM_VIDEOPREVIEW;
    static const wxWindowID ID_MNU_JUKEBOX;
    static const wxWindowID ID_MNU_FINDDATA;
    static const wxWindowID ID_MNU_SUPPRESSDOCK_HP;
    static const wxWindowID ID_MNU_SUPPRESSDOCK_MP;
    static const wxWindowID ID_MENUITEM3;
    static const wxWindowID ID_MENUITEM_WINDOWS_PERSPECTIVE;
    static const wxWindowID ID_MENUITEM_WINDOWS_DOCKALL;
    static const wxWindowID ID_MENUITEM11;
    static const wxWindowID ID_MENUITEM10;
    static const wxWindowID ID_PLAY_FULL;
    static const wxWindowID ID_MNU_1POINT5SPEED;
    static const wxWindowID ID_MN_2SPEED;
    static const wxWindowID ID_MNU_3SPEED;
    static const wxWindowID ID_MNU_4SPEED;
    static const wxWindowID ID_PLAY_3_4;
    static const wxWindowID ID_PLAY_1_2;
    static const wxWindowID ID_PLAY_1_4;
    static const wxWindowID ID_MNU_LOUDVOLUME;
    static const wxWindowID ID_MNU_MEDVOLUME;
    static const wxWindowID ID_MNU_QUIET;
    static const wxWindowID ID_MNU_SUPERQUIET;
    static const wxWindowID ID_MNU_SILENT;
    static const wxWindowID ID_IMPORT_EFFECTS;
    static const wxWindowID ID_MNU_TOD;
    static const wxWindowID ID_MNU_MANUAL;
    static const wxWindowID ID_MNU_ZOOM;
    static const wxWindowID ID_MENUITEM1;
    static const wxWindowID idMenuHelpContent;
    static const wxWindowID ID_MENU_HELP_FORMUM;
    static const wxWindowID ID_MNU_VIDEOS;
    static const wxWindowID ID_MENU_HELP_DOWNLOAD;
    static const wxWindowID ID_MNU_HELP_RELEASE_NOTES;
    static const wxWindowID ID_MENU_HELP_ISSUE;
    static const wxWindowID ID_MENU_HELP_FACEBOOK;
    static const wxWindowID ID_MNU_DONATE;
    static const wxWindowID ID_MNU_UPDATE;
    static const wxWindowID ID_TIMER_OutputTimer;
    static const wxWindowID ID_TIMER_AutoSave;
    static const wxWindowID ID_TIMER_EFFECT_SETTINGS;
    static const wxWindowID ID_TIMER_RENDERSTATUS;
    //*)

    static const long ID_PANEL_EFFECTS1;
    static const long ID_PANEL_EFFECTS;
    static const long ID_NOTEBOOK_EFFECTS;
    static const long ID_MENUITEM_REVERTTO;
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
    wxButton* ButtonAddControllerEthernet;
    wxButton* ButtonAddControllerNull;
    wxButton* ButtonAddControllerSerial;
    wxButton* ButtonControllerDelete;
    wxButton* ButtonDiscover;
    wxButton* ButtonFPPConnect;
    wxButton* ButtonOpen;
    wxButton* ButtonSaveSetup;
    wxButton* ButtonUploadInput;
    wxButton* ButtonUploadOutput;
    wxButton* ButtonVisualise;
    wxButton* Button_ChangeBaseShowDir;
    wxButton* Button_ChangeShowDirPermanently;
    wxButton* Button_ChangeTemporarilyAgain;
    wxButton* Button_CheckShowFolderTemporarily;
    wxButton* Button_ClearBaseShowDir;
    wxButton* Button_OpenBaseShowDir;
    wxButton* Button_OpenProxy;
    wxButton* Button_UpdateBase;
    wxCheckBox* CheckBox_AutoUpdateBase;
    wxChoice* ChoiceParm1;
    wxChoice* ChoiceParm2;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizerSetup;
    wxFlexGridSizer* FlexGridSizerSetupControllerButtons;
    wxFlexGridSizer* FlexGridSizerSetupControllers;
    wxFlexGridSizer* FlexGridSizerSetupProperties;
    wxFlexGridSizer* FlexGridSizerSetupRight;
    wxFlexGridSizer* GaugeSizer;
    wxGauge* ProgressBar;
    wxGridBagSizer* GridBagSizer1;
    wxGridBagSizer* StatusBarSizer;
    wxMenu* AudioMenu;
    wxMenu* Menu1;
    wxMenu* Menu3;
    wxMenu* MenuFile;
    wxMenu* MenuItem18;
    wxMenu* MenuItem1;
    wxMenu* MenuItemPerspectives;
    wxMenu* MenuView;
    wxMenu* RecentSequencesMenu;
    wxMenu* RecentShowFoldersMenu;
    wxMenuBar* MenuBar;
    wxMenuItem* ActionTestMenuItem;
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
    wxMenuItem* MenuItemBackup;
    wxMenuItem* MenuItemBulkControllerUpload;
    wxMenuItem* MenuItemCheckSequence;
    wxMenuItem* MenuItemColourDropper;
    wxMenuItem* MenuItemColours;
    wxMenuItem* MenuItemConvert;
    wxMenuItem* MenuItemDisplayElements;
    wxMenuItem* MenuItemEffectAssist;
    wxMenuItem* MenuItemEffectDropper;
    wxMenuItem* MenuItemEffectSettings;
    wxMenuItem* MenuItemFindData;
    wxMenuItem* MenuItemHinksPixExport;
    wxMenuItem* MenuItemHousePreview;
    wxMenuItem* MenuItemJukebox;
    wxMenuItem* MenuItemLayerBlending;
    wxMenuItem* MenuItemLayerSettings;
    wxMenuItem* MenuItemLoadEditPerspective;
    wxMenuItem* MenuItemModelPreview;
    wxMenuItem* MenuItemRestoreBackup;
    wxMenuItem* MenuItemRunScript;
    wxMenuItem* MenuItemSearchEffects;
    wxMenuItem* MenuItemSelectEffect;
    wxMenuItem* MenuItemShiftEffects;
    wxMenuItem* MenuItemShiftSelectedEffects;
    wxMenuItem* MenuItemUserDict;
    wxMenuItem* MenuItemValueCurves;
    wxMenuItem* MenuItemVideoPreview;
    wxMenuItem* MenuItemViewSaveAsPerspective;
    wxMenuItem* MenuItemViewSavePerspective;
    wxMenuItem* MenuItem_ACLIghts;
    wxMenuItem* MenuItem_CleanupFileLocations;
    wxMenuItem* MenuItem_ColorReplace;
    wxMenuItem* MenuItem_CrashXLights;
    wxMenuItem* MenuItem_Donate;
    wxMenuItem* MenuItem_DownloadSequences;
    wxMenuItem* MenuItem_ExportControllerConnections;
    wxMenuItem* MenuItem_ExportEffects;
    wxMenuItem* MenuItem_FPP_Connect;
    wxMenuItem* MenuItem_File_Close_Sequence;
    wxMenuItem* MenuItem_File_Export_Video;
    wxMenuItem* MenuItem_File_Open_Sequence;
    wxMenuItem* MenuItem_File_Save;
    wxMenuItem* MenuItem_File_SaveAs_Sequence;
    wxMenuItem* MenuItem_Generate2DPath;
    wxMenuItem* MenuItem_GenerateLyrics;
    wxMenuItem* MenuItem_Help_Download;
    wxMenuItem* MenuItem_Help_Facebook;
    wxMenuItem* MenuItem_Help_Forum;
    wxMenuItem* MenuItem_Help_Isue_Tracker;
    wxMenuItem* MenuItem_Help_ReleaseNotes;
    wxMenuItem* MenuItem_ImportEffects;
    wxMenuItem* MenuItem_KeyBindings;
    wxMenuItem* MenuItem_LogRenderState;
    wxMenuItem* MenuItem_LoudVol;
    wxMenuItem* MenuItem_MedVol;
    wxMenuItem* MenuItem_PackageSequence;
    wxMenuItem* MenuItem_PerspectiveAutosave;
    wxMenuItem* MenuItem_PrepareAudio;
    wxMenuItem* MenuItem_PurgeRenderCache;
    wxMenuItem* MenuItem_PurgeVendorCache;
    wxMenuItem* MenuItem_QuietVol;
    wxMenuItem* MenuItem_RemapCustom;
    wxMenuItem* MenuItem_SD_HP;
    wxMenuItem* MenuItem_SD_MP;
    wxMenuItem* MenuItem_ShowACRamps;
    wxMenuItem* MenuItem_ShowKeyBindings;
    wxMenuItem* MenuItem_SilentVol;
    wxMenuItem* MenuItem_TOD;
    wxMenuItem* MenuItem_Update;
    wxMenuItem* MenuItem_UserManual;
    wxMenuItem* MenuItem_VQuietVol;
    wxMenuItem* MenuItem_VideoTutorials;
    wxMenuItem* MenuItem_ViewLog;
    wxMenuItem* MenuItem_ViewZoomIn;
    wxMenuItem* MenuItem_ViewZoomOut;
    wxMenuItem* MenuItem_Zoom;
    wxMenuItem* MenuItem_xScanner;
    wxMenuItem* MenuItem_xSchedule;
    wxMenuItem* Menu_GenerateCustomModel;
    wxMenuItem* Menu_Settings_Sequence;
    wxMenuItem* QuitMenuItem;
    wxMenuItem* mAltBackupMenuItem;
    wxMenuItem* mExportModelsMenuItem;
    wxMenuItem* xCaptureMenuItem;
    wxPanel* AUIStatusBar;
    wxPanel* Panel2;
    wxPanel* Panel5;
    wxPanel* PanelPreview;
    wxPanel* PanelSequencer;
    wxPanel* PanelSetup;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticText* FileNameText;
    wxStaticText* ShowDirectoryLabel;
    wxStaticText* StaticTextDummy;
    wxStaticText* StaticText_BaseShowDir;
    wxStaticText* StaticText_BaseShowDirLabel;
    wxStaticText* StatusText;
    wxTimer AutoSaveTimer;
    wxTimer EffectSettingsTimer;
    wxTimer RenderStatusTimer;
    xLightsTimer OutputTimer;
    xlAuiToolBar* ACToolbar;
    xlAuiToolBar* EditToolBar;
    xlAuiToolBar* EffectsToolBar;
    xlAuiToolBar* MainToolBar;
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

    wxArrayString mruDirectories;  // most recently used directories
    wxMenuItem* mrud_MenuItem[MRUD_LENGTH];
    wxArrayString mruFiles;  // most recently used directories
    wxMenuItem* mruf_MenuItem[MRUF_LENGTH];
    wxMenu *revertToMenu = nullptr;
    wxMenuItem* revertToMenuItem = nullptr;

    OutputManager _outputManager;
    OutputModelManager _outputModelManager;
    long DragRowIdx;
    //wxListCtrl* DragListBox;
    bool UnsavedNetworkChanges = false;
    int mSavedChangeCount = 0;
    int mLastAutosaveCount = 0;
    wxDateTime starttime;
    ModelPreview* modelPreview = nullptr;
    EffectManager effectManager;
    int effGridPrevX;
    int effGridPrevY;
    bool _backupSubfolders = true;
    bool _excludePresetsFromPackagedSequences = true;
    bool _excludeAudioFromPackagedSequences = true;
    bool _promptBatchRenderIssues = true;
    bool _disablePromptBatchRenderIssues = false;
    bool _hwVideoAccleration = false;
    int _hwVideoRenderer = 1;
    bool _showACLights = false;
    bool _showACRamps = false;
    wxString _enableRenderCache;
    size_t _renderCacheMaximumSizeMB = 0;
    bool _playControlsOnPreview = true;
    bool _showBaseShowFolder = false;
    bool _autoShowHousePreview = false;
    bool _zoomMethodToCursor = true;
    bool _hidePresetPreview = false;
    bool _smallWaveform = false;
    bool _modelBlendDefaultOff = true;
    bool _lowDefinitionRender = false;
    bool _saveLowDefinitionRender = false; // saves the value of the low definition render during batch render when it may be temporarily overridden
    bool _snapToTimingMarks = true;
    bool _autoSavePerspecive = true;
    bool _ignoreVendorModelRecommendations = false;
    bool _purgeDownloadCacheOnStart = false;
    int _fseqVersion;
    int _timelineZooming;
    bool _wasMaximised = false;
    bool _suspendRender = false;
    wxArrayString _randomEffectsToUse;
    Model* _presetModel = nullptr;
    SequenceData _presetSequenceData; // we create our own sequence data to render into
    SequenceElements _presetSequenceElements;
    bool _presetRendering = false;
    wxString _defaultSeqView;
    wxString _videoExportCodec;
    int _videoExportBitrate;

    std::unique_ptr< wxAppProgressIndicator> _appProgress;
    std::unique_ptr< wxTaskBarIcon > _taskBarIcon;

    HttpServer* _automationServer = nullptr;
    int _xFadePort = 0;

    void StartAutomationListener();
    bool ProcessHttpRequest(HttpConnection &connection, HttpRequest &request);
    bool ProcessAutomation(std::vector<std::string> &paths,
                           std::map<std::string, std::string> &params,
                           const std::function<bool(const std::string &msg,
                                                    const std::string &jsonKey,
                                                    int responseCode,
                                                    bool msgIsJSON)> &sendResponse);
    std::string ProcessxlDoAutomation(const std::string& msg);
    std::string FindSequence(const std::string& seq);

    void CollectUserEmail();
    void ShowACLights();
    void UpdateControllerSave();
    void UpdateLayoutSave();

    void DoBackup(bool prompt = true, bool startup = false, bool forceallfiles = false);
    void DoBackupPurge();
    void SetBackupPurgeDays(int i);
    int GetBackupPugeDays() const { return BackupPurgeDays; }
    void DoAltBackup(bool prompt = true);

    const std::list<std::string> &GetMediaFolders() { return mediaDirectories; }
    void SetMediaFolders(const std::list<std::string> &folders);
    void GetFSEQFolder(bool& useShow, std::string& folder);
    void SetFSEQFolder(bool useShow, const std::string& folder);
    void GetRenderCacheFolder(bool& useShow, std::string& folder);
    void SetRenderCacheFolder(bool useShow, const std::string& folder);
    void UpdateViewMenu();

    void GetBackupFolder(bool& useShow, std::string& folder);
    void SetBackupFolder(bool useShow, const std::string& folder);
    void GetAltBackupFolder(std::string& folder);
    void SetAltBackupFolder(const std::string& folder);
    bool BackupOnSave() const {return mBackupOnSave;}
    void SetBackupOnSave(bool b) { mBackupOnSave = b;}
    bool BackupOnLaunch() const {return mBackupOnLaunch;}
    void SetBackupOnLaunch(bool b) { mBackupOnLaunch = b;}
    bool BackupSubFolders() const {return _backupSubfolders;}
    void SetBackupSubFolders(bool b) { _backupSubfolders = b;}

    bool GridNodeValues() const { return mGridNodeValues; }
    void SetGridNodeValues(bool b);

    bool GridIconBackgrounds() const { return mGridIconBackgrounds;}
    void SetGridIconBackgrounds(bool b);

    bool SmallWaveform() const { return _smallWaveform; }
    void SetSmallWaveform(bool b);

    int GridSpacing() const { return mGridSpacing; }
    void SetGridSpacing(int i);

    bool TimingPlayOnDClick() const { return mTimingPlayOnDClick;}
    void SetTimingPlayOnDClick(bool b);

    int ToolIconSize() const { return mIconSize; }
    void SetToolIconSize(int size);

    bool GetSnapToTimingMarks() const { return _snapToTimingMarks; }
    bool SnapToTimingMarks() const { return _snapToTimingMarks; }
    void SetSnapToTimingMarks(bool b);

    int XFadePort() const { return _xFadePort; }
    void SetXFadePort(int i);

    bool E131Sync() const {return me131Sync;}
    void SetE131Sync(bool b);

    int SuppressDuplicateFrames() const;
    void SetSuppressDuplicateFrames(int i);

    bool HardwareVideoAccelerated() const { return _hwVideoAccleration; }
    void SetHardwareVideoAccelerated(bool b);

    int HardwareVideoRenderer() const { return _hwVideoRenderer; }
    void SetHardwareVideoRenderer(int type);

    bool ShadersOnBackgroundThreads() const;
    void SetShadersOnBackgroundThreads(bool b);

    bool UseGPURendering() const;
    void SetUseGPURendering(bool b);

    const wxString& UserEMAIL() const { return _userEmail; }
    void SetUserEMAIL(const wxString &e);

    const wxString& GetLinkedSave() const { return _linkedSave; }
    void SetLinkedSave(const wxString& e);

    const wxString& GetLinkedControllerUpload() const { return _linkedControllerUpload; }
    void SetLinkedControllerUpload(const wxString& e);

    int SaveFSEQVersion() const { return _fseqVersion; }
    void SetSaveFSEQVersion(int i) { _fseqVersion = i; }

    int GetTimelineZooming() const { return _timelineZooming; }
    void SetTimelineZooming(int choice) { _timelineZooming = choice; }

    bool ExcludePresetsFromPackagedSequences() const { return _excludePresetsFromPackagedSequences;}
    void SetExcludePresetsFromPackagedSequences(bool b) {_excludePresetsFromPackagedSequences = b;}

    bool ExcludeAudioFromPackagedSequences() const { return _excludeAudioFromPackagedSequences;}
    void SetExcludeAudioFromPackagedSequences(bool b) {_excludeAudioFromPackagedSequences = b;}

    bool GetPromptBatchRenderIssues() const { return _promptBatchRenderIssues; }
    void SetPromptBatchRenderIssues(bool b) { _promptBatchRenderIssues = b; }
    void DisablePromptBatchRenderIssues()
    {
        _disablePromptBatchRenderIssues = true;
        _promptBatchRenderIssues = false;
    }

    bool GetIgnoreVendorModelRecommendations() const { return _ignoreVendorModelRecommendations; }
    void SetIgnoreVendorModelRecommendations(bool b) { _ignoreVendorModelRecommendations = b; }
    void PurgeDownloadCache();

    bool GetPurgeDownloadCacheOnStart() const { return _purgeDownloadCacheOnStart; }
    void SetPurgeDownloadCacheOnStart(bool b) { _purgeDownloadCacheOnStart = b; }

    bool GetRecycleTips() const;
    void SetRecycleTips(bool b);

    bool ModelBlendDefaultOff() const { return _modelBlendDefaultOff;}
    void SetModelBlendDefaultOff(bool b) { _modelBlendDefaultOff = b;}
    void SetLowDefinitionRender(bool b)
    {
        _lowDefinitionRender = b;
    }
    bool IsLowDefinitionRender() const
    {
        return _lowDefinitionRender;
    }
    const wxString &EnableRenderCache() const { return _enableRenderCache; }
    void SetEnableRenderCache(const wxString &t);
    void SetRenderCacheMaximumSizeMB(size_t maxSizeMB);
    size_t RenderCacheMaximumSizeMB() const
    {
        return _renderCacheMaximumSizeMB;
    }

    bool RenderOnSave() const { return mRenderOnSave; }
    void SetRenderOnSave(bool b);

    bool SaveFseqOnSave() const { return mSaveFseqOnSave; }
    void SetSaveFseqOnSave(bool b);

    int AutoSaveInterval() const { return mAutoSaveInterval; }
    void SetAutoSaveInterval(int i);

    bool IsSuppressFadeHints() const { return mSuppressFadeHints; }
    bool SuppressFadeHints() const { return mSuppressFadeHints; }
    void SetSuppressFadeHints(bool b);

    bool IsSuppressColorWarn() const { return mSuppressColorWarn; }
    bool SuppressColorWarn() const { return mSuppressColorWarn; }
    void SetSuppressColorWarn(bool b);

    bool PlayControlsOnPreview() const { return _playControlsOnPreview;}
    void SetPlayControlsOnPreview(bool b);

    bool HidePresetPreview() const { return _hidePresetPreview;}
    void SetHidePresetPreview(bool b);

    bool IsShowBaseShowFolder() const
    {
        return _showBaseShowFolder;
    }
    void SetShowBaseShowFolder(bool b);

    bool AutoShowHousePreview() const { return _autoShowHousePreview;}
    void SetAutoShowHousePreview(bool b);

    bool ZoomMethodToCursor() const { return _zoomMethodToCursor;}
    void SetZoomMethodToCursor(bool b);

    int EffectAssistMode() const { return mEffectAssistMode;}
    void SetEffectAssistMode(int i);

    int GetModelHandleScale() const { return _modelHandleSize; }
    int ModelHandleSize() const { return _modelHandleSize;}
    void SetModelHandleSize(int i);

    const wxArrayString &RandomEffectsToUse() const { return _randomEffectsToUse;}
    void SetRandomEffectsToUse(const wxArrayString &e);

    const wxString& GetDefaultSeqView() const { return _defaultSeqView; }
    void SetDefaultSeqView(const wxString& view);
    wxArrayString GetSequenceViews();

    const wxString& GetVideoExportCodec() const { return _videoExportCodec; }
    void SetVideoExportCodec(const wxString& codec);

    std::string GetMinTipLevel() const;
    void SetMinTipLevel(const wxString& level);

    const int& GetVideoExportBitrate() const { return _videoExportBitrate; }
    void SetVideoExportBitrate(int bitrate);

    bool EnableOutputs(bool ignoreCheck = false);
    bool ToggleOutputs(bool ignoreCheck = false);
    bool DisableOutputs();
    void CycleOutputsIfOn();

    bool ForceEnableOutputs(bool startTimer = true);
    void EnableNetworkChanges();
    void InitEffectsPanel(EffectsPanel* panel);
    void LogPerspective(const wxString& perspective) const;

    // setup
    wxListCtrl* List_Controllers = nullptr;
    bool inInitialize = false;
    wxPropertyGrid* Controllers_PropertyEditor = nullptr;
    wxLed* LedPing = nullptr;

    void OnListItemActivatedControllers(wxListEvent& event);
    void OnListItemSelectedControllers(wxListEvent& event);
    void OnListKeyDownControllers(wxListEvent& event);
    void OnListControllersRClick(wxContextMenuEvent& event);
    void OnListControllersColClick(wxListEvent& event);
    void OnListControllersItemRClick(wxListEvent& event);
    void OnControllerPropertyGridChange(wxPropertyGridEvent& event);
    void OnControllerPropertyGridCollapsed(wxPropertyGridEvent& event);
    void OnControllerPropertyGridExpanded(wxPropertyGridEvent& event);
    void OnListItemDeselectedControllers(wxListEvent& event);

    void SelectController(const std::string& controllerName);
    void UnselectAllControllers();
    void InitialiseControllersTab(bool rebuildPropGrid = true);
    void SetControllersProperties(bool rebuildPropGrid = true);
    void DeleteSelectedControllers();
    void UnlinkSelectedControllers();
    void ActivateSelectedControllers(const std::string& active);
    void SelectAllControllers();
    ControllerCaps* GetControllerCaps(const std::string& name);
    bool UploadInputToController(Controller* controller, wxString& message);
    bool UploadOutputToController(Controller* controller, wxString& message);
    int GetFirstSelectedControllerIndex() const;
    std::list<std::string> GetSelectedControllerNames() const;
    void OnListControllerPopup(wxCommandEvent& event);
    int GetSelectedControllerCount() const;
    int FindControllerInListControllers(const std::string& name) const;

    void ValidateControllerProperties();
    void MoveSelectedControllerRows(bool up);
    void MoveListControllerRows(int toRow, bool reverse);
    void OnListItemBeginDragControllers(wxListEvent& event);
    void OnListItemDragQuitControllers(wxMouseEvent& event);
    void OnListItemDragEndControllers(wxMouseEvent& event);
    void OnListItemMoveControllers(wxMouseEvent& event);
    void OnListItemScrollTimerControllers(wxTimerEvent& event);

    void OnMenuMRU(wxCommandEvent& event);
    void OnMRUSequence(wxCommandEvent& event);
    bool SetDir(const wxString& dirname, bool permanent);
    void SetBaseShowDir(const wxString& baseShowDir);
    void UpdateFromBaseShowFolder(bool prompt);
    void UpdateRecentFilesList(bool reload);
    void AddToMRU(const std::string& filename);
    bool PromptForShowDirectory(bool permanent, const std::string &defaultDir = "");
    bool PromptForDirectorySelection(const std::string &msg, std::string &dir);
    bool SaveNetworksFile();
    bool IsControllersAndLayoutTabSaveLinked() { return _linkedSave == "Controllers and Layout Tab"; }
    bool IsControllerUploadLinked() { return _linkedControllerUpload == "Inputs and Outputs"; }
    void NetworkChange();
    void NetworkChannelsChange();
	void PingController(Controller* e);
    void SetModelData(ControllerEthernet* controller, ModelManager* modelManager, OutputManager* outputManager, std::string showDir);
    int SetZCPPPort(Controller* c, std::list<ZCPP_packet_t*>& modelDatas, int index, UDControllerPort* port, int portNum, int virtualString, long baseStart, bool isSerial, ZCPPOutput* zcpp);
    void SetZCPPExtraConfig(std::list<ZCPP_packet_t*>& extraConfig, int portNum, int virtualStringNum, const std::string& name, ZCPPOutput* zcpp);

    void OnProgressBarDoubleClick(wxMouseEvent& event);

    void DoPostStartupCommands();

    std::list<RenderProgressInfo *>renderProgressInfo;
    std::queue<RenderEvent*> mainThreadRenderEvents;
    std::mutex renderEventLock;

    std::string _permanentShowFolder;
    std::string mediaFilename;
    std::string showDirectory;
    std::list<std::string> mediaDirectories;
    std::string fseqDirectory;
    std::string renderCacheDirectory;
    std::string _backupDirectory;
    SeqDataType _seqData;
    wxTimer _scrollTimer;

    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;
    long seekPoint;

    wxString mBackgroundImage;
    int mBackgroundBrightness;
    int mBackgroundAlpha;
    bool mScaleBackgroundImage = false;
    std::string mStoredLayoutGroup;
    bool _suspendAutoSave = false;

    // convert
public:
    bool UnsavedRgbEffectsChanges;
    unsigned int modelsChangeCount;
    bool _renderMode = false;
    bool _checkSequenceMode = false;

    void SuspendAutoSave(bool dosuspend) { _suspendAutoSave = dosuspend; }
    void ClearLastPeriod();
    void WriteVirFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf); //       Vixen *.vir
    void WriteHLSFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf);  //      HLS *.hlsnc
    void WriteLcbFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf, int ver, int cpn);  //      LOR *.lcb
    void WriteLSPFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame, SeqDataType *dataBuf, int cpn);  //      LSP UserPatterns.xml
    void ReadXlightsFile(const wxString& FileName, wxString *mediaFilename = nullptr);
    void ReadFalconFile(const wxString& FileName, ConvertDialog* convertdlg);
    void WriteFalconPiFile(const wxString& filename, bool allowSparse = true); //  Falcon Pi Player *.fseq
    OutputManager* GetOutputManager() { return &_outputManager; };
    OutputModelManager* GetOutputModelManager() { return&_outputModelManager; }
    void WriteGIFForPreset(const std::string& preset);

private:

    void WriteFalconPiModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
                                SeqDataType *dataBuf, int startAddr, int modelSize,
                                bool v2 = false); //Falcon Pi sub sequence .eseq
    void WriteVideoModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
        SeqDataType *dataBuf, int startAddr, int modelSize, Model* model, bool compressed); //.avi file
    void WriteMinleonNECModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
        SeqDataType *dataBuf, int startAddr, int modelSize, Model* model); //.bin file
    void WriteGIFModelFile(const wxString& filename, long numChans, unsigned int startFrame, unsigned int endFrame,
        SeqDataType* dataBuf, int startAddr, int modelSize, Model* model, unsigned int frameTime) const;

    // sequence
    void LoadEffectsFile();
    wxString LoadEffectsFileNoCheck();
    void CreateDefaultEffectsXml();
    bool TimerRgbSeq(long msec);
    void SetChoicebook(wxChoicebook* cb, const wxString& PageName);
    void SetPanelSequencerLabel(const std::string& sequence);

    void DisplayXlightsFilename(const wxString& filename) const;
    int ChooseRandomEffect();

public:
    bool IsNewModel(Model* m) const;
    int GetCurrentPlayTime();
    bool InitPixelBuffer(const std::string &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased = false);
    Model *GetModel(const std::string& name) const;
    void RenderGridToSeqData(std::function<void(bool)>&& callback);
    bool AbortRender(int maxTimeMs = 60000, int* numThreadsAborted = nullptr);
    std::string GetSelectedLayoutPanelPreview() const;
    void UpdateRenderStatus();
    void LogRenderStatus();
    bool RenderEffectFromMap(bool suppress, Effect *effect, int layer, int period, SettingsMap& SettingsMap,
                             PixelBufferClass &buffer, bool &ResetEffectState,
                             bool bgThread = false, RenderEvent *event = nullptr);
    void RenderMainThreadEffects();
    void RenderEffectOnMainThread(RenderEvent *evt);
    void RenderEffectForModel(const std::string &model, int startms, int endms, bool clear = false);
    void RenderDirtyModels();
    void RenderTimeSlice(int startms, int endms, bool clear);
    void Render(SequenceElements& seqElements,
                SequenceData& seqData,
                const std::list<Model*> models,
                const std::list<Model *> &restrictToModels,
                int startFrame, int endFrame,
                bool progressDialog, bool clear,
                std::function<void(bool)>&& callback);
    void BuildRenderTree();

    void RenderRange(RenderCommandEvent &cmd);
    void RenderDone();
    bool IsDrawRamps();

    void EnableSequenceControls(bool enable);
    SequenceElements& GetSequenceElements() { return _sequenceElements; }
    TimingElement* AddTimingElement(const std::string& name, const std::string &subType = "");
    void DeleteTimingElement(const std::string& name);
    void RenameTimingElement(const std::string& old_name, const std::string& new_name);
    void ImportTimingElement();
    void ExecuteImportTimingElement(wxCommandEvent &command);
    void ExecuteImportNotes(wxCommandEvent &command);
    void ConvertDataRowToEffects(wxCommandEvent &command);
    void DoConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime, bool eraseExisting);
    void PromoteEffects(wxCommandEvent &command);
    void DoPromoteEffects(ModelElement *element);
    wxXmlNode* CreateEffectNode(wxString& name);
    void UpdateEffectNode(wxXmlNode* node);
    wxXmlNode* FindPreset(wxXmlNode* node, wxArrayString& path, int level = 0) const;
    void ApplyEffectsPreset(wxString& data, const wxString &pasteDataVersion);
    Effect* ApplyEffectsPreset(const std::string& presetName);
    std::vector<std::string> GetPresets() const;
    void RenameModelInViews(const std::string old_name, const std::string& new_name);
    bool RenameModel(const std::string old_name, const std::string& new_name);
    bool RenameObject(const std::string old_name, const std::string& new_name);
    void ReplaceModelWithModelFixGroups(const std::string& oldModel, const std::string& newModel);
    bool EnsureSequenceElementsAreOrderedCorrectly(const std::string ModelName, std::vector<std::string>& submodelOrder);
    void UpdateSequenceLength();

    void SetSequenceEnd(int ms);
    void SetFrequency(int frequency);
    void RenderAll();

    void SetXmlSetting(const wxString& settingName,const wxString& value);
    wxString GetXmlSetting(const wxString& settingName,const wxString& defaultValue) const;
    uint32_t GetMaxNumChannels();

    void UpdateSequenceVideoPanel( const wxString& path );

protected:
    bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
    bool SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels);
    void ResetEffectsXml();
    std::string CreateEffectStringRandom(std::string &settings, std::string &palette);
    bool CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, bool forceallfiles, std::string& errors);
    void BackupDirectory(wxString sourceDir, wxString targetDirName, wxString lastCreatedDirectory, bool forceallfiles, bool backupSubfolders, std::string& errors);
    void CreateMissingDirectories(wxString targetDirName, wxString lastCreatedDirectory, std::string& errors);
    static constexpr int RENDER_EXIT_ON_DONE = 1;
    static constexpr int RENDER_ALREADY_RETRIED = 2;
    void OpenRenderAndSaveSequencesF(const wxArrayString &filenames, int flags);
    void OpenRenderAndSaveSequences(const wxArrayString& filenames, bool exitOnDone, bool alreadyRetried = false);
    void OpenAndCheckSequence(const wxArrayString& origFilenames, bool exitOnDone);
    std::string OpenAndCheckSequence(const std::string& origFilenames);
    void AddAllModelsToSequence();
    void ShowPreviewTime(long ElapsedMSec);
    void TimerOutput(int period);
    void UpdateChannelNames();
    void StopNow();
    bool ShowFolderIsInBackup(const std::string showdir);
    bool ShowFolderIsInProgramFiles(const std::string showdir);
    std::string PackageSequence(bool showDialogs = true);
    std::string PackageDebugFiles(bool showDialogs = true);

    bool Grid1HasFocus; //cut/copy/paste handled differently with grid vs. other text controls -DJ
    wxXmlDocument EffectsXml;
	SequenceViewManager _sequenceViewManager;
    wxXmlNode* EffectsNode = nullptr;
    wxXmlNode* PerspectivesNode = nullptr;
public:
    bool RebuildControllerConfig(OutputManager* outputManager, ModelManager* modelManager);
    wxXmlNode* PalettesNode = nullptr;
    wxXmlNode* ModelsNode = nullptr;
    wxXmlNode* ModelGroupsNode = nullptr;
    wxXmlNode* LayoutGroupsNode = nullptr;
    wxXmlNode* ViewObjectsNode = nullptr;
    SequenceViewManager* GetViewsManager() { return &_sequenceViewManager; }
    void OpenSequence(const wxString &passed_filename, ConvertLogDialog* plog, const wxString &realPath = "");
    void OpenSequence(const wxString& passed_filename) {
     OpenSequence(passed_filename, nullptr); 
    }
    void SaveSequence();
    void SetSequenceTiming(int timingMS);
    bool CloseSequence();
    void NewSequence(const std::string& media = "", uint32_t durationMS = 0, uint32_t frameMS = 0, const std::string& defView = "");
    void SaveAsSequence();
    void SaveAsSequence(const std::string& filename);
    void SetPasteByCell();
    void SetPasteByTime();
    void ShowSequenceSettings();
    bool HandleAllKeyBinding(wxKeyEvent& event);

private:
    wxXmlNode* SettingsNode = nullptr;

    bool MixTypeChanged = false;
    bool FadesChanged = false;
    long SeqBaseChannel;
    bool SeqChanCtrlBasic;
    bool SeqChanCtrlColor;
	bool mLoopAudio = false;

    std::unique_ptr<wxPreferencesEditor> mPreferencesEditor;
    bool mResetToolbars = false;
    bool mRenderOnSave = false;
    bool mBackupOnSave = false;
    bool mBackupOnLaunch = true;
    bool me131Sync = false;
    bool mSuppressFadeHints = false;
    bool mSuppressColorWarn = false;
    wxString mAltBackupDir;
    int mIconSize;
    int mGridSpacing;
    bool mGridIconBackgrounds;
    bool mTimingPlayOnDClick;
    bool mGridNodeValues;
    int mEffectAssistMode = 0;
    int tempEffectAssistMode = 0;
	bool mRendering;
    int abortedRenderJobs = 0;
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
    int mAutoSaveInterval;
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
    void SetDisplay2DGrid(bool grid);
    bool GetDisplay2DGrid() const;
    void SetDisplay2DGridSpacing(long gridSpacing);
    long GetDisplay2DGridSpacing() const;
    void SetDisplay2DCenter0(bool bb);
    bool GetDisplay2DCenter0() const;
    const wxString & GetDefaultPreviewBackgroundImage();
    bool GetDefaultPreviewBackgroundScaled();
    int GetDefaultPreviewBackgroundBrightness();
    int GetDefaultPreviewBackgroundAlpha();
    void SetPreviewBackgroundBrightness(int brightness, int alpha);
    void UpdateModelsList();
    void RowHeadingsChanged( wxCommandEvent& event);
    void DoForceSequencerRefresh();
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
    void RenderLayout();
    ViewsModelsPanel* GetDisplayElementsPanel() const { return displayElementsPanel; }
    EffectsPanel* GetEffectsPanel() const { return EffectsPanel1; }
    void ResetPanelDefaultSettings(const std::string& effect, const Model* model, bool optionbased);

    void UnselectEffect();
    FindDataPanel* GetFindDataPanel() const
    {
        return _findDataPanel;
    }
    void ShowDataFindPanel();

    static xLightsFrame *GetFrame();
    void CallOnEffectBeforeSelected(std::function<bool(Effect *)> &&cb);
    void CallOnEffectAfterSelected(std::function<bool(Effect *)> &&cb);
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
    SequenceElements _sequenceElements;
    MainSequencer* mainSequencer = nullptr;
    ModelPreview * _modelPreviewPanel = nullptr;
    HousePreviewPanel *_housePreviewPanel = nullptr;
    LayoutPanel *layoutPanel = nullptr;
    EffectAssist* sEffectAssist = nullptr;
    ColorPanel* colorPanel = nullptr;
    TimingPanel* timingPanel = nullptr;
    PerspectivesPanel* perspectivePanel = nullptr;
    EffectIconPanel* effectPalettePanel = nullptr;
    ValueCurvesPanel* _valueCurvesPanel = nullptr;
    ColoursPanel* _coloursPanel = nullptr;
    JukeboxPanel* jukeboxPanel = nullptr;
    FindDataPanel* _findDataPanel = nullptr;
    BufferPanel* bufferPanel = nullptr;
    ViewsModelsPanel *displayElementsPanel = nullptr;
    TopEffectsPanel* effectsPnl = nullptr;
    EffectsPanel* EffectsPanel1 = nullptr;
    SelectPanel *_selectPanel = nullptr;
    SequenceVideoPanel* sequenceVideoPanel = nullptr;
    SearchPanel* _searchPanel = nullptr;
    std::unique_ptr<ScriptsDialog> _scriptsDialog;
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
    void Scrub(wxCommandEvent& event);
    void Zoom(wxCommandEvent& event);
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
    void CutModelEffects(wxCommandEvent& event);
    void CopyModelEffects(wxCommandEvent& event);
    void PasteModelEffects(wxCommandEvent& event);
    void ModelSelected(wxCommandEvent& event);
    void PlaySequence(wxCommandEvent& event);
    void PauseSequence(wxCommandEvent& event);
    void VCChanged(wxCommandEvent& event);
    void ColourChanged(wxCommandEvent& event);
    void StopSequence(wxCommandEvent& event);
    void SequenceFirstFrame(wxCommandEvent& event);
    void SequenceLastFrame(wxCommandEvent& event);
    void SequenceRewind10(wxCommandEvent& event);
    void SequenceFForward10(wxCommandEvent& event);
    void SequenceSeekTo(wxCommandEvent& event);
    void SequenceReplaySection(wxCommandEvent& event);
    void TogglePlay(wxCommandEvent& event);
    void ExportModel(wxCommandEvent& event);
    bool DoExportModel(unsigned int startFrame, unsigned int endFrame, const std::string& model, const std::string& fn, const std::string& fmt, bool doRender);
    void ShowDisplayElements(wxCommandEvent& event);
    void ShowHidePreviewWindow(wxCommandEvent& event);
    void ShowHideAllPreviewWindows(wxCommandEvent& event);
    void SetEffectChoice(wxCommandEvent& event);
    void TipOfDayReady(wxCommandEvent& event);
    void SetEffectDuration(wxCommandEvent& event);

    bool isRandom_(wxControl* ctl, const char*debug);
    void SetSyncUniverse(int syncUniverse);

    bool IsDockable(const std::string& panel);
    void LoadDockable();
    void SaveDockable();

    Effect* GetPersistentEffectOnModelStartingAtTime(const std::string& model, uint32_t startms) const;
    void EnableToolbarButton(wxAuiToolBar* toolbar, int id, bool enable);
    void CheckForAndCreateDefaultPerpective();
    void ResizeAndMakeEffectsScroll();
    void ResizeMainSequencer();
    void LoadSequencer(xLightsXmlFile& xml_file);
    void DoLoadPerspective(wxXmlNode *p);
    void CheckForValidModels();
    void ExportModels(wxString const& filename);
    void ExportEffects(wxString const& filename);
    int ExportElement(wxFile& f, Element* e, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles);
    int ExportNodes(wxFile& f, StrandElement* e, NodeLayer* nl, int n, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles);
    std::map<int, std::vector<float>> LoadPolyphonicTranscription(AudioManager* audio, int intervalMS);
    std::map<int, std::vector<float>> LoadAudacityFile(std::string file, int intervalMS);
    std::map<int, std::vector<float>> LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track);
    std::map<int, std::vector<float>> LoadMusicXMLFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track);
    void CreateNotes(EffectLayer* el, std::map<int, std::vector<float>>& notes, int interval, int frames);
    std::string CreateNotesLabel(const std::vector<float>& notes) const;
    std::string CheckSequence(bool displayInEditor, bool writeToFile);
    void ValidateEffectAssets();
    bool CleanupRGBEffectsFileLocations();
    bool CleanupSequenceFileLocations();
    void CheckElement(Element* e, wxFile& f, size_t& errcount, size_t& warncount, const std::string& name, const std::string& modelName, bool& videoCacheWarning, bool& disabledEffects, std::list<std::pair<std::string, std::string>>& faces, std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints, bool& usesShader, std::list<std::string>& allfiles);
    void CheckEffect(Effect* ef, wxFile& f, size_t& errcount, size_t& warncount, const std::string& name, const std::string& modelName, bool node, bool& videoCacheWarning, bool& disabledEffects, std::list<std::pair<std::string, std::string>>& faces, std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints);
    bool CheckStart(wxFile& f, const std::string& startmodel, std::list<std::string>& seen, std::string& nextmodel);
    void ValidateWindow();
    void DoDonate();
    void AutoShowHouse();
    bool CheckForUpdate(int maxRetries, bool canSkipUpdates, bool showMessageBoxes);
    void ShiftEffectsOnLayer(EffectLayer* el, int milliseconds);
    void ShiftSelectedEffectsOnLayer(EffectLayer* el, int milliseconds);
    void InitSequencer();
    void CreateSequencer();
    void DoStopSequence();

    wxMenu* MenuItemPreviews = nullptr;
    wxMenuItem* MenuItemPreviewSeparator = nullptr;
    static const long ID_MENU_ITEM_PREVIEWS;
    static const long ID_MENU_ITEM_PREVIEWS_SHOW_ALL;

    static const long ID_List_Controllers;
    static const long ID_NETWORK_ADDSERIAL;
    static const long ID_NETWORK_ADDNULL;
    static const long ID_NETWORK_ADDETHERNET;
    static const long ID_NETWORK_ACTIVE;
    static const long ID_NETWORK_ACTIVEXLIGHTS;
    static const long ID_NETWORK_UNLINKFROMBASE;
    static const long ID_NETWORK_INACTIVE;
    static const long ID_NETWORK_DELETE;

    #define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)

    DECLARE_EVENT_TABLE()
    friend class xLightsApp; //kludge: allow xLightsApp to call OnPaneNutcrackerChar -DJ

public:
    FILE* _logfile = nullptr;
    std::vector<Model *> PreviewModels;
    std::vector<LayoutGroup *> LayoutGroups;
    std::vector<ModelPreview *> PreviewWindows;
    ModelManager AllModels;
    ViewObjectManager AllObjects;
    ColorManager color_mgr;
    ViewpointMgr viewpoint_mgr;
    EffectTreeDialog *EffectTreeDlg = nullptr;

    ModelGroup* GetSelectedModelGroup() const;
    void LoadJukebox(wxXmlNode* node);
    static wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false);
    TimingPanel* GetLayerBlendingPanel() const { return timingPanel; }

    int GetPlayStatus() const { return playType; }
    void SetPlayStatus(int status);
    void StartOutputTimer();
    void StopOutputTimer();
    
    MainSequencer* GetMainSequencer() const { return mainSequencer; }
    wxString GetSeqXmlFileName();

    std::string MoveToShowFolder(const std::string& file, const std::string& subdirectory, const bool reuse = false);
    bool IsInShowFolder(const std::string & file) const;
    bool FilesMatch(const std::string & file1, const std::string & file2) const;
    ColorPanel* GetColorPanel() const { return colorPanel; }

    std::string GetEffectTextFromWindows(std::string &palette) const;
    void ValidatePanels();

	void DoPlaySequence();
    void RecalcModels();
    std::string GetUniqueTimingName(const std::string& baseName);
};

