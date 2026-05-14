/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
// Author:    Matt Brown (dowdybrown@yahoo.com)
// Created:   2012-11-03
// Copyright: Matt Brown ()

#ifdef _DEBUG
//#define SIMULATE_UPGRADE
#endif

#include <wx/stopwatch.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/debugrpt.h>
#include <wx/dir.h>
#include <wx/grid.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/protocol/http.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/taskbar.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/settings.h>
#include <wx/display.h>
#include <wx/tooltip.h>
#include <wx/valnum.h>
#include <wx/version.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <cctype>
#include <cstring>
#include <thread>
#include <string>

#include "app-shell/AboutDialog.h"
#include "sequencer/BatchRenderDialog.h"
#include "CachedFileDownloader.h"
#include "shared/dialogs/CheckboxSelectDialog.h"
#include "shared/dialogs/OptionChooser.h"
#include "graphics/GLContextManager.h"
#ifndef __APPLE__
#include "effectpanels/ShaderPanel.h"
#include "graphics/opengl/xlGLCanvas.h"
#endif
#include "color/ColourReplaceDialog.h"
#include "color/ColoursPanel.h"
#include "import_export/ConvertDialog.h"
#include "effects/EffectAssist.h"
#include "sequencer/EffectIconPanel.h"
#include "sequencer/EffectsPanel.h"
#include "app-shell/EmailDialog.h"
#include "import_export/ExportSettings.h"
#include "shared/utils/ExternalHooksUI.h"
#include "diagnostics/FindDataPanel.h"
#include "render/GPURenderUtils.h"
#include "render/SequenceMedia.h"
#include "render/SequencePackage.h"
#include "shared/utils/wxUtilities.h"
#include "graphics/wxTextDrawingContext.h"
#ifdef LINUX
#include "render/FreeTypeTextDrawingContext.h"
#endif
#include "utils/AppCallbacks.h"
#include "utils/xlImage.h"
#include <wx/mstream.h>
#include "model/GenerateCustomModelDialog.h"
#include "sequencer/GenerateLyricsDialog.h"
#include "layout/HousePreviewPanel.h"
#include "setup/IPEntryDialog.h"
#include "media/JukeboxPanel.h"
#include "app-shell/KeyBindingEditDialog.h"
#include "layout/LayoutGroup.h"
#include "layout/LayoutPanel.h"
#include "sequencer/LyricUserDictDialog.h"
#include "layout/ModelPreview.h"
#include "import_export/ModelRemap.h"
#include "setup/MultiControllerUploadDialog.h"
#include "Parallel.h"
#include "model/PathGenerationDialog.h"
#include "setup/PixelTestDialog.h"
#include "sequencer/RenderCommandEvent.h"
#include "app-shell/RestoreBackupDialog.h"
#include "sequencer/SeqSettingsDialog.h"
#include "effects/ShaderDownloadDialog.h"
#include "utils/SpecialOptions.h"
#include "app-shell/SplashDialog.h"
#include "diagnostics/SequenceChecker.h"
#include "diagnostics/ShowFolderSearchDialog.h"
#include "sequencer/TopEffectsPanel.h"
#include "utils/TraceLog.h"
#include "app-shell/UpdaterDialog.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "shared/controls/ValueCurveButton.h"
#include "shared/controls/ValueCurvesPanel.h"
#include "import_export/VendorModelDialog.h"
#include "import_export/VendorMusicDialog.h"
#include "media/VideoExporter.h"
#include "layout/ViewsModelsPanel.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "xLightsVersion.h"
#include "settings/XLightsConfigAdapter.h"
#include "controllerproperties/ControllerPropertyAdapter.h"
#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ESPixelStick.h"
#include "controllers/FPPConnectDialog.h"
#include "controllers/Falcon.h"
#include "controllers/HinksPixExportDialog.h"
#include "effectpanels/EffectIconCache.h"
#include "effects/FacesEffect.h"
#include "effects/RenderableEffect.h"
#include "effects/ShaderEffect.h"
#include "effects/StateEffect.h"
#include "graphics/opengl/xlGLCanvas.h"
#include "models/ModelGroup.h"
#include "models/RulerObject.h"
#include "models/SubModel.h"
#include "models/ViewObject.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/E131Output.h"
#include "outputs/IPOutput.h"
#include "outputs/ZCPPOutput.h"
#include "sequencer/MainSequencer.h"
#include "utils/ip_utils.h"
#include "TempFileManager.h"
#include "color/xlColourData.h"
#include "utils/CurlManager.h"
#include "utils/FileUtils.h"
#include "ai/chatGPT.h"
#include "ai/AIImageDialog.h"
#include "ai/WxServiceSettingsStore.h"
#include "models/DMX/DmxMovingHeadComm.h"
#include "color/ColorPanel.h"

#include "../dependencies/wxHTTPServer/wxhttpserver.h"

// Linux needs this
#include <wx/stdpaths.h>
#include <wx/progdlg.h>
#include <wx/filename.h>

// image files
#include "../include/control-pause-blue-icon.xpm"
#include "../include/control-play-blue-icon.xpm"

#include <xlsxwriter.h>
#include "diagnostics/CheckSequenceReport.h"
#include <log.h>

//(*InternalHeaders(xLightsFrame)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#define TOOLBAR_SAVE_VERSION "0003:"
#define MAXBACKUPFILE_MB 30

//(*IdInit(xLightsFrame)
const wxWindowID xLightsFrame::ID_AUITOOLBAR_OPENSHOW = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_NEWSEQUENCE = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_OPEN = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_SAVE = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_SAVEAS = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_RENDERALL = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_MAIN = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_PLAY_NOW = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_PAUSE = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_STOP = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_FIRST_FRAME = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_LAST_FRAME = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_REPLAY_SECTION = wxNewId();
const wxWindowID xLightsFrame::ID_CHECKBOX_LIGHT_OUTPUT = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_PLAY = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM2 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM5 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM7 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM3 = wxNewId();
const wxWindowID xLightsFrame::ID_TOGGLE_MODEL_PREVIEW = wxNewId();
const wxWindowID xLightsFrame::ID_TOGGLE_HOUSE_PREVIEW = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM6 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM8 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM9 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM10 = wxNewId();
const wxWindowID xLightsFrame::ID_AUIWINDOWTOOLBAR = wxNewId();
const wxWindowID xLightsFrame::ID_PASTE_BY_TIME = wxNewId();
const wxWindowID xLightsFrame::ID_PASTE_BY_CELL = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_EDIT = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACDISABLED = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACSELECT = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACOFF = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACON = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACSHIMMER = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACTWINKLE = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACINTENSITY = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACRAMPUP = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACRAMPDOWN = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACRAMPUPDOWN = wxNewId();
const wxWindowID xLightsFrame::ID_CHOICE_PARM1 = wxNewId();
const wxWindowID xLightsFrame::ID_CHOICE_PARM2 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACFILL = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACCASCADE = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACFOREGROUND = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM_ACBACKGROUND = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_AC = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBARITEM14 = wxNewId();
const wxWindowID xLightsFrame::ID_AUITOOLBAR_VIEW = wxNewId();
const wxWindowID xLightsFrame::ID_AUIEFFECTSTOOLBAR = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON3 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON11 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON13 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT4 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT2 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON14 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON17 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON15 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT3 = wxNewId();
const wxWindowID xLightsFrame::ID_CHECKBOX1 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON16 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON_SAVE_SETUP = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON9 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON6 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON10 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON5 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON18 = wxNewId();
const wxWindowID xLightsFrame::ID_BITMAPBUTTON1 = wxNewId();
const wxWindowID xLightsFrame::ID_BITMAPBUTTON2 = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL2 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON1 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON2 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON7 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT1 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON8 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON4 = wxNewId();
const wxWindowID xLightsFrame::ID_BUTTON12 = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL3 = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL_SETUP = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL_PREVIEW = wxNewId();
const wxWindowID xLightsFrame::XLIGHTS_SEQUENCER_TAB = wxNewId();
const wxWindowID xLightsFrame::ID_NOTEBOOK1 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT6 = wxNewId();
const wxWindowID xLightsFrame::ID_GAUGE1 = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL5 = wxNewId();
const wxWindowID xLightsFrame::ID_STATICTEXT7 = wxNewId();
const wxWindowID xLightsFrame::ID_PANEL1 = wxNewId();
const wxWindowID xLightsFrame::ID_NEW_SEQUENCE = wxNewId();
const wxWindowID xLightsFrame::ID_OPEN_SEQUENCE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM4 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_OPENRECENTSEQUENCE = wxNewId();
const wxWindowID xLightsFrame::IS_SAVE_SEQ = wxNewId();
const wxWindowID xLightsFrame::ID_SAVE_AS_SEQUENCE = wxNewId();
const wxWindowID xLightsFrame::ID_CLOSE_SEQ = wxNewId();
const wxWindowID xLightsFrame::ID_SEQ_SETTINGS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_KEYBINDINGS = wxNewId();
const wxWindowID xLightsFrame::ID_EXPORT_VIDEO = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM2 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM8 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_RECENTFOLDERS = wxNewId();
const wxWindowID xLightsFrame::ID_FILE_BACKUP = wxNewId();
const wxWindowID xLightsFrame::ID_FILE_RESTOREBACKUP = wxNewId();
const wxWindowID xLightsFrame::ID_FILE_ALTBACKUP = wxNewId();
const wxWindowID xLightsFrame::ID_SHIFT_EFFECTS_AND_TIMING = wxNewId();
const wxWindowID xLightsFrame::ID_SHIFT_EFFECTS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SHIFT_SELECTED_EFFECTS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_COLOURREPLACE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM13 = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_CHECKSEQ = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_CLEANUPFILE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_PACKAGESEQUENCE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_DOWNLOADSEQUENCES = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_BATCH_RENDER = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_FPP_CONNECT = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_BULKUPLOAD = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_HINKSPIX_EXPORT = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_RUN_SCRIPT = wxNewId();
const wxWindowID xLightsFrame::ID_EXPORT_MODELS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_EXPORT_EFFECTS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_EXPORT_CONTROLLER_CONNECTIONS = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_VIEW_LOG = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM18 = wxNewId();
const wxWindowID xLightsFrame::iD_MNU_VENDORCACHEPURGE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_PURGERENDERCACHE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_CRASH = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_DUMPRENDERSTATE = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_GENERATE2DPATH = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_GenerateCustomModel = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_REMAPCUSTOM = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_GenerateAIImage = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_GENERATELYRICS = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_CONVERT = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_PREPAREAUDIO = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_USER_DICT = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_FIND_SHOW_FOLDER = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM5 = wxNewId();
const wxWindowID xLightsFrame::MNU_ID_ACLIGHTS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SHOWRAMPS = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_SAVE_PERSPECTIVE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_SAVE_AS_PERSPECTIVE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_LOAD_PERSPECTIVE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_PERSPECTIVES_AUTOSAVE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM7 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_DISPLAY_ELEMENTS = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_TOGGLE_MODEL_PREVIEW = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_TOGGLE_HOUSE_PREVIEW = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM14 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM15 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM16 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM9 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM17 = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_VALUECURVES = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_COLOURDROPPER = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_EFFECT_ASSIST_WINDOW = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_EFFECT_PRESETS = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_SELECT_EFFECT = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_SEARCH_EFFECTS = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_VIDEOPREVIEW = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_JUKEBOX = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_FINDDATA = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SUPPRESSDOCK_HP = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SUPPRESSDOCK_MP = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM3 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_WINDOWS_PERSPECTIVE = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM_WINDOWS_DOCKALL = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM11 = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM10 = wxNewId();
const wxWindowID xLightsFrame::ID_PLAY_FULL = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_1POINT5SPEED = wxNewId();
const wxWindowID xLightsFrame::ID_MN_2SPEED = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_3SPEED = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_4SPEED = wxNewId();
const wxWindowID xLightsFrame::ID_PLAY_3_4 = wxNewId();
const wxWindowID xLightsFrame::ID_PLAY_1_2 = wxNewId();
const wxWindowID xLightsFrame::ID_PLAY_1_4 = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_LOUDVOLUME = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_MEDVOLUME = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_QUIET = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SUPERQUIET = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_SILENT = wxNewId();
const wxWindowID xLightsFrame::ID_IMPORT_EFFECTS = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_TOD = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_MANUAL = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_ZOOM = wxNewId();
const wxWindowID xLightsFrame::ID_MENUITEM1 = wxNewId();
const wxWindowID xLightsFrame::idMenuHelpContent = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_HELP_FORMUM = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_VIDEOS = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_HELP_DOWNLOAD = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_HELP_RELEASE_NOTES = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_HELP_ISSUE = wxNewId();
const wxWindowID xLightsFrame::ID_MENU_HELP_FACEBOOK = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_DONATE = wxNewId();
const wxWindowID xLightsFrame::ID_MNU_UPDATE = wxNewId();
const wxWindowID xLightsFrame::ID_TIMER_OutputTimer = wxNewId();
const wxWindowID xLightsFrame::ID_TIMER_AutoSave = wxNewId();
const wxWindowID xLightsFrame::ID_TIMER_EFFECT_SETTINGS = wxNewId();
const wxWindowID xLightsFrame::ID_TIMER_RENDERSTATUS = wxNewId();
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
const long xLightsFrame::ID_COPYROW_EFFECT = wxNewId();  // copy random effect across row -DJ
const long xLightsFrame::ID_CLEARROW_EFFECT = wxNewId(); // clear all effects on this row -DJ

const long xLightsFrame::ID_XFADESERVER = wxNewId();
const long xLightsFrame::ID_XFADESOCKET = wxNewId();

const long xLightsFrame::ID_MENU_ITEM_PREVIEWS = wxNewId();
const long xLightsFrame::ID_MENU_ITEM_PREVIEWS_SHOW_ALL = wxNewId();

const long xLightsFrame::ID_MENUITEM_REVERTTO = wxNewId();


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
wxDEFINE_EVENT(EVT_COPY_MODEL_EFFECTS_TO_MODELS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PASTE_MODEL_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_PASTE_MODEL_EFFECTS_WITH_SUB_LAYERS, wxCommandEvent);
wxDEFINE_EVENT(EVT_MODEL_SELECTED, wxCommandEvent);
wxDEFINE_EVENT(EVT_PLAY_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_PAUSE_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);
wxDEFINE_EVENT(EVT_STOP_SEQUENCE, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_FIRST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_LAST_FRAME, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_REWIND10, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_FFORWARD10, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_PRIOR_TAG, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_NEXT_TAG, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_SEEKTO, wxCommandEvent);
wxDEFINE_EVENT(EVT_SEQUENCE_REPLAY_SECTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_SHOW_SELECTED_EFFECTS, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMPORT_TIMING, wxCommandEvent);
wxDEFINE_EVENT(EVT_IMPORT_NOTES, wxCommandEvent);
wxDEFINE_EVENT(EVT_AI_LYRICS, wxCommandEvent);
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

BEGIN_EVENT_TABLE(xLightsFrame, wxFrame)
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
EVT_COMMAND(wxID_ANY, EVT_COPY_MODEL_EFFECTS_TO_MODELS, xLightsFrame::CopyModelEffectsToModels)
EVT_COMMAND(wxID_ANY, EVT_PASTE_MODEL_EFFECTS, xLightsFrame::PasteModelEffects)
EVT_COMMAND(wxID_ANY, EVT_PASTE_MODEL_EFFECTS_WITH_SUB_LAYERS, xLightsFrame::PasteModelEffectsWithSubModelLayers)
EVT_COMMAND(wxID_ANY, EVT_MODEL_SELECTED, xLightsFrame::ModelSelected)
EVT_COMMAND(wxID_ANY, EVT_PLAY_SEQUENCE, xLightsFrame::PlaySequence)
EVT_COMMAND(wxID_ANY, EVT_PAUSE_SEQUENCE, xLightsFrame::PauseSequence)
EVT_COMMAND(wxID_ANY, EVT_STOP_SEQUENCE, xLightsFrame::StopSequence)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_FIRST_FRAME, xLightsFrame::SequenceFirstFrame)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_LAST_FRAME, xLightsFrame::SequenceLastFrame)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_REWIND10, xLightsFrame::SequenceRewind10)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_FFORWARD10, xLightsFrame::SequenceFForward10)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_PRIOR_TAG, xLightsFrame::SequencePriorTag)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_NEXT_TAG, xLightsFrame::SequenceNextTag)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_SEEKTO, xLightsFrame::SequenceSeekTo)
EVT_COMMAND(wxID_ANY, EVT_SEQUENCE_REPLAY_SECTION, xLightsFrame::SequenceReplaySection)
EVT_COMMAND(wxID_ANY, EVT_TOGGLE_PLAY, xLightsFrame::TogglePlay)
EVT_COMMAND(wxID_ANY, EVT_SHOW_DISPLAY_ELEMENTS, xLightsFrame::ShowDisplayElements)
EVT_COMMAND(wxID_ANY, EVT_SHOW_SELECTED_EFFECTS, xLightsFrame::ShowHideSelectEffectsWindow)
EVT_COMMAND(wxID_ANY, EVT_IMPORT_TIMING, xLightsFrame::ExecuteImportTimingElement)
EVT_COMMAND(wxID_ANY, EVT_IMPORT_NOTES, xLightsFrame::ExecuteImportNotes)
EVT_COMMAND(wxID_ANY, EVT_AI_LYRICS, xLightsFrame::GenerateAILyrics)
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
    int size = EffectsToolBar->FromDIP(16);
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

inline wxBitmapBundle GetToolbarBitmapBundle(const wxString& id)
{
    return wxArtProvider::GetBitmapBundle(id, wxART_TOOLBAR);
}

inline wxBitmapBundle GetMenuItemBitmapBundle(const wxString& id)
{
    return wxArtProvider::GetBitmapBundle(id, wxART_MENU);
}

inline wxBitmapBundle GetOtherBitmapBundle(const wxString& id)
{
    return wxArtProvider::GetBitmapBundle(id, wxART_OTHER);
}

inline wxBitmapBundle GetButtonBitmapBundle(const wxString& id)
{
    return wxArtProvider::GetBitmapBundle(id, wxART_BUTTON);
}

#ifdef __WXOSX__
const long NEWINSTANCE_ID = wxNewId();

class xlMacDockIcon : public wxTaskBarIcon
{
public:
    xlMacDockIcon(xLightsFrame* f) :
        wxTaskBarIcon(wxTBI_DOCK), _frame(f)
    {
    }

    virtual wxMenu* CreatePopupMenu() override
    {
        wxMenu* menu = new wxMenu;
        menu->Append(NEWINSTANCE_ID, "Open New Instance");
        return menu;
    }

    void OnMenuOpenNewInstance(wxCommandEvent& e)
    {
        _frame->OnMenuItem_File_NewXLightsInstance(e);
    }

    xLightsFrame* _frame;
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(xlMacDockIcon, wxTaskBarIcon)
    EVT_MENU(NEWINSTANCE_ID, xlMacDockIcon::OnMenuOpenNewInstance)
        wxEND_EVENT_TABLE()
#endif

            struct SplashScreenShow {
    SplashScreenShow(bool suppress)
    {
        if (!suppress) {
            splash = new SplashDialog(nullptr);
        }
    }

    ~SplashScreenShow()
    {
        if (splash) {
            delete splash;
            splash = nullptr;
        }
    }

    void Show()
    {
        if (splash)
            splash->Show();
    }

    void Hide()
    {
        if (splash)
            splash->Hide();
    }

    void Update()
    {
        if (splash)
            splash->Update();
    }

    SplashDialog* splash = nullptr;
};

xLightsFrame *xLightsFrame::GetFrame() {
    return xLightsApp::__frame;
}

xLightsFrame::xLightsFrame(wxWindow* parent, int ab, wxWindowID id, bool renderOnlyMode) :
    _presetSequenceElements(this),
    _renderMode(renderOnlyMode),
    jobPool("RenderPool"),
    _sequenceElements(this),
    AllModels(&_outputManager, static_cast<RenderContext*>(this)),
    AllObjects(static_cast<RenderContext*>(this)),
    color_mgr(this)
{
    
    spdlog::debug("xLightsFrame being constructed.");

    xLightsApp::__frame = this;

    CurlManager::INSTANCE.setYieldFunction([] { wxYieldIfNeeded(); });

    OutputManager::SetConfirmCallback([](const std::string& message, const std::string& title) -> bool {
        return wxMessageBox(message, title, wxICON_QUESTION | wxYES_NO) == wxYES;
    });

    AppCallbacks::SetDisplayMessageCallback([](AppCallbacks::DisplayMessageLevel level, const std::string& msg) {
        auto showBox = [level, msg]() {
            switch (level) {
            case AppCallbacks::DisplayMessageLevel::Error:
                wxMessageBox(msg, "Error", wxICON_ERROR | wxOK);
                break;
            case AppCallbacks::DisplayMessageLevel::Warning:
                wxMessageBox(msg, "Warning", wxICON_WARNING | wxOK);
                break;
            case AppCallbacks::DisplayMessageLevel::Info:
                wxMessageBox(msg, "Information", wxICON_INFORMATION | wxOK);
                break;
            }
        };
        if (wxThread::IsMain()) {
            showBox();
        } else {
            AppCallbacks::PostToMainThread(showBox);
        }
    });

    ValueCurve::SetSequenceElements(&_sequenceElements);

    // Initialize the GL context manager for shader rendering.
    {
        GLContextManager::InitParams glParams;
#ifdef _WIN32
        // Windows: provide main-thread runner and lazy shared-context accessor
        glParams.mainThreadRunner = [this](std::function<void()> fn) {
            if (wxThread::IsMain()) {
                // Already on the main thread; CallAfter + wait would deadlock.
                fn();
                return;
            }
            std::mutex mtx;
            std::condition_variable cv;
            bool done = false;
            CallAfter([&]() {
                fn();
                {
                    std::lock_guard<std::mutex> lk(mtx);
                    done = true;
                }
                cv.notify_all();
            });
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&] { return done; });
        };
        glParams.getSharedGLContext = []() -> void* {
            auto* ctx = xlGLCanvas::GetSharedContext();
            return ctx ? (void*)ctx->GetGLRC() : nullptr;
        };
#elif defined(__APPLE__)
        // macOS: tell ANGLE to use the same Metal GPU as the compute effects
#ifdef USE_GLES
        glParams.metalDeviceRegistryID = GetMetalComputeDeviceRegistryID();
#endif
#else
        // Linux: GLContextManager creates its own pure GLX+Pbuffer contexts,
        // completely independent of the wx canvas hierarchy.
        // No callbacks needed.
#endif
        GLContextManager::Instance().Initialize(glParams);
    }

    _renderEngine = std::make_unique<RenderEngine>(*this, jobPool, _renderCache);
    _renderEngine->SetOnRenderStatusTimerStart([this]() { RenderStatusTimer.Start(100, false); });
    _renderEngine->SetOnRenderJobComplete([this](const std::string& modelName) {
        CallAfter(&xLightsFrame::SetStatusText, wxString("Done Rendering \"" + modelName + "\""), 0);
    });
    _renderEngine->SetOnAllRenderJobsComplete([this]() { CallAfter(&xLightsFrame::RenderDone); });

    _exiting = false;
    SplashScreenShow splash(renderOnlyMode);
    splash.Show();
    splash.Update();
    wxYield();

    mCurrentPerpective = nullptr;
    MenuItemPreviews = nullptr;
    _renderMode = renderOnlyMode;
    _checkSequenceMode = false;
    _suspendAutoSave = false;
    _sequenceViewManager.SetModelManager(&AllModels);
    _pingTimer = new wxTimer(this, wxID_ANY);
    Bind(wxEVT_TIMER, &xLightsFrame::OnPingTimer, this, _pingTimer->GetId());
    _statusRefreshTimer = new wxTimer(this, wxID_ANY);
    Bind(wxEVT_TIMER, &xLightsFrame::StatusRefreshTimer, this, _statusRefreshTimer->GetId());
    Bind(EVT_SELECTED_EFFECT_CHANGED, &xLightsFrame::SelectedEffectChanged, this);
    Bind(EVT_RENDER_RANGE, &xLightsFrame::RenderRange, this);
    wxHTTP::Initialize();

    //(*Initialize(xLightsFrame)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizerNetworks;
    wxFlexGridSizer* FlexGridSizerPreview;
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
    WindowMgmtToolbar->AddTool(ID_AUITOOLBARITEM10, _("Select Effects"), GetToolbarBitmapBundle("xlART_EFFECTASSISTANT"), wxNullBitmap, wxITEM_NORMAL, _("Select Effects"), wxEmptyString, NULL);
    WindowMgmtToolbar->Realize();
    MainAuiManager->AddPane(WindowMgmtToolbar, wxAuiPaneInfo().Name(_T("Windows Tool Bar")).ToolbarPane().Caption(_("Windows Tool Bar")).CloseButton(false).Layer(10).Position(12).Top().Gripper());
    EditToolBar = new xlAuiToolBar(this, ID_AUITOOLBAR_EDIT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    EditToolBar->AddTool(ID_PASTE_BY_TIME, _("Paste By Time"), GetToolbarBitmapBundle("xlART_PASTE_BY_TIME"), wxNullBitmap, wxITEM_CHECK, _("Paste By Time"), wxEmptyString, NULL);
    EditToolBar->AddTool(ID_PASTE_BY_CELL, _("Paste By Cell"), GetToolbarBitmapBundle("xlART_PASTE_BY_CELL"), wxNullBitmap, wxITEM_CHECK, _("Paste By Cell"), wxEmptyString, NULL);
    EditToolBar->Realize();
    MainAuiManager->AddPane(EditToolBar, wxAuiPaneInfo().Name(_T("Edit Tool Bar")).ToolbarPane().Caption(_("Pane caption")).CloseButton(false).Layer(10).Position(5).Top().Gripper());
    ACToolbar = new xlAuiToolBar(this, ID_AUITOOLBAR_AC, wxPoint(1,30), wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
    ChoiceParm1 = new wxChoice(ACToolbar, ID_CHOICE_PARM1, wxPoint(276,12), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PARM1"));
    ChoiceParm1->SetSelection( ChoiceParm1->Append(_T("0")) );
    ChoiceParm1->Append(_T("10"));
    ChoiceParm1->Append(_T("20"));
    ChoiceParm1->Append(_T("25"));
    ChoiceParm1->Append(_T("30"));
    ChoiceParm1->Append(_T("33"));
    ChoiceParm1->Append(_T("40"));
    ChoiceParm1->Append(_T("50"));
    ChoiceParm1->Append(_T("60"));
    ChoiceParm1->Append(_T("66"));
    ChoiceParm1->Append(_T("70"));
    ChoiceParm1->Append(_T("75"));
    ChoiceParm1->Append(_T("80"));
    ChoiceParm1->Append(_T("90"));
    ChoiceParm1->Append(_T("100"));
    ChoiceParm2 = new wxChoice(ACToolbar, ID_CHOICE_PARM2, wxPoint(476,11), wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PARM2"));
    ChoiceParm2->Append(_T("0"));
    ChoiceParm2->Append(_T("10"));
    ChoiceParm2->Append(_T("20"));
    ChoiceParm2->Append(_T("25"));
    ChoiceParm2->Append(_T("30"));
    ChoiceParm2->Append(_T("33"));
    ChoiceParm2->Append(_T("40"));
    ChoiceParm2->Append(_T("50"));
    ChoiceParm2->Append(_T("60"));
    ChoiceParm2->Append(_T("66"));
    ChoiceParm2->Append(_T("70"));
    ChoiceParm2->Append(_T("75"));
    ChoiceParm2->Append(_T("80"));
    ChoiceParm2->Append(_T("90"));
    ChoiceParm2->SetSelection( ChoiceParm2->Append(_T("100")) );
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
    Button_ChangeShowDirPermanently = new wxButton(PanelSetup, ID_BUTTON3, _("Change Permanently"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    GridBagSizer1->Add(Button_ChangeShowDirPermanently, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_CheckShowFolderTemporarily = new wxButton(PanelSetup, ID_BUTTON11, _("Change Temporarily"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    GridBagSizer1->Add(Button_CheckShowFolderTemporarily, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ChangeTemporarilyAgain = new wxButton(PanelSetup, ID_BUTTON13, _("Change Temporarily Again"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    Button_ChangeTemporarilyAgain->Hide();
    GridBagSizer1->Add(Button_ChangeTemporarilyAgain, wxGBPosition(0, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ShowDirectoryLabel = new wxStaticText(PanelSetup, ID_STATICTEXT4, _("{Show Directory not set}"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    GridBagSizer1->Add(ShowDirectoryLabel, wxGBPosition(0, 4), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_BaseShowDirLabel = new wxStaticText(PanelSetup, ID_STATICTEXT2, _("Base Show Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    GridBagSizer1->Add(StaticText_BaseShowDirLabel, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_ChangeBaseShowDir = new wxButton(PanelSetup, ID_BUTTON14, _("Change"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON14"));
    FlexGridSizer2->Add(Button_ChangeBaseShowDir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_OpenBaseShowDir = new wxButton(PanelSetup, ID_BUTTON17, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON17"));
    Button_OpenBaseShowDir->SetToolTip(_("Open the base show folder"));
    FlexGridSizer2->Add(Button_OpenBaseShowDir, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizer1->Add(FlexGridSizer2, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ClearBaseShowDir = new wxButton(PanelSetup, ID_BUTTON15, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON15"));
    GridBagSizer1->Add(Button_ClearBaseShowDir, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_BaseShowDir = new wxStaticText(PanelSetup, ID_STATICTEXT3, _("No base show directory"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    GridBagSizer1->Add(StaticText_BaseShowDir, wxGBPosition(1, 4), wxDefaultSpan, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_AutoUpdateBase = new wxCheckBox(PanelSetup, ID_CHECKBOX1, _("Auto Update On Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_AutoUpdateBase->SetValue(false);
    FlexGridSizer1->Add(CheckBox_AutoUpdateBase, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_UpdateBase = new wxButton(PanelSetup, ID_BUTTON16, _("Update"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON16"));
    FlexGridSizer1->Add(Button_UpdateBase, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizer1->Add(FlexGridSizer1, wxGBPosition(1, 3), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridBagSizer1->AddGrowableCol(4);
    StaticBoxSizer1->Add(GridBagSizer1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelSetup, _("Controllers"));
    FlexGridSizerNetworks = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizerNetworks->AddGrowableCol(2);
    FlexGridSizerNetworks->AddGrowableRow(0);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    ButtonSaveSetup = new wxButton(PanelSetup, ID_BUTTON_SAVE_SETUP, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_SETUP"));
    BoxSizer1->Add(ButtonSaveSetup, 1, wxALL|wxEXPAND, 3);
    BoxSizer1->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    ButtonAddControllerSerial = new wxButton(PanelSetup, ID_BUTTON9, _("Add USB"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    BoxSizer1->Add(ButtonAddControllerSerial, 1, wxALL|wxEXPAND, 3);
    ButtonAddControllerEthernet = new wxButton(PanelSetup, ID_BUTTON6, _("Add Ethernet"), wxDefaultPosition, wxSize(98,28), 0, wxDefaultValidator, _T("ID_BUTTON6"));
    BoxSizer1->Add(ButtonAddControllerEthernet, 1, wxALL|wxEXPAND, 3);
    ButtonAddControllerNull = new wxButton(PanelSetup, ID_BUTTON10, _("Add Null"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    BoxSizer1->Add(ButtonAddControllerNull, 1, wxALL|wxEXPAND, 3);
    BoxSizer1->Add(-1,-1,1, wxALL|wxEXPAND, 5);
    ButtonDiscover = new wxButton(PanelSetup, ID_BUTTON5, _("Discover"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    BoxSizer1->Add(ButtonDiscover, 1, wxALL|wxEXPAND, 3);
    ButtonFPPConnect = new wxButton(PanelSetup, ID_BUTTON18, _("FPP Connect"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON18"));
    BoxSizer1->Add(ButtonFPPConnect, 1, wxALL|wxEXPAND, 3);
    FlexGridSizerNetworks->Add(BoxSizer1, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 0);
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    BitmapButtonMoveNetworkUp = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON1, GetButtonBitmapBundle("wxART_GO_UP"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    BitmapButtonMoveNetworkUp->SetToolTip(_("Move selected item up"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkUp, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButtonMoveNetworkDown = new wxBitmapButton(PanelSetup, ID_BITMAPBUTTON2, GetButtonBitmapBundle("wxART_GO_DOWN"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    BitmapButtonMoveNetworkDown->SetToolTip(_("Move selected item down"));
    FlexGridSizer9->Add(BitmapButtonMoveNetworkDown, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerNetworks->Add(FlexGridSizer9, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_TOP, 10);
    Panel2 = new wxPanel(PanelSetup, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizerSetupControllers = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupControllers->AddGrowableCol(0);
    FlexGridSizerSetupControllers->AddGrowableRow(0);
    Panel2->SetSizer(FlexGridSizerSetupControllers);
    FlexGridSizerNetworks->Add(Panel2, 1, wxALL|wxEXPAND, 2);
    Panel5 = new wxPanel(PanelSetup, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxALWAYS_SHOW_SB|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL3"));
    Panel5->SetMinSize(wxSize(200,-1));
    Panel5->SetMaxSize(wxSize(200,-1));
    FlexGridSizerSetupRight = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupRight->AddGrowableCol(0);
    FlexGridSizerSetupRight->AddGrowableRow(0);
    FlexGridSizerSetupProperties = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizerSetupProperties->AddGrowableCol(0);
    FlexGridSizerSetupProperties->AddGrowableRow(0);
    FlexGridSizerSetupRight->Add(FlexGridSizerSetupProperties, 1, wxALL|wxEXPAND, 0);
    FlexGridSizerSetupControllerButtons = new wxFlexGridSizer(2, 0, 0, 0);
    ButtonVisualise = new wxButton(Panel5, ID_BUTTON1, _("Visualise ..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizerSetupControllerButtons->Add(ButtonVisualise, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonUploadInput = new wxButton(Panel5, ID_BUTTON2, _("Upload Input"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizerSetupControllerButtons->Add(ButtonUploadInput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonOpen = new wxButton(Panel5, ID_BUTTON7, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizerSetupControllerButtons->Add(ButtonOpen, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextDummy = new wxStaticText(Panel5, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizerSetupControllerButtons->Add(StaticTextDummy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonControllerDelete = new wxButton(Panel5, ID_BUTTON8, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizerSetupControllerButtons->Add(ButtonControllerDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonUploadOutput = new wxButton(Panel5, ID_BUTTON4, _("Upload Output"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizerSetupControllerButtons->Add(ButtonUploadOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_OpenProxy = new wxButton(Panel5, ID_BUTTON12, _("Open Proxy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizerSetupControllerButtons->Add(Button_OpenProxy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizerSetupRight->Add(FlexGridSizerSetupControllerButtons, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
    Panel5->SetSizer(FlexGridSizerSetupRight);
    FlexGridSizerNetworks->Add(Panel5, 1, wxALL|wxEXPAND, 2);
    StaticBoxSizer2->Add(FlexGridSizerNetworks, 1, wxALL|wxEXPAND, 5);
    FlexGridSizerSetup->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    PanelSetup->SetSizer(FlexGridSizerSetup);
    PanelPreview = new wxPanel(Notebook1, ID_PANEL_PREVIEW, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_PREVIEW"));
    FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizerPreview->AddGrowableCol(0);
    FlexGridSizerPreview->AddGrowableRow(0);
    PanelPreview->SetSizer(FlexGridSizerPreview);
    PanelSequencer = new wxPanel(Notebook1, XLIGHTS_SEQUENCER_TAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("XLIGHTS_SEQUENCER_TAB"));
    m_mgr = new wxAuiManager(PanelSequencer, wxAUI_MGR_ALLOW_FLOATING|wxAUI_MGR_DEFAULT);
    Notebook1->AddPage(PanelSetup, _("Controllers"), true);
    Notebook1->AddPage(PanelPreview, _("Layout"));
    Notebook1->AddPage(PanelSequencer, _("Sequencer"));
    MainAuiManager->AddPane(Notebook1, wxAuiPaneInfo().Name(_T("MainPain")).CenterPane().Caption(_("Pane caption")).PaneBorder(false));
    AUIStatusBar = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    StatusBarSizer = new wxGridBagSizer(0, 0);
    StatusBarSizer->Add(8,0,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    StatusText = new wxStaticText(AUIStatusBar, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    StatusBarSizer->Add(StatusText, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxEXPAND, 2);
    Panel1 = new wxPanel(AUIStatusBar, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    Panel1->SetMinSize(wxDLG_UNIT(AUIStatusBar,wxSize(100,-1)));
    GaugeSizer = new wxFlexGridSizer(1, 1, 0, 0);
    GaugeSizer->AddGrowableCol(0);
    ProgressBar = new wxGauge(Panel1, ID_GAUGE1, 100, wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(100,-1)), 0, wxDefaultValidator, _T("ID_GAUGE1"));
    GaugeSizer->Add(ProgressBar, 0, wxEXPAND, 0);
    Panel1->SetSizer(GaugeSizer);
    StatusBarSizer->Add(Panel1, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FileNameText = new wxStaticText(AUIStatusBar, ID_STATICTEXT7, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    StatusBarSizer->Add(FileNameText, wxGBPosition(0, 3), wxDefaultSpan, wxALL|wxEXPAND, 2);
    StatusBarSizer->AddGrowableRow(0);
    AUIStatusBar->SetSizer(StatusBarSizer);
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
    MenuItem_REDO = new wxMenuItem(Menu3, wxID_REDO, _("Redo\tCtrl-y"), wxEmptyString, wxITEM_NORMAL);
    MenuItem_REDO->SetBitmap(GetMenuItemBitmapBundle("wxART_REDO"));
    Menu3->Append(MenuItem_REDO);
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
    MenuItemShiftEffectsAndTiming = new wxMenuItem(Menu3, ID_SHIFT_EFFECTS_AND_TIMING, _("Shift Effects And Timing"), _("Use this options to shift all timing and effects in the sequence."), wxITEM_NORMAL);
    Menu3->Append(MenuItemShiftEffectsAndTiming);
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
    Menu_GenerateAIImage = new wxMenuItem(Menu1, ID_MENUITEM_GenerateAIImage, _("Generate AI Image"), _("Create images using AI - if configured."), wxITEM_NORMAL);
    Menu1->Append(Menu_GenerateAIImage);
    MenuItem_GenerateLyrics = new wxMenuItem(Menu1, ID_MNU_GENERATELYRICS, _("Generate &Lyrics From Data"), _("Generate lyric phenomes from data"), wxITEM_NORMAL);
    Menu1->Append(MenuItem_GenerateLyrics);
    MenuItemConvert = new wxMenuItem(Menu1, ID_MENUITEM_CONVERT, _("&Convert"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemConvert);
    MenuItem_PrepareAudio = new wxMenuItem(Menu1, ID_MNU_PREPAREAUDIO, _("Prepare Audio"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_PrepareAudio);
    MenuItemUserDict = new wxMenuItem(Menu1, ID_MENU_USER_DICT, _("User Lyric Dictionary"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemUserDict);
    MenuItemFindShowFolder = new wxMenuItem(Menu1, ID_MENU_FIND_SHOW_FOLDER, _("Search for Show Folders"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItemFindShowFolder);
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
    MenuItemEffectPresets = new wxMenuItem(MenuItem18, ID_MENUITEM_EFFECT_PRESETS, _("Effect Presets"), wxEmptyString, wxITEM_CHECK);
    MenuItem18->Append(MenuItemEffectPresets);
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
    MenuItem_Zoom = new wxMenuItem(MenuHelp, ID_MNU_ZOOM, _("Zoom Room Help"), _("Access the free Zoom Room for Help"), wxITEM_NORMAL);
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

    Connect(ID_AUITOOLBAR_OPENSHOW, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_AUITOOLBAR_NEWSEQUENCE, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_AUITOOLBAR_OPEN, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnBitmapButtonOpenSeqClick);
    Connect(ID_AUITOOLBAR_SAVE, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnBitmapButtonSaveSeqClick);
    Connect(ID_AUITOOLBAR_SAVEAS, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonClickSaveAs);
    Connect(ID_AUITOOLBAR_RENDERALL, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemRenderAllClick);
    Connect(ID_AUITOOLBAR_PLAY_NOW, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPlayButtonClick);
    Connect(ID_AUITOOLBAR_PAUSE, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPauseButtonClick);
    Connect(ID_AUITOOLBAR_STOP, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemStopClick);
    Connect(ID_AUITOOLBAR_FIRST_FRAME, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarFirstFrameClick);
    Connect(ID_AUITOOLBAR_LAST_FRAME, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarLastFrameClick);
    Connect(ID_AUITOOLBAR_REPLAY_SECTION, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemReplaySectionClick);
    Connect(ID_CHECKBOX_LIGHT_OUTPUT, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnCheckBoxLightOutputClick);
    Connect(ID_AUITOOLBARITEM2, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_AUITOOLBARITEM5, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_AUITOOLBARITEM7, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideBufferSettingsWindow);
    Connect(ID_AUITOOLBARITEM3, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideLayerBlendingWindow);
    Connect(ID_TOGGLE_MODEL_PREVIEW, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_TOGGLE_HOUSE_PREVIEW, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_AUITOOLBARITEM6, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideDisplayElementsWindow);
    Connect(ID_AUITOOLBARITEM8, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemShowHideEffects);
    Connect(ID_AUITOOLBARITEM9, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectAssistWindow);
    Connect(ID_AUITOOLBARITEM10, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::ShowHideSelectEffectsWindow);
    Connect(ID_PASTE_BY_TIME, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPasteByTimeClick);
    Connect(ID_PASTE_BY_CELL, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemPasteByCellClick);
    Connect(ID_AUITOOLBARITEM_ACDISABLED, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_DisableClick);
    Connect(ID_AUITOOLBARITEM_ACSELECT, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_SelectClick);
    Connect(ID_AUITOOLBARITEM_ACOFF, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_OffClick);
    Connect(ID_AUITOOLBARITEM_ACON, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_OnClick);
    Connect(ID_AUITOOLBARITEM_ACSHIMMER, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_ShimmerClick);
    Connect(ID_AUITOOLBARITEM_ACTWINKLE, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_TwinkleClick);
    Connect(ID_AUITOOLBARITEM_ACINTENSITY, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_IntensityClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPUP, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_RampUpClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPDOWN, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_RampDownClick);
    Connect(ID_AUITOOLBARITEM_ACRAMPUPDOWN, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_RampUpDownClick);
    Connect(ID_CHOICE_PARM1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnChoiceParm1Select);
    Connect(ID_CHOICE_PARM2, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnChoiceParm2Select);
    Connect(ID_AUITOOLBARITEM_ACFILL, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_FillClick);
    Connect(ID_AUITOOLBARITEM_ACCASCADE, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_CascadeClick);
    Connect(ID_AUITOOLBARITEM_ACFOREGROUND, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_ForegroundClick);
    Connect(ID_AUITOOLBARITEM_ACBACKGROUND, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAC_BackgroundClick);
    Connect(ID_AUITOOLBAR_AC, wxEVT_COMMAND_AUITOOLBAR_TOOL_DROPDOWN, (wxObjectEventFunction)&xLightsFrame::OnACToolbarDropdown);
    Connect(wxID_ZOOM_IN, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(wxID_ZOOM_OUT, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_AUITOOLBARITEM14, wxEVT_COMMAND_TOOL_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_BUTTON11, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_ChangeShowFolderTemporarily);
    Connect(ID_BUTTON13, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_ChangeTemporarilyAgainClick);
    Connect(ID_BUTTON14, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_ChangeBaseShowDirClick);
    Connect(ID_BUTTON17, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_OpenBaseShowDirClick);
    Connect(ID_BUTTON15, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_ClearBaseShowDirClick);
    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnCheckBox_AutoUpdateBaseClick);
    Connect(ID_BUTTON16, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_UpdateBaseClick);
    Connect(ID_BUTTON_SAVE_SETUP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonSaveSetupClick);
    Connect(ID_BUTTON9, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerSerialClick);
    Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerEthernetClick);
    Connect(ID_BUTTON10, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonAddControllerNullClick);
    Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonDiscoverClick);
    Connect(ID_BUTTON18, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonFPPConnectClick);
    Connect(ID_BITMAPBUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveUpClick);
    Connect(ID_BITMAPBUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonNetworkMoveDownClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonVisualiseClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonUploadInputClick);
    Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonOpenClick);
    Connect(ID_BUTTON8, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonControllerDeleteClick);
    Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButtonUploadOutputClick);
    Connect(ID_BUTTON12, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xLightsFrame::OnButton_OpenProxyClick);
    m_mgr->Connect(wxEVT_AUI_PANE_CLOSE, (wxObjectEventFunction)&xLightsFrame::OnPaneClose, NULL, this);
    PanelSequencer->Connect(wxEVT_PAINT, (wxObjectEventFunction)&xLightsFrame::OnPanelSequencerPaint, NULL, this);
    Connect(ID_NOTEBOOK1, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged1);
    Connect(ID_NOTEBOOK1, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, (wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanging);
    Connect(ID_NEW_SEQUENCE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnButtonNewSequenceClick);
    Connect(ID_OPEN_SEQUENCE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Open_SequenceSelected);
    Connect(IS_SAVE_SEQ, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Save_Selected);
    Connect(ID_SAVE_AS_SEQUENCE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected);
    Connect(ID_CLOSE_SEQ, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Close_SequenceSelected);
    Connect(wxID_PREFERENCES, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemPreferencesSelected);
    Connect(ID_SEQ_SETTINGS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenu_Settings_SequenceSelected);
    Connect(ID_MNU_KEYBINDINGS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_KeyBindingsSelected);
    Connect(ID_EXPORT_VIDEO, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_Export_VideoSelected);
    Connect(ID_MENUITEM2, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuOpenFolderSelected);
    Connect(ID_FILE_BACKUP, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemBackupSelected);
    Connect(ID_FILE_RESTOREBACKUP, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemRestoreBackupSelected);
    Connect(ID_FILE_ALTBACKUP, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnmAltBackupMenuItemSelected);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnQuit);
    Connect(ID_SHIFT_EFFECTS_AND_TIMING, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemShiftEffectsAndTimingSelected);
    Connect(ID_SHIFT_EFFECTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemShiftEffectsSelected);
    Connect(ID_MNU_SHIFT_SELECTED_EFFECTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemShiftSelectedEffectsSelected);
    Connect(ID_MNU_COLOURREPLACE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ColorReplaceSelected);
    Connect(ID_MENUITEM13, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnActionTestMenuItemSelected);
    Connect(ID_MNU_CHECKSEQ, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemCheckSequenceSelected);
    Connect(ID_MNU_CLEANUPFILE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_CleanupFileLocationsSelected);
    Connect(ID_MNU_PACKAGESEQUENCE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_PackageSequenceSelected);
    Connect(ID_MNU_DOWNLOADSEQUENCES, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_DownloadSequencesSelected);
    Connect(ID_MENU_BATCH_RENDER, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemBatchRenderSelected);
    Connect(ID_MENU_FPP_CONNECT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_FPP_ConnectSelected);
    Connect(ID_MNU_BULKUPLOAD, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemBulkControllerUploadSelected);
    Connect(ID_MENU_HINKSPIX_EXPORT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemHinksPixExportSelected);
    Connect(ID_MENU_RUN_SCRIPT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemRunScriptSelected);
    Connect(ID_EXPORT_MODELS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnmExportModelsMenuItemSelected);
    Connect(ID_MNU_EXPORT_EFFECTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ExportEffectsSelected);
    Connect(ID_MNU_EXPORT_CONTROLLER_CONNECTIONS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ExportControllerConnectionsSelected);
    Connect(ID_MENU_VIEW_LOG, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MENUITEM18, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemPackageDebugFiles);
    Connect(iD_MNU_VENDORCACHEPURGE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_PurgeVendorCacheSelected);
    Connect(ID_MNU_PURGERENDERCACHE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_PurgeRenderCacheSelected);
    Connect(ID_MNU_CRASH, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_CrashXLightsSelected);
    Connect(ID_MNU_DUMPRENDERSTATE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_LogRenderStateSelected);
    Connect(ID_MENU_GENERATE2DPATH, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Generate2DPathSelected);
    Connect(ID_MENUITEM_GenerateCustomModel, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenu_GenerateCustomModelSelected);
    Connect(ID_MNU_REMAPCUSTOM, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_RemapCustomSelected);
    Connect(ID_MENUITEM_GenerateAIImage, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_GenerateAIImageSelected);
    Connect(ID_MNU_GENERATELYRICS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_GenerateLyricsSelected);
    Connect(ID_MENUITEM_CONVERT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemConvertSelected);
    Connect(ID_MNU_PREPAREAUDIO, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_PrepareAudioSelected);
    Connect(ID_MENU_USER_DICT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemUserDictSelected);
    Connect(ID_MENU_FIND_SHOW_FOLDER, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemFindShowFolderSelected);
    Connect(wxID_ZOOM_IN, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItemZoominClick);
    Connect(wxID_ZOOM_OUT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnAuiToolBarItem_ZoomOutClick);
    Connect(ID_MENUITEM5, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ResetToolbarLocations);
    Connect(MNU_ID_ACLIGHTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ACLIghtsSelected);
    Connect(ID_MNU_SHOWRAMPS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ShowACRampsSelected);
    Connect(ID_MENUITEM_SAVE_PERSPECTIVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemViewSavePerspectiveSelected);
    Connect(ID_MENUITEM_SAVE_AS_PERSPECTIVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemViewSaveAsPerspectiveSelected);
    Connect(ID_MENUITEM_LOAD_PERSPECTIVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemLoadEditPerspectiveSelected);
    Connect(ID_MNU_PERSPECTIVES_AUTOSAVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_PerspectiveAutosaveSelected);
    Connect(ID_MENUITEM_DISPLAY_ELEMENTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideDisplayElementsWindow);
    Connect(ID_MENU_TOGGLE_MODEL_PREVIEW, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideModelPreview);
    Connect(ID_MENU_TOGGLE_HOUSE_PREVIEW, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideHousePreview);
    Connect(ID_MENUITEM14, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectSettingsWindow);
    Connect(ID_MENUITEM15, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideColorWindow);
    Connect(ID_MENUITEM16, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideLayerBlendingWindow);
    Connect(ID_MENUITEM9, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideBufferSettingsWindow);
    Connect(ID_MENUITEM17, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectDropper);
    Connect(ID_MNU_VALUECURVES, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ValueCurvesSelected);
    Connect(ID_MNU_COLOURDROPPER, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ColourDropperSelected);
    Connect(ID_MENUITEM_EFFECT_ASSIST_WINDOW, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectAssistWindow);
    Connect(ID_MENUITEM_EFFECT_PRESETS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideEffectPresetsWindow);
    Connect(ID_MENUITEM_SELECT_EFFECT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemSelectEffectSelected);
    Connect(ID_MENUITEM_SEARCH_EFFECTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemSearchEffectsSelected);
    Connect(ID_MENUITEM_VIDEOPREVIEW, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemShowHideVideoPreview);
    Connect(ID_MNU_JUKEBOX, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_JukeboxSelected);
    Connect(ID_MNU_FINDDATA, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemFindDataSelected);
    Connect(ID_MNU_SUPPRESSDOCK_HP, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_SuppressDock);
    Connect(ID_MNU_SUPPRESSDOCK_MP, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_SuppressDock);
    Connect(ID_MENUITEM_WINDOWS_PERSPECTIVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHidePerspectivesWindow);
    Connect(ID_MENUITEM_WINDOWS_DOCKALL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuDockAllSelected);
    Connect(ID_MENUITEM11, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ResetWindowsToDefaultPositions);
    Connect(ID_PLAY_FULL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_1POINT5SPEED, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MN_2SPEED, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_3SPEED, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_4SPEED, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_3_4, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_2, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_PLAY_1_4, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::SetPlaySpeed);
    Connect(ID_MNU_LOUDVOLUME, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_LoudVolSelected);
    Connect(ID_MNU_MEDVOLUME, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_MedVolSelected);
    Connect(ID_MNU_QUIET, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_QuietVolSelected);
    Connect(ID_MNU_SUPERQUIET, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_VQuietVolSelected);
    Connect(ID_MNU_SILENT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_SilentVolSelected);
    Connect(ID_IMPORT_EFFECTS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItemImportEffects);
    Connect(ID_MNU_TOD, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_TODSelected);
    Connect(ID_MNU_MANUAL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_UserManualSelected);
    Connect(ID_MNU_ZOOM, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ZoomSelected);
    Connect(ID_MENUITEM1, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_ShowKeyBindingsSelected);
    Connect(idMenuHelpContent, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnBitmapButtonTabInfoClick);
    Connect(ID_MENU_HELP_FORMUM, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_ForumSelected);
    Connect(ID_MNU_VIDEOS, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_VideoTutorialsSelected);
    Connect(ID_MENU_HELP_DOWNLOAD, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_DownloadSelected);
    Connect(ID_MNU_HELP_RELEASE_NOTES, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_ReleaseNotesSelected);
    Connect(ID_MENU_HELP_ISSUE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_Isue_TrackerSelected);
    Connect(ID_MENU_HELP_FACEBOOK, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_Help_FacebookSelected);
    Connect(ID_MNU_DONATE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_DonateSelected);
    Connect(ID_MNU_UPDATE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_UpdateSelected);
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnAbout);
    Connect(ID_TIMER_OutputTimer, wxEVT_TIMER, (wxObjectEventFunction)&xLightsFrame::OnOutputTimerTrigger);
    Connect(ID_TIMER_AutoSave, wxEVT_TIMER, (wxObjectEventFunction)&xLightsFrame::OnTimer_AutoSaveTrigger);
    Connect(ID_TIMER_EFFECT_SETTINGS, wxEVT_TIMER, (wxObjectEventFunction)&xLightsFrame::OnEffectSettingsTimerTrigger);
    Connect(ID_TIMER_RENDERSTATUS, wxEVT_TIMER, (wxObjectEventFunction)&xLightsFrame::OnRenderStatusTimerTrigger);
    Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&xLightsFrame::OnClose);
    Connect(wxEVT_CHAR, (wxObjectEventFunction)&xLightsFrame::OnChar);
    //*)

    Notebook1->SetArtProvider(new wxAuiGenericTabArt());

    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        spdlog::error("Null config ... this wont end well.");
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
    spdlog::debug("Show directory {}.", (const char*)dir.c_str());

#if !defined(_DEBUG)
    if (dir != "") {
#ifdef __WXMSW__
        _tod.PrepTipOfDay(this);
#else
        _tod = new TipOfTheDayDialog("", this);
        _tod->PrepTipOfDay(this);
#endif
    }
#endif
    Connect(wxEVT_HELP, (wxObjectEventFunction)&xLightsFrame::OnHelp);
    Notebook1->Connect(wxEVT_HELP, (wxObjectEventFunction)&xLightsFrame::OnHelp, 0, this);

    spdlog::debug("xLightsFrame constructor UI code done.");

    GaugeSizer->Fit(Panel1);
    GaugeSizer->SetSizeHints(Panel1);
    StatusBarSizer->Fit(AUIStatusBar);
    StatusBarSizer->SetSizeHints(AUIStatusBar);

    LoadDockable();

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(xLightsFrame::OnCharHook), nullptr, this);

    // Suppress OSX display of a warning when reading config ... "entry %s appears more than once in group '%s'
    wxLogNull logNo;

    // need to direct these menu items to different places depending on what is active
    Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_REDO, wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::DoMenuAction);
    Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::DoMenuAction);

    ShowDirectoryLabel->Bind(wxEVT_LEFT_DCLICK,
                             [&](wxMouseEvent&) {
                                 wxLaunchDefaultApplication(showDirectory);
                             });

    SetPanelSequencerLabel("");

    _outputModelManager.SetCallbacks(
        [this]() { CallAfter(&xLightsFrame::DoASAPWork); },
        [this](uint32_t work, const std::string& type, BaseObject* m, const std::string& selectedModel) {
            DoWork(work, type, m, selectedModel);
        });

    mRendering = false;

    _appProgress = std::make_unique<wxAppProgressIndicator>(this);
    _appProgress->SetRange(100);
    _appProgress->Reset();

    AddEffectToolbarButtons(effectManager, EffectsToolBar);
    wxSize sz = EffectsToolBar->GetSize();
    wxAuiPaneInfo& info = MainAuiManager->GetPane("EffectsToolBar");
    info.BestSize(sz);
    MainAuiManager->Update();

    wxToolTip::SetAutoPop(20000); // globally set tooltips stay on screen for a long time - may not work on all platforms per wxWidgets documentation

    SetTitle(xlights_base_name + xlights_qualifier + " (Ver " + GetDisplayVersionString() + ") " + xlights_build_date);

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
    mSuppressColorWarn = false;
    me131Sync = false;
    mAltBackupDir = "";
    mIconSize = 16;
    _modelHandleSize = 1;
    _crosshairSize = 1;

    _acParm1Intensity = 100;
    _acParm1RampUp = 0;
    _acParm2RampUp = 100;
    _acParm1RampDown = 0;
    _acParm2RampDown = 100;
    _acParm1RampUpDown = 0;
    _acParm2RampUpDown = 100;
    ChoiceParm1->SetStringSelection("100");
    ChoiceParm2->SetStringSelection("100");

    StatusBarSizer->AddGrowableCol(1, 2);
    StatusBarSizer->AddGrowableCol(3, 1);
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

    spdlog::debug("Config: AppName '{}' Path '{}' Entries {} Groups {} Style {} Vendor {}.",
                      (const char*)config->GetAppName().c_str(),
                      (const char*)config->GetPath().c_str(),
                      (int)config->GetNumberOfEntries(),
                      (int)config->GetNumberOfGroups(),
                      config->GetStyle(),
                      (const char*)config->GetVendorName().c_str());

    xlColourData::INSTANCE.Load(config);
    config->Read("xLightsPlayControlsOnPreview", &_playControlsOnPreview, false);
    spdlog::debug("Play Controls On Preview: {}.", toStr(_playControlsOnPreview));

    config->Read("xLightsShowBaseFolder", &_showBaseShowFolder, false);
    spdlog::debug("Show base show folder controls: {}.", toStr(_showBaseShowFolder));
    SetShowBaseShowFolder(_showBaseShowFolder);

    config->Read("xLightsAutoShowHousePreview", &_autoShowHousePreview, false);
    spdlog::debug("Autoshow House Preview: {}.", toStr(_autoShowHousePreview));

    config->Read("xLightsZoomMethodToCursor", &_zoomMethodToCursor, true);
    spdlog::debug("Zoom Method To Cursor: {}.", toStr(_zoomMethodToCursor));

    config->Read("xLightsHidePresetPreview", &_hidePresetPreview, false);
    spdlog::debug("Hide Preset Preview: {}.", toStr(_hidePresetPreview));

    config->Read("xLightsSmallWaveform", &_smallWaveform, false);
    spdlog::debug("Small Waveform: {}.", toStr(_smallWaveform));

    config->Read("xLightsRenderBell", &_renderBellEnabled, false);
    spdlog::debug("Render Bell Enabled: {}.", toStr(_renderBellEnabled));

    config->Read("xLightsModelBlendDefaultOff", &_modelBlendDefaultOff, false);
    spdlog::debug("Model Blend Default Off: {}.", toStr(_modelBlendDefaultOff));

    config->Read("xLightsLowDefinitionRender", &_lowDefinitionRender, false);
    spdlog::debug("Low Defintion Render: {}.", toStr(_lowDefinitionRender));

    config->Read("xLightsSnapToTimingMarks", &_snapToTimingMarks, false);
    spdlog::debug("Snap To Timing Marks: {}.", toStr(_snapToTimingMarks));

    config->Read("xLightsFSEQVersion", &_fseqVersion, 5);
    spdlog::debug("FSEQ Save Version: {}.", _fseqVersion);

    // If upgrading from a version older than 2026.03.3, reset FSEQ version to 5 (V2 ZSTD/sparse)
    wxString prevVersion;
    config->Read("xLightsVersion", &prevVersion, "");
    if (prevVersion.empty() || IsVersionOlder("2026.03.3", prevVersion.ToStdString())) {
        _fseqVersion = 5;
        spdlog::info("Reset FSEQ version to 5 (V2 ZSTD/sparse) due to upgrade from version '{}'.", prevVersion.ToStdString());
    }

    config->Read("xLightsDisableKeyAccelerations", &_disableKeyAcceleration, false);
    spdlog::debug("Disable Key Accelerations: {}.", _disableKeyAcceleration ? "Y": "N");

    config->Read("xLightsTimelineZooming", &_timelineZooming, 0);
    config->Read("xLightsPlayVolume", &playVolume, 100);
    MenuItem_LoudVol->Check(playVolume == 100);
    MenuItem_MedVol->Check(playVolume == 66);
    MenuItem_QuietVol->Check(playVolume == 33);
    MenuItem_VQuietVol->Check(playVolume == 10);
    MenuItem_SilentVol->Check(playVolume == 0);
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);

    wxString randomEffects = "";
    config->Read("xLightsRandomEffects", &randomEffects);
    if (randomEffects.IsEmpty()) {
        for (int i = 0; i < (int)effectManager.size(); i++) {
            if (effectManager[i]->CanBeRandom()) {
                _randomEffectsToUse.Add(effectManager[i]->Name());
            }
        }
        randomEffects = wxJoin(_randomEffectsToUse, ',');
        config->Write("xLightsRandomEffects", randomEffects);
    } else {
        _randomEffectsToUse = wxSplit(randomEffects, ',');
    }

    spdlog::debug("xLightsFrame constructor creating sequencer.");

    CreateSequencer();

    spdlog::debug("xLightsFrame constructor sequencer creation done.");

    layoutPanel = new LayoutPanel(PanelPreview, this, PanelSequencer);
    spdlog::debug("LayoutPanel creation done.");
    FlexGridSizerPreview->Add(layoutPanel, 1, wxALL | wxEXPAND, 5);
    FlexGridSizerPreview->Fit(PanelPreview);
    FlexGridSizerPreview->SetSizeHints(PanelPreview);

    modelPreview = layoutPanel->GetMainPreview();
    spdlog::debug("LayoutPanel setup done.");

    playIcon = wxBitmap(control_play_blue_icon);
    pauseIcon = wxBitmap(control_pause_blue_icon);

    Grid1HasFocus = false; // set this before grid gets any events -DJ

    SetIcons(wxArtProvider::GetIconBundle("xlART_xLights_Icons", wxART_FRAME_ICON));
    spdlog::debug("IconBundle creation done.");

    SetName("xLights");
    wxPersistenceManager::Get().RegisterAndRestore(this);
    spdlog::debug("Window Location Restored.");

    effGridPrevX = 0;
    effGridPrevY = 0;
    mSavedChangeCount = 0;
    mLastAutosaveCount = 0;

    _scrollTimer.Connect(wxEVT_TIMER, wxTimerEventHandler(xLightsFrame::OnListItemScrollTimerControllers), nullptr, this);

    // get list of most recently used directories
    wxString dirmru;
    for (int i = 0; i < MRUD_LENGTH; i++) {
        std::string mru_name = "mru" + std::to_string(i);
        dirmru.clear();
        if (config->Read(mru_name, &dirmru)) {
            if (!dirmru.IsEmpty()) {
                int idx = mruDirectories.Index(dirmru);
                if (idx == wxNOT_FOUND)
                    mruDirectories.Add(dirmru);
            }
        }
        mrud_MenuItem[i] = nullptr;
    }
    for (int i = 0; i < MRUF_LENGTH; i++) {
        mruf_MenuItem[i] = nullptr;
    }
    MenuFile->FindItem(ID_MENUITEM_RECENTFOLDERS)->SetBitmap(GetMenuItemBitmapBundle("wxART_FOLDER_OPEN"));
    MenuFile->FindItem(ID_MENUITEM_OPENRECENTSEQUENCE)->SetBitmap(GetMenuItemBitmapBundle("wxART_FILE_OPEN"));

    spdlog::debug("xLightsFrame constructor loading config.");

    wxString md;

    if (!xLightsApp::mediaDir.IsNull()) {
        md = xLightsApp::mediaDir;
        spdlog::debug("Media directory {}.", (const char*)md.c_str());
        if (!ObtainAccessToURL(md)) {
            std::string mds = md;
            PromptForDirectorySelection("Reselect Media Directory", mds);
        }
        mediaDirectories.push_back(md);
    } else if (config->Read("MediaDir", &md)) {
        wxArrayString entries = wxSplit(md, '|', '\0');
        for (auto& d : entries) {
            std::string dstd = d.ToStdString();
            if (!ObtainAccessToURL(dstd)) {
                PromptForDirectorySelection("Reselect Media Directory", dstd);
            }
            if (std::find(mediaDirectories.begin(), mediaDirectories.end(), dstd) == mediaDirectories.end()) {
                mediaDirectories.push_back(dstd);
            }
        }
    }
    FileUtils::SetFixFileDirectories(mediaDirectories);
    wxString tbData = wxString(config->Read("ToolbarLocations"));
    if (tbData.StartsWith(TOOLBAR_SAVE_VERSION)) {
        const int size = AUIStatusBar->GetSize().GetHeight();
        MainAuiManager->LoadPerspective(tbData.Right(tbData.size() - 5));
        MainAuiManager->GetPane("Status Bar").MinSize(wxSize(-1, size));
        MainAuiManager->Update();
    }
    spdlog::debug("Perspectives loaded.");

    config->Read("xLightsBackupSubdirectories", &_backupSubfolders, true);
    spdlog::debug("Backup subdirectories: {}.", toStr(_backupSubfolders));

    // Previously `xLightsExcludePresetsPkgSeq` — that option stripped
    // the `<effects>` node from packaged rgbeffects, but presets now
    // live under a different element so the strip was a no-op. Replaced
    // with "Exclude Videos" which covers the legitimate concern
    // (copyright) that the old label was trying to address. Old config
    // value is silently discarded — semantics don't translate.
    config->Read("xLightsExcludeVideosPkgSeq", &_excludeVideosFromPackagedSequences, false);
    spdlog::debug("Exclude Videos From Packaged Sequences: {}.", toStr(_excludeVideosFromPackagedSequences));

    config->Read("xLightsPromptBatchRenderIssues", &_promptBatchRenderIssues, true);
    spdlog::debug("Prompt for issues during batch render: {}.", toStr(_promptBatchRenderIssues));

    // I was willing to default this off ... but after multiple attempts to sneak this in ... this will default off in windows and if it is changed
    // again it will be totally and permanently disabled in windows.
#ifdef __WXMSW__
    bool defVMR = true;
#else
    bool defVMR = false;
#endif
    config->Read("xLightsIgnoreVendorModelRecommendations2", &_ignoreVendorModelRecommendations, defVMR);
    spdlog::debug("Ignore vendor model recommendations: {}.", toStr(_ignoreVendorModelRecommendations));

    config->Read("xLightsControllerPingInterval", &_controllerPingInterval, 0);
    if (_controllerPingInterval > 0) {
        _pingTimer->Start(_controllerPingInterval * 1000);
        _statusRefreshTimer->Start(_controllerPingInterval/2 * 1000);

    }
    spdlog::debug("Controller ping interval in seconds: {}.", toStr(_controllerPingInterval));

    config->Read("xLightsPurgeDownloadCacheOnStart", &_purgeDownloadCacheOnStart, false);
    spdlog::debug("Purge download cache on start: {}.", toStr(_purgeDownloadCacheOnStart));
    config->Read("xLightsEnablePositionZones", &_enablePositionZones, true);
    config->Read("xLightsShowZoneIndicator", &_showZoneIndicator, false);

    config->Read("xLightsVideoExportCodec", &_videoExportCodec, "H.264");
    spdlog::debug("Video Export Codec: {}.", (const char*)_videoExportCodec.c_str());

    config->Read("xLightsVideoExportBitrate", &_videoExportBitrate, 0);
    spdlog::debug("Video Export Bitrate: {}.", _videoExportBitrate);

    config->Read("xLightsExcludeAudioPkgSeq", &_excludeAudioFromPackagedSequences, false);
    spdlog::debug("Exclude Audio From Packaged Sequences: {}.", toStr(_excludeAudioFromPackagedSequences));

    config->Read("xLightsShowACLights", &_showACLights, false);
    MenuItem_ACLIghts->Check(_showACLights);
    spdlog::debug("Show AC Lights toolbar: {}.", toStr(_showACLights));

    config->Read("xLightsShowACRamps", &_showACRamps, false);
    MenuItem_ShowACRamps->Check(_showACRamps);
    spdlog::debug("Show AC Ramps: {}.", toStr(_showACRamps));

    config->Read("xLightsEnableRenderCache", &_enableRenderCache, _("Locked Only"));
    spdlog::debug("Enable Render Cache: {}.", (const char*)_enableRenderCache.c_str());
    _renderCache.Enable(_enableRenderCache);

    config->Read("xLightsRenderCacheMaxSizeMB", &_renderCacheMaximumSizeMB, 20 * 1024);
    spdlog::debug("Render Cache Maximum Size: {}MB.", _renderCacheMaximumSizeMB);
    _renderCache.SetMaximumSizeMB(_renderCacheMaximumSizeMB);

    config->Read("xLightsAutoSavePerspectives", &_autoSavePerspecive, false);
    MenuItem_PerspectiveAutosave->Check(_autoSavePerspecive);
    spdlog::debug("Autosave perspectives: {}.", toStr(_autoSavePerspecive));

    config->Read("xLightsRenderOnSave", &mRenderOnSave, false);
    spdlog::debug("Render on save: {}.", toStr(mRenderOnSave));

    config->Read("xLightsSaveFseqOnSave", &mSaveFseqOnSave, true);
    spdlog::debug("Save Fseq on save: {}.", toStr(mSaveFseqOnSave));

    if (!mSaveFseqOnSave) {
        spdlog::debug("Render on save changed to false, because Save Fseq on save is false.");
        mRenderOnSave = false;
    }

    config->Read("xLightsModelHandleSize", &_modelHandleSize, 1);
    spdlog::debug("Model Handle Size: {}.", _modelHandleSize);

    config->Read("xLightsCrosshairSize", &_crosshairSize, 1);
    spdlog::debug("Group View Crosshair Size: {}.", _crosshairSize);

    config->Read("xLightsBackupOnSave", &mBackupOnSave, false);
    spdlog::debug("Backup on save: {}.", toStr(mBackupOnSave));

    config->Read("xLightsBackupOnLaunch", &mBackupOnLaunch, true);
    spdlog::debug("Backup on launch: {}.", toStr(mBackupOnLaunch));

    config->Read("xLightsSuppressFadeHints", &mSuppressFadeHints, false);
    spdlog::debug("Suppress Transition Hints: {}.", toStr(mSuppressFadeHints));

    config->Read("xLightsSuppressColorWarn", &mSuppressColorWarn, false);
    spdlog::debug("Suppress Color Warning: {}.", toStr(mSuppressColorWarn));

    config->Read("xLightsAltBackupDir", &mAltBackupDir);
    spdlog::debug("Alternate Backup Dir: '{}'.", (const char*)mAltBackupDir.c_str());

    if (_purgeDownloadCacheOnStart) {
        PurgeDownloadCache();
    }

    if (wxDir::Exists(mAltBackupDir)) {
        if (!ObtainAccessToURL(mAltBackupDir, true)) {
            std::string orig = mAltBackupDir;
            PromptForDirectorySelection("Reselect Alternate Backup Directory", orig);
            if (orig != mAltBackupDir) {
                mAltBackupDir = orig;
                config->Write("xLightsAltBackupDir", mAltBackupDir);
            }
        }
        mAltBackupMenuItem->SetHelp(mAltBackupDir);
    } else {
        mAltBackupMenuItem->SetHelp("");
    }

    config->Read("xLightsIconSize", &mIconSize, 16);
    SetToolIconSize(mIconSize);
    spdlog::debug("Icon size: {}.", mIconSize);

    config->Read("AutoSaveInterval", &mAutoSaveInterval, 3);
    SetAutoSaveInterval(mAutoSaveInterval);
    spdlog::debug("Autosave interval: {}.", mAutoSaveInterval);

    config->Read("xFadePort", &_xFadePort, 0);

    // overide ab setting from command line
    if (ab != 0) {
        _xFadePort = ab;
    }

    spdlog::debug("xFadePort: {}.", _xFadePort == 0 ? "Disabled" : ((_xFadePort == 1) ? "A" : "B"));
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
    spdlog::debug("Backup purge age: {} days.", BackupPurgeDays);

    config->Read("xLightsGridSpacing", &mGridSpacing, 16);
    SetGridSpacing(mGridSpacing);
    spdlog::debug("Grid spacing: {}.", mGridSpacing);

    config->Read("xLightsGridIconBackgrounds", &mGridIconBackgrounds, true);
    SetGridIconBackgrounds(mGridIconBackgrounds);
    spdlog::debug("Grid icon backgrounds: {}.", toStr(mGridIconBackgrounds));

    config->Read("xLightsShowAlternateTimingFormat", &mShowAlternateTimingFormat, false);
    SetShowAlternateTimingFormat(mShowAlternateTimingFormat);
    spdlog::debug("Show Alternate Timing Format: {}.", toStr(mShowAlternateTimingFormat));

    config->Read("xLightsGroupEffectIndicator", &mShowGroupEffectIndicator, true);
    SetShowGroupEffectIndicator(mShowGroupEffectIndicator);
    spdlog::debug("Group Effect Indicators: {}.", toStr(mShowGroupEffectIndicator));

    config->Read("xLightsTimingPlayOnDClick", &mTimingPlayOnDClick, true);
    SetTimingPlayOnDClick(mTimingPlayOnDClick);
    spdlog::debug("Timing Play on DClick: {}.", toStr(mTimingPlayOnDClick));

    config->Read("xLightsGridNodeValues", &mGridNodeValues, true);
    SetGridNodeValues(mGridNodeValues);
    spdlog::debug("Grid node values: {}.", toStr(mGridNodeValues));

    config->Read("xLightsEffectAssistMode", &mEffectAssistMode, EFFECT_ASSIST_TOGGLE_MODE);
    if (mEffectAssistMode < 0 || mEffectAssistMode > EFFECT_ASSIST_TOGGLE_MODE) {
        mEffectAssistMode = EFFECT_ASSIST_TOGGLE_MODE;
    }
    tempEffectAssistMode = mEffectAssistMode;
    spdlog::debug("Effect Assist Mode: {}.", mEffectAssistMode);
    if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    } else {
        SetEffectAssistWindowState(false);
    }

    InitEffectsPanel(EffectsPanel1);
    spdlog::debug("Effects panel initialised.");

    _serviceSettingsStore = std::make_unique<WxServiceSettingsStore>();
    {
        wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
        std::string pluginDir = (exePath.GetPath() + wxFILE_SEP_PATH + "ai_plugins").ToStdString();
        // ServiceManager registers all built-in services (chatGPT,
        // Claude, Ollama, Gemini, GenericClient, plus AppleIntelligence
        // on Apple Silicon) and loads any plugin DLLs in `pluginDir`.
        _serviceManager = std::make_unique<ServiceManager>(_serviceSettingsStore.get(), pluginDir);
    }

    starttime = wxDateTime::UNow();
    ResetEffectsXml();
    EnableSequenceControls(true);
    if (ok && !dir.IsEmpty()) {
        if (!SetDir(dir, !showDirFromCommandLine)) {
            CurrentDir = "";
            if (!PromptForShowDirectory(true, dir)) {
                CurrentDir = "";
                splash.Hide();
                wxMessageBox("Exiting as setting a show folder is not optional.");
                wxExit();
                return;
            }
        } else {
            if (ShowFolderIsInBackup(dir.ToStdString())) {
                DisplayWarning("WARNING: Opening a show folder inside a backup folder. This is generally a very very bad idea.", this);
            }
#ifdef __WXMSW__
            else if (ShowFolderIsInProgramFiles(dir.ToStdString())) {
                DisplayWarning("ERROR: Show folder inside your Program Files folder either just wont work or will cause you security issues.", this);
            }
#endif
        }
    } else {
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
    spdlog::debug("xLights Crash Menu item not removed.");
#ifdef _MSC_VER
    if (IsDarkMode()) {
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
    } else {
        spdlog::debug("xLights Crash Menu item not removed.");
    }
#endif

#ifdef __WXOSX__
    MenuItem_Update->GetMenu()->Remove(MenuItem_Update);
    MenuItem_Update = nullptr;
#endif

    _valueCurvesPanel->UpdateValueCurveButtons(false);
    _coloursPanel->UpdateColourButtons(false, this);

    MixTypeChanged = true;

    // This is used by xSchedule
    Notebook1->SetLabel("XLIGHTS_NOTEBOOK");

    Notebook1->ChangeSelection(SETUPTAB);
    EnableNetworkChanges();

    wxImage::AddHandler(new wxGIFHandler);
    wxImage::AddHandler(new wxWEBPHandler);

    // Helper lambda: convert wxImage to xlImage (handles both alpha and mask transparency)
    auto wxToXl = [](const wxImage& img) -> xlImage {
        return wxImageToXlImage(img);
    };

    // Register WebP animation loader (GIF loading is handled by the core stb_image loader)
    ImageCacheEntry::SetWebPLoader([wxToXl](const uint8_t* data, size_t len, const std::string&) -> AnimatedImageData {
        AnimatedImageData result;
        wxMemoryInputStream stream(data, len);
        std::vector<wxWebPAnimationFrame> frames;
        wxWEBPHandler handler;
        if (handler.LoadAnimation(frames, stream)) {
            for (const auto& frame : frames) {
                result.frames.push_back(wxToXl(frame.image));
                result.frameTimes.push_back(frame.duration);
            }
            if (!frames.empty()) {
                result.width = frames[0].image.GetWidth();
                result.height = frames[0].image.GetHeight();
                result.backgroundColor = xlColor(frames[0].bgColour.Red(), frames[0].bgColour.Green(), frames[0].bgColour.Blue());
            }
        }
        return result;
    });

    config->Read("xLightse131Sync", &me131Sync, false);
    _outputManager.SetSyncEnabled(me131Sync);
    spdlog::debug("Sync: {}.", toStr(me131Sync));

    // this is no longer used ... as it is now stored in the networks file
    wxString tmpString;
    config->Read("xLightsLocalIP", &tmpString, "");
    if (ip_utils::IsValidLocalIP(tmpString) && tmpString != "") {
        _outputManager.SetGlobalForceLocalIP(tmpString);
        config->DeleteEntry("xLightsLocalIP");
    }

    SetControllersProperties();
    UpdateACToolbar();
    ShowACLights();

    DoBackupPurge();

    // start out with 50ms timer, once we load a file or create a new one, we'll reset
    // to whatever the timing that is selected.   If the timer triggers and is then not
    // needed, it will be turned off later
    OutputTimer.Start(50, wxTIMER_CONTINUOUS);

    // What makes 4 the right answer ... try 10 ... why ... usually it is one thread that runs slow and that model
    // holds up others so in the time while we wait for the busy thread we can actually run a lot more models
    // what is the worst that could happen ... all models want to run hard so we lose some efficiency while we churn between
    // threads ... a minor loss of efficiency ... I think the one thread blocks the others is more common.
    // Dan is concerned on 32 bit windows 10 will chew up too much heap memory ... so splitting the difference we get 7
    int multiplier = (sizeof(size_t) == 8) ? 10 : 7;
    if (GetPhysicalMemorySizeMB() > 12 * 1024) {
        // if we have over 12GB of memory, creating more threads shouldn't be an issue
        multiplier *= 2;
    }
    int threadCount = wxThread::GetCPUCount() * multiplier;
    if (threadCount < 20) {
        threadCount = 20;
    }
    jobPool.Start(threadCount);

    if (!xLightsApp::sequenceFiles.IsEmpty()) {
        spdlog::debug("Opening sequence: {}.", (const char*)xLightsApp::sequenceFiles[0].c_str());
        OpenSequence(xLightsApp::sequenceFiles[0], nullptr);
    }

    SetAudioControls();

    if (mRenderOnSave) {
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVE, _("Render and Save"));
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVEAS, _("Render and Save As"));
        MainToolBar->Realize();
    }

#ifdef __WXOSX__
    revertToMenu = new wxMenu;
    revertToMenu->Append(wxID_ANY, "Last Saved");
    revertToMenuItem = MenuFile->Insert(5, ID_MENUITEM_REVERTTO, _("Revert To..."), revertToMenu, wxEmptyString);
    MenuFile->Bind(wxEVT_MENU_OPEN, [&](wxMenuEvent& event) {
        event.Skip();
        if (CurrentSeqXmlFile && CurrentSeqXmlFile->GetSequenceLoaded()) {
            revertToMenuItem->Enable(true);
            while (revertToMenu->GetMenuItemCount() > 1) {
                revertToMenu->Remove(revertToMenu->FindItemByPosition(1));
            }
            for (auto &r : GetFileRevisions(CurrentSeqXmlFile->GetFullPath())) {
                revertToMenu->Append(wxID_ANY, r);
            }
        } else {
            revertToMenuItem->Enable(false);
        }
    });
    revertToMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, [&](wxCommandEvent &event) {
        if (CurrentSeqXmlFile) {
            wxString rev = revertToMenu->GetLabelText(event.GetId());
            std::string origUrl = CurrentSeqXmlFile->GetFullPath();
            std::string newUrl = GetURLForRevision(origUrl, rev);
            if (CloseSequence()) {
                OpenSequence(origUrl, nullptr, newUrl);
            }
            if (newUrl != origUrl) {
                wxRemoveFile(newUrl);
            }
        }
    });


    config->Read("xLightsVideoReaderAccelerated", &_hwVideoAccleration, true);
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
    VideoReader::InitHWAcceleration();


    MenuFile->AppendSeparator();
    const long newInstId = wxNewId();
    wxMenuItem* newInst = new wxMenuItem(MenuFile, newInstId, _("Open New xLights Instance"), wxEmptyString, wxITEM_NORMAL);
    MenuFile->Append(newInst);

    Connect(newInstId, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuItem_File_NewXLightsInstance);

    bool gpuRendering = true;
    config->Read("xLightsGPURendering", &gpuRendering, true);
    GPURenderUtils::SetEnabled(gpuRendering);

    _taskBarIcon = std::make_unique<xlMacDockIcon>(this);
#else
    config->Read("xLightsVideoReaderAccelerated", &_hwVideoAccleration, false);
    config->Read("xLightsVideoReaderRenderer", &_hwVideoRenderer, 1);
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
    VideoReader::SetHardwareRenderType(_hwVideoRenderer);
#endif
#ifdef __WXMSW__
    // make sure Direct2DRenderer is created on the main thread before the other threads need it
    wxGraphicsRenderer::GetDirect2DRenderer();

    bool bgShaders = false;
    config->Read("xLightsShadersOnBackgroundThreads", &bgShaders, false);
    ShaderEffect::SetBackgroundRender(bgShaders);
#endif

#ifdef LINUX
    // FreeType + HarfBuzz + Fontconfig path. Thread-safe per-instance, which
    // lets TextEffect/ShapeEffect render on background threads (the wx/Pango
    // path requires the main thread).
    FreeTypeTextDrawingContext::Register();
#else
    TextDrawingContext::RegisterFactory(wxTextDrawingContext::Create,
                                        wxTextDrawingContext::ParseTextFont,
                                        wxTextDrawingContext::ParseShapeFont);
    TextDrawingContext::Initialize();
#endif

    MenuItem_File_Save->Enable(true);
    MenuItem_File_Save->SetItemLabel("Save Setup\tCTRL-s");

    splash.Hide();

    config->Read("xLightsUserEmail", &_userEmail, "");

    config->Read("xLightsLinkedControllerUpload", &_linkedControllerUpload, "Inputs and Outputs");
    spdlog::debug("Linked controller upload: {}.", (const char*)_linkedControllerUpload.c_str());

    config->Read("xLightsModelRename", &_aliasRenameBehavior, "Always Prompt");
    spdlog::debug("Model Rename Behavior: {}.", (const char*)_aliasRenameBehavior.c_str());

    std::thread th([this]() {
        try {
            xlCrashHandler::SetupCrashHandlerForNonWxThread();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            this->CallAfter(&xLightsFrame::DoPostStartupCommands);
        } catch (...) {
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

    if (renderOnlyMode) {
        DisablePromptBatchRenderIssues();
    }

    ValidateWindow();
    
    GetOutputModelManager()->DisableASAPWork(false);

    spdlog::debug("xLightsFrame construction complete.");
}

xLightsFrame::~xLightsFrame()
{
    
    static bool reenter = false;

    if (reenter) {
        spdlog::error("~xLightsFrame re-entered ... this wont end well ... so bailing now.");
        return;
    }
    reenter = true;
    // make sure we abort any render that is going on or we could crash as things get destroyed
    // however, only wait 2000ms to avoid complete hang on shutdown if a
    // render thread is completely stuck
    AbortRender(2000);

    delete _presetModel;
    _presetModel = nullptr;
    delete _presetModelManager;
    _presetModelManager = nullptr;

    AutoSaveTimer.Stop();
    EffectSettingsTimer.Stop();
    OutputTimer.Stop();
    RenderStatusTimer.Stop();
    TextDrawingContext::CleanUp();
    EffectIconCache::Clear();

    if (_pingTimer != nullptr) {
        _pingTimer->Stop();
        delete _pingTimer;
        _pingTimer = nullptr;
        _statusRefreshTimer->Stop();
        delete _statusRefreshTimer;
        _statusRefreshTimer = nullptr;
    }

    if (_automationServer != nullptr) {
        _automationServer->Stop();
        delete _automationServer;
        _automationServer = nullptr;
    }

    selectedEffect = nullptr;
    _outputManager.AllOff();
    _outputManager.StopOutput();
    SetConfigBool("OutputActive", false);

    auto* config = GetXLightsConfig();
    if (mResetToolbars) {
        config->DeleteEntry("ToolbarLocations");
    } else {
        config->Write("ToolbarLocations", TOOLBAR_SAVE_VERSION + MainAuiManager->SavePerspective());
    }
    config->Write("xLightsIconSize", mIconSize);
    config->Write("xLightsGridSpacing", mGridSpacing);
    config->Write("xLightsGridIconBackgrounds", mGridIconBackgrounds);
    config->Write("xLightsShowAlternateTimingFormat", mShowAlternateTimingFormat);
    config->Write("xLightsGroupEffectIndicator", mShowGroupEffectIndicator);
    config->Write("xLightsTimingPlayOnDClick", mTimingPlayOnDClick);
    config->Write("xLightsGridNodeValues", mGridNodeValues);
    config->Write("xLightsRenderOnSave", mRenderOnSave);
    config->Write("xLightsSaveFseqOnSave", mSaveFseqOnSave);
    config->Write("xLightsBackupSubdirectories", _backupSubfolders);
    config->Write("xLightsExcludeVideosPkgSeq", _excludeVideosFromPackagedSequences);
    config->Write("xLightsPromptBatchRenderIssues", _promptBatchRenderIssues);
    config->Write("xLightsIgnoreVendorModelRecommendations2", _ignoreVendorModelRecommendations);
    config->Write("xLightsControllerPingInterval", _controllerPingInterval);
    config->Write("xLightsPurgeDownloadCacheOnStart", _purgeDownloadCacheOnStart);
    config->Write("xLightsEnablePositionZones", _enablePositionZones);
    config->Write("xLightsShowZoneIndicator", _showZoneIndicator);
    config->Write("xLightsExcludeAudioPkgSeq", _excludeAudioFromPackagedSequences);
    config->Write("xLightsShowACLights", _showACLights);
    config->Write("xLightsShowACRamps", _showACRamps);
    config->Write("xLightsEnableRenderCache", _enableRenderCache);
    config->Write("xLightsRenderCacheMaxSizeMB", _renderCacheMaximumSizeMB);
    config->Write("xLightsPlayControlsOnPreview", _playControlsOnPreview);
    config->Write("xLightsShowBaseFolder", _showBaseShowFolder);
    config->Write("xLightsAutoShowHousePreview", _autoShowHousePreview);
    config->Write("xLightsZoomMethodToCursor", _zoomMethodToCursor);
    config->Write("xLightsHidePresetPreview", _hidePresetPreview);
    config->Write("xLightsSmallWaveform", _smallWaveform);
    config->Write("xLightsRenderBell", _renderBellEnabled);
    config->Write("xLightsModelBlendDefaultOff", _modelBlendDefaultOff);
    config->Write("xLightsLowDefinitionRender", _lowDefinitionRender);
    config->Write("xLightsTimelineZooming", _timelineZooming);
    config->Write("xLightsSnapToTimingMarks", _snapToTimingMarks);
    config->Write("xLightsFSEQVersion", _fseqVersion);
    config->Write("xLightsVersion", xlights_version_string);
    config->Write("xLightsDisableKeyAccelerations", _disableKeyAcceleration);
    config->Write("xLightsAutoSavePerspectives", _autoSavePerspecive);
    config->Write("xLightsBackupOnSave", mBackupOnSave);
    config->Write("xLightsBackupOnLaunch", mBackupOnLaunch);
    config->Write("xLightsSuppressFadeHints", mSuppressFadeHints);
    config->Write("xLightsSuppressColorWarn", mSuppressColorWarn);
    config->Write("xLightse131Sync", me131Sync);
    config->Write("xLightsEffectAssistMode", mEffectAssistMode);
    config->Write("xLightsAltBackupDir", mAltBackupDir);
    config->Write("xFadePort", _xFadePort);
    config->Write("xLightsModelHandleSize", _modelHandleSize);
    config->Write("xLightsCrosshairSize", _crosshairSize);
    config->Write("xLightsPlayVolume", playVolume);
    config->Write("xLightsVideoExportCodec", _videoExportCodec);
    config->Write("xLightsVideoExportBitrate", _videoExportBitrate);

    wxString colOrd;
    for (int c : List_Controllers->GetColumnsOrder()) {
        colOrd += wxString::Format("%d", c) + ",";
    }
    config->Write("ControllerTabColumnOrder", colOrd.RemoveLast());

    SaveDockable();

    if (layoutPanel != nullptr) {
        layoutPanel->SaveLayoutPerspective();
    }

    xlColourData::INSTANCE.Save(config);

    // definitely not outputting data anymore
    config->Write("OutputActive", false);

    config->Flush();

    // must call these or the app will crash on exit
    m_mgr->UnInit();
    MainAuiManager->UnInit();

    for (int x = 0; x < (int)Notebook1->GetPageCount(); x++) {
        wxWindow* w = Notebook1->GetPage(x);
        if (w->GetEventHandler() == m_mgr) {
            w->RemoveEventHandler(m_mgr);
        }
    }

    // unconnect these as the call to DeleteAllPages will cause pages to change and the page numbers to possibly not match
    Disconnect(ID_NOTEBOOK1, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanged1);
    Disconnect(ID_NOTEBOOK1, wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGING, (wxObjectEventFunction)&xLightsFrame::OnNotebook1PageChanging);

    // I still have examples where this crashes ... maybe we should just not delete them
    Notebook1->DeleteAllPages();

    delete m_mgr;
    delete MainAuiManager;

    waitForPingsToComplete();
    _outputManager.DeleteAllControllers();
    ip_utils::shutdownResolvePool();

    if (CurrentSeqXmlFile) {
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
    if (_tod != nullptr)
        delete _tod;
#endif

    //(*Destroy(xLightsFrame)
    //*)

    if (_logfile != nullptr) {
        wxLog::SetActiveTarget(nullptr);
        fclose(_logfile);
        _logfile = nullptr;
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Error);
    }

    reenter = false;
}

bool xLightsFrame::IsCheckSequenceOptionDisabledS(const std::string& option)
{
    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        return false;
    }
    bool value = false;
    config->Read("xLightsCS" + option, &value, false);
    return value;
}

void xLightsFrame::SetCheckSequenceOptionDisable(const std::string& option, bool value)
{
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Write("xLightsCS" + option, value);
    }
}

AudioManager* xLightsFrame::GetCurrentMediaManager() const
{
    if (CurrentSeqXmlFile == nullptr) {
        return nullptr;
    }
    return CurrentSeqXmlFile->GetMedia();
}

AudioManager* xLightsFrame::GetPlaybackAudio() const
{
    if (CurrentSeqXmlFile == nullptr) {
        return nullptr;
    }
    if (mainSequencer != nullptr) {
        int trackIdx = mainSequencer->GetActiveAudioTrackIndex();
        if (trackIdx > 0) {
            AudioManager* alt = CurrentSeqXmlFile->GetAltTrackMedia(trackIdx - 1);
            if (alt != nullptr) {
                return alt;
            }
        }
    }
    return CurrentSeqXmlFile->GetMedia();
}

const std::string& xLightsFrame::GetHeaderInfo(HEADER_INFO_TYPES type) const
{
    static const std::string empty;
    if (CurrentSeqXmlFile == nullptr) {
        return empty;
    }
    return CurrentSeqXmlFile->GetHeaderInfo(type);
}

// ---- UICallbacks implementation ----

void xLightsFrame::ShowMessage(const std::string& message,
                               const std::string& caption) const {
    wxMessageBox(message, caption);
}

bool xLightsFrame::PromptYesNo(const std::string& message,
                               const std::string& caption) const {
    return wxMessageBox(message, caption, wxYES_NO | wxICON_QUESTION) == wxYES;
}

bool xLightsFrame::PromptYesNoAll(const std::string& message,
                                   const std::string& caption,
                                   bool& acceptAll,
                                   bool& rejectAll) const {
    if (acceptAll) return true;
    if (rejectAll) return false;

    wxDialog dlg(const_cast<xLightsFrame*>(this), wxID_ANY, caption,
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxCAPTION | wxRESIZE_BORDER);

    auto* outer = new wxBoxSizer(wxVERTICAL);
    auto* messageText = new wxStaticText(&dlg, wxID_ANY, message, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    messageText->Wrap(500);
    outer->Add(messageText, 0, wxALL | wxEXPAND, 15);

    auto* btnSizer = new wxStdDialogButtonSizer();
    auto* btnYes    = new wxButton(&dlg, wxID_YES,  "Yes");
    auto* btnYesAll = new wxButton(&dlg, wxID_ANY,  "Yes to All");
    auto* btnNo     = new wxButton(&dlg, wxID_NO,   "No");
    auto* btnNoAll  = new wxButton(&dlg, wxID_ANY,  "No to All");
    btnSizer->Add(btnYes,    0, wxALL, 4);
    btnSizer->Add(btnYesAll, 0, wxALL, 4);
    btnSizer->Add(btnNo,     0, wxALL, 4);
    btnSizer->Add(btnNoAll,  0, wxALL, 4);
    btnSizer->Realize();
    outer->Add(btnSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    dlg.SetSizerAndFit(outer);
    dlg.CenterOnParent();

    int result = wxID_NO;
    btnYes->Bind(wxEVT_BUTTON,    [&](wxCommandEvent&) { result = wxID_YES; dlg.EndModal(wxID_OK); });
    btnYesAll->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { result = wxID_YES; acceptAll = true; dlg.EndModal(wxID_OK); });
    btnNo->Bind(wxEVT_BUTTON,     [&](wxCommandEvent&) { result = wxID_NO;  dlg.EndModal(wxID_OK); });
    btnNoAll->Bind(wxEVT_BUTTON,  [&](wxCommandEvent&) { result = wxID_NO;  rejectAll = true; dlg.EndModal(wxID_OK); });

    dlg.ShowModal();
    return result == wxID_YES;
}

std::string xLightsFrame::PromptForDirectory(const std::string& message,
                                             const std::string& defaultPath) const {
    wxDirDialog dlg(nullptr, message, defaultPath);
    if (dlg.ShowModal() == wxID_OK) {
        return ToStdString(dlg.GetPath());
    }
    return {};
}

std::string xLightsFrame::PromptForFile(const std::string& message,
                                        const std::string& wildcard,
                                        const std::string& defaultPath) const {
    wxString result = wxFileSelector(message, defaultPath, wxEmptyString,
                                     wxEmptyString, wildcard, wxFD_OPEN);
    return ToStdString(result);
}

long xLightsFrame::PromptForNumber(const std::string& message,
                                   const std::string& caption,
                                   long defaultValue,
                                   long min, long max) const {
    return wxGetNumberFromUser(message, "", caption, defaultValue, min, max);
}

std::string xLightsFrame::PromptForText(const std::string& message,
                                        const std::string& caption,
                                        const std::string& defaultValue) const {
    wxTextEntryDialog ted(const_cast<xLightsFrame*>(this), message, caption, defaultValue);
    if (ted.ShowModal() == wxID_OK) {
        return ted.GetValue().ToStdString();
    }
    return defaultValue;
}

std::vector<std::string> xLightsFrame::ChooseFromList(
    const std::string& prompt,
    const std::vector<std::string>& options) const {
    wxArrayString wxOptions;
    for (const auto& opt : options) {
        wxOptions.push_back(opt);
    }
    OptionChooser dlg(const_cast<xLightsFrame*>(this));
    dlg.SetInstructionText(prompt);
    dlg.SetOptions(wxOptions);
    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString selected;
        dlg.GetSelectedOptions(selected);
        std::vector<std::string> result;
        result.reserve(selected.size());
        for (const auto& s : selected) {
            result.push_back(s.ToStdString());
        }
        return result;
    }
    return {};
}

std::vector<std::string> xLightsFrame::ChooseFromList(
    const std::string& prompt,
    const std::vector<std::string>& options,
    const std::vector<std::string>& preSelected) const {
    wxArrayString wxOptions;
    for (const auto& opt : options) {
        wxOptions.push_back(opt);
    }
    wxArrayString wxPreSelected;
    for (const auto& ps : preSelected) {
        wxPreSelected.push_back(ps);
    }
    CheckboxSelectDialog dlg(const_cast<xLightsFrame*>(this), prompt, wxOptions, wxPreSelected);
    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString selected = dlg.GetSelectedItems();
        std::vector<std::string> result;
        result.reserve(selected.size());
        for (const auto& s : selected) {
            result.push_back(s.ToStdString());
        }
        return result;
    }
    return {};
}

UICallbacks::ProgressToken xLightsFrame::BeginProgress(const std::string& message,
                                                       int maximum) {
    auto* dlg = new wxProgressDialog(message, "", maximum, this,
                                     wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    ProgressToken token = _nextProgressToken++;
    _progressDialogs[token] = dlg;
    return token;
}

void xLightsFrame::UpdateProgress(ProgressToken token, int value,
                                  const std::string& newMessage) {
    auto it = _progressDialogs.find(token);
    if (it != _progressDialogs.end()) {
        it->second->Update(value, newMessage);
    }
}

void xLightsFrame::EndProgress(ProgressToken token) {
    auto it = _progressDialogs.find(token);
    if (it != _progressDialogs.end()) {
        delete it->second;
        _progressDialogs.erase(it);
    }
}

void xLightsFrame::OnHelp(wxHelpEvent& event)
{
    // deliberately do nothing
}

void xLightsFrame::DoPostStartupCommands()
{
    
    spdlog::debug("In Post Startup");

    // dont check for updates if batch rendering
    if (!_renderMode && !_checkSequenceMode) {
// Don't bother checking for updates when debugging.
#if !defined(_DEBUG) || defined(SIMULATE_UPGRADE)
#ifndef __WXOSX__
        CheckForUpdate(1, true, false);
#endif
#endif
        if (_userEmail == "")
            CollectUserEmail();
        if (_userEmail != "noone@nowhere.xlights.org")
            spdlog::debug("User email address: <email>{}</email>", (const char*)_userEmail.c_str());

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
#ifdef __WXOSX__
        if (hasFullDiskAccess()) {
            wxMessageBox("xLights has been granted \"Full Disk Access\" in System settings.  This is strongly discouraged.",
                         "Full Disk Access Detected",
                         wxICON_INFORMATION | wxCENTER | wxOK);
        }
#endif
    }
}

void xLightsFrame::DoMenuAction(wxMenuEvent& evt)
{
    static bool inMenuAction = false;
    if (inMenuAction) {
        return;
    }
    inMenuAction = true;
    wxWindow* w = FindFocus();
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

    
    spdlog::info("Quit");
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

void xLightsFrame::LogPerspective(const wxString& perspective) const
{
    
    wxArrayString entries = wxSplit(perspective, '|');
    for (const auto& it : entries) {
        TraceLog::AddTraceMessage(it.ToStdString());
        spdlog::debug("    {}", (const char*)it.c_str());
    }
}

void xLightsFrame::OnAbout(wxCommandEvent& event)
{
    wxString hdg = wxString::Format(_("About xLights %s"), GetDisplayVersionString());
    wxString ver = wxString::Format(_("Version: %s\n%s"), GetDisplayVersionString(), wxVERSION_STRING);
#if defined(_MSC_VER)
    ver += wxString::Format("\nVisual C++ %d" ,_MSC_VER);
#endif
#if defined(__clang_version__)
    ver += wxString::Format("\nClang %s", __clang_version__);
#else
#if defined(__GNUC__)//clang also defines this
    ver += wxString::Format("\nGCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#endif
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
    bool needTimer = false;
    AddTraceMessage("OutputTimer");
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
            if (_outputManager.IsOutputting()) {
                needTimer = true;
                _outputManager.StartFrame(curtime);
                _outputManager.EndFrame();
            }
            break;
        }
    }
    if (!needTimer) {
        // printf("Stopping timer\n");
        StopOutputTimer();
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

    // this logging is extra until we find out why this function crashes
    spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows");

    if (m_mgr == nullptr) {
        spdlog::critical("ShowHideAllSequencerWindows m_mgr is null ... this is going to crash");
    }
    wxAuiPaneInfoArray& info = m_mgr->GetAllPanes();
    bool update = false;
    if (show && savedPaneShown.size() > 0) {
        spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows - show {} {}", (int)info.size(), (int)savedPaneShown.size());
        for (size_t x = 0; x < info.size(); x++) {
            spdlog::debug("     {}", (const char*)info[x].name.c_str());
            if (info[x].IsOk() &&
                savedPaneShown.find(info[x].name) != savedPaneShown.end() &&
                savedPaneShown[info[x].name]) {
                if (info[x].frame != nullptr)
                    info[x].frame->Show();
            }
        }
        savedPaneShown.clear();
    } else {
        savedPaneShown.clear();
        spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows - hide {}", (int)info.size());
        for (size_t x = 0; x < info.size(); x++) {
            spdlog::debug("     {}", (const char*)info[x].name.c_str());
            savedPaneShown[info[x].name] = false;
            if (info[x].IsOk()) {
                if (info[x].frame != nullptr) {
                    if (info[x].IsFloating() && info[x].IsShown()) {
                        savedPaneShown[info[x].name] = true;
                    }
                    info[x].frame->Hide();
                }
            } else {
                spdlog::warn("Pane {} was not valid ... ShowHideAllSequencerWindows", x);
            }
        }
    }

    if (update) {
        spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows - update");
        m_mgr->Update();
    }

    // show/hide Layout Previews
    spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows - layout previews");
    for (const auto& [name, grp] : LayoutGroups) {
        if (grp->GetMenuItem() == nullptr) {
            spdlog::critical("ShowHideAllSequencerWindows grp->GetMenuItem() is null ... this is going to crash");
        }
        if (grp->GetMenuItem() && grp->GetMenuItem()->IsChecked()) {
            grp->SetPreviewActive(show);
        }
    }

    // Handle the effect Assist
    if (tempEffectAssistMode == EFFECT_ASSIST_TOGGLE_MODE) {
        if (sEffectAssist->GetPanel() != sEffectAssist->GetDefaultAssistPanel() && sEffectAssist->GetPanel() != nullptr) {
            SetEffectAssistWindowState(true);
        } else {
            SetEffectAssistWindowState(false);
        }
    } else if (tempEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    } else {
        SetEffectAssistWindowState(false);
    }
    UpdateViewMenu();

    spdlog::debug("xLightsFrame::ShowHideAllSequencerWindows - layout previews - done");
}

void xLightsFrame::RecalcModels()
{
    auto logger_work = spdlog::get("work");
    logger_work->debug("        RecalcModels.");

    if (IsExiting())
        return;

    SetCursor(wxCURSOR_WAIT);

    // abort any render as it will crash if the model changes
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
        layoutPanel->HideFloatingPanes();
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
        layoutPanel->RestoreFloatingPanes();
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

void xLightsFrame::CycleOutputsIfOn()
{
    if (_outputManager.IsOutputting()) {
        _outputManager.StopOutput();
        SetConfigBool("OutputActive", false);
        EnableSleepModes();
        ForceEnableOutputs();
    }
}

bool xLightsFrame::ForceEnableOutputs(bool startTimer)
{
    bool outputting = false;
    if (!_outputManager.IsOutputting()) {
        DisableSleepModes();
        outputting = _outputManager.StartOutput();
        if (outputting) SetConfigBool("OutputActive", true);
        if (startTimer) {
            StartOutputTimer();
        }
        if (outputting) {
            for (auto& controller : _outputManager.GetControllers()) {
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
                    // upload config
                }
            }
        }
    }
    return outputting;
}

bool xLightsFrame::EnableOutputs(bool ignoreCheck)
{
    if (!ignoreCheck && GetConfigBool("OutputActive", false)) {
        DisplayWarning("Another process seems to be outputting to lights right now. This may not generate the result expected.", this);
    }
    bool ok = ForceEnableOutputs();
    CheckBoxLightOutput->SetBitmap(GetToolbarBitmapBundle("xlART_OUTPUT_LIGHTS_ON"));
    CheckBoxLightOutput->SetValue(true);
    EnableNetworkChanges();
    return ok;
}
bool xLightsFrame::DisableOutputs()
{
    if (_outputManager.IsOutputting()) {
        _outputManager.AllOff();
        _outputManager.StopOutput();
        SetConfigBool("OutputActive", false);
        EnableSleepModes();

        // for (auto &controller : _outputManager.GetControllers()) {
        //     if (controller->IsActive() && controller->IsAutoUpload() && controller->SupportsAutoUpload()) {
        //         auto ip = controller->GetResolvedIP();
        //         if (ip == "" || ip == "MULTICAST" || controller->GetProtocol() == OUTPUT_ZCPP) {
        //             continue;
        //         }
        //         BaseController* bc = BaseController::CreateBaseController(controller);
        //         if (bc != nullptr && bc->IsConnected()) {
        //             bc->ResetAfterOutput(&_outputManager, controller, this);
        //         }
        //         if (bc) {
        //             delete bc;
        //         }
        //     }
        // }
    }
    CheckBoxLightOutput->SetBitmap(GetToolbarBitmapBundle("xlART_OUTPUT_LIGHTS"));
    CheckBoxLightOutput->SetValue(false);
    EnableNetworkChanges();
    return true;
}
bool xLightsFrame::ToggleOutputs(bool ignoreCheck)
{
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

// factored out from below so it can be reused by play/pause button -DJ
void xLightsFrame::StopNow()
{
    int actTab = Notebook1->GetSelection();
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr) {
        CurrentSeqXmlFile->GetMedia()->Stop();
    }
    starttime = wxDateTime::UNow();
    switch (actTab) {
    case NEWSEQUENCER: {
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    } break;
    default:
        break;
    }
}

bool xLightsFrame::ShowFolderIsInBackup(const std::string showdir)
{
    int i = showdir.length() - 1;
    wxString dir = "";
    while (i >= 0) {
        if (showdir[i] == '\\' || showdir == "/") {
            if (dir.Lower() == "backup") {
                return true;
            }
            dir = "";
        } else if (showdir[i] == ':') {
            return false;
        } else {
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
    if (first == "Program Files" || first == "Program Files (x86)") {
        return true;
    }
#endif

    return false;
}

// make these static so they can be accessed outside of xLightsFrame: -DJ
// NOTE: this assumes there will only be one xLightsMain object
wxString xLightsFrame::CurrentDir = "";
wxString xLightsFrame::FseqDir = "";
wxString xLightsFrame::PlaybackMarker = "";
wxString xLightsFrame::xlightsFilename = "";
SequenceFile* xLightsFrame::CurrentSeqXmlFile = nullptr;

void xLightsFrame::OnClose(wxCloseEvent& event)
{
    if (!QuitMenuItem->IsEnabled()) {
        return;
    }

    static bool inClose = false;

    if (inClose)
        return;

    inClose = true;

    spdlog::info("xLights Closing");

    StopNow();

    if (!CloseSequence()) {
        spdlog::info("Closing aborted.");
        event.Veto();
        inClose = false;
        return;
    }

    selectedEffect = nullptr;

    CheckUnsavedChanges();

    _exiting = true;

    ShowHideAllSequencerWindows(false);

    spdlog::debug("Destroying {} preview windows.", (int)PreviewWindows.size());
    // destroy preview windows
    for (const auto& it : PreviewWindows) {
        ModelPreview* preview = it;
        delete preview;
    }

    spdlog::debug("Heartbeat exit.");

    if (xLightsApp::cleanupDir != "") {
        spdlog::info("Cleaning up temp folder {}", (const char*)xLightsApp::cleanupDir.c_str());
        wxDir::Remove(xLightsApp::cleanupDir, wxPATH_RMDIR_RECURSIVE);
    }

    Destroy();
    spdlog::info("xLights Closed.");

    inClose = false;
}

void xLightsFrame::DoBackup(bool prompt, bool startup, bool forceallfiles)
{
    
    time_t cur;
    time(&cur);
    wxFileName newDirH;
    wxDateTime curTime(cur);

    //  first make sure there is a Backup sub directory
    if (_backupDirectory == "") {
        wxMessageBox("Backup directory has not been set. Aborting backup.");
        return;
    }

    wxString newDirBackup = _backupDirectory + GetPathSeparator() + "Backup";

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
        spdlog::warn("Backup directory '{}' already existed ... trying again", (const char*)newDir.c_str());

        newDir = wxString::Format("%s%c%s-%s",
                                  newDirBackup, wxFileName::GetPathSeparator(),
                                  curTime.FormatISODate(), curTime.Format("%H%M%S")) +
                 "_" + char(65 + tries);
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
                                     newDir + "\". Proceed?",
                                 "Backup", wxICON_QUESTION | wxYES_NO)) {
            return;
        }
    }

    if (!newDirH.Mkdir(newDir)) {
        DisplayError(wxString::Format("Unable to create directory '%s'! Backup failed.", newDir).ToStdString());
        return;
    } else {
        spdlog::info("Backup directory '{}' created", (const char*)newDir.c_str());
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

    if (wxDir::Exists(targetDirName))
        return;
    if (!wxDir::Exists(lastCreatedDirectory))
        return;

    if (targetDirName.Length() > 256) {
        spdlog::warn("Target directory {} is {} characters long. This may be an issue on your operating system.", (const char*)targetDirName.c_str(), targetDirName.Length());
    }

    wxFileName tgt(targetDirName);
    wxFileName lst(lastCreatedDirectory);

    if (!tgt.GetFullPath().StartsWith(lst.GetFullPath()))
        return;

    spdlog::debug("Create missing directories. Target {}. Last Created {}.", (const char*)tgt.GetFullPath().c_str(), (const char*)lst.GetFullPath().c_str());

    wxArrayString tgtd = wxSplit(targetDirName, wxFileName::GetPathSeparator());
    wxArrayString lstd = wxSplit(lastCreatedDirectory, wxFileName::GetPathSeparator());
    wxString newDir = lastCreatedDirectory;

    bool cont = true;
    for (size_t i = lstd.Count(); cont && i < tgtd.Count(); i++) {
        wxDir dir(newDir);
        newDir += wxFileName::GetPathSeparator() + tgtd[i];
        if (!wxDir::Exists(newDir)) {
            spdlog::debug("    Create folder '{}'.", (const char*)newDir.c_str());
            if (!dir.Make(newDir)) {
                cont = false;
                errors += wxString::Format("Failed to create folder %s\n", newDir);
                spdlog::error("        Folder Create failed.");
            }
        }
    }
}

bool xLightsFrame::CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, bool forceallfiles, std::string& errors)
{
    
    bool res = false;
    wxFileName srcFile;
    srcFile.SetPath(srcDir.GetNameWithSep());

    wxArrayString files;
    GetAllFilesInDir(srcDir.GetNameWithSep(), files, wildcard);
    for (auto& file : files) {
        auto const fname = wxFileName(file).GetFullName();
        srcFile.SetFullName(file);
        if (FileExists(srcFile.GetFullPath())) { // checking if exists will force it to be downloaded if in the cloud
            spdlog::debug("Backing up file {}.", (const char*)(srcFile.GetFullPath()).c_str());
            res = true;

            CreateMissingDirectories(targetDirName, lastCreatedDirectory, errors);

            wxULongLong fsize = srcFile.GetSize();
            if (!forceallfiles && fsize > MAXBACKUPFILE_MB * 1024 * 1024) // skip any xml files > MAXBACKUPFILE_MB mbytes, they are something other than xml files
            {
                spdlog::warn("    Skipping file as it is too large.");
                continue;
            }

            wxString dstFullPath = targetDirName + wxFileName::GetPathSeparator() + fname;
            spdlog::debug("    to {}.", (const char*)dstFullPath.c_str());
            SetStatusText("Copying File \"" + srcFile.GetFullPath());
            std::error_code ec;
            std::filesystem::copy_file(std::filesystem::path(srcFile.GetFullPath().ToStdString()),
                                       std::filesystem::path(dstFullPath.ToStdString()),
                                       std::filesystem::copy_options::overwrite_existing,
                                       ec);
            if (ec) {
                spdlog::error("    Copy Failed: '{}' -> '{}': {} (errno {})",
                              srcFile.GetFullPath().ToStdString(),
                              dstFullPath.ToStdString(),
                              ec.message(),
                              ec.value());
                errors += "Unable to copy file \"" + srcDir.GetNameWithSep() + fname + "\": " + ec.message() + "\n";
                if (srcDir.GetNameWithSep().length() + fname.length() > 225) {
                    errors += "Consider shortening the directory path or filename.\n";
                }
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
            CopyFiles("*.json", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
            CopyFiles("*.jbkp", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
            CopyFiles("*.xmap", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) +
            CopyFiles("*.xschedule", srcDir, targetDirName, lastCreatedDirectory, forceallfiles, errors) >
        0) {
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

bool xLightsFrame::isRandom_(wxControl* ctl, const char* debug)
{
    bool retval = (buttonState[std::string(ctl->GetName())] != Locked);
    return retval;
}

void xLightsFrame::OnEffectsPanel1Paint(wxPaintEvent& event)
{}

void xLightsFrame::OnGrid1SetFocus(wxFocusEvent& event)
{
    Grid1HasFocus = true;
}

void xLightsFrame::OnGrid1KillFocus(wxFocusEvent& event)
{
    Grid1HasFocus = false;
}

void xLightsFrame::OntxtCtrlSparkleFreqText(wxCommandEvent& event)
{}

ModelGroup* xLightsFrame::GetSelectedModelGroup() const
{
    if (Notebook1->GetSelection() != LAYOUTTAB)
        return nullptr;
    return layoutPanel->GetSelectedModelGroup();
}

// LoadJukebox(wxXmlNode*) removed — SequenceElements now calls JukeboxPanel::Load(pugi::xml_node) directly

// sigh; a function like this should have been built into wxWidgets
pugi::xml_node xLightsFrame::FindNode(pugi::xml_node parent, const std::string& tag, const std::string& attr, const std::string& value, bool create /*= false*/)
{
    return FindXmlNode(parent, tag, attr, value, create);
}

void xLightsFrame::SetPreviewSize(int width, int height)
{
    SetXmlSetting("previewWidth", std::to_string(width));
    SetXmlSetting("previewHeight", std::to_string(height));
    modelPreview->SetCanvasSize(width, height);
    modelPreview->Refresh();
    _housePreviewPanel->GetModelPreview()->SetVirtualCanvasSize(width, height);
    _housePreviewPanel->Refresh();
}

void xLightsFrame::SetXmlSetting(const std::string& settingName, const std::string& value)
{
    auto& entry = _xmlSettings[settingName];
    if (entry != value) {
        entry = value;
        UnsavedRgbEffectsChanges = true;
    }
}

std::string xLightsFrame::GetXmlSetting(const std::string& settingName, const std::string& defaultValue) const
{
    auto it = _xmlSettings.find(settingName);
    if (it != _xmlSettings.end()) {
        return it->second;
    }
    return defaultValue;
}

void xLightsFrame::OnButtonClickSaveAs(wxCommandEvent& event)
{
    SaveAsSequence();
}

wxString xLightsFrame::GetSeqXmlFileName()
{
    if (CurrentSeqXmlFile == nullptr)
        return "";

    return CurrentSeqXmlFile->GetFullPath();
}

void xLightsFrame::ShowSequenceSettings()
{
    if (xLightsFrame::CurrentSeqXmlFile == nullptr)
        return;

    // abort any in progress render ... it may be using media or we may change the sequence length ... and that would be bad
    int numThreadsAborted = 0;
    AbortRender(60000, &numThreadsAborted);

    // populate dialog
    SeqSettingsDialog dialog(this, xLightsFrame::CurrentSeqXmlFile, &_sequenceElements, mediaDirectories, wxEmptyString, wxEmptyString);
    // Constrain before Fit() so Fit() → SetSize() → DoSetSize() clips to the max.
    // Setting max size here is the reliable cross-platform approach: DoSetSize()
    // checks GetMaxSize() and won't exceed it, even on Wayland where post-Fit
    // SetSize() calls may be ignored until the window is first mapped.
    {
        wxRect usable = wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea();
        // Reserve ~40px each side for window decorations / compositor shadows.
        dialog.SetMaxSize(wxSize(usable.GetWidth() - 40, usable.GetHeight() - 40));
    }
    dialog.Fit();
    dialog.Centre();
    int ret_code = dialog.ShowModal();

    if (ret_code == NEEDS_RENDER || numThreadsAborted > 0) {
        RenderAll();
    }

    if (ret_code != wxID_OK)
        return; // user pressed cancel

    if (CurrentSeqXmlFile->GetSequenceType() == "Animation" || CurrentSeqXmlFile->GetSequenceType() == "Effect") {
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
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
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
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemStopClick(wxCommandEvent& event)
{
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        // playStartTime = playEndTime = 0;
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarFirstFrameClick(wxCommandEvent& event)
{
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_FIRST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarLastFrameClick(wxCommandEvent& event)
{
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_LAST_FRAME);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemReplaySectionClick(wxCommandEvent& event)
{
    // if (Notebook1->GetSelection() == NEWSEQUENCER)
    {
        wxCommandEvent playEvent(EVT_SEQUENCE_REPLAY_SECTION);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::OnAuiToolBarItemZoominClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER) {
        mainSequencer->PanelTimeLine->ZoomIn();
    }
}

void xLightsFrame::OnAuiToolBarItem_ZoomOutClick(wxCommandEvent& event)
{
    if (Notebook1->GetSelection() == NEWSEQUENCER) {
        mainSequencer->PanelTimeLine->ZoomOut();
    }
}

void xLightsFrame::OnMenuItem_File_Open_SequenceSelected(wxCommandEvent& event)
{
    if (readOnlyMode) {
        DisplayError("Sequences cannot be opened in read only mode!", this);
        return;
    }

    OpenSequence("", nullptr);
}

void xLightsFrame::OnMenuItem_File_SaveAs_SequenceSelected(wxCommandEvent& event)
{
    SaveAsSequence();
}

void xLightsFrame::AskCloseSequence()
{
    
    spdlog::info("Closing sequence.");
    CloseSequence();
    spdlog::info("Sequence closed.");

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
    std::string path = CurrentSeqXmlFile->GetName() + ".mp4";
    wxFileDialog pExportDlg(this, _("Export House Preview Video"), wxEmptyString, path, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    int exportChoice = pExportDlg.ShowModal();

    if (exportChoice != wxID_OK) {
        return;
    }

    ExportVideoPreview(pExportDlg.GetPath());
}

bool xLightsFrame::ExportVideoPreview(wxString const& path)
{
    int frameCount = _seqData.NumFrames();

    if (CurrentSeqXmlFile == nullptr || frameCount == 0) {
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

    ModelPreview* housePreview = _housePreviewPanel->GetModelPreview();
    if (housePreview == nullptr) {
        return false;
    }

    int playStatus = GetPlayStatus();
    SetPlayStatus(PLAY_TYPE_STOPPED);

    wxStopWatch sw;

    
    spdlog::debug("Writing house-preview video to {}.", (const char*)path.c_str());

    int width = housePreview->getWidth();
    int height = housePreview->getHeight();
    double contentScaleFactor = housePreview->GetContentScaleFactor();
#ifdef _WIN32
    contentScaleFactor = 1.;
#endif // WIN32

    int audioChannelCount = 0;
    int audioSampleRate = 0;
    AudioManager* audioMgr = CurrentSeqXmlFile->GetMedia();
    if (audioMgr != nullptr) {
        audioSampleRate = audioMgr->GetRate();
        audioChannelCount = audioMgr->GetChannels();
    }
    int audioFrameIndex = 0;
    bool exportStatus = false;
    std::string emsg;
    try {
        VideoExporter videoExporter(this, width, height, contentScaleFactor, _seqData.FrameTime(), _seqData.NumFrames(),
                                    audioChannelCount, audioSampleRate, path, _videoExportCodec, _videoExportBitrate);

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
        auto videoLambda = [=, this](AVFrame* f, uint8_t* buf, int bufSize, unsigned frameIndex) {
            const SequenceData::FrameData& frameData(this->_seqData[frameIndex]);
            const uint8_t* data = frameData[0];
            housePreview->captureNextFrame(width * contentScaleFactor, height * contentScaleFactor);
            housePreview->Render(frameIndex * this->_seqData.FrameTime(), data, false);
            return housePreview->getFrameForExport(width * contentScaleFactor, height * contentScaleFactor, f, buf, bufSize);
        };
        videoExporter.setGetVideoCallback(videoLambda);

        exportStatus = videoExporter.Export(_appProgress.get());
    } catch (const std::runtime_error& re) {
        emsg = (const char*)re.what();
        spdlog::error("Error exporting video : {}", (const char*)re.what());
        exportStatus = false;
    }

    SetPlayStatus(playStatus);

    if (!visible) {
        m_mgr->GetPane("HousePreview").Hide();
        m_mgr->Update();
    }

    if (exportStatus) {
        spdlog::debug("Finished writing house-preview video.");
        float elapsedTime = sw.Time() / 1000.0; // msec => sec
        SetStatusText(wxString::Format("'%s' exported in %4.3f sec.", path.c_str(), elapsedTime));
    } else {
        DisplayError("Exporting house preview video failed.  " + emsg, this);
    }
    return exportStatus;
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

void AUIToolbarButtonWrapper::SetBitmap(const wxBitmapBundle& bmp)
{
    toolbar->SetToolBitmap(id, bmp);
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
    size = FromDIP(size);
    for (size_t x = 0; x < EffectsToolBar->GetToolCount(); x++) {
        EffectsToolBar->FindToolByIndex(x)->GetWindow()->SetSizeHints(size, size, size, size);
        EffectsToolBar->FindToolByIndex(x)->SetMinSize(EffectsToolBar->FindToolByIndex(x)->GetWindow()->GetMinSize());
    }
    EffectsToolBar->Realize();
    wxSize sz = EffectsToolBar->GetSize();
    wxAuiPaneInfo& info = MainAuiManager->GetPane("EffectsToolBar");
    info.BestSize(sz);
    MainAuiManager->Update();

    const wxWindowList& lst = effectPalettePanel->GetChildren();
    for (size_t x = 0; x < lst.size(); x++) {
        lst[x]->SetSizeHints(size, size, size, size);
    }
    effectPalettePanel->Layout();
    effectsPnl->BitmapButtonSelectedEffect->SetSizeHints(size, size, size, size);
    effectsPnl->Layout();

    if (layoutPanel != nullptr) {
        layoutPanel->UpdateModelButtonSizes();
    }
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
    Effect::EnableBackgroundDisplayLists(b);
    mainSequencer->PanelEffectGrid->SetEffectIconBackground(mGridIconBackgrounds);
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::SetShowAlternateTimingFormat(bool b)
{
    mShowAlternateTimingFormat = b;
    mainSequencer->PanelTimeLine->SetShowAlternateTimingFormat(mShowAlternateTimingFormat);
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->SetShowAlternateTimingMark(mShowAlternateTimingFormat);
    mainSequencer->UpdateTimeDisplay(mainSequencer->PanelTimeLine->GetCurrentPlayMarkerMS(), {});
    mainSequencer->Refresh();
}

void xLightsFrame::SetShowGroupEffectIndicator(bool b)
{
    mShowGroupEffectIndicator = b;
    mainSequencer->PanelRowHeadings->SetShowGroupEffectIndicator(mShowGroupEffectIndicator);
    mainSequencer->PanelRowHeadings->Refresh();
}

void xLightsFrame::SetGridNodeValues(bool b)
{
    mGridNodeValues = b;
    mainSequencer->PanelEffectGrid->SetEffectNodeValues(mGridNodeValues);
    mainSequencer->PanelEffectGrid->Refresh();
}

float xLightsFrame::GetPlaySpeed() {
    return playSpeed;
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
    } else if (speed == 1.5) {
        AudioMenu->Check(ID_MNU_1POINT5SPEED, true);
    } else if (speed == 2) {
        AudioMenu->Check(ID_MN_2SPEED, true);
    } else if (speed == 3) {
        AudioMenu->Check(ID_MNU_3SPEED, true);
    } else if (speed == 4) {
        AudioMenu->Check(ID_MNU_4SPEED, true);
    } else if (speed == 0.75) {
        AudioMenu->Check(ID_PLAY_3_4, true);
    } else if (speed == 0.5) {
        AudioMenu->Check(ID_PLAY_1_2, true);
    } else if (speed == 0.25) {
        AudioMenu->Check(ID_PLAY_1_4, true);
    }
}

void xLightsFrame::SetPlaySpeed(wxCommandEvent& event)
{
    if (event.GetId() == ID_PLAY_FULL) {
        SetPlaySpeedTo(1.0);
    } else if (event.GetId() == ID_PLAY_3_4) {
        SetPlaySpeedTo(0.75);
    } else if (event.GetId() == ID_PLAY_1_2) {
        SetPlaySpeedTo(0.5);
    } else if (event.GetId() == ID_PLAY_1_4) {
        SetPlaySpeedTo(0.25);
    } else if (event.GetId() == ID_MNU_1POINT5SPEED) {
        SetPlaySpeedTo(1.5);
    } else if (event.GetId() == ID_MN_2SPEED) {
        SetPlaySpeedTo(2.0);
    } else if (event.GetId() == ID_MNU_3SPEED) {
        SetPlaySpeedTo(3.0);
    } else if (event.GetId() == ID_MNU_4SPEED) {
        SetPlaySpeedTo(4.0);
    } else {
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
    tempEffectAssistMode = i;
    if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_ON) {
        SetEffectAssistWindowState(true);
    } else if (mEffectAssistMode == EFFECT_ASSIST_ALWAYS_OFF) {
        SetEffectAssistWindowState(false);
    }
}

void xLightsFrame::SetEffectAssistWindowState(bool show)
{
    bool visible = m_mgr->GetPane("EffectAssist").IsShown();
    if (visible && !show) {
        m_mgr->GetPane("EffectAssist").Hide();
        m_mgr->Update();
    } else if (!visible && show) {
        m_mgr->GetPane("EffectAssist").Show();
        m_mgr->Update();
    }
    UpdateViewMenu();
}

void xLightsFrame::UpdateEffectAssistWindow(Effect* effect, RenderableEffect* ren_effect)
{
    if (effect == nullptr || ren_effect == nullptr) {
        sEffectAssist->SetPanel(nullptr);
        return;
    }

    bool effect_is_supported = effectPanelManager.HasAssistPanel(ren_effect->GetId());

    if (tempEffectAssistMode == EFFECT_ASSIST_TOGGLE_MODE) {
        if (effect_is_supported) {
            SetEffectAssistWindowState(true);
        } else {
            SetEffectAssistWindowState(false);
        }
    }

    AssistPanel* panel;
    if (effect_is_supported) {
        panel = effectPanelManager.GetAssistPanel(ren_effect->GetId(), sEffectAssist, this);
    } else {
        panel = sEffectAssist->GetDefaultAssistPanel();
    }
    panel->SetEffectInfo(effect, this);
    sEffectAssist->SetPanel(panel);
}

void xLightsFrame::CheckUnsavedChanges()
{
    if (readOnlyMode) {
        return;
    }
    if (UnsavedRgbEffectsChanges) {
        // This is not necessary but it shows the user that the save button is red which I am hoping makes it clearer
        // to the user what this prompt is for
        Notebook1->SetSelection(LAYOUTTAB);

        if (wxYES == wxMessageBox("Save Models, Views, Perspectives, and Preset changes?",
                                  "RGB Effects File Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT)) {
            SaveEffectsFile();
        } else {
            wxFileName effectsFile;
            effectsFile.AssignDir(CurrentDir);
            effectsFile.SetFullName(_(XLIGHTS_RGBEFFECTS_FILE));
            wxFileName fn(effectsFile.GetFullPath());
            if (FileExists(fn.GetFullPath())) {
                fn.Touch();
            }
        }
    }

    if (UnsavedNetworkChanges) {
        // This is not necessary but it shows the user that the save button is red which I am hoping makes it clearer
        // to the user what this prompt is for
        Notebook1->SetSelection(SETUPTAB);

        if (wxYES == wxMessageBox("Save Network Setup changes?",
                                  "Networks Changes Confirmation", wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT)) {
            SaveNetworksFile();
        }
    }
}

void xLightsFrame::MarkEffectsFileDirty()
{
    auto logger_work = spdlog::get("work");
    logger_work->debug("        MarkEffectsFileDirty.");

    layoutPanel->SetDirtyHiLight(true);
    UnsavedRgbEffectsChanges = true;
}

void xLightsFrame::MarkModelsAsNeedingRender()
{
    auto logger_work = spdlog::get("work");
    logger_work->debug("        MarkModelsAsNeedingRender {}.", modelsChangeCount);
    modelsChangeCount++;
}

uint32_t xLightsFrame::GetMaxNumChannels()
{
    return std::max((uint32_t)_outputManager.GetTotalChannels(), (uint32_t)(AllModels.GetLastChannel() + 1));
}

void xLightsFrame::UpdateSequenceLength()
{
    if (CurrentSeqXmlFile->GetSequenceLoaded()) {
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

    // save the media playing state and stop it if it is playing
    MEDIAPLAYINGSTATE mps = MEDIAPLAYINGSTATE::STOPPED;
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr) {
        mps = CurrentSeqXmlFile->GetMedia()->GetPlayingState();
        if (mps == MEDIAPLAYINGSTATE::PLAYING) {
            spdlog::debug("Test: Suspending play.");
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
        spdlog::debug("Test: Turning off output to lights.");
        DisableOutputs();
    }

    // creating the dialog can take some time so display an hourglass
    SetCursor(wxCURSOR_WAIT);

    // Make sure all the models in model groups are valid
    AllModels.ResetModelGroups();

    spdlog::debug("Test: Opening test dialog.");

    // display the test dialog
    PixelTestDialog dialog(this, &_outputManager, networkFile, &AllModels);
    dialog.ShowModal();

    spdlog::debug("Test: Test dialog closed.");

    SetCursor(wxCURSOR_DEFAULT);

    // resume output if it was set
    if (output) {
        spdlog::debug("Test: Turning back on output to lights.");
        EnableOutputs();
    }

    // Restart the timer without changing the interval
    if (timerRunning) {
        OutputTimer.Start();
    }

    // resume playing the media if it was playing
    if (mps == MEDIAPLAYINGSTATE::PLAYING) {
        spdlog::debug("Test: Resuming play.");
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
    if (CurrentSeqXmlFile != nullptr && CurrentSeqXmlFile->GetMedia() != nullptr) {
        mps = CurrentSeqXmlFile->GetMedia()->GetPlayingState();
        if (mps == MEDIAPLAYINGSTATE::PLAYING) {
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
    if (mps == MEDIAPLAYINGSTATE::PLAYING) {
        CurrentSeqXmlFile->GetMedia()->Play();
        SetAudioControls();
    }
}

void xLightsFrame::OnPaneClose(wxAuiManagerEvent& event)
{
    SetFocus();
    if (event.pane != nullptr)
        event.pane->Hide();
    UpdateViewMenu();
}

void xLightsFrame::CreateDebugReport(xlCrashHandler* crashHandler)
{
    
    wxDebugReportCompress* const report = &crashHandler->GetDebugReport();

    report->SetCompressedFileDirectory(CurrentDir);

    wxFileName fn(CurrentDir, OutputManager::GetNetworksFileName());
    if (FileExists(fn)) {
        report->AddFile(fn.GetFullPath(), OutputManager::GetNetworksFileName());
    }
    if (FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xml"))) {
        report->AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xml").GetFullPath(), "xlights_rgbeffects.xml");
    }
    if (UnsavedRgbEffectsChanges && FileExists(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp"))) {
        report->AddFile(wxFileName(CurrentDir, "xlights_rgbeffects.xbkp").GetFullPath(), "xlights_rgbeffects.xbkp");
    }

    if (GetSeqXmlFileName() != "") {
        wxFileName fn2(GetSeqXmlFileName());
        if (FileExists(fn2) && !fn2.IsDir()) {
            report->AddFile(GetSeqXmlFileName(), fn2.GetName());
            wxFileName fnb(fn2.GetPath() + "/" + fn2.GetName() + ".xbkp");
            if (FileExists(fnb)) {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        } else {
            wxFileName fnb(CurrentDir + "/" + "__.xbkp");
            if (FileExists(fnb)) {
                report->AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
    } else {
        wxFileName fnb(CurrentDir + "/" + "__.xbkp");
        if (FileExists(fnb)) {
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
    for (auto& a : traceMessages) {
        threadStatus += a;
        threadStatus += "\n";
    }

    report->AddText("threads.txt", threadStatus, "Threads Status");
    spdlog::critical("{}", (const char*)threadStatus.c_str());

    crashHandler->ProcessCrashReport(xlCrashHandler::SendReportOptions::ASK_USER_TO_SEND);
}

void xLightsFrame::OnMenuItemPackageDebugFiles(wxCommandEvent& event)
{
    PackageDebugFiles();
}

std::string xLightsFrame::PackageDebugFiles(bool showDialog)
{

    wxString zipFileName{ "xLightsProblem.zip" };
    wxString zipDir{ CurrentDir };

    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written

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

    spdlog::debug("Dumping registry configuration:");
    auto* config = GetXLightsConfig();
    std::string key;
    long index;
    bool ce = config->GetFirstEntry(key, index);

    while (ce) {
        std::string value = "<UNKNOWN>";
        std::string type = "<UNREAD>";
        switch (config->GetEntryType(key)) {
        case XLightsConfigAdapter::EntryType::String:
            type = "String";
            config->Read(key, &value);
            break;
        case XLightsConfigAdapter::EntryType::Boolean:
            type = "Boolean";
            value = wxString::Format("%s", config->ReadBool(key, false) ? "True" : "False").ToStdString();
            break;
        case XLightsConfigAdapter::EntryType::Integer:
            type = "Integer";
            value = wxString::Format("%ld", config->ReadLong(key, 0)).ToStdString();
            break;
        case XLightsConfigAdapter::EntryType::Float:
            type = "Float";
            value = wxString::Format("%f", config->ReadDouble(key, 0.0)).ToStdString();
            break;
        case XLightsConfigAdapter::EntryType::Unknown:
            type = "Unknown";
            break;
        default:
            break;
        }

        spdlog::debug("      '{}' ({}) ='{}'", (const char*)key.c_str(), (const char*)type.c_str(), (const char*)value.c_str());

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
        wxRemoveFile(filenamee);
    }

    report.Process();

    wxRemoveFile(filename);

    return zipDir + wxFileName::GetPathSeparator() + zipFileName;
}

static void AddLogFile(const wxString& CurrentDir, const wxString& fileName, wxDebugReport& report)
{
    wxString const filename = GetLogFilePath().string();

    if (FileExists(filename)) {
        report.AddFile(filename, fileName);
    } else if (FileExists(wxFileName(CurrentDir, fileName).GetFullPath())) {
        report.AddFile(wxFileName(CurrentDir, fileName).GetFullPath(), fileName);
    } else if (FileExists(wxFileName(wxGetCwd(), fileName).GetFullPath())) {
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
    AddLogFile(CurrentDir, "xLights_spdlog.log", report);
    // if the rolled log exists, add it to just in case it has the information we need
    AddLogFile(CurrentDir, "xLights_spdlog.1.log", report);

#ifdef __APPLE__
    // MetricKit payloads (crash / hang / CPU / disk diagnostics + daily
    // metrics) accumulate in a Diagnostics/ folder next to the spdlog
    // file. Apple delivers payloads ~24h after the underlying event,
    // so the directory contains everything since the last crash zip
    // was processed; whichever zip ships next sweeps them up.
    {
        std::filesystem::path diagnosticsDir = GetLogFilePath().parent_path() / "Diagnostics";
        std::error_code ec;
        if (std::filesystem::exists(diagnosticsDir, ec) &&
            std::filesystem::is_directory(diagnosticsDir, ec)) {
            for (auto const& entry : std::filesystem::directory_iterator(diagnosticsDir, ec)) {
                if (!entry.is_regular_file(ec)) continue;
                std::string ext = entry.path().extension().string();
                if (ext != ".json") continue;
                std::string fullPath = entry.path().string();
                std::string archiveName = std::string("MetricKit/") + entry.path().filename().string();
                report.AddFile(wxString::FromUTF8(fullPath), wxString::FromUTF8(archiveName));
            }
        }
    }
#endif

    if (GetSeqXmlFileName() != "") {
        wxFileName fn2(GetSeqXmlFileName());
        if (FileExists(fn2) && !fn2.IsDir()) {
            report.AddFile(GetSeqXmlFileName(), fn2.GetName());
            if (mSavedChangeCount != (unsigned int)_sequenceElements.GetChangeCount()) {
                wxFileName fnb(fn2.GetPath() + "/" + fn2.GetName() + ".xbkp");
                if (FileExists(fnb)) {
                    report.AddFile(fnb.GetFullPath(), fnb.GetName());
                }
            }
        } else {
            if (mSavedChangeCount != (unsigned int)_sequenceElements.GetChangeCount()) {
                wxFileName fnb(CurrentDir + "/" + "__.xbkp");
                if (FileExists(fnb)) {
                    report.AddFile(fnb.GetFullPath(), fnb.GetName());
                }
            }
        }
    } else {
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            wxFileName fnb(CurrentDir + "/" + "__.xbkp");
            if (FileExists(fnb)) {
                report.AddFile(fnb.GetFullPath(), fnb.GetName());
            }
        }
    }
    // report.AddAll(wxDebugReport::Context_Current);
}

void xLightsFrame::SaveWorkingLayout()
{
    SaveEffectsFile(true);
}

bool xLightsFrame::SaveWorking()
{
    // dont save if no file in existence
    if (CurrentSeqXmlFile == nullptr)
        return true;

    // dont save if batch rendering
    if (_renderMode || _checkSequenceMode)
        return true;

    // dont save if currently saving
    std::unique_lock<std::mutex> lock(saveLock, std::try_to_lock);
    if (!lock.owns_lock())
        return true;

    wxString p = CurrentSeqXmlFile->GetPath();
    wxString fn = CurrentSeqXmlFile->GetFullName();
    wxString tmp;

    wxFileName fnp(fn);
    if (fn == "") {
        tmp = p + "/" + "__.xbkp";
    } else if ((size_t)CountChar(fnp.GetName(), '_') == fnp.GetName().size()) {
        tmp = p + "/" + fnp.GetName() + "_.xbkp";
    } else {
        tmp = p + "/" + fnp.GetName() + ".xbkp";
    }
    wxFileName ftmp(tmp);

    std::string origPath = CurrentSeqXmlFile->GetFullPath();
    CurrentSeqXmlFile->SetFullPath(tmp.ToStdString());

    // Sync jukebox UI state to sequence data before saving
    if (GetJukeboxPanel()) {
        GetJukeboxPanel()->SyncToData(CurrentSeqXmlFile->GetJukeboxButtons());
    }
    bool b = CurrentSeqXmlFile->Save(_sequenceElements);
    if (!b) {
        wxMessageDialog msgDlg(this, "Error Saving Sequence to " + tmp,
                               "Error Saving Sequence", wxOK | wxCENTRE);
        msgDlg.ShowModal();
    }

    CurrentSeqXmlFile->SetFullPath(origPath);
    return b;
}

void xLightsFrame::OnTimer_AutoSaveTrigger(wxTimerEvent& event)
{
    
    // dont save if currently playing or in render mode
    if (playType != PLAY_TYPE_MODEL && !_renderMode && !_checkSequenceMode && !_suspendAutoSave) {
        spdlog::debug("Autosaving backup of sequence.");
        wxStopWatch sw;
        if (mSavedChangeCount != _sequenceElements.GetChangeCount()) {
            if (_sequenceElements.GetChangeCount() != mLastAutosaveCount) {
                if (SaveWorking()) {
                    mLastAutosaveCount = _sequenceElements.GetChangeCount();
                }
            } else {
                spdlog::debug("    Autosave skipped ... no changes detected since last autosave.");
            }
        } else {
            spdlog::debug("    Autosave skipped ... no changes detected since last save.");
            mLastAutosaveCount = _sequenceElements.GetChangeCount();
        }
        if (UnsavedRgbEffectsChanges) {
            spdlog::debug("    Autosaving backup of layout.");
            SaveWorkingLayout();
        }
        spdlog::debug("    AutoSave took {} ms.", sw.Time());

        if (mAutoSaveInterval > 0) {
            AutoSaveTimer.StartOnce(mAutoSaveInterval * 60000);
        }
    } else {
        if (_renderMode) {
            static bool logged = false;
            if (!logged) {
                spdlog::debug("AutoSave skipped because batch rendering.");
                logged = true;
            }
        } else {
            static bool logged = false;
            if (!logged) {
                spdlog::debug("AutoSave skipped because sequence is playing or suspended.");
                logged = true;
            }
        }
        if (mAutoSaveInterval > 0) {
            AutoSaveTimer.StartOnce(1000); // try again in a short period of time as we did not actually save this time
        }
    }
}

void xLightsFrame::SetAutoSaveInterval(int nasi)
{
    if (nasi != mAutoSaveInterval) {
        auto* config = GetXLightsConfig();
        config->Write("AutoSaveInterval", nasi);
        mAutoSaveInterval = nasi;
    }
    if (mAutoSaveInterval > 0) {
        AutoSaveTimer.StartOnce(mAutoSaveInterval * 60000);
    } else {
        AutoSaveTimer.Stop();
    }
}

void xLightsFrame::AddPreviewOption(LayoutGroup* grp)
{
    bool menu_created = false;
    if (MenuItemPreviews == nullptr) {
        MenuItemPreviews = new wxMenu();
        MenuView->Insert(3, ID_MENU_ITEM_PREVIEWS, _("Previews"), MenuItemPreviews, wxEmptyString);
        menu_created = true;
    }
    if (LayoutGroups.size() > 1) {
        wxMenuItem* first_item = MenuItemPreviews->GetMenuItems().GetFirst()->GetData();
        if (first_item->GetId() != ID_MENU_ITEM_PREVIEWS_SHOW_ALL) {
            wxMenuItem* menu_item = new wxMenuItem(MenuItemPreviews, ID_MENU_ITEM_PREVIEWS_SHOW_ALL, _("Show All"), wxEmptyString, wxITEM_CHECK);
            MenuItemPreviews->Insert(0, menu_item);
            Connect(ID_MENU_ITEM_PREVIEWS_SHOW_ALL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideAllPreviewWindows, nullptr, this);
        }
    }
    grp->AddToPreviewMenu(MenuItemPreviews);
    Connect(grp->GetMenuId(), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHidePreviewWindow, nullptr, this);
    if (menu_created) {
        MenuItemPreviewSeparator = MenuView->InsertSeparator(4);
    }
}

void xLightsFrame::RemovePreviewOption(LayoutGroup* grp)
{
    Disconnect(grp->GetMenuId(), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHidePreviewWindow, nullptr, this);
    grp->RemoveFromPreviewMenu(MenuItemPreviews);
    if (LayoutGroups.size() == 1) {
        Disconnect(ID_MENU_ITEM_PREVIEWS_SHOW_ALL, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::ShowHideAllPreviewWindows, nullptr, this);
        MenuView->Delete(ID_MENU_ITEM_PREVIEWS);
        MenuItemPreviews = nullptr;
        MenuView->Delete(MenuItemPreviewSeparator);
    }
}

void xLightsFrame::ShowHidePreviewWindow(wxCommandEvent& event)
{
    wxMenuItem* item = MenuItemPreviews->FindItem(event.GetId());
    for (const auto& [name, grp] : LayoutGroups) {
        if (grp->GetMenuItem() == item) {
            grp->ShowPreview(item->IsChecked());
        }
    }
}

void xLightsFrame::ShowHideAllPreviewWindows(wxCommandEvent& event)
{
    wxMenuItem* first_item = MenuItemPreviews->GetMenuItems().GetFirst()->GetData();
    for (const auto& [name, grp] : LayoutGroups) {
        grp->ShowPreview(first_item->IsChecked());
    }
}

void xLightsFrame::DoAltBackup(bool prompt)
{
    if (mAltBackupDir == "" || !wxDirExists(mAltBackupDir)) {
        DisplayError(wxString::Format("Alternate backup directory location not defined or does not exist: '%s'", mAltBackupDir).ToStdString());
        return;
    }

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

    wxString newDir = mAltBackupDir + wxFileName::GetPathSeparator() + wxString::Format("Backup%c%s-%s", wxFileName::GetPathSeparator(), curTime.FormatISODate(), curTime.Format("%H%M%S"));

    if (prompt) {
        if (wxNO == wxMessageBox("All xml & xsq files under " + wxString::Format("%i", MAXBACKUPFILE_MB) + "MB in your xlights directory will be backed up to \"" +
                                     newDir + "\". Proceed?",
                                 "Backup", wxICON_QUESTION | wxYES_NO)) {
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
    
    auto* config = GetXLightsConfig();

    wxString setting;
    mediaDirectories.clear();
    for (auto const& dir : folders) {
        ObtainAccessToURL(dir);
        if (std::find(mediaDirectories.begin(), mediaDirectories.end(), dir) == mediaDirectories.end()) {
            mediaDirectories.push_back(dir);
            spdlog::debug("Adding Media directory: {}.", (const char*)dir.c_str());
            if (setting != "") {
                setting += "|";
            }
            setting += dir;
        }
    }
    config->Write("MediaDir", setting);
    FileUtils::SetFixFileDirectories(mediaDirectories);
    mediaDirectories.push_back(showDirectory);
}

void xLightsFrame::GetFSEQFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == fseqDirectory);
    folder = fseqDirectory;
}

void xLightsFrame::SetFSEQFolder(bool useShow, const std::string& folder)
{

    auto* config = GetXLightsConfig();

    if (useShow) {
        config->Write("FSEQLinkFlag", true);
        if (fseqDirectory == showDirectory)
            return;
        fseqDirectory = showDirectory;
    } else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            config->Write("FSEQLinkFlag", false);
            if (fseqDirectory == folder)
                return;
            fseqDirectory = folder;
        } else {
            DisplayError("FSEQ directory does not exist. FSEQ folder was not changed to " + folder + ". FSEQ folder remains : " + fseqDirectory, this);
            return;
        }
    }

    SetXmlSetting("fseqDir", fseqDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    spdlog::debug("FSEQ directory set to : {}.", (const char*)fseqDirectory.c_str());
}

void xLightsFrame::GetRenderCacheFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == renderCacheDirectory);
    folder = renderCacheDirectory;
}

void xLightsFrame::SetRenderCacheFolder(bool useShow, const std::string& folder)
{

    if (useShow) {
        if (renderCacheDirectory == showDirectory)
            return;
        renderCacheDirectory = showDirectory;
    } else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            if (renderCacheDirectory == folder)
                return;
            renderCacheDirectory = folder;
        } else {
            DisplayError("Render Cache directory does not exist. Render Cache folder was not changed to " + folder + ". Render Cache folder remains : " + renderCacheDirectory, this);
            return;
        }
    }

    SetXmlSetting("renderCacheDir", renderCacheDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    spdlog::debug("Render Cache directory set to : {}.", (const char*)renderCacheDirectory.c_str());
}

void xLightsFrame::GetBackupFolder(bool& useShow, std::string& folder)
{
    useShow = (showDirectory == _backupDirectory);
    folder = _backupDirectory;
}

void xLightsFrame::SetBackupFolder(bool useShow, const std::string& folder)
{

    if (useShow) {
        if (_backupDirectory == showDirectory)
            return;
        _backupDirectory = showDirectory;
    } else {
        if (wxDir::Exists(folder)) {
            ObtainAccessToURL(folder);
            if (_backupDirectory == folder)
                return;
            _backupDirectory = folder;
        } else {
            DisplayError("Backup directory does not exist. Backup folder was not changed to " + folder + ". Backup folder remains : " + _backupDirectory, this);
            return;
        }
    }

    SetXmlSetting("backupDir", _backupDirectory);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();

    spdlog::debug("Backup directory set to : {}.", (const char*)_backupDirectory.c_str());
}

void xLightsFrame::GetAltBackupFolder(std::string& folder)
{
    folder = mAltBackupDir;
}

void xLightsFrame::SetAltBackupFolder(const std::string& folder)
{

    if (folder == mAltBackupDir)
        return;

    auto* config = GetXLightsConfig();

    if (folder != "" && !wxDir::Exists(folder)) {
        DisplayError("Alternate backup directory does not exist. Alternate backup folder was not changed to " + folder + ". Alternate backup folder remains : " + mAltBackupDir, this);
    } else {
        ObtainAccessToURL(folder);
        config->Write("xLightsAltBackupDir", folder);
        mAltBackupDir = folder;
        spdlog::debug("Alt Backup directory set to : {}.", (const char*)mAltBackupDir.c_str());
    }
}

void xLightsFrame::OnmAltBackupMenuItemSelected(wxCommandEvent& event)
{
    if (mAltBackupDir == "") {
        wxDirDialog dir(this, _("Select alternate backup directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
        if (dir.ShowModal() == wxID_OK) {
            mAltBackupDir = dir.GetPath();
            
            spdlog::info("Alternate backup location set to {}.", (const char*)mAltBackupDir.c_str());
        }
    }

    if (mAltBackupDir == "") {
        return;
    }
    if (!ObtainAccessToURL(mAltBackupDir, true)) {
        std::string orig = mAltBackupDir;
        PromptForDirectorySelection("Reselect alternate backup directory", orig);
        mAltBackupDir = orig;
    }
    SaveWorking();

    DoAltBackup();
}

void xLightsFrame::OnmExportModelsMenuItemSelected(wxCommandEvent& event)
{
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "Export Models", wxEmptyString, "Export files (*.xlsx)|*.xlsx", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) {
        return;
    }

    ExportModels(filename);
    SetStatusText("Model Spreadsheet saved at " + filename);
}

void xLightsFrame::OnMenuItem_ViewLogSelected(wxCommandEvent& event)
{
    wxString filePath = GetLogFilePath().string();
    wxString fileName = GetLogFileName();

    wxString fn = "";
    if (FileExists(filePath)) {
        fn = filePath;
    } else if (FileExists(wxFileName(CurrentDir, fileName).GetFullPath())) {
        fn = wxFileName(CurrentDir, fileName).GetFullPath();
    } else if (FileExists(wxFileName(wxGetCwd(), fileName).GetFullPath())) {
        fn = wxFileName(wxGetCwd(), fileName).GetFullPath();
    }

    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (fn != "" && ft) {
        wxString command = ft->GetOpenCommand("foo.txt");
        command.Replace("foo.txt", fn);

        spdlog::debug("Viewing log file {}.", (const char*)fn.c_str());

        wxUnsetEnv("LD_PRELOAD");
        wxExecute(command);
        delete ft;
    } else {
        DisplayError(wxString::Format("Unable to show log file '%s'.", fn).ToStdString(), this);
    }
}

namespace {

void LogCheckSequenceMsg(const std::string& msg) {
    spdlog::debug("CheckSequence: " + msg);
}

// Helpers used by the wx-only network / preferences / OS sections of CheckSequence.
// The bulk of the checks delegate to SequenceChecker which has its own RecordIssue.
void LogAndTrack(CheckSequenceReport& report,
                 const std::string& sectionId, CheckSequenceReport::ReportIssue::Type type, const std::string& msg,
                 const std::string& category, size_t& errcount, size_t& warncount) {
    LogCheckSequenceMsg(msg);
    report.AddIssue(sectionId, CheckSequenceReport::ReportIssue(type, msg, category));

    if (type == CheckSequenceReport::ReportIssue::CRITICAL)
        errcount++;
    else if (type == CheckSequenceReport::ReportIssue::WARNING)
        warncount++;
}

void LogAndTrackInfo(CheckSequenceReport& report,
                     const std::string& sectionId,
                     const std::string& msg,
                     const std::string& category = "info") {
    LogCheckSequenceMsg(msg);
    report.AddIssue(sectionId, CheckSequenceReport::ReportIssue(CheckSequenceReport::ReportIssue::INFO, msg, category));
}

// Forwards SequenceChecker callbacks to the wx desktop:
//  - Reads per-check disable toggles from xLightsFrame's static accessor.
//  - Returns the desktop's render-cache preference string.
//  - Drives the wxProgressDialog the wx wrapper opened.
//  - FFmpeg desktop builds decode every video format, so the codec
//    compatibility probe always reports OK.
class DesktopCheckCallbacks : public SequenceCheckerCallbacks {
public:
    DesktopCheckCallbacks(xLightsFrame* frame, wxProgressDialog* prog) : _frame(frame), _prog(prog) {}

    bool IsCheckOptionDisabled(const std::string& option) const override {
        return xLightsFrame::IsCheckSequenceOptionDisabledS(option);
    }

    std::string GetRenderCacheMode() const override {
        return _frame->EnableRenderCache().ToStdString();
    }

    std::string CheckVideoCompatibility(const std::string& /*path*/) override {
        return "";
    }

    void OnProgress(int percent, const std::string& step) override {
        if (_prog != nullptr) {
            // SequenceChecker reports 0..100 across all four Run* calls. The
            // wrapper carves out 5..95 for the delegated checks (network/prefs
            // get 0..5 and OS checks get 95..100).
            int scaled = 5 + (percent * 90) / 100;
            _prog->Update(scaled, step);
            wxYield();
        }
    }

private:
    xLightsFrame* _frame;
    wxProgressDialog* _prog;
};

} // namespace

std::string xLightsFrame::CheckSequence(bool displayInEditor, bool writeToFile)
{

    // make sure everything is up to date
    if (Notebook1->GetSelection() != LAYOUTTAB)
        layoutPanel->UnSelectAllModels();
    RecalcModels();

    size_t errcount = 0;
    size_t warncount = 0;
    size_t toterrcount = 0;
    size_t totwarncount = 0;

    wxFile f;
    wxFileName fnTemp;
    fnTemp.AssignTempFileName("xLightsCheckSequence");
    wxFileName fn(GetShowDirectory(), fnTemp.GetFullName() + ".html");
    wxString filename = fn.GetFullPath();
    TempFileManager::GetTempFileManager().AddTempFile(filename);

    if (writeToFile || displayInEditor) {
        f.Open(filename, wxFile::write);
        if (!f.IsOpened()) {
            DisplayError("Unable to create results file for Check Sequence. Aborted.", this);
            return "";
        }
    }

    wxProgressDialog prog("Check Sequence", "", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();
    CheckSequenceReport report;

    // Add all predefined sections
    for (const auto& section : CheckSequenceReport::REPORT_SECTIONS) {
        report.AddSection(section);
    }

    LogCheckSequenceMsg("Checking sequence.");
    LogCheckSequenceMsg("");

    LogCheckSequenceMsg("Show folder: " + GetShowDirectory());
    report.SetShowFolder(GetShowDirectory());

    if (CurrentSeqXmlFile != nullptr) {
        wxFileName fn(CurrentSeqXmlFile->GetFullPath());
        fn.SetExt("xsq");
        LogCheckSequenceMsg("Sequence: " + fn.GetFullPath());
        report.SetSequencePath(fn.GetFullPath());
    } else {
        LogCheckSequenceMsg("Sequence: No sequence open.");
    }

    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("Network Checks");

    prog.Update(0, "Checking network");
    wxYield();

    wxDatagramSocket* testSocket;
    wxIPV4address addr;
    wxString fullhostname = wxGetFullHostName();
    if (_outputManager.GetGlobalForceLocalIP() != "") {
        addr.Hostname(_outputManager.GetGlobalForceLocalIP());
        testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
    } else {
        addr.AnyAddress();
        testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
        addr.Hostname(fullhostname);
        if (addr.IPAddress() == "255.255.255.255") {
            addr.Hostname(wxGetHostName());
        }
    }

    LogAndTrackInfo(report, "network", "Full host name: " + fullhostname.ToStdString(), "network_info");
    LogAndTrackInfo(report, "network", "IP Address we are outputting data from: " + addr.IPAddress().ToStdString(), "network_info");
    LogAndTrackInfo(report, "network", "If your PC has multiple network connections (such as wired and wireless) this should be the IP Address of the adapter your controllers are connected to. If it isn't your controllers may not receive output data.", "network_info");
    LogAndTrackInfo(report, "network", "If you are experiencing this problem you may need to set the local IP address to use.", "network_info");

    if (testSocket == nullptr || !testSocket->IsOk() || testSocket->Error()) {
        wxString msg("    ERR: Cannot create socket on IP address '");
        msg += addr.IPAddress();
        msg += "'. Is the network connected?    ";
        msg = msg + " Ok : " + (testSocket->IsOk() ? "TRUE" : "FALSE");
        if (testSocket != nullptr && testSocket->IsOk()) {
            msg += wxString::Format(" : Error %d : ", testSocket->LastError()) + DecodeIPError(testSocket->LastError());
        }
        LogAndTrack(report, "network", CheckSequenceReport::ReportIssue::CRITICAL, msg.ToStdString(), "socket", errcount, warncount);
    }

    if (testSocket != nullptr) {
        delete testSocket;
    }

    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("IP Addresses on this machine:");
    for (const auto& it : ip_utils::GetLocalIPs()) {
        LogAndTrackInfo(report, "network", wxString::Format("    %s", it), "network_ips");
    }

    size_t errcountsave = errcount;
    size_t warncountsave = warncount;

    LogCheckSequenceMsg(wxString::Format("\nSection Errors (Network): %u. Warnings: %u", (unsigned int)errcount, (unsigned int)warncount).ToStdString());
    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    toterrcount += errcount;
    totwarncount += warncount;
    errcount = 0;
    warncount = 0;

    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("Preference Checks");

    prog.Update(1, "Checking preferences");
    wxYield();

    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("Working in a backup directory");

    if (ShowFolderIsInBackup(GetShowDirectory())) {
        wxString msg = wxString::Format("    ERR: Show directory is a (or is under a) backup show directory. %s.", GetShowDirectory());
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::CRITICAL, msg.ToStdString(), "backup", errcount, warncount);
    }

    if (errcount + warncount == errcountsave + warncountsave) {
        LogCheckSequenceMsg("    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("Potentially problematic settings");

    if (!_backupSubfolders) {
        wxString msg = wxString::Format("    WARN: Backup is not including subfolders. If you store your sequences in subfolders then they are NOT being backed up by xLights.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "backup", errcount, warncount);
    }

    if (_suspendRender) {
        wxString msg = wxString::Format("    WARN: Rendering is currently suspended. The FSEQ data may not be up to date.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "rendering", errcount, warncount);
    }

    if (mRenderOnSave) {
        wxString msg = wxString::Format("    WARN: Render on save is enabled ... this is generally unnecessary.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "rendering", errcount, warncount);
    }

    if (mBackupOnSave) {
        wxString msg = wxString::Format("    WARN: Backup on save is enabled ... this creates an awful lot of backups.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "backup", errcount, warncount);
    }

    if (!mSaveFseqOnSave) {
        wxString msg = wxString::Format("    ERR: Saving FSEQ on save is disabled ... this is almost always a bad idea.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::CRITICAL, msg.ToStdString(), "autosave", errcount, warncount);
    }

    if (mAutoSaveInterval <= 0) {
        wxString msg = wxString::Format("    WARN: Autosave is disabled ... you will lose work if xLights abnormally terminates.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "autosave", errcount, warncount);
    }

    if (_lowDefinitionRender) {
        wxString msg = wxString::Format("    WARN: Rendering in low definition is active.");
        LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "rendering", errcount, warncount);
    }

    if (mBackgroundImage != "") {
        if (!wxIsReadable(mBackgroundImage) || !wxImage::CanRead(mBackgroundImage)) {
            wxString msg = wxString::Format("    ERR: Layout Background image not loadable as an image: %s.", mBackgroundImage);
            LogAndTrack(report, "preferences", CheckSequenceReport::ReportIssue::CRITICAL, msg.ToStdString(), "layout", errcount, warncount);
        }
    }

   if (errcount + warncount == errcountsave + warncountsave) {
        LogCheckSequenceMsg("    No problems found");
    }

    errcountsave = errcount;
    warncountsave = warncount;

    LogCheckSequenceMsg(wxString::Format("\nSection Errors (Preferences): %u. Warnings: %u", (unsigned int)errcount, (unsigned int)warncount).ToStdString());
    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    toterrcount += errcount;
    totwarncount += warncount;
    errcount = 0;
    warncount = 0;

    // Delegate the bulk of the checks to SequenceChecker (controllers, models,
    // per-effect/per-element walk, file references). The wx-only chunks above
    // (network/preferences) and below (AllObjects, OS performance probe, HTML)
    // stay here.
    DesktopCheckCallbacks cb(this, &prog);
    SequenceChecker checker(_sequenceElements, AllModels, _outputManager,
                            CurrentSeqXmlFile, GetShowDirectory(), &cb);

    int sectionStartErrors = report.GetTotalErrors();
    int sectionStartWarnings = report.GetTotalWarnings();
    checker.RunControllerChecks(report);
    LogCheckSequenceMsg(wxString::Format("\nSection Errors (Controllers): %u. Warnings: %u",
                                         (unsigned int)(report.GetTotalErrors() - sectionStartErrors),
                                         (unsigned int)(report.GetTotalWarnings() - sectionStartWarnings)).ToStdString());
    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    toterrcount += (report.GetTotalErrors() - sectionStartErrors);
    totwarncount += (report.GetTotalWarnings() - sectionStartWarnings);

    sectionStartErrors = report.GetTotalErrors();
    sectionStartWarnings = report.GetTotalWarnings();
    checker.RunModelChecks(report);

    // ViewObjects live on xLightsFrame (wx-bound); run their settings checks
    // here so the resulting issues fold into the same Models section.
    for (const auto& it : AllObjects) {
        std::list<std::string> warnings = it.second->CheckModelSettings();
        for (const auto& w : warnings) {
            auto issueType = (w.find("WARN:") != std::string::npos)
                                 ? CheckSequenceReport::ReportIssue::WARNING
                                 : CheckSequenceReport::ReportIssue::CRITICAL;
            LogAndTrack(report, "models", issueType, w, "settings", errcount, warncount);
        }
    }
    LogCheckSequenceMsg(wxString::Format("\nSection Errors (Models): %u. Warnings: %u",
                                         (unsigned int)(report.GetTotalErrors() - sectionStartErrors),
                                         (unsigned int)(report.GetTotalWarnings() - sectionStartWarnings)).ToStdString());
    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    toterrcount += (report.GetTotalErrors() - sectionStartErrors);
    totwarncount += (report.GetTotalWarnings() - sectionStartWarnings);
    errcount = 0;
    warncount = 0;

    sectionStartErrors = report.GetTotalErrors();
    sectionStartWarnings = report.GetTotalWarnings();
    checker.RunSequenceChecks(report);

#ifndef __WXOSX__
    // OpenGL core-profile guard for shader effects — wx-bound because the
    // canvas wrapper lives in src-ui-wx/.
    bool sequenceUsesShader = false;
    for (size_t i = 0; i < _sequenceElements.GetElementCount(MASTER_VIEW); i++) {
        Element* e = _sequenceElements.GetElement(i);
        if (e == nullptr || e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
            continue;
        for (const auto& el : e->GetEffectLayers()) {
            for (const auto& ef : el->GetEffects()) {
                if (ef->GetEffectName() == "Shader") {
                    sequenceUsesShader = true;
                    break;
                }
            }
            if (sequenceUsesShader)
                break;
        }
        if (sequenceUsesShader)
            break;
    }
    if (sequenceUsesShader && mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr &&
        !mainSequencer->PanelEffectGrid->IsCoreProfile()) {
        wxString msg = "    ERR: Sequence has one or more shader effects but open GL version is lower than version 3. These effects may not render.";
        LogAndTrack(report, "sequence", CheckSequenceReport::ReportIssue::CRITICAL, msg.ToStdString(), "sequencegeneral", errcount, warncount);
    }
#endif

    LogCheckSequenceMsg(wxString::Format("\nSection Errors (Sequence): %u. Warnings: %u",
                                         (unsigned int)(report.GetTotalErrors() - sectionStartErrors),
                                         (unsigned int)(report.GetTotalWarnings() - sectionStartWarnings)).ToStdString());
    LogCheckSequenceMsg("-----------------------------------------------------------------------------------------------------------------");
    toterrcount += (report.GetTotalErrors() - sectionStartErrors);
    totwarncount += (report.GetTotalWarnings() - sectionStartWarnings);
    errcount = 0;
    warncount = 0;

    // RunFileReferenceChecks emits show-folder-name-repeated warnings into the
    // "os" section. Run it here so its issues are counted alongside the
    // BadDriveAccess probe below.
    sectionStartErrors = report.GetTotalErrors();
    sectionStartWarnings = report.GetTotalWarnings();
    checker.RunFileReferenceChecks(report);

    LogCheckSequenceMsg("OS Checks");

    prog.Update(95, "Checking performance");

    // Build the list of files referenced by models + effects. SequenceChecker
    // already builds this internally for RunFileReferenceChecks but doesn't
    // expose it; rebuilding here is cheap and keeps the BadDriveAccess probe
    // wx-bound (it lives on xLightsFrame).
    std::list<std::string> allfiles;
    for (const auto& it : AllModels) {
        allfiles.splice(allfiles.end(), it.second->GetFileReferences());
    }
    if (CurrentSeqXmlFile != nullptr) {
        EffectManager& em = _sequenceElements.GetEffectManager();
        for (size_t i = 0; i < _sequenceElements.GetElementCount(MASTER_VIEW); ++i) {
            Element* e = _sequenceElements.GetElement(i);
            if (e == nullptr || e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
                continue;
            ModelElement* me = dynamic_cast<ModelElement*>(e);
            Model* model = me ? AllModels[me->GetModelName()] : nullptr;
            for (const auto& el : e->GetEffectLayers()) {
                for (const auto& ef : el->GetEffects()) {
                    RenderableEffect* eff = em.GetEffect(ef->GetEffectIndex());
                    if (eff != nullptr && model != nullptr) {
                        allfiles.splice(allfiles.end(), eff->GetFileReferences(model, ef->GetSettings()));
                    }
                }
            }
        }
    }

#define SLOWDRIVE 1000
    std::list<std::pair<std::string, uint64_t>> slowaccess;
    BadDriveAccess(allfiles, slowaccess, SLOWDRIVE);
    if (slowaccess.size() > 0) {
        wxString msg = wxString::Format("    WARN: Test of access speed to files your sequence shows the following files take longer than the recommended %dms.", SLOWDRIVE / 1000);
        LogAndTrack(report, "os", CheckSequenceReport::ReportIssue::WARNING, msg.ToStdString(), "file_access", errcount, warncount);

        for (const auto& it : slowaccess) {
            msg = wxString::Format("    %.2fms  %s.", (float)((double)it.second / 1000.0), (const char*)it.first.c_str());
            LogAndTrackInfo(report, "os", msg.ToStdString(), "file_access");
        }
    }

    int osSectionErrors = report.GetTotalErrors() - sectionStartErrors;
    int osSectionWarnings = report.GetTotalWarnings() - sectionStartWarnings;
    if (osSectionErrors == 0 && osSectionWarnings == 0) {
        LogCheckSequenceMsg("    No problems found");
    }
    toterrcount += osSectionErrors;
    totwarncount += osSectionWarnings;

    LogCheckSequenceMsg(wxString::Format("\nSection Errors (OS): %u. Warnings: %u", (unsigned int)osSectionErrors, (unsigned int)osSectionWarnings).ToStdString());
    LogCheckSequenceMsg("=================================================================================================================");
    LogCheckSequenceMsg("");
    LogCheckSequenceMsg("Check sequence completed.");
    LogCheckSequenceMsg("");
    LogCheckSequenceMsg(wxString::Format("Total Errors: %u. Warnings: %u", (unsigned int)toterrcount, (unsigned int)totwarncount).ToStdString());

    prog.Update(100, "Done");
    wxYield();
    prog.Hide();

    if (f.IsOpened()) {
        wxString resourcesPath = GetResourcesDirectory();
        wxString srcCss = resourcesPath + wxFileName::GetPathSeparator() + "html" + wxFileName::GetPathSeparator() + "tailwind.min.css";
        wxString destCss = wxString(report.GetShowFolder()) + wxString(wxFileName::GetPathSeparator()) + "checksequence_tailwind.min.css";
        wxCopyFile(srcCss, destCss);
        TempFileManager::GetTempFileManager().AddTempFile(ToStdString(destCss));

        std::string html = report.GenerateHTML(IsDarkMode());
        f.Write(html.c_str(), html.length());
        f.Close();

        if (displayInEditor) {
            wxUnsetEnv("LD_PRELOAD");
            spdlog::debug("Viewing xLights Check Sequence results {}.", (const char*)filename.c_str());
            if (!wxLaunchDefaultApplication(filename)) {
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
        auto f = FileUtils::FixFile("", it);
        ObtainAccessToURL(f);
        if (!FileExists(f, false)) {
            missing += it + "\n";
        }
    }

    std::string relocated;
    for (const auto& [orig, resolved] : _sequenceElements.GetSequenceMedia().GetImageRelocations()) {
        relocated += orig + " -> " + resolved + "\n";
    }

    if ((!_renderMode && !_checkSequenceMode) || _promptBatchRenderIssues) {
        if (missing != "") {
            wxMessageBox("Sequence references files which cannot be found:\nShow Folder: " + showDirectory + "\n\n" + missing + "\n Use Tools/Check Sequence for more details.", "Missing assets");
        }
        if (relocated != "") {
            wxMessageBox("Sequence references files which have been moved. Paths will be updated on save:\nShow Folder: " + showDirectory + "\n\n" + relocated, "Relocated assets");
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

void xLightsFrame::OnMenuItem_File_NewXLightsInstance(wxCommandEvent& event)
{
#ifdef __WXOSX__
    if (!SpawnNewXLightsInstance(wxEmptyString)) {
        wxMessageBox(_("Could not locate the xLights application bundle. "
                       "A new instance can only be launched when xLights is run from a .app bundle."),
                     _("Open New xLights Instance"), wxOK | wxICON_WARNING, this);
    }
#endif
}

void xLightsFrame::OnMenuItem_Help_ReleaseNotesSelected(wxCommandEvent& event)
{
#ifdef __WXOSX__
    std::string loc = "https://raw.githubusercontent.com/xLightsSequencer/xLights/" + xlights_version_string + "/README.txt";
    std::string file = CachedFileDownloader::GetDefaultCache().GetFile(loc, CACHETIME_SESSION);
    if (file == "" || !FileExists(file)) {
        // a patch version may not have release notes so strip it off
        std::string vs = xlights_version_string;
        vs = vs.substr(0, vs.find_last_of("."));
        loc = "https://raw.githubusercontent.com/xLightsSequencer/xLights/" + vs + "/README.txt";
    }
    ::wxLaunchDefaultBrowser(loc);
#else
    wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (ft) {
        wxString command = ft->GetOpenCommand("README.txt");
        wxUnsetEnv("LD_PRELOAD");
        wxExecute(command);
    }
#endif
}

void xLightsFrame::OnMenuItem_Help_Isue_TrackerSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://github.com/xLightsSequencer/xLights/issues");
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

    for (int k = 0; k < nl->GetEffectCount(); k++) {
        Effect* ef = nl->GetEffect(k);

        std::string fs = "";
        if (ef->GetEffectIndex() >= 0) {
            RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
            auto files = eff->GetFileReferences(m, ef->GetSettings());

            for (auto it = files.begin(); it != files.end(); ++it) {
                if (fs != "") {
                    fs += ",";
                }
                fs += (*it);
            }

            allfiles.splice(allfiles.end(), files);
        }

        int duration = ef->GetEndTimeMS() - ef->GetStartTimeMS();
        if (effectfrequency.find(ef->GetEffectName()) != effectfrequency.end()) {
            effectfrequency[ef->GetEffectName()]++;
            effectTotalTime[ef->GetEffectName()] += duration;
        } else {
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
                                 sm.Contains("X_Effect_Description") ? std::string(sm["X_Effect_Description"]) : std::string(""),
                                 name,
                                 type,
                                 fs));
        effects++;
    }

    return effects;
}

int xLightsFrame::ExportElement(wxFile& f, Element* e, std::map<std::string, int>& effectfrequency, std::map<std::string, int>& effectTotalTime, std::list<std::string>& allfiles)
{
    int effects = 0;

    if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        Model* m = AllModels.GetModel(e->GetModelName());

        wxString type = "Unknown";
        switch (e->GetType()) {
        case ElementType::ELEMENT_TYPE_MODEL:
            if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                type = "Model Group";
            } else {
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

        for (int j = 0; j < (int)e->GetEffectLayerCount(); j++) {
            EffectLayer* el = e->GetEffectLayer(j);

            for (int k = 0; k < el->GetEffectCount(); k++) {
                Effect* ef = el->GetEffect(k);
                std::string fs = "";
                if (ef->GetEffectIndex() >= 0) {
                    RenderableEffect* eff = effectManager[ef->GetEffectIndex()];
                    auto files = eff->GetFileReferences(m, ef->GetSettings());

                    for (auto it = files.begin(); it != files.end(); ++it) {
                        if (fs != "") {
                            fs += ",";
                        }
                        fs += (*it);
                    }

                    allfiles.splice(allfiles.end(), files);
                }

                int duration = ef->GetEndTimeMS() - ef->GetStartTimeMS();
                if (effectfrequency.find(ef->GetEffectName()) != effectfrequency.end()) {
                    effectfrequency[ef->GetEffectName()]++;
                    effectTotalTime[ef->GetEffectName()] += duration;
                } else {
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
                                         sm.Contains("X_Effect_Description") ? std::string(sm["X_Effect_Description"]) : std::string(""),
                                         (const char*)(e->GetFullName()).c_str(),
                                         type,
                                         fs));
                effects++;
            }
        }
    }

    return effects;
}

void xLightsFrame::OnMenuItem_ExportEffectsSelected(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile == nullptr) {
        DisplayError("No sequence open", this);
        return;
    }

    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, CurrentSeqXmlFile->GetName(), wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty())
        return;

    ExportEffects(filename);
    SetStatusText("Effects CSV saved at " + filename);
}

void xLightsFrame::OnMenuItemShiftEffectsAndTimingSelected(wxCommandEvent& event)
{
    if (CurrentSeqXmlFile == nullptr)
        return;
    wxTextEntryDialog ted(this, "Enter the number of milliseconds to shift all effects and timing marks:\n\n"
        "Note: Will be rounded to the nearest timing interval.\n"
        "      This operation cannot be reversed with Undo.\n"
        "      Effects shifted left may be truncated or deleted.",
        "Shift Effects And Timings", "", wxOK | wxCANCEL | wxCENTER);
    if (ted.ShowModal() == wxID_OK) {
        int milliseconds = wxAtoi(ted.GetValue());
        if (CurrentSeqXmlFile->GetSequenceLoaded()) {
            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);
            milliseconds /= ms;
            milliseconds *= ms;
        }
        for (int elem = 0; elem < (int)_sequenceElements.GetElementCount(MASTER_VIEW); elem++) {
            Element* ele = _sequenceElements.GetElement(elem, MASTER_VIEW);
            for (int layer = 0; layer < (int)ele->GetEffectLayerCount(); layer++) {
                EffectLayer* el = ele->GetEffectLayer(layer);
                ShiftEffectsOnLayer(el, milliseconds);
            }
            if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* me = dynamic_cast<ModelElement*>(ele);
                for (int i = 0; i < me->GetStrandCount(); ++i) {
                    Element* se = me->GetStrand(i);
                    StrandElement* ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); ++k) {
                        NodeLayer* nl = ste->GetNodeLayer(k, false);
                        if (nl != nullptr) {
                            ShiftEffectsOnLayer(nl, milliseconds);
                        }
                    }
                }
                for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i) {
                    Element* se = me->GetSubModel(i);
                    for (int layer = 0; layer < (int)se->GetEffectLayerCount(); layer++) {
                        EffectLayer* sel = se->GetEffectLayer(layer);
                        ShiftEffectsOnLayer(sel, milliseconds);
                    }
                }
            }
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
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
        for (int elem = 0; elem < (int)_sequenceElements.GetElementCount(MASTER_VIEW); elem++) {
            Element* ele = _sequenceElements.GetElement(elem, MASTER_VIEW);
            for (int layer = 0; layer < (int)ele->GetEffectLayerCount(); layer++) {
                EffectLayer* el = ele->GetEffectLayer(layer);
                ShiftSelectedEffectsOnLayer(el, milliseconds);
            }
            if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* me = dynamic_cast<ModelElement*>(ele);
                for (int i = 0; i < me->GetStrandCount(); ++i) {
                    Element* se = me->GetStrand(i);
                    StrandElement* ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); ++k) {
                        NodeLayer* nl = ste->GetNodeLayer(k, false);
                        if (nl != nullptr) {
                            ShiftSelectedEffectsOnLayer(nl, milliseconds);
                        }
                    }
                }
                for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i) {
                    Element* se = me->GetSubModel(i);
                    for (int layer = 0; layer < (int)se->GetEffectLayerCount(); layer++) {
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
    if (CurrentSeqXmlFile == nullptr)
        return;
    wxTextEntryDialog ted(this, "Enter the number of milliseconds to shift all effects:\n\n"
                                "Note: Will be rounded to the nearest timing interval.\n"
                                "      This operation cannot be reversed with Undo.\n"
                                "      Effects shifted left may be truncated or deleted.",
                          "Shift Effects", "", wxOK | wxCANCEL | wxCENTER);
    if (ted.ShowModal() == wxID_OK) {
        int milliseconds = wxAtoi(ted.GetValue());
        if (CurrentSeqXmlFile->GetSequenceLoaded()) {
            wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
            int ms = wxAtoi(mss);
            milliseconds /= ms;
            milliseconds *= ms;
        }
        for (int elem = 0; elem < (int)_sequenceElements.GetElementCount(MASTER_VIEW); elem++) {
            Element* ele = _sequenceElements.GetElement(elem, MASTER_VIEW);
            if (ele->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                for (int layer = 0; layer < (int)ele->GetEffectLayerCount(); layer++) {
                    EffectLayer* el = ele->GetEffectLayer(layer);
                    ShiftEffectsOnLayer(el, milliseconds);
                }
            }
            if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* me = dynamic_cast<ModelElement*>(ele);
                for (int i = 0; i < me->GetStrandCount(); ++i) {
                    Element* se = me->GetStrand(i);
                    StrandElement* ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); ++k) {
                        NodeLayer* nl = ste->GetNodeLayer(k, false);
                        if (nl != nullptr) {
                            ShiftEffectsOnLayer(nl, milliseconds);
                        }
                    }
                }
                for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i) {
                    Element* se = me->GetSubModel(i);
                    for (int layer = 0; layer < (int)se->GetEffectLayerCount(); layer++) {
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
    for (int ef = el->GetEffectCount() - 1; ef >= 0; ef--) { // count backwards so we can delete if needed
        Effect* eff = el->GetEffect(ef);
        int start_ms = eff->GetStartTimeMS();
        int end_ms = eff->GetEndTimeMS();
        if (start_ms + milliseconds < 0) {
            if (end_ms + milliseconds < 0) {
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
            eff->SetStartTimeMS(start_ms + milliseconds);
        }
        eff->SetEndTimeMS(end_ms + milliseconds);
    }
}

void xLightsFrame::ShiftSelectedEffectsOnLayer(EffectLayer* el, int milliseconds)
{
    if (milliseconds < 0) {
        std::list<int> toRemove;
        for (int ef = 0; ef < el->GetEffectCount(); ef++) {
            // move left
            Effect* eff = el->GetEffect(ef);
            if (eff->GetSelected()) {
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
                    } else {
                        // truncate start of effect
                        eff->SetStartTimeMS(0);
                        moved = true;
                    }
                } else {
                    auto effectsInTime = el->GetAllEffectsByTime(start_ms + milliseconds, end_ms + milliseconds);
                    bool clash = false;
                    for (const auto& it : effectsInTime) {
                        if (it->GetID() != eff->GetID()) {
                            clash = true;
                            break;
                        }
                    }
                    if (!clash) {
                        eff->SetStartTimeMS(start_ms + milliseconds);
                        moved = true;
                    }
                }
                if (moved) {
                    eff->SetEndTimeMS(end_ms + milliseconds);
                }
            }
        }
        for (auto it = toRemove.begin(); it != toRemove.end(); ++it) {
            el->RemoveEffect(*it);
        }
    } else {
        // Move right
        for (int ef = el->GetEffectCount() - 1; ef >= 0; ef--) { // count backwards so we can delete if needed
            Effect* eff = el->GetEffect(ef);
            if (eff->GetSelected()) {
                bool moved = false;
                int start_ms = eff->GetStartTimeMS();
                int end_ms = eff->GetEndTimeMS();
                auto effectsInTime = el->GetAllEffectsByTime(start_ms + milliseconds, end_ms + milliseconds);
                bool clash = false;
                for (const auto& it : effectsInTime) {
                    if (it->GetID() != eff->GetID()) {
                        clash = true;
                        break;
                    }
                }
                if (!clash) {
                    eff->SetStartTimeMS(start_ms + milliseconds);
                    moved = true;
                }
                if (moved) {
                    eff->SetEndTimeMS(end_ms + milliseconds);
                }
            }
        }
    }
}

// Former helpers `AddFileToZipFile`, `FixFile`, and `StripPresets`
// were removed when `PackageSequence` moved to
// `SequencePackage::Pack` in `src-core/render/`. The new packager
// walks SequenceMedia / ModelManager / ViewObjectManager directly,
// preserves show-relative paths, and rewrites external references
// on in-memory copies of rgbeffects + .xsq — no `_lost/` dumping
// and no on-disk temp files.

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
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written

    if (mSavedChangeCount != _sequenceElements.GetChangeCount() && showDialogs) {
        DisplayWarning("Your sequence has unsaved changes. These changes will not be packaged but any new referenced files will be. We suggest you consider saving and trying this again.", this);
    }

    wxFileName fn(CurrentSeqXmlFile->GetFullPath());
    wxString filename = fn.GetName() + ".xsqz";
    wxString filePath = fn.GetPath() + wxFileName::GetPathSeparator() + filename;

    if (showDialogs) {
        wxFileDialog fd(this, "Zip file to create.", CurrentDir, filename, "zip file(*.zip;*.xsqz)|*.xsqz;*.zip", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
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

    spdlog::debug("Packaging sequence into {}.", (const char*)filePath.c_str());

    wxProgressDialog prog("Package Sequence", "", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog.Show();

    // Collect the desktop-specific extras: house background image
    // and any data-layer `.iseq` sources the sequence references.
    // Pack() derives xlights_rgbeffects.xml and xlights_networks.xml
    // itself from showDir; the SequenceMedia + model / view-object
    // walks cover everything else.
    std::vector<std::string> extras;
    if (!mBackgroundImage.empty()) {
        extras.push_back(mBackgroundImage);
    }
    DataLayerSet& dataLayers = CurrentSeqXmlFile->GetDataLayers();
    for (int j = 0; j < dataLayers.GetNumLayers(); ++j) {
        DataLayer* dl = dataLayers.GetDataLayer(j);
        if (dl && dl->GetName() != "Nutcracker") {
            extras.push_back(dl->GetDataSource());
        }
    }

    // Alt audio tracks — gated inside Pack() by excludeAudio.
    std::vector<std::string> altAudio;
    for (int j = 0; j < CurrentSeqXmlFile->GetAltTrackCount(); ++j) {
        const auto& track = CurrentSeqXmlFile->GetAltTrack(j);
        if (!track.path.empty()) {
            altAudio.push_back(track.path);
        }
    }

    SequencePackOptions opts;
    opts.excludeAudio  = _excludeAudioFromPackagedSequences;
    opts.excludeVideos = _excludeVideosFromPackagedSequences;

    std::vector<std::string> packWarnings;
    bool ok = SequencePackage::Pack(
        std::filesystem::path(filePath.ToStdString()),
        CurrentDir.ToStdString(),
        CurrentSeqXmlFile->GetFullPath(),
        CurrentSeqXmlFile->GetMediaFile(),
        altAudio,
        extras,
        _sequenceElements.GetSequenceMedia(),
        AllModels,
        AllObjects,
        _sequenceElements,
        opts,
        &packWarnings,
        [&prog](int pct) -> bool {
            prog.Update(pct);
            return false;
        });

    prog.Update(100);

    if (!ok) {
        spdlog::warn("Error packaging sequence into {}.", (const char*)filePath.c_str());
        if (showDialogs) {
            wxString msg = "Failed to create sequence package. See the log for details.";
            if (!packWarnings.empty()) {
                msg += "\n\nIncomplete list of problems encountered:\n";
                int shown = 0;
                for (const auto& w : packWarnings) {
                    if (shown++ >= 8) { msg += wxString::Format("  … and %zu more", packWarnings.size() - 8); break; }
                    msg += "  • " + wxString(w) + "\n";
                }
            }
            DisplayWarning(msg, this);
        }
        return "";
    }

    // Per-file problems (missing referenced files, permission-denied
    // reads on files outside the sandbox's bookmark set, etc.) don't
    // abort the pack — Pack produces the best package it can and
    // returns the list of things it skipped. Surface that to the user
    // so they know the .xsqz they just created may be missing assets.
    if (showDialogs && !packWarnings.empty()) {
        wxString msg = wxString::Format(
            "The sequence was packaged but %zu file%s couldn't be included:\n\n",
            packWarnings.size(), packWarnings.size() == 1 ? "" : "s");
        int shown = 0;
        for (const auto& w : packWarnings) {
            if (shown++ >= 15) {
                msg += wxString::Format("\n  … and %zu more — see the log for the full list",
                                        packWarnings.size() - 15);
                break;
            }
            msg += "  • " + wxString(w) + "\n";
        }
        msg += "\nThe resulting package may be incomplete. Files outside the show "
               "folder and any configured media folders can't be packaged unless "
               "they're moved or copied into one of those locations first.";
        DisplayWarning(msg, this);
    }
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

bool xLightsFrame::IsInShowOrMediaFolder(const std::string& file) const
{
    if (IsInShowFolder(file)) return true;

    wxString f(file);
#ifdef __WXMSW__
    f.LowerCase();
    f.Replace("\\", "/");
#else
    f.Replace("\\", "/");
#endif

    for (const auto& dir : mediaDirectories) {
        wxString md(dir);
#ifdef __WXMSW__
        md.LowerCase();
#endif
        md.Replace("\\", "/");
        if (!md.EndsWith("/")) md += "/";
        if (f.StartsWith(md)) return true;
    }
    return false;
}

std::string xLightsFrame::MakeRelativePath(const std::string& file) const
{
    if (file.empty()) return {};
    wxFileName fn(file);
    if (!fn.IsAbsolute()) return {};   // already relative

    wxString f(file);
    f.Replace("\\", "/");

    // Try show directory first
    {
        wxString sd(GetShowDirectory());
        sd.Replace("\\", "/");
#ifdef __WXMSW__
        sd.MakeLower();
        wxString fl = f.Lower();
#else
        const wxString& fl = f;
#endif
        if (!sd.EndsWith("/")) sd += "/";
#ifdef __WXMSW__
        if (fl.StartsWith(sd))
            return f.Mid(sd.Length()).ToStdString();
#else
        if (fl.StartsWith(sd))
            return f.Mid(sd.Length()).ToStdString();
#endif
    }

    // Try each media directory
    for (const auto& dir : mediaDirectories) {
        wxString md(dir);
        md.Replace("\\", "/");
#ifdef __WXMSW__
        md.MakeLower();
        wxString fl = f.Lower();
#else
        const wxString& fl = f;
#endif
        if (!md.EndsWith("/")) md += "/";
        if (fl.StartsWith(md))
            return f.Mid(md.Length()).ToStdString();
    }

    return {};
}

#define FILES_MATCH_COMPARE 8192
bool xLightsFrame::FilesMatch(const std::string& file1, const std::string& file2) const
{
    // only equal if they both exist
    if (!FileExists(file1))
        return false;
    if (!FileExists(file2))
        return false;

    // and they are the same size
    wxFileName f1(file1);
    wxFileName f2(file2);
    if (f1.GetSize() != f2.GetSize())
        return false;

    // and the first 8K matches byte for byte ... we could check it all but this seems reasonable
    wxByte buf1[FILES_MATCH_COMPARE];
    wxByte buf2[FILES_MATCH_COMPARE];
    memset(buf1, 0x00, sizeof(buf1));
    memset(buf1, 0x00, sizeof(buf2));

    wxFile ff1;
    if (ff1.Open(file1)) {
        ff1.Read(buf1, sizeof(buf1));
    }

    wxFile ff2;
    if (ff2.Open(file2)) {
        ff2.Read(buf2, sizeof(buf2));
    }

    return (memcmp(buf1, buf2, sizeof(buf1)) == 0);
}

std::string xLightsFrame::MoveToShowFolder(const std::string& file, const std::string& subdirectory, const bool reuse)
{
    
    wxFileName fn(file);

    wxString target = GetShowDirectory();
    if (target.EndsWith("/") || target.EndsWith("\\")) {
        target = target.SubString(0, target.Length() - 2);
    }
    target += subdirectory;
    wxString dir = target;

    if (!wxDir::Exists(dir)) {
        wxFileName d;
        if (!d.Mkdir(dir)) {
            spdlog::error("Unable to create target folder {}.", (const char*)dir.c_str());
        }
    }

    if (target.EndsWith("/") || target.EndsWith("\\")) {
        target = target.SubString(0, target.Length() - 2);
    }

    target += wxFileName::GetPathSeparator();
    target += fn.GetFullName();

    int i = 1;
    while (FileExists(target) && !FilesMatch(file, target) && !reuse) {
        target = dir + wxFileName::GetPathSeparator() + fn.GetName() + "_" + wxString::Format("%d", i++) + "." + fn.GetExt();
    }

    if (!FileExists(target)) {
        spdlog::debug("Copying file {} to {}.", (const char*)file.c_str(), (const char*)target.c_str());
        wxCopyFile(file, target, false);
    } else if (reuse) {
        spdlog::debug("Reusing file {} for {}.", (const char*)target.c_str(), (const char*)file.c_str());
    }

    return target.ToStdString();
}

bool xLightsFrame::CleanupSequenceFileLocations()
{
    if (GetShowDirectory() == "") {
        wxMessageBox("Show directory invalid. Cleanup aborted.");
        return false;
    }

    wxString media = CurrentSeqXmlFile->GetMediaFile();
    if (FileExists(media) && !IsInShowFolder(media)) {
        CurrentSeqXmlFile->SetMediaFile(GetShowDirectory(), MoveToShowFolder(media, wxString(wxFileName::GetPathSeparator()) + "Audio"), false);
        _sequenceElements.IncrementChangeCount(nullptr);
    }

    bool changed = false;
    for (size_t j = 0; j < _sequenceElements.GetElementCount(0); j++) {
        Element* e = _sequenceElements.GetElement(j);
        changed = e->CleanupFileLocations(this, effectManager) || changed;

        if (dynamic_cast<ModelElement*>(e) != nullptr) {
            for (int s = 0; s < dynamic_cast<ModelElement*>(e)->GetSubModelAndStrandCount(); s++) {
                SubModelElement* se = dynamic_cast<ModelElement*>(e)->GetSubModel(s);
                changed = se->CleanupFileLocations(this, effectManager) || changed;
            }
            for (int s = 0; s < dynamic_cast<ModelElement*>(e)->GetStrandCount(); s++) {
                StrandElement* se = dynamic_cast<ModelElement*>(e)->GetStrand(s);
                changed = se->CleanupFileLocations(this, effectManager) || changed;
            }
        }
    }

    if (changed) {
        _sequenceElements.IncrementChangeCount(nullptr);
    }

    return true;
}

bool xLightsFrame::CleanupRGBEffectsFileLocations()
{
    if (FileExists(mBackgroundImage) && !IsInShowFolder(mBackgroundImage)) {
        wxString bi = MoveToShowFolder(mBackgroundImage, wxString(wxFileName::GetPathSeparator()));
        SetPreviewBackgroundImage(bi);
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
    }

    for (auto m : AllModels) {
        if (m.second->CleanupFileLocations(this)) {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
        }
    }

    for (auto m : AllObjects) {
        if (m.second->CleanupFileLocations(this)) {
            GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CleanupRGBEffectsFileLocations");
        }
    }

    return true;
}

void xLightsFrame::OnMenuItem_CleanupFileLocationsSelected(wxCommandEvent& event)
{
    
    spdlog::debug("Cleaning up file locations.");
    CleanupRGBEffectsFileLocations();
    if (CurrentSeqXmlFile != nullptr) {
        SetStatusText("Cleaning up file locations.");
        CleanupSequenceFileLocations();
        SetStatusText("Cleaning up file locations ... DONE.");
        wxBell();
    } else {
        wxMessageBox("You must have a sequence opened in order to run Cleanup File Locations.", "Missing Sequence", wxOK);
    }
    spdlog::debug("Cleaning up file locations ... DONE.");
}


#pragma endregion Tools Menu

void xLightsFrame::ValidateWindow()
{
    if (_outputManager.GetBaseShowDir() == "") {
        Button_UpdateBase->Disable();
        Button_ClearBaseShowDir->Disable();
        CheckBox_AutoUpdateBase->Disable();
        Button_OpenBaseShowDir->Disable();
    } else {
        if (_outputManager.GetBaseShowDir() == this->GetShowDirectory()) {
            Button_UpdateBase->Disable();
        } else {
            Button_UpdateBase->Enable();
        }
        Button_ClearBaseShowDir->Enable();
        CheckBox_AutoUpdateBase->Enable();
        Button_OpenBaseShowDir->Enable();

    }
}

void xLightsFrame::TimerOutput(int period)
{
    if (CheckBoxLightOutput->IsChecked()) {
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
    if (tb.IsOk()) {
        if (_showACLights) {
            tb.Show();
        } else {
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
    ::wxLaunchDefaultBrowser("https://www.paypal.com/donate/?hosted_button_id=BB6366BT755H6");
}

void xLightsFrame::OnMenuItem_DonateSelected(wxCommandEvent& event)
{
#ifdef __WXOSX__
    if (IsFromAppStore()) {
        DoInAppPurchases(this);
        return;
    }
#endif
    DoDonate();
}

#pragma endregion Help Menu

#pragma region AC Sequencing

void xLightsFrame::OnAC_OnClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACOn->SetValue(true);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
        Button_ACIntensity->SetValue(true);
    }
    Button_ACSelect->SetValue(false);

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_OffClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(true);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(false);
    Button_ACCascade->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACSelect->SetValue(false);

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::UpdateACToolbar(bool forceState)
{
    if (Button_ACDisabled->IsChecked() && _seqData.NumFrames() != 0 && _showACLights && forceState) {
        wxAuiToolBarItem* button = ACToolbar->FindTool(ID_AUITOOLBARITEM_ACON);
        int state = button->GetState();
        if (state & wxAUI_BUTTON_STATE_DISABLED) {
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
        if (Button_ACIntensity->IsChecked()) {
            ChoiceParm2->Enable(false);
        } else {
            ChoiceParm2->Enable(true);
        }
    } else {
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
    // MainAuiManager->Update();
    EnableSequenceControls(true);
}

#pragma endregion AC Sequencing

void xLightsFrame::OnACToolbarDropdown(wxAuiToolBarEvent& event)
{
}

void xLightsFrame::OnAC_ShimmerClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(false);
    Button_ACShimmer->SetValue(true);
    Button_ACSelect->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_TwinkleClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACOn->SetValue(false);
    Button_ACOff->SetValue(false);
    Button_ACTwinkle->SetValue(true);
    Button_ACShimmer->SetValue(false);
    Button_ACSelect->SetValue(false);
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
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
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    if (!Button_ACCascade->IsChecked()) {
        if (!Button_ACOn->IsChecked() && !Button_ACOff->IsChecked() && !Button_ACTwinkle->IsChecked() && !Button_ACShimmer->IsChecked()) {
            Button_ACSelect->SetValue(true);
        }
        if (!Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
            Button_ACIntensity->SetValue(true);
        }
    } else {
        Button_ACIntensity->SetValue(false);
        Button_ACRampUp->SetValue(false);
        Button_ACRampDown->SetValue(false);
        Button_ACRampUpDown->SetValue(false);
        Button_ACFill->SetValue(false);
        Button_ACForeground->SetValue(false);
        Button_ACBackground->SetValue(false);
    }

    if (Button_ACFill->IsChecked()) {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, false);
    } else {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, true);
    }

    if (Button_ACCascade->IsChecked() && wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_FillClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    if (Button_ACFill->IsChecked()) {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, false);
        Button_ACForeground->SetValue(false);
        Button_ACBackground->SetValue(false);
    } else {
        EnableToolbarButton(ACToolbar, ID_AUITOOLBARITEM_ACOFF, true);
    }

    Button_ACCascade->SetValue(false);

    if (!Button_ACOn->IsChecked() && !Button_ACTwinkle->IsChecked() && !Button_ACShimmer->IsChecked()) {
        if (Button_ACFill->IsChecked()) {
            Button_ACOn->SetValue(true);
        } else {
            if (!Button_ACOff->IsChecked()) {
                Button_ACSelect->SetValue(true);
            }
        }
    }

    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
        Button_ACIntensity->SetValue(true);
    }

    if (wasSelect && Button_ACFill->IsChecked()) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampUpDownClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(true);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked()) {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampUpDown));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampUpDown));

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampDownClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(true);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked()) {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampDown));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampDown));

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_RampUpClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(false);
    Button_ACRampUp->SetValue(true);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked()) {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(true);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1RampUp));
    ChoiceParm2->SetStringSelection(wxString::Format("%i", _acParm2RampUp));

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::OnAC_IntensityClick(wxCommandEvent& event)
{
    bool wasSelect = false;
    if (Button_ACSelect->IsChecked()) {
        wasSelect = true;
    }

    Button_ACIntensity->SetValue(true);
    Button_ACRampUp->SetValue(false);
    Button_ACRampDown->SetValue(false);
    Button_ACRampUpDown->SetValue(false);
    Button_ACFill->SetValue(false);
    Button_ACCascade->SetValue(false);
    if (Button_ACOff->IsChecked()) {
        Button_ACOff->SetValue(false);
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
    }
    ChoiceParm2->Enable(false);
    ChoiceParm1->SetStringSelection(wxString::Format("%i", _acParm1Intensity));

    if (wasSelect) {
        if (mainSequencer != nullptr && mainSequencer->PanelEffectGrid != nullptr) {
            mainSequencer->PanelEffectGrid->DoACDraw();
        }
    }
}

void xLightsFrame::GetACSettings(ACTYPE& type, ACSTYLE& style, ACTOOL& tool, ACMODE& mode)
{
    if (Button_ACSelect->IsChecked()) {
        type = ACTYPE::SELECT;
    } else if (Button_ACOn->IsChecked()) {
        type = ACTYPE::ON;
    } else if (Button_ACOff->IsChecked()) {
        type = ACTYPE::OFF;
    } else if (Button_ACTwinkle->IsChecked()) {
        type = ACTYPE::TWINKLE;
    } else if (Button_ACShimmer->IsChecked()) {
        type = ACTYPE::SHIMMER;
    }

    if (Button_ACIntensity->IsChecked()) {
        style = ACSTYLE::INTENSITY;
    } else if (Button_ACRampUp->IsChecked()) {
        style = ACSTYLE::RAMPUP;
    } else if (Button_ACRampDown->IsChecked()) {
        style = ACSTYLE::RAMPDOWN;
    } else if (Button_ACRampUpDown->IsChecked()) {
        style = ACSTYLE::RAMPUPDOWN;
    }

    tool = ACTOOL::TOOLNIL;
    if (Button_ACFill->IsChecked()) {
        tool = ACTOOL::FILL;
    } else if (Button_ACCascade->IsChecked()) {
        tool = ACTOOL::CASCADE;
    }

    mode = ACMODE::MODENIL;
    if (Button_ACForeground->IsChecked()) {
        mode = ACMODE::FOREGROUND;
    } else if (Button_ACBackground->IsChecked()) {
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
    if (Button_ACRampUp->IsChecked()) {
        a = std::min(_acParm1RampUp, _acParm2RampUp);
        b = std::max(_acParm1RampUp, _acParm2RampUp);
    } else if (Button_ACRampDown->IsChecked()) {
        a = std::max(_acParm1RampDown, _acParm2RampDown);
        b = std::min(_acParm1RampDown, _acParm2RampDown);
    } else if (Button_ACRampUpDown->IsChecked()) {
        a = _acParm1RampUpDown;
        b = _acParm2RampUpDown;
    }
}

void xLightsFrame::OnChoiceParm1Select(wxCommandEvent& event)
{
    if (Button_ACIntensity->IsChecked()) {
        _acParm1Intensity = wxAtoi(ChoiceParm1->GetStringSelection());
    } else if (Button_ACRampUp->IsChecked()) {
        _acParm1RampUp = wxAtoi(ChoiceParm1->GetStringSelection());
    } else if (Button_ACRampDown->IsChecked()) {
        _acParm1RampDown = wxAtoi(ChoiceParm1->GetStringSelection());
    } else if (Button_ACRampUpDown->IsChecked()) {
        _acParm1RampUpDown = wxAtoi(ChoiceParm1->GetStringSelection());
    }
}

void xLightsFrame::OnChoiceParm2Select(wxCommandEvent& event)
{
    if (Button_ACRampUp->IsChecked()) {
        _acParm2RampUp = wxAtoi(ChoiceParm2->GetStringSelection());
    } else if (Button_ACRampDown->IsChecked()) {
        _acParm2RampDown = wxAtoi(ChoiceParm2->GetStringSelection());
    } else if (Button_ACRampUpDown->IsChecked()) {
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
    if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
        Button_ACIntensity->SetValue(true);
    }
}

void xLightsFrame::SetACSettings(ACTOOL tool)
{
    wxCommandEvent event;
    switch (tool) {
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

    if (Button_ACSelect->IsChecked() || Button_ACOff->IsChecked()) {
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
        Button_ACOff->SetValue(false);
    }
    ACToolbar->Refresh();
}

void xLightsFrame::SetACSettings(ACSTYLE style)
{
    if (Button_ACSelect->IsChecked() || Button_ACOff->IsChecked()) {
        Button_ACOn->SetValue(true);
        Button_ACSelect->SetValue(false);
        Button_ACOff->SetValue(false);
    }

    wxCommandEvent event;
    switch (style) {
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
    switch (mode) {
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
    switch (type) {
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

    if (Button_ACOn->IsChecked() || Button_ACShimmer->IsChecked() || Button_ACTwinkle->IsChecked()) {
        if (!Button_ACIntensity->IsChecked() && !Button_ACRampUp->IsChecked() && !Button_ACRampDown->IsChecked() && !Button_ACRampUpDown->IsChecked()) {
            Button_ACIntensity->SetValue(true);
        }
    }
    ACToolbar->Refresh();
}

void xLightsFrame::OnMenuItem_PerspectiveAutosaveSelected(wxCommandEvent& event)
{
    _autoSavePerspecive = MenuItem_PerspectiveAutosave->IsChecked();
}

int xLightsFrame::SuppressDuplicateFrames() const
{
    return _outputManager.GetSuppressFrames();
}

void xLightsFrame::SetSuppressDuplicateFrames(int i)
{
    _outputManager.SetSuppressFrames(i);
    NetworkChange();
}

void xLightsFrame::SetPlayControlsOnPreview(bool b)
{
    _playControlsOnPreview = b;
    _housePreviewPanel->SetToolbar(_playControlsOnPreview);
    _housePreviewPanel->PostSizeEvent();
}

void xLightsFrame::SetShowBaseShowFolder(bool b)
{
    bool changed = _showBaseShowFolder != b;
    _showBaseShowFolder = b;
    Button_UpdateBase->Show(b);
    Button_ClearBaseShowDir->Show(b);
    StaticText_BaseShowDir->Show(b);
    CheckBox_AutoUpdateBase->Show(b);
    Button_ChangeBaseShowDir->Show(b);
    Button_OpenBaseShowDir->Show(b);
    StaticText_BaseShowDirLabel->Show(b);
    FlexGridSizer1->Layout();
    GridBagSizer1->Layout();
    FlexGridSizerSetup->Layout();
    Layout();
    if (!b) {
        _outputManager.SetBaseShowDir("");
        StaticText_BaseShowDir->SetLabel("No base show directory");
        CheckBox_AutoUpdateBase->SetValue(false);
        _outputManager.SetAutoUpdateFromBaseShowDir(false);
        if (changed)
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetShowBaseShowFolder");
    }
    ValidateWindow();
}

void xLightsFrame::SetAutoShowHousePreview(bool b)
{
    _autoShowHousePreview = b;
}

void xLightsFrame::SetZoomMethodToCursor(bool b)
{
    _zoomMethodToCursor = b;
}

void xLightsFrame::SetHidePresetPreview(bool b)
{
    _hidePresetPreview = b;
}

bool xLightsFrame::IsPaneDocked(wxWindow* window) const
{
    if (m_mgr == nullptr)
        return true;

    return m_mgr->GetPane(window).IsDocked();
}

std::vector<std::string> xLightsFrame::GetLayoutGroupNames() const {
    std::vector<std::string> names;
    names.reserve(LayoutGroups.size());
    for (const auto& [name, grp] : LayoutGroups) {
        names.push_back(name);
    }
    return names;
}

IModelPreview* xLightsFrame::GetHousePreview() const
{
    return _housePreviewPanel->GetModelPreview();
}

PreviewCamera* xLightsFrame::GetNamedCamera3D(const std::string& name)
{
    return viewpoint_mgr.GetNamedCamera3D(name);
}

void xLightsFrame::OnMenuItem_GenerateLyricsSelected(wxCommandEvent& event)
{
    GenerateLyricsDialog dlg(this, _seqData.NumChannels());

    if (dlg.ShowModal() == wxID_OK) {
        std::map<std::string, std::list<long>> face; // these channels need to be set to not zero when this phenome is displayed

        std::list<long> data = dlg.GetChannels("AI");
        if (data.size() > 0)
            face["AI"] = data;
        std::list<long> all = data; // this is all the channels used by one or more phenomes
        data = dlg.GetChannels("E");
        if (data.size() > 0)
            face["E"] = data;
        all.merge(data);
        data = dlg.GetChannels("etc");
        if (data.size() > 0)
            face["etc"] = data;
        all.merge(data);
        data = dlg.GetChannels("FV");
        if (data.size() > 0)
            face["FV"] = data;
        all.merge(data);
        data = dlg.GetChannels("L");
        if (data.size() > 0)
            face["L"] = data;
        all.merge(data);
        data = dlg.GetChannels("MBP");
        if (data.size() > 0)
            face["MBP"] = data;
        all.merge(data);
        data = dlg.GetChannels("O");
        if (data.size() > 0)
            face["O"] = data;
        all.merge(data);
        data = dlg.GetChannels("rest");
        if (data.size() > 0)
            face["rest"] = data;
        all.merge(data);
        data = dlg.GetChannels("U");
        if (data.size() > 0)
            face["U"] = data;
        all.merge(data);
        data = dlg.GetChannels("WQ");
        if (data.size() > 0)
            face["WQ"] = data;
        all.merge(data);
        all.unique();

        std::map<std::string, std::list<long>> notface; // this is the list of channels that must be zero for a given phenome
        for (auto it = face.begin(); it != face.end(); ++it) {
            std::list<long> notdata;

            for (auto it2 = all.begin(); it2 != all.end(); ++it2) {
                if (std::find(it->second.begin(), it->second.end(), *it2) == it->second.end()) {
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

        for (size_t i = 0; i < _seqData.NumFrames(); ++i) {
            bool phenomeFound = false;
            for (auto it = face.begin(); it != face.end(); ++it) {
                bool match = true;
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    if (_seqData[i][*it2 - 1] == 0) {
                        match = false;
                        break;
                    }
                }
                for (auto it2 = notface[it->first].begin(); it2 != notface[it->first].end(); ++it2) {
                    if (_seqData[i][*it2 - 1] != 0) {
                        match = false;
                        break;
                    }
                }

                if (match) {
                    if (lastEffect != nullptr && lastPhenome == it->first) {
                        lastEffect->SetEndTimeMS(lastEffect->GetEndTimeMS() + CurrentSeqXmlFile->GetFrameMS());
                    } else {
                        long start = i * CurrentSeqXmlFile->GetFrameMS();
                        lastEffect = tl->AddEffect(0, it->first, "", "", start, start + CurrentSeqXmlFile->GetFrameMS(), false, false);
                        lastPhenome = it->first;
                    }
                    phenomeFound = true;

                    break;
                }
            }

            if (!phenomeFound) {
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
    
    spdlog::critical("^^^^^ xLights crashing on purpose ... bye bye cruel world.");
    int* p = nullptr;
    *p = 0xFFFFFFFF;
}

void xLightsFrame::OnMenuItemBatchRenderSelected(wxCommandEvent& event)
{
    
    BatchRenderDialog dlg(this, &_outputManager);
    if (dlg.Prepare(this->GetShowDirectory()) && dlg.ShowModal() == wxID_OK && CloseSequence()) {
        wxArrayString files = dlg.GetFileList();
        wxArrayString filesToRender;
        for (auto f : files) {
            wxFileName fname(this->GetShowDirectory() + GetPathSeparator() + f);
            if (FileExists(fname))
                filesToRender.push_back(fname.GetFullPath());
            else
                spdlog::info("BatchRender: Sequence File not Found: {}.", (const char*)fname.GetFullPath().c_str());
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
            spdlog::info("BatchRender: No Sequences Selected.");
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

    bool found_update = false;
    // include 6 tags, first will LIKELY be the nightly, this then includes 5 to walk
    // back and find one that has the right asset for the platform
    std::string githubTagURL = "https://api.github.com/repos/xLightsSequencer/xLights/releases?per_page=6";
    MenuItem_Update->Enable(true);
    int rc = 0;
    spdlog::debug("Downloading {}", (const char*)githubTagURL.c_str());

    bool didConnect = false;
    std::string resp;
    nlohmann::json val;
    for (int retry = 0; retry < maxRetries && !didConnect; retry++) {
        resp = CurlManager::INSTANCE.doGet(githubTagURL, rc);
        if (rc == 200 && !resp.empty()) {
            try {
                val = nlohmann::json::parse(resp, nullptr, false);
                if (!val.is_discarded()) {
                    didConnect = true;
                }
            } catch (...) {
            }
        } else {
            wxSleep(1);
        }
    }
    if (!didConnect) {
        spdlog::debug("Version update check failed. Unable to connect.");
        if (showMessageBoxes) {
            wxMessageBox("Unable to connect.", "Version update check failed");
        }
        return true;
    }
    wxString configver;
    auto* config = GetXLightsConfig();
    if (canSkipUpdates && (config != nullptr)) {
        config->Read("SkipVersion", &configver);
    }

#ifdef LINUX
    const std::string ASSET_EXT = "AppImage";
#else
    const std::string ASSET_EXT = "exe";
#endif

    std::string downloadURL;
    std::string urlVersion;
    for (int x = 0; x < (int)val.size() && downloadURL.empty(); x++) {
        if (val[x].contains("name")) {
            std::string verName = val[x].contains("tag_name") ? val[x]["tag_name"].get<std::string>() : val[x]["name"].get<std::string>();
            if (verName != "nightly" && val[x].contains("assets")) {
                // not a nightly, so check if it has the needed asses
                for (int a = 0 ; a < (int)val[x]["assets"].size(); a++) {
                    std::string url = val[x]["assets"][a]["browser_download_url"].get<std::string>();
                    if (url.ends_with(ASSET_EXT)) {
                        downloadURL = url;
                        urlVersion = verName;
                    }
                }
            }
        }
    }

    spdlog::debug("Current Version: '{}'. Latest Available '{}'. Skip Version '{}'.",
                      (const char*)xlights_version_string.c_str(),
                      (const char*)urlVersion.c_str(),
                      (const char*)configver.c_str());
    if (!downloadURL.empty()) {
#ifndef SIMULATE_UPGRADE
        if ((urlVersion != configver) && (urlVersion != xlights_version_string) && IsVersionOlder(urlVersion, xlights_version_string))
#endif
        {
            found_update = true;
            UpdaterDialog* dialog = new UpdaterDialog(this);

            dialog->urlVersion = urlVersion;
            dialog->downloadUrl = downloadURL;
            dialog->StaticTextUpdateLabel->SetLabel("You are currently running xLights " + xlights_version_string + "\n" + "Whereas the current release is " + urlVersion);
            dialog->Show();
        }
    } else {
        spdlog::debug("Version update check failed. Unable to read available versions.");
        if (showMessageBoxes) {
            wxMessageBox("Unable to read available versions.", "Version update check failed");
        }
    }
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
        layoutPanel->SaveEffects();
        break;
    case LAYOUTTAB:
        layoutPanel->SaveEffects();
        SaveNetworksFile();
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
    CachedFileDownloader::GetDefaultCache().ClearCache();
    CachedFileDownloader::GetDefaultCache().Save();
}

bool xLightsFrame::GetRecycleTips() const
{
    auto* config = GetXLightsConfig();
    return config->Read("OnlyShowUnseenTips", true);
}

void xLightsFrame::SetRecycleTips(bool b)
{
    auto* config = GetXLightsConfig();
    config->Write("OnlyShowUnseenTips", b);
}

void xLightsFrame::OnMenuItem_PurgeVendorCacheSelected(wxCommandEvent& event)
{
    PurgeDownloadCache();
}

void xLightsFrame::OnMenuItem_LoudVolSelected(wxCommandEvent& event)
{
    playVolume = 100;
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_MedVolSelected(wxCommandEvent& event)
{
    playVolume = 66;
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_QuietVolSelected(wxCommandEvent& event)
{
    playVolume = 33;
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::OnMenuItem_VQuietVolSelected(wxCommandEvent& event)
{
    playVolume = 10;
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);
}

void xLightsFrame::ShowPresetsPanel()
{
    InitSequencer();
    if (EffectTreeDlg == nullptr)
        return;

    if (!_effectPresetsInitialized) {
        EffectTreeDlg->InitItems(_effectPresetManager);
        _effectPresetsInitialized = true;
    }
    m_mgr->GetPane("EffectPresets").Show();
    m_mgr->Update();
    UpdateViewMenu();
}

uint64_t xLightsFrame::BadDriveAccess(const std::list<std::string>& files, std::list<std::pair<std::string, uint64_t>>& slow, uint64_t thresholdUS)
{
    wxLogNull logNo; // suppress file access errors
    uint64_t worst = 0;

    std::list<std::string> folders;
    std::list<std::string> checkfiles;
    for (const auto& it : files) {
        wxFileName fn(it);
        if (std::find(begin(folders), end(folders), fn.GetPath()) == end(folders)) {
            folders.push_back(fn.GetPath());
            checkfiles.push_back(it);
        }
    }

    for (const auto& it : checkfiles) {
        uint8_t b[8192];
        wxStopWatch sw;
        wxFile f;
        if (f.Open(it)) {
            f.Read(b, sizeof(b));
            uint64_t t = sw.TimeInMicro().GetValue();
            if (t > worst)
                worst = t;
            if (t > thresholdUS) {
                slow.push_back({ it, t });
            }
        }
    }

    return worst;
}

void xLightsFrame::TogglePresetsPanel()
{
    InitSequencer();
    if (EffectTreeDlg == nullptr)
        return;

    if (!_effectPresetsInitialized) {
        EffectTreeDlg->InitItems(_effectPresetManager);
        _effectPresetsInitialized = true;
    }
    bool visible = m_mgr->GetPane("EffectPresets").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectPresets").Hide();
    } else {
        m_mgr->GetPane("EffectPresets").Show();
    }
    m_mgr->Update();
    UpdateViewMenu();
}

void xLightsFrame::ShowHideEffectPresetsWindow(wxCommandEvent& event)
{
    TogglePresetsPanel();
}

void xLightsFrame::OnMenuItemSelectEffectSelected(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("SelectEffect").IsShown();
    if (visible) {
        m_mgr->GetPane("SelectEffect").Hide();
    } else {
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

    if (BackupPurgeDays <= 0) {
        spdlog::debug("Backup purging skipped as it is disabled.");
        return;
    }

    spdlog::debug("Purging backups older than {} days.", BackupPurgeDays);

    time_t cur;
    time(&cur);
    wxDateTime curTime(cur);

    wxDateTime purgeDate = curTime.Add(wxTimeSpan(24 * BackupPurgeDays * -1));
    purgeDate.SetHour(0);
    purgeDate.SetMinute(0);
    purgeDate.SetSecond(0);
    purgeDate.SetMillisecond(0);

    spdlog::debug("    Keep backups on or after {}.", (const char*)purgeDate.FormatISODate().c_str());

    wxString backupDir = _backupDirectory + GetPathSeparator() + "Backup";

    int count = 0;
    int purged = 0;

    if (wxDir::Exists(backupDir)) {
        wxDir dir(backupDir);
        wxString filename;
        enum class BackUpStatus {
            Invalid,
            New,
            Old
        };
        auto OldEnoughtToDelete = [&purgeDate](wxString const& filename) {
            auto fdc = wxSplit(filename, '-');
            if (fdc.size() > 3) {
                int day = wxAtoi(fdc[2]);
                int month = wxAtoi(fdc[1]);
                int year = wxAtoi(fdc[0]);

                if (year < 2010 || month < 1 || month > 12 || day < 1 || day > 31) {
                    // date does not look valid
                    spdlog::debug("    Backup purge ignoring {}.", (const char*)filename.c_str());
                    return BackUpStatus::Invalid;
                } else {
                    wxDateTime bd(day, (wxDateTime::Month)(month - 1), year);
                    return (bd < purgeDate) ? BackUpStatus::Old : BackUpStatus::New;
                }
            }
            return BackUpStatus::Invalid;
        };

        // We dont follow symbolic links
        bool cont = dir.GetFirst(&filename, "", wxDIR_DIRS | wxDIR_NO_FOLLOW);
        while (cont) {
            wxString nextfilename;
            bool nextcont = dir.GetNext(&nextfilename);

            auto current = OldEnoughtToDelete(filename);
            auto next = OldEnoughtToDelete(nextfilename);

            if (current == BackUpStatus::Invalid) {
                // date does not look valid
                spdlog::debug("    Backup purge ignoring {}.", (const char*)filename.c_str());
            } else {
                count++;

                if (current == BackUpStatus::Old && next == BackUpStatus::Old) {
                    spdlog::debug("    Backup purge PURGING {}!", (const char*)filename.c_str());
                    if (!FileUtils::DeleteDirectory((backupDir + wxFileName::GetPathSeparator() + filename).ToStdString())) {
                        spdlog::debug("        FAILED!");
                    } else {
                        purged++;
                    }
                } else {
                    // spdlog::debug("    Backup purge keeping {}.", filename.ToStdString());
                }
            }

            filename = nextfilename;
            cont = nextcont;
        }
        spdlog::debug("Backup purge deleted {} of {} backups.", purged, count);
    } else {
        spdlog::debug("Backup purging skipped as {} does not exist.", backupDir.ToStdString());
    }
}

void xLightsFrame::SetBackupPurgeDays(int nbpi)
{
    if (nbpi != BackupPurgeDays) {
        auto* config = GetXLightsConfig();
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

    if (!wxDirExists(downloadDir)) {
        wxMkdir(downloadDir);
    }

    VendorMusicDialog dlg(this);
    if (dlg.DlgInit("", downloadDir)) {
        dlg.ShowModal();
    } else {
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

void xLightsFrame::LoadPhonemeDictionaries()
{
    std::vector<std::string> searchDirs = {
        CurrentDir.ToStdString(),
        (wxStandardPaths::Get().GetResourcesDir() + "/dictionaries").ToStdString(),
        wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().ToStdString()
    };

    wxProgressDialog dlg("Loading", "Loading phoneme dictionaries", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    dictionary.LoadDictionaries(searchDirs, [&dlg](int pct) { dlg.Update(pct); });
}

void xLightsFrame::OnMenuItemUserDictSelected(wxCommandEvent& event)
{
    SetCursor(wxCURSOR_WAIT);
    SetStatusText(_("Loading dictionaries ..."));
    LoadPhonemeDictionaries();
    SetStatusText(_(""));

    LyricUserDictDialog dlg(&dictionary, showDirectory, this);
    dlg.ShowModal();
    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::OnMenuItem_PurgeRenderCacheSelected(wxCommandEvent& event)
{
    _renderCache.Purge(&_sequenceElements, true);
}

void xLightsFrame::SetEnableRenderCache(const wxString& t)
{
    _enableRenderCache = t;
    if (_enableRenderCache == "Locked Effects Only") {
        _enableRenderCache = "Locked Only";
    }
    _renderCache.Enable(_enableRenderCache);
    _renderCache.CleanupCache(&_sequenceElements); // purge anything the cache no longer needs

    if (_renderCache.IsEnabled() && CurrentSeqXmlFile != nullptr) {
        // this will force a reload of the cache
        _renderCache.SetSequence(renderCacheDirectory, CurrentSeqXmlFile->GetName());
    } else {
        _renderCache.SetSequence("", "");
        _renderCache.Purge(&_sequenceElements, false);
    }
}

void xLightsFrame::SetRenderCacheMaximumSizeMB(size_t maxSizeMB)
{
    _renderCacheMaximumSizeMB = maxSizeMB;
    _renderCache.SetMaximumSizeMB(maxSizeMB);
}

bool xLightsFrame::HandleAllKeyBinding(wxKeyEvent& event)
{
    if (mainSequencer == nullptr)
        return false;

    auto k = event.GetKeyCode();
    if (k == WXK_SHIFT || k == WXK_CONTROL || k == WXK_ALT)
        return false;

    if ((!event.ControlDown() && !event.CmdDown() && !event.AltDown()) ||
        (k == 'A' && (event.ControlDown() || event.CmdDown()) && !event.AltDown())) {
        // Just a regular key ... If current focus is a control then we need to not process this
        if (dynamic_cast<wxControl*>(event.GetEventObject()) != nullptr &&
            (k < 128 || k == WXK_NUMPAD_END || k == WXK_NUMPAD_HOME || k == WXK_NUMPAD_INSERT || k == WXK_HOME || k == WXK_END || k == WXK_NUMPAD_SUBTRACT || k == WXK_NUMPAD_DECIMAL)) {
            return false;
        }
    }

    auto binding = mainSequencer->keyBindings.Find(event, KBSCOPE::All);
    if (binding != nullptr) {
        std::string type = binding->GetType();
        if (type == "RENDER_ALL") {
            RenderAll();
        } else if (type == "LIGHTS_TOGGLE") {
            ToggleOutputs();
            m_mgr->Update();
            PlayToolBar->Refresh();
        } else if (type == "OPEN_SEQUENCE") {
            if (readOnlyMode) {
                DisplayError("Sequences cannot be opened in read only mode!", this);
            } else
            OpenSequence("", nullptr);
        } else if (type == "CLOSE_SEQUENCE") {
            AskCloseSequence();
        } else if (type == "NEW_SEQUENCE") {
            if (readOnlyMode) {
                DisplayError("Sequences cannot be created in read only mode!", this);
            } else {
                NewSequence();
                EnableSequenceControls(true);
            }
        } else if (type == "PASTE_BY_CELL") {
            SetPasteByCell();
        } else if (type == "PASTE_BY_TIME") {
            SetPasteByTime();
        } else if (type == "SAVE_CURRENT_TAB") {
            SaveCurrentTab();
        } else if (type == "SEQUENCE_SETTINGS") {
            ShowSequenceSettings();
        } else if (type == "PLAY_LOOP") {
            wxCommandEvent playEvent(EVT_SEQUENCE_REPLAY_SECTION);
            wxPostEvent(this, playEvent);
        } else if (type == "PLAY") {
            wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
            wxPostEvent(this, playEvent);
        } else if (type == "TOGGLE_PLAY") {
            wxCommandEvent playEvent(EVT_TOGGLE_PLAY);
            wxPostEvent(this, playEvent);
        } else if (type == "FOCUS_SEQUENCER") {
            if (Notebook1->GetSelection() == NEWSEQUENCER) {
                SetFocus();
            }
        } else if (type == "START_OF_SONG") {
            wxCommandEvent playEvent(EVT_SEQUENCE_FIRST_FRAME);
            wxPostEvent(this, playEvent);
        } else if (type == "END_OF_SONG") {
            wxCommandEvent playEvent(EVT_SEQUENCE_LAST_FRAME);
            wxPostEvent(this, playEvent);
        } else if (type == "STOP") {
            wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
            wxPostEvent(this, playEvent);
        } else if (type == "PAUSE") {
            wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
            wxPostEvent(this, playEvent);
        } else if (type == "BACKUP") {
            wxCommandEvent e;
            OnMenuItemBackupSelected(e);
        } else if (type == "ALTERNATE_BACKUP") {
            wxCommandEvent e;
            OnmAltBackupMenuItemSelected(e);
        } else if (type == "SELECT_SHOW_FOLDER") {
            if (readOnlyMode) {
                DisplayError("Show folder cannot be changed in read only mode! Close and restart xLights.", this);
            } else {
                wxCommandEvent e;
                OnMenuOpenFolderSelected(e);
            }
        } else if (type == "FPP_CONNECT") {
            wxCommandEvent e;
            OnButtonFPPConnectClick(e);
        } else {
            return false;
        }
        event.StopPropagation();
        return true;
    }

    return false;
}

void xLightsFrame::OnMenuItem_ShowKeyBindingsSelected(wxCommandEvent& event)
{
    // The key bindings dump is long — a plain wxMessageBox (NSAlert on macOS)
    // doesn't scroll and clips the bottom off on short displays (#5855).
    // Use a resizable dialog with a scrollable monospace text ctrl instead.
    wxDialog dlg(this, wxID_ANY, "Key Bindings", wxDefaultPosition,
                 wxSize(700, 600),
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl* text = new wxTextCtrl(&dlg, wxID_ANY,
                                      mainSequencer->keyBindings.Dump(),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxTE_MULTILINE | wxTE_READONLY |
                                      wxTE_DONTWRAP | wxBORDER_SUNKEN);
    text->SetFont(wxFont(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE)));
    text->SetInsertionPoint(0);
    sizer->Add(text, 1, wxALL | wxEXPAND, 8);
    wxButton* closeBtn = new wxButton(&dlg, wxID_OK, "Close");
    sizer->Add(closeBtn, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    dlg.SetSizer(sizer);
    dlg.SetMinSize(wxSize(400, 300));
    closeBtn->SetDefault();
    dlg.ShowModal();
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
        if (!layoutPanel->HandleLayoutKeyBinding(event)) {
            event.Skip();
        }
        return;
    case NEWSEQUENCER:
        if (!mainSequencer->HandleSequencerKeyBinding(event)) {
            event.Skip();
        }
        return;
    default:
        break;
    }

    if (!HandleAllKeyBinding(event)) {
        event.Skip();
    }
}

void xLightsFrame::OnMenuItem_ZoomSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser("https://zoom.us/j/175801909?pwd=ZU1hNzM5bjJpOGZ1d1BOb1BzMUFndz09");
}

void xLightsFrame::OnMenuItem_Generate2DPathSelected(wxCommandEvent& event)
{
    PathGenerationDialog dlg(this, CurrentDir.ToStdString());
    dlg.ShowModal();
}

void xLightsFrame::OnMenuItem_PrepareAudioSelected(wxCommandEvent& event)
{

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
                                       "Reaper or xAudio files (*.rpp;*.xaudio)|*.rpp;*.xaudio|All files (*.*)|*.*",
                                       wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (filename != "") {
        spdlog::debug("Prepare audio: {}.", (const char*)filename.c_str());

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
                
                spdlog::debug("        Source file: {} Source Pos: {:.3f} Length: {:.3f} Target Pos: {:.3f} Fade In: {:.3f} Fade Out: {:.3f} Volume: {:.3f}",
                                  file, sourceoffset, length, start, fadein, fadeout, volume);
            }
        };

        std::list<musicEdit> edits;
        wxFileName targetFile(CurrentDir);
        AudioManager *firstAudio = nullptr;
        if (filename.Lower().EndsWith(".rpp")) {
            wxFile reaper;
            if (reaper.Open(filename)) {
                wxString reaperContent;
                reaper.ReadAll(&reaperContent);

                targetFile = wxFileName(filename);

                wxRegEx regexTgt("RENDER_FILE \\\"[^\\\"]*?\\/([^\\\"\\/]*)\\\"", wxRE_ADVANCED | wxRE_NEWLINE);
                if (regexTgt.Matches(reaperContent)) {
                    targetFile.SetFullName(regexTgt.GetMatch(reaperContent, 1));
                    spdlog::debug("    Target file: {}", (const char*)targetFile.GetFullPath().c_str());
                } else {
                    targetFile.SetExt("m4a");
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

            pugi::xml_document doc;
            doc.load_file(filename.mb_str());

            if (doc.document_element()) {
                pugi::xml_node r = doc.document_element();
                for (pugi::xml_node n = r.first_child(); n; n = n.next_sibling()) {
                    std::string name = n.name();
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    if (name == "targetfile") {
                        targetFile.SetFullName(n.text().get());
                    } else if (name == "items") {
                        for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                            name = nn.name();
                            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
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

                                for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                                    name = nnn.name();
                                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                                    if (name == "file") {
                                        sourcefile = nnn.text().get();
                                    } else if (name == "targettime") {
                                        start = std::atof(nnn.text().get());
                                    } else if (name == "length") {
                                        length = std::atof(nnn.text().get());
                                    } else if (name == "sourcetime") {
                                        sourceoffset = std::atof(nnn.text().get());
                                    } else if (name == "fadeinsecs") {
                                        fadein = std::atof(nnn.text().get());
                                    } else if (name == "fadeoutsecs") {
                                        fadeout = std::atof(nnn.text().get());
                                    } else if (name == "fadeincrossfade") {
                                        fadeincrossfade = true;
                                    } else if (name == "fadeoutcrossfade") {
                                        fadeoutcrossfade = true;
                                    } else if (name == "gain") {
                                        volume = std::atof(nnn.text().get());
                                    }
                                }
                                edits.push_back(musicEdit(sourcefile, start, length, sourceoffset, fadein, fadeout, volume, fadeincrossfade, fadeoutcrossfade));
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
                wxFileName fn = targetFile;
                fn.SetFullName(it.file);
                if (!fn.Exists()) {
                    wxString music = wxFileSelector("Choose your copy of " + it.file + ".",
                                                    fn.GetPath(), wxEmptyString, wxEmptyString,
                                                    "Audio files|*.mp3;*.ogg;*.m4p;*.mp4;*.avi;*.wma;*.wmv;*.au;*.wav;*.m4a;*.mid;*.mkv;*.mov;*.mpg;*.asf;*.flv;*.mpeg",
                                                    wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);
                    if (music != "") {
                        sourceSongs[it.file] = new AudioManager(music);
                        // wait for song to load
                        while (!sourceSongs[it.file]->IsDataLoaded()) {
                            wxMilliSleep(100);
                        }
                    } else {
                        sourceSongs[it.file] = nullptr;
                    }
                } else {
                    sourceSongs[it.file] = new AudioManager(fn.GetFullPath());
                }
                if (firstAudio == nullptr) {
                    firstAudio = sourceSongs[it.file];
                }
            }
        }

        bool ok = true;

        long sampleRate = -1;
        for (const auto& it : sourceSongs) {
            if (it.second != nullptr) {
                if (sampleRate == -1) {
                    sampleRate = it.second->GetRate();
                } else {
                    if (ok && sampleRate != it.second->GetRate()) {
                        spdlog::debug("Songs do not all have the same bitrate ... unable to do the required mixing.");
                        wxMessageBox("In order to prepare the audio all the input songs must have the same sample rate.");
                        ok = false;
                    }
                }
            }
        }

        if (sampleRate == -1) {
            SetStatusText("Audio file creation failed - No input audio.");
            ok = false;
        }

        if (ok) {
            long totalSamples = sampleRate * outputLength;
            spdlog::debug("    New file will:");
            spdlog::debug("        have {} samples.", totalSamples);
            spdlog::debug("        be {:.3f} seconds long.", outputLength);
            std::vector<float> left(totalSamples);
            std::vector<float> right(totalSamples);

            for (const auto& it : edits) {
                auto audio = sourceSongs[it.file];
                if (audio != nullptr) {
                    // ensure that the audio has been processed by setting a frame interval if its unset
                    if (audio->GetFrameInterval() < 0) {
                        spdlog::debug("Setting default frame interval for {}.", it.file);
                        audio->SetFrameInterval(20);
                    }
                    // check the data is actually loaded
                    audio->GetRawLeftData(audio->GetTrackSize() - 1);

                    SetStatusText("Combining audio clips.");

                    spdlog::debug("Processing sample from {}.", it.file);
                    long startOutput = sampleRate * it.start;
                    long outputSamples = sampleRate * it.length;
                    // spdlog::debug("    Sample Output Start {}-{} [{}].", startOutput, startOutput + outputSamples - 1, outputSamples);
                    wxASSERT(startOutput + outputSamples - 1 <= totalSamples);
                    long startSample = audio->GetRate() * it.sourceoffset;
                    long inputSamples = audio->GetRate() * it.length;
                    // spdlog::debug("    Input file samples {}", audio->GetTrackSize());
                    // spdlog::debug("    Sample Input Start {}-{} [{}].", startSample, startSample + inputSamples - 1, inputSamples);
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
                        if (fadeinsamples > 0 && i < fadeinsamples) {
                            double f = 0.0;
                            double progress = (double)i / fadeinsamples;
                            if (it.crossfadein) {
                                f = std::log10(progress * 9.0 + 1.0);
                            } else {
                                f = exp(-it.fadein * (double)(fadeinsamples - i) / fadeinsamples);
                            }
                            f = std::clamp(f, 0.0, 1.0);
                            l *= f;
                            r *= f;
                        }
                        if (fadeoutsamples > 0 && i > fadeoutstart) {
                            double progress = (double)(inputSamples - i) / fadeoutsamples;
                            double f = 0.0;
                            if (it.crossfadeout) {
                                f = std::log10(progress * 9.0 + 1.0);
                            } else {
                                f = exp(-it.fadeout * (double)(i - fadeoutstart) / fadeoutsamples);
                            }
                            f = std::clamp(f, 0.0, 1.0);
                            l *= f;
                            r *= f;
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

#ifdef __WXOSX__
            // Cannot generate MP3's, change to AAC/m4a (which has better quality anyway)
            wxFileName fn = targetFile;
            if (fn.GetExt() == "mp3") {
                fn.SetExt("m4a");
                targetFile = fn.GetFullPath();
            }
#endif

            if (FileExists(targetFile)) {
                if (wxMessageBox(targetFile.GetFullPath() + " already exists. Do you want to overwrite it?", "Replace", wxYES_NO | wxCENTRE, this) == wxNO) {
                    wxFileDialog fd(this,
                                    "Choose filename to save the audio as.",
                                    targetFile.GetPath(),
                                    targetFile.GetName(),
#ifdef __WXOSX__
                                    "Audio Files|*.m4a",
#else
                                    "Audio Files|*.mp3;*.m4a",
#endif
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

            if (!AudioManager::EncodeAudio(left,
                               right,
                               sampleRate,
                               targetFile.GetFullPath(),
                               firstAudio)) {
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

void xLightsFrame::SetCrosshairSize(int i) {
    _crosshairSize = i;
    layoutPanel->Refresh();
}

void xLightsFrame::SetSuppressFadeHints(bool b)
{
    mSuppressFadeHints = b;
    mainSequencer->PanelEffectGrid->Refresh();
}

void xLightsFrame::SetDisableKeyAcceleration(bool b) {
    _disableKeyAcceleration = b;
}

void xLightsFrame::SetSuppressColorWarn(bool b)
{
    mSuppressColorWarn = b;
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
    auto* config = GetXLightsConfig();
    config->Write("xLightsRandomEffects", randomEffects);
    config->Flush();
}

void xLightsFrame::SetUserEMAIL(const wxString& e)
{
    _userEmail = e;
    auto* config = GetXLightsConfig();
    config->Write("xLightsUserEmail", _userEmail);
    config->Flush();
    spdlog::info("User email changed to {}", _userEmail.ToStdString());
}

void xLightsFrame::SetRenameModelAliasPromptBehavior(const wxString& e)
{
    
    _aliasRenameBehavior = e;
    auto* config = GetXLightsConfig();
    config->Write("xLightsModelRename", _aliasRenameBehavior);
    config->Flush();
    spdlog::info("Rename Alias Prompt Behavior set to {}", _aliasRenameBehavior.ToStdString());
}

void xLightsFrame::CollectUserEmail()
{
    if (_userEmail == "")
        _userEmail = "noone@nowhere.xlights.org";
    EmailDialog dlg(this, _userEmail);
    if (dlg.ShowModal() == wxID_OK) {
        SetUserEMAIL(dlg.GetEmail());
    } else {
        // if the user cancels assume they want it set to the prior address
        SetUserEMAIL(_userEmail);
    }
}

void xLightsFrame::SetDefaultSeqView(const wxString& view)
{
    // if (_sequenceViewManager.GetViewIndex(view) == -1) {
    //     return;
    // }

    _defaultSeqView = view;
    SetXmlSetting("defaultSeqView", view);
    UnsavedRgbEffectsChanges = true;
    UpdateLayoutSave();
    UpdateControllerSave();
}

std::vector<std::string> xLightsFrame::GetSequenceViews()
{
    return _sequenceViewManager.GetViewList();
}

void xLightsFrame::SetMinTipLevel(const wxString& level)
{
    
    auto* config = GetXLightsConfig();
    config->Write("MinTipLevel", level);
    config->Flush();
    spdlog::info("Minimum tip level set to {}", (const char*)level.c_str());
}

std::string xLightsFrame::GetMinTipLevel() const
{
    auto* config = GetXLightsConfig();
    return config->Read("MinTipLevel", "Beginner");
}

void xLightsFrame::SetVideoExportCodec(const wxString& codec)
{
    _videoExportCodec = codec;
    auto* config = GetXLightsConfig();
    config->Write("xLightsVideoExportCodec", _videoExportCodec);
    config->Flush();
    spdlog::info("Video Export Codec set to {}", (const char*)_videoExportCodec.c_str());
}

void xLightsFrame::SetVideoExportBitrate(int bitrate)
{
    
    _videoExportBitrate = bitrate;
    auto* config = GetXLightsConfig();
    config->Write("xLightsVideoExportBitrate", _videoExportBitrate);
    config->Flush();
    spdlog::info("Video Export Bitrate set to {}", _videoExportBitrate);
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

void xLightsFrame::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    event.Skip();
    color_mgr.SysColorChanged();
}

void xLightsFrame::SetHardwareVideoAccelerated(bool b)
{
    _hwVideoAccleration = b;
    VideoReader::SetHardwareAcceleratedVideo(_hwVideoAccleration);
    auto* config = GetXLightsConfig();
    config->Write("xLightsVideoReaderAccelerated", VideoReader::IsHardwareAcceleratedVideo());
}

void xLightsFrame::SetHardwareVideoRenderer(int type) {
    _hwVideoRenderer = type;
    VideoReader::SetHardwareRenderType(_hwVideoRenderer);
    auto* config = GetXLightsConfig();
    config->Write("xLightsVideoReaderRenderer", VideoReader::GetHardwareRenderType());
}

bool xLightsFrame::ShadersOnBackgroundThreads() const
{
    return ShaderEffect::IsBackgroundRender();
}
void xLightsFrame::SetShadersOnBackgroundThreads(bool b)
{
    ShaderEffect::SetBackgroundRender(b);
    auto* config = GetXLightsConfig();
    config->Write("xLightsShadersOnBackgroundThreads", b);
}

bool xLightsFrame::UseGPURendering() const
{
    return GPURenderUtils::IsEnabled();
}
void xLightsFrame::SetUseGPURendering(bool b)
{
    GPURenderUtils::SetEnabled(b);
    auto* config = GetXLightsConfig();
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
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
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

    lxw_format* sr1_format = workbook_add_format(workbook);
    format_set_bg_color(sr1_format, 0x99FF99);
    format_set_border(sr1_format, LXW_BORDER_THIN);

    lxw_format* sr2_format = workbook_add_format(workbook);
    format_set_bg_color(sr2_format, 0xB896FF);
    format_set_border(sr2_format, LXW_BORDER_THIN);

    lxw_format* sr3_format = workbook_add_format(workbook);
    format_set_bg_color(sr3_format, 0xFFC996);
    format_set_border(sr3_format, LXW_BORDER_THIN);

    lxw_format* sr4_format = workbook_add_format(workbook);
    format_set_bg_color(sr4_format, 0x80FFFF);
    format_set_border(sr4_format, LXW_BORDER_THIN);

    lxw_format* sr5_format = workbook_add_format(workbook);
    format_set_bg_color(sr5_format, 0xFF85FF);
    format_set_border(sr5_format, LXW_BORDER_THIN);

    lxw_format* sr6_format = workbook_add_format(workbook);
    format_set_bg_color(sr6_format, 0xFFFF85);
    format_set_border(sr6_format, LXW_BORDER_THIN);

    int row = 0;
    std::map<int, double> _col_widths;

    for (const auto& it : controllers) {
        UDController cud(it, &_outputManager, &AllModels, false);
        int columSize = 0;
        std::vector<std::vector<std::string>> const lines = cud.ExportAsCSV(exportsettings, it->GetDefaultBrightnessUnderFullControl(), columSize);

        worksheet_merge_range(worksheet, row, 0, row, columSize, it->GetShortDescription().c_str(), header_format);
        ++row;
        auto lformat = first_format;

        for (const auto& line : lines) {
            for (int i = 1; i <= columSize; ++i) {
                worksheet_write_blank(worksheet, row, i, lformat);
            }
            int col = 0;
            for (auto const& column : line) {
                if (column.empty()) {
                    continue;
                }
                if (column.find("Remote A") != std::string::npos ||
                    column.find("Remote G") != std::string::npos ||
                    column.find("Remote M") != std::string::npos) {
                    lformat = sr1_format;
                }
                if (column.find("Remote B") != std::string::npos ||
                    column.find("Remote H") != std::string::npos ||
                    column.find("Remote N") != std::string::npos) {
                    lformat = sr2_format;
                }
                if (column.find("Remote C") != std::string::npos ||
                    column.find("Remote I") != std::string::npos ||
                    column.find("Remote O") != std::string::npos) {
                    lformat = sr3_format;
                }
                if (column.find("Remote D") != std::string::npos ||
                    column.find("Remote J") != std::string::npos ||
                    column.find("Remote P") != std::string::npos) {
                    lformat = sr4_format;
                }
                if (column.find("Remote E") != std::string::npos ||
                    column.find("Remote K") != std::string::npos) {
                    lformat = sr5_format;
                }
                if (column.find("Remote F") != std::string::npos ||
                    column.find("Remote L") != std::string::npos) {
                    lformat = sr6_format;
                }
                worksheet_write_string(worksheet, row, col, column.c_str(), lformat);
                double width = column.size() + 1.3; // estimate column width
                if (_col_widths[col] < width) {
                    _col_widths[col] = width;
                    worksheet_set_column(worksheet, col, col, width, NULL);
                }
                ++col;
            }
            ++row;
            lformat = format;
        }
        row += 2;
    }
    lxw_error error = workbook_close(workbook);
    if (error) {
        DisplayError(wxString::Format("Unable to create Spreadsheet, Error %d = %s\n", error, lxw_strerror(error)).ToStdString());
        return;
    }

    SetStatusText("Controller Connections spreadsheet saved to " + filename);
}


std::string xLightsFrame::GetUniqueTimingName(const std::string& baseName)
{
    if (CurrentSeqXmlFile == nullptr)
        return baseName;
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
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
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
        { "EffectPresets", MenuItemEffectPresets },
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
    AudioManager::GetAudioManager()->SetGlobalVolume(playVolume);
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
    if (restore.ShowModal() == wxID_OK) {
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
            bool success = wxCopyFile(restoreFolder + GetPathSeparator() + file,
                                      showDirectory + GetPathSeparator() + file);
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
    
    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        spdlog::error("Null config ... this wont end well.");
        return;
    }
    bool bv;

    config->Read("xLights_SD_HP", &bv, false);
    spdlog::debug("Suppress Dock HousePreview: {}.", toStr(bv));
    MenuItem_SD_HP->Check(!bv);

    config->Read("xLights_SD_MP", &bv, false);
    spdlog::debug("Suppress Dock ModelPreview: {}.", toStr(bv));
    MenuItem_SD_MP->Check(!bv);
}

void xLightsFrame::SaveDockable()
{
    
    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        spdlog::error("Null config ... this wont end well.");
        return;
    }
    config->Write("xLights_SD_HP", IsDockable("HP"));
    config->Write("xLights_SD_MP", IsDockable("MP"));
}

bool xLightsFrame::IsDockable(const std::string& panel)
{
    if (panel == "HP") {
        return !MenuItem_SD_HP->IsChecked();
    } else if (panel == "MP") {
        return !MenuItem_SD_MP->IsChecked();
    }

    return true;
}

void xLightsFrame::SetBaseShowDir(const wxString& baseShowDir)
{
    _outputManager.SetBaseShowDir(ToStdString(baseShowDir));
    if (baseShowDir == "") {
        StaticText_BaseShowDir->SetLabel("No base show directory");
    } else {
        StaticText_BaseShowDir->SetLabel(baseShowDir);
    }
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "MoveSelectedControllerRows");
}

void xLightsFrame::OnButton_ChangeBaseShowDirClick(wxCommandEvent& event)
{
    // set the base show directory and save it in the networks file
    wxDirDialog DirDialog1(this, _("Select Base Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    if (DirDialog1.ShowModal() == wxID_OK) {
        SetBaseShowDir(DirDialog1.GetPath());
        ValidateWindow();
    }
}

void xLightsFrame::OnButton_ClearBaseShowDirClick(wxCommandEvent& event)
{
    // erase the base show directory and save it in the networks file
    SetBaseShowDir("");
    ValidateWindow();
}

void xLightsFrame::OnCheckBox_AutoUpdateBaseClick(wxCommandEvent& event)
{
    // set/reset the auto flag and save it in the networks file
    _outputManager.SetAutoUpdateFromBaseShowDir(CheckBox_AutoUpdateBase->IsChecked());
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnCheckBox_AutoUpdateBaseClick");
}

void xLightsFrame::OnButton_UpdateBaseClick(wxCommandEvent& event)
{
    // execute the update now
    SetCursor(wxCURSOR_WAIT);
    waitForPingsToComplete();
    UpdateFromBaseShowFolder(true);
    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::UpdateFromBaseShowFolder(bool prompt)
{
    
    spdlog::debug("Updating from base show folder.");
    
    ObtainAccessToURL(_outputManager.GetBaseShowDir());
    if (!ObtainAccessToURL(_outputManager.GetBaseShowDir() + GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE)) {
        std::string dstr = _outputManager.GetBaseShowDir() ;
        PromptForDirectorySelection("Reselect Base Show Directory", dstr);
        ObtainAccessToURL(_outputManager.GetBaseShowDir());
    }

    // Shared accept-all / reject-all state threaded across all three merge passes so that
    // "Yes to All" or "No to All" chosen during controller prompts also suppresses the
    // subsequent model and object prompts (and vice-versa).
    bool mergeAcceptAll = false;
    bool mergeRejectAll = false;

    // bring in any controllers overwriting some of their properties ... but not all of them
    if (_outputManager.MergeFromBase(prompt, mergeAcceptAll, mergeRejectAll, this)) {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateFromBaseShowFolder-controller");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateFromBaseShowFolder-controller");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "UpdateFromBaseShowFolder-controller");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateFromBaseShowFolder-controller");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "UpdateFromBaseShowFolder-controller");
    }

    // bring in any models ... overwriting any with the same name
    // bring in any model groups ... again overwriting any ... the models in the group should be a merge and deduplication
    if (AllModels.MergeFromBase(_outputManager.GetBaseShowDir(), prompt, mergeAcceptAll, mergeRejectAll)) {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateFromBaseShowFolder-model");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "UpdateFromBaseShowFolder-model");
    }

    if (AllObjects.MergeFromBase(_outputManager.GetBaseShowDir(), prompt, mergeAcceptAll, mergeRejectAll))
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateFromBaseShowFolder-object");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "UpdateFromBaseShowFolder-object");
    }

    spdlog::debug("Base show folder update done.");

    // other things we could bring in
    // - Test presets
    // - preset effects
    // - perspectives
    // - key bindings

    DoAllWork();
}

void xLightsFrame::UpdateReadOnlyState()
{
    // disable a bunch of things
    ButtonSaveSetup->Enable(!readOnlyMode);
    layoutPanel->ButtonSavePreview->Enable(!readOnlyMode);
    Button_ChangeBaseShowDir->Enable(!readOnlyMode);
    Button_ChangeTemporarilyAgain->Enable(!readOnlyMode);
    Button_CheckShowFolderTemporarily->Enable(!readOnlyMode);
    Button_ChangeShowDirPermanently->Enable(!readOnlyMode);
}

void xLightsFrame::OnMenuItemFindShowFolderSelected(wxCommandEvent& event)
{
    ShowFolderSearchDialog dlg(this);
    dlg.ShowModal();
}

std::list<std::string> xLightsFrame::GetPerspectives() {
    std::list<std::string> perspectives;
    for (const auto& p : _perspectives) {
        if (!p.name.empty())
            perspectives.push_back(p.name);
    }
    return perspectives;
}

aiBase* xLightsFrame::GetAIService(aiType::TYPE serviceType) {
    return _serviceManager->findService(serviceType);
}
std::vector<aiBase*> xLightsFrame::GetAIServices(aiType::TYPE serviceType) {
    return _serviceManager->findServices(serviceType);
}

void xLightsFrame::OnMenuItem_GenerateAIImageSelected(wxCommandEvent& event) {
    auto services = _serviceManager->findServices(aiType::TYPE::IMAGES);
    if (services.empty()) {
        wxMessageBox("No AI Services Registered for creating images", "Error", wxICON_ERROR);
        return;
    }
    auto serv = services[0];
    if (services.size() > 1) {
        wxArrayString choices;
        for (auto s : services) {
            choices.push_back(s->GetLLMName());
        }
        wxSingleChoiceDialog dlg(this, "AI Image Generator", "Choose AI Image Generator", choices, nullptr);
        if (dlg.ShowModal() != wxID_CANCEL) {
            serv = services[dlg.GetSelection()];
        }
    }
    AIImageDialog dlg(this, serv);
    dlg.ShowModal();
}

void xLightsFrame::SetPaletteSizeString(const wxString& size) {
    if (GetPaletteSizeString() != size) {
        auto* config = GetXLightsConfig();
        config->Write("PaletteSize", size);

        if (colorPanel) {
            colorPanel->RefreshPaletteSize();
        }
    }
}
