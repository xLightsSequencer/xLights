
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#ifdef LINUX
#include <arpa/inet.h>
#endif 

#include <wx/config.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "xLightsMain.h"
#include "LayoutPanel.h"
#include "xLightsXmlFile.h"
#include "sequencer/MainSequencer.h"
#include "ViewsModelsPanel.h"
#include "osxMacUtils.h"
#include "UtilFunctions.h"
#include "models/Model.h"
#include "SpecialOptions.h"
#include "LayoutGroup.h"
#include "ControllerVisualiseDialog.h"

#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/Pixlite16.h"
#include "controllers/SanDevices.h"
#include "controllers/J1Sys.h"
#include "controllers/ESPixelStick.h"
#include "controllers/HinksPix.h"
#include "controllers/AlphaPix.h"
#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"

#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/ControllerNull.h"
#include "outputs/Output.h"
#include "outputs/NullOutput.h"
#include "outputs/E131Output.h"
#include "outputs/xxxEthernetOutput.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"
#include "outputs/LOROptimisedOutput.h"

#include <log4cpp/Category.hh>
#include "../xFade/wxLED.h"

const long xLightsFrame::ID_List_Controllers = wxNewId();

const long xLightsFrame::ID_NETWORK_ADDETHERNET = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDNULL = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDSERIAL = wxNewId();
const long xLightsFrame::ID_NETWORK_ACTIVE = wxNewId();
const long xLightsFrame::ID_NETWORK_INACTIVE = wxNewId();
const long xLightsFrame::ID_NETWORK_DELETE = wxNewId();

#pragma region Show Directory
void xLightsFrame::OnMenuMRU(wxCommandEvent& event)
{
    int id = event.GetId();
    wxString newdir = MenuFile->GetLabel(id);
    SetDir(newdir);
}

bool xLightsFrame::SetDir(const wxString& newdir)
{
    static bool HasMenuSeparator = false;
    int idx;

    // don't change show directories with an open sequence because models won't match
    if (!CloseSequence()) {
        return false;
    }

    layoutPanel->ClearSelectedModelGroup();

    // delete any views that were added to the menu
    for (const auto& it : LayoutGroups) {
        LayoutGroup* const grp = dynamic_cast<LayoutGroup* const>(it);
        if (grp != nullptr) {
            RemovePreviewOption(grp);
        }
    }
    PreviewWindows.clear();

    // remove any 3d viewpoints
    viewpoint_mgr.Clear();

    if (newdir != CurrentDir && "" != CurrentDir) {
        wxFileName kbf;
        kbf.AssignDir(CurrentDir);
        kbf.SetFullName("xlights_keybindings.xml");
        mainSequencer->keyBindings.Save(kbf);
    }

    // Check to see if any show directory files need to be saved
    CheckUnsavedChanges();

    // Force update of Preset dialog
    if (EffectTreeDlg != nullptr) {
        delete EffectTreeDlg;
    }
    EffectTreeDlg = nullptr;

    // update most recently used array
    idx = mru.Index(newdir);
    if (idx != wxNOT_FOUND) mru.RemoveAt(idx);
    if (!CurrentDir.IsEmpty())
    {
        idx = mru.Index(CurrentDir);
        if (idx != wxNOT_FOUND) mru.RemoveAt(idx);
        mru.Insert(CurrentDir, 0);
    }
    size_t cnt = mru.GetCount();
    if (cnt > MRU_LENGTH)
    {
        mru.RemoveAt(MRU_LENGTH, cnt - MRU_LENGTH);
        cnt = MRU_LENGTH;
    }

    /*
    wxString msg="UpdateMRU:\n";
    for (int i=0; i<mru.GetCount(); i++) msg+="\n" + mru[i];
    logger_base.debug(msg);
    */

    // save config
    bool DirExists = wxFileName::DirExists(newdir);
    wxString value;
    wxConfigBase* config = wxConfigBase::Get();
    if (DirExists) config->Write(_("LastDir"), newdir);
    for (size_t i = 0; i < MRU_LENGTH; i++)
    {
        wxString mru_name = wxString::Format("mru%d", (int)i);
        if (mru_MenuItem[i] != nullptr)
        {
            Disconnect(mru_MenuItem[i]->GetId(), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
            MenuFile->Delete(mru_MenuItem[i]);
            mru_MenuItem[i] = nullptr;
        }
        if (i < cnt)
        {
            value = mru[i];
        }
        else
        {
            value = wxEmptyString;
        }
        config->Write(mru_name, value);
    }
    //delete config;

    // append mru items to menu
    cnt = mru.GetCount();
    if (!HasMenuSeparator)
    {
        MenuFile->AppendSeparator();
        HasMenuSeparator = true;
    }
    for (size_t i = 0; i < cnt; i++)
    {
        int menuID = wxNewId();
        mru_MenuItem[i] = new wxMenuItem(MenuFile, menuID, mru[i]);
        Connect(menuID, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
        MenuFile->Append(mru_MenuItem[i]);
    }
    MenuFile->UpdateUI();

    if (!DirExists)
    {
        wxString msg = _("The show directory '") + newdir + ("' no longer exists.\nPlease choose a new show directory.");
        DisplayError(msg, this);
        return false;
    }

    ObtainAccessToURL(newdir.ToStdString());

    // update UI
    CheckBoxLightOutput->SetValue(false);
    _outputManager.StopOutput();
    _outputManager.DeleteAllControllers();
    CurrentDir = newdir;
    showDirectory = newdir;

    SpecialOptions::StashShowDir(CurrentDir.ToStdString());
    SpecialOptions::GetOption("", ""); // resets special options

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Show directory set to : %s.", (const char *)showDirectory.c_str());

    if (_logfile != nullptr)
    {
        wxLog::SetActiveTarget(nullptr);
        fclose(_logfile);
        _logfile = nullptr;
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Error);
    }

    if (SpecialOptions::GetOption("wxLogging", "false") == "true")
    {
        _logfile = fopen("wxlog_xlights.txt", "w");
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Debug);
        wxLog::SetActiveTarget(new wxLogStderr(_logfile));
    }
    else
    {
        wxLog::SetActiveTarget(new wxLogStderr()); // write to stderr
    }

    long LinkFlag = 0;
    config->Read(_("LinkFlag"), &LinkFlag);
    if( LinkFlag ) {
        mediaDirectory = CurrentDir;
        config->Write(_("MediaDir"), mediaDirectory);
        logger_base.debug("Media Directory set to : %s.", (const char *)mediaDirectory.c_str());
    }

    long fseqLinkFlag = 0;
    config->Read(_("FSEQLinkFlag"), &fseqLinkFlag);
    if (fseqLinkFlag) {
        fseqDirectory = CurrentDir;
        config->Write(_("FSEQDir"), fseqDirectory);
        logger_base.debug("FSEQ Directory set to : %s.", (const char *)fseqDirectory.c_str());
    }

    while (Notebook1->GetPageCount() > FixedPages)
    {
        Notebook1->DeletePage(FixedPages);
    }

    EnableNetworkChanges();
    DisplayXlightsFilename(wxEmptyString);

    // load network
    networkFile.AssignDir(CurrentDir);
    networkFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
    if (networkFile.FileExists())
    {
        logger_base.debug("Loading networks.");
        wxStopWatch sww;
        if (!_outputManager.Load(CurrentDir.ToStdString()))
        {
            DisplayError(wxString::Format("Unable to load network config %s : Time %ldms", networkFile.GetFullPath(), sww.Time()).ToStdString());
        }
        else
        {
            logger_base.debug("Loaded network config %s : Time %ldms", (const char*)networkFile.GetFullPath().c_str(), sww.Time());
//            SpinCtrl_SyncUniverse->SetValue(_outputManager.GetSyncUniverse());
            InitialiseControllersTab();
        }
    }
    else
    {
        _outputManager.SetShowDir(CurrentDir.ToStdString());
    }

    if (_outputManager.DidConvert())
    {
        NetworkChange();
    }
    else
    {
        UnsavedNetworkChanges = false;
    }

    ShowDirectoryLabel->SetLabel(showDirectory);
    ShowDirectoryLabel->GetParent()->Layout();

    logger_base.debug("Updating networks on setup tab.");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetDir");
    logger_base.debug("    Networks updated.");

    wxFileName kbf;
    kbf.AssignDir(CurrentDir);
    kbf.SetFullName("xlights_keybindings.xml");
    mainSequencer->keyBindings.Load(kbf);

    LoadEffectsFile();

    logger_base.debug("Get start channels right.");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetDir");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "SetDir");
    logger_base.debug("Start channels done.");

    if (mBackupOnLaunch)
    {
        logger_base.debug("Backing up show directory before we do anything this session in this folder : %s.", (const char *)CurrentDir.c_str());
        DoBackup(false, true);
        logger_base.debug("Backup completed.");
    }

    LinkFlag = 0;
    config->Read(_("LinkFlag"), &LinkFlag);
    if (LinkFlag) {
        mediaDirectory = CurrentDir;
        config->Write(_("MediaDir"), mediaDirectory);
        logger_base.debug("Media Directory set to : %s.", (const char *)mediaDirectory.c_str());
    }

    EnableSequenceControls(true);

    Notebook1->ChangeSelection(SETUPTAB);
    SetStatusText("");
    FileNameText->SetLabel(newdir);
    
    if (AllModels.ReworkStartChannel() || UnsavedRgbEffectsChanges)
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "SetDir");
    }

    return true;
}

void xLightsFrame::OnMenuOpenFolderSelected(wxCommandEvent& event)
{
    PromptForShowDirectory();
}

bool xLightsFrame::PromptForShowDirectory()
{
    wxDirDialog DirDialog1(this, _("Select Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    while (DirDialog1.ShowModal() == wxID_OK)
    {
        AbortRender(); // make sure nothing is still rendering
        wxString newdir = DirDialog1.GetPath();
        if (newdir == CurrentDir) return true;

        if (ShowFolderIsInBackup(newdir.ToStdString()))
        {
            DisplayWarning("WARNING: Opening a show folder inside a backup folder. This is ok but please make sure you change back to your proper show folder and dont make changes in this folder.", this);
        }

#ifdef __WXMSW__
        if (ShowFolderIsInProgramFiles(newdir.ToStdString()))
        {
            DisplayWarning("ERROR: Show folder inside your Program Files folder either just wont work or will cause you security issues ... so please choose again.", this);
        }
        else
#endif
        {
            displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr, nullptr, nullptr);
            layoutPanel->ClearUndo();
            return SetDir(newdir);
        }
    }
    return false;
}
#pragma endregion

void xLightsFrame::GetControllerDetailsForChannel(int32_t channel, std::string& controllername, std::string& type, std::string& protocol, std::string& description, int32_t& channeloffset, std::string &ip, std::string& u, std::string& inactive, std::string& baud, int& start_universe, int& start_universe_channel)
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

    if (c != nullptr)
    {
        type = c->GetVMF();
        description = c->GetDescription();
        channeloffset = csc;
        controllername = c->GetName();

        auto eth = dynamic_cast<ControllerEthernet*>(c);
        if (eth != nullptr)
        {
            ip = eth->GetIP();
            u = o->GetUniverseString();
        }

        auto ser = dynamic_cast<ControllerSerial*>(c);
        if (ser != nullptr)
        {
            ip = ser->GetPort();
            baud = wxString::Format("%d", ser->GetSpeed());
        }

        if (!c->IsActive())
        {
            inactive = "DISABLED";
        }
    }

    if (o != nullptr)
    {
        protocol = o->GetType();
        start_universe = o->GetUniverse();
        start_universe_channel = sc;
    }
}

std::string xLightsFrame::GetChannelToControllerMapping(int32_t channel)
{
    int32_t stch;
    Controller* c = _outputManager.GetController(channel, stch);

    if (c != nullptr)
    {
        return c->GetChannelMapping(channel);
    }
    else
    {
        return wxString::Format("Channel %d could not be mapped to a controller.", channel).ToStdString();
    }
}

// reset test channel listbox
void xLightsFrame::UpdateChannelNames()
{
    wxString FormatSpec, RGBFormatSpec;
    int NodeNum;
    size_t ChannelNum, NodeCount, n, c, ChanPerNode;

    ChNames.clear();
    ChNames.resize(_outputManager.GetTotalChannels());
    // update names with RGB models where MyDisplay is checked

    // KW left as some of the conversions seem to use this
    for (auto it = AllModels.begin(); it != AllModels.end(); ++it) {
        Model *model = it->second;
        if (model->GetDisplayAs() != "ModelGroup") {
            NodeCount = model->GetNodeCount();
            ChanPerNode = model->GetChanCountPerNode();
            FormatSpec = "Ch %d: " + model->name + " #%d";
            for (n = 0; n < NodeCount; n++)
            {
                ChannelNum = model->NodeStartChannel(n);

                NodeNum = n + 1;
                if (ChanPerNode == 1)
                {
                    if (ChannelNum < ChNames.Count())
                    {
                        if (ChNames[ChannelNum] == "")
                        {
                            ChNames[ChannelNum] = wxString::Format(FormatSpec, (int)ChannelNum + 1, NodeNum);
                        }
                    }
                }
                else
                {
                    for (c = 0; c < ChanPerNode; c++)
                    {
                        if (ChannelNum < ChNames.Count())
                        {
                            if (ChNames[ChannelNum] == "")
                            {
                                ChNames[ChannelNum] = wxString::Format(FormatSpec, (int)ChannelNum + 1, NodeNum) + model->GetChannelColorLetter(c);
                            }
                        }
                        ChannelNum++;
                    }
                }
            }
        }
    }
}

void xLightsFrame::ActivateSelectedControllers(bool active)
{
    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        auto controller = _outputManager.GetController(List_Controllers->GetItemText(item));
        controller->SetActive(active);

        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ActivateSelectedControllers", nullptr);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ActivateSelectedControllers", nullptr);

        item = List_Controllers->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

#pragma region Move Controllers
void xLightsFrame::MoveSelectedControllerRows(bool up)
{
    auto selected = GetSelectedControllerNames();

    if (up)
    {
        for (const auto& it : selected)
        {
            auto c = _outputManager.GetController(it);
            int index = _outputManager.GetControllerIndex(c);
            if (index != 0)
            {
                _outputManager.MoveController(c, index - 1);
            }
        }
    }
    else
    {
        for (auto it = selected.rbegin(); it != selected.rend(); ++it)
        {
            auto c = _outputManager.GetController(*it);
            int index = _outputManager.GetControllerIndex(c);
            if (index != _outputManager.GetControllerCount() - 1)
            {
                _outputManager.MoveController(c, index + 1);
            }
        }
    }

    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANGE, "MoveSelectedControllerRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "MoveSelectedControllerRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "MoveSelectedControllerRows");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MoveSelectedControllerRows");

    for (const auto& it : selected)
    {
        int index = FindControllerInListControllers(it);
        if (index >= 0)
        {
            List_Controllers->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            List_Controllers->EnsureVisible(index);
        }
    }
}

int xLightsFrame::FindControllerInListControllers(const std::string& name) const
{
    for (int i = 0; i < List_Controllers->GetItemCount(); i++)
    {
        if (List_Controllers->GetItemText(i) == name) return i;
    }
    return -1;
}

void xLightsFrame::OnButtonNetworkMoveUpClick(wxCommandEvent& event)
{
    MoveSelectedControllerRows(true);
}

void xLightsFrame::OnButtonNetworkMoveDownClick(wxCommandEvent& event)
{
    MoveSelectedControllerRows(false);
}

void xLightsFrame::OnListItemBeginDragControllers(wxListEvent& event)
{
    if (!ButtonAddControllerSerial->IsEnabled()) return;

    DragRowIdx = event.GetIndex();	// save the start index
    // do some checks here to make sure valid start
    // ...
    // trigger when user releases left button (drop)
    List_Controllers->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(xLightsFrame::OnListItemDragEndControllers), nullptr,this);
    // trigger when user leaves window to abort drag
    List_Controllers->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(xLightsFrame::OnListItemDragQuitControllers), nullptr,this);
    // trigger when mouse moves
    List_Controllers->Connect(wxEVT_MOTION, wxMouseEventHandler(xLightsFrame::OnListItemMoveControllers), nullptr, this);

    // give visual feedback that we are doing something
    List_Controllers->SetCursor(wxCursor(wxCURSOR_HAND));
}

void xLightsFrame::MoveListControllerRows(int toRow, bool reverse)
{
    auto selected = GetSelectedControllerNames();

    std::list<Controller*> tomove;
    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        if (reverse)
        {
            tomove.push_back(_outputManager.GetControllerIndex(item));
        }
        else
        {
            tomove.push_front(_outputManager.GetControllerIndex(item));
        }
        item = List_Controllers->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
    if (toRow == -1)
    {
        for (const auto& it : selected)
        {
            _outputManager.MoveController(_outputManager.GetController(it), toRow);
        }
    }
    else
    {
        int adjustment = 0;
        if (reverse)
        {
            adjustment = tomove.size() - 2;
        }

        int moved = 0;
        for (const auto& it : tomove)
        {
            _outputManager.MoveController(it, toRow + adjustment);
            moved++;
        }
    }

    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANGE, "MoveControllerRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "MoveControllerRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "MoveControllerRows");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MoveControllerRows");

    for (const auto& it : selected)
    {
        int index = FindControllerInListControllers(it);
        if (index >= 0)
        {
            List_Controllers->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            List_Controllers->EnsureVisible(index);
        }
    }
}

// drop a list item (start row is in DragRowIdx)
void xLightsFrame::OnListItemDragEndControllers(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();  // must reference the event
    int flags = wxLIST_HITTEST_ONITEM;
    long index = List_Controllers->HitTest(pos,flags,nullptr); // got to use it at last
    if(index >= 0 && index != DragRowIdx)
    {
        if (DragRowIdx < index)
        {
            // drag down
            int selected = GetSelectedControllerCount();
            MoveListControllerRows(index - selected + 1, true);
        }
        else
        {
            // drag up
            MoveListControllerRows(index, false);
        }
    }
    else if (index == -1)
    {
        // move to end
        MoveListControllerRows(index, true);
    }

    // restore cursor
    List_Controllers->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    // disconnect both functions
    List_Controllers->Disconnect(wxEVT_LEFT_UP,wxMouseEventHandler(xLightsFrame::OnListItemDragEndControllers));
    List_Controllers->Disconnect(wxEVT_LEAVE_WINDOW,wxMouseEventHandler(xLightsFrame::OnListItemDragQuitControllers));
    List_Controllers->Disconnect(wxEVT_MOTION, wxMouseEventHandler(xLightsFrame::OnListItemMoveControllers));
    DragRowIdx = -1;
    _scrollTimer.Stop();
}

// abort dragging a list item because user has left window
void xLightsFrame::OnListItemDragQuitControllers(wxMouseEvent& event)
{
    _scrollTimer.Stop();
    // restore cursor and disconnect unconditionally
    List_Controllers->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    List_Controllers->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(xLightsFrame::OnListItemDragEndControllers));
    List_Controllers->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(xLightsFrame::OnListItemDragQuitControllers));
    List_Controllers->Disconnect(wxEVT_MOTION, wxMouseEventHandler(xLightsFrame::OnListItemMoveControllers));
    DragRowIdx = -1;
}

void xLightsFrame::OnListItemScrollTimerControllers(wxTimerEvent& event)
{
    if (DragRowIdx >= 0)
    {
        wxMouseEvent* e = new wxMouseEvent();
        wxPostEvent(this, *e);
    }
    else
    {
        _scrollTimer.Stop();
    }
}

void xLightsFrame::OnListItemMoveControllers(wxMouseEvent& event)
{
    if (DragRowIdx < 0) return;

    static int scrollspersec = 2;
    static wxLongLong last = wxGetLocalTimeMillis() - 10000;
    static int lastitem = 99999;

    wxPoint pos = event.GetPosition();  // must reference the event
    int flags = wxLIST_HITTEST_ONITEM;
    long index = List_Controllers->HitTest(pos, flags, nullptr); // got to use it at last

    // dont scroll too fast
    if (lastitem == index && (wxGetLocalTimeMillis() - last).ToLong() < 1000 / scrollspersec)
    {
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
        return;
    }

    lastitem = index;
    last = wxGetLocalTimeMillis();

    int topitem = List_Controllers->GetTopItem();
    int bottomitem = topitem + List_Controllers->GetCountPerPage() - 1;

    if (index >= 0 && index == topitem && topitem != 0)
    {
        // scroll up
        List_Controllers->EnsureVisible(topitem - 1);
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
    }
    else if (index >= 0 && index == bottomitem && bottomitem < List_Controllers->GetItemCount())
    {
        // scroll down
        List_Controllers->EnsureVisible(bottomitem + 1);
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
    }

    // Highlight the row we are dropping on
    for (int i = 0; i < List_Controllers->GetItemCount(); i++)
    {
        List_Controllers->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
    }
    index = List_Controllers->HitTest(pos, flags, nullptr); // got to use it at last
    if (index >= 0)
    {
        List_Controllers->SetItemState(index, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
    }
}
#pragma endregion

void xLightsFrame::NetworkChange()
{
    _outputManager.SomethingChanged();
    UnsavedNetworkChanges = true;
#ifdef __WXOSX__
    ButtonSaveSetup->SetBackgroundColour(wxColour(255, 0, 0));
    ButtonSaveSetup->Refresh();
#else
    ButtonSaveSetup->SetBackgroundColour(wxColour(255, 108, 108));
#endif
}

void xLightsFrame::NetworkChannelsChange()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        NetowrkChannelsChange.");

    _outputManager.SomethingChanged();
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG, "NetworkChannelsChange");
}

bool xLightsFrame::SaveNetworksFile()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        SaveNetworksFile.");

    if (_outputManager.Save()) {
        UnsavedNetworkChanges = false;
#ifdef __WXOSX__
        ButtonSaveSetup->SetBackgroundColour(wxTransparentColour);
        ButtonSaveSetup->Refresh();
#else
        ButtonSaveSetup->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
        return true;
    } else {
        DisplayError(_("Unable to save network definition file"), this);
        return false;
    }
}

void xLightsFrame::OnButtonSaveSetupClick(wxCommandEvent& event)
{
    SaveNetworksFile();
}

void xLightsFrame::SetSyncUniverse(int syncUniverse)
{
    _outputManager.SetSyncUniverse(syncUniverse);
}

void xLightsFrame::UploadFPPProxyOuputs() {
    //SetStatusText("");
    //if (wxMessageBox("This will upload the output UDP configuration for FPP based on controllers with a FPP Proxy set. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES) {
    //    SetCursor(wxCURSOR_WAIT);
    //    wxString ip;
    //    wxString proxy;
    //    std::list<int> selected = GetSelectedOutputs(ip, proxy);
    //    
    //    if (ip == "") {
    //        return;
    //    }
    //    // Recalc all the models to make sure any changes on setup are incorporated
    //    RecalcModels();
    //    
    //    std::set<std::string> done;
    //    for (auto x : selected) {
    //        Output *o = _outputManager.GetOutput(x);
    //        if (o->GetFPPProxyIP() != "" && done.find(o->GetFPPProxyIP()) == done.end()) {
    //            done.emplace(o->GetFPPProxyIP());
    //            FPP fpp(o->GetFPPProxyIP());
    //            fpp.AuthenticateAndUpdateVersions();
    //            fpp.UploadUDPOutputsForProxy(&_outputManager);
    //        }
    //    }
    //    SetCursor(wxCURSOR_ARROW);
    //}

}

void xLightsFrame::PingController(Controller* e)
{
	if (e != nullptr)
	{
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
				DisplayWarning("Controller Status is Unavailible: " + name, this);
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
void xLightsFrame::DoASAPWork()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("Doing ASAP Work.");
    DoWork(_outputModelManager.GetASAPWork(), "ASAP");
}

bool xLightsFrame::DoAllWork()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("Doing All Work.");
    DoWork(_outputModelManager.GetSetupWork(), "Setup");
    DoWork(_outputModelManager.GetLayoutWork(), "Layout");
    DoWork(_outputModelManager.GetASAPWork(), "ASAP");
    return (_outputModelManager.GetASAPWork() == OutputModelManager::WORK_NOTHING && 
            _outputModelManager.GetSetupWork() == OutputModelManager::WORK_NOTHING && 
            _outputModelManager.GetLayoutWork() == OutputModelManager::WORK_NOTHING);
}

void xLightsFrame::DoWork(uint32_t work, const std::string& type, BaseObject* m, const std::string& selected)
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));

    if (work == OutputModelManager::WORK_NOTHING) return;

    std::string selectedModel = selected;
    if (selectedModel == "")
        selectedModel = _outputModelManager.GetSelectedModel();
    if (work & OutputModelManager::WORK_NETWORK_CHANGE)
    {
        logger_work.debug("    WORK_NETWORK_CHANGE.");
        // Mark networks file dirty
        NetworkChange();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_NETWORK_CHANNELSCHANGE |
        OutputModelManager::WORK_UPDATE_NETWORK_LIST |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
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
    if (work & OutputModelManager::WORK_NETWORK_CHANNELSCHANGE)
    {
        logger_work.debug("    WORK_NETWORK_CHANNELSCHANGE.");
        // Recalculates all the channels in the outputs
        NetworkChannelsChange();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_UPDATE_NETWORK_LIST |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
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
    if (work & OutputModelManager::WORK_UPDATE_NETWORK_LIST)
    {
        logger_work.debug("    WORK_UPDATE_NETWORK_LIST.");
        // Updates the list of outputs on the screen
        //UpdateNetworkList();
        InitialiseControllersTab();

        std::string selectedController = _outputModelManager.GetSelectedController();
        if (selectedController != "")
        {
            SelectController(selectedController);
        }
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RGBEFFECTS_CHANGE |
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
    if (work & OutputModelManager::WORK_RGBEFFECTS_CHANGE)
    {
        logger_work.debug("    WORK_RGBEFFECTS_CHANGE.");
        // Mark the rgb effects file as needing to be saved
        MarkEffectsFileDirty();
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
    if (work & OutputModelManager::WORK_RELOAD_MODEL_FROM_XML && !(work & OutputModelManager::WORK_RELOAD_ALLMODELS))
    {
        logger_work.debug("    WORK_RELOAD_MODEL_FROM_XML.");
        BaseObject* mm = m;
        if (mm == nullptr) mm = _outputModelManager.GetModelToReload();
        if (mm != nullptr)
        {
            mm->ReloadModelXml();
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
    if (work & OutputModelManager::WORK_RELOAD_ALLMODELS)
    {
        logger_work.debug("    WORK_RELOAD_ALLMODELS.");
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
    if (work & OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS)
    {
        logger_work.debug("    WORK_MODELS_REWORK_STARTCHANNELS.");
        // Moves all the models around optimally
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
    if (work & OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER)
    {
        logger_work.debug("    WORK_MODELS_CHANGE_REQUIRING_RERENDER.");
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
    if (work & OutputModelManager::WORK_CALCULATE_START_CHANNELS)
    {
        logger_work.debug("    WORK_CALCULATE_START_CHANNELS.");
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
    if (work & OutputModelManager::WORK_RESEND_CONTROLLER_CONFIG)
    {
        logger_work.debug("    WORK_RESEND_CONTROLLER_CONFIG.");
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
    if (work & OutputModelManager::WORK_RELOAD_MODELLIST)
    {
        logger_work.debug("    WORK_RELOAD_MODELLIST.");
        // reload the models list on the layout panel
        layoutPanel->refreshModelList();
        //layoutPanel->ReloadModelList();
        // reload the whole list if no model is selected
        //layoutPanel->UpdateModelList(layoutPanel->GetSelectedModelName() == "");
        //layoutPanel->UpdateModelList(true);
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_OBJECTLIST |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_RELOAD_OBJECTLIST)
    {
        logger_work.debug("    WORK_RELOAD_OBJECTLIST.");
        // reload the objects list on the layout panel
        layoutPanel->refreshObjectList();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW)
    {
        logger_work.debug("    WORK_REDRAW_LAYOUTPREVIEW.");
        // repaint the layout panel
        layoutPanel->UpdatePreview();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_RELOAD_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (selectedModel != "")
    {
        logger_work.debug("    Selecting model '%s'.", (const char*)selectedModel.c_str());
        //SelectModel(selectModel);
        layoutPanel->SelectBaseObject(selectedModel);
    }
    if (work & OutputModelManager::WORK_RELOAD_PROPERTYGRID)
    {
        logger_work.debug("    WORK_RELOAD_PROPERTYGRID.");
        // Reload the property grid either because a value changed and needs to be shown or optional properties should be added or removed
        layoutPanel->resetPropertyGrid();
    }
    work = _outputModelManager.ClearWork(type, work, 
        OutputModelManager::WORK_UPDATE_PROPERTYGRID |
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_UPDATE_PROPERTYGRID)
    {
        logger_work.debug("    WORK_UPDATE_PROPERTYGRID.");
        // Update the property grid mainly enabling and disabling of properties
        layoutPanel->updatePropertyGrid();
    }
    work = _outputModelManager.ClearWork(type, work,
        OutputModelManager::WORK_SAVE_NETWORKS
    );
    if (work & OutputModelManager::WORK_SAVE_NETWORKS)
    {
        logger_work.debug("    WORK_SAVE_NETWORKS.");
        // write the networks file to disk and clears the dirty flag
        SaveNetworksFile();
    }

    // ensure all model groups have all valid model pointers
    AllModels.ResetModelGroups();

    // Keep the selected model for any subsequence ASAP work
    _outputModelManager.SetSelectedModelIfASAPWorkExists(selectedModel);
}

void xLightsFrame::DoLayoutWork()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("Doing Switch To Layout Tab Work.");
    DoWork(_outputModelManager.GetLayoutWork(), "Layout");
}

void xLightsFrame::DoSetupWork()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("Doing Switch To Setup Tab Work.");
    DoWork(_outputModelManager.GetLayoutWork(), "Setup");
}
#pragma endregion

void xLightsFrame::SetE131Sync(bool b)
{
    NetworkChange();
    me131Sync = b;
    _outputManager.SetSyncEnabled(me131Sync);
    if (me131Sync) {
        // recycle output connections if necessary
        if (_outputManager.IsOutputting()) {
            _outputManager.StopOutput();
            _outputManager.StartOutput();
        }
    }
    NetworkChange();
}

void xLightsFrame::EnableNetworkChanges()
{
    bool flag = (!_outputManager.IsOutputting() && !CurrentDir.IsEmpty());
    ButtonAddControllerSerial->Enable(flag);
    ButtonAddControllerEthernet->Enable(flag);
    ButtonAddControllerNull->Enable(flag);
    ButtonDeleteAllControllers->Enable(flag);
    BitmapButtonMoveNetworkUp->Enable(flag);
    BitmapButtonMoveNetworkDown->Enable(flag);
    ButtonDiscover->Enable(flag);
    ButtonSaveSetup->Enable(!CurrentDir.IsEmpty());
    CheckBoxLightOutput->Enable(!CurrentDir.IsEmpty());
    BitmapButtonMoveNetworkDown->Enable(flag);
    BitmapButtonMoveNetworkUp->Enable(flag);
    Panel5->Enable(flag);
}

#pragma region Left Buttons

void xLightsFrame::OnButtonDiscoverClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Running controller discovery.");
    SetStatusText("Running controller discovery ...");
    SetCursor(wxCURSOR_WAIT);

    std::list<std::string> startAddresses;
    std::list<FPP*> instances;
    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        wxString newForce;
        for (auto& a : ips) {
            startAddresses.push_back(a);
        }
    }
    FPP::Discover(startAddresses, instances, true, true);
    std::list<FPP*> consider;
    for (auto fpp : instances) {
        auto controllers = _outputManager.GetControllers(fpp->ipAddress, fpp->hostName);
        if (controllers.size() == 1) {
            auto c = controllers.front();
            if (c->GetName() != fpp->description) {
                if (fpp->description == "") {
                    fpp->SetDescription(c->GetName());
                }
                else if (c->GetName() == "") {
                    // THIS IS NOT POSSIBLE
                    c->SetName(fpp->description);
                }
                else {
                    //FIXME - descriptions aren't equal, ask what to do....
                }
            }
            std::string v, m;
            Controller::ConvertOldTypeToVendorModel(fpp->pixelControllerType, v, m);
            if (c->GetVendor() != v) {
                if (c->GetVendor() == "") {
                    c->SetVendor(v);
                    c->SetModel(m);
                }
                else if (fpp->pixelControllerType == "") {
                    // this will get set later at upload time
                }
                else {
                    //FIXME - controller types don't match, ask what to do
                    //for now, I'll use what FPP is configured for
                    c->SetVendor(v);
                    c->SetModel(m);
                }
            }
            delete fpp;
        }
        else if (controllers.size() > 1) {
            // not sure what to do if multiple outputs match
            delete fpp;
        }
        else {
            consider.push_back(fpp);
        }
    }
    for (auto fpp : consider) {
        ControllerEthernet* controller = nullptr;
        std::string v, m;
        Controller::ConvertOldTypeToVendorModel(fpp->pixelControllerType, v, m);

        if (fpp->pixelControllerType == "") v = "FPP";

        controller = new ControllerEthernet(&_outputManager, false);
        controller->SetProtocol(OUTPUT_DDP);

        if (v == "ESPixelStick") {
            dynamic_cast<DDPOutput*>(controller->GetOutputs().front())->SetKeepChannelNumber(false);
        }

        controller->SetFPPProxy(fpp->proxy);
        if (fpp->proxy == "" && fpp->hostName != "") {
            controller->SetIP(fpp->hostName);
        }
        else {
            controller->SetIP(fpp->ipAddress);
        }
        controller->SetVendor(v);
        controller->SetModel(m);
        if (fpp->description == "") {
            if (fpp->hostName != "") {
                controller->SetName(fpp->hostName);
            }
        }
        else {
            controller->SetName(fpp->description);
        }
        int min = 9999999; int max = 0;
        if (fpp->ranges != "") {
            wxArrayString r1 = wxSplit(wxString(fpp->ranges), ',');
            for (auto a : r1) {
                wxArrayString r = wxSplit(a, '-');
                int start = wxAtoi(r[0]);
                int len = 4; //at least 4
                if (r.size() == 2) {
                    len = wxAtoi(r[1]) - start + 1;
                }
                min = std::min(min, start);
                max = std::max(max, start + len - 1);
            }
        }
        int count = max - min + 1;
        if (count < 512) {
            count = 512;
        }
        controller->GetOutputs().front()->SetChannels(count);
        _outputManager.AddController(controller, -1);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButton_DiscoverClick", nullptr);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButton_DiscoverClick", nullptr);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButton_DiscoverClick", nullptr, controller);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButton_DiscoverClick", nullptr);
        delete fpp;
    }

    std::map<std::string, std::string> renames;
    if (_outputManager.Discover(this, renames))
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButton_DiscoverClick");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButton_DiscoverClick");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButton_DiscoverClick");
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButton_DiscoverClick");

        // update the controller name on any models which use renamed controllers
        for (auto it = renames.begin(); it != renames.end(); ++it)
        {
            logger_base.debug("Discovered controller renamed from '%s' to '%s'", (const char*)it->first.c_str(), (const char*)it->second.c_str());

            for (auto itm = AllModels.begin(); itm != AllModels.end(); ++itm)
            {
                if (itm->second->GetControllerName() == it->first)
                {
                    itm->second->SetControllerName(it->second);
                }
            }
        }
    }
    SetCursor(wxCURSOR_DEFAULT);
    SetStatusText("Discovery complete.");
    logger_base.debug("Controller discovery complete.");
}

void xLightsFrame::OnButtonDeleteAllControllersClick(wxCommandEvent& event)
{
    SelectAllControllers();
    DeleteSelectedControllers();
}

void xLightsFrame::OnButtonAddControllerSerialClick(wxCommandEvent& event)
{
    auto c = new ControllerSerial(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerSerialClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerSerialClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerSerialClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerSerialClick");
}

void xLightsFrame::OnButtonAddControllerEthernetClick(wxCommandEvent& event)
{
    auto c = new ControllerEthernet(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerEthernetClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerEthernetClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerEthernetClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerEthernetClick");
}

void xLightsFrame::OnButtonAddControllerNullClick(wxCommandEvent& event)
{
    auto c = new ControllerNull(&_outputManager);
    _outputManager.AddController(c, -1);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonAddControllerNullClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonAddControllerNullClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonAddControllerNullClick", nullptr, c);
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonAddControllerNullClick");
}

#pragma endregion

void xLightsFrame::InitialiseControllersTab()
{
    // create the checked tree control
    if (List_Controllers == nullptr)
    {
        List_Controllers = new wxListCtrl(Panel2, ID_List_Controllers, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_List_Controllers"));
        FlexGridSizerSetupControllers->Add(List_Controllers, 1, wxALL | wxEXPAND, 5);

        Connect(ID_List_Controllers, wxEVT_LIST_KEY_DOWN, (wxObjectEventFunction)&xLightsFrame::OnListKeyDownControllers);
        //Connect(ID_List_Controllers, wxEVT_LIST_ITEM_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnListItemSelectedControllers);
        Connect(ID_List_Controllers, wxEVT_LIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xLightsFrame::OnListItemActivatedControllers);
        Connect(ID_List_Controllers, wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&xLightsFrame::OnListControllersRClick);
        Connect(ID_List_Controllers, wxEVT_LIST_COL_CLICK, (wxObjectEventFunction)&xLightsFrame::OnListControllersColClick);
        Connect(ID_List_Controllers, wxEVT_LIST_ITEM_RIGHT_CLICK, (wxObjectEventFunction)&xLightsFrame::OnListControllersItemRClick);
        Connect(ID_List_Controllers, wxEVT_LIST_ITEM_FOCUSED, (wxObjectEventFunction)&xLightsFrame::OnListItemFocussedControllers);
        Connect(ID_List_Controllers, wxEVT_LIST_ITEM_DESELECTED, (wxObjectEventFunction)&xLightsFrame::OnListItemDeselectedControllers);
        Connect(ID_List_Controllers, wxEVT_LIST_BEGIN_DRAG, (wxObjectEventFunction)&xLightsFrame::OnListItemBeginDragControllers);

        List_Controllers->AppendColumn("Name");
        List_Controllers->AppendColumn("Protocol");
        List_Controllers->AppendColumn("Address");
        List_Controllers->AppendColumn("Universes/Id");
        List_Controllers->AppendColumn("Channels");
        List_Controllers->AppendColumn("Description");

        ButtonAddControllerEthernet->SetToolTip("Use this button to add E1.31, Artnet, DDP and ZCPP controllers.");
        ButtonAddControllerNull->SetToolTip("Use this button to add channels that you never want to send to a controller.");
        ButtonAddControllerSerial->SetToolTip("Use this button to add typically USB attached Serial/RS485 devices running protocols like DMX, LOR, and Renard.");
        ButtonDiscover->SetToolTip("Probe the network for unlisted controllers.");
    }

    if (Controllers_PropertyEditor == nullptr)
    {
        Controllers_PropertyEditor = new wxPropertyGrid(Panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            // Here are just some of the supported window styles
            //wxPG_AUTO_SORT | // Automatic sorting after items added
            wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
            // Default style
            wxPG_DEFAULT_STYLE);
        FlexGridSizerSetupProperties->Add(Controllers_PropertyEditor, 1, wxALL | wxEXPAND, 5);
#ifdef __WXOSX__
        Controllers_PropertyEditor->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING | wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
#else
        Controllers_PropertyEditor->SetExtraStyle(wxWS_EX_PROCESS_IDLE | wxPG_EX_HELP_AS_TOOLTIPS);
#endif
        Controllers_PropertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&xLightsFrame::OnControllerPropertyGridChange, 0, this);
        Controllers_PropertyEditor->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);
    }

    List_Controllers->Freeze();

    // remember where the list was scrolled to so we can hold its position
    int itemBottom = List_Controllers->GetTopItem() + List_Controllers->GetCountPerPage() - 1;
    int itemSelected = GetFirstSelectedControllerIndex();

    auto selections = GetSelectedControllerNames();

    // Start with an empty List
    List_Controllers->DeleteAllItems();

    // Reload the list
    for (const auto& it : _outputManager.GetControllers())
    {
        int row = List_Controllers->InsertItem(List_Controllers->GetItemCount(), it->GetName());
        if (std::find(begin(selections), end(selections), it->GetName()) != selections.end())
        {
            List_Controllers->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        //List_Controllers->SetItem(row, 1, it->GetColumn6Label());
        List_Controllers->SetItem(row, 1, it->GetColumn1Label());
        List_Controllers->SetItem(row, 2, it->GetColumn2Label());
        List_Controllers->SetItem(row, 3, it->GetColumn3Label());
        List_Controllers->SetItem(row, 4, it->GetColumn4Label());
        List_Controllers->SetItem(row, 5, it->GetColumn5Label());
        if (!it->IsActive())
        {
            List_Controllers->SetItemTextColour(row, *wxLIGHT_GREY);
        }
    }

    auto sz = 0;
    for (int i = 0; i < List_Controllers->GetColumnCount() - 1; i++)
    {
        List_Controllers->SetColumnWidth(i, wxLIST_AUTOSIZE);
        if (List_Controllers->GetColumnWidth(i) < 100) List_Controllers->SetColumnWidth(i, 100);
        sz += List_Controllers->GetColumnWidth(i);
    }

    int lc = List_Controllers->GetColumnCount() - 1;
    List_Controllers->SetColumnWidth(lc, wxLIST_AUTOSIZE);
    if (List_Controllers->GetColumnWidth(lc) < 100) List_Controllers->SetColumnWidth(lc, 100);

    if (sz + List_Controllers->GetColumnWidth(lc) < List_Controllers->GetSize().GetWidth())
    {
        List_Controllers->SetColumnWidth(lc, List_Controllers->GetSize().GetWidth() - sz);
    }
    if (LedPing == nullptr)
    {
        LedPing = new wxLed(Panel5, wxID_ANY);
        FlexGridSizerSetupControllerButtons->Add(LedPing, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        LedPing->Enable();
        LedPing->Show();
        LedPing->SetColor("808080");

        if (StaticTextDummy != nullptr)
        {
            // I remove the static text as this was the only way I seem to be able to make the LED visible
            FlexGridSizerSetupControllerButtons->Detach(StaticTextDummy);
            Panel5->RemoveChild(StaticTextDummy);
            delete StaticTextDummy;
            StaticTextDummy = nullptr;
        }
    }

    // try to ensure what should be visible is visible in roughly the same part of the screen
    if (itemBottom >= List_Controllers->GetItemCount()) itemBottom = List_Controllers->GetItemCount() - 1;
    if (itemBottom != -1) {
        List_Controllers->EnsureVisible(itemBottom);
    }
    if (itemSelected >= List_Controllers->GetItemCount()) itemSelected = List_Controllers->GetItemCount() - 1;
    if (itemSelected != -1) {
        List_Controllers->EnsureVisible(itemSelected);
    }

    List_Controllers->Thaw();
    
    Panel2->Layout();
    Panel5->Layout();
    Layout();

    //CallAfter(&xLightsFrame::SetControllersProperties);
    SetControllersProperties();
}

ControllerCaps* xLightsFrame::GetControllerCaps(const std::string& name)
{
    auto controller = _outputManager.GetController(name);
    if (controller == nullptr) return nullptr;
    return ControllerCaps::GetControllerConfig(controller->GetVendor(), controller->GetModel(), controller->GetFirmwareVersion());
}

#pragma region Controller Properties
void xLightsFrame::SetControllersProperties()
{
    if (GetFirstSelectedControllerIndex() >= 0 && ButtonAddControllerSerial->IsEnabled())
    {
        if (Controllers_PropertyEditor->GetPropertyByName("ControllerName") == nullptr || List_Controllers->GetItemText(GetFirstSelectedControllerIndex()) != Controllers_PropertyEditor->GetPropertyByName("ControllerName")->GetValue().GetString())
        {
            _outputManager.GetController(List_Controllers->GetItemText(GetFirstSelectedControllerIndex()))->AsyncPing();
        }
        BitmapButtonMoveNetworkUp->Enable();
        BitmapButtonMoveNetworkDown->Enable();
    }
    else
    {
        BitmapButtonMoveNetworkUp->Enable(false);
        BitmapButtonMoveNetworkDown->Enable(false);
    }

    Controllers_PropertyEditor->Freeze();

    // save property grid location
    auto save = Controllers_PropertyEditor->SaveEditableState();
    wxString selProp = "";
    if (Controllers_PropertyEditor->GetSelection() != nullptr)
    {
        selProp = Controllers_PropertyEditor->GetSelection()->GetName();
    }

    Controllers_PropertyEditor->Clear();

    auto selections = GetSelectedControllerNames();

    if (selections.size() != 1 || _outputManager.GetController(selections.front()) == nullptr)
    {
        ButtonVisualise->Enable(false);
        ButtonUploadInput->Enable(false);
        ButtonUploadOutput->Enable(false);
        ButtonOpen->Enable(false);
        ButtonControllerDelete->Enable(false);
        LedPing->SetColor("808080");

        wxPGProperty* p = Controllers_PropertyEditor->Append(new wxBoolProperty("Controller Sync", "ControllerSync", me131Sync));
        p->SetEditor("CheckBox");
        p->SetHelpString("Sends a sync packet at the end of each frame for controllers to syncronise light change to. Supported by E1.31, ArtNET and ZCPP. Controller support varies.");

        // nothing selected or many items selected - display global properties
        if (me131Sync)
        {
            p = Controllers_PropertyEditor->Append(new wxUIntProperty("E1.31 Sync Universe", "E131SyncUniverse", _outputManager.GetSyncUniverse()));
            p->SetAttribute("Min", 0);
            p->SetAttribute("Max", 64000);
            p->SetEditor("SpinCtrl");
        }
        p = Controllers_PropertyEditor->Append(new wxUIntProperty("Max Duplicate Frames To Suppress", "MaxSuppressFrames", _outputManager.GetSuppressFrames()));
        p->SetAttribute("Min", 0);
        p->SetAttribute("Max", 1000);
        p->SetEditor("SpinCtrl");

        auto ips = GetLocalIPs();
        wxPGChoices choices;
        int val = 0;
        choices.Add("");
        for (const auto& it : ips) { 
            if (it == mLocalIP) val = choices.GetCount();
            choices.Add(it);
        }

        p = Controllers_PropertyEditor->Append(new wxEnumProperty("Force Local IP", "ForceLocalIP", choices, val));

        p = Controllers_PropertyEditor->Append(new wxStringProperty("Global FPP Proxy", "GlobalFPPProxy", _outputManager.GetGlobalFPPProxy()));
    }
    else if (selections.size() == 1)
    {
        auto controller = _outputManager.GetController(selections.front());
        if (controller != nullptr) {
            int usingip = _outputManager.GetControllerCount(controller->GetType(), controller->GetColumn2Label());

            if (usingip == 1)
            {
                ButtonVisualise->Enable();
            }
            else
            {
                ButtonVisualise->Enable(false);
            }

            auto caps = GetControllerCaps(selections.front());
            if (caps != nullptr && caps->SupportsUpload() && usingip == 1)
            {
                if (caps->SupportsInputOnlyUpload())
                {
                    ButtonUploadInput->Enable();
                }
                else
                {
                    ButtonUploadInput->Enable(false);
                }
                ButtonUploadOutput->Enable();
            }
            else
            {
                ButtonUploadInput->Enable(false);
                ButtonUploadOutput->Enable(false);
            }
            if (dynamic_cast<ControllerEthernet*>(controller) != nullptr)
            {
                ButtonOpen->Enable();
            }
            else
            {
                ButtonOpen->Enable(false);
            }
            ButtonControllerDelete->Enable();

            auto pingresult = controller->GetLastPingState();
            if (pingresult == Output::PINGSTATE::PING_ALLFAILED) {
                LedPing->SetColor("FF0000");
            }
            else if (pingresult == Output::PINGSTATE::PING_UNKNOWN || pingresult == Output::PINGSTATE::PING_UNAVAILABLE) {
                LedPing->SetColor("808000");
            }
            else {
                LedPing->SetColor("00FF00");
            }

            // one item selected - display selected controller properties
            controller->AddProperties(Controllers_PropertyEditor, &AllModels);
        }
    }

    // restore property grid location
    Controllers_PropertyEditor->RestoreEditableState(save);
    if (selProp != "")
    {
        auto p = Controllers_PropertyEditor->GetPropertyByName(selProp);
        if (p != nullptr) Controllers_PropertyEditor->EnsureVisible(p);
    }

    ValidateControllerProperties();

    Controllers_PropertyEditor->Thaw();

    Controllers_PropertyEditor->ExpandAll();
}

void xLightsFrame::ValidateControllerProperties()
{
    auto p = Controllers_PropertyEditor->GetPropertyByName("ControllerName");
    if (p == nullptr)
    {
        // general settings
    }
    else
    {
        auto name = p->GetValue().GetString();
        auto controller = _outputManager.GetController(name);
        // controller settings
        controller->ValidateProperties(&_outputManager, Controllers_PropertyEditor);
    }
}

void xLightsFrame::OnControllerPropertyGridChange(wxPropertyGridEvent& event) {

    wxString name = event.GetPropertyName();
    auto selections = GetSelectedControllerNames();

    if (selections.size() == 1) {
        auto controllername = selections.front();
        auto controller = _outputManager.GetController(controllername);
        auto eth = dynamic_cast<ControllerEthernet*>(controller);

        std::string oldName = controllername;
        std::string oldIP = "";
        if (eth != nullptr) {
            oldIP = eth->GetIP();
        }

        auto processed = controller->HandlePropertyEvent(event, &_outputModelManager);

        if (name == "ControllerName") {
            // it may not have been processed if it would have resulted in a duplicate
            if (processed)
            {
                // change the value immediately otherwise the focus will be lost
                auto c = _outputManager.GetController(event.GetValue().GetString());
                int cn = _outputManager.GetControllerIndex(c);
                List_Controllers->SetItemText(cn, event.GetValue().GetString());

                // This fixes up any start channels dependent on the controller name
                AllModels.RenameController(oldName, event.GetValue().GetString());

                _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "xLightsFrame::OnControllerPropertyGridChange::ControllerName", nullptr);
            }
        }
        else if (name == "IP") {
            // This fixes up any start channels dependent on the controller IP
            if (eth != nullptr && IsIPValid(oldIP) && IsIPValid(eth->GetIP()) && _outputManager.GetControllers(oldIP).size() == 0)
            {
                AllModels.ReplaceIPInStartChannels(oldIP, eth->GetIP());
            }
        }
    }
    else {
        // we handle general properties only
        if (name == "ControllerSync") {
            me131Sync = event.GetValue().GetBool();
            _outputManager.SetSyncEnabled(me131Sync);
            SetControllersProperties();

            if (me131Sync) {
                // recycle output connections if necessary
                if (_outputManager.IsOutputting())
                {
                    _outputManager.StopOutput();
                    _outputManager.StartOutput();
                }
            }
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnControllerPropertyGridChange::ControllerSync");
        }
        else if (name == "E131SyncUniverse")
        {
            _outputManager.SetSyncUniverse((int)event.GetValue().GetLong());
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnControllerPropertyGridChange::E131SyncUniverse");
        }
        else if (name == "MaxSuppressFrames")
        {
            SetSuppressDuplicateFrames((int)event.GetValue().GetLong());
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnControllerPropertyGridChange::MaxSuppressFrames");
        }
        else if (name == "GlobalFPPProxy")
        {
            _outputManager.SetGlobalFPPProxy(event.GetValue().GetString());
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnControllerPropertyGridChange::MaxSuppressFrames");
        }
        else if (name == "ForceLocalIP")
        {
            auto ips = GetLocalIPs();

            if (event.GetValue().GetLong() == 0)
            {
                mLocalIP = "";
            }
            else
            {
                if (event.GetValue().GetLong() >= ips.size())
                {
                    // likely the number of IPs changed after the list was loaded so ignore
                }
                else
                {
                    auto it = begin(ips);
                    std::advance(it, event.GetValue().GetLong() - 1);
                    mLocalIP = *it;
                }
            }

            _outputManager.SetForceFromIP(mLocalIP);
            if (_outputManager.IsOutputting())
            {
                _outputManager.StopOutput();
                _outputManager.StartOutput();
            }
        }
    }

    // Only validate if we are not going to reload the list
    if (!_outputModelManager.IsASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST)) {
        ValidateControllerProperties();
    }
}
#pragma endregion

#pragma region List_Controllers

std::string xLightsFrame::GetFocussedController() const {

    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
    if (item != -1) return List_Controllers->GetItemText(item, 0);
    return "";
}

std::list<std::string> xLightsFrame::GetSelectedControllerNames() const {

    std::list<std::string> selected;
    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1) {
        selected.push_back(List_Controllers->GetItemText(item, 0));
        item = List_Controllers->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    if (selected.size() == 0)
    {
        auto focussed = GetFocussedController();
        if (focussed != "") selected.push_back(focussed);
    }

    return selected;
}

int xLightsFrame::GetFirstSelectedControllerIndex() const
{
    return List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

int xLightsFrame::GetSelectedControllerCount() const
{
    int count = 0;

    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        count++;
        item = List_Controllers->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return count;
}

void xLightsFrame::OnListItemFocussedControllers(wxListEvent& event)
{
    SetControllersProperties();
}

void xLightsFrame::OnListItemActivatedControllers(wxListEvent& event)
{
    auto name = List_Controllers->GetItemText(event.GetItem());
    auto controller = dynamic_cast<ControllerEthernet*>(_outputManager.GetController(name));
    if (controller != nullptr)
    {
        if (controller->GetFPPProxy() != "") {
            ::wxLaunchDefaultBrowser("http://" + controller->GetFPPProxy() + "/proxy/" + controller->GetIP() + "/");
        }
        else {
            ::wxLaunchDefaultBrowser("http://" + controller->GetIP());
        }
    }
}

void xLightsFrame::OnListItemDeselectedControllers(wxListEvent& event)
{
    SetControllersProperties();
}

void xLightsFrame::OnListKeyDownControllers(wxListEvent& event)
{
    wxChar uc = event.GetKeyCode();
    // This is not ideal as it gets the current state which may be different from when the event was generated
    bool ctrl = wxGetKeyState(WXK_CONTROL);

    switch (uc)
    {
    case WXK_ESCAPE:
        UnselectAllControllers();
        break;
    case WXK_DELETE:
        DeleteSelectedControllers();
        break;
    case 'A':
        if (ctrl)
        {
            SelectAllControllers();
        }
        break;
    default:
        break;
    }
}

void xLightsFrame::SelectAllControllers()
{
    for (int i = 0; i < List_Controllers->GetItemCount(); i++)
    {
        List_Controllers->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::DeleteSelectedControllers()
{
    auto selections = GetSelectedControllerNames();

    for (const auto &it : selections)
    {
        _outputManager.DeleteController(it);
    }
    _outputManager.UpdateUnmanaged();
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DeleteSelectedControllers");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "DeleteSelectedControllers");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "DeleteSelectedControllers");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DeleteSelectedControllers");
}

void xLightsFrame::SelectController(const std::string& controllerName)
{
    auto s = GetSelectedControllerNames();

    if (s.size() == 1 && s.front() == controllerName) return;

    UnselectAllControllers();

    int index = FindControllerInListControllers(controllerName);
    if (index >= 0)
    {
        List_Controllers->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        List_Controllers->EnsureVisible(index);
        SetControllersProperties();
    }
}

void xLightsFrame::UnselectAllControllers()
{
    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item >= 0)
    {
        List_Controllers->SetItemState(item, 0, wxLIST_STATE_SELECTED);
        item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    // remove the focus from all items
    item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
    if (item >= 0)
    {
        List_Controllers->SetItemState(item, 0, wxLIST_STATE_FOCUSED);
    }

    SetControllersProperties();
}

void xLightsFrame::OnListControllersColClick(wxListEvent& event)
{
    UnselectAllControllers();
}

void xLightsFrame::OnListControllersRClick(wxContextMenuEvent& event)
{
    if (!ButtonAddControllerSerial->IsEnabled()) return;

    // NOTE: This function is only required if you want a right click menu on an empty list ... right now I dont need that

    List_Controllers->SetFocus();

    int flags;
    //if the click is in the area where there are items, ignore the event and
    //let the normal ItemRClick stuff handle it
    int i = List_Controllers->HitTest(List_Controllers->ScreenToClient(event.GetPosition()), flags);
    if (i >= 0) {
        event.Skip();
        return;
    }

    // If there is an item selected then use the item right click
    if (List_Controllers->GetSelectedItemCount() > 0) {
        wxListEvent e(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK);
        OnListControllersItemRClick(e);
        return;
    }

    //wxMenu mnu;
    //std::string ethernet = "Insert E1.31/ArtNET/ZCPP/DDP";
    //if (SpecialOptions::GetOption("xxx") == "true")
    //{
    //    ethernet += "xxx";
    //}
    //mnu.Append(ID_NETWORK_ADDETHERNET, ethernet)->Enable(ButtonAddControllerSerial->IsEnabled());
    //mnu.Append(ID_NETWORK_ADDNULL, "Insert NULL")->Enable(ButtonAddControllerSerial->IsEnabled());
    //mnu.Append(ID_NETWORK_ADDSERIAL, "Insert DMX/LOR/DLight/Renard")->Enable(ButtonAddControllerSerial->IsEnabled());

    //mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnListControllerPopup, nullptr, this);
    //PopupMenu(&mnu);
    //List_Controllers->SetFocus();
}

void xLightsFrame::OnListControllersItemRClick(wxListEvent& event)
{
    if (!ButtonAddControllerSerial->IsEnabled()) return;

    wxMenu mnu;
    std::string ethernet = "Insert E1.31/ArtNET/ZCPP/DDP";
    if (SpecialOptions::GetOption("xxx") == "true")
    {
        ethernet += "xxx";
    }
    mnu.Append(ID_NETWORK_ADDETHERNET, ethernet)->Enable(ButtonAddControllerSerial->IsEnabled());
    mnu.Append(ID_NETWORK_ADDNULL, "Insert NULL")->Enable(ButtonAddControllerSerial->IsEnabled());
    mnu.Append(ID_NETWORK_ADDSERIAL, "Insert DMX/LOR/DLight/Renard")->Enable(ButtonAddControllerSerial->IsEnabled());
    mnu.Append(ID_NETWORK_ACTIVE, "Activate")->Enable(ButtonAddControllerSerial->IsEnabled());
    mnu.Append(ID_NETWORK_INACTIVE, "Inactivate")->Enable(ButtonAddControllerSerial->IsEnabled());
    mnu.Append(ID_NETWORK_DELETE, "Delete")->Enable(ButtonAddControllerSerial->IsEnabled());

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnListControllerPopup, nullptr, this);
    PopupMenu(&mnu);
    List_Controllers->SetFocus();
}

void xLightsFrame::OnListControllerPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    int item = List_Controllers->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    int selcount = List_Controllers->GetSelectedItemCount();

    if (id == ID_NETWORK_ADDSERIAL) {
        auto c = new ControllerSerial(&_outputManager);
        _outputManager.AddController(c, item);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:ADDSERIAL");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:ADDSERIAL");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:ADDSERIAL", nullptr, c);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:ADDSERIAL");
    }
    else if (id == ID_NETWORK_ADDETHERNET) {
        auto c = new ControllerEthernet(&_outputManager);
        _outputManager.AddController(c, item);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:ADDETHERNET");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:ADDETHERNET", nullptr, c);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:ADDETHERNET");
    }
    else if (id == ID_NETWORK_ADDNULL) {
        auto c = new ControllerNull(&_outputManager);
        _outputManager.AddController(c, item);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:ADDNULL");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:ADDNULL");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:ADDNULL", nullptr, c);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:ADDNULL");
    }
    else if (id == ID_NETWORK_ACTIVE) {
        ActivateSelectedControllers(true);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:DELETE");
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:DELETE");
    }
    else if (id == ID_NETWORK_INACTIVE) {
        ActivateSelectedControllers(false);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:DELETE");
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:DELETE");
    }
    else if (id == ID_NETWORK_DELETE) {
        DeleteSelectedControllers();
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnListControllerPopup:DELETE");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnListControllerPopup:DELETE");
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnListControllerPopup:DELETE");
    }
}
#pragma endregion

#pragma region Selected Controller Actions
void xLightsFrame::OnButtonVisualiseClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // handle right click on an item
    auto name = Controllers_PropertyEditor->GetProperty("ControllerName")->GetValue().GetString();
    auto controller = _outputManager.GetController(name);
    if (controller != nullptr)
    {
        std::string check;
        UDController cud(controller, &_outputManager, &AllModels, check);
        ControllerVisualiseDialog dlg(this, cud, controller->GetColumn2Label(), controller->GetDescription());
        dlg.ShowModal();
    }
    else
    {
        logger_base.debug("OnButtonVisualiseClick unable to get controller.");
    }
}

void xLightsFrame::OnButtonControllerDeleteClick(wxCommandEvent& event)
{
    auto name = Controllers_PropertyEditor->GetProperty("ControllerName")->GetValue().GetString();
    _outputManager.DeleteController(name);
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DeleteSelectedControllers");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "DeleteSelectedControllers");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "DeleteSelectedControllers");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DeleteSelectedControllers");
}

void xLightsFrame::OnButtonOpenClick(wxCommandEvent& event)
{
    auto name = Controllers_PropertyEditor->GetProperty("ControllerName")->GetValue().GetString();
    auto controller = dynamic_cast<ControllerEthernet*>(_outputManager.GetController(name));
    if (controller != nullptr)
    {
        if (controller->GetFPPProxy() != "") {
            ::wxLaunchDefaultBrowser("http://" + controller->GetFPPProxy() + "/proxy/" + controller->GetIP() + "/");
        }
        else {
            ::wxLaunchDefaultBrowser("http://" + controller->GetIP());
        }
    }
}

void xLightsFrame::OnButtonUploadInputClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    SetStatusText("");
    SetCursor(wxCURSOR_WAIT);

    auto name = Controllers_PropertyEditor->GetProperty("ControllerName")->GetValue().GetString();
    logger_base.debug("Uploading controller inputs to" + name);

    auto controller = dynamic_cast<ControllerEthernet*>(_outputManager.GetController(name));
    UploadInputToController(controller);
    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::OnButtonUploadOutputClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    SetStatusText("");
    SetCursor(wxCURSOR_WAIT);

    auto name = Controllers_PropertyEditor->GetProperty("ControllerName")->GetValue().GetString();
    logger_base.debug("Uploading controller outputs to" + name);

    auto controller = dynamic_cast<ControllerEthernet*>(_outputManager.GetController(name));
    UploadOutputToController(controller);
    SetCursor(wxCURSOR_ARROW);
}

void xLightsFrame::UploadInputToController(ControllerEthernet* controller)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (controller == nullptr) return;

    auto caps = GetControllerCaps(controller->GetName());
    if (caps != nullptr)
    {
        caps->Dump();
        if (caps->SupportsInputOnlyUpload())
        {
            auto vendor = controller->GetVendor();
            auto model = controller->GetModel();
            auto ip = controller->GetResolvedIP();
            if (ip == "MULTICAST")
            {
                wxTextEntryDialog dlg(this, "Controller IP Address", "IP Address", ip);
                if (dlg.ShowModal() != wxID_OK) {
                    SetCursor(wxCURSOR_ARROW);
                    return;
                }
                ip = dlg.GetValue();
            }
            auto proxy = controller->GetFPPProxy();
            RecalcModels();

            if (vendor == "Falcon")
            {
                Falcon falcon(ip, proxy);
                if (falcon.IsConnected()) {
                    if (falcon.SetInputUniverses(controller)) {
                        SetStatusText("Falcon Input Upload Complete.");
                    }
                    else {
                        SetStatusText("Falcon Input Upload Failed.");
                    }
                }
                else {
                    SetStatusText("Falcon Input Upload Failed. Unable to connect.");
                }
            }
            else if (vendor == "FPP Generic" && model == "Bridge")
            {
                wxConfigBase* config = wxConfigBase::Get();
                wxString fip;
                config->Read("xLightsPiIP", &fip, "");
                wxString user;
                config->Read("xLightsPiUser", &user, "fpp");
                wxString password;
                config->Read("xLightsPiPassword", &password, "true");

                auto ips = wxSplit(fip, '|');
                auto users = wxSplit(user, '|');
                auto passwords = wxSplit(password, '|');

                // they should all be the same size ... but if not base it off the smallest
                int count = std::min(ips.size(), std::min(users.size(), passwords.size()));

                wxString theUser = "fpp";
                wxString thePassword = "true";
                for (int i = 0; i < count; i++) {
                    if (ips[i] == ip) {
                        theUser = users[i];
                        thePassword = passwords[i];
                    }
                }

                if (thePassword == "true") {
                    if (theUser == "pi") {
                        password = "raspberry";
                    }
                    else if (theUser == "fpp") {
                        password = "falcon";
                    }
                    else {
                        wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
                        if (ted.ShowModal() == wxID_OK) {
                            password = ted.GetValue();
                        }
                    }
                }
                else {
                    wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
                    if (ted.ShowModal() == wxID_OK) {
                        password = ted.GetValue();
                    }
                }

                FPP fpp(ip);
                fpp.parent = this;
                fpp.username = theUser;
                fpp.password = password;
                if (fpp.AuthenticateAndUpdateVersions() && !fpp.SetInputUniversesBridge(controller)) {
                    SetStatusText("FPP Input Upload Complete.");
                }
                else {
                    SetStatusText("FPP Input Upload Failed.");
                }
            }
            else if (vendor == "SanDevices")
            {
                SanDevices sanDevices(ip, proxy);
                if (sanDevices.IsConnected()) {
                    if (sanDevices.SetInputUniverses(controller)) {
                        SetStatusText("SanDevices Input Upload Complete.");
                    }
                    else {
                        SetStatusText("SanDevices Input Upload Failed.");
                    }
                }
                else {
                    SetStatusText("SanDevices Input Upload Failed. Unable to connect.");
                }
            }
            else if (vendor == "HinksPix")
            {
                HinksPix hinkspix(ip, proxy);
                if (hinkspix.IsConnected()) {
                    if (hinkspix.SetInputUniverses(controller)) {
                        SetStatusText("HinksPix Input Upload Complete.");
                    }
                    else {
                        SetStatusText("HinksPix Input Upload Failed.");
                    }
                }
                else {
                    SetStatusText("HinksPix Input Upload Failed. Unable to connect.");
                }
            }
        }
        else
        {
            // This controller does not support uploads
            logger_base.error("Attempt to upload controller inputs on a unsupported controller %s:%s:%s", (const char*)controller->GetVendor().c_str(), (const char*)controller->GetModel().c_str(), (const char*)controller->GetFirmwareVersion().c_str());
            wxASSERT(false);
        }
    }
    else
    {
        logger_base.error("Unable to find controller capabilities info.");
        wxASSERT(false);
    }
}

void xLightsFrame::UploadOutputToController(ControllerEthernet* controller)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (controller == nullptr) return;

    auto caps = GetControllerCaps(controller->GetName());
    if (caps != nullptr)
    {
        caps->Dump();
        if (caps->SupportsUpload())
        {
            auto vendor = controller->GetVendor();
            auto model = controller->GetModel();
            auto ip = controller->GetResolvedIP();
            if (ip == "MULTICAST")
            {
                wxTextEntryDialog dlg(this, "Controller IP Address", "IP Address", ip);
                if (dlg.ShowModal() != wxID_OK) {
                    SetCursor(wxCURSOR_ARROW);
                    return;
                }
                ip = dlg.GetValue();
            }
            auto proxy = controller->GetFPPProxy();
            RecalcModels();

            if (vendor == "Falcon")
            {
                Falcon falcon(ip, proxy);
                if (falcon.IsConnected()) {
                    if (falcon.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("Falcon Output Upload Complete.");
                    }
                    else {
                        SetStatusText("Falcon Output Upload Failed.");
                    }
                }
                else {
                    SetStatusText("Falcon Output Upload Failed. Unable to connect");
                }
            }
            else if (vendor == "Advatek")
            {
                Pixlite16 pixlite(ip);
                if (pixlite.IsConnected()) {
                    if (pixlite.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("PixLite/PixCon Upload Complete.");
                    }
                    else {
                        SetStatusText("PixLite/PixCon Upload Failed.");
                    }
                }
                else
                {
                    SetStatusText("Unable to connect to PixLite/PixCon.");
                }
            }
            else if (vendor == "ESPixelStick")
            {
                ESPixelStick esPixelStick(ip);
                if (esPixelStick.IsConnected()) {
                    if (esPixelStick.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("ES Pixel Stick Upload Complete.");
                    }
                    else {
                        SetStatusText("ES Pixel Stick Upload Failed.");
                    }
                }
                else
                {
                    SetStatusText("ES Pixel Stick Upload Failed. Unable to connect.");
                }
            }
            else if (vendor == "J1Sys")
            {
                J1Sys j1sys(ip, proxy);
                if (j1sys.IsConnected()) {
                    if (j1sys.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("J1SYS Upload Complete.");
                    }
                    else {
                        SetStatusText("J1SYS Upload Failed.");
                    }
                }
                else
                {
                    SetStatusText("J1SYS Upload Failed. Unable to connect.");
                }
            }
            else if (vendor == "SanDevices")
            {
                SanDevices sanDevices(ip, proxy);
                if (sanDevices.IsConnected()) {
                    if (sanDevices.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("SanDevices Output Upload Complete.");
                    }
                    else {
                        SetStatusText("SanDevices Output Upload Failed.");
                    }
                }
                else {
                    SetStatusText("SanDevices Output Upload Failed. Unable to connect to controller.");
                }
            }
            else if (vendor == "HinksPix")
            {
                HinksPix hinkspix(ip, proxy);
                if (hinkspix.IsConnected()) {
                    if (hinkspix.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                        SetStatusText("HinksPix Output Upload Complete.");
                    }
                    else {
                        SetStatusText("HinksPix Output Upload Failed.");
                    }
                }
                else
                {
                    SetStatusText("HinksPix Output Upload Failed. Unable to connect to controller.");
                }
            }
            else if (vendor == "FPP" || vendor == "Kulp")
            {
                    FPP fpp(controller);
                    fpp.parent = this;
                    if (fpp.AuthenticateAndUpdateVersions() && !fpp.UploadPixelOutputs(&AllModels, &_outputManager, controller)) {
                        SetStatusText("FPP Upload Complete.");
                    } else {
                        SetStatusText("FPP Upload Failed.");
                    }
            }
            else if (vendor == "HolidayCoro")
            {
                    AlphaPix alphapix(ip, proxy);
                    if (alphapix.IsConnected()) {
                        if (alphapix.SetOutputs(&AllModels, &_outputManager, controller, this)) {
                            SetStatusText("AlphaPix Upload Complete.");
                        }
                        else {
                            SetStatusText("AlphaPix Upload Failed.");
                        }
                    }
                    else
                    {
                        SetStatusText("Unable to connect to AlphaPix.");
                    }
            }
        }
        else
        {
            logger_base.error("Controller does not support upload.");
        }
    }
    else
    {
        logger_base.error("Unable to find controller capabilities info.");
        wxASSERT(false);
    }
}

#pragma endregion

#pragma region ZCPP

int xLightsFrame::SetZCPPPort(Controller* controller, std::list<ZCPP_packet_t*>& modelDatas, int index, UDControllerPort* port, int portNum, int virtualString, long baseStart, bool isSerial, ZCPPOutput* zcpp)
{
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));

    auto current = modelDatas.back();
    if (current->Configuration.ports >= ZCPP_CONFIG_MAX_PORT_PER_PACKET)
    {
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
    if (port != nullptr && port->GetVirtualStringCount() > 0)
    {
        vs = port->GetVirtualString(vvs);
    }

    logger_zcpp.debug("    Port/String/SmartRemote %d/%d", portNum, vvs, ssr);

    ZCPP_PortConfig* p = current->Configuration.PortConfig + (current->Configuration.ports - 1);
    wxASSERT((size_t)p < (size_t)current + sizeof(ZCPP_packet_t) - sizeof(ZCPP_PortConfig)); // check pointer has not gone rogue

    p->port = portNum | (isSerial ? 0x80 : 0x00);
    p->string = virtualString;
    std::string protocol = "ws2811";
    if (port != nullptr)
    {
        protocol = port->GetProtocol();
    }
    p->protocol = ZCPPOutput::EncodeProtocol(protocol);
    logger_zcpp.debug("       Protocol %d/%s", ZCPPOutput::EncodeProtocol(protocol), (const char*)protocol.c_str());

    int32_t sc = 0;
    if (vs != nullptr)
    {
        sc = vs->_startChannel - baseStart;
    }
    else if (port != nullptr)
    {
        if (port->IsPixelProtocol())
        {
            if (port->GetModels().size() > 0)
            {
                sc = port->GetStartChannel() - baseStart;
            }
        }
        else
        {
            sc = 999999999;
            for (const auto it : port->GetModels())
            {
                // we ignore chained models as they cant be the first
                if (it->GetModel()->GetModelChain() == "" || it->GetModel()->GetModelChain() == "Beginning")
                {
                    sc = std::min(sc, (int32_t)it->GetStartChannel() - (int32_t)baseStart - it->GetDMXChannelOffset() + 1);
                    break;
                }
            }
            if (sc == 999999999) sc = 0;
        }
    }
    if (sc < 0) sc = 0;
    p->startChannel = ntohl(sc);
    logger_zcpp.debug("       Start Channel %d", sc);

    long c = 0;
    if (vs != nullptr)
    {
        c = vs->Channels();
    }
    else if (port != nullptr)
    {
        if (port->IsPixelProtocol())
        {
            if (port->GetModels().size() > 0)
            {
                c = port->Channels();
            }
        }
        else
        {
            for (const auto& it : port->GetModels())
            {
                c = std::max(c, it->GetEndChannel() - sc - baseStart + 1);
            }
        }
    }
    p->channels = ntohl(c);
    logger_zcpp.debug("       Channels %d", c);

    wxByte gc = 1;
    if (vs != nullptr)
    {
        if (vs->_groupCountSet)
        {
            gc = vs->_groupCount;
        }
    }
    else if (m != nullptr)
    {
        gc = m->GetGroupCount(1);
    }
    p->grouping = gc;
    logger_zcpp.debug("       Group Count %d", (int)gc);

    wxByte directionColourOrder = 0x00;
    if (vs != nullptr)
    {
        if (vs->_reverseSet)
        {
            if (vs->_reverse == "Reverse")
            {
                directionColourOrder += 0x80;
            }
        }

        if (vs->_colourOrderSet)
        {
            directionColourOrder += ZCPPOutput::EncodeColourOrder(vs->_colourOrder);
        }
    }
    else if (m != nullptr)
    {
        directionColourOrder += m->GetDirection("Forward") == "Reverse" ? 0x80 : 0x00;
        directionColourOrder += ZCPPOutput::EncodeColourOrder(port->GetFirstModel()->GetColourOrder("RGB"));
    }
    p->directionColourOrder = directionColourOrder;
    logger_zcpp.debug("       Direction/Colour Order %d/%d", (int)directionColourOrder & 0x80, (int)directionColourOrder & 0x7F);

    wxByte np = 0;
    if (vs != nullptr)
    {
        if (vs->_nullPixelsSet)
        {
            np = vs->_nullPixels;
        }
    }
    else if (m != nullptr)
    {
        np = m->GetNullPixels(0);
    }
    p->nullPixels = np;
    logger_zcpp.debug("       Null Pixels %d", (int)np);

    wxByte b = 100;
    if (vs != nullptr)
    {
        if (vs->_brightnessSet)
        {
            b = vs->_brightness;
        }
    }
    else if (m != nullptr)
    {
        b = m->GetBrightness(100);
    }
    p->brightness = b;
    logger_zcpp.debug("       Brightness %d", (int)b);

    wxByte g = 10;
    if (vs != nullptr)
    {
        if (vs->_gammaSet)
        {
            g = vs->_gamma * 10.0;
        }
    }
    else if (m != nullptr)
    {
        g = m->GetGamma(1) * 10.0;
    }
    p->gamma = g;
    logger_zcpp.debug("       Gamma %d", (int)g);

    return sizeof(ZCPP_PortConfig);
}

void xLightsFrame::SetZCPPExtraConfig(std::list<ZCPP_packet_t*>& extraConfigs, int portNum, int virtualStringNum, const std::string& name, ZCPPOutput* zcpp)
{
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    auto current = extraConfigs.back();
    uint16_t pos = ZCPP_GetPacketActualSize(*current);
    wxASSERT(pos < sizeof(ZCPP_packet_t)); // check packet has not gone rogue
    if (pos + ZCPP_PORTEXTRADATA_HEADER_SIZE + name.size() >= sizeof(ZCPP_packet_t) - 4) // The 4 is a small allowance to ensure we dont blow past the end of the buffer
    {
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
    logger_zcpp.debug("       Extra : %d/%d '%s'", portNum, virtualStringNum, (const char*)name.c_str());
}

void xLightsFrame::SetModelData(ControllerEthernet* controller, ModelManager* modelManager, OutputManager* outputManager, std::string showDir)
{
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Setting ZCPP model data");

    auto zcpp = dynamic_cast<ZCPPOutput*>(controller->GetFirstOutput());

    std::string check;
    UDController cud(controller, outputManager, modelManager, check);

    long baseStart = zcpp->GetStartChannel();

    logger_zcpp.debug("    Model Change Count : %d", modelsChangeCount);

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
    for (int i = 0; i < cud.GetMaxPixelPort(); i++)
    {
        auto port = cud.GetControllerPixelPort(i + 1);
        port->CreateVirtualStrings(true);

        if (port == nullptr)
        {
            SetZCPPPort(controller, modelDatas, index, nullptr, i, 0, baseStart, false, zcpp);
            SetZCPPExtraConfig(extraConfigs, i, 0, "", zcpp);
            index++;
        }
        else
        {
            if (zcpp->IsSupportsVirtualStrings() && port->GetVirtualStringCount() > 0)
            {
                for (int j = 0; j < port->GetVirtualStringCount(); j++)
                {
                    int string = j;
                    if (zcpp->IsSupportsSmartRemotes())
                    {
                        // put the smart remote number in the top 2 bits
                        string += (port->GetVirtualString(j)->_smartRemote << 6);
                    }
                    SetZCPPPort(controller, modelDatas, index, port, i, string, baseStart, false, zcpp);
                    SetZCPPExtraConfig(extraConfigs, i, string, port->GetVirtualString(j)->_description, zcpp);
                    index++;
                }
            }
            else
            {
                SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, false, zcpp);
                SetZCPPExtraConfig(extraConfigs, i, 0, port->GetPortName(), zcpp);
                index++;
            }
        }
    }

    for (int i = 0; i < cud.GetMaxSerialPort(); i++)
    {
        auto port = cud.GetControllerSerialPort(i + 1);

        if (port == nullptr)
        {
            SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, true, zcpp);
            SetZCPPExtraConfig(extraConfigs, 0x80 + i, 0, "", zcpp);
            index++;
        }
        else
        {
            SetZCPPPort(controller, modelDatas, index, port, i, 0, baseStart, true, zcpp);
            SetZCPPExtraConfig(extraConfigs, 0x80 + i, 0, port->GetPortName(), zcpp);
            index++;
        }
    }

    //cud.Dump();

    if (extraConfigs.back()->ExtraData.ports == 0)
    {
        delete extraConfigs.back();
        extraConfigs.pop_back();
    }

    if (modelDatas.back()->Configuration.ports == 0)
    {
        delete modelDatas.back();
        modelDatas.pop_back();
    }

    if (zcpp->SetModelData(controller, modelDatas, extraConfigs, showDir))
    {
        //#ifdef DEBUG
        cud.Dump();
        //#endif

        // dont need to do this as it should have already been done
        //GetOutputManager()->SomethingChanged();
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetModelData");
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetModelData");
        //GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_SAVE_NETWORKS, "SetModelData");

        if (_outputManager.IsOutputting())
        {
            zcpp->AllOn();
            zcpp->EndFrame(0);
        }
    }
    else
    {
        // ZCPP OUtput did not accept the new config so we can delete it
        while (extraConfigs.size() > 0)
        {
            delete extraConfigs.back();
            extraConfigs.pop_back();
        }

        while (modelDatas.size() > 0)
        {
            delete modelDatas.back();
            modelDatas.pop_back();
        }
    }
}

// This is used to build the ZCPP controller config data that will be needed when it comes time to send data to controllers
bool xLightsFrame::RebuildControllerConfig(OutputManager* outputManager, ModelManager* modelManager)
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        RebuildControllerConfig.");

    for (auto ito : outputManager->GetControllers())
    {
        if (ito->NeedsControllerConfig())
        {
            auto eth = dynamic_cast<ControllerEthernet*>(ito);
            if (eth != nullptr && eth->GetProtocol() == OUTPUT_ZCPP) {
                SetModelData(eth, modelManager, outputManager, CurrentDir.ToStdString());
            }
        }
    }

    return true;
}

#pragma endregion
