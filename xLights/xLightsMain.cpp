/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
// Author:    Matt Brown (dowdybrown@yahoo.com)
// Created:   2012-11-03
// Copyright: Matt Brown ()

#include <wx/msgdlg.h>
#include <wx/artprov.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <wx/debugrpt.h>
#include <wx/protocol/http.h>
#include <wx/textctrl.h>
#include <wx/sstream.h>
#include <wx/regex.h>
#include <wx/grid.h>
#include <wx/mimetype.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/version.h>
#include <wx/tooltip.h>
#include <wx/taskbar.h>

#include <cctype>
#include <cstring>
#include <thread>

#include "xLightsMain.h"
#include "SplashDialog.h"
#include "UpdaterDialog.h"
#include "xLightsApp.h"
#include "SeqSettingsDialog.h"
#include "xLightsVersion.h"
#include "RenderCommandEvent.h"
#include "effects/RenderableEffect.h"
#include "LayoutPanel.h"
#include "models/ModelGroup.h"
#include "PixelTestDialog.h"
#include "ConvertDialog.h"
#include "GenerateCustomModelDialog.h"
#include "PathGenerationDialog.h"
#include "UtilFunctions.h"
#include "controllers/FPPConnectDialog.h"
#include "IPEntryDialog.h"
#include "HousePreviewPanel.h"
#include "BatchRenderDialog.h"
#include "VideoExporter.h"
#include "JukeboxPanel.h"
#include "FindDataPanel.h"
#include "EffectAssist.h"
#include "EffectsPanel.h"
#include "MultiControllerUploadDialog.h"
#include "Parallel.h"
#include "outputs/IPOutput.h"
#include "outputs/E131Output.h"
#include "GenerateLyricsDialog.h"
#include "VendorModelDialog.h"
#include "VendorMusicDialog.h"
#include "sequencer/MainSequencer.h"
#include "LayoutGroup.h"
#include "ModelPreview.h"
#include "TopEffectsPanel.h"
#include "LyricUserDictDialog.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/Falcon.h"
#include "controllers/ESPixelStick.h"
#include "outputs/ZCPPOutput.h"
#include "EffectIconPanel.h"
#include "models/ViewObject.h"
#include "models/SubModel.h"
#include "effects/FacesEffect.h"
#include "effects/StateEffect.h"
#include "effects/ShaderEffect.h"
#include "ShaderDownloadDialog.h"
#include "CheckboxSelectDialog.h"
#include "EmailDialog.h"
#include "ValueCurveButton.h"
#include "ValueCurvesPanel.h"
#include "ColoursPanel.h"
#include "SpecialOptions.h"
#include "controllers/HinksPixExportDialog.h"
#include "controllers/ControllerCaps.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "KeyBindingEditDialog.h"
#include "TraceLog.h"
#include "AboutDialog.h"
#include "ExternalHooks.h"
#include "ExportSettings.h"
#include "GPURenderUtils.h"
#include "ViewsModelsPanel.h"
#include "graphics/opengl/xlGLCanvas.h"
#include "ColourReplaceDialog.h"
#include "ModelRemap.h"
#include "RestoreBackupDialog.h"
#include "utils/ip_utils.h"

#include "../xSchedule/wxHTTPServer/wxhttpserver.h"

// Linux needs this
#include <wx/stdpaths.h>

// image files
#include "../include/control-pause-blue-icon.xpm"
#include "../include/control-play-blue-icon.xpm"

#include <xlsxwriter.h>

//(*InternalHeaders(xLightsFrame)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#define TOOLBAR_SAVE_VERSION "0003:"
#define MAXBACKUPFILE_MB 30


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
const long xLightsFrame::ID_CHECKBOX_LIGHT_OUTPUT = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_PLAY = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM2 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM5 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM7 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM3 = wxNewId();
const long xLightsFrame::ID_TOGGLE_MODEL_PREVIEW = wxNewId();
const long xLightsFrame::ID_TOGGLE_HOUSE_PREVIEW = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM6 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM8 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM9 = wxNewId();
const long xLightsFrame::ID_AUIWINDOWTOOLBAR = wxNewId();
const long xLightsFrame::ID_PASTE_BY_TIME = wxNewId();
const long xLightsFrame::ID_PASTE_BY_CELL = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_EDIT = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACDISABLED = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACSELECT = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACOFF = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACON = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACSHIMMER = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACTWINKLE = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACINTENSITY = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACRAMPUP = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACRAMPDOWN = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACRAMPUPDOWN = wxNewId();
const long xLightsFrame::ID_CHOICE_PARM1 = wxNewId();
const long xLightsFrame::ID_CHOICE_PARM2 = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACFILL = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACCASCADE = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACFOREGROUND = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM_ACBACKGROUND = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_AC = wxNewId();
const long xLightsFrame::ID_AUITOOLBARITEM14 = wxNewId();
const long xLightsFrame::ID_AUITOOLBAR_VIEW = wxNewId();
const long xLightsFrame::ID_AUIEFFECTSTOOLBAR = wxNewId();
const long xLightsFrame::ID_BUTTON3 = wxNewId();
const long xLightsFrame::ID_BUTTON11 = wxNewId();
const long xLightsFrame::ID_BUTTON13 = wxNewId();
const long xLightsFrame::ID_STATICTEXT4 = wxNewId();
const long xLightsFrame::ID_BUTTON_SAVE_SETUP = wxNewId();
const long xLightsFrame::ID_BUTTON9 = wxNewId();
const long xLightsFrame::ID_BUTTON6 = wxNewId();
const long xLightsFrame::ID_BUTTON10 = wxNewId();
const long xLightsFrame::ID_BUTTON5 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON1 = wxNewId();
const long xLightsFrame::ID_BITMAPBUTTON2 = wxNewId();
const long xLightsFrame::ID_PANEL2 = wxNewId();
const long xLightsFrame::ID_BUTTON1 = wxNewId();
const long xLightsFrame::ID_BUTTON2 = wxNewId();
const long xLightsFrame::ID_BUTTON4 = wxNewId();
const long xLightsFrame::ID_BUTTON7 = wxNewId();
const long xLightsFrame::ID_BUTTON12 = wxNewId();
const long xLightsFrame::ID_BUTTON8 = wxNewId();
const long xLightsFrame::ID_STATICTEXT1 = wxNewId();
const long xLightsFrame::ID_PANEL6 = wxNewId();
const long xLightsFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xLightsFrame::ID_PANEL_SETUP = wxNewId();
const long xLightsFrame::ID_PANEL_PREVIEW = wxNewId();
const long xLightsFrame::XLIGHTS_SEQUENCER_TAB = wxNewId();
const long xLightsFrame::ID_NOTEBOOK1 = wxNewId();
const long xLightsFrame::ID_STATICTEXT6 = wxNewId();
const long xLightsFrame::ID_GAUGE1 = wxNewId();
const long xLightsFrame::ID_PANEL5 = wxNewId();
const long xLightsFrame::ID_STATICTEXT7 = wxNewId();
const long xLightsFrame::ID_PANEL1 = wxNewId();
const long xLightsFrame::ID_NEW_SEQUENCE = wxNewId();
const long xLightsFrame::ID_OPEN_SEQUENCE = wxNewId();
const long xLightsFrame::ID_MENUITEM4 = wxNewId();
const long xLightsFrame::ID_MENUITEM_OPENRECENTSEQUENCE = wxNewId();
const long xLightsFrame::IS_SAVE_SEQ = wxNewId();
const long xLightsFrame::ID_SAVE_AS_SEQUENCE = wxNewId();
const long xLightsFrame::ID_CLOSE_SEQ = wxNewId();
const long xLightsFrame::ID_SEQ_SETTINGS = wxNewId();
const long xLightsFrame::ID_MNU_KEYBINDINGS = wxNewId();
const long xLightsFrame::ID_EXPORT_VIDEO = wxNewId();
const long xLightsFrame::ID_MENUITEM2 = wxNewId();
const long xLightsFrame::ID_MENUITEM8 = wxNewId();
const long xLightsFrame::ID_MENUITEM_RECENTFOLDERS = wxNewId();
const long xLightsFrame::ID_FILE_BACKUP = wxNewId();
const long xLightsFrame::ID_FILE_RESTOREBACKUP = wxNewId();
const long xLightsFrame::ID_FILE_ALTBACKUP = wxNewId();
const long xLightsFrame::ID_SHIFT_EFFECTS = wxNewId();
const long xLightsFrame::ID_MNU_SHIFT_SELECTED_EFFECTS = wxNewId();
const long xLightsFrame::ID_MNU_COLOURREPLACE = wxNewId();
const long xLightsFrame::ID_MENUITEM13 = wxNewId();
const long xLightsFrame::ID_MNU_CHECKSEQ = wxNewId();
const long xLightsFrame::ID_MNU_CLEANUPFILE = wxNewId();
const long xLightsFrame::ID_MNU_PACKAGESEQUENCE = wxNewId();
const long xLightsFrame::ID_MNU_DOWNLOADSEQUENCES = wxNewId();
const long xLightsFrame::ID_MENU_BATCH_RENDER = wxNewId();
const long xLightsFrame::ID_MENU_FPP_CONNECT = wxNewId();
const long xLightsFrame::ID_MNU_BULKUPLOAD = wxNewId();
const long xLightsFrame::ID_MENU_HINKSPIX_EXPORT = wxNewId();
const long xLightsFrame::ID_MENU_RUN_SCRIPT = wxNewId();
const long xLightsFrame::ID_EXPORT_MODELS = wxNewId();
const long xLightsFrame::ID_MNU_EXPORT_EFFECTS = wxNewId();
const long xLightsFrame::ID_MNU_EXPORT_CONTROLLER_CONNECTIONS = wxNewId();
const long xLightsFrame::ID_MENU_VIEW_LOG = wxNewId();
const long xLightsFrame::ID_MENUITEM18 = wxNewId();
const long xLightsFrame::iD_MNU_VENDORCACHEPURGE = wxNewId();
const long xLightsFrame::ID_MNU_PURGERENDERCACHE = wxNewId();
const long xLightsFrame::ID_MNU_CRASH = wxNewId();
const long xLightsFrame::ID_MNU_DUMPRENDERSTATE = wxNewId();
const long xLightsFrame::ID_MENU_GENERATE2DPATH = wxNewId();
const long xLightsFrame::ID_MENUITEM_GenerateCustomModel = wxNewId();
const long xLightsFrame::ID_MNU_REMAPCUSTOM = wxNewId();
const long xLightsFrame::ID_MNU_GENERATELYRICS = wxNewId();
const long xLightsFrame::ID_MENUITEM_CONVERT = wxNewId();
const long xLightsFrame::ID_MNU_PREPAREAUDIO = wxNewId();
const long xLightsFrame::ID_MENU_USER_DICT = wxNewId();
const long xLightsFrame::ID_MNU_XSCHEDULE = wxNewId();
const long xLightsFrame::ID_MENU_XCAPTURE = wxNewId();
const long xLightsFrame::ID_MNU_XSCANNER = wxNewId();
const long xLightsFrame::ID_MENUITEM5 = wxNewId();
const long xLightsFrame::MNU_ID_ACLIGHTS = wxNewId();
const long xLightsFrame::ID_MNU_SHOWRAMPS = wxNewId();
const long xLightsFrame::ID_MENUITEM_SAVE_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM_SAVE_AS_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM_LOAD_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MNU_PERSPECTIVES_AUTOSAVE = wxNewId();
const long xLightsFrame::ID_MENUITEM7 = wxNewId();
const long xLightsFrame::ID_MENUITEM_DISPLAY_ELEMENTS = wxNewId();
const long xLightsFrame::ID_MENU_TOGGLE_MODEL_PREVIEW = wxNewId();
const long xLightsFrame::ID_MENU_TOGGLE_HOUSE_PREVIEW = wxNewId();
const long xLightsFrame::ID_MENUITEM14 = wxNewId();
const long xLightsFrame::ID_MENUITEM15 = wxNewId();
const long xLightsFrame::ID_MENUITEM16 = wxNewId();
const long xLightsFrame::ID_MENUITEM9 = wxNewId();
const long xLightsFrame::ID_MENUITEM17 = wxNewId();
const long xLightsFrame::ID_MNU_VALUECURVES = wxNewId();
const long xLightsFrame::ID_MNU_COLOURDROPPER = wxNewId();
const long xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_WINDOW = wxNewId();
const long xLightsFrame::ID_MENUITEM_SELECT_EFFECT = wxNewId();
const long xLightsFrame::ID_MENUITEM_SEARCH_EFFECTS = wxNewId();
const long xLightsFrame::ID_MENUITEM_VIDEOPREVIEW = wxNewId();
const long xLightsFrame::ID_MNU_JUKEBOX = wxNewId();
const long xLightsFrame::ID_MNU_FINDDATA = wxNewId();
const long xLightsFrame::ID_MNU_SUPPRESSDOCK_HP = wxNewId();
const long xLightsFrame::ID_MNU_SUPPRESSDOCK_MP = wxNewId();
const long xLightsFrame::ID_MENUITEM3 = wxNewId();
const long xLightsFrame::ID_MENUITEM_WINDOWS_PERSPECTIVE = wxNewId();
const long xLightsFrame::ID_MENUITEM_WINDOWS_DOCKALL = wxNewId();
const long xLightsFrame::ID_MENUITEM11 = wxNewId();
const long xLightsFrame::ID_MENUITEM10 = wxNewId();
const long xLightsFrame::ID_PLAY_FULL = wxNewId();
const long xLightsFrame::ID_MNU_1POINT5SPEED = wxNewId();
const long xLightsFrame::ID_MN_2SPEED = wxNewId();
const long xLightsFrame::ID_MNU_3SPEED = wxNewId();
const long xLightsFrame::ID_MNU_4SPEED = wxNewId();
const long xLightsFrame::ID_PLAY_3_4 = wxNewId();
const long xLightsFrame::ID_PLAY_1_2 = wxNewId();
const long xLightsFrame::ID_PLAY_1_4 = wxNewId();
const long xLightsFrame::ID_MNU_LOUDVOLUME = wxNewId();
const long xLightsFrame::ID_MNU_MEDVOLUME = wxNewId();
const long xLightsFrame::ID_MNU_QUIET = wxNewId();
const long xLightsFrame::ID_MNU_SUPERQUIET = wxNewId();
const long xLightsFrame::ID_MNU_SILENT = wxNewId();
const long xLightsFrame::ID_IMPORT_EFFECTS = wxNewId();
const long xLightsFrame::ID_MNU_TOD = wxNewId();
const long xLightsFrame::ID_MNU_MANUAL = wxNewId();
const long xLightsFrame::ID_MNU_ZOOM = wxNewId();
const long xLightsFrame::ID_MENUITEM1 = wxNewId();
const long xLightsFrame::idMenuHelpContent = wxNewId();
const long xLightsFrame::ID_MENU_HELP_FORMUM = wxNewId();
const long xLightsFrame::ID_MNU_VIDEOS = wxNewId();
const long xLightsFrame::ID_MENU_HELP_DOWNLOAD = wxNewId();
const long xLightsFrame::ID_MNU_HELP_RELEASE_NOTES = wxNewId();
const long xLightsFrame::ID_MENU_HELP_ISSUE = wxNewId();
const long xLightsFrame::ID_MENU_HELP_FACEBOOK = wxNewId();
const long xLightsFrame::ID_MNU_DONATE = wxNewId();
const long xLightsFrame::ID_MNU_UPDATE = wxNewId();
const long xLightsFrame::ID_TIMER_OutputTimer = wxNewId();
const long xLightsFrame::ID_TIMER_AutoSave = wxNewId();
const long xLightsFrame::ID_TIMER_EFFECT_SETTINGS = wxNewId();
const long xLightsFrame::ID_TIMER_RENDERSTATUS = wxNewId();
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

const long xLightsFrame::ID_XFADESERVER = wxNewId();
const long xLightsFrame::ID_XFADESOCKET = wxNewId();

const long xLightsFrame::ID_MENU_ITEM_PREVIEWS = wxNewId();
const long xLightsFrame::ID_MENU_ITEM_PREVIEWS_SHOW_ALL = wxNewId();

wxDEFINE_EVENT(EVT_ZOOM, wxCommandEvent);
wxDEFINE_EVENT(EVT_SCRUB, wxCommandEvent);
wxDEFINE_EVENT(EVT_GSCROLL, wxCommandEvent);
wxDEFINE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_MOUSE_POSITION, wxCommandEvent);
wxDEFINE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SELECTED_ROW_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_UNSELECTED_EFFECT, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_MODEL_EFFECT, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_DROPPED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECTFILE_DROPPED, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_UPDATED, wxCommandEvent);
wxDEFINE_EVENT(EVT_UPDATE_EFFECT, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_RANDOMIZE, wxCommandEvent);
wxDEFINE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);
wxDEFINE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDEFINE_EVENT(EVT_LOAD_PERSPECTIVE, wxCommandEvent);
wxDEFINE_EVENT(EVT_PERSPECTIVES_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SAVE_PERSPECTIVES, wxCommandEvent);
wxDEFINE_EVENT(EVT_EXPORT_MODEL, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_MODEL, wxCommandEvent);
wxDEFINE_EVENT(EVT_CUT_MODEL_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_COPY_MODEL_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PASTE_MODEL_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_PAUSE_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);
wxDEFINE_EVENT(EVT_STOP_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_FIRST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_LAST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_REWIND10, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_FFORWARD10, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_SEEKTO, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_REPLAY_SECTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMPORT_NOTES, wxCommandEvent);
wxDEFINE_EVENT(EVT_CONVERT_DATA_TO_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PROMOTE_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_RGBEFFECTS_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_RENDER_RANGE, RenderCommandEvent);
wxDEFINE_EVENT(EVT_APPLYLAST, wxCommandEvent);
wxDEFINE_EVENT(EVT_SELECTED_EFFECT_CHANGED, SelectedEffectChangedEvent);
wxDEFINE_EVENT(EVT_TURNONOUTPUTTOLIGHTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAYJUKEBOXITEM, wxCommandEvent);
wxDEFINE_EVENT(EVT_COLOUR_CHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SETEFFECTCHOICE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TIPOFDAY_READY, wxCommandEvent);
wxDEFINE_EVENT(EVT_SET_EFFECT_DURATION, wxCommandEvent);

BEGIN_EVENT_TABLE(xLightsFrame,wxFrame)
    //(*EventTable(xLightsFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SCRUB, xLightsFrame::Scrub)
    EVT_COMMAND(wxID_ANY, EVT_ZOOM, xLightsFrame::Zoom)
    EVT_COMMAND(wxID_ANY, EVT_GSCROLL, xLightsFrame::Scroll)
    EVT_COMMAND(wxID_ANY, EVT_TIME_SELECTED, xLightsFrame::TimeSelected)
    EVT_COMMAND(wxID_ANY, EVT_MOUSE_POSITION, xLightsFrame::MousePositionUpdated)
    EVT_COMMAND(wxID_ANY, EVT_ROW_HEADINGS_CHANGED, xLightsFrame::RowHeadingsChanged)
    EVT_COMMAND(wxID_ANY, EVT_WINDOW_RESIZED, xLightsFrame::WindowResized)
    EVT_COMMAND(wxID_ANY, EVT_SELECTED_ROW_CHANGED, xLightsFrame::SelectedRowChanged)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_CHANGED, xLightsFrame::EffectChanged)
    EVT_COMMAND(wxID_ANY, EVT_UPDATE_EFFECT, xLightsFrame::EffectUpdated)
    EVT_COMMAND(wxID_ANY, EVT_UNSELECTED_EFFECT, xLightsFrame::UnselectedEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_DROPPED, xLightsFrame::EffectDroppedOnGrid)
    EVT_COMMAND(wxID_ANY, EVT_EFFECTFILE_DROPPED, xLightsFrame::EffectFileDroppedOnGrid)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_MODEL_EFFECT, xLightsFrame::PlayModelEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_UPDATED, xLightsFrame::UpdateEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_RANDOMIZE, xLightsFrame::RandomizeEffect)
    EVT_COMMAND(wxID_ANY, EVT_EFFECT_PALETTE_UPDATED, xLightsFrame::UpdateEffectPalette)
    EVT_COMMAND(wxID_ANY, EVT_FORCE_SEQUENCER_REFRESH, xLightsFrame::ForceSequencerRefresh)
    EVT_COMMAND(wxID_ANY, EVT_LOAD_PERSPECTIVE, xLightsFrame::LoadPerspective)
    EVT_COMMAND(wxID_ANY, EVT_SAVE_PERSPECTIVES, xLightsFrame::OnMenuItemViewSavePerspectiveSelected)
    EVT_COMMAND(wxID_ANY, EVT_PERSPECTIVES_CHANGED, xLightsFrame::PerspectivesChanged)
    EVT_COMMAND(wxID_ANY, EVT_EXPORT_MODEL, xLightsFrame::ExportModel)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_MODEL, xLightsFrame::PlayModel)
    EVT_COMMAND(wxID_ANY, EVT_CUT_MODEL_EFFECTS, xLightsFrame::CutModelEffects)
    EVT_COMMAND(wxID_ANY, EVT_COPY_MODEL_EFFECTS, xLightsFrame::CopyModelEffects)
    EVT_COMMAND(wxID_ANY, EVT_PASTE_MODEL_EFFECTS, xLightsFrame::PasteModelEffects)
    EVT_COMMAND(wxID_ANY, EVT_MODEL_SELECTED, xLightsFrame::ModelSelected)
    EVT_COMMAND(wxID_ANY, EVT_PLAY_SEQUENCE, xLightsFrame::PlaySequence)
    EVT_COMMAND(wxID_ANY, EVT_PAUSE_SEQUENCE, xLightsFrame::PauseSequence)
    EVT_COMMAND(wxID_ANY, EVT_STOP_SEQUENCE, xLightsFrame::StopSequence)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_FIRST_FRAME, xLightsFrame::SequenceFirstFrame)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_LAST_FRAME, xLightsFrame::SequenceLastFrame)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_REWIND10, xLightsFrame::SequenceRewind10)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_FFORWARD10, xLightsFrame::SequenceFForward10)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_SEEKTO, xLightsFrame::SequenceSeekTo)
    EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_REPLAY_SECTION, xLightsFrame::SequenceReplaySection)
    EVT_COMMAND(wxID_ANY, EVT_TOGGLE_PLAY, xLightsFrame::TogglePlay)
    EVT_COMMAND(wxID_ANY, EVT_SHOW_DISPLAY_ELEMENTS, xLightsFrame::ShowDisplayElements)
    EVT_COMMAND(wxID_ANY, EVT_IMPORT_TIMING, xLightsFrame::ExecuteImportTimingElement)
    EVT_COMMAND(wxID_ANY, EVT_IMPORT_NOTES, xLightsFrame::ExecuteImportNotes)
    EVT_COMMAND(wxID_ANY, EVT_CONVERT_DATA_TO_EFFECTS, xLightsFrame::ConvertDataRowToEffects)
    EVT_COMMAND(wxID_ANY, EVT_PROMOTE_EFFECTS, xLightsFrame::PromoteEffects)
    EVT_COMMAND(wxID_ANY, EVT_APPLYLAST, xLightsFrame::ApplyLast)
    EVT_COMMAND(wxID_ANY, EVT_RGBEFFECTS_CHANGED, xLightsFrame::PerspectivesChanged)
    wx__DECLARE_EVT1(EVT_RENDER_RANGE, wxID_ANY, &xLightsFrame::RenderRange)
    wx__DECLARE_EVT1(EVT_SELECTED_EFFECT_CHANGED, wxID_ANY, &xLightsFrame::SelectedEffectChanged)
    EVT_COMMAND(29898, EVT_TURNONOUTPUTTOLIGHTS, xLightsFrame::TurnOnOutputToLights)
    EVT_COMMAND(29899, EVT_PLAYJUKEBOXITEM, xLightsFrame::PlayJukeboxItem)
    EVT_COMMAND(wxID_ANY, EVT_VC_CHANGED, xLightsFrame::VCChanged)
    EVT_COMMAND(wxID_ANY, EVT_COLOUR_CHANGED, xLightsFrame::ColourChanged)
    EVT_COMMAND(wxID_ANY, EVT_SETEFFECTCHOICE, xLightsFrame::SetEffectChoice)
    EVT_COMMAND(wxID_ANY, EVT_TIPOFDAY_READY, xLightsFrame::TipOfDayReady)
    EVT_COMMAND(wxID_ANY, EVT_SET_EFFECT_DURATION, xLightsFrame::SetEffectDuration)
    EVT_SYS_COLOUR_CHANGED(xLightsFrame::OnSysColourChanged)
END_EVENT_TABLE()

void AddEffectToolbarButtons(EffectManager& manager, xlAuiToolBar* EffectsToolBar)
{
    int size = ScaleWithSystemDPI(16);
    for (size_t x = 0; x < manager.size(); ++x) {
        DragEffectBitmapButton* bitmapButton = new DragEffectBitmapButton(EffectsToolBar, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(size, size),
                                                                            wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, wxString::Format("DragTBButton%02llu", x));
        bitmapButton->SetMinSize(wxSize(size, size));
        bitmapButton->SetMaxSize(wxSize(size, size));
        bitmapButton->SetEffect(manager[x], 16);
        bitmapButton->SetBitmapMargins(0, 0);
        EffectsToolBar->AddControl(bitmapButton, bitmapButton->GetToolTipText());

        EffectsToolBar->FindToolByIndex(x)->SetMinSize(wxSize(size, size));
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetSizeHints(size, size, size, size);
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetMinSize(wxSize(size, size));
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetMaxSize(wxSize(size, size));
    }
    EffectsToolBar->Realize();
}

inline wxBitmapBundle GetToolbarBitmapBundle(const wxString &id)  {
    return wxArtProvider::GetBitmapBundle(id, wxART_TOOLBAR);
}

inline wxBitmapBundle GetMenuItemBitmapBundle(const wxString &id)  {
    return wxArtProvider::GetBitmapBundle(id, wxART_MENU);
}

inline wxBitmapBundle GetOtherBitmapBundle(const wxString &id)  {
    return wxArtProvider::GetBitmapBundle(id, wxART_OTHER);
}

inline wxBitmapBundle GetButtonBitmapBundle(const wxString &id)  {
    return wxArtProvider::GetBitmapBundle(id, wxART_BUTTON);
}

#ifdef __WXOSX__
const long NEWINSTANCE_ID = wxNewId();

class xlMacDockIcon : public wxTaskBarIcon {
public:
    xlMacDockIcon(xLightsFrame*f) : wxTaskBarIcon(wxTBI_DOCK), _frame(f) {
    }
    
    
    virtual wxMenu *CreatePopupMenu() override {
        wxMenu *menu = new wxMenu;
        menu->Append(NEWINSTANCE_ID, "Open New Instance");
        return menu;
    }

    void OnMenuOpenNewInstance(wxCommandEvent&e) {
        _frame->OnMenuItem_File_NewXLightsInstance(e);
    }
    
    xLightsFrame *_frame;
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(xlMacDockIcon, wxTaskBarIcon)
    EVT_MENU(NEWINSTANCE_ID, xlMacDockIcon::OnMenuOpenNewInstance)
wxEND_EVENT_TABLE()
#endif

xLightsFrame::xLightsFrame(wxWindow* parent, int ab, wxWindowID id) :
    _sequenceElements(this),
    jobPool("RenderPool"),
    AllModels(&_outputManager, this),
    AllObjects(this),
    _presetSequenceElements(this), color_mgr(this)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xLightsFrame being constructed.");

    xLightsApp::__frame = this;

    ValueCurve::SetSequenceElements(&_sequenceElements);

    _exiting = false;
    SplashDialog splash(nullptr);
    splash.Show();
    splash.Update();
    wxYield();

    _fps = -1;
    mCurrentPerpective = nullptr;
    MenuItemPreviews = nullptr;
    _renderMode = false;
    _checkSequenceMode = false;
    _suspendAutoSave = false;
	_sequenceViewManager.SetModelManager(&AllModels);

    Bind(EVT_SELECTED_EFFECT_CHANGED, &xLightsFrame::SelectedEffectChanged, this);
    Bind(EVT_RENDER_RANGE, &xLightsFrame::RenderRange, this);
    wxHTTP::Initialize();

    //(*Initialize(xLightsFrame)
    wxBoxSizer* BoxSizer1;
    wxButton* Button03;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizerPreview;
    wxFlexGridSizer* FlexGridSizerSetup;
    wxGridBagSizer* GridBagSizer1;
    wxMenu* Menu2;
    wxMenu* MenuHelp;
    wxMenuItem* MenuItem11;
    wxMenuItem* MenuItem13;
    wxMenuItem* MenuItem21;
    wxMenuItem* MenuItem26;
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem30;
    wxMenuItem* MenuItem31;
    wxMenuItem* MenuItem4;
    wxMenuItem* MenuItem5;
    wxMenuItem* MenuItem61;
    wxMenuItem* MenuItem7;
    wxMenuItem* MenuItem8;
    wxMenuItem* MenuItem9;
    wxMenuItem* MenuItemBatchRender;
    wxPanel* Panel1;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxStaticText* StaticText38;

    Create(parent, wxID_ANY, _("<use variables in xLightsMain.h>"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(1411,1103));
    MainAuiManager = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_DEFAULT);
    MainToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_MAIN, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPENSHOW, _("Open Show Directory"), GetToolbarBitmapBundle("wxART_FOLDER_OPEN"), wxNullBitmap, wxITEM_NORMAL, _("Select Show Directory"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_NEWSEQUENCE, _("New Sequence"), GetToolbarBitmapBundle("wxART_NEW"), wxNullBitmap, wxITEM_NORMAL, _("New Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_OPEN, _("Open Sequence"), GetToolbarBitmapBundle("wxART_FILE_OPEN"), wxNullBitmap, wxITEM_NORMAL, _("Open Sequence"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVE, _("Save"), GetToolbarBitmapBundle("wxART_FILE_SAVE"), wxNullBitmap, wxITEM_NORMAL, _("Save"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_SAVEAS, _("Save As"), GetToolbarBitmapBundle("wxART_FILE_SAVE_AS"), wxNullBitmap, wxITEM_NORMAL, _("Save As"), wxEmptyString, NULL);
    MainToolBar->AddTool(ID_AUITOOLBAR_RENDERALL, _("Render All"), GetToolbarBitmapBundle("xlART_RENDER_ALL"), wxNullBitmap, wxITEM_NORMAL, _("Render All"), wxEmptyString, NULL);
    MainToolBar->Realize();
    MainAuiManager->AddPane(MainToolBar, wxAuiPaneInfo().Name(_T("Main Tool Bar")).ToolbarPane().Caption(_("Main Tool Bar")).CloseButton(false).Layer(10).Top().Gripper());
    PlayToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_PLAY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    PlayToolBar->AddTool(ID_AUITOOLBAR_PLAY_NOW, _("Play"), GetToolbarBitmapBundle("xlART_PLAY"), wxNullBitmap, wxITEM_NORMAL, _("Play"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_PAUSE, _("Pause"), GetToolbarBitmapBundle("xlART_PAUSE"), wxNullBitmap, wxITEM_NORMAL, _("Pause"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_STOP, _("Stop"), GetToolbarBitmapBundle("xlART_STOP"), wxNullBitmap, wxITEM_NORMAL, _("Stop"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_FIRST_FRAME, _("Item label"), GetToolbarBitmapBundle("xlART_BACKWARD"), wxNullBitmap, wxITEM_NORMAL, _("First Frame"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_LAST_FRAME, _("Item label"), GetToolbarBitmapBundle("xlART_FORWARD"), wxNullBitmap, wxITEM_NORMAL, _("Last Frame"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_AUITOOLBAR_REPLAY_SECTION, _("Item label"), GetToolbarBitmapBundle("xlART_REPLAY"), wxNullBitmap, wxITEM_NORMAL, _("Replay Section"), wxEmptyString, NULL);
    PlayToolBar->AddTool(ID_CHECKBOX_LIGHT_OUTPUT, _("Output To Lights"), GetToolbarBitmapBundle("xlART_OUTPUT_LIGHTS"), wxNullBitmap, wxITEM_CHECK, _("Output To Lights"), wxEmptyString, NULL);
    PlayToolBar->Realize();
    MainAuiManager->AddPane(PlayToolBar, wxAuiPaneInfo().Name(_T("Play Tool Bar")).ToolbarPane().Caption(_("Play Tool Bar")).CloseButton(false).Layer(10).Position(11).Top().Gripper());
    WindowMgmtToolbar = new xlAuiToolBar(this, ID_AUIWINDOWTOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM2, _("Effect Settings"), GetToolbarBitmapBundle("xlART_EFFECTSETTINGS"), wxNullBitmap, wxITEM_CHECK, _("Effect Settings"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM5, _("Effect Colors"), GetToolbarBitmapBundle("xlART_COLORS"), wxNullBitmap, wxITEM_CHECK, _("Effect Colors"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM7, _("Layer Settings"), GetToolbarBitmapBundle("xlART_LAYERS"), wxNullBitmap, wxITEM_NORMAL, _("Layer Settings"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM3, _("Layer Blending"), GetToolbarBitmapBundle("xlART_LAYERS2"), wxNullBitmap, wxITEM_NORMAL, _("Layer Blending"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_TOGGLE_MODEL_PREVIEW, _("Model Preview"), GetToolbarBitmapBundle("xlART_MODEL_PREVIEW"), wxNullBitmap, wxITEM_NORMAL, _("Model Preview"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_TOGGLE_HOUSE_PREVIEW, _("House Preview"), GetToolbarBitmapBundle("xlART_HOUSE_PREVIEW"), wxNullBitmap, wxITEM_NORMAL, _("House Preview"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM6, _("Models"), GetToolbarBitmapBundle("xlART_SEQUENCE_ELEMENTS"), wxNullBitmap, wxITEM_NORMAL, _("Display Elements"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM8, _("Effects"), GetToolbarBitmapBundle("xlART_EFFECTS"), wxNullBitmap, wxITEM_NORMAL, _("Effects"), wxEmptyString, NULL);
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM9, _("Effects Assistant"), GetToolbarBitmapBundle("xlART_EFFECTASSISTANT"), wxNullBitmap, wxITEM_NORMAL, _("Effects Assistant"), wxEmptyString, NULL);
    WindowMgmtToolbar->Realize();
    MainAuiManager->AddPane(WindowMgmtToolbar, wxAuiPaneInfo().Name(_T("Windows Tool Bar")).ToolbarPane().Caption(_("Windows Tool Bar")).CloseButton(false).Layer(10).Position(12).Top().Gripper());
    EditToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_EDIT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    EditToolBar->AddTool(ID_PASTE_BY_TIME, _("Paste By Time"), GetToolbarBitmapBundle("xlART_PASTE_BY_TIME"), wxNullBitmap, wxITEM_CHECK, _("Paste By Time"), wxEmptyString, NULL);
    EditToolBar->AddTool(ID_PASTE_BY_CELL, _("Paste By Cell"), GetToolbarBitmapBundle("xlART_PASTE_BY_CELL"), wxNullBitmap, wxITEM_CHECK, _("Paste By Cell"), wxEmptyString, NULL);
    EditToolBar->Realize();
    MainAuiManager->AddPane(EditToolBar, wxAuiPaneInfo().Name(_T("Edit Tool Bar")).ToolbarPane().Caption(_("Pane caption")).CloseButton(false).Layer(10).Position(5).Top().Gripper());
    ACToolbar = new xlAuiToolBar(this, ID_AUITOOLBAR_AC, wxPoint(1,30), wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    ChoiceParm1 = new wxChoice(ACToolbar, ID_CHOICE_PARM1, wxPoint(276,12), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PARM1"));
    ChoiceParm1->SetSelection( ChoiceParm1->Append(_("0")) );
    ChoiceParm1->Append(_("10"));
    ChoiceParm1->Append(_("20"));
    ChoiceParm1->Append(_("25"));
    ChoiceParm1->Append(_("30"));
    ChoiceParm1->Append(_("33"));
    ChoiceParm1->Append(_("40"));
    ChoiceParm1->Append(_("50"));
    ChoiceParm1->Append(_("60"));
    ChoiceParm1->Append(_("66"));
    ChoiceParm1->Append(_("70"));
    ChoiceParm1->Append(_("75"));
    ChoiceParm1->Append(_("80"));
    ChoiceParm1->Append(_("90"));
    ChoiceParm1->Append(_("100"));
    ChoiceParm2 = new wxChoice(ACToolbar, ID_CHOICE_PARM2, wxPoint(476,11), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PARM2"));
    ChoiceParm2->Append(_("0"));
    ChoiceParm2->Append(_("10"));
    ChoiceParm2->Append(_("20"));
    ChoiceParm2->Append(_("25"));
    ChoiceParm2->Append(_("30"));
    ChoiceParm2->Append(_("33"));
    ChoiceParm2->Append(_("40"));
    ChoiceParm2->Append(_("50"));
    ChoiceParm2->Append(_("60"));
    ChoiceParm2->Append(_("66"));
    ChoiceParm2->Append(_("70"));
    ChoiceParm2->Append(_("75"));
    ChoiceParm2->Append(_("80"));
    ChoiceParm2->Append(_("90"));
    ChoiceParm2->SetSelection( ChoiceParm2->Append(_("100")) );
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACDISABLED, _("Disable"), GetToolbarBitmapBundle("xlAC_DISABLED"), wxNullBitmap, wxITEM_CHECK, wxEmptyString, wxEmptyString, NULL);
    ACToolbar->AddSeparator();
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACSELECT, _("Select"), GetToolbarBitmapBundle("xlAC_SELECT"), wxNullBitmap, wxITEM_CHECK, _("Select - SHIFT L"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACOFF, _("Off"), GetToolbarBitmapBundle("xlAC_OFF"), wxNullBitmap, wxITEM_CHECK, _("Off - DELETE"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACON, _("On"), GetToolbarBitmapBundle("xlAC_ON"), wxNullBitmap, wxITEM_CHECK, _("On - O"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACSHIMMER, _("Shimmer"), GetToolbarBitmapBundle("xlAC_SHIMMER"), wxNullBitmap, wxITEM_CHECK, _("Shimmer - S"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACTWINKLE, _("Twinkle"), GetToolbarBitmapBundle("xlAC_TWINKLE"), wxNullBitmap, wxITEM_CHECK, _("Twinkle - K"), wxEmptyString, NULL);
    ACToolbar->AddSeparator();
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACINTENSITY, _("Intensity"), GetToolbarBitmapBundle("xlAC_INTENSITY"), wxNullBitmap, wxITEM_CHECK, _("Intensity - I"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACRAMPUP, _("Ramp Up"), GetToolbarBitmapBundle("xlAC_RAMPUP"), wxNullBitmap, wxITEM_CHECK, _("Ramp Up - U"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACRAMPDOWN, _("Ramp Down"), GetToolbarBitmapBundle("xlAC_RAMPDOWN"), wxNullBitmap, wxITEM_CHECK, _("Ramp Down - D"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACRAMPUPDOWN, _("Ramp Up/Down"), GetToolbarBitmapBundle("xlAC_RAMPUPDOWN"), wxNullBitmap, wxITEM_CHECK, _("Ramp Up/Down - A"), wxEmptyString, NULL);
    ACToolbar->AddControl(ChoiceParm1, _("Parm1"));
    ACToolbar->AddControl(ChoiceParm2, _("Parm2"));
    ACToolbar->AddSeparator();
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACFILL, _("Fill"), GetToolbarBitmapBundle("xlAC_FILL"), wxNullBitmap, wxITEM_CHECK, _("Fill - F"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACCASCADE, _("Cascade"), GetToolbarBitmapBundle("xlAC_CASCADE"), wxNullBitmap, wxITEM_CHECK, _("Cascade - H"), wxEmptyString, NULL);
    ACToolbar->AddSeparator();
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACFOREGROUND, _("Foreground"), GetToolbarBitmapBundle("xlAC_FOREGROUND"), wxNullBitmap, wxITEM_CHECK, _("Foreground - G"), wxEmptyString, NULL);
    ACToolbar->AddTool(ID_AUITOOLBARITEM_ACBACKGROUND, _("Background"), GetToolbarBitmapBundle("xlAC_BACKGROUND"), wxNullBitmap, wxITEM_CHECK, _("Background - B"), wxEmptyString, NULL);
    ACToolbar->Realize();
    MainAuiManager->AddPane(ACToolbar, wxAuiPaneInfo().Name(_T("ACToolbar")).ToolbarPane().Caption(_("AC Toolbar")).CloseButton(false).Layer(6).Top().Gripper());
    ViewToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_VIEW, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    ViewToolBar->AddTool(wxID_ZOOM_IN, _("Zoom In"), GetToolbarBitmapBundle("xlART_ZOOM_IN"), wxNullBitmap, wxITEM_NORMAL, _("Zoom In"), wxEmptyString, NULL);
    ViewToolBar->AddTool(wxID_ZOOM_OUT, _("Zoom Out"), GetToolbarBitmapBundle("xlART_ZOOM_OUT"), wxNullBitmap, wxITEM_NORMAL, _("Zoom Out"), wxEmptyString, NULL);
    ViewToolBar->AddTool(ID_AUITOOLBARITEM14, _("Sequence Settings"), GetToolbarBitmapBundle("xlART_SETTINGS"), wxNullBitmap, wxITEM_NORMAL, _("Settings"), wxEmptyString, NULL);
    ViewToolBar->Realize();
    MainAuiManager->AddPane(ViewToolBar, wxAuiPaneInfo().Name(_T("View Tool Bar")).ToolbarPane().Caption(_("Pane caption")).CloseButton(false).Layer(10).Position(13).Top().Gripper());
    EffectsToolBar = new xlAuiToolBar(this, ID_AUIEFFECTSTOOLBAR, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    EffectsToolBar->Realize();
    MainAuiManager->AddPane(EffectsToolBar, wxAuiPaneInfo().Name(_T("EffectsToolBar")).ToolbarPane().Caption(_("Effects")).CloseButton(false).Layer(5).Top().Gripper());
    Notebook1 = new wxAuiNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_TOP|wxBORDER_NONE);
    PanelSetup = new wxPanel(Notebook1, ID_PANEL_SETUP, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_SETUP"));
    FlexGridSizerSetup = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetup->AddGrowableCol(0);
    FlexGridSizerSetup->AddGrowableRow(1);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, PanelSetup, _("Directories"));
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    StaticText38 = new wxStaticText(PanelSetup, wxID_ANY, _("Show Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    GridBagSizer1->Add(StaticText38, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Button03 = new wxButton(PanelSetup, ID_BUTTON3, _("Change Permanently"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    GridBagSizer1->Add(Button03, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_CheckShowFolderTemporarily = new wxButton(PanelSetup, ID_BUTTON11, _("Change Temporarily"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    GridBagSizer1->Add(Button_CheckShowFolderTemporarily, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ChangeTemporarilyAgain = new wxButton(PanelSetup, ID_BUTTON13, _("Change Temporarily Again"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    Button_ChangeTemporarilyAgain->Hide();
    GridBagSizer1->Add(Button_ChangeTemporarilyAgain, wxGBPosition(0, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ShowDirectoryLabel = new wxStaticText(PanelSetup, ID_STATICTEXT4, _("{Show Directory not set}"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    GridBagSizer1->Add(ShowDirectoryLabel, wxGBPosition(0, 4), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(GridBagSizer1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelSetup, _("Controllers"));
    FlexGridSizerNetworks = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizerNetworks->AddGrowableCol(2);
    FlexGridSizerNetworks->AddGrowableRow(0);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    ButtonSaveSetup = new wxButton(PanelSetup, ID_BUTTON_SAVE_SETUP, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_SETUP"));
    BoxSizer1->Add(ButtonSaveSetup, 1, wxALL|wxEXPAND, 3);
    ButtonAddControllerSerial = new wxButton(PanelSetup, ID_BUTTON9, _("Add USB"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    BoxSizer1->Add(ButtonAddControllerSerial, 1, wxALL|wxEXPAND, 3);
    ButtonAddControllerEthernet = new wxButton(PanelSetup, ID_BUTTON6, _("Add Ethernet"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    BoxSizer1->Add(ButtonAddControllerEthernet, 1, wxALL|wxEXPAND, 3);
    ButtonAddControllerNull = new wxButton(PanelSetup, ID_BUTTON10, _("Add Null"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    BoxSizer1->Add(ButtonAddControllerNull, 1, wxALL|wxEXPAND, 3);
    ButtonDiscover = new wxButton(PanelSetup, ID_BUTTON5, _("Discover"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    BoxSizer1->Add(ButtonDiscover, 1, wxALL|wxEXPAND, 3);
    FlexGridSizerNetworks->Add(BoxSizer1, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 0);
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    BitmapButtonMoveNetworkUp = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON1, GetButtonBitmapBundle("wxART_GO_UP"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    BitmapButtonMoveNetworkUp->SetToolTip(_("Move selected item up"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkUp, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonMoveNetworkDown = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON2, GetButtonBitmapBundle("wxART_GO_DOWN"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    BitmapButtonMoveNetworkDown->SetToolTip(_("Move selected item down"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkDown, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerNetworks->Add(FlexGridSizer9, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 10);
    SplitterWindowControllers = new wxSplitterWindow(PanelSetup, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindowControllers->SetMinimumPaneSize(20);
    SplitterWindowControllers->SetSashGravity(0.8);
    Panel2 = new wxPanel(SplitterWindowControllers, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizerSetupControllers = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupControllers->AddGrowableCol(0);
    FlexGridSizerSetupControllers->AddGrowableRow(0);
    Panel2->SetSizer(FlexGridSizerSetupControllers);
    FlexGridSizerSetupControllers->Fit(Panel2);
    FlexGridSizerSetupControllers->SetSizeHints(Panel2);
    Panel5 = new wxPanel(SplitterWindowControllers, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxALWAYS_SHOW_SB, _T("ID_PANEL6"));
    Panel5->SetMinSize(wxSize(20,-1));
    FlexGridSizerSetupRight = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupRight->AddGrowableCol(0);
    FlexGridSizerSetupRight->AddGrowableRow(0);
    FlexGridSizerSetupProperties = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupProperties->AddGrowableCol(0);
    FlexGridSizerSetupProperties->AddGrowableRow(0);
    FlexGridSizerSetupRight->Add(FlexGridSizerSetupProperties, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerSetupControllerButtons = new wxFlexGridSizer(1, 0, 0, 0);
    ButtonVisualise = new wxButton(Panel5, ID_BUTTON1, _("Visualise ..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizerSetupControllerButtons->Add(ButtonVisualise, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonUploadInput = new wxButton(Panel5, ID_BUTTON2, _("Upload Input"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizerSetupControllerButtons->Add(ButtonUploadInput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonUploadOutput = new wxButton(Panel5, ID_BUTTON4, _("Upload Output"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizerSetupControllerButtons->Add(ButtonUploadOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonOpen = new wxButton(Panel5, ID_BUTTON7, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizerSetupControllerButtons->Add(ButtonOpen, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_OpenProxy = new wxButton(Panel5, ID_BUTTON12, _("Open Proxy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizerSetupControllerButtons->Add(Button_OpenProxy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonControllerDelete = new wxButton(Panel5, ID_BUTTON8, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizerSetupControllerButtons->Add(ButtonControllerDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDummy = new wxStaticText(Panel5, ID_STATICTEXT1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizerSetupControllerButtons->Add(StaticTextDummy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerSetupRight->Add(FlexGridSizerSetupControllerButtons, 1, wxALL|wxEXPAND, 5);
    Panel5->SetSizer(FlexGridSizerSetupRight);
    FlexGridSizerSetupRight->Fit(Panel5);
    FlexGridSizerSetupRight->SetSizeHints(Panel5);
    SplitterWindowControllers->SplitVertically(Panel2, Panel5);
    SplitterWindowControllers->SetSashPosition(1000);
    FlexGridSizerNetworks->Add(SplitterWindowControllers, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer2->Add(FlexGridSizerNetworks, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    PanelSetup->SetSizer(FlexGridSizerSetup);
    FlexGridSizerSetup->Fit(PanelSetup);
    FlexGridSizerSetup->SetSizeHints(PanelSetup);
    PanelPreview = new wxPanel(Notebook1, ID_PANEL_PREVIEW, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_PREVIEW"));
    FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerPreview->AddGrowableCol(0);
    FlexGridSizerPreview->AddGrowableRow(0);
    PanelPreview->SetSizer(FlexGridSizerPreview);
    FlexGridSizerPreview->Fit(PanelPreview);
    FlexGridSizerPreview->SetSizeHints(PanelPreview);
    PanelSequencer = new wxPanel(Notebook1, XLIGHTS_SEQUENCER_TAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("XLIGHTS_SEQUENCER_TAB"));
    m_mgr = new wxAuiManager(PanelSequencer, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_DEFAULT);
    Notebook1->AddPage(PanelSetup, _("Controllers"), true);
    Notebook1->AddPage(PanelPreview, _("Layout"));
    Notebook1->AddPage(PanelSequencer, _("Sequencer"));
    MainAuiManager->AddPane(Notebook1, wxAuiPaneInfo().Name(_T("MainPain")).CenterPane().Caption(_("Pane caption")).PaneBorder(false));
    AUIStatusBar = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    StatusBarSizer = new wxGridBagSizer(0, 0);
    StatusText = new wxStaticText(AUIStatusBar, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    StatusBarSizer->Add(StatusText, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxEXPAND, 2);
    Panel1 = new wxPanel(AUIStatusBar, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    Panel1->SetMinSize(wxDLG_UNIT(AUIStatusBar,wxSize(100,-1)));
    GaugeSizer = new wxFlexGridSizer(1, 1, 0, 0);
    GaugeSizer->AddGrowableCol(0);
    ProgressBar = new wxGauge(Panel1, ID_GAUGE1, 100, wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(100,-1)), 0, wxDefaultValidator, _T("ID_GAUGE1"));
    GaugeSizer->Add(ProgressBar, 0, wxEXPAND, 0);
    Panel1->SetSizer(GaugeSizer);
    GaugeSizer->Fit(Panel1);
    GaugeSizer->SetSizeHints(Panel1);
    StatusBarSizer->Add(Panel1, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FileNameText = new wxStaticText(AUIStatusBar, ID_STATICTEXT7, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    StatusBarSizer->Add(FileNameText, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxEXPAND, 2);
    StatusBarSizer->AddGrowableRow(0);
    AUIStatusBar->SetSizer(StatusBarSizer);
    StatusBarSizer->Fit(AUIStatusBar);
    StatusBarSizer->SetSizeHints(AUIStatusBar);
    MainAuiManager->AddPane(AUIStatusBar, wxAuiPaneInfo().Name(_T("Status Bar")).DefaultPane().Caption(_("Status bar")).CaptionVisible(false).CloseButton(false).Bottom().DockFixed().Dockable(false).Floatable(false).FloatingPosition(wxPoint(0,0)).FloatingSize(wxSize(0,0)).Movable(false).PaneBorder(false));
    MainAuiManager->Update();
    MenuBar = new wxMenuBar();
    MenuFile = new wxMenu();
    MenuItem3 = new wxMenuItem(MenuFile, ID_NEW_SEQUENCE, _("New Sequence\tCtrl-n"), wxEmptyString, wxITEM_NORMAL);
    MenuItem3->SetBitmap(GetMenuItemBitmapBundle("wxART_NEW"));
    MenuFile->Append(MenuItem3);
    MenuItem_File_Open_Sequence = new wxMenuItem(MenuFile, ID_OPEN_SEQUENCE, _("Open Sequence\tCTRL-o"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_File_Open_Sequence->SetBitmap(GetMenuItemBitmapBundle("wxART_FILE_OPEN"));
    MenuFile->Append(MenuItem_File_Open_Sequence);
    RecentSequencesMenu = new wxMenu();
    MenuItem7 = new wxMenuItem(RecentSequencesMenu, ID_MENUITEM4, _("RECENT1"), wxEmptyString, wxITEM_NORMAL);
    RecentSequencesMenu->Append(MenuItem7);
    MenuFile->Append(ID_MENUITEM_OPENRECENTSEQUENCE, _("Open Recent Sequence"), RecentSequencesMenu, wxEmptyString);
    MenuItem_File_Save = new wxMenuItem(MenuFile, IS_SAVE_SEQ, _("Save\tCTRL-S"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_File_Save->SetBitmap(GetMenuItemBitmapBundle("wxART_FILE_SAVE"));
    MenuFile->Append(MenuItem_File_Save);
    MenuItem_File_Save->Enable(false);
    MenuItem_File_SaveAs_Sequence = new wxMenuItem(MenuFile, ID_SAVE_AS_SEQUENCE, _("Save Sequence As"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_File_SaveAs_Sequence->SetBitmap(GetMenuItemBitmapBundle("wxART_FILE_SAVE_AS"));
    MenuFile->Append(MenuItem_File_SaveAs_Sequence);
    MenuItem_File_Close_Sequence = new wxMenuItem(MenuFile, ID_CLOSE_SEQ, _("Close Sequence"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem_File_Close_Sequence);
    MenuItem_File_Close_Sequence->Enable(false);
    MenuFile->AppendSeparator();
    MenuItem61 = new wxMenuItem(MenuFile, wxID_PREFERENCES, _("Preferences...\tCTRL-,"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem61);
    Menu_Settings_Sequence = new wxMenuItem(MenuFile, ID_SEQ_SETTINGS, _("Sequence Settings"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(Menu_Settings_Sequence);
    MenuItem_KeyBindings = new wxMenuItem(MenuFile, ID_MNU_KEYBINDINGS, _("Key bindings"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem_KeyBindings);
    MenuFile->AppendSeparator();
    MenuItem_File_Export_Video = new wxMenuItem(MenuFile, ID_EXPORT_VIDEO, _("Export House Preview Video"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItem_File_Export_Video);
    MenuFile->AppendSeparator();
    MenuItem5 = new wxMenuItem(MenuFile, ID_MENUITEM2, _("Select Show Folder"), wxEmptyString, wxITEM_NORMAL);
    MenuItem5->SetBitmap(GetMenuItemBitmapBundle("wxART_FOLDER_OPEN"));
    MenuFile->Append(MenuItem5);
    RecentShowFoldersMenu = new wxMenu();
    MenuItem11 = new wxMenuItem(RecentShowFoldersMenu, ID_MENUITEM8, _("RECENTFOLDER1"), wxEmptyString, wxITEM_NORMAL);
    RecentShowFoldersMenu->Append(MenuItem11);
    MenuFile->Append(ID_MENUITEM_RECENTFOLDERS, _("Recent Show Folders"), RecentShowFoldersMenu, wxEmptyString);
    MenuItemBackup = new wxMenuItem(MenuFile, ID_FILE_BACKUP, _("Backup\tF10"), wxEmptyString, wxITEM_NORMAL);
    MenuItemBackup->SetBitmap(GetMenuItemBitmapBundle("wxART_HARDDISK"));
    MenuFile->Append(MenuItemBackup);
    MenuItemRestoreBackup = new wxMenuItem(MenuFile, ID_FILE_RESTOREBACKUP, _("Restore Backup"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(MenuItemRestoreBackup);
    mAltBackupMenuItem = new wxMenuItem(MenuFile, ID_FILE_ALTBACKUP, _("Alternate Backup\tF11"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(mAltBackupMenuItem);
    QuitMenuItem = new wxMenuItem(MenuFile, wxID_EXIT, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    QuitMenuItem->SetBitmap(GetMenuItemBitmapBundle("wxART_QUIT"));
    MenuFile->Append(QuitMenuItem);
    MenuBar->Append(MenuFile, _("&File"));
    Menu3 = new wxMenu();
    MenuItem37 = new wxMenuItem(Menu3, wxID_UNDO, _("Undo\tCtrl-z"), wxEmptyString, wxITEM_NORMAL);
    MenuItem37->SetBitmap(GetMenuItemBitmapBundle("wxART_UNDO"));
    Menu3->Append(MenuItem37);
    Menu3->AppendSeparator();
    MenuItem34 = new wxMenuItem(Menu3, wxID_CUT, _("Cut\tCTRL-x"), wxEmptyString, wxITEM_NORMAL);
    MenuItem34->SetBitmap(GetMenuItemBitmapBundle("wxART_CUT"));
    Menu3->Append(MenuItem34);
    MenuItem35 = new wxMenuItem(Menu3, wxID_COPY, _("Copy\tCTRL-c"), wxEmptyString, wxITEM_NORMAL);
    MenuItem35->SetBitmap(GetMenuItemBitmapBundle("wxART_COPY"));
    Menu3->Append(MenuItem35);
    MenuItem36 = new wxMenuItem(Menu3, wxID_PASTE, _("Paste\tCTRL-v"), wxEmptyString, wxITEM_NORMAL);
    MenuItem36->SetBitmap(GetMenuItemBitmapBundle("wxART_PASTE"));
    Menu3->Append(MenuItem36);
    Menu3->AppendSeparator();
    MenuItemShiftEffects = new wxMenuItem(Menu3, ID_SHIFT_EFFECTS, _("Shift Effects"), _("Use this options to shift all effects in the sequence."), wxITEM_NORMAL);
    Menu3->Append(MenuItemShiftEffects);
    MenuItemShiftSelectedEffects = new wxMenuItem(Menu3, ID_MNU_SHIFT_SELECTED_EFFECTS, _("Shift Selected Effects"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItemShiftSelectedEffects);
    Menu3->AppendSeparator();
    MenuItem_ColorReplace = new wxMenuItem(Menu3, ID_MNU_COLOURREPLACE, _("Color Replace"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_ColorReplace);
    MenuBar->Append(Menu3, _("&Edit"));
    Menu1 = new wxMenu();
    ActionTestMenuItem = new wxMenuItem(Menu1, ID_MENUITEM13, _("&Test"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(ActionTestMenuItem);
    Menu1->AppendSeparator();
    MenuItemCheckSequence = new wxMenuItem(Menu1, ID_MNU_CHECKSEQ, _("C&heck Sequence"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemCheckSequence);
    MenuItem_CleanupFileLocations = new wxMenuItem(Menu1, ID_MNU_CLEANUPFILE, _("Cleanup File Locations"), _("Moves all files into or under the show folder."), wxITEM_NORMAL);
    Menu1->Append(MenuItem_CleanupFileLocations);
    MenuItem_PackageSequence = new wxMenuItem(Menu1, ID_MNU_PACKAGESEQUENCE, _("Package &Sequence"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_PackageSequence);
    MenuItem_DownloadSequences = new wxMenuItem(Menu1, ID_MNU_DOWNLOADSEQUENCES, _("Download Sequences/Lyrics"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_DownloadSequences);
    Menu1->AppendSeparator();
    MenuItemBatchRender = new wxMenuItem(Menu1, ID_MENU_BATCH_RENDER, _("&Batch Render"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemBatchRender);
    MenuItem_FPP_Connect = new wxMenuItem(Menu1, ID_MENU_FPP_CONNECT, _("&FPP Connect"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_FPP_Connect);
    MenuItemBulkControllerUpload = new wxMenuItem(Menu1, ID_MNU_BULKUPLOAD, _("Bulk Controller Upload"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemBulkControllerUpload);
    MenuItemHinksPixExport = new wxMenuItem(Menu1, ID_MENU_HINKSPIX_EXPORT, _("HinksPix Export"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemHinksPixExport);
    MenuItemRunScript = new wxMenuItem(Menu1, ID_MENU_RUN_SCRIPT, _("Run Scripts"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemRunScript);
    Menu1->AppendSeparator();
    mExportModelsMenuItem = new wxMenuItem(Menu1, ID_EXPORT_MODELS, _("E&xport Models"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(mExportModelsMenuItem);
    MenuItem_ExportEffects = new wxMenuItem(Menu1, ID_MNU_EXPORT_EFFECTS, _("Export &Effects"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ExportEffects);
    MenuItem_ExportControllerConnections = new wxMenuItem(Menu1, ID_MNU_EXPORT_CONTROLLER_CONNECTIONS, _("Export Controller Connections"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ExportControllerConnections);
    Menu1->AppendSeparator();
    MenuItem_ViewLog = new wxMenuItem(Menu1, ID_MENU_VIEW_LOG, _("&View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ViewLog);
    MenuItem38 = new wxMenuItem(Menu1, ID_MENUITEM18, _("&Package Log Files"), _("Packages up current configuration, logs and sequence for reporting a problem to development team."), wxITEM_NORMAL);
    Menu1->Append(MenuItem38);
    MenuItem_PurgeVendorCache = new wxMenuItem(Menu1, iD_MNU_VENDORCACHEPURGE, _("Purge Download Cache"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_PurgeVendorCache);
    MenuItem_PurgeRenderCache = new wxMenuItem(Menu1, ID_MNU_PURGERENDERCACHE, _("Purge Render Cache"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_PurgeRenderCache);
    MenuItem_CrashXLights = new wxMenuItem(Menu1, ID_MNU_CRASH, _("Crash xLights"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_CrashXLights);
    MenuItem_LogRenderState = new wxMenuItem(Menu1, ID_MNU_DUMPRENDERSTATE, _("Log Render State"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_LogRenderState);
    Menu1->AppendSeparator();
    MenuItem_Generate2DPath = new wxMenuItem(Menu1, ID_MENU_GENERATE2DPATH, _("Generate 2D Path"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_Generate2DPath);
    Menu_GenerateCustomModel = new wxMenuItem(Menu1, ID_MENUITEM_GenerateCustomModel, _("&Generate Custom Model"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(Menu_GenerateCustomModel);
    MenuItem_RemapCustom = new wxMenuItem(Menu1, ID_MNU_REMAPCUSTOM, _("Remap Custom Model"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_RemapCustom);
    MenuItem_GenerateLyrics = new wxMenuItem(Menu1, ID_MNU_GENERATELYRICS, _("Generate &Lyrics From Data"), _("Generate lyric phenomes from data"), wxITEM_NORMAL);
    Menu1->Append(MenuItem_GenerateLyrics);
    MenuItemConvert = new wxMenuItem(Menu1, ID_MENUITEM_CONVERT, _("&Convert"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemConvert);
    MenuItem_PrepareAudio = new wxMenuItem(Menu1, ID_MNU_PREPAREAUDIO, _("Prepare Audio"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_PrepareAudio);
    MenuItemUserDict = new wxMenuItem(Menu1, ID_MENU_USER_DICT, _("User Lyric Dictionary"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemUserDict);
    Menu1->AppendSeparator();
    MenuItem_xSchedule = new wxMenuItem(Menu1, ID_MNU_XSCHEDULE, _("xSchedu&le"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_xSchedule);
    xCaptureMenuItem = new wxMenuItem(Menu1, ID_MENU_XCAPTURE, _("xCapture"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(xCaptureMenuItem);
    MenuItem_xScanner = new wxMenuItem(Menu1, ID_MNU_XSCANNER, _("xScanner"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_xScanner);
    MenuBar->Append(Menu1, _("&Tools"));
    MenuView = new wxMenu();
    MenuItem_ViewZoomIn = new wxMenuItem(MenuView, wxID_ZOOM_IN, _("Zoom In"), wxEmptyString, wxITEM_NORMAL);
    MenuView->Append(MenuItem_ViewZoomIn);
    MenuItem_ViewZoomOut = new wxMenuItem(MenuView, wxID_ZOOM_OUT, _("Zoom Out"), wxEmptyString, wxITEM_NORMAL);
    MenuView->Append(MenuItem_ViewZoomOut);
    MenuView->AppendSeparator();
    MenuItem13 = new wxMenuItem(MenuView, ID_MENUITEM5, _("Reset Toolbars"), wxEmptyString, wxITEM_NORMAL);
    MenuView->Append(MenuItem13);
    MenuItem_ACLIghts = new wxMenuItem(MenuView, MNU_ID_ACLIGHTS, _("AC Lights Toolbar"), wxEmptyString, wxITEM_CHECK);
    MenuView->Append(MenuItem_ACLIghts);
    MenuItem_ShowACRamps = new wxMenuItem(MenuView, ID_MNU_SHOWRAMPS, _("Show AC Ramps"), _("Show on effects and twinkle effects as ramps."), wxITEM_CHECK);
    MenuView->Append(MenuItem_ShowACRamps);
    MenuView->AppendSeparator();
    MenuItemPerspectives = new wxMenu();
    MenuItemViewSavePerspective = new wxMenuItem(MenuItemPerspectives, ID_MENUITEM_SAVE_PERSPECTIVE, _("Save Current"), wxEmptyString, wxITEM_NORMAL);
    MenuItemPerspectives->Append(MenuItemViewSavePerspective);
    MenuItemViewSaveAsPerspective = new wxMenuItem(MenuItemPerspectives, ID_MENUITEM_SAVE_AS_PERSPECTIVE, _("Save As New"), wxEmptyString, wxITEM_NORMAL);
    MenuItemPerspectives->Append(MenuItemViewSaveAsPerspective);
    MenuItemLoadEditPerspective = new wxMenuItem(MenuItemPerspectives, ID_MENUITEM_LOAD_PERSPECTIVE, _("Edit/Load"), wxEmptyString, wxITEM_NORMAL);
    MenuItemPerspectives->Append(MenuItemLoadEditPerspective);
    MenuItem_PerspectiveAutosave = new wxMenuItem(MenuItemPerspectives, ID_MNU_PERSPECTIVES_AUTOSAVE, _("Auto Save"), _("Save the current perspective between sessions independent of the show directory."), wxITEM_CHECK);
    MenuItemPerspectives->Append(MenuItem_PerspectiveAutosave);
    MenuItemPerspectives->AppendSeparator();
    MenuView->Append(ID_MENUITEM7, _("Perspectives"), MenuItemPerspectives, wxEmptyString);
    MenuItem18 = new wxMenu();
    MenuItemDisplayElements = new wxMenuItem(MenuItem18, ID_MENUITEM_DISPLAY_ELEMENTS, _("Display Elements"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemDisplayElements);
    MenuItemModelPreview = new wxMenuItem(MenuItem18, ID_MENU_TOGGLE_MODEL_PREVIEW, _("Model Preview"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemModelPreview);
    MenuItemHousePreview = new wxMenuItem(MenuItem18, ID_MENU_TOGGLE_HOUSE_PREVIEW, _("House Preview"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemHousePreview);
    MenuItemEffectSettings = new wxMenuItem(MenuItem18, ID_MENUITEM14, _("Effect Settings"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemEffectSettings);
    MenuItemColours = new wxMenuItem(MenuItem18, ID_MENUITEM15, _("Colors"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemColours);
    MenuItemLayerBlending = new wxMenuItem(MenuItem18, ID_MENUITEM16, _("Layer Blending"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemLayerBlending);
    MenuItemLayerSettings = new wxMenuItem(MenuItem18, ID_MENUITEM9, _("Layer Settings"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemLayerSettings);
    MenuItemEffectDropper = new wxMenuItem(MenuItem18, ID_MENUITEM17, _("Effect Dropper"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemEffectDropper);
    MenuItemValueCurves = new wxMenuItem(MenuItem18, ID_MNU_VALUECURVES, _("Value Curves"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemValueCurves);
    MenuItemColourDropper = new wxMenuItem(MenuItem18, ID_MNU_COLOURDROPPER, _("Color Dropper"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemColourDropper);
    MenuItemEffectAssist = new wxMenuItem(MenuItem18, ID_MENUITEM_EFFECT_ASSIST_WINDOW, _("Effect Assist"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemEffectAssist);
    MenuItemSelectEffect = new wxMenuItem(MenuItem18, ID_MENUITEM_SELECT_EFFECT, _("Select Effect"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemSelectEffect);
    MenuItemSearchEffects = new wxMenuItem(MenuItem18, ID_MENUITEM_SEARCH_EFFECTS, _("Search Effects"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemSearchEffects);
    MenuItemVideoPreview = new wxMenuItem(MenuItem18, ID_MENUITEM_VIDEOPREVIEW, _("Video Preview"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemVideoPreview);
    MenuItemJukebox = new wxMenuItem(MenuItem18, ID_MNU_JUKEBOX, _("Jukebox"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemJukebox);
    MenuItemFindData = new wxMenuItem(MenuItem18, ID_MNU_FINDDATA, _("Find Effect Data"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemFindData);
    MenuItem18->AppendSeparator();
    MenuItem1 = new wxMenu();
    MenuItem_SD_HP = new wxMenuItem(MenuItem1, ID_MNU_SUPPRESSDOCK_HP, _("House Preview"), wxEmptyString, wxITEM_CHECK);
    MenuItem1->Append(MenuItem_SD_HP);
    MenuItem_SD_MP = new wxMenuItem(MenuItem1, ID_MNU_SUPPRESSDOCK_MP, _("Model Preview"), wxEmptyString, wxITEM_CHECK);
    MenuItem1->Append(MenuItem_SD_MP);
    MenuItem18->Append(ID_MENUITEM3, _("Suppress Dock"), MenuItem1, wxEmptyString);
    MenuItem26 = new wxMenuItem(MenuItem18, ID_MENUITEM_WINDOWS_PERSPECTIVE, _("Perspectives"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem26);
    MenuItem21 = new wxMenuItem(MenuItem18, ID_MENUITEM_WINDOWS_DOCKALL, _("Dock All"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem21);
    MenuItem33 = new wxMenuItem(MenuItem18, ID_MENUITEM11, _("Reset to Defaults"), wxEmptyString, wxITEM_NORMAL);
    MenuItem18->Append(MenuItem33);
    MenuView->Append(ID_MENUITEM10, _("Windows"), MenuItem18, wxEmptyString);
    MenuBar->Append(MenuView, _("&View"));
    AudioMenu = new wxMenu();
    MenuItem8 = new wxMenuItem(AudioMenu, ID_PLAY_FULL, _("Play Full Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem8);
    MenuItem39 = new wxMenuItem(AudioMenu, ID_MNU_1POINT5SPEED, _("Play 1.5x Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem39);
    MenuItem49 = new wxMenuItem(AudioMenu, ID_MN_2SPEED, _("Play 2x Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem49);
    MenuItem50 = new wxMenuItem(AudioMenu, ID_MNU_3SPEED, _("Play 3x Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem50);
    MenuItem51 = new wxMenuItem(AudioMenu, ID_MNU_4SPEED, _("Play 4x Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem51);
    MenuItem9 = new wxMenuItem(AudioMenu, ID_PLAY_3_4, _("Play 3/4 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem9);
    MenuItem30 = new wxMenuItem(AudioMenu, ID_PLAY_1_2, _("Play 1/2 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem30);
    MenuItem31 = new wxMenuItem(AudioMenu, ID_PLAY_1_4, _("Play 1/4 Speed"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem31);
    AudioMenu->AppendSeparator();
    MenuItem_LoudVol = new wxMenuItem(AudioMenu, ID_MNU_LOUDVOLUME, _("Loud"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem_LoudVol);
    MenuItem_MedVol = new wxMenuItem(AudioMenu, ID_MNU_MEDVOLUME, _("Medium"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem_MedVol);
    MenuItem_QuietVol = new wxMenuItem(AudioMenu, ID_MNU_QUIET, _("Quiet"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem_QuietVol);
    MenuItem_VQuietVol = new wxMenuItem(AudioMenu, ID_MNU_SUPERQUIET, _("Very Quiet"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem_VQuietVol);
    MenuItem_SilentVol = new wxMenuItem(AudioMenu, ID_MNU_SILENT, _("Silent"), wxEmptyString, wxITEM_RADIO);
    AudioMenu->Append(MenuItem_SilentVol);
    MenuBar->Append(AudioMenu, _("&Audio"));
    Menu2 = new wxMenu();
    MenuItem_ImportEffects = new wxMenuItem(Menu2, ID_IMPORT_EFFECTS, _("Import Effects"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem_ImportEffects);
    MenuBar->Append(Menu2, _("&Import"));
    MenuHelp = new wxMenu();
    MenuItem_TOD = new wxMenuItem(MenuHelp, ID_MNU_TOD, _("Tip of the Day"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_TOD);
    MenuItem_UserManual = new wxMenuItem(MenuHelp, ID_MNU_MANUAL, _("User Manual"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_UserManual);
    MenuItem_Zoom = new wxMenuItem(MenuHelp, ID_MNU_ZOOM, _("Zoom"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Zoom);
    MenuItem_ShowKeyBindings = new wxMenuItem(MenuHelp, ID_MENUITEM1, _("Key Bindings"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_ShowKeyBindings);
    MenuItem4 = new wxMenuItem(MenuHelp, idMenuHelpContent, _("Content\tF1"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem4);
    MenuItem_Help_Forum = new wxMenuItem(MenuHelp, ID_MENU_HELP_FORMUM, _("Forum"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Help_Forum);
    MenuItem_VideoTutorials = new wxMenuItem(MenuHelp, ID_MNU_VIDEOS, _("Video Tutorials"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_VideoTutorials);
    MenuItem_Help_Download = new wxMenuItem(MenuHelp, ID_MENU_HELP_DOWNLOAD, _("Download"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Help_Download);
    MenuItem_Help_ReleaseNotes = new wxMenuItem(MenuHelp, ID_MNU_HELP_RELEASE_NOTES, _("Release Notes"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Help_ReleaseNotes);
    MenuItem_Help_Isue_Tracker = new wxMenuItem(MenuHelp, ID_MENU_HELP_ISSUE, _("Issue Tracker"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Help_Isue_Tracker);
    MenuItem_Help_Facebook = new wxMenuItem(MenuHelp, ID_MENU_HELP_FACEBOOK, _("Facebook"), wxEmptyString, wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Help_Facebook);
    MenuItem_Donate = new wxMenuItem(MenuHelp, ID_MNU_DONATE, _("Donate"), _("Donate to the xLights project."), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Donate);
    MenuItem_Update = new wxMenuItem(MenuHelp, ID_MNU_UPDATE, _("Check for Updates"), _("Check for newer xLights updates"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem_Update);
    MenuItem_Update->Enable(false);
    MenuItem2 = new wxMenuItem(MenuHelp, wxID_ABOUT, _("About"), _("Show info about this application"), wxITEM_NORMAL);
    MenuHelp->Append(MenuItem2);
    MenuBar->Append(MenuHelp, _("&Help"));
    SetMenuBar(MenuBar);
    OutputTimer.SetOwner(this, ID_TIMER_OutputTimer);
    AutoSaveTimer.SetOwner(this, ID_TIMER_AutoSave);
    EffectSettingsTimer.SetOwner(this, ID_TIMER_EFFECT_SETTINGS);
    RenderStatusTimer.SetOwner(this, ID_TIMER_RENDERSTATUS);
    RenderStatusTimer.Start(100, false);

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
    Connect(ID_CHECKBOX_LIGHT_OUTPUT,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnCheckBoxLightOutputClick);
    Connect(ID_AUITOOLBARITEM2,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_AUITOOLBARITEM5,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_AUITOOLBARITEM7,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideBufferSettingsWindow);
    Connect(ID_AUITOOLBARITEM3,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideLayerTimingWindow);
    Connect(ID_TOGGLE_MODEL_PREVIEW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_TOGGLE_HOUSE_PREVIEW,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_AUITOOLBARITEM6,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideDisplayElementsWindow);
    Connect(ID_AUITOOLBARITEM8,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemShowHideEffects);
    Connect(ID_AUITOOLBARITEM9,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectAssistWindow);
    Connect(ID_PASTE_BY_TIME,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPasteByTimeClick);
    Connect(ID_PASTE_BY_CELL,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPasteByCellClick);
    Connect(ID_AUITOOLBARITEM_ACDISABLED,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_DisableClick);
    Connect(ID_AUITOOLBARITEM_ACSELECT,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_SelectClick);
    Connect(ID_AUITOOLBARITEM_ACOFF,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_OffClick);
    Connect(ID_AUITOOLBARITEM_ACON,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_OnClick);
    Connect(ID_AUITOOLBARITEM_ACSHIMMER,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_ShimmerClick);
    Connect(ID_AUITOOLBARITEM_ACTWINKLE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_TwinkleClick);
    Connect(ID_AUITOOLBARITEM_ACINTENSITY,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_IntensityClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPUP,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_RampUpClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPDOWN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_RampDownClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPUPDOWN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_RampUpDownClick);
    Connect(ID_CHOICE_PARM1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoiceParm1Select);
    Connect(ID_CHOICE_PARM2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnChoiceParm2Select);
    Connect(ID_AUITOOLBARITEM_ACFILL,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_FillClick);
    Connect(ID_AUITOOLBARITEM_ACCASCADE,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_CascadeClick);
    Connect(ID_AUITOOLBARITEM_ACFOREGROUND,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_ForegroundClick);
    Connect(ID_AUITOOLBARITEM_ACBACKGROUND,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAC_BackgroundClick);
    Connect(ID_AUITOOLBAR_AC,wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN,(wxObjectEventFunction)&xLightsFrame::OnACToolbarDropdown);
    Connect(wxID_ZOOM_IN,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(wxID_ZOOM_OUT,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_AUITOOLBARITEM14,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ChangeShowFolderTemporarily);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_ChangeTemporarilyAgainClick);
    Connect(ID_BUTTON_SAVE_SETUP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonSaveSetupClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerSerialClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerEthernetClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerNullClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonDiscoverClick);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveUpClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveDownClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonVisualiseClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonUploadInputClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonUploadOutputClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonOpenClick);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButton_OpenProxyClick);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xLightsFrame::OnButtonControllerDeleteClick);
    m_mgr->Connect(wxEVT_AUI_PANE_CLOSE,(wxObjectEventFunction)&xLightsFrame::OnPaneClose,0,this);
    PanelSequencer->Connect(wxEVT_PAINT,(wxObjectEventFunction)&xLightsFrame::OnPanelSequencerPaint,0,this);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged1);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanging);
    Connect(ID_NEW_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_OPEN_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Open_SequenceSelected);
    Connect(IS_SAVE_SEQ,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Save_Selected);
    Connect(ID_SAVE_AS_SEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected);
    Connect(ID_CLOSE_SEQ,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Close_SequenceSelected);
    Connect(wxID_PREFERENCES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemPreferencesSelected);
    Connect(ID_SEQ_SETTINGS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_MNU_KEYBINDINGS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_KeyBindingsSelected);
    Connect(ID_EXPORT_VIDEO,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Export_VideoSelected);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_FILE_BACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBackupSelected);
    Connect(ID_FILE_RESTOREBACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRestoreBackupSelected);
    Connect(ID_FILE_ALTBACKUP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnmAltBackupMenuItemSelected);
    Connect(wxID_EXIT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnQuit);
    Connect(ID_SHIFT_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemShiftEffectsSelected);
    Connect(ID_MNU_SHIFT_SELECTED_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemShiftSelectedEffectsSelected);
    Connect(ID_MNU_COLOURREPLACE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ColorReplaceSelected);
    Connect(ID_MENUITEM13,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnActionTestMenuItemSelected);
    Connect(ID_MNU_CHECKSEQ,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemCheckSequenceSelected);
    Connect(ID_MNU_CLEANUPFILE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_CleanupFileLocationsSelected);
    Connect(ID_MNU_PACKAGESEQUENCE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_PackageSequenceSelected);
    Connect(ID_MNU_DOWNLOADSEQUENCES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_DownloadSequencesSelected);
    Connect(ID_MENU_BATCH_RENDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBatchRenderSelected);
    Connect(ID_MENU_FPP_CONNECT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_FPP_ConnectSelected);
    Connect(ID_MNU_BULKUPLOAD,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemBulkControllerUploadSelected);
    Connect(ID_MENU_HINKSPIX_EXPORT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemHinksPixExportSelected);
    Connect(ID_MENU_RUN_SCRIPT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemRunScriptSelected);
    Connect(ID_EXPORT_MODELS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnmExportModelsMenuItemSelected);
    Connect(ID_MNU_EXPORT_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ExportEffectsSelected);
    Connect(ID_MNU_EXPORT_CONTROLLER_CONNECTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ExportControllerConnectionsSelected);
    Connect(ID_MENU_VIEW_LOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MENUITEM18,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemPackageDebugFiles);
    Connect(iD_MNU_VENDORCACHEPURGE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_PurgeVendorCacheSelected);
    Connect(ID_MNU_PURGERENDERCACHE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_PurgeRenderCacheSelected);
    Connect(ID_MNU_CRASH,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_CrashXLightsSelected);
    Connect(ID_MNU_DUMPRENDERSTATE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_LogRenderStateSelected);
    Connect(ID_MENU_GENERATE2DPATH,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Generate2DPathSelected);
    Connect(ID_MENUITEM_GenerateCustomModel,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenu_GenerateCustomModelSelected);
    Connect(ID_MNU_REMAPCUSTOM,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_RemapCustomSelected);
    Connect(ID_MNU_GENERATELYRICS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_GenerateLyricsSelected);
    Connect(ID_MENUITEM_CONVERT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemConvertSelected);
    Connect(ID_MNU_PREPAREAUDIO,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_PrepareAudioSelected);
    Connect(ID_MENU_USER_DICT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemUserDictSelected);
    Connect(ID_MNU_XSCHEDULE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_xScheduleSelected);
    Connect(ID_MENU_XCAPTURE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_xCaptureSelected);
    Connect(ID_MNU_XSCANNER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_xScannerSelected);
    Connect(wxID_ZOOM_IN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(wxID_ZOOM_OUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ResetToolbarLocations);
    Connect(MNU_ID_ACLIGHTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ACLIghtsSelected);
    Connect(ID_MNU_SHOWRAMPS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ShowACRampsSelected);
    Connect(ID_MENUITEM_SAVE_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemViewSavePerspectiveSelected);
    Connect(ID_MENUITEM_SAVE_AS_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemViewSaveAsPerspectiveSelected);
    Connect(ID_MENUITEM_LOAD_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadEditPerspectiveSelected);
    Connect(ID_MNU_PERSPECTIVES_AUTOSAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_PerspectiveAutosaveSelected);
    Connect(ID_MENUITEM_DISPLAY_ELEMENTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideDisplayElementsWindow);
    Connect(ID_MENU_TOGGLE_MODEL_PREVIEW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_MENU_TOGGLE_HOUSE_PREVIEW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_MENUITEM14,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_MENUITEM15,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_MENUITEM16,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideLayerTimingWindow);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideBufferSettingsWindow);
    Connect(ID_MENUITEM17,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectDropper);
    Connect(ID_MNU_VALUECURVES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ValueCurvesSelected);
    Connect(ID_MNU_COLOURDROPPER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ColourDropperSelected);
    Connect(ID_MENUITEM_EFFECT_ASSIST_WINDOW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideEffectAssistWindow);
    Connect(ID_MENUITEM_SELECT_EFFECT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemSelectEffectSelected);
    Connect(ID_MENUITEM_SEARCH_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemSearchEffectsSelected);
    Connect(ID_MENUITEM_VIDEOPREVIEW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemShowHideVideoPreview);
    Connect(ID_MNU_JUKEBOX,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_JukeboxSelected);
    Connect(ID_MNU_FINDDATA,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemFindDataSelected);
    Connect(ID_MNU_SUPPRESSDOCK_HP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_SuppressDock);
    Connect(ID_MNU_SUPPRESSDOCK_MP,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_SuppressDock);
    Connect(ID_MENUITEM_WINDOWS_PERSPECTIVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHidePerspectivesWindow);
    Connect(ID_MENUITEM_WINDOWS_DOCKALL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuDockAllSelected);
    Connect(ID_MENUITEM11,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ResetWindowsToDefaultPositions);
    Connect(ID_PLAY_FULL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_1POINT5SPEED,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MN_2SPEED,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_3SPEED,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_4SPEED,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_3_4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_LOUDVOLUME,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_LoudVolSelected);
    Connect(ID_MNU_MEDVOLUME,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_MedVolSelected);
    Connect(ID_MNU_QUIET,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_QuietVolSelected);
    Connect(ID_MNU_SUPERQUIET,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_VQuietVolSelected);
    Connect(ID_MNU_SILENT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_SilentVolSelected);
    Connect(ID_IMPORT_EFFECTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItemImportEffects);
    Connect(ID_MNU_TOD,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_TODSelected);
    Connect(ID_MNU_MANUAL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_UserManualSelected);
    Connect(ID_MNU_ZOOM,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ZoomSelected);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_ShowKeyBindingsSelected);
    Connect(idMenuHelpContent,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(ID_MENU_HELP_FORMUM,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_ForumSelected);
    Connect(ID_MNU_VIDEOS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_VideoTutorialsSelected);
    Connect(ID_MENU_HELP_DOWNLOAD,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_DownloadSelected);
    Connect(ID_MNU_HELP_RELEASE_NOTES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_ReleaseNotesSelected);
    Connect(ID_MENU_HELP_ISSUE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_Isue_TrackerSelected);
    Connect(ID_MENU_HELP_FACEBOOK,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_FacebookSelected);
    Connect(ID_MNU_DONATE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_DonateSelected);
    Connect(ID_MNU_UPDATE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuItem_UpdateSelected);
    Connect(wxID_ABOUT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnAbout);
    Connect(ID_TIMER_OutputTimer,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnOutputTimerTrigger);
    Connect(ID_TIMER_AutoSave,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnTimer_AutoSaveTrigger);
    Connect(ID_TIMER_EFFECT_SETTINGS,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnEffectSettingsTimerTrigger);
    Connect(ID_TIMER_RENDERSTATUS,wxEVT_TIMER,(wxObjectEventFunction)&xLightsFrame::OnRenderStatusTimerTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xLightsFrame::OnClose);
    Connect(wxEVT_CHAR,(wxObjectEventFunction)&xLightsFrame::OnChar);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xLightsFrame::OnResize);
    //*)

    wxConfigBase* config = wxConfigBase::Get();
    if (config == nullptr) {
        logger_base.error("Null config ... this wont end well.");
    }

    wxString dir;
    dir.clear();
    bool ok = true;
    bool showDirFromCommandLine = false;
    if (!xLightsApp::showDir.IsNull()) {
        wxString t;
        config->Read("LastDir", &t);

        if (t != xLightsApp::showDir) {
            showDirFromCommandLine = true;
        }
        dir = xLightsApp::showDir;
    } else {
        ok = config->Read("LastDir", &dir);
    }
    logger_base.debug("Show directory %s.", (const char*)dir.c_str());

    if (dir != "") {
#ifdef __WXMSW__
        _tod.PrepTipOfDay(this);
#else
        _tod = new TipOfTheDayDialog("", this);
        _tod->PrepTipOfDay(this);
#endif
    }

    Connect(wxEVT_HELP, (wxObjectEventFunction)&xLightsFrame::OnHelp);
    Notebook1->Connect(wxEVT_HELP, (wxObjectEventFunction) & xLightsFrame::OnHelp, 0, this);

    logger_base.debug("xLightsFrame constructor UI code done.");

    GaugeSizer->Fit(Panel1);
    GaugeSizer->SetSizeHints(Panel1);
    StatusBarSizer->Fit(AUIStatusBar);
    StatusBarSizer->SetSizeHints(AUIStatusBar);

    LoadDockable();

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(xLightsFrame::OnCharHook), nullptr, this);

    // Suppress OSX display of a warning when reading config ... "entry %s appears more than once in group '%s'
    wxLogNull logNo;

    //need to direct these menu items to different places depending on what is active
    Connect(wxID_UNDO, wxEVT_MENU,(wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_REDO, wxEVT_MENU,(wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_CUT, wxEVT_MENU,(wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_COPY, wxEVT_MENU,(wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_PASTE, wxEVT_MENU,(wxObjectEventFunction)&xLightsFrame::DoMenuAction);

    ShowDirectoryLabel->Bind(wxEVT_LEFT_DCLICK,
         [&](wxMouseEvent&) {
             wxLaunchDefaultApplication(showDirectory);
         });

    SetPanelSequencerLabel("");

    _outputModelManager.SetFrame(this);

	mRendering = false;

    _appProgress = std::make_unique<wxAppProgressIndicator>(this);
    _appProgress->SetRange(100);
    _appProgress->Reset();
    

    AddEffectToolbarButtons(effectManager, EffectsToolBar);
    wxSize sz = EffectsToolBar->GetSize();
    wxAuiPaneInfo &info = MainAuiManager->GetPane("EffectsToolBar");
    info.BestSize(sz);
    MainAuiManager->Update();

    wxToolTip::SetAutoPop(20000); // globally set tooltips stay on screen for a long time - may not work on all platforms per wxWidgets documentation

    SetTitle( xlights_base_name + xlights_qualifier + " (Ver " + GetDisplayVersionString() + ") " + xlights_build_date );

    CheckBoxLightOutput = new AUIToolbarButtonWrapper(PlayToolBar, ID_CHECKBOX_LIGHT_OUTPUT);
    ButtonPasteByTime = new AUIToolbarButtonWrapper(EditToolBar, ID_PASTE_BY_TIME);
    ButtonPasteByCell = new AUIToolbarButtonWrapper(EditToolBar, ID_PASTE_BY_CELL);

    Button_ACDisabled = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACDISABLED);
    Button_ACDisabled->SetValue(false);
    Button_ACSelect = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACSELECT);
    Button_ACSelect->SetValue(true);
    Button_ACOn = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACON);
    Button_ACOn->SetValue(false);
    Button_ACOff = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACOFF);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACTWINKLE);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACSHIMMER);
    Button_ACShimmer->SetValue(false);

    Button_ACIntensity = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACINTENSITY);
    Button_ACIntensity->SetValue(true);
    Button_ACRampUp = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUP);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACRAMPDOWN);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUPDOWN);
    Button_ACRampUpDown->SetValue(false);

    Button_ACFill = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACFILL);
    Button_ACFill->SetValue(false);
    Button_ACCascade = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACCASCADE);
    Button_ACCascade->SetValue(false);

    Button_ACForeground = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACFOREGROUND);
    Button_ACForeground->SetValue(false);
    Button_ACBackground = new AUIToolbarButtonWrapper(ACToolbar, ID_AUITOOLBARITEM_ACBACKGROUND);
    Button_ACBackground->SetValue(false);

    ButtonPasteByTime->SetValue(true);
    ButtonPasteByCell->SetValue(false);
    mResetToolbars = false;
    mRenderOnSave = true;
    mSaveFseqOnSave = true;
    mBackupOnSave = false;
    mBackupOnLaunch = true;
    mSuppressFadeHints = false;
    me131Sync = false;
    mAltBackupDir = "";
    mIconSize = 16;
    _modelHandleSize = 1;

    _acParm1Intensity = 100;
    _acParm1RampUp = 0;
    _acParm2RampUp = 100;
    _acParm1RampDown = 0;
    _acParm2RampDown = 100;
    _acParm1RampUpDown = 0;
    _acParm2RampUpDown = 100;
    ChoiceParm1->SetStringSelection("100");
    ChoiceParm2->SetStringSelection("100");

    StatusBarSizer->AddGrowableCol(0,2);
    StatusBarSizer->AddGrowableCol(2,1);
    ProgressBar->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&xLightsFrame::OnProgressBarDoubleClick, nullptr, this);
    ProgressBar->Hide();
    selectedEffectPalette = "";
    selectedEffect = nullptr;
    playStartTime = playEndTime = 0;
    replaySection = false;
    playType = 0;
    playModel = nullptr;
	mLoopAudio = false;
    playSpeed = 1.0;
    playVolume = 100;
    playAnimation = false;
    UnsavedNetworkChanges = false;

    UnsavedRgbEffectsChanges = false;
    mStoredLayoutGroup = "Default";

    modelsChangeCount = 0;

    logger_base.debug("Config: AppName '%s' Path '%s' Entries %d Groups %d Style %ld Vendor %s.",
        (const char *)config->GetAppName().c_str(),
        (const char *)config->GetPath().c_str(),
        (int)config->GetNumberOfEntries(),
        (int)config->GetNumberOfGroups(),
        config->GetStyle(),
        (const char*)config->GetVendorName().c_str());

    config->Read("xLightsPlayControlsOnPreview", &_playControlsOnPreview, false);
    logger_base.debug("Play Controls On Preview: %s.", toStr( _playControlsOnPreview ));

    config->Read("xLightsAutoShowHousePreview", &_autoShowHousePreview, false);
    logger_base.debug("Autoshow House Preview: %s.", toStr( _autoShowHousePreview ));

    config->Read("xLightsSmallWaveform", &_smallWaveform, false);
    logger_base.debug("Small Waveform: %s.", toStr(_smallWaveform ));

    config->Read("xLightsModelBlendDefaultOff", &_modelBlendDefaultOff, false);
    logger_base.debug("Model Blend Default Off: %s.", toStr( _modelBlendDefaultOff ));

    config->Read("xLightsLowDefinitionRender", &_lowDefinitionRender, false);
    logger_base.debug("Low Defintion Render: %s.", toStr(_lowDefinitionRender));

    config->Read("xLightsSnapToTimingMarks", &_snapToTimingMarks, false);
    logger_base.debug("Snap To Timing Marks: %s.", toStr( _snapToTimingMarks ));

    config->Read("xLightsFSEQVersion", &_fseqVersion, 2);

    config->Read("xLightsPlayVolume", &playVolume, 100);
    MenuItem_LoudVol->Check(playVolume == 100);
    MenuItem_MedVol->Check(playVolume == 66);
    MenuItem_QuietVol->Check(playVolume == 33);
    MenuItem_VQuietVol->Check(playVolume == 10);
    MenuItem_SilentVol->Check(playVolume == 0);
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);

    wxString randomEffects = "";
    config->Read("xLightsRandomEffects", &randomEffects);
    if (randomEffects.IsEmpty()) {
        for (int i = 0; i < effectManager.size(); i++) {
            if (effectManager[i]->CanBeRandom()) {
                _randomEffectsToUse.Add(effectManager[i]->Name());
            }
        }
        randomEffects = wxJoin(_randomEffectsToUse, ',');
        config->Write("xLightsRandomEffects", randomEffects);
    }
    else {
        _randomEffectsToUse = wxSplit(randomEffects, ',');
    }

    logger_base.debug("xLightsFrame constructor creating sequencer.");

    CreateSequencer();

    logger_base.debug("xLightsFrame constructor sequencer creation done.");

    layoutPanel = new LayoutPanel(PanelPreview, this, PanelSequencer);
    logger_base.debug("LayoutPanel creation done.");
    FlexGridSizerPreview->Add(layoutPanel, 1, wxALL | wxEXPAND, 5);
    FlexGridSizerPreview->Fit(PanelPreview);
    FlexGridSizerPreview->SetSizeHints(PanelPreview);

    modelPreview = layoutPanel->GetMainPreview();
    logger_base.debug("LayoutPanel setup done.");

    playIcon = wxBitmap(control_play_blue_icon);
    pauseIcon = wxBitmap(control_pause_blue_icon);

    Grid1HasFocus = false; //set this before grid gets any events -DJ

    SetIcons(wxArtProvider::GetIconBundle("xlART_xLights_Icons", wxART_FRAME_ICON));
    logger_base.debug("IconBundle creation done.");

    SetName("xLights");
    wxPersistenceManager::Get().RegisterAndRestore(this);
    logger_base.debug("Window Location Restored.");

    effGridPrevX = 0;
    effGridPrevY = 0;
    mSavedChangeCount = 0;
    mLastAutosaveCount = 0;

    _scrollTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(xLightsFrame::OnListItemScrollTimerControllers), nullptr, this);

    // get list of most recently used directories
    wxString dirmru;
    for (int i = 0; i < MRUD_LENGTH; i++) {
        wxString mru_name = wxString::Format("mru%d",i);
        dirmru.clear();
        if (config->Read(mru_name, &dirmru)) {
            if (!dirmru.IsEmpty()) {
                int idx = mruDirectories.Index(dirmru);
                if (idx == wxNOT_FOUND) mruDirectories.Add(dirmru);
            }
        }
        mrud_MenuItem[i] = nullptr;
    }
    for (int i = 0; i < MRUF_LENGTH; i++) {
        mruf_MenuItem[i] = nullptr;
    }
    MenuFile->FindItem(ID_MENUITEM_RECENTFOLDERS)->SetBitmap(GetMenuItemBitmapBundle("wxART_FOLDER_OPEN"));
    MenuFile->FindItem(ID_MENUITEM_OPENRECENTSEQUENCE)->SetBitmap(GetMenuItemBitmapBundle("wxART_FILE_OPEN"));

    logger_base.debug("xLightsFrame constructor loading config.");

    wxString md;

    if (!xLightsApp::mediaDir.IsNull()) {
        md = xLightsApp::mediaDir;
        logger_base.debug("Media directory %s.", (const char *)md.c_str());
        ObtainAccessToURL(md);
        mediaDirectories.push_back(md);
    } else if (config->Read(_("MediaDir"), &md)) {
        wxArrayString entries = wxSplit(md, '|', '\0');
        for (auto & d : entries) {
            std::string dstd = d.ToStdString();
            ObtainAccessToURL(dstd);
            if (std::find(mediaDirectories.begin(), mediaDirectories.end(), dstd) == mediaDirectories.end()) {
                mediaDirectories.push_back(dstd);
            }
        }
    }
    SetFixFileDirectories(mediaDirectories);
    wxString tbData = config->Read("ToolbarLocations");
    if (tbData.StartsWith(TOOLBAR_SAVE_VERSION)) {
        MainAuiManager->LoadPerspective(tbData.Right(tbData.size() - 5));
    }
    logger_base.debug("Perspectives loaded.");

    config->Read("xLightsBackupSubdirectories", &_backupSubfolders, true);
    logger_base.debug("Backup subdirectories: %s.", toStr( _backupSubfolders ));

    config->Read("xLightsExcludePresetsPkgSeq", &_excludePresetsFromPackagedSequences, false);
    logger_base.debug("Exclude Presets From Packaged Sequences: %s.", toStr( _excludePresetsFromPackagedSequences ));

    config->Read("xLightsPromptBatchRenderIssues", &_promptBatchRenderIssues, true);
    logger_base.debug("Prompt for issues during batch render: %s.", toStr( _promptBatchRenderIssues ));

    // I was willing to default this off ... but after multiple attempts to sneak this in ... this will default off in windows and if it is changed
    // again it will be totally and permanently disabled in windows.
#ifdef __WXMSW__
    bool defVMR = true;
#else
    bool defVMR = false;
#endif
    config->Read("xLightsIgnoreVendorModelRecommendations2", &_ignoreVendorModelRecommendations, defVMR);
    logger_base.debug("Ignore vendor model recommendations: %s.", toStr(_ignoreVendorModelRecommendations));

    config->Read("xLightsPurgeDownloadCacheOnStart", &_purgeDownloadCacheOnStart, false);
    logger_base.debug("Purge download cache on start: %s.", toStr(_purgeDownloadCacheOnStart));

    config->Read("xLightsVideoExportCodec", &_videoExportCodec, "H.264");
    logger_base.debug("Video Export Codec: %s.", (const char*)_videoExportCodec.c_str());

    config->Read("xLightsVideoExportBitrate", &_videoExportBitrate,0);
    logger_base.debug("Video Export Bitrate: %d.", _videoExportBitrate);

    config->Read("xLightsExcludeAudioPkgSeq", &_excludeAudioFromPackagedSequences, false);
    logger_base.debug("Exclude Audio From Packaged Sequences: %s.", toStr( _excludeAudioFromPackagedSequences ));

    config->Read("xLightsShowACLights", &_showACLights, false);
    MenuItem_ACLIghts->Check(_showACLights);
    logger_base.debug("Show AC Lights toolbar: %s.", toStr( _showACLights ));

    config->Read("xLightsShowACRamps", &_showACRamps, false);
    MenuItem_ShowACRamps->Check(_showACRamps);
    logger_base.debug("Show AC Ramps: %s.", toStr( _showACRamps ));

    bool bit64 = GetBitness() == "64bit";
    config->Read(_("xLightsEnableRenderCache"), &_enableRenderCache, _("Locked Only"));

    // Dont enable render caching in 32 bit ... there just isnt enough memory
    if (!bit64) {
        logger_base.debug("Enable Render Cache: false due to running 32 bit.");
        _enableRenderCache = "Disabled";
    }
    logger_base.debug("Enable Render Cache: %s.", (const char*)_enableRenderCache.c_str());
    _renderCache.Enable(_enableRenderCache);

    config->Read("xLightsAutoSavePerspectives", &_autoSavePerspecive, false);
    MenuItem_PerspectiveAutosave->Check(_autoSavePerspecive);
    logger_base.debug("Autosave perspectives: %s.", toStr( _autoSavePerspecive ));

    config->Read("xLightsRenderOnSave", &mRenderOnSave, false);
    logger_base.debug("Render on save: %s.", toStr( mRenderOnSave));

    config->Read("xLightsSaveFseqOnSave", &mSaveFseqOnSave, true);
    logger_base.debug("Save Fseq on save: %s.", toStr( mSaveFseqOnSave ));

    if (!mSaveFseqOnSave) {
        logger_base.debug("Render on save changed to false, because Save Fseq on save is false.");
        mRenderOnSave = false;
    }

    config->Read("xLightsModelHandleSize", &_modelHandleSize, 1);
    logger_base.debug("Model Handle Size: %d.", _modelHandleSize);

    config->Read("xLightsBackupOnSave", &mBackupOnSave, false);
    logger_base.debug("Backup on save: %s.", toStr( mBackupOnSave ));

    config->Read("xLightsBackupOnLaunch", &mBackupOnLaunch, true);
    logger_base.debug("Backup on launch: %s.", toStr( mBackupOnLaunch ));

    config->Read("xLightsSuppressFadeHints", &mSuppressFadeHints, false);
    logger_base.debug("Suppress Transition Hints: %s.", toStr( mSuppressFadeHints ));

    config->Read(_("xLightsAltBackupDir"), &mAltBackupDir);
    logger_base.debug("Alternate Backup Dir: '%s'.", (const char *)mAltBackupDir.c_str());

    if (_purgeDownloadCacheOnStart) {
        PurgeDownloadCache();
    }

    if (wxDir::Exists(mAltBackupDir)) {
        ObtainAccessToURL(mAltBackupDir);
        mAltBackupMenuItem->SetHelp(mAltBackupDir);
    }
    else {
        mAltBackupMenuItem->SetHelp("");
    }

    config->Read("xLightsIconSize", &mIconSize, 16);
    SetToolIconSize(mIconSize);
    logger_base.debug("Icon size: %d.", mIconSize);

    config->Read("AutoSaveInterval", &mAutoSaveInterval, 3);
    SetAutoSaveInterval(mAutoSaveInterval);
    logger_base.debug("Autosave interval: %d.", mAutoSaveInterval);

    config->Read("xFadePort", &_xFadePort, 0);

    // overide ab setting from command line
    if (ab != 0) {
        _xFadePort = ab;
    }

    logger_base.debug("xFadePort: %s.", _xFadePort == 0 ? "Disabled" : ((_xFadePort == 1) ? "A" : "B"));
    StartAutomationListener();

    if (_xFadePort > 0 && _automationServer == nullptr) {
        // try opening it on the other port

        if (_xFadePort == 1) {
            _xFadePort = 2;
        } else if (_xFadePort == 2) {
            _xFadePort = 1;
        }

        StartAutomationListener();
        if (_xFadePort > 0 && _automationServer == nullptr) {
            // Give up
            _xFadePort = 0;
        }
    }

    config->Read("BackupPurgeDays", &BackupPurgeDays, 0);
    logger_base.debug("Backup purge age: %d days.", BackupPurgeDays);

    int glVer = 99;
    config->Read("ForceOpenGLVer", &glVer, 99);
    if (glVer != 99) {
        int lastGlVer;
        config->Read("LastOpenGLVer", &lastGlVer, 0);
        if (lastGlVer == 0) {
            config->Write("ForceOpenGLVer", 99);
            glVer = 99;
        } else if (glVer != lastGlVer) {
            CallAfter(&xLightsFrame::MaybePackageAndSendDebugFiles);
        }
    }
    logger_base.debug("Force OpenGL version: %d.", glVer);
    config->Write("LastOpenGLVer", glVer);

    config->Read("xLightsGridSpacing", &mGridSpacing, 16);
    SetGridSpacing(mGridSpacing);
    logger_base.debug("Grid spacing: %d.", mGridSpacing);

    config->Read("xLightsGridIconBackgrounds", &mGridIconBackgrounds, true);
    SetGridIconBackgrounds(mGridIconBackgrounds);
    logger_base.debug("Grid icon backgrounds: %s.", toStr( mGridIconBackgrounds ));

    config->Read("xLightsTimingPlayOnDClick", &mTimingPlayOnDClick, true);
    SetTimingPlayOnDClick(mTimingPlayOnDClick);
    logger_base.debug("Timing Play on DClick: %s.", toStr( mTimingPlayOnDClick ));

    config->Read("xLightsGridNodeValues", &mGridNodeValues, true);
    SetGridNodeValues(mGridNodeValues);
    logger_base.debug("Grid node values: %s.", toStr( mGridNodeValues ));

    config->Read("xLightsEffectAssistMode", &mEffectAssistMode, EFFECT_ASSIST_TOGGLE_MODE);
    if (mEffectAssistMode < 0 || mEffectAssistMode > EFFECT_ASSIST_TOGGLE_MODE) {
        mEffectAssistMode = EFFECT_ASSIST_TOGGLE_MODE;
    }
    logger_base.debug("Effect Assist Mode: %d.", mEffectAssistMode);
    if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    }
    else {
        SetEffectAssistWindowState(false);
    }

    InitEffectsPanel(EffectsPanel1);
    logger_base.debug("Effects panel initialised.");

    auto consash = config->ReadLong("xLightsControllerSash", SplitterWindowControllers->GetSashPosition());
    SplitterWindowControllers->SetSashPosition(consash);
    logger_base.debug("Controller Sash Position: %d.", consash);

    EffectTreeDlg = nullptr;  // must be before any call to SetDir

    starttime = wxDateTime::UNow();
    ResetEffectsXml();
    EnableSequenceControls(true);
    if (ok && !dir.IsEmpty()) {
        if (!SetDir(dir, !showDirFromCommandLine)) {
            CurrentDir = "";
            if (!PromptForShowDirectory(true)) {
                CurrentDir = "";
                splash.Hide();
                wxMessageBox("Exiting as setting a show folder is not optional.");
                wxExit();
                return;
            }
        }
        else {
            if (ShowFolderIsInBackup(dir.ToStdString())) {
                DisplayWarning("WARNING: Opening a show folder inside a backup folder. This is generally a very very bad idea.", this);
            }

#ifdef __WXMSW__
            else if (ShowFolderIsInProgramFiles(dir.ToStdString())) {
                DisplayWarning("ERROR: Show folder inside your Program Files folder either just wont work or will cause you security issues.", this);
            }
#endif
        }
    }
    else {
        if (!PromptForShowDirectory(true)) {
            CurrentDir = "";
            splash.Hide();
            wxMessageBox("Exiting as setting a show folder is not optional.");
            wxExit();
            return;
        }
    }

    // This is for keith ... I like my debug version to be distinctive so I can tell it apart from the prior version
#ifndef NDEBUG
    logger_base.debug("xLights Crash Menu item not removed.");
#ifdef _MSC_VER
    if (wxSystemSettings::GetAppearance().IsDark()) {
        Notebook1->SetBackgroundColour(wxColour(0x006000));
    } else {
        Notebook1->SetBackgroundColour(*wxGREEN);
    }
#endif
#else
    // only keep the crash option if the special option is set
    if (::Lower(SpecialOptions::GetOption("EnableCrash", "false")) == "false") {
        MenuItem_CrashXLights->GetMenu()->Remove(MenuItem_CrashXLights);
        MenuItem_CrashXLights = nullptr;
        MenuItem_LogRenderState->GetMenu()->Remove(MenuItem_LogRenderState);
        MenuItem_LogRenderState = nullptr;
    }
    else {
        logger_base.debug("xLights Crash Menu item not removed.");
    }
#endif

    if (IsFromAppStore()) {
        MenuItem_Update->GetMenu()->Remove(MenuItem_Update);
        MenuItem_Update = nullptr;
    }

    _valueCurvesPanel->UpdateValueCurveButtons(false);
    _coloursPanel->UpdateColourButtons(false, this);

    MixTypeChanged=true;

    // This is used by xSchedule
    Notebook1->SetLabel("XLIGHTS_NOTEBOOK");

    Notebook1->ChangeSelection(SETUPTAB);
    EnableNetworkChanges();

    wxImage::AddHandler(new wxGIFHandler);

    config->Read("xLightse131Sync", &me131Sync, false);
    _outputManager.SetSyncEnabled(me131Sync);
    logger_base.debug("Sync: %s.", toStr( me131Sync ));

    // this is no longer used ... as it is now stored in the networks file
    wxString tmpString;
    config->Read("xLightsLocalIP", &tmpString, "");
    if (IsValidLocalIP(tmpString) && tmpString != "") {
        _outputManager.SetGlobalForceLocalIP(tmpString);
        config->DeleteEntry("xLightsLocalIP");
    }

    SetControllersProperties();
    UpdateACToolbar();
    ShowACLights();

    DoBackupPurge();

    //start out with 50ms timer, once we load a file or create a new one, we'll reset
    //to whatever the timing that is selected.   If the timer triggers and is then not
    //needed, it will be turned off later
    OutputTimer.Start(50, wxTIMER_CONTINUOUS);

    // What makes 4 the right answer ... try 10 ... why ... usually it is one thread that runs slow and that model
    // holds up others so in the time while we wait for the busy thread we can actually run a lot more models
    // what is the worst that could happen ... all models want to run hard so we lose some efficiency while we churn between
    // threads ... a minor loss of efficiency ... I think the one thread blocks the others is more common.
    // Dan is concerned on 32 bit windows 10 will chew up too much heap memory ... so splitting the difference we get 7
    int multiplier = (sizeof(size_t) == 8) ? 10 : 7;
    if (GetPhysicalMemorySizeMB() > 12*1024) {
        // if we have over 12GB of memory, creating more threads shouldn't be an issue
        multiplier *= 2;
    }
    int threadCount = wxThread::GetCPUCount() * multiplier;
    if (threadCount < 20) {
        threadCount = 20;
    }
    jobPool.Start(threadCount);

    if (!xLightsApp::sequenceFiles.IsEmpty())
    {
        logger_base.debug("Opening sequence: %s.", (const char *)xLightsApp::sequenceFiles[0].c_str());
        OpenSequence(xLightsApp::sequenceFiles[0], nullptr);
    }

    SetAudioControls();

#ifdef __WXOSX__
    config->Read(_("xLightsVideoReaderAccelerated"), &_hwVideoAccleration, true);
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
    VideoReader::InitHWAcceleration();

    MenuItem_xSchedule->GetMenu()->Remove(MenuItem_xSchedule->GetId());
    MenuItem_xSchedule = nullptr;
    MenuItem_xScanner->GetMenu()->Remove(MenuItem_xScanner->GetId());
    MenuItem_xScanner = nullptr;


    MenuFile->AppendSeparator();
    const long newInstId = wxNewId();
    wxMenuItem *newInst = new wxMenuItem(MenuFile, newInstId, _("Open New xLights Instance"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(newInst);

    Connect(newInstId, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_NewXLightsInstance);


    bool gpuRendering = false;
    config->Read(_("xLightsGPURendering"), &gpuRendering, false);
    GPURenderUtils::SetEnabled(gpuRendering);
    
    _taskBarIcon = std::make_unique<xlMacDockIcon>(this);
#else
    config->Read(_("xLightsVideoReaderAccelerated"), &_hwVideoAccleration, false);
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
#endif

#ifdef __WXMSW__
    //make sure Direct2DRenderer is created on the main thread before the other threads need it
    wxGraphicsRenderer::GetDirect2DRenderer();
#endif

    bool bgShaders = false;
    config->Read(_("xLightsShadersOnBackgroundThreads"), &bgShaders, false);
    ShaderEffect::SetBackgroundRender(bgShaders);

    DrawingContext::Initialize(this);

    MenuItem_File_Save->Enable(true);
    MenuItem_File_Save->SetItemLabel("Save Setup\tCTRL-s");

    splash.Hide();

    config->Read("xLightsUserEmail", &_userEmail, "");

    config->Read("xLightsLinkedSave", &_linkedSave, "Controllers and Layout Tab");
    logger_base.debug("Linked save: %s.", (const char*)_linkedSave.c_str());

    config->Read("xLightsLinkedControllerUpload", &_linkedControllerUpload, "Inputs and Outputs");
    logger_base.debug("Linked controller upload: %s.", (const char*)_linkedControllerUpload.c_str());


    std::thread th([this]() {
        try
        {
            xlCrashHandler::SetupCrashHandlerForNonWxThread();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            this->CallAfter(&xLightsFrame::DoPostStartupCommands);
        }
        catch (...)
        {
            wxTheApp->OnUnhandledException();
        }
    });
    th.detach();
    wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

    UpdateLayoutSave();
    UpdateControllerSave();

    // remove the forum for now until/if Sean restores the forum
    MenuItem_Help_Forum->GetMenu()->Remove(MenuItem_Help_Forum);
    MenuItem_Help_Forum = nullptr;

    logger_base.debug("xLightsFrame construction complete.");
}

xLightsFrame::~xLightsFrame()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static bool reenter = false;

    if (reenter) {
        logger_base.error("~xLightsFrame re-entered ... this wont end well ... so bailing now.");
        return;
    }
    reenter = true;
    //make sure we abort any render that is going on or we could crash as things get destroyed
    //however, only wait 2000ms to avoid complete hang on shutdown if a
    //render thread is completely stuck
    AbortRender(2000);

    AutoSaveTimer.Stop();
    EffectSettingsTimer.Stop();
    OutputTimer.Stop();
    RenderStatusTimer.Stop();
    DrawingContext::CleanUp();

    if (_automationServer != nullptr) {
        _automationServer->Stop();
        delete _automationServer;
        _automationServer = nullptr;
    }

    selectedEffect = nullptr;
    _outputManager.AllOff();
    _outputManager.StopOutput();

    wxConfigBase* config = wxConfigBase::Get();
    if (mResetToolbars)
    {
        config->DeleteEntry("ToolbarLocations");
    }
    else
    {
        config->Write("ToolbarLocations", TOOLBAR_SAVE_VERSION + MainAuiManager->SavePerspective());
    }
    config->Write("xLightsIconSize", mIconSize);
    config->Write("xLightsGridSpacing", mGridSpacing);
    config->Write("xLightsGridIconBackgrounds", mGridIconBackgrounds);
    config->Write("xLightsTimingPlayOnDClick", mTimingPlayOnDClick);
    config->Write("xLightsGridNodeValues", mGridNodeValues);
    config->Write("xLightsRenderOnSave", mRenderOnSave);
    config->Write("xLightsSaveFseqOnSave", mSaveFseqOnSave);
    config->Write("xLightsBackupSubdirectories", _backupSubfolders);
    config->Write("xLightsExcludePresetsPkgSeq", _excludePresetsFromPackagedSequences);
    config->Write("xLightsPromptBatchRenderIssues", _promptBatchRenderIssues);
    config->Write("xLightsIgnoreVendorModelRecommendations2", _ignoreVendorModelRecommendations);
    config->Write("xLightsPurgeDownloadCacheOnStart", _purgeDownloadCacheOnStart);
    config->Write("xLightsExcludeAudioPkgSeq", _excludeAudioFromPackagedSequences);
    config->Write("xLightsShowACLights", _showACLights);
    config->Write("xLightsShowACRamps", _showACRamps);
    config->Write("xLightsEnableRenderCache", _enableRenderCache);
    config->Write("xLightsPlayControlsOnPreview", _playControlsOnPreview);
    config->Write("xLightsAutoShowHousePreview", _autoShowHousePreview);
    config->Write("xLightsModelBlendDefaultOff", _modelBlendDefaultOff);
    config->Write("xLightsLowDefinitionRender", _lowDefinitionRender);
    config->Write("xLightsSnapToTimingMarks", _snapToTimingMarks);
    config->Write("xLightsFSEQVersion", _fseqVersion);
    config->Write("xLightsAutoSavePerspectives", _autoSavePerspecive);
    config->Write("xLightsBackupOnSave", mBackupOnSave);
    config->Write("xLightsBackupOnLaunch", mBackupOnLaunch);
    config->Write("xLightsSuppressFadeHints", mSuppressFadeHints);
    config->Write("xLightse131Sync", me131Sync);
    config->Write("xLightsEffectAssistMode", mEffectAssistMode);
    config->Write("xLightsAltBackupDir", mAltBackupDir);
    config->Write("xFadePort", _xFadePort);
    config->Write("xLightsModelHandleSize", _modelHandleSize);
    config->Write("xLightsPlayVolume", playVolume);
    config->Write("xLightsVideoExportCodec", _videoExportCodec);
    config->Write("xLightsVideoExportBitrate", _videoExportBitrate);

    config->Write("xLightsControllerSash", SplitterWindowControllers->GetSashPosition());

    SaveDockable();

    //definitely not outputting data anymore
    config->Write("OutputActive", false);

    config->Flush();

    //must call these or the app will crash on exit
    m_mgr->UnInit();
    MainAuiManager->UnInit();

    for (int x = 0; x < Notebook1->GetPageCount(); x++) {
        wxWindow *w = Notebook1->GetPage(x);
        if (w->GetEventHandler() == m_mgr) {
            w->RemoveEventHandler(m_mgr);
        }
    }

    //unconnect these as the call to DeleteAllPages will cause pages to change and the page numbers to possibly not match
    Disconnect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged1);
    Disconnect(ID_NOTEBOOK1,wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING,(wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanging);

    // I still have examples where this crashes ... maybe we should just not delete them
    Notebook1->DeleteAllPages();

    delete m_mgr;
    delete MainAuiManager;

    _outputManager.DeleteAllControllers();

    if( CurrentSeqXmlFile )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = nullptr;
    }

    delete CheckBoxLightOutput;
    delete ButtonPasteByTime;
    delete ButtonPasteByCell;

    delete Button_ACDisabled;
    delete Button_ACSelect;
    delete Button_ACOn;
    delete Button_ACOff;
    delete Button_ACTwinkle;
    delete Button_ACShimmer;
    delete Button_ACIntensity;
    delete Button_ACRampUp;
    delete Button_ACRampDown;
    delete Button_ACRampUpDown;
    delete Button_ACFill;
    delete Button_ACCascade;
    delete Button_ACForeground;
    delete Button_ACBackground;

    #ifndef __WXMSW__
    if (_tod != nullptr) delete _tod;
    #endif

    //(*Destroy(xLightsFrame)
    //*)

    if (_logfile != nullptr)
    {
        wxLog::SetActiveTarget(nullptr);
        fclose(_logfile);
        _logfile = nullptr;
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Error);
    }

    reenter = false;
}

bool xLightsFrame::IsCheckSequenceOptionDisabled(const std::string& option)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config == nullptr) {
        return false;
    }
    bool value = false;
    config->Read("xLightsCS" + option, &value, false);
    return value;
}

void xLightsFrame::SetCheckSequenceOptionDisable(const std::string& option, bool value)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("xLightsCS" + option, value);
    }
}

void xLightsFrame::OnHelp(wxHelpEvent& event)
{
    // deliberately do nothing
}

void xLightsFrame::DoPostStartupCommands() {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("In Post Startup");

    // dont check for updates if batch rendering
    if (!_renderMode && !_checkSequenceMode) {
// Don't bother checking for updates when debugging.
#ifndef _DEBUG
        if (!IsFromAppStore()) {
            CheckForUpdate(1, true, false);
        }
#endif
        if (_userEmail == "") CollectUserEmail();
        if (_userEmail != "noone@nowhere.xlights.org") logger_base.debug("User email address: <email>%s</email>", (const char*)_userEmail.c_str());
        
#ifdef __WXMSW__
        int verMaj = -1;
        int verMin = -1;
        wxOperatingSystemId o = wxGetOsVersion(&verMaj, &verMin);
        static bool hasWarned = false;
        if (verMaj < 8 && !hasWarned) {
            hasWarned = true;
            wxMessageBox("Windows 7 has known issues rendering some effects.  Support for Windows 7 may be removed entirely soon.",
                         "Windows Version",
                          wxICON_INFORMATION | wxCENTER | wxOK);
        }
#endif
    }
}

void xLightsFrame::DoMenuAction(wxMenuEvent &evt) {
    static bool inMenuAction = false;
    if (inMenuAction) {
        return;
    }
    inMenuAction = true;
    wxWindow *w = FindFocus();
    evt.Skip();
    if (w != nullptr && w->GetEventHandler() != nullptr) {
        w->GetEventHandler()->ProcessEventLocally(evt);
    }
    inMenuAction = false;
}

void xLightsFrame::OnQuit(wxCommandEvent& event)
{
    static bool inQuit = false;

    if (inQuit)
        return;

    inQuit = true;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Quit");
    wxCloseEvent evt;
    if (QuitMenuItem->IsEnabled()) {
        OnClose(evt);
    }

    inQuit = false;
}

void xLightsFrame::InitEffectsPanel(EffectsPanel* panel)
{
    panel->CurrentDir = &CurrentDir;
}

void xLightsFrame::LogPerspective(const wxString & perspective) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxArrayString entries = wxSplit(perspective, '|');
    for (const auto& it : entries) {
        TraceLog::AddTraceMessage(it.ToStdString());
        logger_base.debug("    %s", (const char *)it.c_str());
    }
}

void xLightsFrame::OnAbout(wxCommandEvent& event)
{
    wxString hdg = wxString::Format(_("About xLights %s"), GetDisplayVersionString());
    wxString ver = wxString::Format(_("Version: %s\n%s"), GetDisplayVersionString(), wxVERSION_STRING);
    AboutDialog dlg(this);

    dlg.IconBitmap->SetIcon(wxArtProvider::GetIconBundle("xlART_xLights_Icons", wxART_FRAME_ICON).GetIcon(wxSize(128, 128)));

    dlg.VersionLabel->SetLabel(ver);
    dlg.SetTitle(hdg);
    dlg.LegalTextLabel->SetLabel(XLIGHTS_LICENSE);
    dlg.MainSizer->Fit(&dlg);
    dlg.MainSizer->SetSizeHints(&dlg);

    dlg.LegalTextLabel->Wrap(dlg.LegalTextLabel->GetClientSize().GetWidth() - 10);
    dlg.MainSizer->Fit(&dlg);
    dlg.MainSizer->SetSizeHints(&dlg);

    if (IsFromAppStore()) {
        dlg.EULAHyperlinkCtrl->SetLabel("End User License Agreement");
        dlg.EULAHyperlinkCtrl->SetURL("http://kulplights.com/xlights/eula.html");
        dlg.EULAHyperlinkCtrl->Show();
    }

    dlg.ShowModal();
}

void xLightsFrame::OnOutputTimerTrigger(wxTimerEvent& event)
{
    PushTraceContext();
    wxTimeSpan ts = wxDateTime::UNow() - starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    bool needTimer = _outputManager.IsOutputting();
    AddTraceMessage("OutputTimer");
    _outputManager.StartFrame(curtime);
    AddTraceMessage("Output frame started");
    if (Notebook1 != nullptr) {
        switch (Notebook1->GetSelection()) {
        case NEWSEQUENCER:
            if (playAnimation) {
                needTimer |= TimerRgbSeq(curtime * playSpeed);
            } else {
                needTimer |= TimerRgbSeq(curtime);
            }
            break;
        default:
            break;
        }
    }
    AddTraceMessage("TimerRgbSeq called");
    _outputManager.EndFrame();
    AddTraceMessage("Output frame complete");
    if (!needTimer) {
        //printf("Stopping timer\n");
        OutputTimer.Stop();
    }

    PopTraceContext();
}

void xLightsFrame::OnBitmapButtonTabInfoClick(wxCommandEvent& event)
{
    event.Skip();

    wxString caption, msg;

    switch (Notebook1->GetSelection()) {
    case SETUPTAB:
        caption = _("Setup Tab");
        msg = _("Show Directory\n\nThe first thing you need to know about xLights is that it expects you to organize all of your sequence files and associated audio or video files into a single directory. For example, you can have a directory called '2012 Show'. Once you have your show directory created and populated with the relevant files, you are ready to proceed. Tell xLights where your new show directory is by clicking the 'Change' button on the Setup tab, navigate to your show directory, then click 'OK'.\n\nLighting Networks\n\nThe next thing you will need to do is define your lighting network(s). xLights ignores most of the information about your lighting network contained in your LOR or Vixen sequence. Thus this step is very important! Add a row in the lower half of the Setup tab for each network used in your display. xLights can drive a mixture of network types (for example, the first network can be DMX, and the second one LOR, and the third one Renard). When you are finished, do not forget to SAVE YOUR CHANGES by clicking the 'Save Setup' button.");
        break;
    case LAYOUTTAB:
        caption = _("Layout Tab");
        msg = _("Create display elements by clicking on the Models buttons. You can drag your cursor across the preview area to move the element. Don't forget to click the Save button to save your preview!\n\nClick the Open button to select an xLights sequence to be previewed. Note that any xLights sequence can be previewed, not just those created on the Sequencer tab. Click Play to start preview playback. Use the Pause button to stop play, and then the Play button to resume. You can drag the slider that appears across the top of the preview area to move playback to any spot in your sequence. The Stop Now button in the upper left will also stop playback.");
        break;
    case NEWSEQUENCER:
        caption = _("Sequencer Tab");
        msg = _("The Sequencer tab can be used to create RGB sequences. First, create a model of your RGB display element(s) by clicking on the Models button. Then try the different effects and settings until you create something you like. You can save the settings as a preset by clicking the New Preset button. From then on, that preset will be available in the presets drop-down list. You can combine effects by creating a second effect in the Effect 2 area, then choosing a Layering Method. To create a series of effects that will be used in a sequence, click the open file icon to open an xLights sequence. Choose which display elements/models you will use in this sequence. Then click the insert rows icon and type in the start time in seconds when that effect should begin. Rows will automatically sort by start time. To add an effect to the sequence, click on the grid cell in the desired display model column and the desired start time row, then click the Update button. When you are done creating effects for the sequence, click the save icon and the xLights sequence will be updated with the effects you stored in the grid.");
        break;
    default:
        break;
    }
    wxMessageBox(msg, caption, 5L, this);
}

void xLightsFrame::ResetAllSequencerWindows()
{
    wxAuiPaneInfoArray& info = m_mgr->GetAllPanes();
    bool update = false;
    for (size_t x = 0; x < info.size(); x++) {
        if (info[x].IsFloating() && info[x].IsShown()) {
            info[x].Dock();
            update = true;
        }
    }
    if (update) {
        m_mgr->Update();
    }
}

void xLightsFrame::ShowHideAllSequencerWindows(bool show)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // this logging is extra until we find out why this function crashes
    logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows");

    if (m_mgr == nullptr) {
        logger_base.crit("ShowHideAllSequencerWindows m_mgr is null ... this is going to crash");
    }
    wxAuiPaneInfoArray& info = m_mgr->GetAllPanes();
    bool update = false;
    if (show && savedPaneShown.size() > 0) {
        logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows - show %d %d", (int)info.size(), (int)savedPaneShown.size());
        for (size_t x = 0; x < info.size(); x++) {
            logger_base.debug("     %s", (const char*)info[x].name.c_str());
            if (info[x].IsOk() &&
                savedPaneShown.find(info[x].name) != savedPaneShown.end() &&
                savedPaneShown[info[x].name]) {
                if (info[x].frame != nullptr)
                    info[x].frame->Show();
            }
        }
        savedPaneShown.clear();
    }
    else {
        savedPaneShown.clear();
        logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows - hide %d", (int)info.size());
        for (size_t x = 0; x < info.size(); x++) {
            logger_base.debug("     %s", (const char*)info[x].name.c_str());
            savedPaneShown[info[x].name] = false;
            if (info[x].IsOk()) {
                if (info[x].frame != nullptr) {
                    if (info[x].IsFloating() && info[x].IsShown()) {
                        savedPaneShown[info[x].name] = true;
                    }
                    info[x].frame->Hide();
                }
            }
            else {
                logger_base.warn("Pane %d was not valid ... ShowHideAllSequencerWindows", x);
            }
        }
    }

    if (update) {
        logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows - update");
        m_mgr->Update();
    }

    // show/hide Layout Previews
    logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows - layout previews");
    for (const auto& it : LayoutGroups) {
        if (it->GetMenuItem() == nullptr) {
            logger_base.crit("ShowHideAllSequencerWindows grp->GetMenuItem() is null ... this is going to crash");
        }
        if (it->GetMenuItem() && it->GetMenuItem()->IsChecked()) {
            it->SetPreviewActive(show);
        }
    }

    // Handle the effect Assist
    if (mEffectAssistMode == EFFECT_ASSIST_TOGGLE_MODE) {
        if (sEffectAssist->GetPanel() != sEffectAssist->GetDefaultAssistPanel() && sEffectAssist->GetPanel() != nullptr) {
            SetEffectAssistWindowState(true);
        }
        else {
            SetEffectAssistWindowState(false);
        }
    }
    else if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    }
    else {
        SetEffectAssistWindowState(false);
    }
    UpdateViewMenu();

    logger_base.debug("xLightsFrame::ShowHideAllSequencerWindows - layout previews - done");
}

void xLightsFrame::RecalcModels()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        RecalcModels.");

    if (IsExiting()) return;

    SetCursor(wxCURSOR_WAIT);

    //abort any render as it will crash if the model changes
    AbortRender();
    if (AllModels.RecalcStartChannels()) {
        // Now notify the layout as the model start numbers may have been impacted
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "RecalcModels");
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "RecalcModels");
    }

    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::OnNotebook1PageChanging(wxAuiNotebookEvent& event)
{
    static bool isChanging = false;
    if (layoutPanel == nullptr) {
        event.Veto();
        return;
    }

    // In some cases on OSX, the ShowHideAllSequencerWindows can cause this
    // method ot be re-called recursively and thus trigger a stack overflow eventually.
    // If we are already changing, we'll bail.
    if (isChanging) {
        event.Veto();
        return;
    }
    isChanging = true;
    if (event.GetOldSelection() == NEWSEQUENCER) {
        layoutPanel->Set3d(_housePreviewPanel->Is3d());
        ShowHideAllSequencerWindows(false);
    } else if (event.GetOldSelection() == SETUPTAB) {
        layoutPanel->UnSelectAllModels();
    } else if (event.GetOldSelection() == LAYOUTTAB) {
        _housePreviewPanel->Set3d(layoutPanel->Is3d());
    }
    if (event.GetSelection() == SETUPTAB) {
        DoSetupWork();
    } else if (event.GetSelection() == LAYOUTTAB) {
        DoLayoutWork();
    }
    isChanging = false;
}

void xLightsFrame::RenderLayout()
{
    layoutPanel->RenderLayout();
}

void xLightsFrame::OnNotebook1PageChanged1(wxAuiNotebookEvent& event)
{
    int pagenum = event.GetSelection(); // Notebook1->GetSelection();
    if (pagenum == LAYOUTTAB) {
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "OnNotebook1PageChanged");
        SetStatusText(_(""));
        MenuItem_File_Save->Enable(true);
        MenuItem_File_Save->SetItemLabel("Save Layout\tCTRL-s");
    } else if (pagenum == NEWSEQUENCER) {
        InitSequencer();
        ShowHideAllSequencerWindows(true);
        EffectSettingsTimer.Start(50, wxTIMER_ONE_SHOT);
        MenuItem_File_Save->SetItemLabel("Save Sequence\tCTRL-s");
        MenuItem_File_Save->Enable(MenuItem_File_SaveAs_Sequence->IsEnabled());
    } else if (pagenum == SETUPTAB) {
        MenuItem_File_Save->SetItemLabel("Save Setup\tCTRL-s");
        MenuItem_File_Save->Enable(true);
        SetStatusText(_(""));
        SetControllersProperties();
    } else {
        MenuItem_File_Save->SetItemLabel("Save");
        SetStatusText(_(""));
    }
    SetAudioControls();
}

void xLightsFrame::CycleOutputsIfOn() {
    if (_outputManager.IsOutputting()) {
        _outputManager.StopOutput();
        EnableSleepModes();
        ForceEnableOutputs();
    }
}

bool xLightsFrame::ForceEnableOutputs(bool startTimer) {
    bool outputting = false;
    if (!_outputManager.IsOutputting()) {
        DisableSleepModes();
        outputting = _outputManager.StartOutput();
        if (startTimer) {
            OutputTimer.Start(_seqData.FrameTime(), wxTIMER_CONTINUOUS);
        }
        if (outputting) {
            for (auto &controller : _outputManager.GetControllers()) {
                if (controller->IsActive() && controller->IsAutoUpload() && controller->SupportsAutoUpload()) {
                    auto ip = controller->GetResolvedIP();
                    if (ip == "" || ip == "MULTICAST" || controller->GetProtocol() == OUTPUT_ZCPP) {
                        continue;
                    }
                    BaseController* bc = BaseController::CreateBaseController(controller);
                    if (bc != nullptr && bc->IsConnected()) {
                        if (bc->UploadForImmediateOutput(&AllModels, &_outputManager, controller, this)) {
                            SetStatusText(controller->GetName() + " Upload Complete.");
                        } else {
                            SetStatusText(controller->GetName() + " Upload Failed.");
                        }
                    } else {
                        SetStatusText(controller->GetName() + " Upload Failed. Unable to connect");
                    }
                    if (bc) {
                        delete bc;
                    }
                    //upload config
                }
            }
        }
    }
    return outputting;
}


bool xLightsFrame::EnableOutputs(bool ignoreCheck) {
    if (!ignoreCheck && _outputManager.IsOutputOpenInAnotherProcess()) {
        DisplayWarning("Another process seems to be outputting to lights right now. This may not generate the result expected.", this);
    }
    bool ok = ForceEnableOutputs();
    CheckBoxLightOutput->SetBitmap(GetToolbarBitmapBundle("xlART_OUTPUT_LIGHTS_ON"));
    CheckBoxLightOutput->SetValue(true);
    EnableNetworkChanges();
    return ok;
}
bool xLightsFrame::DisableOutputs() {
    if (_outputManager.IsOutputting()) {
        _outputManager.AllOff();
        _outputManager.StopOutput();
        EnableSleepModes();

        for (auto &controller : _outputManager.GetControllers()) {
            if (controller->IsActive() && controller->IsAutoUpload() && controller->SupportsAutoUpload()) {
                auto ip = controller->GetResolvedIP();
                if (ip == "" || ip == "MULTICAST" || controller->GetProtocol() == OUTPUT_ZCPP) {
                    continue;
                }
                BaseController* bc = BaseController::CreateBaseController(controller);
                if (bc != nullptr && bc->IsConnected()) {
                    bc->ResetAfterOutput(&_outputManager, controller, this);
                }
                if (bc) {
                    delete bc;
                }
            }
        }
    }
    CheckBoxLightOutput->SetBitmap(GetToolbarBitmapBundle("xlART_OUTPUT_LIGHTS"));
    CheckBoxLightOutput->SetValue(false);
    EnableNetworkChanges();
    return true;
}
bool xLightsFrame::ToggleOutputs(bool ignoreCheck) {
    if (CheckBoxLightOutput->IsChecked()) {
       return DisableOutputs();
    }
    return EnableOutputs(ignoreCheck);
}


void xLightsFrame::OnCheckBoxLightOutputClick(wxCommandEvent& event)
{
    if (event.GetInt()) {
        EnableOutputs();
    } else {
        DisableOutputs();
    }
}

//factored out from below so it can be reused by play/pause button -DJ
void xLightsFrame::StopNow()
{
    int actTab = Notebook1->GetSelection();
	if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr)
	{
		CurrentSeqXmlFile->GetMedia()->Stop();
	}
    starttime = wxDateTime::UNow();
    switch (actTab)
    {
    case NEWSEQUENCER:
        {
            wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
            wxPostEvent(this, playEvent);
        }
        break;
    default:
        break;
    }
}

bool xLightsFrame::ShowFolderIsInBackup(const std::string showdir)
{
    int i = showdir.length() - 1;
    wxString dir = "";
    while (i >= 0)
    {
        if (showdir[i] == '\\' || showdir == '/')
        {
            if (dir.Lower() == "backup")
            {
                return true;
            }
            dir = "";
        }
        else if (showdir[i] == ':')
        {
            return false;
        }
        else
        {
            dir = showdir[i] + dir;
        }
        i--;
    }

    return false;
}

bool xLightsFrame::ShowFolderIsInProgramFiles(const std::string showdir)
{
#ifdef __WXMSW__
    wxString sd(showdir);
    wxString first = sd.AfterFirst('\\').BeforeFirst('\\');
    if (first == "Program Files" || first == "Program Files (x86)")
    {
        return true;
    }
#endif

    return false;

}

//make these static so they can be accessed outside of xLightsFrame: -DJ
//NOTE: this assumes there will only be one xLightsMain object
wxString xLightsFrame::CurrentDir = "";
wxString xLightsFrame::FseqDir = "";
wxString xLightsFrame::PlaybackMarker = "";
wxString xLightsFrame::xlightsFilename = "";
xLightsXmlFile* xLightsFrame::CurrentSeqXmlFile = nullptr;

void xLightsFrame::OnClose(wxCloseEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!QuitMenuItem->IsEnabled()) {
        return;
    }

    static bool inClose = false;

    if (inClose) return;

    inClose = true;

    logger_base.info("xLights Closing");

    StopNow();

    if (!CloseSequence())
    {
        logger_base.info("Closing aborted.");
        event.Veto();
        inClose = false;
        return;
    }

    selectedEffect = nullptr;

    CheckUnsavedChanges();

    _exiting = true;

    ShowHideAllSequencerWindows(false);

    logger_base.debug("Destroying %d preview windows.", (int)PreviewWindows.size());
    // destroy preview windows
    for (const auto& it : PreviewWindows) {
        ModelPreview* preview = it;
        delete preview;
    }

    logger_base.debug("Heartbeat exit.");

    Destroy();
    logger_base.info("xLights Closed.");

    inClose = false;
}

void xLightsFrame::DoBackup(bool prompt, bool startup, bool forceallfiles)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString folderName;
    time_t cur;
    time(&cur);
    wxFileName newDirH;
    wxDateTime curTime(cur);

    //  first make sure there is a Backup sub directory
    if (_backupDirectory == "") {
        wxMessageBox("Backup directory has not been set. Aborting backup.");
        return;
    }

    wxString newDirBackup = _backupDirectory + wxFileName::GetPathSeparator() + "Backup";

    if (!wxDirExists(newDirBackup) && !newDirH.Mkdir(newDirBackup)) {
        DisplayError(wxString::Format("Unable to create backup directory '%s'!", newDirBackup).ToStdString());
        return;
    }

    wxString newDir = wxString::Format("%s%c%s-%s",
        newDirBackup, wxFileName::GetPathSeparator(),
        curTime.FormatISODate(), curTime.Format("%H%M%S"));
    if (startup) {
        newDir += "_OnStart";
    }

    int tries = 0;
    while (wxDirExists(newDir) && tries < 11) {
        logger_base.warn("Backup directory '%s' already existed ... trying again", (const char*)newDir.c_str());

        newDir = wxString::Format("%s%c%s-%s",
            newDirBackup, wxFileName::GetPathSeparator(),
            curTime.FormatISODate(), curTime.Format("%H%M%S")) + "_" + char(65 + tries);
        if (startup) {
            newDir += "_OnStart";
        }

        tries++;
    }

    if (tries == 11) {
        DisplayError("Unable to find a unique name for backup directory! Backup failed.");
        return;
    }

    if (prompt) {
        if (wxNO == wxMessageBox("All xml & xsq files under " + wxString::Format("%i", MAXBACKUPFILE_MB) + "MB in your xlights directory will be backed up to \"" +
            newDir + "\". Proceed?", "Backup", wxICON_QUESTION | wxYES_NO)) {
            return;
        }
    }

    if (!newDirH.Mkdir(newDir)) {
        DisplayError(wxString::Format("Unable to create directory '%s'! Backup failed.", newDir).ToStdString());
        return;
    }
    else {
        logger_base.info("Backup directory '%s' created", (const char*)newDir.c_str());
    }

    std::string errors = "";
    BackupDirectory(CurrentDir, newDir, newDir, forceallfiles, _backupSubfolders, errors);

    if (errors != "") {
        DisplayError(errors, this);
    }
}

void xLightsFrame::OnMenuItemBackupSelected(wxCommandEvent& event)
{
    SaveWorking();

    DoBackup(true);
}

void xLightsFrame::CreateMissingDirectories(wxString targetDirName, wxString lastCreatedDirectory, std::string& errors)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (wxDir::Exists(targetDirName)) return;
    if (!wxDir::Exists(lastCreatedDirectory)) return;

    if (targetDirName.Length() > 256) {
        logger_base.warn("Target directory %s is %d characters long. This may be an issue on your operating system.", (const char*)targetDirName.c_str(), targetDirName.Length());
    }

    wxFileName tgt(targetDirName);
    wxFileName lst(lastCreatedDirectory);

    if (!tgt.GetFullPath().StartsWith(lst.GetFullPath())) return;

    logger_base.debug("Create missing directories. Target %s. Last Created %s.", (const char*)tgt.GetFullPath().c_str(), (const char*)lst.GetFullPath().c_str());

    wxArrayString tgtd = wxSplit(targetDirName, wxFileName::GetPathSeparator());
    wxArrayString lstd = wxSplit(lastCreatedDirectory, wxFileName::GetPathSeparator());
    wxString newDir = lastCreatedDirectory;

    bool cont = true;
    for (size_t i = lstd.Count(); cont && i < tgtd.Count(); i++) {
        wxDir dir(newDir);
        newDir += wxFileName::GetPathSeparator() + tgtd[i];
        if (!wxDir::Exists(newDir)) {
            logger_base.debug("    Create folder '%s'.", (const char*)newDir.c_str());
            if (!dir.Make(newDir)) {
                cont = false;
                errors += wxString::Format("Failed to create folder %s\n", newDir);
                logger_base.error("        Folder Create failed.");
            }
        }
    }
}

bool xLightsFrame::CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, bool forceallfiles, std::string& errors)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = false;
    wxString srcDirName = srcDir.GetNameWithSep();
    wxFileName srcFile;
    srcFile.SetPath(srcDir.GetNameWithSep());

    wxArrayString files;
    GetAllFilesInDir(srcDir.GetNameWithSep(), files, wildcard);
    for (auto &file : files) {
        auto const fname = wxFileName(file).GetFullName();
        srcFile.SetFullName(file);
        if (FileExists(srcFile.GetFullPath())) { //checking if exists will force it to be downloaded if in the cloud
            logger_base.debug("Backing up file %s.", (const char*)(srcFile.GetFullPath()).c_str());
            res = true;

            CreateMissingDirectories(targetDirName, lastCreatedDirectory, errors);


            wxULongLong fsize = srcFile.GetSize();
            if (!forceallfiles && fsize > MAXBACKUPFILE_MB * 1024 * 1024) // skip any xml files > MAXBACKUPFILE_MB mbytes, they are something other than xml files
            {
                logger_base.warn("    Skipping file as it is too large.");
                continue;
            }

            logger_base.debug("    to %s.", (const char*)(targetDirName + wxFileName::GetPathSeparator() + fname).c_str());
            SetStatusText("Copying File \"" + srcFile.GetFullPath());
            bool success = wxCopyFile(srcFile.GetFullPath(),
                targetDirName + wxFileName::GetPathSeparator() + fname);
            if (!success) {
                logger_base.error("    Copy Failed.");
                errors += "Unable to copy file \"" + srcDir.GetNameWithSep() + fname + "\"\n";
            }
        }
    }

    return res;
}

void xLightsFrame::BackupDirectory(wxString sourceDir, wxString targetDirName, wxString lastCreatedDirectory, bool forceallfiles, bool backupSubfolders, std::string& errors)
{
    wxDir srcDir(sourceDir);

    if (!srcDir.IsOpened()) {
        return;
    }

    if (CopyFiles("*.xsq", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
        CopyFiles("*.xml", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
        CopyFiles("*.xbkp", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
        CopyFiles("*.xmap", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
        CopyFiles("*.xschedule", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) > 0) {
        lastCreatedDirectory = targetDirName;
    }

    // recurse through all directories but folders named Backup
    if (backupSubfolders) {
        wxString dir;
        // I dont think backup should follow symbolic links
        bool cont = srcDir.GetFirst(&dir, "", wxDIR_DIRS | wxDIR_NO_FOLLOW);
        while (cont) {
            if (dir.Lower() != "backup") {
                wxDir subdir(srcDir.GetNameWithSep() + dir);
                BackupDirectory(subdir.GetNameWithSep(), targetDirName + wxFileName::GetPathSeparator() + dir, lastCreatedDirectory, forceallfiles, backupSubfolders, errors);
            }
            cont = srcDir.GetNext(&dir);
        }
    }

    SetStatusText("All xml files backed up.");
}

bool xLightsFrame::isRandom_(wxControl* ctl, const char*debug)
{
    bool retval = (buttonState[std::string(ctl->GetName())] != Locked);
    return retval;
}

void xLightsFrame::OnEffectsPanel1Paint(wxPaintEvent& event) {}

void xLightsFrame::OnGrid1SetFocus(wxFocusEvent& event)
{
    Grid1HasFocus = true;
}

void xLightsFrame::OnGrid1KillFocus(wxFocusEvent& event)
{
    Grid1HasFocus = false;
}

void xLightsFrame::OntxtCtrlSparkleFreqText(wxCommandEvent& event) {}

static void AddNonDupAttr(wxXmlNode* node, const wxString& name, const wxString& value)
{
    wxString junk;
    if (node->GetAttribute(name, &junk)) node->DeleteAttribute(name); //kludge: avoid dups
    if (!value.empty()) node->AddAttribute(name, value);
}

ModelGroup* xLightsFrame::GetSelectedModelGroup() const
{
    if (Notebook1->GetSelection() != LAYOUTTAB) return nullptr;
    return layoutPanel->GetSelectedModelGroup();
}

void xLightsFrame::LoadJukebox(wxXmlNode* node)
{
    jukeboxPanel->Load(node);
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
        for (wxXmlNode* node = parent->GetChildren(); node != nullptr; node = node->GetNext())
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
    for (wxXmlNode* node = parent->GetChildren(); node != nullptr; node = node->GetNext())
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

void xLightsFrame::SetPreviewSize(int width,int height)
{
    SetXmlSetting("previewWidth", wxString::Format(wxT("%i"),width));
    SetXmlSetting("previewHeight", wxString::Format(wxT("%i"),height));
    modelPreview->SetCanvasSize(width, height);
    modelPreview->Refresh();
    _housePreviewPanel->GetModelPreview()->SetVirtualCanvasSize(width, height);
    _housePreviewPanel->Refresh();
}

void xLightsFrame::SetXmlSetting(const wxString& settingName, const wxString& value)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // Delete existing setting node
    if (SettingsNode != nullptr) {
        for (wxXmlNode* e = SettingsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
            if (e->GetName() == settingName) {
                SettingsNode->RemoveChild(e);
                delete e;
                break;
            }
        }

        // Add new one
        wxXmlNode* setting = new wxXmlNode(wxXML_ELEMENT_NODE, settingName);
        setting->AddAttribute("value", value);
        SettingsNode->AddChild(setting);
    }
    else {
        logger_base.warn("xLightsFrame::SetXmlSetting SettingsNode unexpectantly null.");
    }
}

wxString xLightsFrame::GetXmlSetting(const wxString& settingName, const wxString& defaultValue) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (SettingsNode != nullptr) {
        for (wxXmlNode* e = SettingsNode->GetChildren(); e != nullptr; e = e->GetNext()) {
            if (e->GetName() == settingName) {
                return e->GetAttribute("value", defaultValue);
            }
        }
    }
    else {
        logger_base.warn("xLightsFrame::GetXmlSetting SettingsNode unexpectantly null.");
    }

    return defaultValue;
}

void xLightsFrame::OnButtonClickSaveAs(wxCommandEvent& event)
{
    SaveAsSequence();
}

wxString xLightsFrame::GetSeqXmlFileName()
{
    if (CurrentSeqXmlFile == nullptr) return "";

    return CurrentSeqXmlFile->GetFullPath();
}

void xLightsFrame::ShowSequenceSettings()
{
    if (xLightsFrame::CurrentSeqXmlFile == nullptr)
        return;

    // abort any in progress render ... it may be using media or we may change the sequence length ... and that would be bad
    bool aborted = AbortRender();

    // populate dialog
    SeqSettingsDialog dialog(this, xLightsFrame::CurrentSeqXmlFile, mediaDirectories, wxEmptyString, wxEmptyString);
    dialog.Fit();
    int ret_code = dialog.ShowModal();

    if (ret_code == NEEDS_RENDER || aborted) {
        RenderAll();
    }

    if (ret_code != wxID_OK)
        return; // user pressed cancel

    if (CurrentSeqXmlFile->GetSequenceType() == "Animation") {
        mediaFilename = "";
        CurrentSeqXmlFile->ClearMediaFile();
        wxString error;
        GetMainSequencer()->PanelWaveForm->OpenfileMedia(nullptr, error);
    } else if (CurrentSeqXmlFile->GetMedia() != nullptr) {
        if (CurrentSeqXmlFile->GetMedia()->GetFrameInterval() < 0) {
            CurrentSeqXmlFile->GetMedia()->SetFrameInterval(CurrentSeqXmlFile->GetFrameMS());
        }
    }

    SetAudioControls();

    _sequenceElements.IncrementChangeCount(nullptr);
}

void xLightsFrame::OnMenu_Settings_SequenceSelected(wxCommandEvent& event)
{
    ShowSequenceSettings();
}

void xLightsFrame::OnAuiToolBarItemPlayButtonClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

Effect* xLightsFrame::GetPersistentEffectOnModelStartingAtTime(const std::string& model, uint32_t startms) const
{
    Element* e = _sequenceElements.GetElement(model);

    if (e == nullptr)
        return nullptr;

    for (size_t i = 0; i < e->GetEffectLayerCount(); ++i) {
        Effect* ef = e->GetEffectLayer(i)->GetEffectStartingAtTime(startms);
        if (ef != nullptr) {
            if (ef->IsPersistent()) {
                return ef;
            }
        }
    }
    return nullptr;
}

void xLightsFrame::EnableToolbarButton(wxAuiToolBar* toolbar, int id, bool enable)
{
    wxAuiToolBarItem* button = toolbar->FindTool(id);
    if (button != nullptr) {
        int state = enable ? wxAUI_BUTTON_STATE_NORMAL : wxAUI_BUTTON_STATE_DISABLED;
        button->SetState(state);
        toolbar->Refresh();
    }
}

void xLightsFrame::OnAuiToolBarItemPauseButtonClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemStopClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        //playStartTime = playEndTime = 0;
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarFirstFrameClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_FIRST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarLastFrameClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_LAST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event)
{
    //if (Notebook1->GetSelection() == NEWSEQUENCER)
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
    OpenSequence("", nullptr);
}

void xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected(wxCommandEvent& event)
{
    SaveAsSequence();
}

void xLightsFrame::AskCloseSequence()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Closing sequence.");
    CloseSequence();
    logger_base.info("Sequence closed.");

    // force refreshes since grid has been cleared
    mainSequencer->PanelTimeLine->RaiseChangeTimeline();
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::OnMenuItem_File_Close_SequenceSelected(wxCommandEvent& event)
{
    AskCloseSequence();
}

void xLightsFrame::OnMenuItem_File_Export_VideoSelected(wxCommandEvent& event)
{
    const char wildcard[] = "MP4 files (*.mp4)|*.mp4";
    wxFileDialog pExportDlg(this, _("Export House Preview Video"), wxEmptyString, CurrentSeqXmlFile->GetName(), wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int exportChoice = pExportDlg.ShowModal();

    if (exportChoice != wxID_OK) {
        return;
    }

    ExportVideoPreview(pExportDlg.GetPath());
}

bool xLightsFrame::ExportVideoPreview(wxString const& path)
{
    int frameCount = _seqData.NumFrames();

    if (CurrentSeqXmlFile == nullptr || frameCount == 0)
    {
        return false;
    }

    // Ensure all pending work is done before we do anything
    DoAllWork();

    wxAuiPaneInfo& pi = m_mgr->GetPane("HousePreview");
    bool visible = pi.IsShown();
    if (!visible) {
        pi.Show();
        m_mgr->Update();
    }

    ModelPreview *housePreview = _housePreviewPanel->GetModelPreview();
    if (housePreview == nullptr) {
        return false;
    }


    int playStatus = GetPlayStatus();
    SetPlayStatus(PLAY_TYPE_STOPPED);

    wxStopWatch sw;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Writing house-preview video to %s.", (const char *)path.c_str());

    int width = housePreview->getWidth();
    int height = housePreview->getHeight();
    double contentScaleFactor = housePreview->GetContentScaleFactor();
#ifdef _WIN32
    contentScaleFactor = 1.;
#endif // WIN32

    int audioChannelCount = 0;
    int audioSampleRate = 0;
    AudioManager *audioMgr = CurrentSeqXmlFile->GetMedia();
    if (audioMgr != nullptr)
    {
        audioSampleRate = audioMgr->GetRate();
        audioChannelCount = audioMgr->GetChannels();
    }
    int audioFrameIndex = 0;
    bool exportStatus = false;
    std::string emsg;
    try {
        VideoExporter videoExporter(this, width, height, contentScaleFactor, _seqData.FrameTime(), _seqData.NumFrames(),
            audioChannelCount, audioSampleRate, path, _videoExportCodec, _videoExportBitrate );

        auto audioLambda = [audioMgr, &audioFrameIndex](float* leftCh, float* rightCh, int frameSize) {
            int trackSize = audioMgr->GetTrackSize();
            int clampedSize = std::min(frameSize, trackSize - audioFrameIndex);
            if (clampedSize > 0) {
                const float* leftptr = audioMgr->GetRawLeftDataPtr(audioFrameIndex);
                const float* rightptr = audioMgr->GetRawRightDataPtr(audioFrameIndex);

                if (leftptr != nullptr) {
                    std::memcpy(leftCh, leftptr, clampedSize * sizeof(float));
                    std::memcpy(rightCh, rightptr, clampedSize * sizeof(float));
                    audioFrameIndex += frameSize;
                }
            }
            return true;
        };

        if (audioMgr != nullptr) {
            videoExporter.setGetAudioCallback(audioLambda);
        }
        auto videoLambda = [=](AVFrame *f, uint8_t* buf, int bufSize, unsigned frameIndex) {
            const SequenceData::FrameData& frameData(this->_seqData[frameIndex]);
            const uint8_t* data = frameData[0];
            housePreview->captureNextFrame(width*contentScaleFactor, height*contentScaleFactor);
            housePreview->Render(frameIndex * this->_seqData.FrameTime(), data, false);
            return housePreview->getFrameForExport(width*contentScaleFactor, height*contentScaleFactor, f, buf, bufSize);
        };
        videoExporter.setGetVideoCallback(videoLambda);

        exportStatus = videoExporter.Export(_appProgress.get());
    }
    catch (const std::runtime_error& re) {
        emsg = (const char*)re.what();
        logger_base.error("Error exporting video : %s", (const char*)re.what());
        exportStatus = false;
    }

    SetPlayStatus(playStatus);

    if (!visible) {
        m_mgr->GetPane( "HousePreview" ).Hide();
        m_mgr->Update();
    }

    if (exportStatus) {
        logger_base.debug( "Finished writing house-preview video." );
        float elapsedTime = sw.Time()/1000.0; //msec => sec
        SetStatusText(wxString::Format("'%s' exported in %4.3f sec.", path.c_str(), elapsedTime));
    } else {
        DisplayError( "Exporting house preview video failed.  " + emsg, this );
    }
    return exportStatus;
}

void xLightsFrame::OnResize(wxSizeEvent& event)
{
}

void xLightsFrame::OnAuiToolBarItemRenderAllClick(wxCommandEvent& event)
{
    RenderAll();
}

bool AUIToolbarButtonWrapper::IsChecked()
{
    return toolbar->GetToolToggled(id);
}

void AUIToolbarButtonWrapper::SetValue(bool b)
{
    toolbar->ToggleTool(id, b);
}

void AUIToolbarButtonWrapper::SetBitmap(const wxBitmapBundle &bmp)
{
    toolbar->SetToolBitmap(id,bmp);
    toolbar->Refresh();
}

void AUIToolbarButtonWrapper::Enable(bool b)
{
    toolbar->EnableTool(id, b);
}

void xLightsFrame::SetGridSpacing(int size)
{
    mGridSpacing = size;
    DEFAULT_ROW_HEADING_HEIGHT = size + 6;
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::ResetToolbarLocations(wxCommandEvent& event)
{
    DisplayInfo("Toolbar locations will reset to defaults upon restart.", this);
    mResetToolbars = true;
}

void xLightsFrame::SetToolIconSize(int size)
{
    mIconSize = size;
    size = ScaleWithSystemDPI(size);
    for (size_t x = 0; x < EffectsToolBar->GetToolCount(); x++)
    {
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetSizeHints(size, size, size, size);
        EffectsToolBar->FindToolByIndex(x)->SetMinSize(EffectsToolBar->FindToolByIndex(x)->GetWindow()->GetMinSize());
    }
    EffectsToolBar->Realize();
    wxSize sz = EffectsToolBar->GetSize();
    wxAuiPaneInfo &info = MainAuiManager->GetPane("EffectsToolBar");
    info.BestSize(sz);
    MainAuiManager->Update();

    const wxWindowList& lst =effectPalettePanel->GetChildren();
    for (size_t x = 0; x < lst.size(); x++)
    {
        lst[x]->SetSizeHints(size, size, size, size);
    }
    effectPalettePanel->Layout();
    effectsPnl->BitmapButtonSelectedEffect->SetSizeHints(size, size, size, size);
    effectsPnl->Layout();
}

void xLightsFrame::SetFrequency(int frequency)
{
    _sequenceElements.SetFrequency(frequency);
    mainSequencer->PanelTimeLine->SetTimeFrequency(frequency);
    mainSequencer->PanelWaveForm->SetTimeFrequency(frequency);
}

void xLightsFrame::SetGridIconBackgrounds(bool b)
{
    mGridIconBackgrounds = b;
    mainSequencer->PanelEffectGrid->SetEffectIconBackground(mGridIconBackgrounds);
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::SetGridNodeValues(bool b)
{
    mGridNodeValues = b;
    mainSequencer->PanelEffectGrid->SetEffectNodeValues(mGridNodeValues);
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::SetPlaySpeedTo(float speed)
{
    playAnimation = false;
    playSpeed = speed;

    AudioManager::SetPlaybackRate(playSpeed);
    if (CurrentSeqXmlFile != nullptr) {
        if (CurrentSeqXmlFile->GetMedia() == nullptr) {
            playAnimation = true;
        }
    }

    if (speed == 1.0) {
        AudioMenu->Check(ID_PLAY_FULL, true);
    }
    else if (speed == 1.5) {
        AudioMenu->Check(ID_MNU_1POINT5SPEED, true);
    }
    else if (speed == 2) {
        AudioMenu->Check(ID_MN_2SPEED, true);
    }
    else if (speed == 3) {
        AudioMenu->Check(ID_MNU_3SPEED, true);
    }
    else if (speed == 4) {
        AudioMenu->Check(ID_MNU_4SPEED, true);
    }
    else if (speed == 0.75) {
        AudioMenu->Check(ID_PLAY_3_4, true);
    }
    else if (speed == 0.5) {
        AudioMenu->Check(ID_PLAY_1_2, true);
    }
    else if (speed == 0.25) {
        AudioMenu->Check(ID_PLAY_1_4, true);
    }
}

void xLightsFrame::SetPlaySpeed(wxCommandEvent& event)
{
    if (event.GetId() == ID_PLAY_FULL)
    {
        SetPlaySpeedTo(1.0);
    }
    else if (event.GetId() == ID_PLAY_3_4)
    {
        SetPlaySpeedTo(0.75);
    }
    else if (event.GetId() == ID_PLAY_1_2)
    {
        SetPlaySpeedTo(0.5);
    }
    else if (event.GetId() == ID_PLAY_1_4)
    {
        SetPlaySpeedTo(0.25);
    }
    else if (event.GetId() == ID_MNU_1POINT5SPEED)
    {
        SetPlaySpeedTo(1.5);
    }
    else if (event.GetId() == ID_MN_2SPEED)
    {
        SetPlaySpeedTo(2.0);
    }
    else if (event.GetId() == ID_MNU_3SPEED)
    {
        SetPlaySpeedTo(3.0);
    }
    else if (event.GetId() == ID_MNU_4SPEED)
    {
        SetPlaySpeedTo(4.0);
    }
    else         {
        SetPlaySpeedTo(1.0);
    }
}

void xLightsFrame::SetRenderOnSave(bool b)
{
    mRenderOnSave = b;
}

void xLightsFrame::SetSaveFseqOnSave(bool b)
{
    mSaveFseqOnSave = b;
    if (!mSaveFseqOnSave) {
        mRenderOnSave = false;
        DisplayWarning("Turning off save of the FSEQ is really not recommended. This will often require you to re-render a sequence every time you load it ... all to save yourself a couple of seconds save time.", this);
    }
}

void xLightsFrame::SetEffectAssistMode(int i)
{
    mEffectAssistMode = i;
    if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    } else if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_OFF) {
        SetEffectAssistWindowState(false);
    }
}

void xLightsFrame::SetEffectAssistWindowState(bool show)
{
    bool visible = m_mgr->GetPane("EffectAssist").IsShown();
    if (visible && !show)
    {
        m_mgr->GetPane("EffectAssist").Hide();
        m_mgr->Update();
    }
    else if(!visible && show)
    {
        m_mgr->GetPane("EffectAssist").Show();
        m_mgr->Update();
    }
    UpdateViewMenu();
}

void xLightsFrame::UpdateEffectAssistWindow(Effect* effect, RenderableEffect* ren_effect)
{
    if( effect == nullptr || ren_effect == nullptr )
    {
        sEffectAssist->SetPanel(nullptr);
        return;
    }

    bool effect_is_supported = ren_effect->HasAssistPanel();

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

    AssistPanel *panel;
    if (effect_is_supported) {
        panel = ren_effect->GetAssistPanel(sEffectAssist, this);
    } else {
        panel = sEffectAssist->GetDefaultAssistPanel();
    }
    panel->SetEffectInfo(effect, this);
    sEffectAssist->SetPanel(panel);
}

void xLightsFrame::CheckUnsavedChanges()
{
    if (UnsavedRgbEffectsChanges)
    {
        // This is not necessary but it shows the user that the save button is red which I am hoping makes it clearer
        // to the user what this prompt is for
        Notebook1->SetSelection(LAYOUTTAB);

        if (wxYES == wxMessageBox("Save Models, Views, Perspectives, and Preset changes?",
            "RGB Effects File Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT))
        {
            SaveEffectsFile();
        }
        else
        {
            wxFileName effectsFile;
            effectsFile.AssignDir(CurrentDir);
            effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
            wxFileName fn(effectsFile.GetFullPath());
            if (FileExists(fn.GetFullPath()))
            {
                fn.Touch();
            }
        }
    }

    if (UnsavedNetworkChanges)
    {
        // This is not necessary but it shows the user that the save button is red which I am hoping makes it clearer
        // to the user what this prompt is for
        Notebook1->SetSelection(SETUPTAB);

        if (wxYES == wxMessageBox("Save Network Setup changes?",
            "Networks Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT))
        {
            SaveNetworksFile();
        }
    }
}

void xLightsFrame::MarkEffectsFileDirty()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        MarkEffectsFileDirty.");

    layoutPanel->SetDirtyHiLight(true);
    UnsavedRgbEffectsChanges=true;
}

void xLightsFrame::MarkModelsAsNeedingRender()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        MarkModelsAsNeedingRender %d.", modelsChangeCount);
    modelsChangeCount++;
}

uint32_t xLightsFrame::GetMaxNumChannels() {
    return std::max((uint32_t)_outputManager.GetTotalChannels(), (uint32_t)(AllModels.GetLastChannel() + 1));
}

void xLightsFrame::UpdateSequenceLength()
{
    if( CurrentSeqXmlFile->GetSequenceLoaded() )
    {
        wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
        int ms = wxAtoi(mss);

        AbortRender();
        _seqData.init(GetMaxNumChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        _sequenceElements.IncrementChangeCount(nullptr);

        mainSequencer->PanelTimeLine->SetTimeLength(CurrentSeqXmlFile->GetSequenceDurationMS());
        mainSequencer->PanelTimeLine->Initialize();
        int maxZoom = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
        mainSequencer->PanelTimeLine->SetZoomLevel(maxZoom);
        mainSequencer->PanelWaveForm->SetZoomLevel(maxZoom);
        mainSequencer->PanelTimeLine->RaiseChangeTimeline();
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
    }
}

void xLightsFrame::OnActionTestMenuItemSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	// save the media playing state and stop it if it is playing
	MEDIAPLAYINGSTATE mps = MEDIAPLAYINGSTATE::STOPPED;
	if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr) {
		mps = CurrentSeqXmlFile->GetMedia()->GetPlayingState();
		if (mps == MEDIAPLAYINGSTATE::PLAYING) {
            logger_base.debug("Test: Suspending play.");
			CurrentSeqXmlFile->GetMedia()->Pause();
			SetAudioControls();
		}
	}

    bool timerRunning = OutputTimer.IsRunning();
    if (timerRunning) {
        OutputTimer.Stop();
    }

	// save the output state and turn it off
	bool output = CheckBoxLightOutput->IsChecked();
	if (output) {
        logger_base.debug("Test: Turning off output to lights.");
        DisableOutputs();
	}

	// creating the dialog can take some time so display an hourglass
	SetCursor(wxCURSOR_WAIT);

    // Make sure all the models in model groups are valid
    AllModels.ResetModelGroups();

    logger_base.debug("Test: Opening test dialog.");

	// display the test dialog
    PixelTestDialog dialog(this, &_outputManager, networkFile, &AllModels);
    dialog.ShowModal();

    logger_base.debug("Test: Test dialog closed.");

	SetCursor(wxCURSOR_DEFAULT);

	// resume output if it was set
	if (output) {
        logger_base.debug("Test: Turning back on output to lights.");
        EnableOutputs();
	}

    // Restart the timer without changing the interval
    if (timerRunning) {
        OutputTimer.Start();
    }

	// resume playing the media if it was playing
	if (mps == MEDIAPLAYINGSTATE::PLAYING) {
        logger_base.debug("Test: Resuming play.");
        CurrentSeqXmlFile->GetMedia()->Play();
		SetAudioControls();
	}
}

void xLightsFrame::SetPasteByCell()
{
    ButtonPasteByTime->SetValue(false);
    ButtonPasteByCell->SetValue(true);
    mainSequencer->SetPasteByCell(true);
    m_mgr->Update();
    EditToolBar->Refresh();
}

void xLightsFrame::SetPasteByTime()
{
    ButtonPasteByTime->SetValue(true);
    ButtonPasteByCell->SetValue(false);
    mainSequencer->SetPasteByCell(false);
    m_mgr->Update();
    EditToolBar->Refresh();
}

void xLightsFrame::OnAuiToolBarItemPasteByTimeClick(wxCommandEvent& event)
{
    SetPasteByTime();
}

void xLightsFrame::OnAuiToolBarItemPasteByCellClick(wxCommandEvent& event)
{
    SetPasteByCell();
}

void xLightsFrame::OnMenuItemConvertSelected(wxCommandEvent& event)
{
    UpdateChannelNames();
    ConvertDialog dialog(this, _seqData, &_outputManager, mediaFilename, ChannelNames, ChannelColors, ChNames);
    dialog.CenterOnParent();
    dialog.ShowModal();
}

void xLightsFrame::OnMenu_GenerateCustomModelSelected(wxCommandEvent& event)
{
    // save the media playing state and stop it if it is playing
    MEDIAPLAYINGSTATE mps = MEDIAPLAYINGSTATE::STOPPED;
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr)
    {
        mps = CurrentSeqXmlFile->GetMedia()->GetPlayingState();
        if (mps == MEDIAPLAYINGSTATE::PLAYING)
        {
            CurrentSeqXmlFile->GetMedia()->Pause();
            SetAudioControls();
        }
    }

    bool timerRunning = OutputTimer.IsRunning();
    if (timerRunning) {
        OutputTimer.Stop();
    }

    // save the output state and turn it off
    bool output = CheckBoxLightOutput->IsChecked();
    if (output) {
        DisableOutputs();
    }

    // creating the dialog can take some time so display an hourglass
    SetCursor(wxCURSOR_WAIT);

    GenerateCustomModelDialog dialog(this, &_outputManager);
    dialog.CenterOnParent();
    dialog.ShowModal();

    SetCursor(wxCURSOR_DEFAULT);

    // resume output if it was set
    if (output) {
        EnableOutputs();
    }

    // restarts the timer without changing the interval
    if (timerRunning) {
        OutputTimer.Start();
    }

    // resume playing the media if it was playing
    if (mps == MEDIAPLAYINGSTATE::PLAYING)
    {
        CurrentSeqXmlFile->GetMedia()->Play();
        SetAudioControls();
    }
}

void xLightsFrame::OnPaneClose(wxAuiManagerEvent& event)
{
    SetFocus();
    if (event.pane != nullptr) event.pane->Hide();
    UpdateViewMenu();
}

void xLightsFrame::MaybePackageAndSendDebugFiles() {
    wxString message = "You forced the OpenGL setting to a non-default value.  Is it OK to send the debug logs to the developers for analysis?";
    wxMessageDialog dlg(this, message, "Send Debug Files",wxYES_NO|wxCENTRE);
    if (dlg.ShowModal() == wxID_YES) {
        wxTextEntryDialog ted(this, "Can you briefly describe what wasn't working?\n"
                              "Also include who you are (email, forum username, etc..)", "Additional Information", "",
                              wxTE_MULTILINE|wxOK|wxCENTER);
        ted.SetSize(400, 400);
        ted.ShowModal();

        wxDebugReportCompress report;
        report.SetCompressedFileBaseName("xlights_debug");
        report.SetCompressedFileDirectory(wxFileName::GetTempDir());
        report.AddText("description", ted.GetValue(), "description");
        AddDebugFilesToReport(report);
        report.Process();
        xlCrashHandler::SendReport("xLights", "oglUpload", report);
        wxRemoveFile(report.GetCompressedFileName());
    }
}

void xLightsFrame::CreateDebugReport(xlCrashHandler* crashHandler)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxDebugReportCompress* const report = &crashHandler->GetDebugReport();

    report->SetCompressedFileDirectory(CurrentDir);

    wxFileName fn(CurrentDir, OutputManager::GetNetworksFileName());
    if (FileExists(fn))
    {
        report->AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
    }
    if (FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xml")))
    {
        report->AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    }
    if (UnsavedRgbEffectsChanges &&  FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp")))
    {
        report->AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp").GetFullPath(), "xlights_rgbeffects.xbkp");
    }

    if (GetSeqXmlFileName() != "")
    {
        wxFileName fn2(GetSeqXmlFileName());
        if (FileExists(fn2) && !fn2.IsDir())
        {
            report->AddFile(GetSeqXmlFileName(), fn2.GetName());
            wxFileName fnb(fn2.GetPath() + "/" + fn2.GetName() + ".xbkp");
            if (FileExists(fnb))
            {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
        else
        {
            wxFileName fnb(CurrentDir + "/" + "__.xbkp");
            if (FileExists(fnb))
            {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
    }
    else
    {
        wxFileName fnb(CurrentDir + "/" + "__.xbkp");
        if (FileExists(fnb))
        {
            report->AddFile(fnb.GetFullPath(), fnb.GetName());
        }
    }

    std::string threadStatus = "User Email: " + _userEmail.ToStdString() + "\n";

    threadStatus += "\n";
    threadStatus += "Render Pool:\n";
    threadStatus += GetThreadStatusReport();

    threadStatus += "\n";
    threadStatus += "Parallel Job Pool:\n";
    threadStatus += ParallelJobPool::POOL.GetThreadStatus();

    threadStatus += "\n";
    threadStatus += "Thread traces:\n";
    std::list<std::string> traceMessages;
    TraceLog::GetTraceMessages(traceMessages);
    for (auto &a : traceMessages)
    {
        threadStatus += a;
        threadStatus += "\n";
    }

    report->AddText("threads.txt", threadStatus, "Threads Status");
    logger_base.crit("%s", (const char *)threadStatus.c_str());

    crashHandler->ProcessCrashReport(xlCrashHandler::SendReportOptions::ASK_USER_TO_SEND);
}

void xLightsFrame::OnMenuItemPackageDebugFiles(wxCommandEvent& event)
{
    PackageDebugFiles();
}

std::string xLightsFrame::PackageDebugFiles(bool showDialog)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString zipFileName{ "xLightsProblem.zip" };
    wxString zipDir{ CurrentDir };

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    if (showDialog) {
        wxFileDialog fd(this, "Zip file to create.", zipDir, zipFileName, "zip file(*.zip)|*.zip", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (fd.ShowModal() == wxID_CANCEL) {
            return "";
        }
        zipFileName = fd.GetFilename();
        zipDir = fd.GetDirectory();
    }

    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB) {
        layoutPanel->UnSelectAllModels();
    }
    RecalcModels();

    // check the current sequence to ensure this analysis is in the log
    CheckSequence(false, false);

    logger_base.debug("Dumping registry configuration:");
    wxConfigBase* config = wxConfigBase::Get();
    wxString key;
    long index;
    bool ce = config->GetFirstEntry(key, index);

    while (ce) {
        wxString value = "<UNKNOWN>";
        wxString type = "<UNREAD>";
        switch (config->GetEntryType(key)) {
        case wxConfigBase::EntryType::Type_String:
            type = "String";
            config->Read(key, &value);
            break;
        case wxConfigBase::EntryType::Type_Boolean:
            type = "Boolean";
            value = wxString::Format("%s", config->ReadBool(key, false) ? "True" : "False");
            break;
        case wxConfigBase::EntryType::Type_Integer: // long
            type = "Integer";
            value = wxString::Format("%ld", config->ReadLong(key, 0));
            break;
        case wxConfigBase::EntryType::Type_Float: // double
            type = "Float";
            value = wxString::Format("%f", config->ReadDouble(key, 0.0));
            break;
        case wxConfigBase::EntryType::Type_Unknown:
            type = "Unknown";
            break;
        default:
            break;
        }

        logger_base.debug("      '%s' (%s) ='%s'", (const char*)key.c_str(), (const char*)type.c_str(), (const char*)value.c_str());

        ce = config->GetNextEntry(key, index);
    }

    wxDebugReportCompress report;
    report.SetCompressedFileBaseName(wxFileName(zipFileName).GetName());
    report.SetCompressedFileDirectory(zipDir);
    AddDebugFilesToReport(report);

    // export the models to an easy to read file
    wxString filename = wxFileName::CreateTempFileName("Models") + ".xlsx";
    ExportModels(filename);
    wxFileName fn(filename);
    report.AddFile(fn.GetFullPath(), "All Models");

    // export the effects to an easy to read file
    if (CurrentSeqXmlFile != nullptr) {
        wxString filenamee = wxFileName::CreateTempFileName("Effects") + ".csv";
        ExportEffects(filenamee);
        wxFileName fne(filenamee);
        report.AddFile(fne.GetFullPath(), "All Effects");
    }

    report.Process();

    wxRemoveFile(filename);

    return zipDir + wxFileName::GetPathSeparator() + zipFileName;
}

static void AddLogFile(const wxString& CurrentDir, const wxString& fileName, wxDebugReport& report)
{
    wxString dir;
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    wxString filename = dir + "/" + fileName;
#endif
#ifdef __WXOSX__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    wxString filename = dir + "/Library/Logs/" + fileName;
#endif
#ifdef __LINUX__
    wxString filename = "/tmp/" + fileName;
#endif
    if (FileExists(filename)) {
        report.AddFile(filename, fileName);
    }
    else if (FileExists(wxFileName(CurrentDir, fileName).GetFullPath())) {
        report.AddFile(wxFileName(CurrentDir, fileName).GetFullPath(), fileName);
    }
    else if (FileExists(wxFileName(wxGetCwd(), fileName).GetFullPath())) {
        report.AddFile(wxFileName(wxGetCwd(), fileName).GetFullPath(), fileName);
    }
}

void xLightsFrame::AddDebugFilesToReport(wxDebugReport& report)
{

    wxFileName fn(CurrentDir, OutputManager::GetNetworksFileName());
    if (FileExists(fn)) {
        report.AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
    }
    if (FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xml"))) {
        report.AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    }
    if (UnsavedRgbEffectsChanges && FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp"))) {
        report.AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp").GetFullPath(), "xlights_rgbeffects.xbkp");
    }

    AddLogFile(CurrentDir, "xLights_l4cpp.log", report);
    //if the rolled log exists, add it to just in case it has the information we need
    AddLogFile(CurrentDir, "xLights_l4cpp.log.1", report);

    if (GetSeqXmlFileName() != "") {
        wxFileName fn2(GetSeqXmlFileName());
        if (FileExists(fn2) && !fn2.IsDir()) {
            report.AddFile(GetSeqXmlFileName(), fn2.GetName());
            if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
                wxFileName fnb(fn2.GetPath() + "/" + fn2.GetName() + ".xbkp");
                if (FileExists(fnb)) {
                    report.AddFile(fnb.GetFullPath(), fnb.GetName());
                }
            }
        }
        else {
            if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
                wxFileName fnb(CurrentDir + "/" + "__.xbkp");
                if (FileExists(fnb)) {
                    report.AddFile(fnb.GetFullPath(), fnb.GetName());
                }
            }
        }
    }
    else {
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            wxFileName fnb(CurrentDir + "/" + "__.xbkp");
            if (FileExists(fnb)) {
                report.AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
    }
    //report.AddAll(wxDebugReport::Context_Current);
}

int xLightsFrame::OpenGLVersion() const {
    int orig;
    wxConfigBase* config = wxConfigBase::Get();
    config->Read("ForceOpenGLVer", &orig, 99);
    return orig;
}
void xLightsFrame::SetOpenGLVersion(int i) {
    if (i == 0) {
        //auto detect
        i = 99;
    }
    if (i == 2) {
        //opengl 2 is now handled by opengl 1.x
        i = 1;
    }
    int orig;
    wxConfigBase* config = wxConfigBase::Get();
    config->Read("ForceOpenGLVer", &orig, 99);

    config->Write("ForceOpenGLVer", i);
    if (i != orig) {
        DisplayInfo("OpenGL changes require a restart", this);
    }
}

int xLightsFrame::OpenGLRenderOrder() const {
    wxConfigBase* config = wxConfigBase::Get();
    int orig;
    config->Read("OGLRenderOrder", &orig, 0);
    return orig;
}

void xLightsFrame::SetOpenGLRenderOrder(int order) {
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("OGLRenderOrder", order);
    _housePreviewPanel->GetModelPreview()->SetRenderOrder(order);
    _modelPreviewPanel->SetRenderOrder(order);
    modelPreview->SetRenderOrder(order);
}

void xLightsFrame::SaveWorkingLayout()
{
    // update xml with offsets and scale
    for (const auto& it: modelPreview->GetModels()) {
        if (AllModels.IsModelValid(it) || IsNewModel(it)) { // this IsModelValid should not be necessary but we are getting crashes due to invalid models - looks like we are missing a LayoutPanel::UpdateModelList call in some situation
            it->UpdateXmlWithScale();
        }
        else {
            wxASSERT(false); // why did we get here
        }
    }
    for (const auto& it : AllObjects) {
        ViewObject* view_object = it.second;
        view_object->UpdateXmlWithScale();
    }
    SaveEffectsFile(true);
}

void xLightsFrame::SaveWorking()
{
    // dont save if no file in existence
    if (CurrentSeqXmlFile == nullptr)
        return;

    // dont save if batch rendering
    if (_renderMode || _checkSequenceMode)
        return;

    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock())
        return;

    wxString p = CurrentSeqXmlFile->GetPath();
    wxString fn = CurrentSeqXmlFile->GetFullName();
    wxString tmp;

    wxFileName fnp(fn);
    if (fn == "") {
        tmp = p + "/" + "__.xbkp";
    } else if (CountChar(fnp.GetName(), '_') == fnp.GetName().size()) {
        tmp = p + "/" + fnp.GetName() + "_.xbkp";
    }
    else {
        tmp = p + "/" + fnp.GetName() + ".xbkp";
    }
    wxFileName ftmp(tmp);

    CurrentSeqXmlFile->SetPath(ftmp.GetPath());
    CurrentSeqXmlFile->SetFullName(ftmp.GetFullName());

    CurrentSeqXmlFile->Save(_sequenceElements);

    CurrentSeqXmlFile->SetPath(p);
    CurrentSeqXmlFile->SetFullName(fn);
}

void xLightsFrame::OnTimer_AutoSaveTrigger(wxTimerEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // dont save if currently playing or in render mode
    if (playType != PLAY_TYPE_MODEL && !_renderMode && !_checkSequenceMode && !_suspendAutoSave) {
        logger_base.debug("Autosaving backup of sequence.");
        wxStopWatch sw;
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (_sequenceElements.GetChangeCount() != mLastAutosaveCount) {
                SaveWorking();
                mLastAutosaveCount = _sequenceElements.GetChangeCount();
            } else {
                logger_base.debug("    Autosave skipped ... no changes detected since last autosave.");
            }
        } else {
            logger_base.debug("    Autosave skipped ... no changes detected since last save.");
            mLastAutosaveCount = _sequenceElements.GetChangeCount();
        }
        if (UnsavedRgbEffectsChanges) {
            logger_base.debug("    Autosaving backup of layout.");
            SaveWorkingLayout();
        }
        logger_base.debug("    AutoSave took %d ms.", sw.Time());

        if (mAutoSaveInterval > 0) {
            AutoSaveTimer.StartOnce(mAutoSaveInterval * 60000);
        }
    } else {
        logger_base.debug("AutoSave skipped because sequence is playing or batch rendering or suspended.");
        if (mAutoSaveInterval > 0) {
            AutoSaveTimer.StartOnce(10000); // try again in a short period of time as we did not actually save this time
        }
    }
}

void xLightsFrame::SetAutoSaveInterval(int nasi)
{
    if (nasi != mAutoSaveInterval) {
        wxConfigBase* config = wxConfigBase::Get();
        config->Write("AutoSaveInterval", nasi);
        mAutoSaveInterval = nasi;
    }
    if (mAutoSaveInterval > 0) {
        AutoSaveTimer.StartOnce(mAutoSaveInterval * 60000);
    }
    else {
        AutoSaveTimer.Stop();
    }
}

void xLightsFrame::AddPreviewOption(LayoutGroup* grp)
{
    bool menu_created = false;
    if( MenuItemPreviews == nullptr ) {
        MenuItemPreviews = new wxMenu();
        MenuView->Insert(3, ID_MENU_ITEM_PREVIEWS, _("Previews"), MenuItemPreviews, wxEmptyString);
        menu_created = true;
    }
    if( LayoutGroups.size() > 1 ) {
        wxMenuItem* first_item = MenuItemPreviews->GetMenuItems().GetFirst()->GetData();
        if( first_item->GetId() != ID_MENU_ITEM_PREVIEWS_SHOW_ALL ) {
            wxMenuItem* menu_item = new wxMenuItem(MenuItemPreviews, ID_MENU_ITEM_PREVIEWS_SHOW_ALL, _("Show All"), wxEmptyString, wxITEM_CHECK);
            MenuItemPreviews->Insert(0, menu_item);
            Connect(ID_MENU_ITEM_PREVIEWS_SHOW_ALL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideAllPreviewWindows, nullptr, this);
        }
    }
    grp->AddToPreviewMenu(MenuItemPreviews);
    Connect(grp->GetMenuId(),wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHidePreviewWindow, nullptr, this);
    if( menu_created ) {
        MenuItemPreviewSeparator = MenuView->InsertSeparator(4);
    }
}

void xLightsFrame::RemovePreviewOption(LayoutGroup* grp)
{
    Disconnect(grp->GetMenuId(),wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHidePreviewWindow, nullptr, this);
    grp->RemoveFromPreviewMenu(MenuItemPreviews);
    if( LayoutGroups.size() == 1 ) {
        Disconnect(ID_MENU_ITEM_PREVIEWS_SHOW_ALL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::ShowHideAllPreviewWindows, nullptr, this);
        MenuView->Delete(ID_MENU_ITEM_PREVIEWS);
        MenuItemPreviews = nullptr;
        MenuView->Delete(MenuItemPreviewSeparator);
    }
}

void xLightsFrame::ShowHidePreviewWindow(wxCommandEvent& event)
{
    wxMenuItem* item = MenuItemPreviews->FindItem(event.GetId());
    for (const auto& it : LayoutGroups) {
        if (it != nullptr) {
            if( it->GetMenuItem() == item ) {
                it->ShowPreview(item->IsChecked());
            }
        }
    }
}

void xLightsFrame::ShowHideAllPreviewWindows(wxCommandEvent& event)
{
    wxMenuItem* first_item = MenuItemPreviews->GetMenuItems().GetFirst()->GetData();
    for (const auto& it : LayoutGroups) {
        if (it != nullptr) it->ShowPreview(first_item->IsChecked());
    }
}

void xLightsFrame::DoAltBackup(bool prompt)
{

    if (mAltBackupDir == "" || !wxDirExists(mAltBackupDir)) {
        DisplayError(wxString::Format("Alternate backup directory location not defined or does not exist: '%s'", mAltBackupDir).ToStdString());
        return;
    }

    wxString folderName;
    time_t cur;
    time(&cur);
    wxFileName newDirH;
    wxDateTime curTime(cur);

    //  first make sure there is a Backup sub directory
    wxString newDirBackup = mAltBackupDir + wxFileName::GetPathSeparator() + "Backup";
    if (!wxDirExists(newDirBackup) && !newDirH.Mkdir(newDirBackup)) {
        DisplayError(wxString::Format("Unable to create backup directory '%s'!", newDirBackup).ToStdString());
        return;
    }

    wxString newDir = mAltBackupDir + wxFileName::GetPathSeparator() + wxString::Format(
        "Backup%c%s-%s", wxFileName::GetPathSeparator(),
        curTime.FormatISODate(), curTime.Format("%H%M%S"));

    if (prompt) {
        if (wxNO == wxMessageBox("All xml & xsq files under " + wxString::Format("%i", MAXBACKUPFILE_MB) + "MB in your xlights directory will be backed up to \"" +
            newDir + "\". Proceed?", "Backup", wxICON_QUESTION | wxYES_NO)) {
            return;
        }
    }

    if (!newDirH.Mkdir(newDir)) {
        DisplayError(wxString::Format("Unable to create directory '%s'!", newDir).ToStdString());
        return;
    }

    std::string errors;
    BackupDirectory(CurrentDir, newDir, newDir, false, _backupSubfolders, errors);

    if (errors != "") {
        DisplayError(errors, this);
    }
}

void xLightsFrame::SetMediaFolders(const std::list<std::string>& folders)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();

    wxString setting;
    mediaDirectories.clear();
    for (auto const& dir : folders) {
        ObtainAccessToURL(dir);
        if (std::find(mediaDirectories.begin(), mediaDirectories.end(), dir) == mediaDirectories.end()) {
            mediaDirectories.push_back(dir);
            logger_base.debug("Adding Media directory: %s.", (const char*)dir.c_str());
            if (setting != "") {
                setting += "|";
            }
            setting += dir;
        }        
    }
    config->Write(_("MediaDir"), setting);
    SetFixFileDirectories(mediaDirectories);
    mediaDirectories.push_back(showDirectory);
}

void xLightsFrame::GetFSEQFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == fseqDirectory);
    folder = fseqDirectory;
}

void xLightsFrame::SetFSEQFolder(bool useShow, const std::string& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxConfigBase* config = wxConfigBase::Get();

    if (useShow) {
        config->Write("FSEQLinkFlag", true);
        if (fseqDirectory == showDirectory) return;
        fseqDirectory = showDirectory;
    }
    else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            config->Write("FSEQLinkFlag", false);
            if (fseqDirectory == folder) return;
            fseqDirectory = folder;
        }
        else {
            DisplayError("FSEQ directory does not exist. FSEQ folder was not changed to " + folder + ". FSEQ folder remains : " + fseqDirectory, this);
            return;
        }
    }

    SetXmlSetting("fseqDir", fseqDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    logger_base.debug("FSEQ directory set to : %s.", (const char*)fseqDirectory.c_str());
}

void xLightsFrame::GetRenderCacheFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == renderCacheDirectory);
    folder = renderCacheDirectory;
}

void xLightsFrame::SetRenderCacheFolder(bool useShow, const std::string& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (useShow) {
        if (renderCacheDirectory == showDirectory) return;
        renderCacheDirectory = showDirectory;
    }
    else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            if (renderCacheDirectory == folder) return;
            renderCacheDirectory = folder;
        }
        else {
            DisplayError("Render Cache directory does not exist. Render Cache folder was not changed to " + folder + ". Render Cache folder remains : " + renderCacheDirectory, this);
            return;
        }
    }

    SetXmlSetting("renderCacheDir", renderCacheDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    logger_base.debug("Render Cache directory set to : %s.", (const char*)renderCacheDirectory.c_str());
}

void xLightsFrame::GetBackupFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == _backupDirectory);
    folder = _backupDirectory;
}

void xLightsFrame::SetBackupFolder(bool useShow, const std::string& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (useShow) {
        if (_backupDirectory == showDirectory) return;
        _backupDirectory = showDirectory;
    }
    else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            if (_backupDirectory == folder) return;
            _backupDirectory = folder;
        }
        else {
            DisplayError("Backup directory does not exist. Backup folder was not changed to " + folder + ". Backup folder remains : " + _backupDirectory, this);
            return;
        }
    }

    SetXmlSetting("backupDir", _backupDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    logger_base.debug("Backup directory set to : %s.", (const char*)_backupDirectory.c_str());
}

void xLightsFrame::GetAltBackupFolder(std::string& folder)
{
    folder = mAltBackupDir;
}

void xLightsFrame::SetAltBackupFolder(const std::string& folder)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (folder == mAltBackupDir) return;

    wxConfigBase* config = wxConfigBase::Get();

    if (folder != "" && !wxDir::Exists(folder)) {
        DisplayError("Alternate backup directory does not exist. Alternate backup folder was not changed to " + folder + ". Alternate backup folder remains : " + mAltBackupDir, this);
    }
    else {
        ObtainAccessToURL(folder);
        config->Write(_("xLightsAltBackupDir"), wxString(folder));
        mAltBackupDir = folder;
        logger_base.debug("Alt Backup directory set to : %s.", (const char*)mAltBackupDir.c_str());
    }
}

void xLightsFrame::OnmAltBackupMenuItemSelected(wxCommandEvent& event)
{
    if (mAltBackupDir == "") {
        wxDirDialog dir(this, _("Select alternate backup directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
        if (dir.ShowModal() == wxID_OK) {
            mAltBackupDir = dir.GetPath();
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.info("Alternate backup location set to %s.", (const char*)mAltBackupDir.c_str());
        }
    }

    if (mAltBackupDir == "") {
        return;
    }
    ObtainAccessToURL(mAltBackupDir);
    SaveWorking();

    DoAltBackup();
}

void xLightsFrame::ExportModels(wxString const& filename)
{
    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB) {
        layoutPanel->UnSelectAllModels();
    }
    RecalcModels();

    constexpr double FACTOR = 1.3;

    uint32_t minchannel = 99999999;
    int32_t maxchannel = -1;

    lxw_workbook* workbook = workbook_new(filename.c_str());
    lxw_worksheet* modelsheet = workbook_add_worksheet(workbook, "Models");
    lxw_worksheet* groupsheet = workbook_add_worksheet(workbook, "Groups");
    lxw_worksheet* controllersheet = workbook_add_worksheet(workbook, "Controllers");
    lxw_worksheet* totalsheet = workbook_add_worksheet(workbook, "Totals");

    lxw_format* header_format = workbook_add_format(workbook);
    format_set_border(header_format, LXW_BORDER_MEDIUM);
    format_set_bold(header_format);

    lxw_format* format = workbook_add_format(workbook);
    format_set_border(format, LXW_BORDER_THIN);

    auto write_worksheet_string = [FACTOR](lxw_worksheet* sheet, int row, int col, std::string text, lxw_format* format, std::map<int, double>& col_widths) {
        worksheet_write_string(sheet, row, col, text.c_str(), format);
        col_widths[col] = std::max(text.size() + FACTOR, col_widths[col]);
    };

    const std::vector<std::string> model_header_cols{ "Model Name", "Shadowing", "Description", "Display As", "Dimensions", "String Type", "String Count", "Node Count", "Light Count", "Est Current (Amps)", "Channels Per Node", "Channel Count", "Start Channel", "Start Channel No", "#Universe(or id):Start Channel", "End Channel No", "Default Buffer W x H", "Preview", "Controller Ports", "Connection Protocol", "Connection Attributes", "Controller Name", "Controller Type", "Protocol", "Controller Description", "IP", "Baud", "Universe/Id", "Universe Channel", "Controller Channel", "Active" };

    std::map<int, double> _model_col_widths;
    for (int i = 0; i < model_header_cols.size(); i++) {

        worksheet_write_string(modelsheet, 0, i, model_header_cols[i].c_str(), header_format);
        _model_col_widths[i] = model_header_cols[i].size() + FACTOR; //estimate column width
    }
    worksheet_freeze_panes(modelsheet, 1, 0);

    int modelCount = 0;
    int row = 1;
    //models
    for (auto const& m : AllModels) {
        Model* model = m.second;
        if (model->GetDisplayAs() != "ModelGroup") {
            modelCount++;
            wxString const stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
            uint32_t ch = model->GetFirstChannel() + 1;
            std::string type, description, ip, universe, inactive, baud, protocol, controllername;
            int32_t channeloffset;
            int stu = 0;
            int stuc = 0;
            GetControllerDetailsForChannel(ch, controllername, type, protocol, description, channeloffset, ip, universe, inactive, baud, stu, stuc);

            std::string current;

            wxString const stype = wxString(model->GetStringType());

            int32_t lightcount = (long)(model->GetNodeCount() * model->GetLightsPerNode());
            if (!stype.Contains("Node")) {
                if (model->GetNodeCount() == 1) {
                    lightcount = model->GetCoordCount(0);
                } else {
                    lightcount = model->NodesPerString() * model->GetLightsPerNode();
                }
            }

            if (stype.Contains("Node") || stype.Contains("Channel RGB")) {
                current = wxString::Format("%0.2f", (float)lightcount * AMPS_PER_PIXEL).ToStdString();
            }

            int w, h;
            model->GetBufferSize("Default", "2D", "None", w, h);
            write_worksheet_string(modelsheet, row, 0, model->name, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 1, model->GetShadowModelFor(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 2, model->description, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 3, model->GetDisplayAs(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 4, model->GetDimension(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 5, model->GetStringType(), format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 6, model->GetNumPhysicalStrings(), format);
            worksheet_write_number(modelsheet, row, 7, model->GetNodeCount(), format);
            worksheet_write_number(modelsheet, row, 8, lightcount, format);
            worksheet_write_number(modelsheet, row, 9, (float)lightcount * AMPS_PER_PIXEL, format);
            worksheet_write_number(modelsheet, row, 10, model->GetChanCountPerNode(), format);
            worksheet_write_number(modelsheet, row, 11, model->GetActChanCount(), format);
            write_worksheet_string(modelsheet, row, 12, stch, format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 13, ch, format);
            write_worksheet_string(modelsheet, row, 14, wxString::Format("#%i:%i", stu, stuc), format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 15, model->GetLastChannel() + 1, format);
            write_worksheet_string(modelsheet, row, 16, wxString::Format("%i x %i", w, h), format, _model_col_widths);

            write_worksheet_string(modelsheet, row, 17, model->GetLayoutGroup(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 18, model->GetControllerConnectionPortRangeString(), format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 19, model->GetControllerProtocol(), format, _model_col_widths);
            wxString con_attributes = model->GetControllerConnectionAttributeString();
            con_attributes.Replace(":", ",");
            if (con_attributes.StartsWith(",")) {
                con_attributes.Remove(0, 1);
            }
            write_worksheet_string(modelsheet, row, 20, con_attributes, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 21, controllername, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 22, type, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 23, protocol, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 24, description, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 25, ip, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 26, baud, format, _model_col_widths);
            write_worksheet_string(modelsheet, row, 27, universe, format, _model_col_widths);
            worksheet_write_number(modelsheet, row, 28, stuc, format);
            worksheet_write_number(modelsheet, row, 29, channeloffset, format);
            write_worksheet_string(modelsheet, row, 30, inactive, format, _model_col_widths);
            ++row;

            if (ch < minchannel) {
                minchannel = ch;
            }
            int32_t lastch = model->GetLastChannel() + 1;
            if (lastch > maxchannel) {
                maxchannel = lastch;
            }
        }
    }
    //set column widths
    for (auto const& [col, width] : _model_col_widths) {
        worksheet_set_column(modelsheet, col, col, width, NULL);
    }

    std::map<int, double> _group_col_widths;
    const std::vector<std::string> groupHeader{ "Group Name", "Models", "Models Count", "Default Buffer W x H", "Preview" };
    for (int i = 0; i < groupHeader.size(); i++) {
        worksheet_write_string(groupsheet, 0, i, groupHeader[i].c_str(), header_format);
        _group_col_widths[i] = groupHeader[i].size() + FACTOR; //estimate column width
    }
    worksheet_freeze_panes(groupsheet, 1, 0);
    int groupCount = 0;
    row = 1;
    for (auto const& m : AllModels) {
        Model* model = m.second;
        if (model->GetDisplayAs() == "ModelGroup") {
            groupCount++;
            ModelGroup* mg = static_cast<ModelGroup*>(model);
            std::string models;
            for (const auto& it : mg->ModelNames()) {
                if (models.empty()) {
                    models = it;
                } else {
                    models += ", " + it;
                }
            }
            int w, h;
            model->GetBufferSize("Default", "2D", "None", w, h);

            write_worksheet_string(groupsheet, row, 0, model->name, format, _group_col_widths);
            write_worksheet_string(groupsheet, row, 1, models, format, _group_col_widths);
            worksheet_write_number(groupsheet, row, 2, mg->ModelNames().size(), format);
            write_worksheet_string(groupsheet, row, 3, wxString::Format("%d x %d", w, h), format, _group_col_widths);
            write_worksheet_string(groupsheet, row, 4, model->GetLayoutGroup(), format, _group_col_widths);
            ++row;
        }
    }
    for (auto const& [col, width] : _group_col_widths) {
        worksheet_set_column(groupsheet, col, col, width, NULL);
    }

    row = 1;
    std::map<int, double> _controller_col_widths;

    auto control_cols = OutputManager::GetExportHeaders();

    for (int i = 0; i < control_cols.size(); i++) {
        worksheet_write_string(controllersheet, 0, i, control_cols[i].c_str(), header_format);
        _controller_col_widths[i] = control_cols[i].size() + FACTOR; //estimate column width
    }
    worksheet_freeze_panes(controllersheet, 1, 0);

    for (const auto& it : _outputManager.GetControllers()) {
        auto scolumns = it->GetExport();
        auto columns = wxSplit(scolumns, ',');
        for (int j = 0; j < columns.size(); j++) {
            write_worksheet_string(controllersheet, row, j, columns[j], format, _controller_col_widths);
        }
        ++row;
        for (auto it2 : it->GetOutputs()) {
            auto s = it2->GetExport();
            if (!s.empty()) {
                auto scolumns2 = it2->GetExport();
                auto columns2 = wxSplit(scolumns2, ',');
                for (int k = 0; k < columns2.size(); k++) {
                    write_worksheet_string(controllersheet, row, k, columns2[k], format, _controller_col_widths);
                }
                row++;
            }
        }
    }
    for (auto const& [col, width] : _controller_col_widths) {
        worksheet_set_column(controllersheet, col, col, width, NULL);
    }

    uint32_t bulbs = 0;
    uint32_t usedchannels = 0;
    if (minchannel == 99999999) {
        // No channels so we dont do this
        minchannel = 0;
        maxchannel = 0;
    }
    else {
        int* chused = (int*)malloc((maxchannel - minchannel + 1) * sizeof(int));
        memset(chused, 0x00, (maxchannel - minchannel + 1) * sizeof(int));

        for (auto const& m : AllModels) {
            Model* model = m.second;
            if (model->GetDisplayAs() != "ModelGroup") {
                int ch = model->GetFirstChannel() + 1;
                int endch = model->GetLastChannel() + 1;

                int uniquechannels = 0;
                for (int i = ch; i <= endch; i++) {
                    wxASSERT(i - minchannel < maxchannel - minchannel + 1);
                    if (chused[i - minchannel] == 0) {
                        uniquechannels++;
                    }
                    chused[i - minchannel]++;
                }

                if (wxString(model->GetStringType()).StartsWith("Single Color")) {
                    bulbs += uniquechannels * model->GetCoordCount(0);
                }
                else if (wxString(model->GetStringType()).StartsWith("3 Channel")) {
                    bulbs += uniquechannels * model->GetNodeCount() / 3 * model->GetCoordCount(0);
                }
                else if (wxString(model->GetStringType()).StartsWith("4 Channel")) {
                    bulbs += uniquechannels * model->GetNodeCount() / 4 * model->GetCoordCount(0);
                }
                else if (wxString(model->GetStringType()).StartsWith("Strobes")) {
                    bulbs += uniquechannels * model->GetNodeCount() * model->GetCoordCount(0);
                }
                else if (model->GetStringType() == "Node Single Color") {
                    bulbs += uniquechannels * model->GetNodeCount() * model->GetCoordCount(0);
                }
                else {
                    int den = model->GetChanCountPerNode();
                    if (den == 0) {
                        den = 1;
                    }
                    bulbs += uniquechannels / den * model->GetLightsPerNode();
                }
            }
        }

        for (long i = 0; i < (long)(maxchannel - minchannel + 1); i++) {
            if (chused[i] > 0) {
                usedchannels++;
            }
        }

        free(chused);
    }

    worksheet_write_string(totalsheet, 0, 0, "Model Count", format);
    worksheet_write_number(totalsheet, 0, 1, modelCount, format);
    worksheet_write_string(totalsheet, 1, 0, "Group Count", format);
    worksheet_write_number(totalsheet, 1, 1, groupCount, format);
    worksheet_write_string(totalsheet, 2, 0, "First Used Channel", format);
    worksheet_write_number(totalsheet, 2, 1, minchannel, format);
    worksheet_write_string(totalsheet, 3, 0, "Last Used Channel", format);
    worksheet_write_number(totalsheet, 3, 1, maxchannel, format);
    worksheet_write_string(totalsheet, 4, 0, "Actual Used Channel", format);
    worksheet_write_number(totalsheet, 4, 1, usedchannels, format);
    worksheet_write_string(totalsheet, 5, 0, "Bulbs", format);
    worksheet_write_number(totalsheet, 5, 1, bulbs, format);

    worksheet_set_column(totalsheet, 0, 0, 25, NULL);

    lxw_error error = workbook_close(workbook);
    if (error) {
        DisplayError(wxString::Format("Unable to create Spreadsheet, Error %d = %s\n", error, lxw_strerror(error)).ToStdString());
    }
}

void xLightsFrame::OnmExportModelsMenuItemSelected(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "Export Models", wxEmptyString, "Export files (*.xlsx)|*.xlsx", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) {
        return;
    }

    ExportModels(filename);
    SetStatusText("Model Spreadsheet saved at " + filename);
}

void xLightsFrame::OnMenuItem_ViewLogSelected(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString dir;
    wxString fileName = "xLights_l4cpp.log";
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    if (dir.EndsWith("/") || dir.EndsWith("\\")) {
        dir = dir.Left(dir.Length() - 1);
    }
    wxString filename = dir + "/" + fileName;
#endif
#ifdef __WXOSX__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    if (dir.EndsWith("/")) {
        dir = dir.Left(dir.Length() - 1);
    }
    wxString filename = dir + "/Library/Logs/" + fileName;
#endif
#ifdef __LINUX__
    wxString filename = "/tmp/" + fileName;
#endif
    wxString fn = "";
    if (FileExists(filename)) {
        fn = filename;
    }
    else if (FileExists(wxFileName(CurrentDir, fileName).GetFullPath())) {
        fn = wxFileName(CurrentDir, fileName).GetFullPath();
    }
    else if (FileExists(wxFileName(wxGetCwd(), fileName).GetFullPath())) {
        fn = wxFileName(wxGetCwd(), fileName).GetFullPath();
    }

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (fn != "" && ft) {
        wxString command = ft->GetOpenCommand("foo.txt");
        command.Replace("foo.txt", fn);

        logger_base.debug("Viewing log file %s.", (const char*)fn.c_str());

        wxUnsetEnv("LD_PRELOAD");
        wxExecute(command);
        delete ft;
    }
    else {
        DisplayError(wxString::Format("Unable to show log file '%s'.", fn).ToStdString(), this);
    }
}

void LogAndWrite(wxFile& f, const std::string& msg)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("CheckSequence: " + msg);
    if (f.IsOpened())
    {
        f.Write(msg + "\r\n");
    }
}

// recursively check whether a start channel refers to a model in a way that creates a referencing loop
bool xLightsFrame::CheckStart(wxFile& f, const std::string& startmodel, std::list<std::string>& seen, std::string& nextmodel)
{
    Model* m = AllModels.GetModel(nextmodel);
    if (m == nullptr)
    {
        return true; // this is actually an error but we have already reported these errors
    }
    else
    {
        std::string start = m->ModelStartChannel;

        if (start[0] == '>' || start[0] == '@')
        {
            seen.push_back(nextmodel);
            size_t colon = start.find(':', 1);
            std::string reference = start.substr(1, colon - 1);

            if (std::find(seen.begin(), seen.end(), reference) != seen.end())
            {
                wxString msg = wxString::Format("    ERR: Model '%s' start channel results in a reference loop.", startmodel);
                LogAndWrite(f, msg.ToStdString());
                for (const auto& it : seen)
                {
                    msg = wxString::Format("       '%s' ->", it);
                    LogAndWrite(f, msg.ToStdString());
                }
                msg = wxString::Format("       '%s'", reference);
                LogAndWrite(f, msg.ToStdString());
                return false;
            }
            else
            {
                return CheckStart(f, startmodel, seen, reference);
            }
        }
        else
        {
            // it resolves to something ok
            return true;
        }
    }
}

bool compare_modelstartchannel(const Model* first, const Model* second)
{
    int firstmodelstart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondmodelstart = second->GetNumberFromChannelString(second->ModelStartChannel);

    return firstmodelstart < secondmodelstart;
}

std::string xLightsFrame::CheckSequence(bool displayInEditor, bool writeToFile)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB)
        layoutPanel->UnSelectAllModels();
    RecalcModels();

    size_t errcount = 0;
    size_t warncount = 0;

    wxFile f;
    wxString filename = wxFileName::CreateTempFileName("xLightsCheckSequence") + ".txt";

    if (writeToFile || displayInEditor) {
        f.Open(filename, wxFile::write);
        if (!f.IsOpened()) {
            DisplayError("Unable to create results file for Check Sequence. Aborted.", this);
            return "";
        }
    }

    wxProgressDialog prog("Check Sequence", "", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();

    LogAndWrite(f, "Checking sequence.");
    LogAndWrite(f, "");

    LogAndWrite(f, "Show folder: " + GetShowDirectory());
    LogAndWrite(f, "");

    if (CurrentSeqXmlFile != nullptr) {
        wxFileName fn(CurrentSeqXmlFile->GetFullPath());
        fn.SetExt("xsq");
        LogAndWrite(f, "Sequence: " + fn.GetFullPath());
    }
    else {
        LogAndWrite(f, "Sequence: No sequence open.");
    }

    prog.Update(0, "Checking network");
    wxYield();

    wxDatagramSocket *testSocket;
    wxIPV4address addr;
    wxString fullhostname = wxGetFullHostName();
    if (_outputManager.GetGlobalForceLocalIP() != "") {
        addr.Hostname(_outputManager.GetGlobalForceLocalIP());
        testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
    }
    else {
        addr.AnyAddress();
        testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
        addr.Hostname(fullhostname);
        if (addr.IPAddress() == "255.255.255.255") {
            addr.Hostname(wxGetHostName());
        }
    }

    LogAndWrite(f, "");
    LogAndWrite(f, "Full host name: " + fullhostname.ToStdString());
    LogAndWrite(f, "IP Address we are outputting data from: " + addr.IPAddress().ToStdString());
    LogAndWrite(f, "If your PC has multiple network connections (such as wired and wireless) this should be the IP Address of the adapter your controllers are connected to. If it isn't your controllers may not receive output data.");
    LogAndWrite(f, "If you are experiencing this problem you may need to set the local IP address to use.");

    if (testSocket == nullptr || !testSocket->IsOk() || testSocket->Error() != wxSOCKET_NOERROR) {
        wxString msg("    ERR: Cannot create socket on IP address '");
        msg += addr.IPAddress();
        msg += "'. Is the network connected?    ";
        msg = msg + " Ok : " + (testSocket->IsOk() ? "TRUE" : "FALSE");
        if (testSocket != nullptr && testSocket->IsOk()) {
            msg += wxString::Format(" : Error %d : ", testSocket->LastError()) + DecodeIPError(testSocket->LastError());
        }
        LogAndWrite(f, msg.ToStdString());
        errcount++;
    }

    if (testSocket != nullptr) {
        delete testSocket;
    }

    LogAndWrite(f, "");
    LogAndWrite(f, "IP Addresses on this machine:");
    for (const auto& it : GetLocalIPs()) {
        LogAndWrite(f, wxString::Format("    %s", it));
    }

    size_t errcountsave = errcount;
    size_t warncountsave = warncount;

    prog.Update(1, "Checking preferences");
    wxYield();

    LogAndWrite(f, "");
    LogAndWrite(f, "Working in a backup directory");

    if (ShowFolderIsInBackup(GetShowDirectory())) {
        wxString msg = wxString::Format("    ERR: Show directory is a (or is under a) backup show directory. %s.", GetShowDirectory());
        LogAndWrite(f, msg.ToStdString());
        errcount++;
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Potentially problematic settings");

    if (!_backupSubfolders) {
        wxString msg = wxString::Format("    WARN: Backup is not including subfolders. If you store your sequences in subfolders then they are NOT being backed up by xLights.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (_suspendRender) {
        wxString msg = wxString::Format("    WARN: Rendering is currently suspended. The FSEQ data may not be up to date.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (mRenderOnSave) {
        wxString msg = wxString::Format("    WARN: Render on save is enabled ... this is generally unnecessary.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (mBackupOnSave) {
        wxString msg = wxString::Format("    WARN: Backup on save is enabled ... this creates an awful lot of backups.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (!mSaveFseqOnSave) {
        wxString msg = wxString::Format("    ERR: Saving FSEQ on save is disabled ... this is almost always a bad idea.");
        LogAndWrite(f, msg.ToStdString());
        errcount++;
    }

    if (mAutoSaveInterval <= 0) {
        wxString msg = wxString::Format("    WARN: Autosave is disabled ... you will lose work if xLights abnormally terminates.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (_lowDefinitionRender) {
        wxString msg = wxString::Format("    WARN: Rendering in low definition is active.");
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (mBackgroundImage != "") {
        if (!wxIsReadable(mBackgroundImage) || !wxImage::CanRead(mBackgroundImage)) {
            wxString msg = wxString::Format("    ERR: Layout Background image not loadable as an image: %s.", mBackgroundImage);
            LogAndWrite(f, msg.ToStdString());
            errcount++;
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    prog.Update(3, "Checking controllers");
    wxYield();

    LogAndWrite(f, "");
    LogAndWrite(f, "Inactive Controllers");

    // Check for inactive outputs
    for (const auto& c : _outputManager.GetControllers()) {
        if (!c->IsEnabled() && c->CanSendData()) {
            wxString msg = wxString::Format("    WARN: Inactive controller %s %s:%s.",
                c->GetName(), c->GetColumn1Label(), c->GetColumn2Label());
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // multiple outputs to same universe and same IP
    LogAndWrite(f, "");
    LogAndWrite(f, "Multiple outputs sending to same destination");

    std::list<std::string> used;
    for (const auto& o : _outputManager.GetAllOutputs()) {
        if (o->IsIpOutput() && (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_KINET)) {
            std::string usedval = o->GetIP() + "|" + o->GetUniverseString();

            if (std::find(used.begin(), used.end(), usedval) != used.end()) {
                int32_t sc;
                auto c = _outputManager.GetController(o->GetStartChannel(), sc);

                wxString msg = wxString::Format("    ERR: Multiple outputs being sent to the same controller '%s' (%s) and universe %s.",
                    (const char*)c->GetName().c_str(),
                    (const char*)o->GetIP().c_str(),
                    (const char*)o->GetUniverseString().c_str());
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
            else {
                used.push_back(usedval);
            }
        }
        else if (o->IsSerialOutput()) {
            if (o->GetCommPort() != "NotConnected") {
                if (std::find(used.begin(), used.end(), o->GetCommPort()) != used.end()) {
                    wxString msg = wxString::Format("    ERR: Multiple outputs being sent to the same comm port %s '%s'.", (const char*)o->GetType().c_str(), (const char*)o->GetCommPort().c_str());
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
                else {
                    used.push_back(o->GetCommPort());
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave)
    {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Controller Checks
    //do these checks for all Managed Controllers
    std::list<Controller*> uniqueControllers;
    for (const auto& it : _outputManager.GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && (eth->GetProtocol() == OUTPUT_ZCPP || eth->GetProtocol() == OUTPUT_DDP || eth->IsManaged())) {
            uniqueControllers.push_back(eth);
        }
    }

    if (uniqueControllers.size() > 0) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Controller Checks");

        // controller ip address must only be on one output ... no duplicates
        for (const auto& it : uniqueControllers) {
            for (const auto& itc : _outputManager.GetControllers()) {
                auto eth = dynamic_cast<ControllerEthernet*>(itc);
                if (eth != nullptr) {
                    if (eth != it && it->GetIP() != "MULTICAST" && (it->GetIP() == eth->GetIP() || it->GetIP() == eth->GetResolvedIP())) {
                        wxString msg = wxString::Format("    ERR: %s IP Address '%s' for controller '%s' used on another controller '%s'. This is not allowed.",
                            (const char*)it->GetProtocol().c_str(),
                            (const char*)it->GetIP().c_str(),
                            (const char*)it->GetName().c_str(),
                            (const char*)eth->GetName().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                        break;
                    }
                }
            }
        }

        std::map<std::string, std::map<std::string, std::list<Model*>>> modelsByPortByController;
        for (const auto& it : AllModels) {
            if (it.second->GetControllerName() != "") {
                auto c = _outputManager.GetController(it.second->GetControllerName());
                if (c != nullptr) {
                    auto caps = c->GetControllerCaps();
                    if (!it.second->IsControllerConnectionValid() && (caps != nullptr && caps->GetMaxPixelPort() != 0 && caps->GetMaxSerialPort() != 0)) {
                        wxString msg = wxString::Format("    ERR: Model %s on %s controller '%s:%s' has invalid controller connection '%s'.",
                            (const char*)it.second->GetName().c_str(),
                            (const char*)c->GetProtocol().c_str(),
                            (const char*)c->GetName().c_str(),
                            (const char*)c->GetIP().c_str(),
                            (const char*)it.second->GetControllerConnectionString().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }

                    if (modelsByPortByController.find(c->GetName()) == modelsByPortByController.end()) {
                        std::map<std::string, std::list<Model*>> pm;
                        modelsByPortByController[c->GetName()] = pm;
                    }
                    modelsByPortByController[c->GetName()][wxString::Format("%s:%d:%d", it.second->IsPixelProtocol() ? _("PIXEL") : _("SERIAL"), it.second->GetControllerPort(), it.second->GetSmartRemote()).Lower().ToStdString()].push_back(it.second);
                }
            }
        }

        // Models with chains to models that dont exist or are not on same controller and port
        // Multiple models with the same chain value on the same port on the same controller
        // Loops in model chains

        // for each controller
        for (auto& it : modelsByPortByController) {
            //it->first is controller
            //it->second is a list of ports

            auto c = _outputManager.GetController(it.first);

            // for each port
            for (auto& itp : it.second) {
                // itp->first is the port name
                // itp->second  is the model list

                // dont scan serial because the chaining rules are different
                if (Contains(itp.first, "pixel")) {
                    // validate that all chained

                    // order the models
                    std::string last = "";

                    while (itp.second.size() > 0) {
                        bool pushed = false;
                        for (auto itms = begin(itp.second); itms != end(itp.second); ++itms) {
                            if (((*itms)->GetModelChain() == "Beginning" && last == "") ||
                                (*itms)->GetModelChain() == last ||
                                (*itms)->GetModelChain() == ">" + last) {
                                pushed = true;
                                last = (*itms)->GetName();
                                itp.second.erase(itms);
                                break;
                            }
                        }

                        if (!pushed && itp.second.size() > 0) {
                            // chain is broken ... so just put the rest in in random order
                            while (itp.second.size() > 0) {
                                wxString msg = wxString::Format("    ERR: Model %s on ZCPP controller '%s:%s' on port '%s' has invalid Model Chain '%s'. It may be a duplicate or point to a non existent model on this controller port or there may be a loop.",
                                    (const char*)itp.second.front()->GetName().c_str(),
                                    (const char*)c->GetIP().c_str(),
                                    (const char*)c->GetName().c_str(),
                                    (const char*)itp.second.front()->GetControllerConnectionString().c_str(),
                                    (const char*)itp.second.front()->GetModelChain().c_str());
                                LogAndWrite(f, msg.ToStdString());
                                errcount++;
                                itp.second.pop_front();
                            }
                        }
                    }
                }
            }
        }

        // Apply the vendor specific validations
        for (const auto& it : _outputManager.GetControllers()) {
            wxString msg = wxString::Format("        Applying controller rules for %s:%s:%s", it->GetName(), it->GetIP(), it->GetDescription());
            LogAndWrite(f, msg.ToStdString());

            std::string check;
            UDController edc(it, &_outputManager, &AllModels, false);

            check = "";
            auto fcr = ControllerCaps::GetControllerConfig(it->GetVendor(), it->GetModel(), it->GetVariant());
            if (fcr != nullptr) {
                edc.Check(fcr, check);
            }
            else {
                LogAndWrite(f, "Unknown controller vendor.");
            }
            if (check != "") {
                LogAndWrite(f, check);
                errcount += CountStrings("ERR:", check);
                warncount += CountStrings("WARN:", check);
            }
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;
    }

    if (!IsCheckSequenceOptionDisabled("DupUniv")) {
        // multiple outputs to same universe/ID
        LogAndWrite(f, "");
        LogAndWrite(f, "Multiple outputs with same universe/id number");

        std::map<int, int> useduid;
        auto outputs = _outputManager.GetAllOutputs();
        for (auto o : outputs) {
            if (o->GetType() != OUTPUT_ZCPP) {
                useduid[o->GetUniverse()]++;
            }
        }

        for (auto u : useduid) {
            if (u.second > 1) {
                wxString msg = wxString::Format("    WARN: Multiple outputs (%d) with same universe/id number %d. If using #universe:start_channel result may be incorrect.", u.second, u.first);
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;
    }
    else {
        LogAndWrite(f, "");
        LogAndWrite(f, "Multiple outputs with same universe/id number - CHECK DISABLED");
    }

    // Controller universes out of order
    LogAndWrite(f, "");
    LogAndWrite(f, "Controller universes out of order - because some controllers care");

    std::map<std::string, int> lastuniverse;
    for (auto n : _outputManager.GetAllOutputs()) {
        if (n->IsIpOutput() && (n->GetType() == OUTPUT_E131 || n->GetType() == OUTPUT_ARTNET)) {
            if (lastuniverse.find(n->GetIP()) == lastuniverse.end()) {
                lastuniverse[n->GetIP()] = n->GetUniverse();
            }
            else {
                if (lastuniverse[n->GetIP()] > n->GetUniverse()) {
                    wxString msg = wxString::Format("    WARN: Controller %s Universe %d occurs after universe %d. Some controllers do not like out of order universes.",
                        n->GetIP(), n->GetUniverse(), lastuniverse[n->GetIP()]);
                    LogAndWrite(f, msg.ToStdString());
                    warncount++;
                }
                else {
                    lastuniverse[n->GetIP()] = n->GetUniverse();
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // controllers sending to routable IP addresses
    LogAndWrite(f, "");
    LogAndWrite(f, "Invalid controller IP addresses");

    for (const auto& c : _outputManager.GetControllers()) {
        auto eth = c;
        if (eth->GetIP() != "" && eth->GetIP() != "MULTICAST") {
            if (!ip_utils::IsIPValidOrHostname(eth->GetIP())) {
                wxString msg = wxString::Format("    WARN: IP address '%s' on controller '%s' does not look valid.",
                    (const char*)eth->GetIP().c_str(),
                    (const char*)eth->GetName().c_str());
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
            else {
                wxArrayString ipElements = wxSplit(eth->GetIP(), '.');
                if (ipElements.size() > 3) {
                    //looks like an IP address
                    int ip1 = wxAtoi(ipElements[0]);
                    int ip2 = wxAtoi(ipElements[1]);
                    int ip3 = wxAtoi(ipElements[2]);
                    int ip4 = wxAtoi(ipElements[3]);

                    if (ip1 == 10) {
                        if (ip2 == 255 && ip3 == 255 && ip4 == 255) {
                            wxString msg = wxString::Format("    ERR: IP address '%s' on controller '%s' is a broadcast address.",
                                (const char*)eth->GetIP().c_str(),
                                (const char*)eth->GetName().c_str());
                            LogAndWrite(f, msg.ToStdString());
                            errcount++;
                        }
                        // else this is valid
                    }
                    else if (ip1 == 192 && ip2 == 168) {
                        if (ip3 == 255 && ip4 == 255) {
                            wxString msg = wxString::Format("    ERR: IP address '%s' on controller '%s' is a broadcast address.",
                                (const char*)eth->GetIP().c_str(),
                                (const char*)eth->GetName().c_str());
                            LogAndWrite(f, msg.ToStdString());
                            errcount++;
                        }
                        // else this is valid
                    }
                    else if (ip1 == 172 && ip2 >= 16 && ip2 <= 31) {
                        // this is valid
                    }
                    else if (ip1 == 255 && ip2 == 255 && ip3 == 255 && ip4 == 255) {
                        wxString msg = wxString::Format("    ERR: IP address '%s' on controller '%s' is a broadcast address.",
                            (const char*)eth->GetIP().c_str(),
                            (const char*)eth->GetName().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                    else if (ip1 == 0) {
                        wxString msg = wxString::Format("    ERR: IP address '%s' on controller '%s' not valid.",
                            (const char*)eth->GetIP().c_str(),
                            (const char*)eth->GetName().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                    else if (ip1 >= 224 && ip1 <= 239) {
                        wxString msg = wxString::Format("    ERR: IP address '%s' on controller '%s' is a multicast address.",
                            (const char*)eth->GetIP().c_str(),
                            (const char*)eth->GetName().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                    else {
                        wxString msg = wxString::Format("    WARN: IP address '%s' on controller '%s' in internet routable ... are you sure you meant to do this.",
                            (const char*)eth->GetIP().c_str(),
                            (const char*)eth->GetName().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        warncount++;
                    }
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Models spanning controllers");
    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            int32_t start = it.second->GetFirstChannel() + 1;
            int32_t end = it.second->GetLastChannel() + 1;

            int32_t sc;
            Controller* ostart = _outputManager.GetController(start, sc);
            Controller* oend = _outputManager.GetController(end, sc);

            auto eth_ostart = dynamic_cast<ControllerEthernet*>(ostart);
            auto eth_oend = dynamic_cast<ControllerEthernet*>(oend);
            auto ser_ostart = dynamic_cast<ControllerSerial*>(ostart);
            auto ser_oend = dynamic_cast<ControllerSerial*>(oend);

            if (ostart != nullptr && oend == nullptr) {
                wxString msg = wxString::Format("    ERR: Model '%s' starts on controller '%s' but ends at channel %d which is not on a controller.", it.first, ostart->GetName(), end);
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
            else if (ostart == nullptr || oend == nullptr) {
                wxString msg = wxString::Format("    ERR: Model '%s' is not configured for a controller.", it.first);
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
            else if (ostart->GetType() != oend->GetType()) {
                wxString msg = wxString::Format("    WARN: Model '%s' starts on controller '%s' of type '%s' but ends on a controller '%s' of type '%s'.", it.first, ostart->GetName(), ostart->GetType(), oend->GetDescription(), oend->GetType());
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
            else if (eth_ostart != nullptr && eth_oend != nullptr && eth_ostart->GetIP() == "MULTICAST" && eth_oend->GetIP() == "MULTICAST") {
                // ignore these
            }
            else if (eth_ostart != nullptr && eth_oend != nullptr && eth_ostart->GetIP() + eth_oend->GetIP() != "") {
                if (eth_ostart->GetIP() != eth_oend->GetIP()) {
                    wxString msg = wxString::Format("    WARN: Model '%s' starts on controller '%s' with IP '%s' but ends on a controller '%s' with IP '%s'.",
                        it.first,
                        ostart->GetName(),
                        eth_ostart->GetIP(),
                        oend->GetName(),
                        eth_oend->GetIP());
                    LogAndWrite(f, msg.ToStdString());
                    warncount++;
                }
            }
            else if (ser_ostart != nullptr && ser_oend != nullptr && ser_ostart->GetPort() + ser_oend->GetPort() != "") {
                if (ser_ostart->GetPort() != ser_oend->GetPort()) {
                    wxString msg = wxString::Format("    WARN: Model '%s' starts on controller '%s' with CommPort '%s' but ends on a controller '%s' with CommPort '%s'.",
                        it.first,
                        ostart->GetName(),
                        ser_ostart->GetPort(),
                        ser_oend->GetName(),
                        ser_oend->GetPort());
                    LogAndWrite(f, msg.ToStdString());
                    warncount++;
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    prog.Update(50, "Checking models");
    wxYield();

    LogAndWrite(f, "");
    LogAndWrite(f, "Invalid start channels");

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            std::string start = it.second->ModelStartChannel;

            if (start[0] == '>' || start[0] == '@') {
                size_t colon = start.find(':', 1);
                std::string reference = start.substr(1, colon - 1);

                if (reference == it.first) {
                    wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' refers to itself.", it.first, start);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
                else {
                    Model* m = AllModels.GetModel(reference);
                    if (m == nullptr) {
                        wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' refers to non existent model '%s'.", it.first, start, reference);
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                }
            }
            else if (start[0] == '!') {
                auto comp = wxSplit(start.substr(1), ':');
                if (_outputManager.GetController(comp[0]) == nullptr) {
                    wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' refers to non existent controller '%s'.", it.first, start, comp[0]);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
            if (it.second->GetLastChannel() == (unsigned int)-1) {
                wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' evaluates to an illegal channel number.", it.first, start);
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
        }
    }

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            std::string start = it.second->ModelStartChannel;

            if (start[0] == '>' || start[0] == '@') {
                std::list<std::string> seen;
                seen.push_back(it.first);
                size_t colon = start.find(':', 1);
                if (colon != std::string::npos) {
                    std::string reference = start.substr(1, colon - 1);
                    if (reference != it.first) {
                        if (!CheckStart(f, it.first, seen, reference)) {
                            errcount++;
                        }
                    }
                }
                else {
                    wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' invalid.", it.first, start);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
            else if (start[0] == '#') {
                size_t colon = start.find(':', 1);
                if (colon != std::string::npos) {
                    size_t colon2 = start.find(':', colon + 1);
                    if (colon2 == -1) {
                        colon2 = colon;
                        colon = 0;
                    }
                    int universe = wxAtoi(wxString(start.substr(colon + 1, colon2 - 1)));

                    Output* o = _outputManager.GetOutput(universe, "");

                    if (o == nullptr) {
                        wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' refers to undefined universe %d.", it.first, start, universe);
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                }
                else {
                    wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' invalid.", it.first, start);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
            else if (start[0] == '!') {
                // nothing to check
            }
            else if (start.find(':') != std::string::npos) {
                size_t colon = start.find(':');
                int output = wxAtoi(wxString(start.substr(0, colon)));

                auto cnt = _outputManager.GetOutputCount();

                if (output < 1 || output > cnt) {
                    wxString msg = wxString::Format("    ERR: Model '%s' start channel '%s' refers to undefined output %d. Only %d outputs are defined.", it.first, start, output, cnt);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Overlapping model channels");

    // Check for overlapping channels in models
    for (auto it = std::begin(AllModels); it != std::end(AllModels); ++it) {
        if (it->second->GetDisplayAs() != "ModelGroup") {
            auto m1start = it->second->GetFirstChannel() + 1;
            auto m1end = it->second->GetLastChannel() + 1;

            for (auto it2 = std::next(it); it2 != std::end(AllModels); ++it2) {
                if (it2->second->GetDisplayAs() != "ModelGroup" && it2->second->GetShadowModelFor() != it->first && it->second->GetShadowModelFor() != it2->first) {
                    auto m2start = it2->second->GetFirstChannel() + 1;
                    auto m2end = it2->second->GetLastChannel() + 1;

                    if (m2start <= m1end && m2end >= m1start) {
                        wxString msg = wxString::Format("    WARN: Probable model overlap '%s' (%d-%d) and '%s' (%d-%d).",
                            it->first, m1start, m1end,
                            it2->first, m2start, m2end);
                        LogAndWrite(f, msg.ToStdString());
                        warncount++;
                    }
                }
            }
        }
    }
    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for non contiguous models on the same controller connection
    if (!IsCheckSequenceOptionDisabled("NonContigChOnPort")) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Non contiguous channels on controller ports");

        std::map<std::string, std::list<Model*>*> modelsByPort;
        for (const auto& it : AllModels) {
            if (it.second->GetDisplayAs() != "ModelGroup") {
                std::string cc = "";
                if (it.second->IsControllerConnectionValid()) {
                    cc = wxString::Format("%s:%s:%d:%d", it.second->IsPixelProtocol() ? _("pixel") : _("serial"), it.second->GetControllerProtocol(), it.second->GetControllerPort(), it.second->GetSmartRemote()).ToStdString();
                }
                if (cc != "") {
                    int32_t start = it.second->GetFirstChannel() + 1;
                    int32_t sc;
                    Output* o = _outputManager.GetOutput(start, sc);

                    if (o != nullptr && o->IsIpOutput() && o->GetIP() != "MULTICAST") {
                        std::string key = o->GetIP() + cc;
                        if (modelsByPort.find(key) == modelsByPort.end()) {
                            modelsByPort[key] = new std::list<Model*>();
                        }
                        modelsByPort[key]->push_back(it.second);
                    }
                }
            }
        }

        for (auto& it : modelsByPort) {
            if (it.second->size() == 1 || Contains(it.first, "serial")) {
                // we dont need to check this one because one model or a serial protocol
            }
            else {
                it.second->sort(compare_modelstartchannel);

                auto it2 = it.second->begin();
                auto it3 = it2;
                ++it3;

                while (it3 != it.second->end()) {
                    int32_t m1start = (*it2)->GetNumberFromChannelString((*it2)->ModelStartChannel);
                    int32_t m1end = m1start + (*it2)->GetChanCount() - 1;
                    int32_t m2start = (*it3)->GetNumberFromChannelString((*it3)->ModelStartChannel);

                    if (m1end + 1 != m2start && m2start - m1end - 1 > 0) {
                        int32_t sc;
                        Output* o = _outputManager.GetOutput(m1start, sc);
                        wxString msg;
                        if (m2start - m1end - 1 <= 30) {
                            msg = wxString::Format("    WARN: Model '%s' and Model '%s' are on controller IP '%s' Output Connection '%s' but there is a small gap of %d channels between them. Maybe these are NULL Pixels?",
                                (*it2)->GetName(),
                                (*it3)->GetName(),
                                o->GetIP(),
                                (*it2)->GetControllerConnectionString(),
                                m2start - m1end - 1);
                            warncount++;
                        }
                        else {
                            msg = wxString::Format("    WARN: Model '%s' and Model '%s' are on controller IP '%s' Output Connection '%s' but there is a gap of %d channels between them.",
                                (*it2)->GetName(),
                                (*it3)->GetName(),
                                o->GetIP(),
                                (*it2)->GetControllerConnectionString(),
                                m2start - m1end - 1);
                            warncount++;
                        }
                        LogAndWrite(f, msg.ToStdString());
                    }

                    ++it2;
                    ++it3;
                }
            }
            delete it.second;
            it.second = nullptr;
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;
    } else {
        LogAndWrite(f, "");
        LogAndWrite(f, "Non contiguous channels on controller ports - CHECK DISABLED");
    }

    LogAndWrite(f, "");
    LogAndWrite(f, "Model nodes not allocated to layers correctly");

    for (auto it = AllModels.begin(); it != AllModels.end(); ++it) {
        if (it->second->GetDisplayAs() != "ModelGroup") {
            if (wxString(it->second->GetStringType()).EndsWith("Nodes") && !it->second->AllNodesAllocated()) {
                wxString msg = wxString::Format("    WARN: %s model '%s' Node Count and Layer Size allocations dont match.", it->second->GetDisplayAs().c_str(), it->first);
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Models with issues");

    for (const auto& it : AllModels) {
        std::list<std::string> warnings = it.second->CheckModelSettings();
        for (const auto& it : warnings) {
            LogAndWrite(f, it);
            if (it.find("WARN:") != std::string::npos) {
                warncount++;
            }
            else if (it.find("ERR:") != std::string::npos) {
                errcount++;
            }
        }

        if ((it.second->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE || it.second->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) && it.second->GetNodeCount() > 100) {
            wxString msg = wxString::Format("    WARN: model '%s' uses pixel style '%s' which is known to render really slowly. Consider using a different pixel style.", it.first, Model::GetPixelStyleDescription(it.second->GetPixelStyle()));
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
    }

    for (const auto& it : AllObjects) {
        std::list<std::string> warnings = it.second->CheckModelSettings();
        for (const auto& it : warnings) {
            LogAndWrite(f, it);
            if (it.find("WARN:") != std::string::npos) {
                warncount++;
            }
            else if (it.find("ERR:") != std::string::npos) {
                errcount++;
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    if (!IsCheckSequenceOptionDisabled("PreviewGroup")) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Model Groups containing models from different previews");

        for (const auto& it : AllModels) {
            if (it.second->GetDisplayAs() == "ModelGroup") {
                std::string mgp = it.second->GetLayoutGroup();

                ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
                if (mg == nullptr) {
                    // this should never happen
                    logger_base.error("Model %s says it is a model group but it doesn't cast as one.", (const char*)it.second->GetName().c_str());
                }
                else {
                    auto models = mg->ModelNames();

                    for (auto it2 : models) {
                        Model* m = AllModels.GetModel(it2);
                        if (m == nullptr) {
                            // this should never happen
                            logger_base.error("Model Group %s contains non existent model %s.", (const char*)mg->GetName().c_str(), (const char*)it2.c_str());
                        }
                        else if (m->GetDisplayAs() != "ModelGroup") {
                            // If model is in all previews dont report it as a problem
                            if (m->GetLayoutGroup() != "All Previews" && mg->GetLayoutGroup() != "All Previews" && mgp != m->GetLayoutGroup()) {
                                wxString msg = wxString::Format("    WARN: Model Group '%s' in preview '%s' contains model '%s' which is in preview '%s'. This will cause the '%s' model to also appear in the '%s' preview.", mg->GetName(), mg->GetLayoutGroup(), m->GetName(), m->GetLayoutGroup(), m->GetName(), mg->GetLayoutGroup());
                                LogAndWrite(f, msg.ToStdString());
                                warncount++;
                            }
                        }
                    }
                }
            }
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;
    } else {
        LogAndWrite(f, "");
        LogAndWrite(f, "Model Groups containing models from different previews - CHECK DISABLED");
    }

    // Check for duplicate model/model group names
    LogAndWrite(f, "");
    LogAndWrite(f, "Model/Model Groups without distinct names");

    for (auto it = std::begin(AllModels); it != std::end(AllModels); ++it) {
        for (auto it2 = std::next(it); it2 != std::end(AllModels); ++it2) {
            if (it->second->GetName() == it2->second->GetName()) {
                wxString msg = wxString::Format("    ERR: Duplicate Model/Model Group Name '%s'.", it->second->GetName());
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for model groups containing itself or other model groups
    LogAndWrite(f, "");
    LogAndWrite(f, "Model Groups containing non existent models");

    std::list<std::string> emptyModelGroups;

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() == "ModelGroup") {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) { // this should never fail
                auto models = mg->ModelNames();

                int modelCount = 0;

                for (const auto& m : models) {
                    Model* model = AllModels.GetModel(m);

                    if (model == nullptr) {
                        wxString msg = wxString::Format("    ERR: Model group '%s' refers to non existent model '%s'.", mg->GetName(), m.c_str());
                        LogAndWrite(f, msg.ToStdString());
                        errcount++;
                    }
                    else {
                        modelCount++;
                        if (model->GetName() == mg->GetName()) {
                            wxString msg = wxString::Format("    ERR: Model group '%s' contains reference to itself.", mg->GetName());
                            LogAndWrite(f, msg.ToStdString());
                            errcount++;
                        }
                    }
                }
                if (modelCount == 0) {
                    emptyModelGroups.push_back(it.first);
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for model groups containing no valid models
    LogAndWrite(f, "");
    LogAndWrite(f, "Model Groups containing no models that exist");

    for (const auto& it : emptyModelGroups) {
        wxString msg = wxString::Format("    ERR: Model group '%s' contains no models.", it);
        LogAndWrite(f, msg.ToStdString());
        errcount++;
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    if (!IsCheckSequenceOptionDisabled("DupNodeMG")) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Model Groups containing duplicate nodes");

        for (const auto& it : AllModels) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                std::map<long, Model*> usedch;
                std::map<std::string, bool> warned;
                for (const auto& m : mg->Models()) {
                    std::vector<NodeBaseClassPtr> nodes;
                    int bufwi;
                    int bufhi;
                    m->InitRenderBufferNodes("Default", "2D", "None", nodes, bufwi, bufhi);
                    for (const auto& n : nodes) {
                        auto e = usedch.find(n->ActChan);
                        if (e != end(usedch)) {
                            if (m->GetFullName() != e->second->GetFullName()) { // dont warn about duplicate nodes within a model
                                std::string warn = mg->Name() + m->Name() + e->second->Name();
                                if (warned.find(warn) == end(warned)) {
                                    warned[warn] = true;
                                    wxString msg = wxString::Format("    WARN: Model group '%s' contains model '%s' and model '%s' which contain at least one overlapping node (ch %u). This may not render as expected.", (const char*)mg->Name().c_str(), (const char*)m->GetFullName().c_str(), (const char*)e->second->GetFullName().c_str(), n->ActChan);
                                    LogAndWrite(f, msg.ToStdString());
                                    warncount++;
                                }
                            }
                        }
                        else {
                            usedch[n->ActChan] = m;
                        }
                    }
                }
            }
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;
    } else {
        LogAndWrite(f, "");
        LogAndWrite(f, "Model Groups containing duplicate nodes - CHECK DISABLED");
    }

    // Check for model groups and DMX models and common problems
    LogAndWrite(f, "");
    LogAndWrite(f, "Model Groups with DMX models likely to cause issues");

    std::list<ModelGroup*> modelGroupsWithDMXModels;
    for (const auto& it : AllModels) {
        ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);

        if (mg != nullptr) {
            for (const auto& it2 : mg->ModelNames()) {
                auto m = AllModels[it2];
                if (m->IsDMXModel()) {
                    modelGroupsWithDMXModels.push_back(mg);
                    break;
                }
            }
        }
    }

    // now we have a list of groups containing models ... look for model groups containing those groups
    for (const auto& it : AllModels) {
        ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);

        if (mg != nullptr) {
            for (const auto& it2 : modelGroupsWithDMXModels) {
                if (mg->DirectlyContainsModel(it2)) {
                    wxString msg = wxString::Format("    WARN: Model group '%s' contains model group '%s' which contains one or more DMX models. This is not likely to work as expected.", (const char*)mg->Name().c_str(), (const char*)it2->Name().c_str());
                    LogAndWrite(f, msg.ToStdString());
                    warncount++;
                }
            }
        }
    }

    // Also check those groups only contain models which are all DMX and the same number of channels
    for (const auto& it : modelGroupsWithDMXModels) {
        int numchannels = -1;
        for (const auto& it2 : it->ModelNames()) {
            auto m = AllModels[it2];
            if (!m->IsDMXModel()) {
                wxString msg = wxString::Format("    WARN: Model group '%s' contains a mix of DMX and non DMX models. This is not likely to work as expected.", (const char*)it->Name().c_str());
                LogAndWrite(f, msg.ToStdString());
                warncount++;
                break;
            }
            else {
                if (numchannels == -1) {
                    numchannels = m->GetChanCount();
                }
                else {
                    if (numchannels != m->GetChanCount()) {
                        wxString msg = wxString::Format("    WARN: Model group '%s' contains DMX models with varying numbers of channels. This is not likely to work as expected.", (const char*)it->Name().c_str());
                        LogAndWrite(f, msg.ToStdString());
                        warncount++;
                        break;
                    }
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for submodels with no nodes
    LogAndWrite(f, "");
    LogAndWrite(f, "SubModels with no nodes");

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                Model* sm = it.second->GetSubModel(i);
                if (sm->GetNodeCount() == 0) {
                    wxString msg = wxString::Format("    ERR: SubModel '%s' contains no nodes.", sm->GetFullName());
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for submodels with duplicate nodes
    LogAndWrite(f, "");
    LogAndWrite(f, "SubModels with duplicate nodes");

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                SubModel* sm = dynamic_cast<SubModel*>(it.second->GetSubModel(i));
                if (sm != nullptr) {
                    std::string dups = sm->GetDuplicateNodes();
                    if (dups != "") {
                        wxString msg = wxString::Format("    WARN: SubModel '%s' contains duplicate nodes: %s. This may not render as expected.", (const char*)sm->GetFullName().c_str(), (const char*)dups.c_str());
                        LogAndWrite(f, msg.ToStdString());
                        warncount++;
                    }
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for SubModels that point to nodes outside parent model name
    LogAndWrite(f, "");
    LogAndWrite(f, "SubModels with nodes not in parent model");

    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                SubModel* sm = (SubModel*)it.second->GetSubModel(i);
                if (!sm->IsNodesAllValid()) {
                    wxString msg = wxString::Format("    ERR: SubModel '%s' has invalid nodes outside the range of the parent model.",
                        sm->GetFullName());
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    if (IsCheckSequenceOptionDisabled("CustomSizeCheck")) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Custom models with excessive blank cells - CHECK DISABLED");
    }

    std::list<std::string> allfiles;

    // Check for matrix faces where the file does not exist
    LogAndWrite(f, "");
    LogAndWrite(f, "Missing matrix face images");

    for (const auto& it : AllModels) {
        auto facefiles = it.second->GetFaceFiles(std::list<std::string>(), true, true);
        allfiles.splice(allfiles.end(), it.second->GetFileReferences());

        for (const auto& fit : facefiles) {
            auto ff = wxSplit(fit, '|');
            if (!FileExists(ff[1])) {
                wxString msg = wxString::Format("    ERR: Model '%s' face '%s' image missing %s.", it.second->GetFullName(), ff[0], ff[1]);
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    // Check for large blocks of unused channels
    LogAndWrite(f, "");
    LogAndWrite(f, "Large blocks of unused channels that bloats memory usage and the the fseq file.");

    std::list<Model*> modelssorted;
    for (const auto& it : AllModels) {
        if (it.second->GetDisplayAs() != "ModelGroup") {
            modelssorted.push_back(it.second);
        }
    }

    modelssorted.sort(compare_modelstartchannel);

    int32_t last = 0;
    Model* lastm = nullptr;
    for (const auto& m : modelssorted) {
        int32_t start = m->GetNumberFromChannelString(m->ModelStartChannel);
        int32_t gap = start - last - 1;
        if (gap > 511) // 511 is the maximum acceptable gap ... at that point the user has wasted an entire universe
        {
            wxString level = "WARN";
            if (gap > 49999) // anyything 50,000 or greater should be an error
            {
                level = "ERR";
                errcount++;
            }
            else {
                warncount++;
            }
            wxString msg;
            if (lastm == nullptr) {
                msg = wxString::Format("    %s: First Model '%s' starts at channel %d leaving a block of %d of unused channels.", level, m->GetName(), start, start - 1);
            }
            else {
                msg = wxString::Format("    %s: Model '%s' starts at channel %d leaving a block of %d of unused channels between this and the prior model '%s'.", level, m->GetName(), start, gap, lastm->GetName());
            }
            LogAndWrite(f, msg.ToStdString());
        }
        long newlast = start + m->GetChanCount() - 1;
        if (newlast > last) {
            last = newlast;
            lastm = m;
        }
    }
    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }
    errcountsave = errcount;
    warncountsave = warncount;

    if (CurrentSeqXmlFile != nullptr) {
        LogAndWrite(f, "");
        LogAndWrite(f, "Uncommon and often undesirable settings");

        if (CurrentSeqXmlFile->GetRenderMode() == xLightsXmlFile::CANVAS_MODE) {
            wxString msg = wxString::Format("    WARN: Render mode set to canvas mode. Unless you specifically know you need this it is not recommended.");
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }

        if (!mSaveFseqOnSave) {
            wxString msg = wxString::Format("    WARN: Save FSEQ on save is turned off. This means every time you open the sequence you will need to render all to play your sequence. This is not recommended.");
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }

        if (IsCheckSequenceOptionDisabled("TransTime")) {
            LogAndWrite(f, "");
            LogAndWrite(f, "Effect transition times - CHECK DISABLED.");
        }

        bool dataLayer = false;
        DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
        for (int j = 0; j < data_layers.GetNumLayers(); ++j) {
            DataLayer* dl = data_layers.GetDataLayer(j);
            if (dl->GetName() != "Nutcracker") {
                dataLayer = true;
                break;
            }
        }

        if (dataLayer) {
            wxString msg = wxString::Format("    WARN: Sequence includes a data layer. There is nothing wrong with this but it is uncommon and not always intended.");
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }

        if (CurrentSeqXmlFile->GetSequenceType() == "Media") {
            LogAndWrite(f, "");
            LogAndWrite(f, "Checking media file");

            if (!FileExists(CurrentSeqXmlFile->GetMediaFile())) {
                wxString msg = wxString::Format("    ERR: media file %s does not exist.", CurrentSeqXmlFile->GetMediaFile());
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
            else {
                LogAndWrite(f, "    No problems found");
            }
        }
        errcountsave = errcount;
        warncountsave = warncount;

        LogAndWrite(f, "");
        LogAndWrite(f, "Checking autosave");

        if (CurrentSeqXmlFile->FileExists()) {
            // set to log if >1MB and autosave is more than every 10 minutes
            wxULongLong size = CurrentSeqXmlFile->GetSize();
            if (size > 1000000 && mAutoSaveInterval < 10 && mAutoSaveInterval > 0) {
                wxULongLong mbull = size / 100000;
                double mb = mbull.ToDouble() / 10.0;
                wxString msg = wxString::Format("    WARN: Sequence file size %.1fMb is large. Consider making autosave less frequent to prevent xlights pausing too often when it autosaves.", mb);
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
            else if (size < 1000000 && mAutoSaveInterval > 10) {
                wxULongLong mbull = size / 100000;
                double mb = mbull.ToDouble() / 10.0;
                wxString msg = wxString::Format("    WARN: Sequence file size %.1fMb is small. Consider making autosave more frequent to prevent loss in the event of abnormal termination.", mb);
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }

            if (errcount + warncount == errcountsave + warncountsave) {
                LogAndWrite(f, "    No problems found");
            }
            errcountsave = errcount;
            warncountsave = warncount;
        }
        else {
            LogAndWrite(f, "    Test skipped as sequence has never been saved.");
        }

        // Only warn about model hiding if model blending is turned off.
        if (!CurrentSeqXmlFile->supportsModelBlending()) {
            LogAndWrite(f, "");
            LogAndWrite(f, "Models hidden by effects on groups");

            // Check for groups that contain models that have appeared before the group at the bottom of the master view
            wxString models = _sequenceElements.GetViewModels(_sequenceElements.GetViewName(0));
            wxArrayString modelnames = wxSplit(models, ',');

            std::list<std::string> seenmodels;
            for (const auto& it : modelnames) {
                Model* m = AllModels.GetModel(it.ToStdString());
                if (m == nullptr) {
                    wxString msg = wxString::Format("    ERR: Model %s in your sequence does not seem to exist in the layout. This will need to be deleted or remapped to another model next time you load this sequence.", it);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
                else {
                    if (m->GetDisplayAs() == "ModelGroup") {
                        ModelGroup* mg = dynamic_cast<ModelGroup*>(m);
                        if (mg == nullptr) logger_base.crit("CheckSequence ModelGroup cast was null. We are about to crash.");
                        for (auto it2 : mg->Models()) {
                            if (std::find(seenmodels.begin(), seenmodels.end(), it2->GetName()) != seenmodels.end()) {
                                wxString msg = wxString::Format("    WARN: Model Group '%s' will hide effects on model '%s'.", mg->GetName(), it2->GetName());
                                LogAndWrite(f, msg.ToStdString());
                                warncount++;
                            }
                        }
                    }
                    else {
                        seenmodels.push_back(m->GetName());
                    }
                }
            }

            if (errcount + warncount == errcountsave + warncountsave) {
                LogAndWrite(f, "    No problems found");
            }
            errcountsave = errcount;
            warncountsave = warncount;
        }

        prog.Update(70, "Checking effects");
        wxYield();

        LogAndWrite(f, "");
        LogAndWrite(f, "Effect problems");

        // check all effects
        bool disabledEffects = false;
        bool videoCacheWarning = false;
        std::list<std::pair<std::string, std::string>> faces;
        std::list<std::pair<std::string, std::string>> states;
        std::list<std::string> viewPoints;
        bool usesShader = false;
        for (size_t i = 0; i < _sequenceElements.GetElementCount(MASTER_VIEW); i++) {
            Element* e = _sequenceElements.GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                CheckElement(e, f, errcount, warncount, e->GetFullName(), e->GetName(), videoCacheWarning, disabledEffects, faces, states, viewPoints, usesShader, allfiles);

                if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                    ModelElement* me = dynamic_cast<ModelElement*>(e);
                    Model* model = AllModels[me->GetModelName()];

                    for (size_t j = 0; j < me->GetStrandCount(); ++j) {
                        StrandElement* se = me->GetStrand(j);
                        CheckElement(se, f, errcount, warncount, se->GetFullName(), e->GetName(), videoCacheWarning, disabledEffects, faces, states, viewPoints, usesShader, allfiles);

                        for (size_t k = 0; k < se->GetNodeLayerCount(); ++k) {
                            NodeLayer* nl = se->GetNodeLayer(k);
                            for (size_t l = 0; l < nl->GetEffectCount(); l++) {
                                Effect* ef = nl->GetEffect(l);
                                CheckEffect(ef, f, errcount, warncount, wxString::Format("%s Strand %lu/Node %lu", se->GetFullName(), j + 1, l + 1).ToStdString(), e->GetName(), true, videoCacheWarning, disabledEffects, faces, states, viewPoints);
                                RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
                                allfiles.splice(end(allfiles), eff->GetFileReferences(model, ef->GetSettings()));
                            }
                        }
                    }
                    for (size_t j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
                        Element* sme = me->GetSubModel(j);
                        if (sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                            CheckElement(sme, f, errcount, warncount, sme->GetFullName(), e->GetName(), videoCacheWarning, disabledEffects, faces, states, viewPoints, usesShader, allfiles);
                        }
                    }
                }
            }
        }

#ifndef __WXOSX__
        if (usesShader) {
            if (mainSequencer->PanelEffectGrid->GetCreatedVersion() < 3) {
                wxString msg = wxString::Format("    ERR: Sequence has one or more shader effects but open GL version is lower than version 3 (%d). These effects will not render.", mainSequencer->PanelEffectGrid->GetCreatedVersion());
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            }
        }
#endif

        if (videoCacheWarning) {
            LogAndWrite(f, "    WARN: Sequence has one or more video effects where render caching is turned off. This will render slowly.");
            warncount++;
        }

        if (disabledEffects) {
            LogAndWrite(f, "    WARN: Sequence has one or more effects which are disabled. They are being ignored.");
            warncount++;
        }

        if (errcount + warncount == errcountsave + warncountsave) {
            LogAndWrite(f, "    No problems found");
        }
        errcountsave = errcount;
        warncountsave = warncount;

        prog.Update(90, "Dumping used assets");
        wxYield();

        LogAndWrite(f, "");
        LogAndWrite(f, "-----------------------------------------------------------------------------------------------------------------");

        LogAndWrite(f, "");
        LogAndWrite(f, "If you are planning on importing this sequence be aware the sequence relies on the following items that will not be imported.");
        LogAndWrite(f, "");
        LogAndWrite(f, "Model Faces used by this sequence:");
        for (const auto& it : faces) {
            wxString msg = wxString::Format("        Model: %s, Face: %s.", it.first, it.second);
            LogAndWrite(f, msg.ToStdString());
        }
        LogAndWrite(f, "");
        LogAndWrite(f, "Model States used by this sequence:");
        for (const auto& it : states) {
            wxString msg = wxString::Format("        Model: %s, State: %s.", it.first, it.second);
            LogAndWrite(f, msg.ToStdString());
        }
        LogAndWrite(f, "");
        LogAndWrite(f, "View Points used by this sequence:");
        for (const auto& it : viewPoints) {
            wxString msg = wxString::Format("        Viewpoint: %s.", it);
            LogAndWrite(f, msg.ToStdString());
        }

        LogAndWrite(f, "");
        LogAndWrite(f, "-----------------------------------------------------------------------------------------------------------------");

    }
    else {
        LogAndWrite(f, "");
        LogAndWrite(f, "No sequence loaded so sequence checks skipped.");
    }

    LogAndWrite(f, "");
    LogAndWrite(f, "Checking problems with file access times.");

    prog.Update(95, "Checking performance");

#define SLOWDRIVE 1000
    std::list<std::pair<std::string, uint64_t>> slowaccess;
    BadDriveAccess(allfiles, slowaccess, SLOWDRIVE);
    if (slowaccess.size() > 0) {
        wxString msg = wxString::Format("    WARN: Test of access speed to files your sequence shows the following files take longer than the recommended %dms.", SLOWDRIVE / 1000);
        LogAndWrite(f, msg.ToStdString());
        for (const auto& it : slowaccess) {
            msg = wxString::Format("    %.2fms  %s.", (float)((double)it.second / 1000.0), (const char*)it.first.c_str());
            LogAndWrite(f, msg);
        }
        warncount++;
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Checking problems with file paths containing repeated use of show folder name.");

    std::vector<char> delimiters = { '\\', '/' };
    wxString showdir = showDirectory;
    wxString sd2 = showdir.AfterLast('\\');
    wxString sd3 = showdir.AfterLast('/');
    if (sd2.Length() > 0 && sd2.Length() < showdir.Length()) showdir = sd2;
    if (sd3.Length() > 0 && sd3.Length() < showdir.Length()) showdir = sd3;

    for (const auto& it : allfiles) {
        wxString ff = FixFile(showDirectory, it);
        if (ff.StartsWith(showDirectory)) // only check files in show folder
        {
            if (FileExists(ff)) {
                ff = ff.substr(showDirectory.size());
                wxArrayString folders = Split(ff, delimiters);

                for (auto it2 : folders) {
                    if (it2 == showdir) {
                        wxString msg = wxString::Format("    WARN: path to file %s contains the show folder name '%s' more than once. This will make it hard to move sequence to other computers as it won't be able to fix paths automatically.", (const char*)it.c_str(), (const char*)showdir.c_str());
                        LogAndWrite(f, msg.ToStdString());
                        warncount++;
                    }
                }
            }
            else {
                wxString msg = wxString::Format("    WARN: Unable to check file %s because it was not found. If this location is on another computer please run check sequence there to check this condition properly.", (const char*)it.c_str());
                LogAndWrite(f, msg.ToStdString());
                warncount++;
            }
        }
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogAndWrite(f, "    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    LogAndWrite(f, "");
    LogAndWrite(f, "Check sequence done.");
    LogAndWrite(f, wxString::Format("Errors: %u. Warnings: %u", (unsigned int)errcount, (unsigned int)warncount).ToStdString());

    prog.Update(100, "Done");
    wxYield();
    prog.Hide();

    if (f.IsOpened()) {
        f.Close();

        if (displayInEditor) {
            wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
            if (ft != nullptr) {
                wxString command = ft->GetOpenCommand(filename);

                if (command == "") {
                    DisplayError(wxString::Format("Unable to show xLights Check Sequence results '%s'. See your log for the content.", filename).ToStdString(), this);
                } else {
                    logger_base.debug("Viewing xLights Check Sequence results %s. Command: '%s'", (const char*)filename.c_str(), (const char*)command.c_str());
                    wxUnsetEnv("LD_PRELOAD");
                    wxExecute(command);
                }
                delete ft;
            } else {
                DisplayError(wxString::Format("Unable to show xLights Check Sequence results '%s'. See your log for the content.", filename).ToStdString(), this);
            }
        }
    }

    return filename;
}

void xLightsFrame::ValidateEffectAssets()
{
    std::string missing;
    for (const auto& it : _sequenceElements.GetAllReferencedFiles()) {
        auto f = FixFile("", it);
        if (!FileExists(f, false)) {
            missing += it + "\n";
        }
    }

    if (missing != "") {
        wxMessageBox("Sequence references files which cannot be found:\n" + missing + "\n Use Tools/Check Sequence for more details.", "Missing assets");
    }
}

void xLightsFrame::CheckEffect(Effect* ef, wxFile& f, size_t& errcount, size_t& warncount, const std::string& name, const std::string& modelName, bool node, bool& videoCacheWarning, bool& disabledEffects, std::list<std::pair<std::string, std::string>>& faces, std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints)
{
    EffectManager& em = _sequenceElements.GetEffectManager();
    SettingsMap& sm = ef->GetSettings();

    if (ef->GetEffectName() == "Video") {
        if (_enableRenderCache == "Disabled") {
            videoCacheWarning = true;
        }
        else if (!ef->IsLocked() && _enableRenderCache == "Locked Only") {
            videoCacheWarning = true;
            wxString msg = wxString::Format("    WARN: Video effect unlocked but only locked video effects are being render cached. Effect: %s, Model: %s, Start %s", ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
    }

    if (ef->IsRenderDisabled()) disabledEffects = true;

    // check we are not doing sub-buffers on Per Model* buffer styles
    bool isPerModel = false;
    bool isSubBuffer = false;
    for (const auto& it : sm) {
        isPerModel |= (it.first == "B_CHOICE_BufferStyle" && StartsWith(it.second, "Per Model"));
        isSubBuffer |= (it.first == "B_CUSTOM_SubBuffer" && it.second != "");
    }

    if (isPerModel && isSubBuffer) {
        wxString msg = wxString::Format("    ERR: Effect on a model group using a 'Per Model' render buffer is also using a subbuffer. This will not work as you might expect. Effect: %s, Model: %s, Start %s", ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
        LogAndWrite(f, msg.ToStdString());
        errcount++;
    }

    // check value curves not updated
    for (auto it = sm.begin(); it != sm.end(); ++it) {
        wxString value = it->second;
        if (value.Contains("|Type=") && !value.Contains("RV=TRUE")) {
            int start = value.Find("|Id=") + 4;
            wxString property = value.substr(start);
            property = property.BeforeFirst('|');
            wxString msg = wxString::Format("    ERR: Effect contains very old value curve. Click on this effect and then save the sequence to convert it. Effect: %s, Model: %s, Start %s (%s)", ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()), property);
            LogAndWrite(f, msg.ToStdString());
            errcount++;
        }
    }

    // check excessive fadein/fadeout time
    float fadein = sm.GetFloat("T_TEXTCTRL_Fadein", 0.0);
    float fadeout = sm.GetFloat("T_TEXTCTRL_Fadeout", 0.0);
    float efdur = (ef->GetEndTimeMS() - ef->GetStartTimeMS()) / 1000.0;

    if (sm.Get("T_CHECKBOX_Canvas", "0") == "1") {
        // Warp and off have more complicated logic which is implemented in those effects
        if ((ef->GetEffectName() != "Off" && ef->GetEffectName() != "Warp" && ef->GetEffectName() != "Kaleidoscope" && ef->GetEffectName() != "Shader")) {
            wxString msg = wxString::Format("    WARN: Canvas mode enabled on an effect it is not normally used on. This will slow down rendering. Effect: %s, Model: %s, Start %s", ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
    }

    if (!IsCheckSequenceOptionDisabled("TransTime")) {
        if (fadein > efdur) {
            wxString msg = wxString::Format("    WARN: Transition in time %.2f on effect %s at start time %s  on Model '%s' is greater than effect duration %.2f.", fadein, ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), name, efdur);
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
        if (fadeout > efdur) {
            wxString msg = wxString::Format("    WARN: Transition out time %.2f on effect %s at start time %s  on Model '%s' is greater than effect duration %.2f.", fadeout, ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), name, efdur);
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
        if (fadein <= efdur && fadeout <= efdur && fadein + fadeout > efdur) {
            wxString msg = wxString::Format("    WARN: Transition in time %.2f + transition out time %.2f = %.2f on effect %s at start time %s  on Model '%s' is greater than effect duration %.2f.", fadein, fadeout, fadein + fadeout, ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), name, efdur);
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }
    }

    // effect that runs past end of the sequence
    if (ef->GetEndTimeMS() > CurrentSeqXmlFile->GetSequenceDurationMS()) {
        wxString msg = wxString::Format("    WARN: Effect %s ends at %s after the sequence end %s. Model: '%s' Start: %s", ef->GetEffectName(), FORMATTIME(ef->GetEndTimeMS()), FORMATTIME(CurrentSeqXmlFile->GetSequenceDurationMS()), name, FORMATTIME(ef->GetStartTimeMS()));
        LogAndWrite(f, msg.ToStdString());
        warncount++;
    }

    if (ef->GetEffectIndex() >= 0) {
        RenderableEffect* re = em.GetEffect(ef->GetEffectIndex());

        // check effect is appropriate for a node
        if (node && !re->AppropriateOnNodes()) {
            wxString msg = wxString::Format("    WARN: Effect %s at start time %s  on Model '%s' really shouldnt be used at the node level.",
                ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), name);
            LogAndWrite(f, msg.ToStdString());
            warncount++;
        }

        bool renderCache = _enableRenderCache == "Enabled" || (_enableRenderCache == "Locked Only" && ef->IsLocked());
        std::list<std::string> warnings = re->CheckEffectSettings(sm, CurrentSeqXmlFile->GetMedia(), AllModels.GetModel(modelName), ef, renderCache);
        for (const auto& s : warnings) {
            LogAndWrite(f, s);
            if (s.find("WARN:") != std::string::npos) {
                warncount++;
            }
            else if (s.find("ERR:") != std::string::npos) {
                errcount++;
            }
        }

        if (ef->GetEffectName() == "Faces") {
            for (const auto& it : static_cast<FacesEffect*>(re)->GetFacesUsed(sm)) {
                bool found = false;
                for (auto it2 : faces) {
                    if (it2.first == modelName && it2.second == it) {
                        found = true;
                    }
                }
                if (!found) {
                    faces.push_back({ modelName, it });
                }
            }
        }
        else if (ef->GetEffectName() == "State") {
            for (const auto& it : static_cast<StateEffect*>(re)->GetStatesUsed(sm)) {
                bool found = false;
                for (auto it2 : states) {
                    if (it2.first == modelName && it2.second == it) {
                        found = true;
                    }
                }
                if (!found) {
                    states.push_back({ modelName, it });
                }
            }
        }

        for (const auto& it : sm) {
            if (it.first == "B_CHOICE_PerPreviewCamera") {
                bool found = false;
                for (auto it2 : viewPoints) {
                    if (it2 == it.second) {
                        found = true;
                    }
                }
                if (!found) {
                    viewPoints.push_back(it.second);
                }
            }
        }
    }
}

void xLightsFrame::CheckElement(Element* e, wxFile& f, size_t& errcount, size_t& warncount, const std::string& name, const std::string& modelName,
                                bool& videoCacheWarning, bool& disabledEffects, std::list<std::pair<std::string, std::string>>& faces,
                                std::list<std::pair<std::string, std::string>>& states, std::list<std::string>& viewPoints, bool& usesShader,
                                std::list<std::string>& allfiles)
{
    Model* m = AllModels[modelName];

    int layer = 0;
    for (const auto& el : e->GetEffectLayers()) {
        layer++;
        for (const auto& ef : el->GetEffects()) {
            if (ef->GetEffectName() == "Random") {
                wxString msg = wxString::Format("    ERR: Effect %s (%s-%s) on Model '%s' layer %d is a random effect. This should never happen and may cause other issues.",
                                                ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), FORMATTIME(ef->GetEndTimeMS()),
                                                name, layer);
                LogAndWrite(f, msg.ToStdString());
                errcount++;
            } else {
                RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
                allfiles.splice(end(allfiles), eff->GetFileReferences(m, ef->GetSettings()));

                // Check there are nodes to actually render on
                if (m != nullptr) {
                    if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                        if (m->GetNodeCount() == 0) {
                            wxString msg = wxString::Format("    ERR: Effect %s (%s-%s) on Model '%s' layer %d Has no nodes and wont do anything.",
                                                            ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), FORMATTIME(ef->GetEndTimeMS()),
                                                            name, layer);
                            LogAndWrite(f, msg.ToStdString());
                            errcount++;
                        }
                    } else if (e->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* se = (StrandElement*)e;
                        if (m->GetStrandLength(se->GetStrand()) == 0) {
                            wxString msg = wxString::Format("    ERR: Effect %s (%s-%s) on Model '%s' layer %d Has no nodes and wont do anything.",
                                                            ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), FORMATTIME(ef->GetEndTimeMS()),
                                                            name, layer);
                            LogAndWrite(f, msg.ToStdString());
                            errcount++;
                        }
                    } else if (e->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                        Model* se = AllModels[name];
                        if (se != nullptr) {
                            if (se->GetNodeCount() == 0) {
                                wxString msg = wxString::Format("    ERR: Effect %s (%s-%s) on Model '%s' layer %d Has no nodes and wont do anything.",
                                                                ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), FORMATTIME(ef->GetEndTimeMS()),
                                                                name, layer);
                                LogAndWrite(f, msg.ToStdString());
                                errcount++;
                            }
                        }
                    }
                }

                CheckEffect(ef, f, errcount, warncount, name, modelName, false, videoCacheWarning, disabledEffects, faces, states, viewPoints);
                if (ef->GetEffectName() == "Shader") {
                    usesShader = true;
                }
            }
        }

        // This assumes effects are stored in start time order per layer
        Effect* lastEffect = nullptr;
        for (const auto& ef : el->GetEffects()) {
            if (lastEffect != nullptr) {
                // the start time of an effect should not be before the end of the prior effect
                if (ef->GetStartTimeMS() < lastEffect->GetEndTimeMS()) {
                    wxString msg = wxString::Format("    ERR: Effect %s (%s-%s) overlaps with Effect %s (%s-%s) on Model '%s' layer %d. This shouldn't be possible.",
                                                    ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), FORMATTIME(ef->GetEndTimeMS()), lastEffect->GetEffectName(), FORMATTIME(lastEffect->GetStartTimeMS()), FORMATTIME(lastEffect->GetEndTimeMS()), name, layer);
                    LogAndWrite(f, msg.ToStdString());
                    errcount++;
                }
            }

            lastEffect = ef;
        }
    }
}

void xLightsFrame::OnMenuItemCheckSequenceSelected(wxCommandEvent& event)
{
    CheckSequence(true, true);
}

void xLightsFrame::OnMenuItem_Help_ForumSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://nutcracker123.com/forum/");
}

void xLightsFrame::OnMenuItem_Help_DownloadSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://xlights.org");
}

void xLightsFrame::OnMenuItem_File_NewXLightsInstance(wxCommandEvent& event) {
#ifdef __WXOSX__
    system("open -n /Applications/xLights.app");
#endif
}

void xLightsFrame::OnMenuItem_Help_ReleaseNotesSelected(wxCommandEvent& event)
{
#ifdef __WXOSX__
    std::string loc = "https://raw.githubusercontent.com/smeighan/xLights/" + xlights_version_string + "/README.txt";
    ::wxLaunchDefaultBrowser(loc);
#else
    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (ft) {
        wxString command = ft->GetOpenCommand("README.txt");
        wxUnsetEnv("LD_PRELOAD");
        wxExecute(command);
    }
#endif
}

void xLightsFrame::OnMenuItem_Help_Isue_TrackerSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://github.com/smeighan/xLights/issues");
}

void xLightsFrame::OnMenuItem_Help_FacebookSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://www.facebook.com/groups/628061113896314/");
}

int xLightsFrame::ExportNodes(wxFile& f, StrandElement* e, NodeLayer* nl, int n, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles)
{
    Model* m = AllModels[e->GetModelName()];

    int effects = 0;
    wxString type = "Node";
    wxString name = wxString::Format("%s/%s", e->GetFullName(), m->GetNodeName(n, true));

    for (int k = 0; k < nl->GetEffectCount(); k++)
    {
        Effect* ef = nl->GetEffect(k);

        std::string fs = "";
        if (ef->GetEffectIndex() >= 0)
        {
            RenderableEffect *eff = effectManager[ef->GetEffectIndex()];
            auto files = eff->GetFileReferences(m, ef->GetSettings());

            for (auto it = files.begin(); it != files.end(); ++it)
            {
                if (fs != "")
                {
                    fs += ",";
                }
                fs += (*it);
            }

            allfiles.splice(allfiles.end(), files);
        }

        int duration = ef->GetEndTimeMS() - ef->GetStartTimeMS();
        if (effectfrequency.find(ef->GetEffectName()) != effectfrequency.end())
        {
            effectfrequency[ef->GetEffectName()]++;
            effectTotalTime[ef->GetEffectName()] += duration;
        }
        else
        {
            effectfrequency[ef->GetEffectName()] = 1;
            effectTotalTime[ef->GetEffectName()] = duration;
        }

        SettingsMap& sm = ef->GetSettings();
        f.Write(wxString::Format("\"%s\",%02d:%02d.%03d,%02d:%02d.%03d,%02d:%02d.%03d,\"%s\",\"%s\",%s,%s\n",
            ef->GetEffectName(),
            ef->GetStartTimeMS() / 60000,
            (ef->GetStartTimeMS() % 60000) / 1000,
            ef->GetStartTimeMS() % 1000,
            ef->GetEndTimeMS() / 60000,
            (ef->GetEndTimeMS() % 60000) / 1000,
            ef->GetEndTimeMS() % 1000,
            duration / 60000,
            (duration % 60000) / 1000,
            duration % 1000,
            sm.Contains("X_Effect_Description") ? sm["X_Effect_Description"] : "",
            name,
            type,
            fs
        ));
        effects++;
    }

    return effects;
}

int xLightsFrame::ExportElement(wxFile& f, Element* e, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles)
{
    int effects = 0;

    if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
    {
        Model* m = AllModels.GetModel(e->GetModelName());

        wxString type = "Unknown";
        switch (e->GetType())
        {
        case     ElementType::ELEMENT_TYPE_MODEL:
            if (m->GetDisplayAs() == "ModelGroup")
            {
                type = "Model Group";
            }
            else
            {
                type = "Model";
            }
            break;
        case ElementType::ELEMENT_TYPE_SUBMODEL:
            type = "Submodel";
            break;
        case ElementType::ELEMENT_TYPE_STRAND:
            type = "Strand";
            break;
        case ElementType::ELEMENT_TYPE_TIMING:
            type = "Timing";
            break;
        }

        for (int j = 0; j < e->GetEffectLayerCount(); j++)
        {
            EffectLayer* el = e->GetEffectLayer(j);

            for (int k = 0; k < el->GetEffectCount(); k++)
            {
                Effect* ef = el->GetEffect(k);
                std::string fs = "";
                if (ef->GetEffectIndex() >= 0)
                {
                    RenderableEffect *eff = effectManager[ef->GetEffectIndex()];
                    auto files = eff->GetFileReferences(m, ef->GetSettings());

                    for (auto it = files.begin(); it != files.end(); ++it)
                    {
                        if (fs != "")
                        {
                            fs += ",";
                        }
                        fs += (*it);
                    }

                    allfiles.splice(allfiles.end(), files);
                }

                int duration = ef->GetEndTimeMS() - ef->GetStartTimeMS();
                if (effectfrequency.find(ef->GetEffectName()) != effectfrequency.end())
                {
                    effectfrequency[ef->GetEffectName()]++;
                    effectTotalTime[ef->GetEffectName()] += duration;
                }
                else
                {
                    effectfrequency[ef->GetEffectName()] = 1;
                    effectTotalTime[ef->GetEffectName()] = duration;
                }

                SettingsMap& sm = ef->GetSettings();
                f.Write(wxString::Format("\"%s\",%02d:%02d.%03d,%02d:%02d.%03d,%02d:%02d.%03d,\"%s\",\"%s\",%s,%s\n",
                    ef->GetEffectName(),
                    ef->GetStartTimeMS() / 60000,
                    (ef->GetStartTimeMS() % 60000) / 1000,
                    ef->GetStartTimeMS() % 1000,
                    ef->GetEndTimeMS() / 60000,
                    (ef->GetEndTimeMS() % 60000) / 1000,
                    ef->GetEndTimeMS() % 1000,
                    duration / 60000,
                    (duration % 60000) / 1000,
                    duration % 1000,
                    sm.Contains("X_Effect_Description") ? sm["X_Effect_Description"] : "",
                    (const char *)(e->GetFullName()).c_str(),
                    type,
                    fs
                ));
                effects++;
            }
        }
    }

    return effects;
}

void xLightsFrame::OnMenuItem_ExportEffectsSelected(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile == nullptr)
    {
        DisplayError("No sequence open", this);
        return;
    }

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, CurrentSeqXmlFile->GetName(), wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    ExportEffects(filename);
    SetStatusText("Effects CSV saved at " + filename);
}

void xLightsFrame::ExportEffects(wxString const& filename)
{
    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString(), this);
        return;
    }

    std::map<std::string, int> effectfrequency;
    std::map<std::string, int> effecttotaltime;

    int effects = 0;
    f.Write(_("Effect Name,StartTime,EndTime,Duration,Description,Element,ElementType,Files\n"));

    std::list<std::string> files;

    for (size_t i = 0; i < _sequenceElements.GetElementCount(0); i++)
    {
        Element* e = _sequenceElements.GetElement(i);
        effects += ExportElement(f, e, effectfrequency, effecttotaltime, files);

        if (dynamic_cast<ModelElement*>(e) != nullptr)
        {
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement *se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                effects += ExportElement(f, se, effectfrequency, effecttotaltime, files);
            }
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement *se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                int node = 0;
                for (size_t n = 0; n < se->GetNodeLayerCount(); n++)
                {
                    NodeLayer* nl = se->GetNodeLayer(n);
                    effects += ExportNodes(f, se, nl, node++, effectfrequency, effecttotaltime, files);
                }
            }
        }
    }
    f.Write(wxString::Format("\"Effect Count\",%d\n", effects));
    f.Write(_("\n"));
    f.Write(_("Effect Usage Summary\n"));
    f.Write(_("Effect Name,Occurences,TotalTime\n"));
    for (auto it = effectfrequency.begin(); it != effectfrequency.end(); ++it)
    {
        int tt = effecttotaltime[it->first];
        f.Write(wxString::Format("\"%s\",%d,%02d:%02d.%03d\n",
            (const char *)it->first.c_str(),
            it->second,
            tt / 60000,
            (tt % 60000) / 1000,
            tt % 1000
        ));
    }
    f.Write(_("\n"));
    f.Write(_("Summary of files used\n"));

    files.sort();
    files.unique();
    for (auto it = files.begin(); it != files.end(); ++it)
    {
        f.Write(wxString::Format("%s\n", *it));
    }

    f.Close();
}

void xLightsFrame::OnMenuItemShiftSelectedEffectsSelected(wxCommandEvent& event)
{
    wxTextEntryDialog ted(this, "Enter the number of milliseconds to shift the selected effects:\n\n"
        "Note: Will be rounded to the nearest timing interval.\n"
        "      This operation cannot be reversed with Undo.\n"
        "      Effects shifted left may be truncated or deleted.\n"
        "      Effects that would overlap unselected effects wont be moved.",
        "Shift Selected Effects", "", wxOK | wxCANCEL | wxCENTER);
    if (ted.ShowModal() == wxID_OK) {
        int milliseconds = wxAtoi(ted.GetValue());
        if (CurrentSeqXmlFile->GetSequenceLoaded()) {
            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);
            milliseconds /= ms;
            milliseconds *= ms;
        }
        for (int elem = 0; elem<_sequenceElements.GetElementCount(MASTER_VIEW); elem++) {
            Element* ele = _sequenceElements.GetElement(elem, MASTER_VIEW);
            for (int layer = 0; layer<ele->GetEffectLayerCount(); layer++) {
                EffectLayer* el = ele->GetEffectLayer(layer);
                ShiftSelectedEffectsOnLayer(el, milliseconds);
            }
            if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL)
            {
                ModelElement *me = dynamic_cast<ModelElement *>(ele);
                for (int i = 0; i < me->GetStrandCount(); ++i)
                {
                    Element* se = me->GetStrand(i);
                    StrandElement* ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); ++k)
                    {
                        NodeLayer* nl = ste->GetNodeLayer(k, false);
                        if (nl != nullptr)
                        {
                            ShiftSelectedEffectsOnLayer(nl, milliseconds);
                        }
                    }
                }
                for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i)
                {
                    Element* se = me->GetSubModel(i);
                    for (int layer = 0; layer<se->GetEffectLayerCount(); layer++) {
                        EffectLayer* sel = se->GetEffectLayer(layer);
                        ShiftSelectedEffectsOnLayer(sel, milliseconds);
                    }
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void xLightsFrame::OnMenuItemShiftEffectsSelected(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile == nullptr) return;
    wxTextEntryDialog ted(this, "Enter the number of milliseconds to shift all effects:\n\n"
                                "Note: Will be rounded to the nearest timing interval.\n"
                                "      This operation cannot be reversed with Undo.\n"
                                "      Effects shifted left may be truncated or deleted.",
                                "Shift Effects", "", wxOK | wxCANCEL|wxCENTER);
    if( ted.ShowModal() == wxID_OK ) {
        int milliseconds = wxAtoi(ted.GetValue());
        if( CurrentSeqXmlFile->GetSequenceLoaded() ) {
            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);
            milliseconds /= ms;
            milliseconds *= ms;
        }
        for(int elem=0;elem<_sequenceElements.GetElementCount(MASTER_VIEW);elem++) {
            Element* ele = _sequenceElements.GetElement(elem, MASTER_VIEW);
            for(int layer=0;layer<ele->GetEffectLayerCount();layer++) {
                EffectLayer* el = ele->GetEffectLayer(layer);
                ShiftEffectsOnLayer(el, milliseconds);
            }
            if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL)
            {
                ModelElement *me = dynamic_cast<ModelElement *>(ele);
                for (int i = 0; i < me->GetStrandCount(); ++i)
                {
                    Element* se = me->GetStrand(i);
                    StrandElement* ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); ++k)
                    {
                        NodeLayer* nl = ste->GetNodeLayer(k, false);
                        if (nl != nullptr)
                        {
                            ShiftEffectsOnLayer(nl, milliseconds);
                        }
                    }
                }
                for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i)
                {
                    Element* se = me->GetSubModel(i);
                    for(int layer=0;layer<se->GetEffectLayerCount();layer++) {
                        EffectLayer* sel = se->GetEffectLayer(layer);
                        ShiftEffectsOnLayer(sel, milliseconds);
                    }
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void xLightsFrame::ShiftEffectsOnLayer(EffectLayer* el, int milliseconds)
{
    for(int ef=el->GetEffectCount()-1; ef >= 0; ef--) {  // count backwards so we can delete if needed
        Effect* eff = el->GetEffect(ef);
        int start_ms = eff->GetStartTimeMS();
        int end_ms = eff->GetEndTimeMS();
        if( start_ms+milliseconds < 0 ) {
            if( end_ms+milliseconds < 0 ) {
                // effect shifted off screen - delete

                el->RemoveEffect(ef);
                if (eff == selectedEffect) {
                    UnselectEffect();
                }
                continue;
            } else {
                // truncate start of effect
                eff->SetStartTimeMS(0);
            }
        } else {
            eff->SetStartTimeMS(start_ms+milliseconds);
        }
        eff->SetEndTimeMS(end_ms+milliseconds);
    }
}

void xLightsFrame::ShiftSelectedEffectsOnLayer(EffectLayer* el, int milliseconds)
{
    if (milliseconds < 0)
    {
        std::list<int> toRemove;
        for (int ef = 0; ef < el->GetEffectCount(); ef++) {
            // move left
            Effect* eff = el->GetEffect(ef);
            if (eff->GetSelected())
            {
                bool moved = false;
                int start_ms = eff->GetStartTimeMS();
                int end_ms = eff->GetEndTimeMS();
                if (start_ms + milliseconds < 0) {
                    if (end_ms + milliseconds < 0) {
                        // effect shifted off screen - delete
                        if (eff == selectedEffect) {
                            UnselectEffect();
                        }
                        // move it out of the way so it doesnt cause clashes
                        eff->SetStartTimeMS(-100);
                        eff->SetEndTimeMS(-90);
                        toRemove.push_front(ef); // we need to delete them in reverse order
                        continue;
                    }
                    else {
                        // truncate start of effect
                        eff->SetStartTimeMS(0);
                        moved = true;
                    }
                }
                else {
                    auto effectsInTime = el->GetAllEffectsByTime(start_ms + milliseconds, end_ms + milliseconds);
                    bool clash = false;
                    for (const auto& it : effectsInTime)
                    {
                        if (it->GetID() != eff->GetID())
                        {
                            clash = true;
                            break;
                        }
                    }
                    if (!clash)
                    {
                        eff->SetStartTimeMS(start_ms + milliseconds);
                        moved = true;
                    }
                }
                if (moved)
                {
                    eff->SetEndTimeMS(end_ms + milliseconds);
                }
            }
        }
        for (auto it = toRemove.begin(); it != toRemove.end(); ++it)
        {
            el->RemoveEffect(*it);
        }
    }
    else
    {
        // Move right
        for (int ef = el->GetEffectCount() - 1; ef >= 0; ef--) {  // count backwards so we can delete if needed
            Effect* eff = el->GetEffect(ef);
            if (eff->GetSelected())
            {
                bool moved = false;
                int start_ms = eff->GetStartTimeMS();
                int end_ms = eff->GetEndTimeMS();
                auto effectsInTime = el->GetAllEffectsByTime(start_ms + milliseconds, end_ms + milliseconds);
                bool clash = false;
                for (const auto& it : effectsInTime)
                {
                    if (it->GetID() != eff->GetID())
                    {
                        clash = true;
                        break;
                    }
                }
                if (!clash)
                {
                    eff->SetStartTimeMS(start_ms + milliseconds);
                    moved = true;
                }
                if (moved)
                {
                    eff->SetEndTimeMS(end_ms + milliseconds);
                }
            }
        }
    }
}

// returns the lost files path if required
std::string AddFileToZipFile(const std::string& baseDirectory, const std::string& file, wxZipOutputStream& zip, const std::string& actualfile = "")
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string filetoactuallyzip = actualfile;
    if (actualfile == "") filetoactuallyzip = file;

    std::string lost = "";
    if (FileExists(filetoactuallyzip))
    {
        wxFileName bd(baseDirectory);
        std::string showdir = bd.GetName().ToStdString();

        wxFileName fn(file);
        wxString f(file);
#ifdef __WXMSW__
        // Windows doesnt care about case so we can be more permissive
        if (f.Lower().StartsWith(wxString(baseDirectory).Lower()))
#else
        if (f.StartsWith(baseDirectory))
#endif
        {
            // this is in our folder
            std::string tgt = file.substr(baseDirectory.length());
            if (tgt != "" && (tgt[0] == '\\' || tgt[0] == '/'))
            {
                tgt = tgt.substr(1);
            }
            tgt = showdir + "/" + tgt;
            if (zip.PutNextEntry(tgt))
            {
                wxFileInputStream fis(filetoactuallyzip);
                if (fis.IsOk())
                {
                    zip.Write(fis);
                }
                else
                {
                    logger_base.warn("Error adding %s to %s due to failure to create input stream.", (const char*)file.c_str(), (const char *)tgt.c_str());
                }
                zip.CloseEntry();
            }
            else
            {
                logger_base.warn("    Error zipping %s to %s.", (const char*)file.c_str(), (const char *)tgt.c_str());
            }
        }
        else
        {
            // this isnt
            std::string tgt = "_lost/" + fn.GetName().ToStdString() + "." + fn.GetExt().ToStdString();
            tgt = showdir + "/" + tgt;
            lost = tgt;
            if (zip.PutNextEntry(tgt))
            {
                wxFileInputStream fis(filetoactuallyzip);
                zip.Write(fis);
                zip.CloseEntry();
            }
            else
            {
                logger_base.warn("    Error zipping %s to %s.", (const char*)file.c_str(), (const char *)tgt.c_str());
            }
        }
    }
    return lost;
}

std::string FixFile(const std::string& showdir, const std::string& sourcefile, const std::map<std::string, std::string>& lostfiles)
{
    std::string newfile = "";

    if (lostfiles.size() > 0)
    {
        // create a temporary file
        newfile = wxFileName::CreateTempFileName("rgbe").ToStdString();

        // read all of the existing file into memory
        wxFile in(sourcefile);
        wxString data;
        in.ReadAll(&data);
        in.Close();

        // use regex to search and replace all the lost file locations
        for (auto it = lostfiles.begin(); it != lostfiles.end(); ++it)
        {
            // strip off the show folder
            wxString replace(it->second);
            wxString newreplace = replace.AfterFirst('/');
            if (newreplace == replace)
            {
                newreplace = replace.AfterFirst('\\');
            }

            data.Replace(it->first, showdir + "/" + newreplace, true);
        }

        // write the file out
        wxFile out(newfile, wxFile::write);
        out.Write(data);
        out.Close();
    }

    return newfile;
}

std::string StripPresets(const std::string& sourcefile)
{
    std::string newfile = wxFileName::CreateTempFileName("rgbe").ToStdString();

    // read all of the existing file into memory
    wxFile in(sourcefile);
    wxString data;
    in.ReadAll(&data);
    in.Close();

    int start = data.Find("<effects version=\"");
    int end = data.Find("</effects>") + 10;

    if (end >= 10)
    {
        data = data.substr(0, start) + "<effects version=\"0006\"/>" + data.substr(end);
    }

    // write the file out
    wxFile out(newfile, wxFile::write);
    out.Write(data);
    out.Close();

    return newfile;
}

#pragma region Tools Menu

void xLightsFrame::OnMenuItem_FPP_ConnectSelected(wxCommandEvent& event)
{
    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB)
        layoutPanel->UnSelectAllModels();
    RecalcModels();

    if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
        if (wxMessageBox("Open sequence has not been saved. If you plan on uploading it to FPP then it needs to be saved. Do you want to save it now?", "Save Sequence", wxYES_NO | wxCENTRE, this) == wxYES) {
            SaveSequence();
        }
    }

    FPPConnectDialog dlg(this, &_outputManager);

    dlg.ShowModal();
}

void xLightsFrame::OnMenuItemHinksPixExportSelected(wxCommandEvent& event)
{
    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB)
        layoutPanel->UnSelectAllModels();
    RecalcModels();

    HinksPixExportDialog dlg(this, &_outputManager, &AllModels);

    dlg.ShowModal();
}

void xLightsFrame::OnMenuItem_PackageSequenceSelected(wxCommandEvent& event)
{
    PackageSequence();
}

std::string xLightsFrame::PackageSequence(bool showDialogs)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written

    if (mSavedChangeCount != _sequenceElements.GetChangeCount() && showDialogs) {
        DisplayWarning("Your sequence has unsaved changes. These changes will not be packaged but any new referenced files will be. We suggest you consider saving and trying this again.", this);
    }

    wxFileName fn(CurrentSeqXmlFile->GetFullPath());
    wxString filename = fn.GetName() + ".zip";
    wxString filePath = fn.GetPath() + wxFileName::GetPathSeparator() + filename;

    if (showDialogs) {
        wxFileDialog fd(this, "Zip file to create.", CurrentDir, filename, "zip file(*.zip)|*.zip", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (fd.ShowModal() == wxID_CANCEL) {
            return "";
        }
        filePath = fd.GetPath();
    }
    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB) {
        layoutPanel->UnSelectAllModels();
    }
    RecalcModels();

    wxFileName fnZip(filePath);
    logger_base.debug("Packaging sequence into %s.", (const char*)fnZip.GetFullPath().c_str());

    wxFFileOutputStream out(fnZip.GetFullPath());
    wxZipOutputStream zip(out);

    wxProgressDialog prog("Package Sequence", "", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();

    std::map<std::string, std::string> lostfiles;

    wxFileName fnNetworks(CurrentDir, OutputManager::GetNetworksFileName());
    prog.Update(1, fnNetworks.GetFullName());
    AddFileToZipFile(CurrentDir.ToStdString(), fnNetworks.GetFullPath().ToStdString(), zip);

    // Add house image
    wxFileName fnHouse(mBackgroundImage);
    prog.Update(5, fnHouse.GetFullName());
    auto lost = AddFileToZipFile(CurrentDir.ToStdString(), fnHouse.GetFullPath().ToStdString(), zip);
    if (lost != "") {
        lostfiles[fnHouse.GetFullPath().ToStdString()] = lost;
    }

    prog.Update(10);

    std::list<std::string> facesUsed;
    for (size_t j = 0; j < _sequenceElements.GetElementCount(0); j++) {
        Element* e = _sequenceElements.GetElement(j);
        facesUsed.splice(end(facesUsed), e->GetFacesUsed(effectManager));

        if (dynamic_cast<ModelElement*>(e) != nullptr) {
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement *se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                facesUsed.splice(end(facesUsed), se->GetFacesUsed(effectManager));
            }
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement *se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                facesUsed.splice(end(facesUsed), se->GetFacesUsed(effectManager));
            }
        }
    }
    facesUsed.sort();
    facesUsed.unique();

    // Add any model images
    std::list<std::string> modelfiles;
    for (const auto& m : AllModels) {
        modelfiles.splice(end(modelfiles), m.second->GetFaceFiles(facesUsed, false, false));
        modelfiles.splice(end(modelfiles), m.second->GetFileReferences());
    }
    for (const auto& o : AllObjects) {
        modelfiles.splice(end(modelfiles), o.second->GetFileReferences());
    }
    modelfiles.sort();
    modelfiles.unique();

    float i = 0;
    for (const auto& f : modelfiles) {
        i++;
        wxFileName fnf(f);
        if (FileExists(fnf)) {
            prog.Update(10 + (int)(10.0 * i / (float)modelfiles.size()), fnf.GetFullName());
            lost = AddFileToZipFile(CurrentDir.ToStdString(), fnf.GetFullPath().ToStdString(), zip);
            if (lost != "") {
                lostfiles[fnf.GetFullPath().ToStdString()] = lost;
            }
        } else {
            prog.Update(10 + (int)(10.0 * i / (float)modelfiles.size()));
        }
    }

    wxFileName fnRGBEffects(CurrentDir, "xlights_rgbeffects.xml");
    std::string fixfile = FixFile(CurrentDir.ToStdString(), fnRGBEffects.GetFullPath().ToStdString(), lostfiles);

    if (_excludePresetsFromPackagedSequences) {
        if (fixfile == "") {
            fixfile = StripPresets(fnRGBEffects.GetFullPath().ToStdString());
        } else {
            auto oldfile = fixfile;
            fixfile = StripPresets(fixfile);
            wxRemoveFile(oldfile);
        }
    }

    prog.Update(25, fnRGBEffects.GetFullName());
    AddFileToZipFile(CurrentDir.ToStdString(), fnRGBEffects.GetFullPath().ToStdString(), zip, fixfile);
    if (fixfile != "") {
        wxRemoveFile(fixfile);
    }

    lostfiles.clear();

    if (!_excludeAudioFromPackagedSequences) {
        // Add the media file
        wxFileName fnMedia(CurrentSeqXmlFile->GetMediaFile());
        prog.Update(30, fnMedia.GetFullName());
        lost = AddFileToZipFile(CurrentDir.ToStdString(), fnMedia.GetFullPath().ToStdString(), zip);
        if (lost != "") {
            lostfiles[fnMedia.GetFullPath().ToStdString()] = lost;
        }
        prog.Update(35, fnMedia.GetFullName());
    } else {
        prog.Update(35, "Skipping audio.");
    }

    // Add any iseq files
    DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
    for (int j = 0; j < data_layers.GetNumLayers(); ++j) {
        DataLayer* dl = data_layers.GetDataLayer(j);

        if (dl->GetName() != "Nutcracker") {
            wxFileName fndl(dl->GetDataSource());

            lost = AddFileToZipFile(CurrentDir.ToStdString(), fndl.GetFullPath().ToStdString(), zip);
            if (lost != "") {
                lostfiles[fndl.GetFullPath().ToStdString()] = lost;
            }
        }
    }

    // Add any effects images/videos/glediator files
    std::list<std::string> effectfiles;
    for (size_t j = 0; j < _sequenceElements.GetElementCount(0); j++) {
        Element* e = _sequenceElements.GetElement(j);
        Model* m = AllModels[e->GetModelName()];
        effectfiles.splice(end(effectfiles), e->GetFileReferences(m, effectManager));

        if (dynamic_cast<ModelElement*>(e) != nullptr) {
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement *se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                effectfiles.splice(end(effectfiles), se->GetFileReferences(m, effectManager));
            }
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement *se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                effectfiles.splice(end(effectfiles), se->GetFileReferences(m, effectManager));
            }
        }
    }
    effectfiles.sort();
    effectfiles.unique();

    i = 0;
    for (auto f : effectfiles) {
        i++;
        wxFileName fnf(f);
        if (FileExists(fnf)) {
            prog.Update(35 + (int)(59.0 * i / (float)effectfiles.size()), fnf.GetFullName());
            lost = AddFileToZipFile(CurrentDir.ToStdString(), fnf.GetFullPath().ToStdString(), zip);
            if (lost != "") {
                lostfiles[fnf.GetFullPath().ToStdString()] = lost;
            }
        } else {
            prog.Update(30 + (int)(64.0 * i / (float)effectfiles.size()));
        }
    }

    fixfile =  FixFile(CurrentDir.ToStdString(), CurrentSeqXmlFile->GetFullPath().ToStdString(), lostfiles);

    prog.Update(95, CurrentSeqXmlFile->GetFullName());
    AddFileToZipFile(CurrentDir.ToStdString(), CurrentSeqXmlFile->GetFullPath().ToStdString(), zip, fixfile);
    if (fixfile != "") {
        wxRemoveFile(fixfile);
    }

    if (!zip.Close()) {
        logger_base.warn("Error packaging sequence into %s.", (const char*)filePath.c_str());
    }
    out.Close();

    prog.Update(100);

    return filePath;
}

bool xLightsFrame::IsInShowFolder(const std::string& file) const
{
    wxString sf(GetShowDirectory());
    wxString f(file);

#ifdef __WXMSW__
    // windows filenames are not case sensitive
    sf.LowerCase();
    f.LowerCase();
#endif
    sf.Replace("\\", "/");
    f.Replace("\\", "/");

    if (!sf.EndsWith("/")) {
        sf += "/";
    }

    return f.StartsWith(sf);
}

#define FILES_MATCH_COMPARE 8192
bool xLightsFrame::FilesMatch(const std::string & file1, const std::string & file2) const
{
    // only equal if they both exist
    if (!FileExists(file1)) return false;
    if (!FileExists(file2)) return false;

    // and they are the same size
    wxFileName f1(file1);
    wxFileName f2(file2);
    if (f1.GetSize() != f2.GetSize()) return false;

    // and the first 8K matches byte for byte ... we could check it all but this seems reasonable
    wxByte buf1[FILES_MATCH_COMPARE];
    wxByte buf2[FILES_MATCH_COMPARE];
    memset(buf1, 0x00, sizeof(buf1));
    memset(buf1, 0x00, sizeof(buf2));

    wxFile ff1;
    if (ff1.Open(file1))
    {
        ff1.Read(buf1, sizeof(buf1));
    }

    wxFile ff2;
    if (ff2.Open(file2))
    {
        ff2.Read(buf2, sizeof(buf2));
    }

    return (memcmp(buf1, buf2, sizeof(buf1)) == 0);
}

std::string xLightsFrame::MoveToShowFolder(const std::string& file, const std::string& subdirectory)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxFileName fn(file);

    wxString target = GetShowDirectory();
    if (target.EndsWith("/") || target.EndsWith("\\"))
    {
        target = target.SubString(0, target.Length() - 2);
    }
    target += subdirectory;
    wxString dir = target;

    if (!wxDir::Exists(dir))
    {
        wxFileName d;
        if (!d.Mkdir(dir))
        {
            logger_base.error("Unable to create target folder %s.", (const char*)dir.c_str());
        }
    }

    if (target.EndsWith("/") || target.EndsWith("\\"))
    {
        target = target.SubString(0, target.Length() - 2);
    }

    target += wxFileName::GetPathSeparator();
    target += fn.GetFullName();

    int i = 1;
    while (FileExists(target) && !FilesMatch(file, target))
    {
        target = dir + wxFileName::GetPathSeparator() + fn.GetName() + "_" + wxString::Format("%d", i++) + "." + fn.GetExt();
    }

    if (!FileExists(target))
    {
        logger_base.debug("Copying file %s to %s.", (const char*)file.c_str(), (const char *)target.c_str());
        wxCopyFile(file, target, false);
    }

    return target.ToStdString();
}

void xLightsFrame::CleanupSequenceFileLocations()
{
    if (GetShowDirectory() == "") {
        wxMessageBox("Show directory invalid. Cleanup aborted.");
        return;
    }

    wxString media = CurrentSeqXmlFile->GetMediaFile();
    if (FileExists(media) && !IsInShowFolder(media))
    {
        CurrentSeqXmlFile->SetMediaFile(GetShowDirectory(), MoveToShowFolder(media, wxString(wxFileName::GetPathSeparator()) + "Audio"), false);
        _sequenceElements.IncrementChangeCount(nullptr);
    }

    bool changed = false;
    for (size_t j = 0; j < _sequenceElements.GetElementCount(0); j++)
    {
        Element* e = _sequenceElements.GetElement(j);
        changed = e->CleanupFileLocations(this, effectManager) || changed;

        if (dynamic_cast<ModelElement*>(e) != nullptr)
        {
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement *se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                changed = se->CleanupFileLocations(this, effectManager) || changed;
            }
            for (size_t s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement *se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                changed = se->CleanupFileLocations(this, effectManager) || changed;
            }
        }
    }

    if (changed)
    {
        _sequenceElements.IncrementChangeCount(nullptr);
    }
}

void xLightsFrame::CleanupRGBEffectsFileLocations()
{
    if (FileExists(mBackgroundImage) && !IsInShowFolder(mBackgroundImage))
    {
        wxString bi = MoveToShowFolder(mBackgroundImage, wxString(wxFileName::GetPathSeparator()));
        SetPreviewBackgroundImage(bi);
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
    }

    for (auto m : AllModels)
    {
        if (m.second->CleanupFileLocations(this))
        {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
        }
    }

    for (auto m : AllObjects)
    {
        if (m.second->CleanupFileLocations(this))
        {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
        }
    }
}

void xLightsFrame::OnMenuItem_CleanupFileLocationsSelected(wxCommandEvent& event)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Cleaning up file locations.");
    CleanupRGBEffectsFileLocations();
    if (CurrentSeqXmlFile != nullptr)
    {
        CleanupSequenceFileLocations();
    }
    logger_base.debug("Cleaning up file locations ... DONE.");
}

void xLightsFrame::OnMenuItem_xScheduleSelected(wxCommandEvent& event)
{
#ifdef LINUX
    // Handle xschedule not in path
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());
    wxString cmdline(appPath+wxT("/xSchedule"));
    wxExecute(cmdline, wxEXEC_ASYNC,NULL,NULL);
#else
    wxExecute("xSchedule.exe");
#endif
}

#ifdef __WXOSX__
#include "../xCapture/xCaptureMain.h"
static xCaptureFrame *xCapture = nullptr;
#endif

void xLightsFrame::OnMenuItem_xCaptureSelected(wxCommandEvent& event)
{
#ifdef LINUX
    // Handle xschedule not in path
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());
    wxString cmdline(appPath+wxT("/xCapture"));
    wxExecute(cmdline, wxEXEC_ASYNC,NULL,NULL);
#elif defined(__WXOSX__)
    if (xCapture == nullptr) {
        xCapture = new xCaptureFrame(this);
    }
    xCapture->Show();
#else
    wxExecute("xCapture.exe");
#endif
}

#pragma endregion Tools Menu

void xLightsFrame::ValidateWindow()
{
}

void xLightsFrame::TimerOutput(int period)
{
    if (CheckBoxLightOutput->IsChecked())
    {
        _outputManager.SetManyChannels(0, &_seqData[period][0], _seqData.NumChannels());
    }
}

void xLightsFrame::PlayerError(const wxString& msg)
{
    DisplayError(msg);
}

#pragma region Settings Menu

void xLightsFrame::ShowACLights()
{
    wxAuiPaneInfo& tb = MainAuiManager->GetPane(_T("ACToolbar"));
    if (tb.IsOk())
    {
        if (_showACLights)
        {
            tb.Show();
        }
        else
        {
            tb.Hide();
        }
        MainAuiManager->Update();
    }
}

void xLightsFrame::OnMenuItem_ACLIghtsSelected(wxCommandEvent& event)
{
    _showACLights = MenuItem_ACLIghts->IsChecked();
    ShowACLights();
}

void xLightsFrame::OnMenuItem_ShowACRampsSelected(wxCommandEvent& event)
{
    _showACRamps = MenuItem_ShowACRamps->IsChecked();
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::SetTimingPlayOnDClick(bool b)
{
    mTimingPlayOnDClick = b;
    mainSequencer->PanelEffectGrid->SetTimingClickPlayMode(mTimingPlayOnDClick);
}

bool xLightsFrame::IsDrawRamps()
{
    return _showACRamps;
}

#pragma endregion Settings Menu
#pragma region Help Menu

void xLightsFrame::OnMenuItem_VideoTutorialsSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://videos.xlights.org");
}

void xLightsFrame::DoDonate()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string html = "<html><body><form action = \"https://www.paypal.com/cgi-bin/webscr\" method=\"post\" id=\"paypal\">";
    html += "<input name=\"cmd\" type=\"hidden\" value=\"_s-xclick\"><input name=\"encrypted\" type=\"hidden\" ";
    html += "value=\"-----BEGIN PKCS7-----MIIHLwYJKoZIhvcNAQcEoIIHIDCCBxwCAQExggEwMIIBLAIBADCBlDCBjjELMAkGA1UEBhM";
    html += "CVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRQwEgYDVQQKEwtQYXlQYWwgSW5jLjETMBEGA1UECxQKbGl2ZV9jZXJ0czERMA8GA1UEAxQIbGl2Z";
    html += "V9hcGkxHDAaBgkqhkiG9w0BCQEWDXJlQHBheXBhbC5jb20CAQAwDQYJKoZIhvcNAQEBBQAEgYB6eVIAMC2zoeDtrWp8JDY0kg9aWdLUVR7OLuzygBndSQtvcAxs9GBKSBv";
    html += "EhIBPRyVITPHMHSWJ0sFKphFP8hv4PUHGrJ/jRVJU7Jg4fj3nmzEBykEfV2Ygx6RO7bHOjsVC7wtosSZOkLg1stWv4/9j1k5GdMSUYb5mdnApLHYegDELMAkGBSsOAwIaB";
    html += "QAwgawGCSqGSIb3DQEHATAUBggqhkiG9w0DBwQIF1t+W/JzgKyAgYi1sMjxlEOuJigFRwFXYhQVKQ5Q9iUdxeRK/jpT6dVobbQRw1OtQLKl+LGcJvonJLiFTzAh/O95b2/";
    html += "1OTdNM161soQlUAt/8vbDGkQFVjLlO/C68a1a2pSXEUWYX1CVbb5UT/6wzuJFSZbfl86gCVT1Vv+pyj2+SjFVau/rdMpO9MKyNukXHTDFoIIDhzCCA4MwggLsoAMCAQICA";
    html += "QAwDQYJKoZIhvcNAQEFBQAwgY4xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEWMBQGA1UEBxMNTW91bnRhaW4gVmlldzEUMBIGA1UEChMLUGF5UGFsIEluYy4xEzARBgN";
    html += "VBAsUCmxpdmVfY2VydHMxETAPBgNVBAMUCGxpdmVfYXBpMRwwGgYJKoZIhvcNAQkBFg1yZUBwYXlwYWwuY29tMB4XDTA0MDIxMzEwMTMxNVoXDTM1MDIxMzEwMTMxNVowg";
    html += "Y4xCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTEWMBQGA1UEBxMNTW91bnRhaW4gVmlldzEUMBIGA1UEChMLUGF5UGFsIEluYy4xEzARBgNVBAsUCmxpdmVfY2VydHMxETA";
    html += "PBgNVBAMUCGxpdmVfYXBpMRwwGgYJKoZIhvcNAQkBFg1yZUBwYXlwYWwuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDBR07d/ETMS1ycjtkpkvjXZe9k+6Cie";
    html += "LuLsPumsJ7QC1odNz3sJiCbs2wC0nLE0uLGaEtXynIgRqIddYCHx88pb5HTXv4SZeuv0Rqq4+axW9PLAAATU8w04qqjaSXgbGLP3NmohqM6bV9kZZwZLR/klDaQGo1u9uD";
    html += "b9lr4Yn+rBQIDAQABo4HuMIHrMB0GA1UdDgQWBBSWn3y7xm8XvVk/UtcKG+wQ1mSUazCBuwYDVR0jBIGzMIGwgBSWn3y7xm8XvVk/UtcKG+wQ1mSUa6GBlKSBkTCBjjELM";
    html += "AkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRYwFAYDVQQHEw1Nb3VudGFpbiBWaWV3MRQwEgYDVQQKEwtQYXlQYWwgSW5jLjETMBEGA1UECxQKbGl2ZV9jZXJ0czERMA8GA1U";
    html += "EAxQIbGl2ZV9hcGkxHDAaBgkqhkiG9w0BCQEWDXJlQHBheXBhbC5jb22CAQAwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCBXzpWmoBa5e9fo6ujionW1hUhP";
    html += "kOBakTr3YCDjbYfvJEiv/2P+IobhOGJr85+XHhN0v4gUkEDI8r2/rNk1m0GA8HKddvTjyGw/XqXa+LSTlDYkqI8OwR8GEYj4efEtcRpRYBxV8KxAW93YDWzFGvruKnnLbD";
    html += "AF6VR5w/cCMn5hzGCAZowggGWAgEBMIGUMIGOMQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExFjAUBgNVBAcTDU1vdW50YWluIFZpZXcxFDASBgNVBAoTC1BheVBhbCBJb";
    html += "mMuMRMwEQYDVQQLFApsaXZlX2NlcnRzMREwDwYDVQQDFAhsaXZlX2FwaTEcMBoGCSqGSIb3DQEJARYNcmVAcGF5cGFsLmNvbQIBADAJBgUrDgMCGgUAoF0wGAYJKoZIhvc";
    html += "NAQkDMQsGCSqGSIb3DQEHATAcBgkqhkiG9w0BCQUxDxcNMTUwMzIyMDcwMTM5WjAjBgkqhkiG9w0BCQQxFgQUS+bqsAykJyPDOSftCR69oXQRd6YwDQYJKoZIhvcNAQEBB";
    html += "QAEgYCfmPNOECi2mAVRxYEDVYWJ/QxrX5dvMmrcHC1/0Eb2X89pdO+2pDwuI1uzZ6h1In4UiBJPwVNzxCHUOniej7CQ+xHfo87M/Pb0+9LD9GZYSQbnRP5qs4/FImWV6k2";
    html += "9HKecWmJdow3/AP97eoVFQ4iD1aq7vVl4vdzB6yrC1bNj4Q==-----END PKCS7-----\"><input alt = \"PayPal - The safer, easier way to pay online!\" ";
    html += "name=\"submit\" src=\"https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif\" type=\"image\"><img style=\"display: none ! important;\" ";
    html += "hidden=\"\" src=\"https://www.paypalobjects.com/en_US/i/scr/pixel.gif\" alt=\"\" width=\"1\" height=\"1\">";
    html += "</form><script>document.getElementById(\"paypal\").submit();</script></body></html>";

    wxString filename = wxFileName::CreateTempFileName("Donate") + ".html";

    wxFile f;
    if (f.Create(filename, true))
    {
        f.Write(html);
        f.Close();
        ::wxLaunchDefaultBrowser("file://" + filename);

        // this is a bit dodgy ... basically I wait three seconds and then delete the temporary file.
        // To keep the app responsive I yield frequently
        for (int i = 0; i < 300; ++i)
        {
            wxYield();
            wxMilliSleep(10);
        }

        wxRemoveFile(filename);
    }
    else
    {
        logger_base.error("Unable to create temp file %s.", (const char *)filename.c_str());
    }
}

void xLightsFrame::OnMenuItem_DonateSelected(wxCommandEvent& event)
{
#ifdef __WXOSX__
    DoInAppPurchases(this);
    //::wxLaunchDefaultBrowser("https://xlights.org");
#else
    DoDonate();
#endif
}

#pragma endregion Help Menu

#pragma region AC Sequencing

void xLightsFrame::OnAC_OnClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACOn->SetValue(true);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
    {
        Button_ACIntensity->SetValue(true);
    }
    Button_ACSelect->SetValue(false);

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_OffClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(true);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(false);
    Button_ACCascade->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACSelect->SetValue(false);

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::UpdateACToolbar(bool forceState)
{
    if (Button_ACDisabled->IsChecked() && _seqData.NumFrames() != 0 && _showACLights && forceState)
    {
        wxAuiToolBarItem* button = ACToolbar->FindTool(ID_AUITOOLBARITEM_ACON);
        int state = button->GetState();
        if (state & wxAUI_BUTTON_STATE_DISABLED)
        {
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACSELECT, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACON, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACTWINKLE, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACSHIMMER, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACINTENSITY, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUP, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPDOWN, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUPDOWN, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACCASCADE, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACFILL, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACFOREGROUND, true);
            EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACBACKGROUND, true);
        }
        ChoiceParm1->Enable(true);
        if (Button_ACIntensity->IsChecked())
        {
            ChoiceParm2->Enable(false);
        }
        else
        {
            ChoiceParm2->Enable(true);
        }
    }
    else
    {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACSELECT, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACON, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACTWINKLE, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACSHIMMER, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACINTENSITY, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUP, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPDOWN, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACRAMPUPDOWN, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACCASCADE, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACFILL, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACFOREGROUND, false);
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACBACKGROUND, false);
        ChoiceParm1->Enable(false);
        ChoiceParm2->Enable(false);
    }
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::OnAC_DisableClick(wxCommandEvent& event)
{
    UpdateACToolbar();
    if (Button_ACDisabled->IsChecked() && _seqData.NumFrames() != 0 && _showACLights) {
        ACToolbar->SetToolBitmap(ID_AUITOOLBARITEM_ACDISABLED, GetToolbarBitmapBundle("xlAC_ENABLED"));
        Button_ACSelect->SetValue(true);
        Button_ACIntensity->SetValue(true);

        if (mainSequencer->PanelEffectGrid->GetActiveTimingElement() == nullptr || mainSequencer->PanelEffectGrid->GetActiveTimingElement()->GetEffectLayer(0)->GetEffectCount() == 0) {
            wxMessageBox("You need a timing track selected and timing marks in order to use AC mode. Ideally a timing track with lots of timing marks as you can only place effects between those marks.", "Warning", 5L, this);
        }
    } else {
        ACToolbar->SetToolBitmap(ID_AUITOOLBARITEM_ACDISABLED, GetToolbarBitmapBundle("xlAC_DISABLED"));
    }
    UpdateACToolbar();
    //MainAuiManager->Update();
    EnableSequenceControls(true);
}

#pragma endregion AC Sequencing

void xLightsFrame::OnACToolbarDropdown(wxAuiToolBarEvent& event)
{
}

void xLightsFrame::OnAC_ShimmerClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(true);
    Button_ACSelect->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
    {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_TwinkleClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(true);
    Button_ACShimmer->SetValue(false);
    Button_ACSelect->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
    {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

bool xLightsFrame::IsACActive()
{
    return _seqData.NumFrames() != 0 && _showACLights && Button_ACDisabled->IsChecked();
}

void xLightsFrame::OnAC_BackgroundClick(wxCommandEvent& event)
{
    Button_ACForeground->SetValue(false);
}

void xLightsFrame::OnAC_ForegroundClick(wxCommandEvent& event)
{
    Button_ACBackground->SetValue(false);
}

void xLightsFrame::OnAC_CascadeClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    if (!Button_ACCascade->IsChecked())
    {
        if (!Button_ACOn->IsChecked() && !Button_ACOff->IsChecked() && !Button_ACTwinkle->IsChecked() && !Button_ACShimmer->IsChecked())
        {
            Button_ACSelect->SetValue(true);
        }
        if (!Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
        {
            Button_ACIntensity->SetValue(true);
        }
    }
    else
    {
        Button_ACIntensity->SetValue(false);
        Button_ACRampUp->SetValue(false);
        Button_ACRampDown->SetValue(false);
        Button_ACRampUpDown->SetValue(false);
        Button_ACFill->SetValue(false);
        Button_ACForeground->SetValue(false);
        Button_ACBackground->SetValue(false);
    }

    if (Button_ACFill->IsChecked())
    {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, false);
    }
    else
    {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, true);
    }

    if (Button_ACCascade->IsChecked() && wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_FillClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    if (Button_ACFill->IsChecked())
    {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, false);
        Button_ACForeground->SetValue(false);
        Button_ACBackground->SetValue(false);
    }
    else
    {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, true);
    }

    Button_ACCascade->SetValue(false);

    if (!Button_ACOn->IsChecked() && !Button_ACTwinkle->IsChecked() && !Button_ACShimmer->IsChecked())
    {
        if (Button_ACFill->IsChecked())
        {
            Button_ACOn->SetValue(true);
        }
        else
        {
            if (!Button_ACOff->IsChecked())
            {
                Button_ACSelect->SetValue(true);
            }
        }
    }

    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
    {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect && Button_ACFill->IsChecked())
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampUpDownClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(true);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked())
    {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampUpDown));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampUpDown));

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampDownClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(true);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked())
    {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampDown));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampDown));

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampUpClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(true);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked())
    {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampUp));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampUp));

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_IntensityClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked())
    {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(true);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked())
    {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(false);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1Intensity));

    if (wasSelect)
    {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr)
        {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::GetACSettings(ACTYPE& type, ACSTYLE& style, ACTOOL& tool, ACMODE& mode)
{
    if (Button_ACSelect->IsChecked())
    {
        type = ACTYPE::SELECT;
    }
    else if (Button_ACOn->IsChecked())
    {
        type = ACTYPE::ON;
    }
    else if (Button_ACOff->IsChecked())
    {
        type = ACTYPE::OFF;
    }
    else if (Button_ACTwinkle->IsChecked())
    {
        type = ACTYPE::TWINKLE;
    }
    else if (Button_ACShimmer->IsChecked())
    {
        type = ACTYPE::SHIMMER;
    }

    if (Button_ACIntensity->IsChecked())
    {
        style = ACSTYLE::INTENSITY;
    }
    else if (Button_ACRampUp->IsChecked())
    {
        style = ACSTYLE::RAMPUP;
    }
    else if (Button_ACRampDown->IsChecked())
    {
        style = ACSTYLE::RAMPDOWN;
    }
    else if (Button_ACRampUpDown->IsChecked())
    {
        style = ACSTYLE::RAMPUPDOWN;
    }

    tool = ACTOOL::TOOLNIL;
    if (Button_ACFill->IsChecked())
    {
        tool = ACTOOL::FILL;
    }
    else if (Button_ACCascade->IsChecked())
    {
        tool = ACTOOL::CASCADE;
    }

    mode = ACMODE::MODENIL;
    if (Button_ACForeground->IsChecked())
    {
        mode = ACMODE::FOREGROUND;
    }
    else if (Button_ACBackground->IsChecked())
    {
        mode = ACMODE::BACKGROUND;
    }
}

int xLightsFrame::GetACIntensity()
{
    return _acParm1Intensity;
}

void xLightsFrame::GetACRampValues(int& a, int& b)
{
    a = 0;
    b = 100;
    if (Button_ACRampUp->IsChecked())
    {
        a = std::min(_acParm1RampUp, _acParm2RampUp);
        b = std::max(_acParm1RampUp, _acParm2RampUp);
    }
    else if (Button_ACRampDown->IsChecked())
    {
        a = std::max(_acParm1RampDown, _acParm2RampDown);
        b = std::min(_acParm1RampDown, _acParm2RampDown);
    }
    else if (Button_ACRampUpDown->IsChecked())
    {
        a = _acParm1RampUpDown;
        b = _acParm2RampUpDown;
    }
}

void xLightsFrame::OnChoiceParm1Select(wxCommandEvent& event)
{
    if (Button_ACIntensity->IsChecked())
    {
        _acParm1Intensity = wxAtoi(ChoiceParm1->GetStringSelection());
    }
    else if (Button_ACRampUp->IsChecked())
    {
        _acParm1RampUp = wxAtoi(ChoiceParm1->GetStringSelection());
    }
    else if (Button_ACRampDown->IsChecked())
    {
        _acParm1RampDown = wxAtoi(ChoiceParm1->GetStringSelection());
    }
    else if (Button_ACRampUpDown->IsChecked())
    {
        _acParm1RampUpDown = wxAtoi(ChoiceParm1->GetStringSelection());
    }
}

void xLightsFrame::OnChoiceParm2Select(wxCommandEvent& event)
{
    if (Button_ACRampUp->IsChecked())
    {
        _acParm2RampUp = wxAtoi(ChoiceParm2->GetStringSelection());
    }
    else if (Button_ACRampDown->IsChecked())
    {
        _acParm2RampDown = wxAtoi(ChoiceParm2->GetStringSelection());
    }
    else if (Button_ACRampUpDown->IsChecked())
    {
        _acParm2RampUpDown = wxAtoi(ChoiceParm2->GetStringSelection());
    }
}

void xLightsFrame::OnAC_SelectClick(wxCommandEvent& event)
{
    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(false);
    Button_ACSelect->SetValue(true);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
    {
        Button_ACIntensity->SetValue(true);
    }
}

void xLightsFrame::SetACSettings(ACTOOL tool)
{
    wxCommandEvent event;
    switch (tool)
    {
    case ACTOOL::CASCADE:
        Button_ACCascade->SetValue(true);
        OnAC_CascadeClick(event);
        break;
    case ACTOOL::FILL:
        Button_ACFill->SetValue(true);
        OnAC_FillClick(event);
        break;
    default:
        break;
    }

    if (Button_ACSelect->IsChecked() || Button_ACOff->IsChecked())
    {
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
        Button_ACOff->SetValue(false);
    }
    ACToolbar->Refresh();
}

void xLightsFrame::SetACSettings(ACSTYLE style)
{
    if (Button_ACSelect->IsChecked() || Button_ACOff->IsChecked())
    {
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
        Button_ACOff->SetValue(false);
    }

    wxCommandEvent event;
    switch (style)
    {
    case ACSTYLE::INTENSITY:
        Button_ACIntensity->SetValue(true);
        OnAC_IntensityClick(event);
        break;
    case ACSTYLE::RAMPUP:
        Button_ACRampUp->SetValue(true);
        OnAC_RampUpClick(event);
        break;
    case ACSTYLE::RAMPDOWN:
        Button_ACRampDown->SetValue(true);
        OnAC_RampDownClick(event);
        break;
    case ACSTYLE::RAMPUPDOWN:
        Button_ACRampUpDown->SetValue(true);
        OnAC_RampUpDownClick(event);
        break;
    default:
        break;
    }
    ACToolbar->Refresh();
}

void xLightsFrame::SetACSettings(ACMODE mode)
{
    wxCommandEvent event;
    switch (mode)
    {
    case ACMODE::FOREGROUND:
        Button_ACForeground->SetValue(true);
        OnAC_ForegroundClick(event);
        break;
    case ACMODE::BACKGROUND:
        Button_ACBackground->SetValue(true);
        OnAC_BackgroundClick(event);
        break;
    default:
        break;
    }
    ACToolbar->Refresh();
}

void xLightsFrame::SetACSettings(ACTYPE type)
{
    wxCommandEvent event;
    switch (type)
    {
    case ACTYPE::SELECT:
        Button_ACSelect->SetValue(true);
        OnAC_SelectClick(event);
        break;
    case ACTYPE::OFF:
        Button_ACOff->SetValue(true);
        OnAC_OffClick(event);
        break;
    case ACTYPE::ON:
        Button_ACOn->SetValue(true);
        OnAC_OnClick(event);
        break;
    case ACTYPE::SHIMMER:
        Button_ACShimmer->SetValue(true);
        OnAC_ShimmerClick(event);
        break;
    case ACTYPE::TWINKLE:
        Button_ACTwinkle->SetValue(true);
        OnAC_TwinkleClick(event);
        break;
    default:
        break;
    }

    if (Button_ACOn->IsChecked() || Button_ACShimmer->IsChecked() || Button_ACTwinkle->IsChecked())
    {
        if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked())
        {
            Button_ACIntensity->SetValue(true);
        }
    }
    ACToolbar->Refresh();
}


void xLightsFrame::OnMenuItem_PerspectiveAutosaveSelected(wxCommandEvent& event)
{
    _autoSavePerspecive = MenuItem_PerspectiveAutosave->IsChecked();
}

int xLightsFrame::SuppressDuplicateFrames() const {
    return _outputManager.GetSuppressFrames();
}

void xLightsFrame::SetSuppressDuplicateFrames(int i) {
    _outputManager.SetSuppressFrames(i);
    NetworkChange();
}


void xLightsFrame::SetPlayControlsOnPreview(bool b)
{
    _playControlsOnPreview = b;
    _housePreviewPanel->SetToolbar(_playControlsOnPreview);
    _housePreviewPanel->PostSizeEvent();
}

void xLightsFrame::SetAutoShowHousePreview(bool b)
{
    _autoShowHousePreview = b;
}

bool xLightsFrame::IsPaneDocked(wxWindow* window) const
{
    if (m_mgr == nullptr) return true;

    return m_mgr->GetPane(window).IsDocked();
}

ModelPreview* xLightsFrame::GetHousePreview() const
{
    return _housePreviewPanel->GetModelPreview();
}

void xLightsFrame::OnMenuItem_GenerateLyricsSelected(wxCommandEvent& event)
{
    GenerateLyricsDialog dlg(this, _seqData.NumChannels());

    if (dlg.ShowModal() == wxID_OK)
    {
        std::map<std::string, std::list<long>> face; // these channels need to be set to not zero when this phenome is displayed

        std::list<long> data = dlg.GetChannels("AI");
        if (data.size() > 0) face["AI"] = data;
        std::list<long> all = data; // this is all the channels used by one or more phenomes
        data = dlg.GetChannels("E");
        if (data.size() > 0) face["E"] = data;
        all.merge(data);
        data = dlg.GetChannels("etc");
        if (data.size() > 0) face["etc"] = data;
        all.merge(data);
        data = dlg.GetChannels("FV");
        if (data.size() > 0) face["FV"] = data;
        all.merge(data);
        data = dlg.GetChannels("L");
        if (data.size() > 0) face["L"] = data;
        all.merge(data);
        data = dlg.GetChannels("MBP");
        if (data.size() > 0) face["MBP"] = data;
        all.merge(data);
        data = dlg.GetChannels("O");
        if (data.size() > 0) face["O"] = data;
        all.merge(data);
        data = dlg.GetChannels("rest");
        if (data.size() > 0) face["rest"] = data;
        all.merge(data);
        data = dlg.GetChannels("U");
        if (data.size() > 0) face["U"] = data;
        all.merge(data);
        data = dlg.GetChannels("WQ");
        if (data.size() > 0) face["WQ"] = data;
        all.merge(data);
        all.unique();

        std::map<std::string, std::list<long>> notface; // this is the list of channels that must be zero for a given phenome
        for (auto it = face.begin(); it != face.end(); ++it)
        {
            std::list<long> notdata;

            for (auto it2 = all.begin(); it2 != all.end(); ++it2)
            {
                if (std::find(it->second.begin(), it->second.end(), *it2) == it->second.end())
                {
                    notdata.push_back(*it2);
                }
            }
            notface[it->first] = notdata;
        }

        // now create the phenome timing track
        std::string name = _sequenceElements.UniqueElementName(dlg.GetLyricName());
        int timingCount = _sequenceElements.GetNumberOfTimingElements();
        Element* e = _sequenceElements.AddElement(timingCount, name, "timing", true, false, true, false, false);
        _sequenceElements.AddTimingToCurrentView(name);
        TimingElement* timing = dynamic_cast<TimingElement*>(e);
        timing->AddEffectLayer();
        timing->AddEffectLayer();
        EffectLayer* tl = timing->AddEffectLayer();

        Effect* lastEffect = nullptr;
        std::string lastPhenome = "";

        for (size_t i = 0; i < _seqData.NumFrames(); ++i)
        {
            bool phenomeFound = false;
            for (auto it = face.begin(); it != face.end(); ++it)
            {
                bool match = true;
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    if (_seqData[i][*it2-1] == 0)
                    {
                        match = false;
                        break;
                    }
                }
                for (auto it2 = notface[it->first].begin(); it2 != notface[it->first].end(); ++it2)
                {
                    if (_seqData[i][*it2-1] != 0)
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    if (lastEffect != nullptr && lastPhenome == it->first)
                    {
                        lastEffect->SetEndTimeMS(lastEffect->GetEndTimeMS() + CurrentSeqXmlFile->GetFrameMS());
                    }
                    else
                    {
                        long start = i * CurrentSeqXmlFile->GetFrameMS();
                        lastEffect = tl->AddEffect(0, it->first, "", "", start, start + CurrentSeqXmlFile->GetFrameMS(), false, false);
                        lastPhenome = it->first;
                    }
                    phenomeFound = true;

                    break;
                }
            }

            if (!phenomeFound)
            {
                lastPhenome = "";
                lastEffect = nullptr;
            }
        }

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(this, eventRowHeaderChanged);
    }
}

void xLightsFrame::OnMenuItem_CrashXLightsSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("^^^^^ xLights crashing on purpose ... bye bye cruel world.");
    int *p = nullptr;
    *p = 0xFFFFFFFF;
}

void xLightsFrame::OnMenuItemBatchRenderSelected(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    BatchRenderDialog dlg(this);
    if (dlg.Prepare(this->GetShowDirectory()) && dlg.ShowModal() == wxID_OK && CloseSequence()) {
        wxArrayString files = dlg.GetFileList();
        wxArrayString filesToRender;
        for (auto f : files) {
            wxFileName fname(this->GetShowDirectory() + wxFileName::GetPathSeparator() + f);
            if (FileExists(fname))
                filesToRender.push_back(fname.GetFullPath());
            else
                logger_base.info("BatchRender: Sequence File not Found: %s.", (const char*)fname.GetFullPath().c_str());
        }
        if (filesToRender.size() > 0) {
            _renderMode = true;
            _saveLowDefinitionRender = _lowDefinitionRender;
            if (dlg.CheckBox_ForceHighDefinition->IsChecked()) {
                _lowDefinitionRender = false;
            }
            OpenRenderAndSaveSequences(filesToRender, false);
            if (filesToRender.size() == 0) {
                _lowDefinitionRender = _saveLowDefinitionRender;
                _renderMode = false;
            }
        } else {
            logger_base.info("BatchRender: No Sequences Selected.");
        }
    }
}

void xLightsFrame::OnMenuItem_UpdateSelected(wxCommandEvent& event)
{
    bool update_found = CheckForUpdate(3, false, true);
    if (!update_found) {
        DisplayInfo("Update check complete: No update found", this);
    }
}

bool xLightsFrame::CheckForUpdate(int maxRetries, bool canSkipUpdates, bool showMessageBoxes)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool found_update = false;
#ifdef LINUX
    wxString hostname = wxT("www.adebenham.com");
    wxString path = wxT("/wp-content/uploads/xlights/latest.php");
    wxString downloadUrl = wxT("https://github.com/smeighan/xLights/releases/latest");
    MenuItem_Update->Enable(true);
#else
#ifdef  __WXOSX__
    wxString hostname = _T("dankulp.com");
    wxString path = _T("/xLightsLatest.php");
    wxString downloadUrl = wxT("http://dankulp.com/xlights/");
    if (MenuItem_Update) MenuItem_Update->Enable(true);
#else
    wxString hostname = _T("xlights.org");

    //wxString path = _T("/downloads/");
    wxString downloadUrl = wxT("https://xlights.org/downloads/");
    wxString path = _T("/releases/");
    //wxString downloadUrl = wxT("https://xlights.org/releases/");

    logger_base.debug("Downloading %s", (const char*)downloadUrl.c_str());
    MenuItem_Update->Enable(true);
#endif
#endif

    wxHTTP get;
    get.SetTimeout(5); // 5 seconds of timeout instead of 10 minutes ...
    get.SetHeader("Cache-Control", "no-cache");

    bool didConnect = false;

    for (int retry = 0; retry < maxRetries; retry++) {
        logger_base.debug("Attempting version update check %d/%d...", retry + 1, maxRetries);
        if (get.Connect(hostname)) {
            didConnect = true;
            break;
        } else {
            // If another retry is possible, sleep for N seconds
            // This avoids overloading the remote server with repeat requests
            if (retry < maxRetries - 1) {
                wxSleep(3);
            }
        }
    }

    if (!didConnect) {
        logger_base.debug("Version update check failed. Unable to connect.");
        if (showMessageBoxes) {
            wxMessageBox("Unable to connect.", "Version update check failed");
        }
        return true;
    }

    wxInputStream *httpStream = get.GetInputStream(path);
    if (get.GetError() == wxPROTO_NOERR) {
        wxString res;
        wxString configver = wxT("");
        wxStringOutputStream out_stream(&res);
        httpStream->Read(out_stream);

#ifdef __WXMSW__
        wxString page = wxString(out_stream.GetString());

        //logger_base.debug("    Download page: %s",
        //    (const char *)page.c_str());

        // find the highest version number in the file
        wxString urlVersion = xlights_version_string;

        wxRegEx reVersion("xLights[0-9][0-9]_(2[0-9][0-9][0-9]_[0-9][0-9])\\.exe", wxRE_ADVANCED | wxRE_NEWLINE);
        while (reVersion.Matches(page))
        {
            auto v = reVersion.GetMatch(page, 1);
            size_t start = -1;
            size_t len = -1;
            reVersion.GetMatch(&start, &len, 1);
            v.Replace("_", ".");

            //logger_base.debug("    Found Version: %s",
            //    (const char *)v.c_str());

            if (IsVersionOlder(v, urlVersion))
            {
                urlVersion = v;
            }
            page = page.Mid(start + len);
        }

        wxString dlv = urlVersion;
        dlv.Replace(".", "_");
        wxString bit = GetBitness();
        bit.Replace("bit", "");
        downloadUrl = downloadUrl + "xLights" + bit + "_" + dlv + ".exe";
#else
        wxRegEx reVersion("^.*(2[0-9][0-9][0-9]\\.[0-9]*\\.?[0-9]?)[a-z]?[\\.-].*$");
        wxString urlVersion = wxString(out_stream.GetString());
        reVersion.Replace(&urlVersion, "\\1", 1);
#endif

        wxConfigBase* config = wxConfigBase::Get();
        if (canSkipUpdates && (config != nullptr))
        {
            config->Read("SkipVersion", &configver);
        }

        logger_base.debug("Current Version: '%s'. Latest Available '%s'. Skip Version '%s'.",
            (const char *)xlights_version_string.c_str(),
            (const char *)urlVersion.c_str(),
            (const char *)configver.c_str());

        if ((!urlVersion.Matches(configver))
            && (!urlVersion.Matches(xlights_version_string))
            && IsVersionOlder(urlVersion, xlights_version_string)) {
            found_update = true;
            UpdaterDialog *dialog = new UpdaterDialog(this);

            dialog->urlVersion = urlVersion;
            dialog->downloadUrl = downloadUrl;
            dialog->StaticTextUpdateLabel->SetLabel(wxT("You are currently running xLights "
                + xlights_version_string + "\n"
                + "Whereas the current release is " + urlVersion));
            dialog->Show();
        }
    }
    else {
        logger_base.debug("Version update check failed. Unable to read available versions.");
        if (showMessageBoxes) {
            wxMessageBox("Unable to read available versions.", "Version update check failed");
        }
    }

    wxDELETE(httpStream);
    get.Close();
    return found_update;
}

void xLightsFrame::SetSmallWaveform(bool b)
{
    _smallWaveform = b;
    if (_smallWaveform) {
        mainSequencer->SetSmallWaveform();
    } else {
        mainSequencer->SetLargeWaveform();
    }
}

void xLightsFrame::OnMenuItem_LogRenderStateSelected(wxCommandEvent& event)
{
    LogRenderStatus();
}

void xLightsFrame::SaveCurrentTab()
{
    switch (Notebook1->GetSelection()) {
    case SETUPTAB:
        SaveNetworksFile();
        if (IsControllersAndLayoutTabSaveLinked()) {
            layoutPanel->SaveEffects();
        }
        break;
    case LAYOUTTAB:
        layoutPanel->SaveEffects();
        if (IsControllersAndLayoutTabSaveLinked()) {
            SaveNetworksFile();
        }
        break;
    case NEWSEQUENCER:
        SaveSequence();
        break;
    default:
        break;
    }
}

void xLightsFrame::OnMenuItem_File_Save_Selected(wxCommandEvent& event)
{
    SaveCurrentTab();
}

void xLightsFrame::SetSnapToTimingMarks(bool b)
{
    _snapToTimingMarks = b;
}

void xLightsFrame::PurgeDownloadCache()
{
    VendorModelDialog::GetCache().ClearCache();
    VendorModelDialog::GetCache().Save();
    VendorMusicDialog::GetCache().ClearCache();
    VendorMusicDialog::GetCache().Save();
    ShaderDownloadDialog::GetCache().ClearCache();
    ShaderDownloadDialog::GetCache().Save();
}

bool xLightsFrame::GetRecycleTips() const
{
    wxConfigBase* config = wxConfigBase::Get();
    return config->Read("OnlyShowUnseenTips", true);
}

void xLightsFrame::SetRecycleTips(bool b)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("OnlyShowUnseenTips", b);
}

void xLightsFrame::OnMenuItem_PurgeVendorCacheSelected(wxCommandEvent& event)
{
    PurgeDownloadCache();
}

void xLightsFrame::OnMenuItem_LoudVolSelected(wxCommandEvent& event)
{
    playVolume = 100;
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_MedVolSelected(wxCommandEvent& event)
{
    playVolume = 66;
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_QuietVolSelected(wxCommandEvent& event)
{
    playVolume = 33;
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_VQuietVolSelected(wxCommandEvent& event)
{
    playVolume = 10;
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::ShowPresetsPanel()
{
    if (CurrentSeqXmlFile == nullptr) return;

    if (EffectTreeDlg == nullptr)
    {
        EffectTreeDlg = new EffectTreeDialog(this);
        EffectTreeDlg->InitItems(_sequenceElements.GetEffectsNode());
    }
    EffectTreeDlg->Show();
}

uint64_t xLightsFrame::BadDriveAccess(const std::list<std::string>& files, std::list<std::pair<std::string, uint64_t>>& slow, uint64_t thresholdUS)
{
    wxLogNull logNo;  // suppress file access errors
    uint64_t worst = 0;

    std::list<std::string> folders;
    std::list<std::string> checkfiles;
    for (const auto& it : files)
    {
        wxFileName fn(it);
        if (std::find(begin(folders), end(folders), fn.GetPath()) == end(folders))
        {
            folders.push_back(fn.GetPath());
            checkfiles.push_back(it);
        }
    }

    for (const auto& it : checkfiles)
    {
        uint8_t b[8192];
        wxStopWatch sw;
        wxFile f;
        if (f.Open(it))
        {
            f.Read(b, sizeof(b));
            uint64_t t = sw.TimeInMicro().GetValue();
            if (t > worst) worst = t;
            if (t > thresholdUS)
            {
                slow.push_back({ it, t });
            }
        }
    }

    return worst;
}

void xLightsFrame::TogglePresetsPanel()
{
    if (CurrentSeqXmlFile == nullptr) return;

    if (EffectTreeDlg == nullptr)
    {
        ShowPresetsPanel();
    }
    else if (EffectTreeDlg->IsVisible())
    {
        EffectTreeDlg->Hide();
        EffectTreeDlg->Close();
        delete EffectTreeDlg;
        EffectTreeDlg = nullptr;
    }
    else
    {
        EffectTreeDlg->Show();
    }
}

void xLightsFrame::OnMenuItemSelectEffectSelected(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("SelectEffect").IsShown();
    if (visible)
    {
        m_mgr->GetPane("SelectEffect").Hide();
    }
    else
    {
        m_mgr->GetPane("SelectEffect").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::OnMenuItemShowHideVideoPreview(wxCommandEvent& event)
{
   wxAuiPaneInfo& pane = m_mgr->GetPane("SequenceVideo");

   pane.IsShown() ? pane.Hide() : pane.Show();
   m_mgr->Update();
   UpdateViewMenu();
}

void xLightsFrame::DoBackupPurge()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (BackupPurgeDays <= 0)
    {
        logger_base.debug("Backup purging skipped as it is disabled.");
        return;
    }

    logger_base.debug("Purging backups older than %d days.", BackupPurgeDays);

    time_t cur;
    time(&cur);
    wxDateTime curTime(cur);

    wxDateTime purgeDate = curTime.Add(wxTimeSpan(24 * BackupPurgeDays * -1));
    purgeDate.SetHour(0);
    purgeDate.SetMinute(0);
    purgeDate.SetSecond(0);
    purgeDate.SetMillisecond(0);

    logger_base.debug("    Keep backups on or after %s.", (const char *)purgeDate.FormatISODate().c_str());

    wxString backupDir = _backupDirectory + wxFileName::GetPathSeparator() + "Backup";

    int count = 0;
    int purged = 0;

    if (wxDir::Exists(backupDir))
    {
        wxDir dir(backupDir);
        wxString filename;

        // We dont follow symbolic links
        bool cont = dir.GetFirst(&filename, "", wxDIR_DIRS | wxDIR_NO_FOLLOW);
        while (cont) {
            auto fdc = wxSplit(filename, '-');

            if (fdc.size() > 3)
            {
                int day = wxAtoi(fdc[2]);
                int month = wxAtoi(fdc[1]);
                int year = wxAtoi(fdc[0]);

                if (year < 2010 || month < 1 || month > 12 || day < 1 || day > 31)
                {
                    // date does not look valid
                    logger_base.debug("    Backup purge ignoring %s.", (const char *)filename.c_str());
                }
                else
                {
                    wxDateTime bd(day, (wxDateTime::Month)(month - 1), year);
                    count++;

                    if (bd < purgeDate)
                    {
                        logger_base.debug("    Backup purge PURGING %s!", (const char *)filename.c_str());
                        if (!DeleteDirectory((backupDir + wxFileName::GetPathSeparator() + filename).ToStdString()))
                        {
                            logger_base.debug("        FAILED!");
                        }
                        else
                        {
                            purged++;
                        }
                    }
                    else
                    {
                        //logger_base.debug("    Backup purge keeping %s.", (const char *)filename.c_str());
                    }
                }
            }
            else
            {
                logger_base.debug("Backup purge deleted %d of %d backups.", purged, count);
            }
            cont = dir.GetNext(&filename);
        }
        logger_base.debug("    Backup purge ignoring %s.", (const char *)filename.c_str());
    }
    else
    {
        logger_base.debug("Backup purging skipped as %s does not exist.", (const char *)backupDir.c_str());
    }
}

void xLightsFrame::SetBackupPurgeDays(int nbpi)
{
    if (nbpi != BackupPurgeDays) {
        wxConfigBase* config = wxConfigBase::Get();
        config->Write("BackupPurgeDays", nbpi);
        BackupPurgeDays = nbpi;

        DoBackupPurge();
    }
}

void xLightsFrame::OnMenuItem_DownloadSequencesSelected(wxCommandEvent& event)
{
    if (CurrentDir == "") {
        wxMessageBox("Show folder is not valid. Download aborted.");
        return;
    }

    wxString downloadDir = CurrentDir + wxFileName::GetPathSeparator() + "Downloads";

    if (!wxDirExists(downloadDir))
    {
        wxMkdir(downloadDir);
    }

    VendorMusicDialog dlg(this);
    if (dlg.DlgInit("", downloadDir))
    {
        dlg.ShowModal();
    }
    else
    {
        DisplayError("Unable to access online repositories.", this);
    }
}

void xLightsFrame::OnMenuItem_JukeboxSelected(wxCommandEvent& event)
{
   wxAuiPaneInfo& pane = m_mgr->GetPane("Jukebox");

   pane.IsShown() ? pane.Hide() : pane.Show();
   m_mgr->Update();
   UpdateViewMenu();
}

void xLightsFrame::SetXFadePort(int i)
{
    _xFadePort = i;
    StartAutomationListener();
    if (_automationServer == nullptr) {
        // Give up
        _xFadePort = 0;
    }
}


void xLightsFrame::OnMenuItemUserDictSelected(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);
    SetStatusText(_("Loading dictionaries ..."));
    dictionary.LoadDictionaries(CurrentDir, this);
    SetStatusText(_(""));

    LyricUserDictDialog dlg(&dictionary, showDirectory, this);
    dlg.ShowModal();
    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::OnMenuItem_PurgeRenderCacheSelected(wxCommandEvent& event)
{
    _renderCache.Purge(&_sequenceElements, true);
}

void xLightsFrame::SetEnableRenderCache(const wxString &t)
{
    _enableRenderCache = t;
    if (_enableRenderCache == "Locked Effects Only") {
        _enableRenderCache = "Locked Only";
    }
    _renderCache.Enable(_enableRenderCache);
    _renderCache.CleanupCache(&_sequenceElements); // purge anything the cache no longer needs

    if (_renderCache.IsEnabled() && CurrentSeqXmlFile != nullptr) {
        // this will force a reload of the cache
        _renderCache.SetSequence(renderCacheDirectory, CurrentSeqXmlFile->GetName().ToStdString());
    } else {
        _renderCache.SetSequence("", "");
        _renderCache.Purge(&_sequenceElements, false);
    }
}

bool xLightsFrame::HandleAllKeyBinding(wxKeyEvent& event)
{
    if (mainSequencer == nullptr) return false;

    auto k = event.GetKeyCode();
    if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT) return false;

    if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown()) ||
        (k == 'A' && (event.ControlDown() || event.CmdDown()) && !event.AltDown()))
    {
        // Just a regular key ... If current focus is a control then we need to not process this
        if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
            (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL))
        {
            return false;
        }
    }

    auto binding = mainSequencer->keyBindings.Find(event, KBSCOPE::All);
    if (binding != nullptr) {
        std::string type = binding->GetType();
        if (type == "RENDER_ALL")
        {
            RenderAll();
        }
        else if (type == "LIGHTS_TOGGLE")
        {
            ToggleOutputs();
            m_mgr->Update();
            PlayToolBar->Refresh();
        }
        else if (type == "OPEN_SEQUENCE")
        {
            OpenSequence("", nullptr);
        }
        else if (type == "CLOSE_SEQUENCE")
        {
            AskCloseSequence();
        }
        else if (type == "NEW_SEQUENCE")
        {
            NewSequence();
            EnableSequenceControls(true);
        }
        else if (type == "PASTE_BY_CELL")
        {
            SetPasteByCell();
        }
        else if (type == "PASTE_BY_TIME")
        {
            SetPasteByTime();
        }
        else if (type == "SAVE_CURRENT_TAB")
        {
            SaveCurrentTab();
        }
        else if (type == "SEQUENCE_SETTINGS")
        {
            ShowSequenceSettings();
        }
        else if (type == "PLAY_LOOP")
        {
            wxCommandEvent playEvent(EVT_SEQUENCE_REPLAY_SECTION);
            wxPostEvent(this, playEvent);
        }
        else if (type == "PLAY")
        {
            wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
            wxPostEvent(this, playEvent);
        }
        else if (type == "TOGGLE_PLAY")
        {
            wxCommandEvent playEvent(EVT_TOGGLE_PLAY);
            wxPostEvent(this, playEvent);
        }
        else if (type == "FOCUS_SEQUENCER")
        {
            if (Notebook1->GetSelection() == NEWSEQUENCER)
            {
                SetFocus();
            }
        }
        else if (type == "START_OF_SONG")
        {
            wxCommandEvent playEvent(EVT_SEQUENCE_FIRST_FRAME);
            wxPostEvent(this, playEvent);
        }
        else if (type == "END_OF_SONG")
        {
            wxCommandEvent playEvent(EVT_SEQUENCE_LAST_FRAME);
            wxPostEvent(this, playEvent);
        }
        else if (type == "STOP")
        {
            wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
            wxPostEvent(this, playEvent);
        }
        else if (type == "PAUSE")
        {
            wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
            wxPostEvent(this, playEvent);
        }
        else if (type == "BACKUP")
        {
            wxCommandEvent e;
            OnMenuItemBackupSelected(e);
        }
        else if (type == "ALTERNATE_BACKUP")
        {
            wxCommandEvent e;
            OnmAltBackupMenuItemSelected(e);
        }
        else if (type == "SELECT_SHOW_FOLDER")
        {
            wxCommandEvent e;
            OnMenuOpenFolderSelected(e);
        }
        else
        {
            return false;
        }
        event.StopPropagation();
        return true;
    }

    return false;
}

void xLightsFrame::OnMenuItem_ShowKeyBindingsSelected(wxCommandEvent& event)
{
    DisplayInfo(mainSequencer->keyBindings.Dump(), this);
}

void xLightsFrame::OnChar(wxKeyEvent& event)
{
    OnCharHook(event);
}

void xLightsFrame::OnCharHook(wxKeyEvent& event)
{
    switch (Notebook1->GetSelection()) {
    case SETUPTAB:
        break;
    case LAYOUTTAB:
        if (!layoutPanel->HandleLayoutKeyBinding(event))
        {
            event.Skip();
        }
        return;
    case NEWSEQUENCER:
        if (!mainSequencer->HandleSequencerKeyBinding(event))
        {
            event.Skip();
        }
        return;
    default:
        break;
    }

    if (!HandleAllKeyBinding(event))
    {
        event.Skip();
    }
}

void xLightsFrame::OnMenuItem_ZoomSelected(wxCommandEvent& event)
{
    //::wxLaunchDefaultBrowser("https://zoom.us/j/175801909");
    ::wxLaunchDefaultBrowser("https://zoom.us/j/175801909?pwd=ZU1hNzM5bjJpOGZ1d1BOb1BzMUFndz09");
}

void xLightsFrame::OnMenuItem_Generate2DPathSelected(wxCommandEvent& event)
{
    PathGenerationDialog dlg(this, CurrentDir.ToStdString());
    dlg.ShowModal();
}

void xLightsFrame::OnMenuItem_PrepareAudioSelected(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // This exists solely to provide an easy way to test AudioManager::CreateAudioFile()
#if 0
    if ( CurrentSeqXmlFile != nullptr )
    {
        AudioManager *audioMgr = CurrentSeqXmlFile->GetMedia();
        if ( audioMgr != nullptr )
            audioMgr->WriteCurrentAudio( "C:\\Temp\\audioOut.wav", 44100 );
    }
    return;
#endif

    wxString filename = wxFileSelector("Choose reaper file describing the changes required to the audio.",
                                       CurrentDir, wxEmptyString, "*.rrp",
                                       "Reaper files (*.rpp)|*.rpp|xAudio files (*.xaudio)|*.xaudio|All files (*.*)|*.*",
                                       wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (filename != "") {
        logger_base.debug("Prepare audio: %s.", (const char*)filename.c_str());

        struct musicEdit {
            std::string file;
            double start;
            double length;
            double sourceoffset;
            double fadein;
            double fadeout;
            double volume;
            bool crossfadein;
            bool crossfadeout;
            musicEdit(const std::string& f, double s, double l, double so, double fi, double fo, double v, bool cfi, bool cfo) :
                file(f), start(s), length(l), sourceoffset(so), fadein(fi), fadeout(fo), volume(v), crossfadein(cfi), crossfadeout(cfo)
            {
                static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                logger_base.debug("        Source file: %s Source Pos: %0.3f Length: %0.3f Target Pos: %0.3f Fade In: %0.3f Fade Out: %0.3f Volume: %0.3f",
                                  (const char*)file.c_str(), sourceoffset, length, start, fadein, fadeout, volume);
            }
        };

        std::list<musicEdit> edits;
        wxFileName targetFile;
        targetFile.SetPath(CurrentDir);

        if (filename.Lower().EndsWith(".rpp")) {
            wxFile reaper;
            if (reaper.Open(filename)) {
                wxString reaperContent;
                reaper.ReadAll(&reaperContent);

                wxRegEx regexTgt("RENDER_FILE \\\"[^\\\"]*?\\/([^\\\"\\/]*)\\\"", wxRE_ADVANCED | wxRE_NEWLINE);
                if (regexTgt.Matches(reaperContent)) {
                    targetFile.SetName(regexTgt.GetMatch(reaperContent, 1));
                    logger_base.debug("    Target file: %s", (const char*)targetFile.GetFullPath().c_str());
                }

                wxRegEx regexPosition("POSITION ([0-9\\.]*)", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexLength("LENGTH ([0-9\\.]*)", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexSourceOffset("SOFFS ([0-9\\.]*)", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexFadeIn1("FADEIN [0-9\\.]* ([0-9\\.]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexFadeIn2("FADEIN [0-9\\.]* [0-9\\.]* ([0-9\\.]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexFadeOut1("FADEOUT [0-9\\.]* ([0-9\\.]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexFadeOut2("FADEOUT [0-9\\.]* [0-9\\.]* ([0-9\\.]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexVolume("VOLPAN [0-9\\.]* [0-9\\.]* ([0-9\\.]*) ", wxRE_ADVANCED | wxRE_NEWLINE);
                wxRegEx regexSourceFile(" FILE \\\"[^\\\"]*?\\/([^\\\"\\/]*)\\\"", wxRE_ADVANCED | wxRE_NEWLINE);

                while (regexPosition.Matches(reaperContent)) {
                    regexLength.Matches(reaperContent);
                    regexSourceOffset.Matches(reaperContent);
                    regexFadeIn1.Matches(reaperContent);
                    regexFadeIn2.Matches(reaperContent);
                    regexFadeOut1.Matches(reaperContent);
                    regexFadeOut2.Matches(reaperContent);
                    regexVolume.Matches(reaperContent);
                    regexSourceFile.Matches(reaperContent);

                    std::string sourcefile = "";
                    double start = 0;
                    double length = 0;
                    double sourceoffset = 0;
                    double fadein1 = 0;
                    double fadein2 = 0;
                    double fadeout1 = 0;
                    double fadeout2 = 0;
                    double volume = 1;

                    if (regexPosition.GetMatchCount() > 1) {
                        start = std::atof(regexPosition.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexLength.GetMatchCount() > 1) {
                        length = std::atof(regexLength.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexSourceOffset.GetMatchCount() > 1) {
                        sourceoffset = std::atof(regexSourceOffset.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexFadeIn1.GetMatchCount() > 1) {
                        fadein1 = std::atof(regexFadeIn1.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexFadeIn2.GetMatchCount() > 1) {
                        fadein2 = std::atof(regexFadeIn2.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexFadeOut1.GetMatchCount() > 1) {
                        fadeout1 = std::atof(regexFadeOut1.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexFadeOut2.GetMatchCount() > 1) {
                        fadeout2 = std::atof(regexFadeOut2.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexVolume.GetMatchCount() > 1) {
                        volume = std::atof(regexVolume.GetMatch(reaperContent, 1).c_str());
                    }
                    if (regexSourceFile.GetMatchCount() > 1) {
                        size_t s, l;
                        regexSourceFile.GetMatch(&s, &l, 0);
                        sourcefile = regexSourceFile.GetMatch(reaperContent, 1).c_str();
                        reaperContent = reaperContent.Mid(s + l);
                    }
                    edits.push_back(musicEdit(sourcefile, start, length, sourceoffset, std::max(fadein1, fadein2), std::max(fadeout1, fadeout2), volume, fadein1 < fadein2, fadeout1 < fadeout2));
                }

                if (edits.size() == 0) {
                    SetStatusText("No edits found in RPP file.");
                    return;
                }
            }
        } else if (filename.Lower().EndsWith(".xaudio")) {
            // Sample
            //
            // <xaudio>
            //    <targetfile>01 The Greatest Show Amazon Edited.mp3</targetfile>
            //    <items>
            //       <item>
            //          <targettime>0.0</targettime>    : in seconds
            //          <length>5.5</length>            : in seconds
            //          <sourcetime>1.0</sourcetime>    : in seconds
            //          <fadeinsecs>0.75</fadeinsecs>   : in seconds
            //          <fadeoutsecs>0.5</fadeoutsecs>  : in seconds
            //          <fadeoutcrossfade/>
            //          <gain>0.787367</gain>           : 1.0 = no change 0.0 = silence > 1.0 is amplification
            //          <file>01 - The Greatest Show amazon.mp3</file>
            //       </item>
            //       <item>
            //          <targettime>5</targettime>
            //          <length>3</length>
            //          <sourcetime>13.8</sourcetime>
            //          <fadeinsecs>0.5</fadeinsecs>
            //          <fadeincrossfade/>
            //          <fadeoutsecs>0.5</fadeoutsecs>
            //          <gain>0.787367</gain>
            //          <file>01 - The Greatest Show amazon.mp3</file>
            //       </item>
            //    </items>
            // </xaudio>

            wxXmlDocument doc(filename);

            if (doc.IsOk()) {
                for (wxXmlNode* r = doc.GetRoot(); r != nullptr; r = r->GetNext()) {
                    for (wxXmlNode* n = r->GetChildren(); n != nullptr; n = n->GetNext()) {
                        auto name = n->GetName().Lower();
                        if (name == "targetfile") {
                            if (n->GetChildren() != nullptr) {
                                targetFile.SetName(n->GetChildren()->GetContent());
                            }
                        } else if (name == "items") {
                            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                                name = nn->GetName().Lower();
                                if (name == "item") {
                                    std::string sourcefile = "";
                                    double start = 0;
                                    double length = 0;
                                    double sourceoffset = 0;
                                    double fadein = 0;
                                    bool fadeincrossfade = false;
                                    double fadeout = 0;
                                    bool fadeoutcrossfade = false;
                                    double volume = 1;

                                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                                        name = nnn->GetName().Lower();
                                        if (name == "file") {
                                            if (nnn->GetChildren() != nullptr) {
                                                sourcefile = nnn->GetChildren()->GetContent();
                                            }
                                        } else if (name == "targettime") {
                                            if (nnn->GetChildren() != nullptr) {
                                                start = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        } else if (name == "length") {
                                            if (nnn->GetChildren() != nullptr) {
                                                length = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        } else if (name == "sourcetime") {
                                            if (nnn->GetChildren() != nullptr) {
                                                sourceoffset = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        } else if (name == "fadeinsecs") {
                                            if (nnn->GetChildren() != nullptr) {
                                                fadein = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        } else if (name == "fadeoutsecs") {
                                            if (nnn->GetChildren() != nullptr) {
                                                fadeout = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        } else if (name == "fadeincrossfade") {
                                            fadeincrossfade = true;
                                        } else if (name == "fadeoutcrossfade") {
                                            fadeoutcrossfade = true;
                                        } else if (name == "gain") {
                                            if (nnn->GetChildren() != nullptr) {
                                                volume = std::atof(nnn->GetChildren()->GetContent().c_str());
                                            }
                                        }
                                    }
                                    edits.push_back(musicEdit(sourcefile, start, length, sourceoffset, fadein, fadeout, volume, fadeincrossfade, fadeoutcrossfade));
                                }
                            }
                        }
                    }
                }
                if (edits.size() == 0) {
                    SetStatusText("No edits found in xAudio file.");
                    return;
                }
            } else {
                SetStatusText("Invalid xAudio file.");
                return;
            }
        }

        SetStatusText("Loading audio files.");

        // load the audio files
        std::map<std::string, AudioManager*> sourceSongs;
        double outputLength = 0;
        for (const auto& it : edits) {
            outputLength = std::max(outputLength, it.start + it.length);

            if (sourceSongs.find(it.file) == sourceSongs.end()) {
                wxString music = wxFileSelector("Choose your copy of " + it.file + ".",
                                                CurrentDir, wxEmptyString, wxEmptyString,
                                                "Audio files|*.mp3;*.ogg;*.m4p;*.mp4;*.avi;*.wma;*.wmv;*.au;*.wav;*.m4a;*.mid;*.mkv;*.mov;*.mpg;*.asf;*.flv;*.mpeg",
                                                wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

                if (music != "") {
                    sourceSongs[it.file] = new AudioManager(music);
                } else {
                    sourceSongs[it.file] = nullptr;
                }
            }
        }

        bool ok = true;

        long outputRate = -1;
        for (const auto& it : sourceSongs) {
            if (it.second != nullptr) {
                if (outputRate == -1) {
                    outputRate = it.second->GetRate();
                } else {
                    if (ok && outputRate != it.second->GetRate()) {
                        logger_base.debug("Songs do not all have the same bitrate ... unable to do the required mixing.");
                        wxMessageBox("In order to prepare the audio all the input songs must have the same bitrate.");
                        ok = false;
                    }
                }
            }
        }

        if (outputRate == -1) {
            SetStatusText("Audio file creation failed - No input audio.");
            ok = false;
        }

        if (ok) {
            long totalSamples = outputRate * outputLength;
            logger_base.debug("    New file will:");
            logger_base.debug("        have %ld samples.", totalSamples);
            logger_base.debug("        be %0.3f seconds long.", outputLength);
            std::vector<float> left(totalSamples);
            std::vector<float> right(totalSamples);

            for (const auto& it : edits) {
                auto audio = sourceSongs[it.file];
                if (audio != nullptr) {
                    // ensure that the audio has been processed by setting a frame interval if its unset
                    if (audio->GetFrameInterval() < 0){
                        logger_base.debug("Setting default frame interval for %s.", (const char*)it.file.c_str());
                        audio->SetFrameInterval(20);
                    }
                    // check the data is actually loaded
                    audio->GetRawLeftData(audio->GetTrackSize() - 1);

                    SetStatusText("Combining audio clips.");

                    logger_base.debug("Processing sample from %s.", (const char*)it.file.c_str());
                    long startOutput = outputRate * it.start;
                    long outputSamples = outputRate * it.length;
                    // logger_base.debug("    Sample Output Start %ld-%ld [%ld].", startOutput, startOutput + outputSamples - 1, outputSamples);
                    wxASSERT(startOutput + outputSamples - 1 <= totalSamples);
                    long startSample = audio->GetRate() * it.sourceoffset;
                    long inputSamples = audio->GetRate() * it.length;
                    // logger_base.debug("    Input file samples %ld", audio->GetTrackSize());
                    // logger_base.debug("    Sample Input Start %ld-%ld [%ld].", startSample, startSample + inputSamples - 1, inputSamples);
                    wxASSERT(startSample + inputSamples - 1 < audio->GetTrackSize());

                    // this code does not handle mixed sample rates
                    wxASSERT(inputSamples == outputSamples);

                    float* lsource = audio->GetRawLeftDataPtr(startSample);
                    float* rsource = audio->GetRawRightDataPtr(startSample);
                    long fadeinsamples = it.fadein * audio->GetRate();
                    long fadeoutsamples = it.fadeout * audio->GetRate();
                    long fadeoutstart = inputSamples - fadeoutsamples;

                    for (long i = 0; i < inputSamples; i++) {
                        float l = lsource[i] * it.volume;
                        float r;
                        if (rsource != nullptr) {
                            r = rsource[i] * it.volume;
                        } else {
                            r = l;
                        }
                        if (i < fadeinsamples) {
                            // Linear
                            // l *= (double)i / fadeinsamples; // linear fade for now
                            // r *= (double)i / fadeinsamples; // linear fade for now

                            if (it.crossfadein) {
                                // cross fade in
                                // log10(x/fadeinsamples+.1)*10/11
                                double f = log10((double)i / fadeinsamples + 0.1) * 10.0 / 11.0;
                                if (f < 0)
                                    f = 0.0;
                                if (f > 1)
                                    f = 1.0;
                                l *= f;
                                r *= f;
                            } else {
                                // exponent in
                                //(10 ^ (x/fadeinsamples - 1)-.1) * 1.1
                                double f = pow(10.0, ((double)i / fadeinsamples - 1.0) - 0.1) * 1.1;
                                if (f < 0)
                                    f = 0.0;
                                if (f > 1)
                                    f = 1.0;
                                l *= f;
                                r *= f;
                            }
                        }
                        if (i > fadeoutstart) {
                            // Linear
                            // l *= (double)(inputSamples - i) / fadeoutsamples; // linear fade for now
                            // r *= (double)(inputSamples - i) / fadeoutsamples; // linear fade for now

                            if (it.crossfadeout) {
                                // cross fade out
                                //  1 - 10 ^ (x/fadeoutsamples - .95) + .1
                                double f = 1.0 - log10((double)(inputSamples - i) / fadeinsamples + 0.1) * 10.0 / 11.0;
                                if (f < 0)
                                    f = 0.0;
                                if (f > 1)
                                    f = 1.0;
                                l *= f;
                                r *= f;
                            } else {
                                // exponent out
                                // 1 - log 10 (x/fadeoutsamples +.1)
                                double f = 1.0 - pow(10.0, ((double)(inputSamples - i) / fadeinsamples - 1.0) - 0.1) * 1.1;
                                if (f < 0)
                                    f = 0.0;
                                if (f > 1)
                                    f = 1.0;
                                l *= f;
                                r *= f;
                            }
                        }
                        left[startOutput + i] += l;
                        right[startOutput + i] += r;
                    }
                }
            }

            // Clip it
            for (auto& it : left) {
                if (it > 1.0)
                    it = 1.0;
            }
            for (auto& it : right) {
                if (it > 1.0)
                    it = 1.0;
            }

            if (FileExists(targetFile)) {
                if (wxMessageBox(targetFile.GetFullPath() + " already exists. Do you want to overwrite it?", "Replace", wxYES_NO | wxCENTRE, this) == wxNO) {
                    wxFileDialog fd(this,
                                    "Choose filename to save the audio as.",
                                    targetFile.GetPath(),
                                    targetFile.GetName(),
                                    "MP3 Files|*.mp3",
                                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

                    if (fd.ShowModal() == wxID_OK) {
                        targetFile.SetPath(fd.GetDirectory());
                        targetFile.SetName(fd.GetFilename());
                    } else {
                        return;
                    }
                }
            }
            SetStatusText("Saving output file.");

            if (!AudioManager::CreateAudioFile(left, right, targetFile.GetFullPath(), outputRate)) {
                wxMessageBox("Error creating audio file. See log for details.");
                SetStatusText("Audio file creation failed.");
            } else {
                SetStatusText("Audio file created: " + targetFile.GetFullPath());
            }
        } else {
            SetStatusText("Audio file creation failed.");
        }

        for (const auto& it : sourceSongs) {
            delete it.second;
        }
    }
}

void xLightsFrame::OnMenuItem_UserManualSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://manual.xlights.org/");
}

void xLightsFrame::SetModelHandleSize(int i)
{
    _modelHandleSize = i;
    layoutPanel->Refresh();
}

void xLightsFrame::SetSuppressFadeHints(bool b)
{
    mSuppressFadeHints = b;
    mainSequencer->PanelEffectGrid->Refresh();
}

bool xLightsFrame::IsNewModel(Model* m) const
{
    return layoutPanel->IsNewModel(m);
}

void xLightsFrame::SetRandomEffectsToUse(const wxArrayString& effects)
{
    _randomEffectsToUse = effects;
    const wxString randomEffects = wxJoin(_randomEffectsToUse, ',');
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsRandomEffects", randomEffects);
    config->Flush();
}

void xLightsFrame::SetUserEMAIL(const wxString &e) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _userEmail = e;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsUserEmail", _userEmail);
    config->Flush();
    logger_base.info("User email changed to %s", (const char*)_userEmail.c_str());
}

void xLightsFrame::SetLinkedSave(const wxString& e)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _linkedSave = e;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsLinkedSave", _linkedSave);
    config->Flush();
    logger_base.info("Linked save set to %s", (const char*)_linkedSave.c_str());
}

void xLightsFrame::SetLinkedControllerUpload(const wxString& e)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _linkedControllerUpload = e;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsLinkedControllerUpload", _linkedControllerUpload);
    config->Flush();
    logger_base.info("Linked controller upload set to %s", (const char*)_linkedControllerUpload.c_str());
}

void xLightsFrame::CollectUserEmail()
{
    if (_userEmail == "") _userEmail = "noone@nowhere.xlights.org";
    EmailDialog dlg(this, _userEmail);
    if (dlg.ShowModal() == wxID_OK) {
        SetUserEMAIL(dlg.GetEmail());
    }
    else {
        // if the user cancels assume they want it set to the prior address
        SetUserEMAIL(_userEmail);
    }
}

void xLightsFrame::SetDefaultSeqView(const wxString& view)
{
    //if (_sequenceViewManager.GetViewIndex(view) == -1) {
    //    return;
    //}

    _defaultSeqView = view;
    SetXmlSetting("defaultSeqView", view);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
}

wxArrayString xLightsFrame::GetSequenceViews()
{
    return _sequenceViewManager.GetViewList();
}

void xLightsFrame::SetMinTipLevel(const wxString& level)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("MinTipLevel", level);
    config->Flush();
    logger_base.info("Minimum tip level set to %s", (const char*)level.c_str());
}

std::string xLightsFrame::GetMinTipLevel() const
{
    wxConfigBase* config = wxConfigBase::Get();
    return config->Read("MinTipLevel", "Beginner");
}

void xLightsFrame::SetVideoExportCodec(const wxString& codec)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _videoExportCodec = codec;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsVideoExportCodec", _videoExportCodec);
    config->Flush();
    logger_base.info("Video Export Codec set to %s", (const char*)_videoExportCodec.c_str());
}

void xLightsFrame::SetVideoExportBitrate(int bitrate)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _videoExportBitrate = bitrate;
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsVideoExportBitrate", _videoExportBitrate);
    config->Flush();
    logger_base.info("Video Export Bitrate set to %d", _videoExportBitrate);
}

void xLightsFrame::OnMenuItem_ValueCurvesSelected(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("ValueCurveDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("ValueCurveDropper").Hide();
    } else {
        m_mgr->GetPane("ValueCurveDropper").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::OnMenuItem_ColourDropperSelected(wxCommandEvent& event)
{
    InitSequencer();
    bool visible = m_mgr->GetPane("ColourDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("ColourDropper").Hide();
    } else {
        m_mgr->GetPane("ColourDropper").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::OnSysColourChanged(wxSysColourChangedEvent& event) {
    event.Skip();
    color_mgr.SysColorChanged();
}

void xLightsFrame::SetHardwareVideoAccelerated(bool b)
{
    _hwVideoAccleration = b;
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsVideoReaderAccelerated", VideoReader::IsHardwareAcceleratedVideo());
}

bool xLightsFrame::ShadersOnBackgroundThreads() const {
    return ShaderEffect::IsBackgroundRender();
}
void xLightsFrame::SetShadersOnBackgroundThreads(bool b) {
    ShaderEffect::SetBackgroundRender(b);
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsShadersOnBackgroundThreads", b);
}

bool xLightsFrame::UseGPURendering() const {
    return GPURenderUtils::IsEnabled();
}
void xLightsFrame::SetUseGPURendering(bool b) {
    GPURenderUtils::SetEnabled(b);
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsGPURendering", b);
}


void xLightsFrame::OnMenuItemBulkControllerUploadSelected(wxCommandEvent& event)
{
    MultiControllerUploadDialog dlg(this);
    dlg.ShowModal();
}

void xLightsFrame::OnMenuItem_KeyBindingsSelected(wxCommandEvent& event)
{
    KeyBindingEditDialog dlg(this, &GetMainSequencer()->keyBindings, &effectManager);

    dlg.ShowModal();
}

void xLightsFrame::OnMenuItem_ExportControllerConnectionsSelected(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "Controller_Connections", wxEmptyString, "Export files (*.xlsx)|*.xlsx", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) {
        return;
    }

    auto controllers = GetOutputManager()->GetControllers();
    ExportSettings::SETTINGS exportsettings = ExportSettings::GetSettings(this);

    lxw_workbook* workbook = workbook_new(filename.c_str());
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);

    lxw_format* header_format = workbook_add_format(workbook);
    format_set_align(header_format, LXW_ALIGN_CENTER);
    format_set_align(header_format, LXW_ALIGN_VERTICAL_CENTER);
    format_set_bold(header_format);
    format_set_bg_color(header_format, LXW_COLOR_YELLOW);
    format_set_border(header_format, LXW_BORDER_THIN);

    lxw_format* first_format = workbook_add_format(workbook);
    format_set_border(first_format, LXW_BORDER_MEDIUM);
    format_set_bold(first_format);

    lxw_format* format = workbook_add_format(workbook);
    format_set_border(format, LXW_BORDER_THIN);

    int row = 0;
    std::map<int, double> _col_widths;

    for (const auto& it : controllers) {
        UDController cud(it, &_outputManager, &AllModels, false);
        int columSize = 0;
        std::vector < std::vector<std::string>> const lines = cud.ExportAsCSV(exportsettings, it->GetDefaultBrightnessUnderFullControl(), columSize);

        worksheet_merge_range(worksheet, row, 0, row, columSize, it->GetShortDescription().c_str(), header_format);
        ++row;
        auto lformat = first_format;

        for (const auto& line : lines) {
            for (int i = 1; i <= columSize;++i) {
                worksheet_write_blank(worksheet, row, i, lformat);
            }
            int col = 0;
            for (auto const& column : line) {
                if (column.empty()) {
                    continue;
                }
                worksheet_write_string(worksheet, row, col, column.c_str(), lformat);
                double width = column.size() + 1.3;//estimate column width
                if (_col_widths[col] < width) {
                    _col_widths[col] = width;
                    worksheet_set_column(worksheet, col, col, width, NULL);
                }
                ++col;
            }
            ++row;
            lformat = format;
        }
        row+=2;
    }
    lxw_error error = workbook_close(workbook);
    if (error) {
        DisplayError(wxString::Format("Unable to create Spreadsheet, Error %d = %s\n", error, lxw_strerror(error)).ToStdString());
        return;
    }

    SetStatusText("Controller Connections spreadsheet saved to " + filename);
}

void xLightsFrame::OnMenuItem_xScannerSelected(wxCommandEvent& event)
{
#ifdef LINUX
    // Handle xschedule not in path
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    wxString appPath(f.GetPath());
    wxString cmdline(appPath+wxT("/xScanner"));
    wxExecute(cmdline, wxEXEC_ASYNC,NULL,NULL);
#else
    wxExecute("xScanner.exe");
#endif
}

std::string xLightsFrame::GetUniqueTimingName(const std::string& baseName)
{
    if (CurrentSeqXmlFile == nullptr) return baseName;
    std::string name = baseName;
    int suffix = 2;
    while (CurrentSeqXmlFile->TimingAlreadyExists(name, this)) {
        name = wxString::Format("%s_%d", baseName, suffix++);
    }
    return name;
}

void xLightsFrame::ReplaceModelWithModelFixGroups(const std::string& oldModel, const std::string& newModel)
{
    for (auto& it : AllModels) {
        if (it.second->GetDisplayAs() == "ModelGroup") {
            auto mg = dynamic_cast<ModelGroup*>(it.second);
            mg->ModelRenamed(newModel, oldModel);
        }
    }
}

void xLightsFrame::OnMenuItemRunScriptSelected(wxCommandEvent& event)
{
    if (!_scriptsDialog) {
        _scriptsDialog = std::make_unique<ScriptsDialog>(this);
    }
    _scriptsDialog->Show();
}

void xLightsFrame::UpdateViewMenu()
{
    std::map<std::string, wxMenuItem*> panes = {
        { "DisplayElements", MenuItemDisplayElements },
        { "ModelPreview", MenuItemModelPreview },
        { "HousePreview", MenuItemHousePreview },
        { "Effect", MenuItemEffectSettings },
        { "Color", MenuItemColours },
        { "LayerTiming", MenuItemLayerBlending },
        { "LayerSettings", MenuItemLayerSettings },
        { "EffectDropper", MenuItemEffectDropper },
        { "ValueCurveDropper", MenuItemValueCurves },
        { "ColourDropper", MenuItemColourDropper },
        { "EffectAssist", MenuItemEffectAssist },
        { "SelectEffect", MenuItemSelectEffect },
        { "SequenceVideo", MenuItemVideoPreview },
        { "Jukebox", MenuItemJukebox },
        { "FindData", MenuItemFindData },
        { "SearchPanel", MenuItemSearchEffects }
    };

    wxAuiPaneInfoArray& info = m_mgr->GetAllPanes();
    for (size_t x = 0; x < info.size(); x++) {
        if (info[x].IsOk()) {
            auto pane = panes.find(info[x].name);
            if (pane != panes.end()) {
                (*pane).second->Check(m_mgr->GetPane(info[x].name).IsShown());
            }
        }
    }
}

void xLightsFrame::OnMenuItem_ColorReplaceSelected(wxCommandEvent& event)
{
    ColourReplaceDialog dlg(this, this);
    dlg.ShowModal();

    // need to update the colour panel as the colours may have changed on the selected effect
    if (GetMainSequencer()->GetSelectedEffect() != nullptr) {
        SelectedEffectChangedEvent event(GetMainSequencer()->GetSelectedEffect(), false, true);
        SelectedEffectChanged(event);
    }

    // user will need to render as this does not force a re-render.
}

void xLightsFrame::OnMenuItemFindDataSelected(wxCommandEvent& event)
{
    wxAuiPaneInfo& pane = m_mgr->GetPane("FindData");

    pane.IsShown() ? pane.Hide() : pane.Show();
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowDataFindPanel()
{
    wxAuiPaneInfo& pane = m_mgr->GetPane("FindData");

    if (!pane.IsShown()) {
        pane.Show();
        m_mgr->Update();
        UpdateViewMenu();
    }
}

void xLightsFrame::OnMenuItemSearchEffectsSelected(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("SearchPanel").IsShown();
    if (visible) {
        m_mgr->GetPane("SearchPanel").Hide();
    } else {
        m_mgr->GetPane("SearchPanel").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::OnMenuItem_SilentVolSelected(wxCommandEvent& event)
{
    playVolume = 0;
    AudioManager::GetSDLManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_TODSelected(wxCommandEvent& event)
{
    #ifdef __WXMSW__
    if (!_tod.DoTipOfDay(true)) {
    #else
    if (!_tod->DoTipOfDay(true)) {
    #endif
        wxBell();
    }
}

void xLightsFrame::OnMenuItem_RemapCustomSelected(wxCommandEvent& event)
{
    ModelRemap modelRemap(this);
    modelRemap.ShowModal();
}

void xLightsFrame::OnMenuItemRestoreBackupSelected(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile != nullptr) {
        wxMessageBox("Please Close the Open Sequence Before Using the Restore Backup Dialog");
        return;
    }
    RestoreBackupDialog restore(showDirectory, _backupDirectory, this);
    if (restore.ShowModal() ==  wxID_OK) {
        auto restoreFolder = restore.GetRestoreFolder();
        auto restoreFiles = restore.GetRestoreFiles();
        wxProgressDialog prgs("Restoring Backup",
                              "Restoring Backup", 100, this);

        prgs.Pulse("Restoring Backup... " + restoreFolder);
        prgs.Show();

        if (!wxDirExists(restoreFolder)) {
            DisplayError(wxString::Format("Unable to Restore backup directory '%s'!", restoreFolder).ToStdString());
            return;
        }
        UnsavedNetworkChanges = false;
        UnsavedRgbEffectsChanges = false;
        modelsChangeCount = 0;
        std::string errors;
        for (auto const& file : restoreFiles) {
            prgs.Pulse("Restoring '" + file + "'...");
            bool success = wxCopyFile(restoreFolder + wxFileName::GetPathSeparator() + file,
                                      showDirectory + wxFileName::GetPathSeparator() + file);
            if (!success) {
                errors += "Unable to copy file \"" + file + "\"\n";
            }
        }

        if (!errors.empty()) {
            DisplayError(errors, this);
        } else {
            SetDir(showDirectory, true);
        }
    }
}

void xLightsFrame::OnMenuItem_SuppressDock(wxCommandEvent& event)
{
    m_mgr->GetPane("HousePreview").Dockable(IsDockable("HP"));
    m_mgr->GetPane("ModelPreview").Dockable(IsDockable("MP"));
    SaveDockable();
}

void xLightsFrame::LoadDockable()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();
    if (config == nullptr) {
        logger_base.error("Null config ... this wont end well.");
        return;
    }
    bool bv;

    config->Read("xLights_SD_HP", &bv, false);
    logger_base.debug("Suppress Dock HousePreview: %s.", toStr(bv));
    MenuItem_SD_HP->Check(!bv);

    config->Read("xLights_SD_MP", &bv, false);
    logger_base.debug("Suppress Dock ModelPreview: %s.", toStr(bv));
    MenuItem_SD_MP->Check(!bv);
}

void xLightsFrame::SaveDockable()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();
    if (config == nullptr) {
        logger_base.error("Null config ... this wont end well.");
        return;
    }
    config->Write("xLights_SD_HP", IsDockable("HP"));
    config->Write("xLights_SD_MP", IsDockable("MP"));
}

bool xLightsFrame::IsDockable(const std::string& panel)
{
    if (panel == "HP") {
        return !MenuItem_SD_HP->IsChecked();
    }
    else if (panel == "MP") {
        return !MenuItem_SD_MP->IsChecked();
    }

    return true;
}
