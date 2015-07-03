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
#include <wx/aui/aui.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
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
#include "ModelListDialog.h"
#include "EffectListDialog.h"
#include "SeqExportDialog.h"
#include "ViewsDialog.h"
#include "SeqElementMismatchDialog.h"
#include "PixelBuffer.h"
#include "NetInfo.h"
#include "PaletteMgmtDialog.h"
#include "ExportModelSelect.h"
#include "ViewsDialog.h"
#include "WholeHouseModelNameDialog.h"
#include "CurrentPreviewModels.h"
#include "PreviewModels.h"
#include "ModelPreview.h"
#include "dlgPreviewSize.h"
#include "SequenceData.h"
#include "UtilClasses.h"

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

#include "wx/aui/aui.h"


#include "JobPool.h"


class EffectTreeDialog;

// max number of most recently used show directories on the File menu
#define MRU_LENGTH 4

// notebook pages
#define SETUPTAB 0
#define TESTTAB 1
#define CONVERTTAB 2
#define PREVIEWTAB 3
#define SCHEDULETAB 4
#define PAPAGAYOTAB 5
#define NEWSEQUENCER 6

#define FixedPages 7

#define OVER_NO_HANDLE              0
#define OVER_L_TOP_HANDLE           1
#define OVER_R_TOP_HANDLE           2
#define OVER_L_BOTTOM_HANDLE        3
#define OVER_R_BOTTOM_HANDLE        4
#define OVER_ROTATE_HANDLE          5

#define TEXT_ENTRY_TIMING           0
#define TEXT_ENTRY_EFFECT           1
#define TEXT_ENTRY_DIALOG           2

class RenderCommandEvent;

wxDECLARE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDECLARE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);
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


static const wxString xlights_base_name       = "xLights 4 BETA";
static const wxString xlights_version_string  = "4.1.9";
static const wxString xlights_build_date      = "Jun 30, 2015";

static const wxString strSupportedFileTypes = "LOR Music Sequences (*.lms)|*.lms|LOR Animation Sequences (*.las)|*.las|HLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata|Vixen Sequences (*.vix)|*.vix|Glediator Record File (*.gled)|*.gled)|Lynx Conductor Sequences (*.seq)|*.seq|xLights Sequences(*.xseq)|*.xseq|xLights Imports(*.iseq)|*.iseq|Falcon Pi Player Sequences (*.fseq)|*.fseq";
static wxCriticalSection gs_xoutCriticalSection;



typedef SequenceData SeqDataType;

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

class RenderEvent;
class wxDebugReportCompress;

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

    void BasicPrompt(char* prompt, char* buff, int size);
    void BasicOutput(char *msg);
    void BasicError(const char *msg);
    char ExtType(const wxString& ext);
    bool Play(wxString& filename, long delay);
    void StopPlayback();
    void StopPreviewPlayback();
    void StartScript(const char *scriptname);
    void EndScript(const char *scriptname);
    int  FindNotebookPage(wxString& pagename);
    wxWindow* FindNotebookControl(int nbidx, PlayListIds id);
    void SetEffectControls(const wxString &name, const SettingsMap &settings, const SettingsMap &palette);
    void SetEffectControls(const SettingsMap &settings);
    bool SaveEffectsFile();
    void SetStatusText(const wxString &msg);



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
    static xLightsXmlFile* CurrentSeqXmlFile; // global object for currently opened XML file
    wxString GetShowDirectory() { return showDirectory; }
    NetInfoClass& GetNetInfo() { return NetInfo; }
    wxCheckListBox* GetCheckListBoxTestChannels() { return CheckListBoxTestChannels; }
    void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);
    void ConversionInit();
    void ConversionError(const wxString& msg);
    void PlayerError(const wxString& msg);
    void SetMediaFilename(const wxString& filename);
    void AppendConvertLog(const wxString& msg);
    void RenderIseqData(bool bottom_layers);
    void ClearSequenceData();
    void LoadAudioData(xLightsXmlFile& xml_file);
    void CreateDebugReport(wxDebugReportCompress *report);
    wxString GetThreadStatusReport();

    void ImportSuperStar(const wxFileName &filename);
    void ImportLMS(const wxFileName &filename);
    void ImportHLS(const wxFileName &filename);
    void ImportVix(const wxFileName &filename);
    void ImportXLights(const wxFileName &filename);
    void ImportLSP(const wxFileName &filename);

private:
    bool ImportSuperStar(Element *el, wxXmlDocument &doc, int x_size, int y_size, int x_offset, int y_offset, bool flip_y);
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
    void OnChoice_PgoOutputTypeSelect(wxCommandEvent& event);
    void OnButton_PgoCopyVoicesClick(wxCommandEvent& event);
    void OnTextCtrl_PgoMinRestText(wxCommandEvent& event);
    void OnTextCtrl_PgoMaxRestText(wxCommandEvent& event);
    void OnTextCtrl_PgoAutoFadeText(wxCommandEvent& event);
    void OnbtEditViewsClick(wxCommandEvent& event);
    void OnChoice_ViewsSelect(wxCommandEvent& event);
    void OnButtonBuildCustomModelClick(wxCommandEvent& event);
    void OnButtonBuildWholeHouseModelClick(wxCommandEvent& event);
    void OnTextCtrlModelRotationDegreesText(wxCommandEvent& event);
    void OnButtonSelectModelGroupsClick(wxCommandEvent& event);
    void OnScrolledWindowPreviewPaint(wxPaintEvent& event);
    void OnButtonSetPreviewSizeClick(wxCommandEvent& event);
    void OnButtonSetBackgroundImageClick(wxCommandEvent& event);
    void OnScrolledWindowPreviewRightDown(wxMouseEvent& event);
    void OnSliderPreviewTimeCmdScrollThumbTrack(wxScrollEvent& event);
    void OnSliderPreviewTimeCmdScrollThumbRelease(wxScrollEvent& event);
    void OnSlider_BackgroundBrightnessCmdSliderUpdated(wxScrollEvent& event);
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
    void OnMenuItemSequenceElementsSelected(wxCommandEvent& event);
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
    void OnScaleImageCheckboxClick(wxCommandEvent& event);
    void OnSetGridIconBackground(wxCommandEvent& event);
    void OnSetGridNodeValues(wxCommandEvent& event);
    void OnMenuItemImportEffects(wxCommandEvent& event);
    void SetPlaySpeed(wxCommandEvent& event);
    void OnBitmapButton_Link_DirsClick(wxCommandEvent& event);
    void OnAuiToolBarItemModelsClick(wxCommandEvent& event);
    void OnMenuItemRenderOnSave(wxCommandEvent& event);
    void OnNotebook1PageChanging(wxAuiNotebookEvent& event);
    void OnButtonAddNullClick(wxCommandEvent& event);
    //*)

    void OnPopupClick(wxCommandEvent &evt);
    void DeleteSelectedEffects(wxCommandEvent &evt);
    void InsertRandomEffects(wxCommandEvent &evt);
    void UnprotectSelectedEffects(wxCommandEvent& evt);
    void ProtectSelectedEffects(wxCommandEvent& evt);
    void CopyEffectAcrossRow(wxCommandEvent& evt); //-DJ
    void ClearEffectRow(wxCommandEvent& evt); //-DJ
    void ShowHideAllSequencerWindows(bool show);

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
    static const long ID_AUITOOLBARITEM3;
    static const long ID_AUITOOLBARITEM1;
    static const long ID_AUITOOLBARITEM4;
    static const long ID_AUITOOLBARITEM6;
    static const long ID_AUIWINDOWTOOLBAR;
    static const long ID_AUITOOLBARITEM_ZOOM_IN;
    static const long ID_AUITOOLBARITEM_ZOOM_OUT;
    static const long ID_AUITOOLBARITEM14;
    static const long ID_AUITOOLBAR_VIEW;
    static const long ID_BITMAPBUTTON_TAB_INFO;
    static const long ID_BUTTON_STOP_NOW;
    static const long ID_BUTTON_GRACEFUL_STOP;
    static const long ID_BUTTON_LIGHTS_OFF;
    static const long ID_CHECKBOX_LIGHT_OUTPUT;
    static const long ID_AUITOOLBAR_OUTPUT;
    static const long ID_BITMAPBUTTON7;
    static const long ID_BITMAPBUTTON8;
    static const long ID_BITMAPBUTTON3;
    static const long ID_BITMAPBUTTON4;
    static const long ID_BITMAPBUTTON31;
    static const long ID_BITMAPBUTTON32;
    static const long ID_BITMAPBUTTON33;
    static const long ID_BITMAPBUTTON34;
    static const long ID_BITMAPBUTTON9;
    static const long ID_BITMAPBUTTON10;
    static const long ID_BITMAPBUTTON11;
    static const long ID_BITMAPBUTTON12;
    static const long ID_BITMAPBUTTON13;
    static const long ID_BITMAPBUTTON14;
    static const long ID_BITMAPBUTTON15;
    static const long ID_BITMAPBUTTON16;
    static const long ID_BITMAPBUTTON17;
    static const long ID_BITMAPBUTTON18;
    static const long ID_BITMAPBUTTON19;
    static const long ID_BITMAPBUTTON20;
    static const long ID_BITMAPBUTTON21;
    static const long ID_BITMAPBUTTON22;
    static const long ID_BITMAPBUTTON23;
    static const long ID_BITMAPBUTTON24;
    static const long ID_BITMAPBUTTON25;
    static const long ID_BITMAPBUTTON26;
    static const long ID_BITMAPBUTTON27;
    static const long ID_BITMAPBUTTON28;
    static const long ID_BITMAPBUTTON29;
    static const long ID_BITMAPBUTTON30;
    static const long ID_BITMAPBUTTON35;
    static const long ID_BITMAPBUTTON36;
    static const long ID_BITMAPBUTTON37;
    static const long ID_BITMAPBUTTON38;
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
    static const long ID_STATICTEXT17;
    static const long ID_CHECKBOX_OFF_AT_END;
    static const long ID_STATICTEXT20;
    static const long ID_CHECKBOX_MAP_EMPTY_CHANNELS;
    static const long ID_STATICTEXT33;
    static const long ID_CHECKBOX_LOR_WITH_NO_CHANNELS;
    static const long ID_CHOICE1;
    static const long ID_BUTTON_START_CONVERSION;
    static const long ID_STATICTEXT18;
    static const long ID_TEXTCTRL_CONVERSION_STATUS;
    static const long ID_PANEL_CONVERT;
    static const long ID_BUTTON_PREVIEW_OPEN;
    static const long ID_STATICTEXT23;
    static const long ID_BITMAPBUTTON5;
    static const long ID_BITMAPBUTTON6;
    static const long ID_TEXTCTRL_PREVIEW_TIME;
    static const long ID_SLIDER_PREVIEW_TIME;
    static const long ID_STATICTEXT_CURRENT_PREVIEW_SIZE;
    static const long ID_BUTTON_SET_PREVIEW_SIZE;
    static const long ID_BUTTON_SET_BACKGROUND_IMAGE;
    static const long ID_CHECKBOX1;
    static const long ID_STATICTEXT32;
    static const long ID_SLIDER_BACKGROUND_BRIGHTNESS;
    static const long ID_BUTTON_SELECT_MODEL_GROUPS;
    static const long ID_STATICTEXT21;
    static const long ID_LISTBOX_ELEMENT_LIST;
    static const long ID_BUTTON_MODELS_PREVIEW;
    static const long ID_BUTTON_SAVE_PREVIEW;
    static const long ID_BUTTON_BUILD_WHOLEHOUSE_MODEL;
    static const long ID_STATICTEXT22;
    static const long ID_TEXTCTRL_PREVIEW_ELEMENT_SIZE;
    static const long ID_SLIDER_PREVIEW_SCALE;
    static const long ID_STATICTEXT25;
    static const long ID_TEXTCTRL2;
    static const long ID_SLIDER_PREVIEW_ROTATE;
    static const long ID_PANEL1;
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
    static const long ID_CLOSE_SEQ;
    static const long ID_MENUITEM2;
    static const long ID_FILE_BACKUP;
    static const long idMenuSaveSched;
    static const long idMenuAddList;
    static const long idMenuRenameList;
    static const long idMenuDelList;
    static const long ID_MENUITEM1;
    static const long idCustomScript;
    static const long ID_MENUITEM_VIEW_ZOOM_IN;
    static const long ID_MENUITEM_VIEW_ZOOM_OUT;
    static const long ID_MENUITEM_SAVE_PERSPECTIVE;
    static const long ID_MENUITEM_LOAD_PERSPECTIVE;
    static const long ID_MENUITEM7;
    static const long ID_MENUITEM_SEQUENCE_ELEMENTS;
    static const long ID_MENUITEM12;
    static const long ID_MENUITEM3;
    static const long ID_MENUITEM13;
    static const long ID_MENUITEM14;
    static const long ID_MENUITEM15;
    static const long ID_MENUITEM16;
    static const long ID_MENUITEM17;
    static const long ID_MENUITEM_WINDOWS_PERSPECTIVE;
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

    static const long ID_PREVIEW_ALIGN;
    static const long ID_PREVIEW_MODEL_PROPERTIES;
    static const long ID_PREVIEW_ALIGN_TOP;
    static const long ID_PREVIEW_ALIGN_BOTTOM;
    static const long ID_PREVIEW_ALIGN_LEFT;
    static const long ID_PREVIEW_ALIGN_RIGHT;
    static const long ID_PREVIEW_ALIGN_H_CENTER;
    static const long ID_PREVIEW_ALIGN_V_CENTER;
    static const long ID_PREVIEW_DISTRIBUTE;
    static const long ID_PREVIEW_H_DISTRIBUTE;
    static const long ID_PREVIEW_V_DISTRIBUTE;




    //(*Declarations(xLightsFrame)
    wxRadioButton* RadioButtonRgbChase;
    wxSlider* SliderFgColorC;
    wxRadioButton* RadioButtonRgbCycleMixed;
    DragEffectBitmapButton* BitmapButton27;
    DragEffectBitmapButton* BitmapButton19;
    wxButton* Button_papagayo_output_sequence;
    wxMenu* MenuItem18;
    wxTextCtrl* TextCtrl_PgoAutoFade;
    xlAuiToolBar* ViewToolBar;
    wxButton* ButtonClearLog;
    DragEffectBitmapButton* BitmapButton6;
    xlAuiToolBar* PlayToolBar;
    wxFlexGridSizer* PreviewGLSizer;
    wxStaticText* StaticText68;
    wxSlider* SliderBgIntensity;
    wxButton* Button_CoroGroupDelete;
    wxStaticText* StaticText32;
    wxMenuItem* MenuItemLoadEditPerspective;
    wxStaticText* StaticText36;
    wxPanel* Panel2;
    wxButton* ButtonTestSelectAll;
    wxButton* ButtonSaveSchedule;
    wxRadioButton* RadioButtonRgbTwinkle50;
    wxStaticText* StaticText27;
    DragEffectBitmapButton* BitmapButton13;
    DragEffectBitmapButton* BitmapButton4;
    wxButton* ButtonAddE131;
    wxTextCtrl* TextCtrlFilename;
    DragEffectBitmapButton* BitmapButton2;
    wxMenuItem* MenuItemRenderEraseMode;
    wxMenuItem* MenuItem_File_Close_Sequence;
    wxFileDialog* FileDialogConvert;
    wxTimer Timer1;
    wxRadioButton* RadioButtonTwinkle50;
    wxRadioButton* RadioButtonRgbTwinkle10;
    DragEffectBitmapButton* BitmapButton26;
    wxMenuItem* MenuItem_ViewZoomOut;
    wxMenu* AudioMenu;
    wxPanel* PanelPapagayo;
    DragEffectBitmapButton* BitmapButton32;
    wxRadioButton* RadioButtonChase3;
    wxRadioButton* RadioButtonChase4;
    wxTextCtrl* TextCtrl_PgoMaxRest;
    wxButton* Button_pgo_filename;
    DragEffectBitmapButton* BitmapButton7;
    wxRadioButton* RadioButtonRgbChase4;
    wxButton* ButtonModelsPreview;
    DragEffectBitmapButton* BitmapButton24;
    wxButton* ButtonPreviewOpen;
    wxPanel* PanelTest;
    wxMenuItem* MenuItemBackup;
    DragEffectBitmapButton* BitmapButton23;
    wxButton* ButtonChooseFile;
    DragEffectBitmapButton* BitmapButton15;
    wxRadioButton* RadioButtonRgbDim;
    wxStaticText* StaticText5;
    wxMenuItem* MenuItemGridNodeValuesOn;
    wxMenuItem* MenuItemGridIconBackgroundOn;
    wxCheckBox* CheckBox_CoroPictureScaled;
    wxStaticText* StaticText25;
    wxCheckBox* MapLORChannelsWithNoNetwork;
    wxPanel* PanelPreview;
    wxStaticText* StaticText6;
    wxButton* ButtonTestClear;
    wxPanel* PanelConvert;
    DragEffectBitmapButton* BitmapButton28;
    wxTextCtrl* TextCtrl_PgoMinRest;
    wxButton* ButtonSaveSetup;
    DragEffectBitmapButton* BitmapButton20;
    wxButton* ButtonStartConversion;
    wxStaticText* StaticText19;
    wxPanel* PanelSequencer;
    wxMenuItem* MenuItemViewSavePerspective;
    wxButton* ButtonDeleteShow;
    wxButton* Button_CoroGroupClear;
    DragEffectBitmapButton* BitmapButton17;
    wxButton* ButtonSelectModelGroups;
    wxRadioButton* RadioButtonRgbShimmer;
    wxAuiManager* m_mgr;
    wxListBox* ListBoxElementList;
    wxSlider* Slider_BackgroundBrightness;
    wxPanel* PreviewGLPanel;
    wxStaticText* StaticText10;
    wxMenuItem* mRenderOnSaveMenuItem;
    wxButton* ButtonBuildWholeHouseModel;
    wxTextCtrl* TextCtrlPreviewElementSize;
    wxStaticText* StaticText35;
    wxMenuBar* MenuBar;
    wxChoice* ChoiceOutputFormat;
    DragEffectBitmapButton* BitmapButton12;
    wxDirDialog* DirDialog1;
    wxButton* ButtonAddNull;
    wxCheckListBox* CheckListBoxTestChannels;
    wxStaticText* StaticText33;
    wxStaticText* StaticText23;
    wxRadioButton* RadioButtonTwinkle10;
    wxStaticText* StaticText12;
    wxSlider* SliderPreviewRotate;
    wxTreeCtrl* ListBoxSched;
    wxNotebook* NotebookTest;
    DragEffectBitmapButton* BitmapButton14;
    wxRadioButton* RadioButtonTwinkle25;
    wxAuiManager* MainAuiManager;
    DragEffectBitmapButton* BitmapButton3;
    DragEffectBitmapButton* BitmapButton34;
    DragEffectBitmapButton* BitmapButton18;
    wxSlider* SliderChaseSpeed;
    wxButton* Button_PgoCopyVoices;
    wxCheckBox* CheckBoxMapEmptyChannels;
    wxMenu* MenuItemRenderMode;
    wxButton* ButtonShowDatesChange;
    wxButton* ButtonAddShow;
    wxRadioButton* RadioButtonDim;
    wxMenu* MenuItem15;
    wxButton* ButtonUpdateShow;
    wxStaticText* ShowDirectoryLabel;
    wxStaticText* StaticText7;
    wxMenu* MenuFile;
    DragEffectBitmapButton* BitmapButton33;
    wxButton* ButtonSetPreviewSize;
    wxStaticText* StaticText16;
    DragEffectBitmapButton* BitmapButton30;
    wxMenu* ToolIconSizeMenu;
    wxStaticText* StaticText_PgoOutputType;
    wxMenu* MenuItem1;
    wxMenuItem* MenuItemGridIconBackgroundOff;
    wxButton* ButtonSaveLog;
    wxPanel* PanelSetup;
    DragEffectBitmapButton* BitmapButton1;
    DragEffectBitmapButton* BitmapButton25;
    wxStaticText* StaticText43;
    wxStaticText* StaticText24;
    wxSlider* SliderFgColorB;
    wxButton* ButtonSetBackgroundImage;
    wxStaticText* StaticText65;
    wxRadioButton* RadioButtonShimmer;
    wxMenuItem* MenuItemSavePlaylists;
    wxStaticText* StaticText14;
    wxStaticText* StaticText34;
    wxSlider* SliderBgColorC;
    wxStatusBar* StatusBar1;
    wxTextCtrl* TextCtrlModelRotationDegrees;
    wxCheckBox* CheckBox_PgoAutoRest;
    wxPanel* Panel3;
    wxStaticText* StaticText18;
    xlAuiToolBar* EffectsToolBar;
    wxListCtrl* GridNetwork;
    DragEffectBitmapButton* BitmapButton9;
    wxSlider* SliderRgbChaseSpeed;
    wxButton* ButtonSavePreview;
    wxStaticText* StaticText13;
    wxStaticText* StaticTextPreviewRotation;
    wxSplitterWindow* SplitterWindow1;
    wxSlider* SliderFgIntensity;
    xlAuiToolBar* MainToolBar;
    DragEffectBitmapButton* BitmapButton10;
    wxPanel* PanelTestStandard;
    wxStaticText* StaticText20;
    wxButton* ButtonStartPapagayo;
    DragEffectBitmapButton* BitmapButton21;
    wxButton* Button_Change_Media_Dir;
    wxTextCtrl* TextCtrl_pgo_filename;
    wxCheckBox* CheckBox_CoroEyesRandomBlink;
    wxRadioButton* RadioButtonRgbCycle4;
    wxStaticText* StaticText31;
    wxChoice* Choice_PgoGroupName;
    wxRadioButton* RadioButtonRgbTwinkle05;
    wxAuiNotebook* Notebook1;
    wxBitmapButton* BitmapButton_SaveCoroGroup;
    wxStaticText* StaticText67;
    xlAuiToolBar* OutputToolBar;
    wxButton* ButtonTestSave;
    wxChoice* Choice_PgoOutputType;
    wxButton* Button_PgoStitch;
    wxMenuItem* MenuItem3;
    wxStaticText* StaticTextShowEnd;
    DragEffectBitmapButton* BitmapButton31;
    wxFileDialog* FileDialogPgoImage;
    DragEffectBitmapButton* BitmapButton8;
    wxStaticText* StaticTextCurrentPreviewSize;
    wxCheckBox* CheckBox_PgoAutoFade;
    wxTextCtrl* TextCtrl_papagayo_output_filename;
    wxStaticText* StaticText4;
    wxRadioButton* RadioButtonAlt;
    xlAuiToolBar* WindowMgmtToolbar;
    wxMenuItem* MenuItem_ViewZoomIn;
    wxPanel* PanelRgbCycle;
    wxRadioButton* RadioButtonRgbChase3;
    wxCheckBox* CheckBoxRunSchedule;
    wxBitmapButton* BitmapButtonMoveNetworkUp;
    wxBitmapButton* bbStop;
    wxBitmapButton* bbPlayPause;
    wxRadioButton* RadioButtonChase5;
    wxRadioButton* RadioButtonRgbCycleOff;
    wxPanel* PanelCal;
    wxMenuItem* QuitMenuItem;
    wxMenuItem* MenuItemRenderCanvasMode;
    wxMenuItem* MenuItem_File_Open_Sequence;
    wxStaticText* StaticText26;
    wxStaticText* StaticText15;
    wxStaticText* StaticText8;
    wxMenuItem* MenuItemRefresh;
    wxRadioButton* RadioButtonRgbAlt;
    wxStaticText* StaticText30;
    wxStaticText* StaticText29;
    wxSlider* SliderFgColorA;
    wxCheckBox* CheckBoxOffAtEnd;
    wxSlider* SliderRgbCycleSpeed;
    wxBitmapButton* BitmapButton_Link_Dirs;
    wxButton* ButtonNetworkDelete;
    DragEffectBitmapButton* BitmapButton5;
    wxMenuItem* Menu_Settings_Sequence;
    DragEffectBitmapButton* BitmapButton22;
    wxMenuItem* MenuItemGridNodeValuesOff;
    wxSlider* SliderPreviewScale;
    wxButton* ButtonTestLoad;
    wxRadioButton* RadioButtonOff;
    DragEffectBitmapButton* BitmapButton29;
    wxRadioButton* RadioButtonRgbTwinkle25;
    wxRadioButton* RadioButtonRgbChase5;
    wxTextCtrl* TextCtrlLog;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* MediaDirectoryLabel;
    wxTextCtrl* TextCtrlConversionStatus;
    wxSlider* SliderBgColorB;
    DragEffectBitmapButton* BitmapButton16;
    wxPanel* PanelTestRgb;
    wxButton* ButtonAddDongle;
    wxSplitterWindow* SplitterWindow3;
    wxRadioButton* RadioButtonChase;
    wxMenuItem* MenuItem_File_Save_Sequence;
    wxChoice* LORImportTimeResolution;
    wxStaticText* StaticText17;
    wxStaticText* StaticText11;
    wxMenu* MenuSettings;
    wxCheckBox* ScaleImageCheckbox;
    wxRadioButton* RadioButtonRgbCycle5;
    wxGrid* GridCoroFaces;
    wxStaticText* StaticTextShowStart;
    wxBitmapButton* BitmapButtonMoveNetworkDown;
    wxStaticText* StaticTextPreviewFileName;
    wxSlider* SliderPreviewTime;
    wxStaticText* StaticText9;
    wxMessageDialog* MessageDialog1;
    wxButton* ButtonNetworkDeleteAll;
    wxRadioButton* RadioButtonRgbChaseOff;
    wxButton* ButtonNetworkChange;
    wxCheckBox* CheckBox_CoroEyesRandomLR;
    wxTextCtrl* TextCtrlPreviewTime;
    wxMenu* GridSpacingMenu;
    DragEffectBitmapButton* BitmapButton11;
    wxRadioButton* RadioButtonRgbCycle3;
    wxSlider* SliderBgColorA;
    wxRadioButton* RadioButtonTwinkle05;
    wxStaticText* StaticText3;
    //*)

    AUIToolbarButtonWrapper *CheckBoxLightOutput;
    AUIToolbarButtonWrapper *ButtonGracefulStop;

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
    bool UnsavedChanges;
    int mSavedChangeCount;
    wxDateTime starttime;
    play_modes play_mode;
    NetInfoClass NetInfo;

    ModelPreview* modelPreview;

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
    wxString showDirectory;
    wxString mediaDirectory;
    SeqDataType SeqData;

    wxArrayString FileNames;
    wxArrayString ChannelNames;
    wxArrayInt ChannelColors;
    long seekPoint;

    int mHitTestNextSelectModelIndex;
    wxString mBackgroundImage;
    int mBackgroundBrightness;
    bool mScaleBackgroundImage = false;

    // convert
public:
    void ReadLorFile(const wxString& filename, int LORImportInterval);
    void WriteLorFile(const wxString& filename);  //      LOR *.lms, *.las

private:

    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString &VixChannelNames);
    void ReadVixFile(const wxString& filename);
    void ReadHLSFile(const wxString& filename);
    void ReadXlightsFile(const wxString& FileName, wxString *mediaFilename = NULL);
    void ReadFalconFile(const wxString& FileName);
    void ReadGlediatorFile(const wxString& FileName);
    void ReadConductorFile(const wxString& FileName);
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
    void WriteLcbFile(const wxString& filename);  //      LOR *.lcb
    void WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);  //      LOR *.lcb
    void ClearLastPeriod();
    void DoConversion(const wxString& FileName, const wxString& OutputFormat);
    bool mapEmptyChannels();
    bool isSetOffAtEnd();

    wxString base64_encode();

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
    void UpdateModelsList();
    void ChooseColor(wxTextCtrl* TextCtrl);
    void LoadSizerControlsToAttr(wxSizer* sizer,wxXmlNode* x);
    void PlayRgbEffect(int EffectPeriod, SettingsMap &SettingsMap);
    void TimerRgbSeq(long msec);
    void SetChoicebook(wxChoicebook* cb, const wxString& PageName);
    void UpdateGrid();
    void SetPreviewSize(int width,int height);
    void SetXmlSetting(const wxString& settingName,const wxString& value);
    wxString GetXmlSetting(const wxString& settingName,const wxString& defaultValue);

    wxString CreateEffectString();
    void OpenPaletteDialog(const wxString& id1, const wxString& id2, wxSizer* PrimarySizer,wxSizer* SecondarySizer);
    void ChooseModelsForSequence();
    void GetGridColumnLabels(wxArrayString& a);
    void GetModelNames(wxArrayString& a, bool includeGroups = false);
    wxXmlNode* CreateModelNodeFromGroup(const wxString &name);
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
    wxXmlNode* GetModelNode(const wxString& name);
    bool InitPixelBuffer(const wxString &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased = false);
    ModelClass &GetModelClass(const wxString& name);
    void RenderGridToSeqData();
    bool RenderEffectFromMap(Effect *effect, int layer, int period, const SettingsMap& SettingsMap,
                             PixelBufferClass &buffer, bool &ResetEffectState,
                             bool bgThread = false, RenderEvent *event = NULL);
    void RenderEffectOnMainThread(RenderEvent *evt);
    void RenderEffectForModel(const wxString &model, int startms, int endms, bool clear = false);

    void RenderRange(RenderCommandEvent &cmd);
    void RenderDone();

    void EnableSequenceControls(bool enable);
    SequenceElements& GetSequenceElements() { return mSequenceElements; }
    Element* AddTimingElement(wxString& name);
    void DeleteTimingElement(wxString& name);
    void RenameTimingElement(wxString& old_name, wxString& new_name);
    void ImportTimingElement();
    void ExecuteImportTimingElement(wxCommandEvent &command);
    void ConvertDataRowToEffects(wxCommandEvent &command);
    void ConvertDataRowToEffects(EffectLayer *layer, xlColorVector &colors, int frameTime);
    void PromoteEffects(wxCommandEvent &command);
    void PromoteEffects(Element *element);
    wxXmlNode* CreateEffectNode(wxString& name);
    void UpdateEffectNode(wxXmlNode* node);
    void ApplyEffectsPreset(wxString& data);

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
    wxString CreateEffectStringRandom(wxString &settings, wxString &palette);
    void BackupDirectory(wxString targetDirName);
    void NewSequence();
    void OpenSequence();
    void SaveSequence();
    bool CloseSequence();
    void InsertRow();
    void UpdatePreview();
    wxXmlNode *BuildWholeHouseModel(const wxString &modelName, std::vector<ModelClass*> &models);
    void ShowModelsDialog();
    void ShowPreviewTime(long ElapsedMSec);
    void PreviewOutput(int period);
    void TimerOutput(int period);
    void GetSeqModelNames(wxArrayString& a);
    void UpdateChannelNames();
    void StopNow(void);
    void PlayRgbSequence(void);
    bool IsValidEffectString(wxString& s);
    void PreviewScaleUpdated(float newscale);
    void LoadPapagayoFile(const wxString& filename, int frame_offset = 0);
    void InitPapagayoTab(bool tab_changed);
    bool LoadPgoSettings(void);
    bool SavePgoSettings(void);
    bool GetGroupName(wxString& grpname);
    void PgoGridCellSelect(int row, int col, int where);
    void GetMouthNodes(const wxString& model_name);
    void PreviewRotationUpdated(int newRotation);
    int FindModelsClicked(int x,int y,wxArrayInt* found);
    void SelectModel(wxString name);
    bool SelectSingleModel(int x,int y);
    bool SelectMultipleModels(int x,int y);
    void SetSelectedModelToGroupSelected();
    void UnSelectAllModels();
    int ModelsSelectedCount();
    void SelectAllInBoundingRect();
    void OnPreviewModelPopup(wxCommandEvent &event);
    void PreviewModelAlignTops();
    void PreviewModelAlignBottoms();
    void PreviewModelAlignLeft();
    void PreviewModelAlignRight();
    void PreviewModelAlignHCenter();
    void PreviewModelAlignVCenter();
    void ShowModelProperties();
    int GetSelectedModelIndex();
    void ShowSelectedModelGroups();
    void SetModelAsPartOfDisplay(wxString& model);


    wxXmlDocument pgoXml; //Papagayo settings from xlights_papagayo.xml
    bool Grid1HasFocus; //cut/copy/paste handled differently with grid vs. other text controls -DJ
    wxXmlDocument EffectsXml;
    wxXmlNode* EffectsNode;
    wxXmlNode* ModelsNode;
    wxXmlNode* PalettesNode;
    wxXmlNode* ViewsNode;
    wxXmlNode* PerspectivesNode;
    wxXmlNode* ModelGroupsNode;
    wxXmlNode* SettingsNode;

    bool MixTypeChanged;
    bool FadesChanged;
    long SeqBaseChannel;
    bool SeqChanCtrlBasic;
    bool SeqChanCtrlColor;

    bool mResetToolbars;
    bool mRenderOnSave;
    int mIconSize;
    int mGridSpacing;
    bool mGridIconBackgrounds;
    bool mGridNodeValues;

    PixelBufferClass playBuffer;
    int playType;
    int playStartMS;
    int playStartTime;
    int playOffsetTime;
    int playEndTime;
    bool replaySection;

    wxString selectedEffectName;
    wxString selectedEffectString;
    wxString selectedEffectPalette;
    Effect *selectedEffect;

    wxString lastPlayEffect;
    double mPointSize = 2.0;

//    std::vector<ModelClassPtr> PreviewModels;
    wxHtmlEasyPrinting* HtmlEasyPrint;
    int NextGridRowToPlay;
    int SeqPlayColumn;

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
    wxArrayString RippleObjectToDraw;
    wxArrayString RippleMovement;
    wxArrayString TextEffects;
    wxArrayString TextCountDown;
    wxArrayString WaveType;
    wxArrayString FillColors;
    wxArrayString WaveDirection;


    wxArrayString CurtainEdge;
    wxArrayString CurtainEffect;
    wxGridCellCoords *curCell;
    int PlaybackPeriod; //used to be able to record the frame being played in an animation preview

    bool m_dragging;
    bool m_resizing;
    bool m_rotating;
    bool m_creating_bound_rect;
    int  m_bound_start_x;
    int m_bound_start_y;
    int m_bound_end_x;
    int m_bound_end_y;
    int m_over_handle;
    int m_previous_mouse_x, m_previous_mouse_y;
    wxXmlNode* mCurrentPerpective;
    std::vector<bool> savedPaneShown;

    // New Sequencer variables and methods
    SequenceElements mSequenceElements;
    int mTextEntryContext;

    MainSequencer* mainSequencer;
    ModelPreview * sPreview1;
    ModelPreview * sPreview2;
    ColorPanel* colorPanel;
    TimingPanel* timingPanel;
    PerspectivesPanel* perspectivePanel;
    EffectIconPanel *effectPalettePanel;
    DisplayElementsPanel *displayElementsPanel;

    int mMediaLengthMS;
    bool mSequencerInitialize = false;
    // Methods
    void InitSequencer();
    void CreateSequencer();
    // Events
    void Zoom( wxCommandEvent& event);
    void TimelineChanged( wxCommandEvent& event);
    void RowHeadingsChanged( wxCommandEvent& event);
    void WindowResized( wxCommandEvent& event);
    void HorizontalScrollChanged( wxCommandEvent& event);
    void ScrollRight( wxCommandEvent& event);
    void VerticalScrollChanged( wxCommandEvent& event);
    void TimeSelected( wxCommandEvent& event);
    void SelectedEffectChanged( wxCommandEvent& event);
    void UnselectedEffect( wxCommandEvent& event);
    void EffectDroppedOnGrid(wxCommandEvent& event);
    void PlayModelEffect(wxCommandEvent& event);
    void UpdateEffect(wxCommandEvent& event);
    void ForceSequencerRefresh(wxCommandEvent& event);
    void LoadPerspective(wxCommandEvent& event);
    void PerspectivesChanged(wxCommandEvent& event);
    void PlayModel(wxCommandEvent& event);
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

    void CheckForAndCreateDefaultPerpective();
    void ZoomIn();
    void ZoomOut();
    void EffectsResize(wxSizeEvent& event);
    void EffectsPaint(wxPaintEvent& event);
    void ResizeAndMakeEffectsScroll();
    void ResizeMainSequencer();
    void UpdateEffectGridHorizontalScrollBar();
    void UpdateEffectGridVerticalScrollBar();
    wxString GetEffectTextFromWindows(wxString &palette);

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
    static std::vector<ModelClass *> PreviewModels;
    static std::map<wxString, ModelClassPtr> AllModels; //make public and static for easier access -DJ
    static wxXmlNode* FindNode(wxXmlNode* parent, const wxString& tag, const wxString& attr, const wxString& value, bool create = false);

    wxString GetSeqXmlFileName();

    EffectTreeDialog *EffectTreeDlg;

};

#endif // XLIGHTSMAIN_H
