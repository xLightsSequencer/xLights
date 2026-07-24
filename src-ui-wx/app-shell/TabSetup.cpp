
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef LINUX
#include <arpa/inet.h>
#endif

#include <wx/time.h>
#include <wx/stopwatch.h>
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include "settings/XLightsConfigAdapter.h"
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/thread.h>

#include <thread>
#include <chrono>
#include <ctime>

#include "xLightsMain.h"
#include "xLightsApp.h"
#include "layout/ControllerListPanel.h"
#include "layout/LayoutPanel.h"
#include "render/SequenceFile.h"
#ifdef __WXOSX__
#include "shared/utils/xlPropertyGrid.h"
#endif
#include "sequencer/MainSequencer.h"
#include "layout/ViewsModelsPanel.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "models/Model.h"
#include "utils/SpecialOptions.h"
#include "layout/LayoutGroup.h"
#include "setup/ControllerModelDialog.h"
#include "setup/ShowDirectoriesDialog.h"
#include "utils/ExternalHooks.h"
#include "utils/ip_utils.h"

#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/Minleon.h"
#include "controllers/Pixlite16.h"
#include "controllers/SanDevices.h"
#include "controllers/J1Sys.h"
#include "controllers/ESPixelStick.h"
#include "controllers/HinksPix.h"
#include "controllers/AlphaPix.h"
#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/FPPConnectDialog.h"

#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/ControllerNull.h"
#include "outputs/Output.h"
#include "outputs/NullOutput.h"
#include "outputs/E131Output.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"
#include "outputs/LOROptimisedOutput.h"
#include "outputs/TwinklyOutput.h"
#include "discovery/Discovery.h"
#include "setup/DiscoveryAuthDialog.h"
#include "controllerproperties/ControllerPropertyManager.h"
#include "controllerproperties/ControllerPropertyAdapter.h"

#include <log.h>

void ApplyLoggingSpecialOptions();

// Thread class to ping a single controller
class ControllerPingThread : public wxThread {
public:
    ControllerPingThread(Controller* controller) :
        wxThread(wxTHREAD_DETACHED), _controller(controller) {
        ++pingCount;
    }
    virtual ~ControllerPingThread() {
        --pingCount;
    }
    
    static std::atomic_int pingCount;
    static bool hasOutstandingPings() {
        return pingCount > 0;
    }
protected:
    virtual ExitCode Entry() override {
        if (_controller && _controller->IsActive()) {
            _controller->Ping();
        }
        return (ExitCode)0;
    }

private:
    Controller* _controller;
};
std::atomic_int ControllerPingThread::pingCount(0);

#pragma region Show Directory
void xLightsFrame::OnMenuMRU(wxCommandEvent& event) {
    int id = event.GetId();
    wxString newdir = RecentShowFoldersMenu->GetLabel(id);
    
    if (!ObtainAccessToURL(newdir, true)) {
        std::string dstr = newdir;
        PromptForDirectorySelection("Reselect Show Directory", dstr);
        newdir = dstr;
    }
    SetDir(newdir, true);
}
void xLightsFrame::OnMRUSequence(wxCommandEvent& event) {
    int id = event.GetId();
    wxString fname = RecentSequencesMenu->GetLabel(id);
    OpenSequence(fname, nullptr);
}

void xLightsFrame::UpdateRecentFilesList(bool reload) {
    wxString p = CurrentDir;
    p.Replace("\\", "/");
    int idx = p.Find('/');
    if (idx > 0) {
        p = p.substr(idx + 1);
    }
    // Prefix keys with the show directory so each folder has its own MRU list.
    // SetPath() is a no-op on the flat JSON config store, so the path is embedded
    // directly in the key name instead.
    std::string keyPrefix = p.ToStdString() + "/";
    auto* config = GetXLightsConfig();
    if (reload) {
        mruFiles.clear();
        for (int x = 0; x < MRUF_LENGTH; x++) {
            std::string k = keyPrefix + "file" + std::to_string(x);
            wxString v;
            if (config->Read(k, &v) && v != "") {
                mruFiles.push_back(v);
            }
        }
    }
    while (mruFiles.size() >= MRUF_LENGTH) {
        mruFiles.pop_back();
    }

    for (int x = 0; x < MRUF_LENGTH; x++) {
        if (mruf_MenuItem[x] != nullptr) {
            Disconnect(mruf_MenuItem[x]->GetId(), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMRUSequence);
            RecentSequencesMenu->Delete(mruf_MenuItem[x]);
            mruf_MenuItem[x] = nullptr;
        }
    }
    int i = RecentSequencesMenu->GetMenuItemCount();
    while (i) {
        i--;
        wxMenuItem *item = RecentSequencesMenu->FindItemByPosition(0);
        RecentSequencesMenu->Delete(item);
    }
    int cnt = 0;
    for (int x = 0; x < MRUF_LENGTH; x++) {
        std::string k = keyPrefix + "file" + std::to_string(x);
        if (x < (int)mruFiles.size()) {
            if (!reload) {
                config->Write(k, mruFiles[x]);
            }
            cnt++;
            int menuID = wxNewId();
            mruf_MenuItem[x] = new wxMenuItem(RecentSequencesMenu, menuID, mruFiles[x]);
            mruf_MenuItem[x]->SetBitmap(wxArtProvider::GetBitmapBundle("wxART_FILE_OPEN", wxART_MENU));
            Connect(menuID, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMRUSequence);
            RecentSequencesMenu->Append(mruf_MenuItem[x]);
        } else {
            config->DeleteEntry(k);
        }
    }
    config->Flush();
    RecentSequencesMenu->UpdateUI();
    MenuFile->FindItem(ID_MENUITEM_OPENRECENTSEQUENCE)->Enable(cnt != 0);
}

bool xLightsFrame::SetDir(const wxString& newdir, bool permanent)
{
    if (readOnlyMode) {
        wxMessageBox("Show directory cannot be changed in read only mode.", "Read Only Mode", wxICON_INFORMATION | wxOK);
        return false;
    }
    
    wxString nd = newdir;
    if (nd.EndsWith(wxFileName::GetPathSeparator()))
        nd = nd.SubString(0, nd.size() - 2);

    // don't change show directories with an open sequence because models won't match
    if (!CloseSequence()) {
        return false;
    }

    if (!ObtainAccessToURL(newdir, true)) {
        return false;
    }

    // Everything below synchronously rebuilds the Layout tab's model/controller
    // trees and preview, which scales with show size and can take a while on
    // large shows. Layout is the default visible tab, so without this the
    // window just sits there looking frozen for the whole load.
    if (layoutPanel != nullptr) {
        layoutPanel->ShowLoadingOverlay(_("Loading show..."));
    }
    struct LoadingOverlayGuard {
        LayoutPanel* panel;
        ~LoadingOverlayGuard() {
            if (panel != nullptr) panel->HideLoadingOverlay();
        }
    } loadingOverlayGuard{ layoutPanel };

    layoutPanel->ClearSelectedModelGroup();

    // delete any views that were added to the menu
    for (const auto& [name, grp] : LayoutGroups) {
        RemovePreviewOption(grp.get());
    }
    PreviewWindows.clear();

    // Check to see if any show directory files need to be saved
    CheckUnsavedChanges();
    viewpoint_mgr.Clear();

    // Force re-initialization of Effect Presets panel when show directory changes.
    // If the panel is already visible, reload it immediately; otherwise defer until next show.
    _effectPresetsInitialized = false;
    if (EffectTreeDlg != nullptr && m_mgr->GetPane("EffectPresets").IsShown()) {
        EffectTreeDlg->InitItems(_effectPresetManager);
        _effectPresetsInitialized = true;
    }

    // update most recently used array
    int idx = mruDirectories.Index(nd);
    if (idx != wxNOT_FOUND)
        mruDirectories.RemoveAt(idx);
    if (!CurrentDir.IsEmpty()) {
        idx = mruDirectories.Index(CurrentDir);
        if (idx != wxNOT_FOUND)
            mruDirectories.RemoveAt(idx);
        if (mruDirectories.empty()) {
            mruDirectories.push_back(CurrentDir);
        } else {
            mruDirectories.Insert(CurrentDir, 0);
        }
    }
    size_t cnt = mruDirectories.GetCount();
    if (cnt > MRUD_LENGTH) {
        mruDirectories.RemoveAt(MRUD_LENGTH, cnt - MRUD_LENGTH);
        cnt = MRUD_LENGTH;
    }

    /*
    wxString msg="UpdateMRU:\n";
    for (int i=0; i<mru.GetCount(); i++) msg+="\n" + mru[i];
    spdlog::debug(msg);
    */

    // save config
    bool DirExists = wxFileName::DirExists(nd);
    wxString value;
    auto* config = GetXLightsConfig();
    if (permanent) {
        if (DirExists)
            config->Write("LastDir", nd);
        _permanentShowFolder = nd;
    }
    for (size_t i = 0; i < MRUD_LENGTH; i++) {
        std::string mru_name = "mru" + std::to_string(i);
        if (mrud_MenuItem[i] != nullptr) {
            Disconnect(mrud_MenuItem[i]->GetId(), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
            RecentShowFoldersMenu->Delete(mrud_MenuItem[i]);
            mrud_MenuItem[i] = nullptr;
        }
        if (i < cnt) {
            value = mruDirectories[i];
        } else {
            value = wxEmptyString;
        }
        config->Write(mru_name, value);
    }

    int i = RecentShowFoldersMenu->GetMenuItemCount();
    while (i) {
        i--;
        wxMenuItem* item = RecentShowFoldersMenu->FindItemByPosition(0);
        RecentShowFoldersMenu->Delete(item);
    }

    // append mru items to menu
    cnt = mruDirectories.GetCount();
    for (size_t i = 0; i < cnt; i++) {
        int menuID = wxNewId();
        mrud_MenuItem[i] = new wxMenuItem(RecentShowFoldersMenu, menuID, mruDirectories[i]);
        mrud_MenuItem[i]->SetBitmap(wxArtProvider::GetBitmapBundle("wxART_FOLDER_OPEN", wxART_MENU));
        Connect(menuID, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
        RecentShowFoldersMenu->Append(mrud_MenuItem[i]);
    }
    RecentShowFoldersMenu->UpdateUI();
    MenuFile->FindItem(ID_MENUITEM_RECENTFOLDERS)->Enable(cnt != 0);

    if (!DirExists) {
        wxString msg = _("The show directory '") + nd + ("' no longer exists.\nPlease choose a new show directory.");
        DisplayError(msg, this);
        return false;
    }

    ObtainAccessToURL(ToStdString(nd), true);

    // update UI
    CheckBoxLightOutput->SetValue(false);
    EnableSleepModes();
    _outputManager.StopOutput();
    SetConfigBool("OutputActive", false);
    waitForPingsToComplete();
    _outputManager.DeleteAllControllers();
    CurrentDir = nd;
    showDirectory = nd;
    UpdateRecentFilesList(true);

    SetFixFileShowDir(CurrentDir);
    SpecialOptions::StashShowDir(ToStdString(CurrentDir));
    SpecialOptions::GetOption("", ""); // resets special options
    ApplyLoggingSpecialOptions();

    
    spdlog::info("Show directory set to : {}.", (const char*)showDirectory.c_str());

    if (_logfile != nullptr) {
        wxLog::SetActiveTarget(nullptr);
        fclose(_logfile);
        _logfile = nullptr;
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Error);
    }

    if (SpecialOptions::GetOption("wxLogging", "false") == "true") {
        _logfile = fopen("wxlog_xlights.txt", "w");
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Debug);
        wxLog::SetActiveTarget(new wxLogStderr(_logfile));
    } else {
        wxLog::SetActiveTarget(new wxLogStderr()); // write to stderr
    }

    mediaDirectories.clear();
    if (!xLightsApp::mediaDir.IsNull())
        mediaDirectories.push_back(xLightsApp::mediaDir.ToStdString());
    {
        wxString mediaDirConfig;
        config->Read("MediaDir", &mediaDirConfig);
        if (!mediaDirConfig.empty()) {
            wxArrayString entries = wxSplit(mediaDirConfig, '|', '\0');
            for (auto& d : entries) {
                std::string dstd = d.ToStdString();
                if (std::find(mediaDirectories.begin(), mediaDirectories.end(), dstd) == mediaDirectories.end())
                    mediaDirectories.push_back(dstd);
            }
        }
    }
    {
        std::string curDir = ToStdString(CurrentDir);
        if (std::find(mediaDirectories.begin(), mediaDirectories.end(), curDir) == mediaDirectories.end())
            mediaDirectories.push_back(curDir);
    }

    long fseqLinkFlag = 0;
    config->Read("FSEQLinkFlag", &fseqLinkFlag);
    if (fseqLinkFlag) {
        fseqDirectory = CurrentDir;
        config->Write("FSEQDir", fseqDirectory);
        spdlog::debug("FSEQ Directory set to : {}.", (const char*)fseqDirectory.c_str());
    }

    EnableNetworkChanges();
    DisplayXlightsFilename(wxEmptyString);

    // load network
    networkFile.AssignDir(CurrentDir);
    networkFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
    if (FileExists(networkFile)) {
        ObtainAccessToURL(networkFile.GetFullPath());
        spdlog::debug("Loading networks.");
        wxStopWatch sww;
        if (!_outputManager.Load(ToStdString(CurrentDir))) {
            if (!this->IsVisible()) {
                // File exists, but is not readable, but xLightsFrame hasn't been fully open
                // Assume that xLights doesn't have permission to read from the show directory so
                // prompt to re-aquire access.
                DisplayError(wxString::Format("Unable to load network config %s.  Try reselecting the show directory.", networkFile.GetFullPath()));
                return false;
            }
            DisplayError(ToStdString(wxString::Format("Unable to load network config %s : Time %ldms", networkFile.GetFullPath(), sww.Time())));
        } else {
            spdlog::debug("Loaded network config {} : Time {}ms", (const char*)networkFile.GetFullPath().c_str(), sww.Time());

             // Check for deprecated ZCPP controllers and warn the user
            std::vector<std::string> zcppControllers;
            for (const auto& controller : _outputManager.GetControllers()) {
                for (const auto& output : controller->GetOutputs()) {
                    if (output->GetType() == OUTPUT_ZCPP) {
                        zcppControllers.push_back(controller->GetName());
                        break;
                    }
                }
            }

            if (!zcppControllers.empty()) {
                wxString controllerList;
                for (size_t i = 0; i < zcppControllers.size(); i++) {
                    if (i > 0) controllerList += ", ";
                    controllerList += zcppControllers[i];
                }

                wxString message = wxString::Format(
                    "ZCPP is deprecated and will be removed in a future version.\n\n"
                    "The following controllers are using ZCPP:\n%s\n\n"
                    "Please migrate these controllers to use DDP (preferred) or e1.31.",
                    controllerList);

                wxMessageBox(message, "ZCPP Deprecated", wxOK | wxICON_WARNING, this);
            }
        }
    } else {
        _outputManager.SetShowDir(ToStdString(CurrentDir));
        _outputManager.SetBaseShowDir("");
        _outputManager.SetAutoUpdateFromBaseShowDir(false);
    }

    if (_outputManager.DidConvert()) {
        NetworkChange();
    } else {
        UnsavedNetworkChanges = false;
        UpdateLayoutSave();
    }

    if (layoutPanel != nullptr) {
        layoutPanel->UpdateDirectoriesFooter();
    }

    spdlog::debug("Updating networks on setup tab.");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetDir");
    spdlog::debug("    Networks updated.");

    wxFileName showFolderKbf;
    showFolderKbf.AssignDir(CurrentDir);
    showFolderKbf.SetFullName(XLIGHTS_KEYBINDING_FILE);

    wxFileName appDataKbf;
    appDataKbf.AssignDir(wxString(GetSettingsFilePath().parent_path().wstring()));
    appDataKbf.SetFullName(XLIGHTS_KEYBINDING_FILE);

    // Key bindings can live in the show folder and/or AppData
    bool useAppData = (GetKeybindingsLocation() == "AppData-shared");
    const wxFileName& preferredKbf = useAppData ? appDataKbf : showFolderKbf;
    const wxFileName& otherKbf = useAppData ? showFolderKbf : appDataKbf;

    mainSequencer->keyBindings.SetAdditionalSaveLocation(otherKbf);
    mainSequencer->keyBindings.Load(preferredKbf);

    // Merge controllers from base show folder before loading effects file
    // (model/view object XML merging now happens inside LoadEffectsFile before LoadModels)
    if (_outputManager.IsAutoUpdateFromBaseShowDir() && _outputManager.GetBaseShowDir() != "") {
        if (!ObtainAccessToURL(_outputManager.GetBaseShowDir(), true)) {
            std::string dstr = _outputManager.GetBaseShowDir();
            PromptForDirectorySelection("Reselect Base Show Directory", dstr);
            _outputManager.SetBaseShowDir(dstr);
        }
        if (_outputManager.NeedsBaseControllersUpdate()) {
            spdlog::debug("Updating from base folder on show folder open.");
            bool _acceptAll = false, _rejectAll = false;
            bool changed = false;
            if (_outputManager.MergeFromBase(false, _acceptAll, _rejectAll, nullptr, &changed)) {
                if (changed) {
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetDir-controller");
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetDir-controller");
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "SetDir-controller");
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetDir-controller");
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetDir-controller");
                }
                // Only record the checkpoint if the base file actually loaded; a failed
                // load leaves it unset so the merge is retried on the next open.
                _outputManager.MarkBaseControllersSynced();
            }
        }
    }

    LoadEffectsFile();

    spdlog::debug("Get start channels right.");
    // make sure these won't refire
    _outputModelManager.RemoveWork("ASAP", OutputModelManager::WORK_CALCULATE_START_CHANNELS);
    _outputModelManager.RemoveWork("ASAP", OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG);
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetDir");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "SetDir");
    spdlog::debug("Start channels done.");

    if (mBackupOnLaunch && !_renderMode && !CurrentDir.StartsWith(wxFileName::GetTempDir())) {
        spdlog::debug("Backing up show directory before we do anything this session in this folder : {}.", ToStdString(CurrentDir));
        DoBackup(false, true);
        spdlog::debug("Backup completed.");
    }

    if (std::find(mediaDirectories.begin(), mediaDirectories.end(), CurrentDir) == mediaDirectories.end()) {
        mediaDirectories.push_back(CurrentDir);
    }

    EnableSequenceControls(true);

    Notebook1->ChangeSelection(LAYOUTTAB);
    SetStatusText("");
    FileNameText->SetLabel(nd);

    if (AllModels.ReworkStartChannel() || UnsavedRgbEffectsChanges) {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "SetDir");
    }

    _outputManager.SomethingChanged();
    AllModels.RecalcStartChannels();
    _outputModelManager.RemoveWork("ASAP", OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS);
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RELOAD_MODELLIST, "SetDir-post-rework");

    return true;
}

void xLightsFrame::OnMenuOpenFolderSelected(wxCommandEvent& event) {
    ShowDirectoriesDialog dlg(this);
    dlg.ShowModal();
}

bool xLightsFrame::PromptForDirectorySelection(const std::string &msg, std::string &dir) {
    wxDirDialog DirDialog1(this, msg, dir, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    while (true) {
        if (DirDialog1.ShowModal() == wxID_OK) {
            dir = ToStdString(DirDialog1.GetPath());
            ObtainAccessToURL(dir, true);
            return true;
        }
        DirDialog1.SetPath(dir);
    }
}


bool xLightsFrame::PromptForShowDirectory(bool permanent, const std::string &defaultDir) {

    wxDirDialog DirDialog1(this, _("Select Show Directory"), defaultDir, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    while (DirDialog1.ShowModal() == wxID_OK) {
        bool dirOK = true;
        AbortRender(); // make sure nothing is still rendering
        wxString newdir = DirDialog1.GetPath();
        ObtainAccessToURL(newdir, true);
        if (newdir == CurrentDir) return true;

        if (ShowFolderIsInBackup(ToStdString(newdir))) {
            DisplayWarning("WARNING: Opening a show folder inside a backup folder. This is ok but please make sure you change back to your proper show folder and dont make changes in this folder.", this);
        }

#ifdef __WXMSW__
        if (ShowFolderIsInProgramFiles(ToStdString(newdir))) {
            DisplayWarning("ERROR: Show folder inside your Program Files folder either just wont work or will cause you security issues ... so please choose again.", this);
            dirOK = false;
        }
#endif
        if (dirOK) {
            wxString fn;
            // if new directory contains a networks or rgbeffects file then ok
            if (FileExists(newdir + wxFileName::GetPathSeparator() + XLIGHTS_NETWORK_FILE) || FileExists(newdir + wxFileName::GetPathSeparator() + XLIGHTS_RGBEFFECTS_FILE)) {
            }
            // if new directory is empty then ok
            else if (!wxDir(newdir).GetFirst(&fn)) {
            }
            // otherwise ... this may not be a show directory ... check the user is sure about this
            else if (wxMessageBox("Folder chosen does not contain xLights show folder files and is not empty. Are you sure you chose the right folder?", "Possibly incorrect folder chosen.", wxYES_NO, this) == wxNO) {
                dirOK = false;
            }
        }

        if (dirOK) {
            displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr);
            layoutPanel->ClearUndo();
            return SetDir(newdir, permanent);
        }
    }
    return false;
}

bool xLightsFrame::OfferDefaultShowDirectory(bool permanent) {
    wxString documents = wxStandardPaths::Get().GetDocumentsDir();
    wxString defaultShowDir = documents + wxFileName::GetPathSeparator() + "xLights";

    int answer = wxMessageBox("No show folder is set.\n\nWould you like xLights to create and use a default show folder at:\n\n" + defaultShowDir + "\n\nChoose No to pick a different folder.",
                              "Create Default Show Folder", wxYES_NO | wxICON_QUESTION, this);

    if (answer != wxYES) {
        return PromptForShowDirectory(permanent);
    }

    ObtainAccessToURL(documents, true);
    if (!wxFileName::DirExists(defaultShowDir)) {
        if (!wxFileName::Mkdir(defaultShowDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
            DisplayError(ToStdString("Unable to create the default show folder:\n" + defaultShowDir + "\n\nPlease choose a folder instead."), this);
            return PromptForShowDirectory(permanent);
        }
    }
    ObtainAccessToURL(defaultShowDir, true);
    if (SetDir(defaultShowDir, permanent)) {
        return true;
    }

    // Creating/applying the default failed for some reason - fall back to the picker.
    return PromptForShowDirectory(permanent);
}
#pragma endregion

void xLightsFrame::GetControllerDetailsForChannel(int32_t channel, std::string& controllername, std::string& type, std::string& protocol, std::string& description, int32_t& channeloffset, std::string& ip, std::string& u, std::string& inactive, std::string& baud, int& start_universe, int& start_universe_channel)
{
    int32_t sc = 0;
    int32_t csc = 0;
    Controller* c = _outputManager.GetController(channel, csc);
    Output* o = _outputManager.GetOutput(channel, sc);
    channeloffset = sc;

    channeloffset = -1;
    start_universe = 0;
    start_universe_channel = 0;
    type = "Unknown";
    protocol = "Unknown";
    description = "";
    ip = "";
    u = "";
    inactive = "";
    baud = "";
    controllername = "";

    if (c != nullptr) {
        type = c->GetVMV();
        description = c->GetDescription();
        channeloffset = csc;
        controllername = c->GetName();

        auto eth = dynamic_cast<ControllerEthernet*>(c);
        if (eth != nullptr) {
            ip = eth->GetIP();
            if (o != nullptr)
                u = o->GetUniverseString();
        }

        auto ser = dynamic_cast<ControllerSerial*>(c);
        if (ser != nullptr) {
            ip = ser->GetPort();
            baud = wxString::Format("%d", ser->GetSpeed());
        }

        if (!c->IsActive()) {
            inactive = "DISABLED";
        }
    }

    if (o != nullptr) {
        protocol = o->GetType();
        start_universe = o->GetUniverse();
        start_universe_channel = sc;
    }
}

std::string xLightsFrame::GetChannelToControllerMapping(int32_t channel) {
    int32_t stch;
    Controller* c = _outputManager.GetController(channel, stch);

    if (c != nullptr) {
        return c->GetChannelMapping(channel);
    }
    return ToStdString(wxString::Format("Channel %d could not be mapped to a controller.", channel));
}

// reset test channel listbox
void xLightsFrame::UpdateChannelNames() {

    ChNames.clear();
    ChNames.resize(_outputManager.GetTotalChannels());
    // update names with RGB models where MyDisplay is checked

    // KW left as some of the conversions seem to use this
    for (const auto& it : AllModels) {
        Model *model = it.second;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) {
            auto NodeCount = model->GetNodeCount();
            auto ChanPerNode = model->GetChanCountPerNode();
            wxString FormatSpec = "Ch %d: " + model->name + " #%d";
            for (size_t n = 0; n < NodeCount; n++) {
                auto ChannelNum = model->NodeStartChannel(n);

                size_t NodeNum = n + 1;
                if (ChanPerNode == 1) {
                    if (ChannelNum < (int)ChNames.Count()) {
                        if (ChNames[ChannelNum] == "") {
                            ChNames[ChannelNum] = wxString::Format(FormatSpec, (int)ChannelNum + 1, (int)NodeNum);
                        }
                    }
                }
                else {
                    for (auto c = 0; c < ChanPerNode; c++) {
                        if (ChannelNum < (int)ChNames.Count()) {
                            if (ChNames[ChannelNum] == "") {
                                ChNames[ChannelNum] = wxString::Format(FormatSpec, (int)ChannelNum + 1, (int)NodeNum) + model->GetChannelColorLetter(c);
                            }
                        }
                        ChannelNum++;
                    }
                }
            }
        }
    }
}


void xLightsFrame::NetworkChange() {

    _outputManager.SomethingChanged();
    UnsavedNetworkChanges = true;
    UpdateLayoutSave();
}

void xLightsFrame::NetworkChannelsChange() {

    auto logger_work = spdlog::get("work");
    logger_work->debug("        NetworkChannelsChange.");

    _outputManager.SomethingChanged();
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "NetworkChannelsChange");
}

bool xLightsFrame::SaveNetworksFile() {

    auto logger_work = spdlog::get("work");
    logger_work->debug("        SaveNetworksFile.");

    // if any of the controllers are in auto layout mode ... recalculate them
    bool autoLayout = false;
    for (const auto& it : _outputManager.GetControllers()) {
        if (it->IsAutoLayout()) {
            autoLayout = true;
            break;
        }
    }

    if (autoLayout) {
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ControllerModelDialog::ControllerModelDialog");
        GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ControllerModelDialog::ControllerModelDialog");

        // Now need to let all the recalculations work
        while (!DoAllWork()) {
            // dont get into a redraw loop from here
            GetOutputModelManager()->RemoveWork("ASAP", OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
        }
    }

    if (_outputManager.Save()) {
        UnsavedNetworkChanges = false;
        UpdateLayoutSave();
        return true;
    } else {
        DisplayError(_("Unable to save network definition file"), this);
        return false;
    }
}

void xLightsFrame::UpdateLayoutSave() {
    if (UnsavedRgbEffectsChanges || UnsavedNetworkChanges || UnsavedPresetChanges) {
#ifdef __WXOSX__
        SetButtonBackground(layoutPanel->ButtonSavePreview, wxColour(255, 0, 0), 2);
#else
        layoutPanel->ButtonSavePreview->SetBackgroundColour(wxColour(255, 108, 108));
#endif
    } else {
#ifdef __WXOSX__
        SetButtonBackground(layoutPanel->ButtonSavePreview, wxTransparentColour, 2);
#else
        layoutPanel->ButtonSavePreview->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    }
}

void xLightsFrame::SetSyncUniverse(int syncUniverse) {

    _outputManager.SetSyncUniverse(syncUniverse);
}

void xLightsFrame::PingController(Controller* e) {

    if (e != nullptr) {
		std::string name = e->GetPingDescription();
		switch (e->Ping()) {
        case Output::PINGSTATE::PING_OK:
			case Output::PINGSTATE::PING_WEBOK:
				DisplayInfo("Pinging the Controller was Successful: " + name, this);
				break;
			case Output::PINGSTATE::PING_OPENED:
				DisplayInfo("Serial Port Exists and was Opened: " + name, this);
				break;
			case Output::PINGSTATE::PING_OPEN:
				DisplayWarning("Serial Port Exists but couldn't be opened: " + name, this);
				break;
			case Output::PINGSTATE::PING_UNAVAILABLE:
				DisplayWarning("Controller Status is Unavailable: " + name, this);
				break;
			case Output::PINGSTATE::PING_UNKNOWN:
				DisplayWarning("Controller Status is Unknown: " + name, this);
				break;
			case Output::PINGSTATE::PING_ALLFAILED:
				DisplayError("Unable to Communicate with the Controller: " + name, this);
				break;
		}
	}
}

#pragma region Work
void xLightsFrame::DoASAPWork() {

    auto logger_work = spdlog::get("work");
    // If any function called in DoWork yields then this can reenter and we need to stop that
    static bool reenter = false;
    if (reenter) {
        // we need to clear the work requested flag to ensure we get called again
        _outputModelManager.ClearWorkRequested();
        return;
    }
    reenter = true;

    logger_work->debug("Doing ASAP Work.");
    DoWork(_outputModelManager.GetASAPWork(), "ASAP");

    reenter = false;
}

bool xLightsFrame::DoAllWork() {

    auto logger_work = spdlog::get("work");
    logger_work->debug("Doing All Work.");
    DoWork(_outputModelManager.GetSetupWork(), "Setup");
    DoWork(_outputModelManager.GetLayoutWork(), "Layout");
    DoWork(_outputModelManager.GetASAPWork(), "ASAP");
    return (_outputModelManager.PeekASAPWork() == OutputModelManager::WORK_NOTHING &&
            _outputModelManager.PeekSetupWork() == OutputModelManager::WORK_NOTHING &&
            _outputModelManager.PeekLayoutWork() == OutputModelManager::WORK_NOTHING);
}

void xLightsFrame::DoWork(uint32_t work, const std::string& type, BaseObject* m, const std::string& selected) {

    auto logger_work = spdlog::get("work");
    if (work == OutputModelManager::WORK_NOTHING) return;

    std::string selectedModel = selected;
    if (selectedModel == "") selectedModel = _outputModelManager.GetSelectedModel();

    if (work & OutputModelManager::WORK_NETWORK_CHANGE) {
        logger_work->debug("    WORK_NETWORK_CHANGE.");
        // Mark networks file dirty
        NetworkChange();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_NETWORK_CHANNELSCHANGE |
        OutputModelManager::WORK_UPDATE_NETWORK_LIST |
        OutputModelManager::WORK_UPDATE_NETWORK_PROPERTIES |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
        OutputModelManager::WORK_PRESET_CHANGE |
        OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
        OutputModelManager::WORK_RELOAD_ALLMODELS |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS |
        OutputModelManager::WORK_FOCUS_MODELTREE
    );
    if (work & OutputModelManager::WORK_NETWORK_CHANNELSCHANGE) {
        logger_work->debug("    WORK_NETWORK_CHANNELSCHANGE.");
        // Recalculates all the channels in the outputs
        NetworkChannelsChange();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_UPDATE_NETWORK_LIST |
        OutputModelManager::WORK_UPDATE_NETWORK_PROPERTIES |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
        OutputModelManager::WORK_PRESET_CHANGE |
        OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
        OutputModelManager::WORK_RELOAD_ALLMODELS |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & (OutputModelManager::WORK_UPDATE_NETWORK_LIST | OutputModelManager::WORK_UPDATE_NETWORK_PROPERTIES)) {
        logger_work->debug("    WORK_UPDATE_NETWORK_LIST.");
        // Updates the list of outputs on the screen
        //UpdateNetworkList();
        if (layoutPanel != nullptr && layoutPanel->GetControllerListPanel() != nullptr) {
            layoutPanel->GetControllerListPanel()->UpdateControllerList();
        }

        std::string selectedController = _outputModelManager.GetSelectedController();
        if (selectedController != "") {
            if (layoutPanel != nullptr && layoutPanel->GetControllerListPanel() != nullptr) {
                layoutPanel->GetControllerListPanel()->SelectController(selectedController);
            }
        }
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
        OutputModelManager::WORK_PRESET_CHANGE |
        OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
        OutputModelManager::WORK_RELOAD_ALLMODELS |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RGBEFFECTS_CHANGE) {
        logger_work->debug("    WORK_RGBEFFECTS_CHANGE.");
        // Mark the rgb effects file as needing to be saved
        MarkEffectsFileDirty();
    }
    if (work & OutputModelManager::WORK_PRESET_CHANGE) {
        logger_work->debug("    WORK_PRESET_CHANGE.");
        MarkPresetsDirty();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_MODEL_FROM_XML |
        OutputModelManager::WORK_RELOAD_ALLMODELS |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_MODEL_FROM_XML && !(work & OutputModelManager::WORK_RELOAD_ALLMODELS)) {
        logger_work->debug("    WORK_RELOAD_MODEL_FROM_XML.");
        BaseObject* mm = m;
        if (mm == nullptr) mm = _outputModelManager.GetModelToReload();
        if (mm != nullptr) {
            //abort any render as it might crash if the model changes
            AbortRender();
            mm->ReloadModel();
            //must unselect any effect as it might now be pointing at an invalid model/submodel/strand
            UnselectEffect();
        }
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_ALLMODELS |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_ALLMODELS) {
        logger_work->debug("    WORK_RELOAD_ALLMODELS.");
        UpdateModelsList();
        //layoutPanel->RefreshLayout();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS) {
        logger_work->debug("    WORK_MODELS_REWORK_STARTCHANNELS.");
        // Moves all the models around optimally

        //abort any render as it will crash if the model changes
        AbortRender();
        if (AllModels.ReworkStartChannel())
        {
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "DoWork");
        }
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER) {
        logger_work->debug("    WORK_MODELS_CHANGE_REQUIRING_RERENDER.");
        // increment the model count which triggers re-rendering due to models changing
        MarkModelsAsNeedingRender();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_CALCULATE_START_CHANNELS |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_CALCULATE_START_CHANNELS) {
        logger_work->debug("    WORK_CALCULATE_START_CHANNELS.");
        // Recalculates the models actual start channels based on changes to the outputs
        RecalcModels();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG) {
        logger_work->debug("    WORK_RESEND_CONTROLLER_CONFIG.");
        // Rebuilds generally ZCPP controller config
        // Should happen whenever models are changed or a ZCPP output is changed
        RebuildControllerConfig(&_outputManager, &AllModels);
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_MODELLIST |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_MODELLIST) {

        // if we are drawing a new model
        if (layoutPanel->IsNewModel(nullptr)) {
            logger_work->debug("    WORK_RELOAD_MODELLIST - model being added.");
            // reload the models list on the layout panel
            layoutPanel->refreshModelList();
        }
        else {
            logger_work->debug("    WORK_RELOAD_MODELLIST - model NOT being added.");
            // need to reload the modelPreview model lists or bad things will happen
            layoutPanel->ReloadModelList();
        }
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_OBJECTLIST) {
        logger_work->debug("    WORK_RELOAD_OBJECTLIST.");
        // reload the objects list on the layout panel
        layoutPanel->refreshObjectList();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    // The visualiser runs modal over the Layout tab and queues work as the user
    // drags models between ports. Selecting into the tree from here would fight
    // the dialog for the selection (and switch the notebook page out from under
    // it), so defer until it closes — the tree is rebuilt on close anyway.
    if (selectedModel != "" && !ControllerModelDialog::IsAnyActive()) {
        logger_work->debug("    Selecting model '{}'.", (const char*)selectedModel.c_str());
        //SelectModel(selectModel);
        layoutPanel->SelectBaseObject(selectedModel);
        if (work & OutputModelManager::WORK_FOCUS_MODELTREE) {
            layoutPanel->FocusModelTree();
        }
    }
    if (work & OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW) {
        logger_work->debug("    WORK_REDRAW_LAYOUTPREVIEW.");
        // repaint the layout panel
        layoutPanel->UpdatePreview();
        // Since the layout panel list selection was implemented the redraw triggers a redraw ... this is a problem .. until that is fix suppress the repeat
        _outputModelManager.RemoveWork(type, OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW);
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_PROPERTYGRID) {
        logger_work->debug("    WORK_RELOAD_PROPERTYGRID.");
        // Reload the property grid either because a value changed and needs to be shown or optional properties should be added or removed
        layoutPanel->resetPropertyGrid();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_UPDATE_PROPERTYGRID) {
        logger_work->debug("    WORK_UPDATE_PROPERTYGRID.");
        // Update the property grid mainly enabling and disabling of properties
        layoutPanel->updatePropertyGrid();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_SAVE_NETWORKS) {

        // I am disabling this. It was originally added when ZCPP was introduced to ensure autosize was not lost but
        // in hindsight i think this is actually confusing and should not happen automatically

        //logger_work.debug("    WORK_SAVE_NETWORKS.");
        // write the networks file to disk and clears the dirty flag
        //SaveNetworksFile();
        //if (IsControllersAndLayoutTabSaveLinked()) {
        //    layoutPanel->SaveEffects();
        //}
    }

    UpdateLayoutSave();

    // ensure all model groups have all valid model pointers
    AllModels.ResetModelGroups();

    // Keep the selected model for any subsequence ASAP work
    _outputModelManager.SetSelectedModelIfASAPWorkExists(selectedModel);
}

void xLightsFrame::DoLayoutWork() {

    auto logger_work = spdlog::get("work");
    logger_work->debug("Doing Switch To Layout Tab Work.");
    DoWork(_outputModelManager.GetLayoutWork(), "Layout");
}

#pragma endregion

void xLightsFrame::SetE131Sync(bool b) {

    NetworkChange();
    me131Sync = b;
    _outputManager.SetSyncEnabled(me131Sync);
    if (me131Sync) {
        // recycle output connections if necessary
        CycleOutputsIfOn();
    }
    NetworkChange();
}

void xLightsFrame::EnableNetworkChanges() {
    CheckBoxLightOutput->Enable(!CurrentDir.IsEmpty());
    if (layoutPanel != nullptr && layoutPanel->GetControllerListPanel() != nullptr) {
        layoutPanel->GetControllerListPanel()->UpdateControllerProperties();
    }
}

// Still invoked directly (not via a widget click event) from the Layout tab's
// "Add Controller" button menu (LayoutPanel::OnAddControllerButtonClicked) and
// from ControllerListPanel's own "Discover" action.
void xLightsFrame::OnButtonDiscoverClick(wxCommandEvent& event) {


    spdlog::debug("[Discovery] Running controller discovery.");
    SetStatusText("Running controller discovery ...");
    SetCursor(wxCURSOR_WAIT);

    bool hasChanges = false;

    wxDiscoveryDelegate delegate(this);
    Discovery discovery(&_outputManager, &delegate);
    PrepareAllControllerDiscovery(discovery);
    discovery.Discover();

    SetStatusText("Processing discovered controllers...");
    spdlog::debug("[Discovery] Processing discovered controllers...");
    struct FPPDiscInfo {
        std::string hostname;
        std::string ip;
        std::string uuid;
        int ipsPerUUID;
        bool isUniqueHostname;
        bool isPingable;

        bool operator<(const FPPDiscInfo& other) const {
            return std::tie(hostname, ip, uuid) < std::tie(other.hostname, other.ip, other.uuid);
        }
    };
    std::vector<FPPDiscInfo> fppDiscInfo;

    for (const auto& it : discovery.GetResults()) {
        if (!it->ip.empty() && it->controller != nullptr) {
            fppDiscInfo.push_back({it->hostname, it->ip, it->uuid, 0, true, true});
        }
    }

    std::map<std::string, int> ipsPerUUID;
    std::map<std::string, std::set<std::string>> uniqueHostnames;
    for (const auto& info : fppDiscInfo) {
        if (!info.uuid.empty()) {
            ipsPerUUID[info.uuid]++;
        }
        if (!info.hostname.empty()) {
            uniqueHostnames[info.hostname].insert(info.uuid);
        }
    }

    std::string dupHostnames;
    std::map<std::string, bool> isPingable;
    for (auto& it : fppDiscInfo) {
        it.ipsPerUUID = ipsPerUUID[it.uuid];
        if (it.ipsPerUUID > 1) {
            auto res = IPOutput::Ping(it.ip, "");
            it.isPingable = (res == Output::PINGSTATE::PING_OK);
        }
        isPingable[it.ip] = it.isPingable;
        it.isUniqueHostname = (uniqueHostnames[it.hostname].size() == 1);
        if (uniqueHostnames[it.hostname].size() > 1) {
            if (dupHostnames.find(it.hostname) == std::string::npos) {
                dupHostnames += it.hostname + ",";
            }
        }
    }

    if (!dupHostnames.empty()) {
        dupHostnames.pop_back();
        wxMessageBox("Found the same hostname(s) for multiple devices (" + dupHostnames + "). This is really not good. Names should be unique across all devices. Make sure you rename your default FPP instances. Be careful to read through the prompts that may come up to ensure the proper controllers are added.", "Multiple Hostnames", wxOK);
    }

    std::map<std::string, std::string> renames;
    bool found = false;

    for (int x = 0; x < (int)discovery.GetResults().size(); x++) {
        auto discovered = discovery.GetResults()[x];
        spdlog::debug("[Discovery] Processing: {}  IP: {}", discovered->hostname.c_str(), discovered->ip.c_str());
        SetStatusText("Processing controller " + discovered->hostname + " IP:" + discovered->ip + " ...");

        if (!discovered->controller) {
            continue;
        }

        if (discovered->typeId > 0 && discovered->typeId < 0x80) {
            discovered->controller->SetActive("xLights Only");
            if (discovered->controller->GetVendor().empty()) {
                discovered->controller->SetVendor("FPP");
                discovered->controller->SetModel(StartsWith(discovered->mode, "player") ? "FPP Player Only" : "");
            }
        }

        ControllerEthernet *it = discovered->controller;
        auto c = _outputManager.GetControllers(it->GetIP());
        if (c.size() == 0) {
            bool updated = false;
            for (const auto& itc : _outputManager.GetControllers()) {
                auto eth = dynamic_cast<ControllerEthernet*>(itc);
                if (eth != nullptr
                    && eth->GetName() == it->GetName()
                    && eth->GetProtocol() == it->GetProtocol()) {
                    if (ip_utils::IsValidHostname(eth->GetIP())) {
                        updated = true;
                        found = true;
                    } else {
                        if (isPingable[it->GetIP()]) {
                            wxMessageDialog dialog(nullptr, "The discovered controller (" + it->GetIP() + ") matches an existing controller name (" + eth->GetName() + "/" + eth->GetIP() + "). Do you want to update the IP address of controller?", "Mismatch IP", wxYES_NO | wxCANCEL | wxICON_QUESTION);
                            dialog.SetYesNoCancelLabels("Yes", "Add New", "Skip");
                            int result = dialog.ShowModal();
                            if (result == wxID_YES) {
                                updated = true;
                                eth->SetIP(it->GetIP());
                                found = true;
                            } else if (result == wxID_CANCEL) {
                                updated = true;
                                found = true;
                            }
                        } else {
                            updated = true;
                            found = true;
                        }
                    }
                }
            }
            if (!updated) {
                // we need to ensure the id is still unique
                it->EnsureUniqueId();
                it->EnsureUniqueName();

                if (isPingable[it->GetIP()]) {
                    std::string hostName = discovered->hostname;
                    spdlog::debug("[Discovery] Adding: {} at: {}", hostName.c_str(), it->GetIP().c_str());
                    if (!hostName.empty()) {
                        if (ip_utils::ResolveIP(hostName + ".local") != hostName + ".local") {
                            it->SetIP(::Lower(hostName) + ".local");
                        } else if (ip_utils::ResolveIP(hostName) != hostName) {
                            it->SetIP(::Lower(hostName));
                        }
                    }
                    _outputManager.AddController(it);
                }
                discovered->controller = nullptr;
                found = true;
            }
        } else {
            if (c.size() == 1
                && it->GetName() != c.front()->GetName()
                && c.front()->GetProtocol() == it->GetProtocol()) {
                // existing zcpp with same ip but different name ... maybe we should update the name
                if (wxMessageBox("The discovered controller (" + c.front()->GetName() + ") matches an existing controller IP address but has a different name (" + it->GetName() + "). Do you want to update the name for the existing controller in xLights?", "Mismatch controller name", wxYES_NO, this) == wxYES)
                    {
                    renames[c.front()->GetName()] = it->GetName();
                    c.front()->SetName(it->GetName());
                    found = true;
                }
            }
        }
    }
    if (found) {
        hasChanges = true;
        // update the controller name on any models which use renamed controllers
        for (auto it = renames.begin(); it != renames.end(); ++it) {
            spdlog::debug("Discovered controller renamed from '{}' to '{}'", (const char*)it->first.c_str(), (const char*)it->second.c_str());

            for (auto itm = AllModels.begin(); itm != AllModels.end(); ++itm) {
                if (itm->second->GetControllerName() == it->first) {
                    itm->second->SetControllerName(it->second);
                }
            }
        }
    }

    SetCursor(wxCURSOR_DEFAULT);
    if (hasChanges) {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButton_DiscoverClick");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButton_DiscoverClick");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButton_DiscoverClick");
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButton_DiscoverClick");
    }
    SetStatusText("Discovery complete.");
    spdlog::debug("[Discovery] Controller discovery complete.");
}

void xLightsFrame::OnButtonAddControllerSerialClick(wxCommandEvent& event) {

    auto c = new ControllerSerial(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerSerialClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerSerialClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerSerialClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerSerialClick");
}

void xLightsFrame::OnButtonAddControllerEthernetClick(wxCommandEvent& event) {

    auto c = new ControllerEthernet(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerEthernetClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerEthernetClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerEthernetClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerEthernetClick");
}

void xLightsFrame::OnButtonAddControllerNullClick(wxCommandEvent& event) {

    auto c = new ControllerNull(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerNullClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerNullClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerNullClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerNullClick");
}

bool xLightsFrame::IsControllerListVisible() const {
    return Notebook1->GetSelection() == LAYOUTTAB && layoutPanel != nullptr &&
           layoutPanel->GetControllerListPanel() != nullptr && layoutPanel->IsControllersPageActive();
}

void xLightsFrame::PingActiveControllers() {
    _pingInProgress = true;
    for (const auto& it : _outputManager.GetControllers()) {
        if (it->IsActive()) {
            ControllerPingThread* thread = new ControllerPingThread(it);
            thread->Run();
        }
    }
    _pingInProgress = false;
}

void xLightsFrame::OnPingTimer(wxTimerEvent& event) {
    if (!IsControllerListVisible() || _pingInProgress) {
        return;
    }
    PingActiveControllers();
    StatusRefreshTimer(event);
}
void xLightsFrame::waitForPingsToComplete() {
    while (ControllerPingThread::hasOutstandingPings()) {
        std::this_thread::yield();
    }
}

void xLightsFrame::RefreshControllerStatusColumn() {
    if (Notebook1->GetSelection() == LAYOUTTAB && layoutPanel != nullptr &&
        layoutPanel->GetControllerListPanel() != nullptr && layoutPanel->IsControllersPageActive()) {
        layoutPanel->GetControllerListPanel()->RefreshStatusColumn();
    }
}

void xLightsFrame::StatusRefreshTimer(wxTimerEvent& event) {
    RefreshControllerStatusColumn();
}


void xLightsFrame::RefreshControllerStatusNow() {
    if (_pingInProgress) {
        return;
    }
    PingActiveControllers();
    RefreshControllerStatusColumn();


    if (_controllerPingInterval > 0) {
        _pingTimer->Start(_controllerPingInterval * 1000);
        _statusRefreshTimer->Start(_controllerPingInterval / 2 * 1000);
    }
}

ControllerCaps* xLightsFrame::GetControllerCaps(const std::string& name) {

    auto controller = _outputManager.GetController(name);
    if (controller == nullptr) return nullptr;
    return ControllerCaps::GetControllerConfig(controller->GetVendor(), controller->GetModel(), controller->GetVariant());
}

bool xLightsFrame::ControllerSupportsOutputUpload(Controller* controller) {
    if (controller == nullptr) return false;
    auto caps = GetControllerCaps(controller->GetName());
    if (caps == nullptr || !caps->SupportsUpload()) return false;
    auto eth = dynamic_cast<ControllerEthernet*>(controller);
    return eth == nullptr || eth->GetProtocol() != OUTPUT_ZCPP;
}

#pragma region Selected Controller Actions
void xLightsFrame::UploadControllerInput(Controller* controller) {
    if (controller == nullptr) return;

    if (IsControllerUploadLinked()) {
        SetStatusText("Uploading inputs and outputs.");
    } else {
        SetStatusText("Uploading inputs.");
    }

    SetCursor(wxCURSOR_WAIT);

    if (controller->GetFPPProxy() != "") {
        if (!FPP::ValidateProxy(controller->GetIP(), controller->GetFPPProxy())) {
            wxMessageBox("FPP " + controller->GetFPPProxy() + " is either not online or does not have this controller in its proxy table.");
        }
    }
    wxString message;
    if (UploadInputToController(controller, message)) {
        if (IsControllerUploadLinked() && ControllerSupportsOutputUpload(controller)) {
            UploadOutputToController(controller, message);
        }
    }

    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::UploadControllerOutput(Controller* controller) {
    if (controller == nullptr) return;

    SetCursor(wxCURSOR_WAIT);

    if (controller->GetFPPProxy() != "") {
        if (!FPP::ValidateProxy(controller->GetIP(), controller->GetFPPProxy())) {
            wxMessageBox("FPP proxy " + controller->GetFPPProxy() + " is either not online or does not have this controller in its proxy table. This upload may fail until this is corrected.");
        }
    }

    bool ok = true;
    wxString message;
    auto caps = GetControllerCaps(controller->GetName());
    if (IsControllerUploadLinked() && caps != nullptr && caps->SupportsInputOnlyUpload()) {
        SetStatusText("Uploading inputs and outputs.");
        ok = UploadInputToController(controller, message);
    } else {
        SetStatusText("Uploading outputs");
    }

    if (ok) UploadOutputToController(controller, message);

    SetCursor(wxCURSOR_ARROW);
}

bool xLightsFrame::UploadInputToController(Controller* controller, wxString &message) {
    message.clear();
    bool res = false;

    SetStatusText(message);

    
    if (controller == nullptr) return res;

    auto caps = GetControllerCaps(controller->GetName());
    if (caps != nullptr) {
        SetStatusText("Uploading inputs to controller.");
        caps->Dump();
        if (caps->SupportsInputOnlyUpload()) {
            auto vendor = controller->GetVendor();
            auto model = controller->GetModel();
            auto ip = controller->GetResolvedIP(true);
            if (ip == "MULTICAST" || ip == "") {
                wxTextEntryDialog dlg(this, "Controller IP Address", "IP Address", ip);
                if (dlg.ShowModal() != wxID_OK) {
                    SetCursor(wxCURSOR_ARROW);
                    return res;
                }
                ip = dlg.GetValue();
            }
            RecalcModels();

            BaseController* bc = BaseController::CreateBaseController(controller, ip);
            if (bc != nullptr) {
                if (bc->IsConnected()) {
                    if (bc->SetInputUniverses(controller, this)) {
                        spdlog::debug("Attempt to upload controller inputs successful on controller {}:{}:{}", (const char*)controller->GetVendor().c_str(), (const char*)controller->GetModel().c_str(), (const char*)controller->GetVariant().c_str());
                        message = vendor + " Input Upload complete.";
                        res = true;
                        {
                            auto ts = FormatTimestamp();
                            auto* config = GetXLightsConfig();
                            auto ctrlName = controller->GetName();
                            config->Write(MakeControllerTimestampKey("LastInputUpload", ctrlName, showDirectory), wxString::FromUTF8(ts.c_str()));
                            config->Flush();
                        }
                    }
                    else {
                        spdlog::error("Attempt to upload controller inputs failed on controller {}:{}:{}", (const char*)controller->GetVendor().c_str(), (const char*)controller->GetModel().c_str(), (const char*)controller->GetVariant().c_str());
                        message = vendor + " Input Upload failed.";
                    }
                }
                else {
                    message = vendor + " Input Upload Failed. Unable to connect";
                }
                delete bc;
            }
            else {
                spdlog::error("Unable to create base controller {}:{}:{}", (const char*)controller->GetVendor().c_str(), (const char*)controller->GetModel().c_str(), (const char*)controller->GetVariant().c_str());
                message = vendor + " Input Upload not supported.";
            }
        }
        else {
            // This controller does not support uploads
            spdlog::error("Attempt to upload controller inputs on a unsupported controller {}:{}:{}", (const char*)controller->GetVendor().c_str(), (const char*)controller->GetModel().c_str(), (const char*)controller->GetVariant().c_str());
            message = "Upload inputs not supported.";
        }
    }
    else {
        spdlog::error("Unable to find controller capabilities info.");
        message = "Unable to find controller capabilities info.";
        wxASSERT(false);
    }
    SetStatusText(message);
    return res;
}

bool xLightsFrame::UploadOutputToController(Controller* controller, wxString& message) {
    message.clear();
    bool res = false;

    SetStatusText(message);

    
    if (controller == nullptr) return res;

    auto caps = GetControllerCaps(controller->GetName());
    if (caps != nullptr) {
        SetStatusText("Uploading outputs to controller.");
        caps->Dump();
        if (caps->SupportsUpload()) {
            auto vendor = controller->GetVendor();
            auto model = controller->GetModel();
            auto ip = controller->GetResolvedIP(true);
            if (ip == "MULTICAST") {
                wxTextEntryDialog dlg(this, "Controller IP Address", "IP Address", ip);
                if (dlg.ShowModal() != wxID_OK) {
                    SetCursor(wxCURSOR_ARROW);
                    return res;
                }
                ip = dlg.GetValue();
            }
            //auto proxy = controller->GetFPPProxy();
            RecalcModels();

            BaseController* bc = BaseController::CreateBaseController(controller, ip);
            if (bc != nullptr) {
                if (bc->IsConnected()) {
                    if (bc->SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        message = vendor + " Output Upload Complete.";
                        res = true;
                        {
                            auto ts = FormatTimestamp();
                            auto* config = GetXLightsConfig();
                            auto ctrlName = controller->GetName();
                            config->Write(MakeControllerTimestampKey("LastOutputUpload", ctrlName, showDirectory), wxString::FromUTF8(ts.c_str()));
                            config->Flush();
                        }
                    } else {
                        message = vendor + " Output Upload Failed.";
                    }
                } else {
                    message = vendor + " Output Upload Failed. Unable to connect";
                }
                delete bc;
            } else {
                message = vendor + " Output Upload Failed.";
            }
        } else {
            spdlog::error("Controller does not support upload.");
            message = "Controller does not support upload.";
        }
    } else {
        spdlog::error("Unable to find controller capabilities info.");
        message = "Unable to find controller capabilities info.";
        wxASSERT(false);
    }
    SetStatusText(message);
    return res;
}
#pragma endregion

#pragma region ZCPP
int xLightsFrame::SetZCPPPort(Controller* controller, std::list<ZCPP_packet_t*>& modelDatas, int index, UDControllerPort* port, int portNum, int virtualString, long baseStart, bool isSerial, ZCPPOutput* zcpp) {

    auto current = modelDatas.back();
    if (current->Configuration.ports >= ZCPP_CONFIG_MAX_PORT_PER_PACKET) {
        ZCPP_packet_t* modelData = new ZCPP_packet_t();
        modelDatas.push_back(modelData);
        current = modelDatas.back();
        ZCPPOutput::InitialiseModelDataPacket(current, modelsChangeCount, zcpp->GetPriority(), controller->GetName());
    }
    current->Configuration.ports++;

    UDControllerPortModel* m = nullptr;
    if (port != nullptr && port->GetModels().size() > 0) m = port->GetFirstModel();

    int vvs = (virtualString & 0x3F);
    int ssr = (virtualString >> 6);

    UDVirtualString* vs = nullptr;
    if (port != nullptr && port->GetVirtualStringCount() > 0) {
        vs = port->GetVirtualString(vvs);
    }

    spdlog::debug("    Port/String/SmartRemote {}/{}/{}", portNum, vvs, ssr);

    ZCPP_PortConfig* p = current->Configuration.PortConfig + (current->Configuration.ports - 1);
    wxASSERT((size_t)p < (size_t)current + sizeof(ZCPP_packet_t) - sizeof(ZCPP_PortConfig)); // check pointer has not gone rogue

    p->port = portNum | (isSerial ? 0x80 : 0x00);
    p->string = virtualString;
    std::string protocol = "ws2811";
    if (port != nullptr) {
        protocol = port->GetProtocol();
    }
    p->protocol = ZCPPOutput::EncodeProtocol(protocol);
    spdlog::debug("       Protocol {}/{}", ZCPPOutput::EncodeProtocol(protocol), protocol);

    int32_t sc = 0;
    if (vs != nullptr) {
        sc = vs->_startChannel - baseStart;
    }
    else if (port != nullptr) {
        if (port->IsPixelProtocol()) {
            if (port->GetModels().size() > 0) {
                sc = port->GetStartChannel() - baseStart;
            }
        }
        else {
            sc = 999999999;
            for (const auto it : port->GetModels()) {
                // we ignore chained models as they cant be the first
                if (it->GetModel()->GetModelChain() == "" || it->GetModel()->GetModelChain() == "Beginning") {
                    sc = std::min(sc, (int32_t)it->GetStartChannel() - (int32_t)baseStart - it->GetDMXChannelOffset() + 1);
                    break;
                }
            }
            if (sc == 999999999) sc = 0;
        }
    }
    if (sc < 0) sc = 0;
    p->startChannel = ntohl(sc);
    spdlog::debug("       Start Channel {}", sc);

    long c = 0;
    if (vs != nullptr) {
        c = vs->Channels();
    }
    else if (port != nullptr) {
        if (port->IsPixelProtocol()) {
            if (port->GetModels().size() > 0) {
                c = port->Channels();
            }
        }
        else {
            for (const auto& it : port->GetModels()) {
                c = std::max(c, it->GetEndChannel() - sc - baseStart + 1);
            }
        }
    }
    p->channels = ntohl(c);
    spdlog::debug("       Channels {}", c);

    wxByte gc = 1;
    if (vs != nullptr) {
        if (vs->_groupCountSet) {
            gc = vs->_groupCount;
        }
    }
    else if (m != nullptr) {
        gc = m->GetGroupCount(1);
    }
    p->grouping = gc;
    spdlog::debug("       Group Count {}", (int)gc);

    wxByte directionColourOrder = 0x00;
    if (vs != nullptr) {
        if (vs->_reverseSet) {
            if (vs->_reverse == "Reverse") {
                directionColourOrder += 0x80;
            }
        }

        if (vs->_colourOrderSet) {
            directionColourOrder += ZCPPOutput::EncodeColourOrder(vs->_colourOrder);
        }
    }
    else if (m != nullptr) {
        directionColourOrder += m->GetDirection("Forward") == "Reverse" ? 0x80 : 0x00;
        directionColourOrder += ZCPPOutput::EncodeColourOrder(port->GetFirstModel()->GetColourOrder("RGB"));
    }
    p->directionColourOrder = directionColourOrder;
    spdlog::debug("       Direction/Colour Order {}/{}", (int)directionColourOrder & 0x80, (int)directionColourOrder & 0x7F);

    wxByte np = 0;
    if (vs != nullptr) {
        if (vs->_startNullPixelsSet) {
            np = vs->_startNullPixels;
        }
    }
    else if (m != nullptr) {
        np = m->GetStartNullPixels(0);
    }
    p->nullPixels = np;
    spdlog::debug("       Start Null Pixels {}", (int)np);

    wxByte b = controller->GetDefaultBrightnessUnderFullControl();
    if (vs != nullptr) {
        if (vs->_brightnessSet) {
            b = vs->_brightness;
        }
    }
    else if (m != nullptr) {
        b = m->GetBrightness(controller->GetDefaultBrightnessUnderFullControl());
    }
    p->brightness = b;
    spdlog::debug("       Brightness {}", (int)b);

    wxByte g = 10;
    if (vs != nullptr) {
        if (vs->_gammaSet) {
            g = vs->_gamma * 10.0;
        }
    }
    else if (m != nullptr) {
        g = m->GetGamma(1) * 10.0;
    }
    p->gamma = g;
    spdlog::debug("       Gamma {}", (int)g);

    return sizeof(ZCPP_PortConfig);
}

void xLightsFrame::SetZCPPExtraConfig(std::list<ZCPP_packet_t*>& extraConfigs, int portNum, int virtualStringNum, const std::string& name, ZCPPOutput* zcpp) {

    auto current = extraConfigs.back();
    uint16_t pos = ZCPP_GetPacketActualSize(*current);
    wxASSERT(pos < sizeof(ZCPP_packet_t)); // check packet has not gone rogue
    if (pos + ZCPP_PORTEXTRADATA_HEADER_SIZE + name.size() >= sizeof(ZCPP_packet_t) - 4) { // The 4 is a small allowance to ensure we dont blow past the end of the buffer
        ZCPP_packet_t* extraData = new ZCPP_packet_t();
        extraConfigs.push_back(extraData);
        current = extraConfigs.back();
        ZCPPOutput::InitialiseExtraConfigPacket(current, modelsChangeCount, zcpp->GetPriority());
        pos = ZCPP_GetPacketActualSize(*current);
        wxASSERT(pos < sizeof(ZCPP_packet_t)); // check packet has not gone rogue
    }

    current->ExtraData.ports++;
    ZCPP_PortExtraData* p = (ZCPP_PortExtraData*)&current->raw[pos];
    wxASSERT((size_t)p < (size_t)current + sizeof(ZCPP_packet_t) - ZCPP_PORTEXTRADATA_HEADER_SIZE - name.size()); // check pointer has not gone rogue
    p->port = portNum;
    p->string = virtualStringNum;
    int len = std::min(255, (int)name.size());
    p->descriptionLength = len;
    strncpy(p->description, name.c_str(), len);
    spdlog::debug("       Extra : {}/{} '{}'", portNum, virtualStringNum, (const char*)name.c_str());
}

void xLightsFrame::SetModelData(ControllerEthernet* controller, ModelManager* modelManager, OutputManager* outputManager, std::string showDir) {

    spdlog::debug("Setting ZCPP model data");

    auto zcpp = dynamic_cast<ZCPPOutput*>(controller->GetFirstOutput());

    UDController cud(controller, outputManager, modelManager, false);

    long baseStart = zcpp->GetStartChannel();

    spdlog::debug("    Model Change Count : {}", modelsChangeCount);

    ZCPP_packet_t* extraConfig = new ZCPP_packet_t();
    std::list<ZCPP_packet_t*> extraConfigs;
    extraConfigs.push_back(extraConfig);
    auto current = extraConfigs.back();
    ZCPPOutput::InitialiseExtraConfigPacket(current, modelsChangeCount, zcpp->GetPriority());
    ZCPP_packet_t* modelData = new ZCPP_packet_t();
    std::list<ZCPP_packet_t*> modelDatas;
    modelDatas.push_back(modelData);
    auto currentmd = modelDatas.back();
    ZCPPOutput::InitialiseModelDataPacket(currentmd, modelsChangeCount, zcpp->GetPriority(), controller->GetName());

    int index = 0;
    for (int i = 0; i < cud.GetMaxPixelPort(); i++) {
        auto port = cud.GetControllerPixelPort(i + 1);
        port->CreateVirtualStrings(true);

        if (port == nullptr) {
            SetZCPPPort(controller, modelDatas, index, nullptr, i, 0, baseStart, false, zcpp);
            SetZCPPExtraConfig(extraConfigs, i, 0, "", zcpp);
            index++;
        }
        else {
            if (zcpp->IsSupportsVirtualStrings() && port->GetVirtualStringCount() > 0) {
                for (int j = 0; j < port->GetVirtualStringCount(); j++) {
                    int string = j;
                    if (zcpp->IsSupportsSmartRemotes()) {
                        // put the smart remote number in the top 2 bits
                        string += (port->GetVirtualString(j)->_smartRemote << 6);
                    }
                    SetZCPPPort(controller, modelDatas, index, port, i, string, baseStart, false, zcpp);
                    SetZCPPExtraConfig(extraConfigs, i, string, port->GetVirtualString(j)->_description, zcpp);
                    index++;
                }
            }
            else {
                SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, false, zcpp);
                SetZCPPExtraConfig(extraConfigs, i, 0, port->GetPortName(), zcpp);
                index++;
            }
        }
    }

    for (int i = 0; i < cud.GetMaxSerialPort(); i++) {
        auto port = cud.GetControllerSerialPort(i + 1);

        if (port == nullptr) {
            SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, true, zcpp);
            SetZCPPExtraConfig(extraConfigs, 0x80 + i, 0, "", zcpp);
            index++;
        }
        else {
            SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, true, zcpp);
            SetZCPPExtraConfig(extraConfigs, 0x80 + i, 0, port->GetPortName(), zcpp);
            index++;
        }
    }

    //cud.Dump();

    if (extraConfigs.back()->ExtraData.ports == 0) {
        delete extraConfigs.back();
        extraConfigs.pop_back();
    }

    if (modelDatas.back()->Configuration.ports == 0) {
        delete modelDatas.back();
        modelDatas.pop_back();
    }

    if (zcpp->SetModelData(controller, modelDatas, extraConfigs, showDir)) {
        //#ifdef DEBUG
        cud.Dump();
        //#endif

        // dont need to do this as it should have already been done
        //GetOutputManager()->SomethingChanged();
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetModelData");
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetModelData");
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SAVE_NETWORKS, "SetModelData");

        if (_outputManager.IsOutputting()) {
            zcpp->AllOn();
            zcpp->EndFrame(0);
        }
    }
    else {
        // ZCPP OUtput did not accept the new config so we can delete it
        while (extraConfigs.size() > 0) {
            delete extraConfigs.back();
            extraConfigs.pop_back();
        }

        while (modelDatas.size() > 0) {
            delete modelDatas.back();
            modelDatas.pop_back();
        }
    }
}

// This is used to build the ZCPP controller config data that will be needed when it comes time to send data to controllers
bool xLightsFrame::RebuildControllerConfig(OutputManager* outputManager, ModelManager* modelManager) {

    auto logger_work = spdlog::get("work");
    logger_work->debug("        RebuildControllerConfig.");

    for (auto& itc : outputManager->GetControllers()) {
        if (itc->NeedsControllerConfig()) {
            auto eth = dynamic_cast<ControllerEthernet*>(itc);
            if (eth != nullptr && eth->GetProtocol() == OUTPUT_ZCPP) {
                SetModelData(eth, modelManager, outputManager, ToStdString(CurrentDir));
            }
        }
    }

    return true;
}
#pragma endregion
