
/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#ifdef LINUX
#include <arpa/inet.h>
#endif 

#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/persist.h>
#include <wx/artprov.h>
#include <wx/regex.h>

#include "xLightsMain.h"
#include "LayoutPanel.h"
#include "xLightsXmlFile.h"
#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/Pixlite16.h"
#include "controllers/SanDevices.h"
#include "controllers/J1Sys.h"
#include "controllers/ESPixelStick.h"
#include "controllers/EasyLights.h"
#include "controllers/ControllerRegistry.h"
#include "sequencer/MainSequencer.h"
#include "ViewsModelsPanel.h"
#include "outputs/Output.h"
#include "outputs/NullOutput.h"
#include "outputs/E131Output.h"
#include "outputs/ZCPPOutput.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"
#include "outputs/LOROptimisedOutput.h"
#include "osxMacUtils.h"
#include "UtilFunctions.h"
#include "models/Model.h"

#include "controllers/ControllerUploadData.h"
#include "MultiControllerUploadDialog.h"
#include "ControllerVisualiseDialog.h"
#include <log4cpp/Category.hh>

const long xLightsFrame::ID_NETWORK_ADDUSB = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDNULL = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDE131 = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDZCPP = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDARTNET = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDLOR = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDDDP = wxNewId();
const long xLightsFrame::ID_NETWORK_BEIPADDR = wxNewId();
const long xLightsFrame::ID_NETWORK_BECHANNELS = wxNewId();
const long xLightsFrame::ID_NETWORK_BEDESCRIPTION = wxNewId();
const long xLightsFrame::ID_NETWORK_BECONTROLLERTYPE = wxNewId();
const long xLightsFrame::ID_NETWORK_BESUPPRESSDUPLICATES = wxNewId();
const long xLightsFrame::ID_NETWORK_BESUPPRESSDUPLICATESYES = wxNewId();
const long xLightsFrame::ID_NETWORK_BESUPPRESSDUPLICATESNO = wxNewId();
const long xLightsFrame::ID_NETWORK_ADD = wxNewId();
const long xLightsFrame::ID_NETWORK_BULKEDIT = wxNewId();
const long xLightsFrame::ID_NETWORK_DELETE = wxNewId();
const long xLightsFrame::ID_NETWORK_ACTIVATE = wxNewId();
const long xLightsFrame::ID_NETWORK_DEACTIVATE = wxNewId();
const long xLightsFrame::ID_NETWORK_DEACTIVATEUNUSED = wxNewId();
const long xLightsFrame::ID_NETWORK_OPENCONTROLLER = wxNewId();
const long xLightsFrame::ID_NETWORK_UPLOADCONTROLLER = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOUTPUT = wxNewId();
const long xLightsFrame::ID_NETWORK_UCINPUT = wxNewId();
const long xLightsFrame::ID_NETWORK_UCIFPPB = wxNewId();
const long xLightsFrame::ID_NETWORK_MULTIUPLOAD = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOFPPB = wxNewId();
const long xLightsFrame::ID_NETWORK_UCIFALCON = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOFALCON = wxNewId();
const long xLightsFrame::ID_NETWORK_UCISANDEVICES = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOSANDEVICES = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOPIXLITE16 = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOJ1SYS = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOESPIXELSTICK = wxNewId();
const long xLightsFrame::ID_NETWORK_PINGCONTROLLER = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOEASYLIGHTS = wxNewId();
const long xLightsFrame::ID_NETWORK_UPLOAD_CONTROLLER_CONFIGURED = wxNewId();
const long xLightsFrame::ID_NETWORK_VISUALISE = wxNewId();
const long xLightsFrame::ID_NETWORK_PROXY_OUTPUT = wxNewId();

const long ID_NETWORK_UCOFPP_PIHAT = wxNewId();

const long ID_NETWORK_UCOFPP_F4B = wxNewId();
const long ID_NETWORK_UCOFPP_F8B = wxNewId();
const long ID_NETWORK_UCOFPP_F8B_16 = wxNewId();
const long ID_NETWORK_UCOFPP_F8B_20 = wxNewId();
const long ID_NETWORK_UCOFPP_F8B_EXP = wxNewId();
const long ID_NETWORK_UCOFPP_F8B_EXP_32 = wxNewId();
const long ID_NETWORK_UCOFPP_F8B_EXP_36 = wxNewId();
const long ID_NETWORK_UCOFPP_F16B = wxNewId();
const long ID_NETWORK_UCOFPP_F16B_32 = wxNewId();
const long ID_NETWORK_UCOFPP_F16B_48 = wxNewId();
const long ID_NETWORK_UCOFPP_F32B = wxNewId();
const long ID_NETWORK_UCOFPP_F32B_48 = wxNewId();
const long ID_NETWORK_UCOFPP_RGBCape24 = wxNewId();
const long ID_NETWORK_UCOFPP_RGBCape48C = wxNewId();
const long ID_NETWORK_UCOFPP_RGBCape48F = wxNewId();
const long ID_NETWORK_UPLOAD_INPUT_CONTROLLER_CONFIGURED = wxNewId();

void CleanupIpAddress(wxString& IpAddr)
{
    static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
    if (leadingzero1.Matches(IpAddr))
    {
        wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
        leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
    }
    static wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
    while (leadingzero2.Matches(IpAddr)) // need to do it several times because the results overlap
    {
        wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
        leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
    }
}

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

    // delete any views that were added to the menu
    for (auto it = LayoutGroups.begin(); it != LayoutGroups.end(); ++it) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
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
    wxMessageBox(msg);
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
    _outputManager.DeleteAllOutputs();
    CurrentDir = newdir;
    showDirectory = newdir;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Show directory set to : %s.", (const char *)showDirectory.c_str());

    if (mBackupOnLaunch)
    {
        logger_base.debug("Backing up show directory before we do anything this session in this folder : %s.", (const char *)CurrentDir.c_str());
        DoBackup(false, true);
        logger_base.debug("Backup completed.");
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
        if (!_outputManager.Load(CurrentDir.ToStdString()))
        {
            DisplayError(wxString::Format("Unable to load network config %s", networkFile.GetFullPath()).ToStdString());
        }
        else
        {
            logger_base.debug("Loaded network config %s", (const char*)networkFile.GetFullPath().c_str());
            SpinCtrl_SyncUniverse->SetValue(_outputManager.GetSyncUniverse());
        }
    }
    else
    {
        _outputManager.SetShowDir(CurrentDir.ToStdString());
    }

    UnsavedNetworkChanges = false;
    ShowDirectoryLabel->SetLabel(showDirectory);
    ShowDirectoryLabel->GetParent()->Layout();

    logger_base.debug("Updating networks on setup tab.");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetDir");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetDir");
    logger_base.debug("    Networks updated.");

    wxFileName kbf;
    kbf.AssignDir(CurrentDir);
    kbf.SetFullName("xlights_keybindings.xml");
    mainSequencer->keyBindings.Load(kbf);

    LoadEffectsFile();

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

void xLightsFrame::GetControllerDetailsForChannel(long channel, std::string& type, std::string& description, long& channeloffset, std::string &ip, std::string& u, std::string& inactive, int& output, std::string& baud)
{
    long ch = 0;
    Output* o = _outputManager.GetLevel1Output(channel, ch);
    channeloffset = ch;

    type = "Unknown";
    description = "";
    ip = "";
    u = "";
    inactive = "";
    baud = "";

    if (o != nullptr)
    {
        type = o->GetType();
        description = o->GetDescription();
        u = o->GetUniverseString();
        if (o->IsIpOutput())
        {
            ip = o->GetIP();
        }
        else
        {
            ip = o->GetCommPort();
            baud = wxString::Format(wxT("%i baud"), o->GetBaudRate()).ToStdString();
        }
        if (o->IsEnabled())
        {
            inactive = "FALSE";
        }
        else
        {
            inactive = "TRUE";
        }
        output = o->GetOutputNumber();
    }
    else
    {
        channeloffset = -1;
        output = -1;
    }
}

std::string xLightsFrame::GetChannelToControllerMapping(long channel)
{
    long stch;
    Output* o = _outputManager.GetLevel1Output(channel, stch);

    if (o != nullptr)
    {
        return o->GetChannelMapping(channel);
    }
    else
    {
        return wxString::Format("Channel %ld could not be mapped to a controller.", channel).ToStdString();
    }
}

void xLightsFrame::UpdateNetworkList()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("        UpdateNetworkList.");

    int item = GridNetwork->GetTopItem() + GridNetwork->GetCountPerPage() - 1;
    int itemselected = GetNetworkSelection();

    auto outputs = _outputManager.GetOutputs();

    GridNetwork->Freeze();
    GridNetwork->DeleteAllItems();

    for (auto e = outputs.begin(); e != outputs.end(); ++e) {
        long newidx = GridNetwork->InsertItem(GridNetwork->GetItemCount(), wxString::Format(wxT("%i"), (*e)->GetOutputNumber()));
        GridNetwork->SetItem(newidx, 1, (*e)->GetType());
        if ((*e)->IsIpOutput()) {
            GridNetwork->SetItem(newidx, 2, (*e)->GetIP());
            GridNetwork->SetItem(newidx, 3, (*e)->GetUniverseString());
        }
        else if ((*e)->GetType() == "NULL") {
            GridNetwork->SetItem(newidx, 3, (*e)->GetUniverseString());
        } else if ((*e)->IsSerialOutput()) {
            GridNetwork->SetItem(newidx, 2, (*e)->GetCommPort() + " : " + (*e)->GetBaudRateString() + " baud");
            GridNetwork->SetItem(newidx, 3, (*e)->GetUniverseString());
        }
        GridNetwork->SetItem(newidx, 4, wxString::Format(wxT("%ld"), (*e)->GetChannels()));
        GridNetwork->SetItem(newidx, 5, wxString::Format(wxT("Channels %ld to %ld"), (*e)->GetStartChannel(), (*e)->GetEndChannel()));
        if ((*e)->IsEnabled()) {
            GridNetwork->SetItem(newidx, 6, "Yes");
        } else {
            GridNetwork->SetItem(newidx, 6, "No");
        }
        GridNetwork->SetItem(newidx, 7, (*e)->GetDescription());
        GridNetwork->SetItem(newidx, 8, (*e)->IsSuppressDuplicateFrames() ? "Y" : "");
        const ControllerRules *rules = ControllerRegistry::GetRulesForController((*e)->GetControllerId());
        if (rules) {
            GridNetwork->SetItem(newidx, 9, rules->GetControllerDescription());
        } else {
            GridNetwork->SetItem(newidx, 9, "");
        }
        GridNetwork->SetItem(newidx, 10, (*e)->GetFPPProxyIP());

        if (!(*e)->IsEnabled()) {
            GridNetwork->SetItemTextColour(newidx, *wxLIGHT_GREY);
        }
    }

    GridNetwork->SetColumnWidth(7, wxLIST_AUTOSIZE);
    GridNetwork->SetColumnWidth(2, _outputManager.GetOutputCount() > 0 ? wxLIST_AUTOSIZE : 100);

    // try to ensure what should be visible is visible in roughly the same part of the screen
    if (item >= GridNetwork->GetItemCount()) item = GridNetwork->GetItemCount() - 1;
    if (item != -1) {
        GridNetwork->EnsureVisible(item);
    }

    if (itemselected >= GridNetwork->GetItemCount()) itemselected = GridNetwork->GetItemCount() - 1;

    if (itemselected != -1) {
        GridNetwork->EnsureVisible(itemselected);
    }

    GridNetwork->Thaw();
    GridNetwork->Refresh();
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
            FormatSpec = "Ch %ld: " + model->name + " #%d";
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

void xLightsFrame::OnMenuOpenFolderSelected(wxCommandEvent& event)
{
    PromptForShowDirectory();
}

bool xLightsFrame::PromptForShowDirectory()
{
    wxDirDialog DirDialog1(this, _("Select Show Directory"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    if (DirDialog1.ShowModal() == wxID_OK)
    {
        AbortRender(); // make sure nothing is still rendering
        wxString newdir = DirDialog1.GetPath();
        if (newdir == CurrentDir) return true;

        if (ShowFolderIsInBackup(newdir.ToStdString()))
        {
            DisplayWarning("WARNING: Opening a show folder inside a backup folder. This is ok but please make sure you change back to your proper show folder and dont make changes in this folder.", this);
        }

        displayElementsPanel->SetSequenceElementsModelsViews(nullptr, nullptr, nullptr, nullptr, nullptr);
        return SetDir(newdir);
    }
    return false;
}

// returns -1 if not found
long xLightsFrame::GetNetworkSelection() const
{
    return GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

long xLightsFrame::GetLastNetworkSelection() const
{
    int selected = -1;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        selected = item;
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return selected;
}

void xLightsFrame::MoveNetworkRows(int toRow, bool reverse)
{
    std::list<Output*> tomove;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        if (reverse)
        {
            tomove.push_back(_outputManager.GetOutput(item));
        }
        else
        {
            tomove.push_front(_outputManager.GetOutput(item));
        }
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
    int adjustment = 0;
    if (reverse)
    {
        adjustment = tomove.size() - 1;
    }

    int moved = 0;
    for (auto it = tomove.begin(); it != tomove.end(); ++it)
    {
        _outputManager.MoveOutput(*it, toRow + adjustment);
        moved++;
    }

    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANGE, "MoveNetworkRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "MoveNetworkRows");
    _outputModelManager.AddImmediateWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "MoveNetworkRows");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MoveNetworkRows");

    for (auto it = tomove.begin(); it != tomove.end(); ++it)
    {
        GridNetwork->SetItemState((*it)->GetOutputNumber()-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        GridNetwork->EnsureVisible((*it)->GetOutputNumber()-1);
    }
}

void xLightsFrame::ChangeSelectedNetwork()
{
    if (!ButtonAddE131->IsEnabled()) return;

    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (item == -1 || GridNetwork->GetSelectedItemCount() != 1)
    {
        DisplayError(_("Please select a single row first"), this);
        return;
    }

    Output* o = _outputManager.GetOutput(item);
    std::string oldDesc = o->GetDescription();
    Output* newoutput = o->Configure(this, &_outputManager, &AllModels);
    if (newoutput != nullptr)
    {
        if (newoutput != o)
        {
            _outputManager.Replace(o, newoutput);
        }
        else
        {
            std::string newDesc = o->GetDescription();
            if (o->IsLookedUpByControllerName() && oldDesc != "" && oldDesc != newDesc)
            {
                AllModels.RenameController(oldDesc, newDesc);
            }
        }
        
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ChangeSelectedNetwork", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "ChangeSelectedNetwork", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ChangeSelectedNetwork", nullptr, newoutput);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "ChangeSelectedNetwork", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "ChangeSelectedNetwork", nullptr, newoutput);
    }
}

void xLightsFrame::OnButtonNetworkChangeClick(wxCommandEvent& event)
{
    ChangeSelectedNetwork();
}

void xLightsFrame::UpdateSelectedIPAddresses()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    Output* o = _outputManager.GetOutput(item);

    wxTextEntryDialog dlg(this, "Change controller IP Address", "IP Address", o->GetIP());
    if (dlg.ShowModal() == wxID_OK)
    {
        if (!IsIPValidOrHostname(dlg.GetValue().ToStdString()) && dlg.GetValue().ToStdString() != "MULTICAST")
        {
            DisplayError("Illegal ip address " + dlg.GetValue().ToStdString(), this);
        }
        else
        {
            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);
                o->SetIP(dlg.GetValue().ToStdString());

                _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateSelectedIPAddresses", nullptr, o);
                _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateSelectedIPAddresses", nullptr, o);
                _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateSelectedIPAddresses", nullptr, o);

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
        }
    }
}

void xLightsFrame::UpdateSelectedSuppressDuplicates(bool suppressDuplicates)
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        Output* o = _outputManager.GetOutput(item);
        o->SetSuppressDuplicateFrames(suppressDuplicates);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateSelectedSuppressDuplicates", nullptr, o);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateSelectedSuppressDuplicates", nullptr, o);

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::UpdateSelectedDescriptions()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    Output* f = _outputManager.GetOutput(item);

    wxTextEntryDialog dlg(this, "Change controller description", "Description", f->GetDescription());
    if (dlg.ShowModal() == wxID_OK)
    {
        while (item != -1)
        {
            Output* o = _outputManager.GetOutput(item);
            std::string oldDesc = o->GetDescription();
            std::string newDesc = dlg.GetValue().ToStdString();
            
            if (o->IsLookedUpByControllerName() && oldDesc != "")
            {
                AllModels.RenameController(oldDesc, newDesc);
            }
            
            o->SetDescription(newDesc);

            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateSelectedDescriptions", nullptr, o);
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateSelectedDescriptions", nullptr, o);
            _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateSelectedDescriptions", nullptr, o);

            item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }
    }
}

void xLightsFrame::UpdateSelectedTypes()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    Output* f = _outputManager.GetOutput(item);

    wxArrayString types;

    int x = 0;
    int idx = 0;
    for (auto& a : ControllerRegistry::GetControllerIds()) {
        const ControllerRules* rules = ControllerRegistry::GetRulesForController(a);
        if (rules && rules->GetSupportedInputProtocols().count(f->GetType()) != 0) {
            types.push_back(rules->GetControllerDescription());
            if (f->GetControllerId() == a) {
                idx = x;
            }
            x++;
        }
    }

    if (types.size() > 0) {
        wxSingleChoiceDialog dlg(this, "Chang controller type", "Type", types);
        dlg.SetSelection(idx);
        if (dlg.ShowModal() == wxID_OK)
        {
            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);
                const ControllerRules* rules = ControllerRegistry::GetRulesForController(dlg.GetStringSelection());
                if (rules && rules->GetSupportedInputProtocols().count(o->GetType()) != 0) {
                    o->SetControllerId(dlg.GetStringSelection());
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateSelectedDescriptions", nullptr, o);
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateSelectedDescriptions", nullptr, o);
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
        }
    }
}

void xLightsFrame::UpdateSelectedChannels()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    Output* f = _outputManager.GetOutput(item);
    wxNumberEntryDialog dlg(this, "Change channels per controller", "Channels", wxEmptyString, f->GetChannels(), 1, f->GetMaxChannels());
    if (dlg.ShowModal() == wxID_OK)
    {
        while (item != -1)
        {
            Output* o = _outputManager.GetOutput(item);
            if (dlg.GetValue() > o->GetMaxChannels())
            {
                DisplayError(wxString::Format("%i Too many channels for %s controller %s. Channels not changed.", dlg.GetValue(), o->GetType(), o->GetDescription()), this);
            }
            else
            {
                if (!o->GetAutoSize())
                {
                    o->SetChannels(dlg.GetValue());

                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "UpdateSelectedChannels", nullptr, o);
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "UpdateSelectedChannels", nullptr, o);
                    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "UpdateSelectedChannels", nullptr, o);
                    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "UpdateSelectedChannels", nullptr, o);
                }
            }

            item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }
    }
}

void xLightsFrame::DeactivateUnusedNetworks()
{
    bool changed = false;

    for (int i = 0; i < GridNetwork->GetItemCount(); i++)
    {
        Output* o = _outputManager.GetOutput(i);
        long st = o->GetStartChannel();
        long en = o->GetEndChannel();
        bool used = false;
        for (auto m = AllModels.begin(); m != AllModels.end(); ++m)
        {
            if (m->second->GetDisplayAs() != "ModelGroup")
            {
                long mst = m->second->GetFirstChannel() + 1;
                long men = m->second->GetLastChannel() + 1;
                if (mst <= en && men >= st)
                {
                    used = true;
                    break;
                }
            }
        }
        if (!used)
        {
            o->Enable(false);
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DeactivateUnusedNetworks", nullptr, o);
            _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "DeactivateUnusedNetworks", nullptr, o);
        }
    }
}

void xLightsFrame::ActivateSelectedNetworks(bool active)
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        Output* o = _outputManager.GetOutput(item);
        o->Enable(active);

        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "ActivateSelectedNetworks", nullptr, o);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "ActivateSelectedNetworks", nullptr, o);

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::DeleteSelectedNetworks()
{
    int removed = 0;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        Output* o = _outputManager.GetOutput(item - removed);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "DeleteSelectedNetworks", nullptr, o);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "DeleteSelectedNetworks", nullptr, o);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "DeleteSelectedNetworks", nullptr, o);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "DeleteSelectedNetworks", nullptr, o);

        _outputManager.DeleteOutput(o);

        removed++;
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::OnButtonNetworkDeleteClick(wxCommandEvent& event)
{
    DeleteSelectedNetworks();
}

void xLightsFrame::OnButtonNetworkDeleteAllClick(wxCommandEvent& event)
{
    _outputManager.DeleteAllOutputs();
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButtonNetworkDeleteAllClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButtonNetworkDeleteAllClick");
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButtonNetworkDeleteAllClick");
    _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButtonNetworkDeleteAllClick");
}

void xLightsFrame::OnButtonNetworkMoveUpClick(wxCommandEvent& event)
{
    long SelectedItem = GetNetworkSelection();
    if (SelectedItem == -1)
    {
        DisplayError(_("Please select a single row first"), this);
        return;
    }
    if (SelectedItem == 0) return;

    MoveNetworkRows(SelectedItem-1, false);
}

void xLightsFrame::OnButtonNetworkMoveDownClick(wxCommandEvent& event)
{
    long SelectedItem = GetLastNetworkSelection();
    if (SelectedItem == -1)
    {
        DisplayError(_("Please select a single row first"), this);
        return;
    }
    if (SelectedItem == GridNetwork->GetItemCount()-1) return;
    int selected = GetNetworkSelectedItemCount();
    MoveNetworkRows(SelectedItem + 1 - selected + 1, true);
}

// drop a list item (start row is in DragRowIdx)
void xLightsFrame::OnGridNetworkDragEnd(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();  // must reference the event
    int flags = wxLIST_HITTEST_ONITEM;
    long index = GridNetwork->HitTest(pos,flags,nullptr); // got to use it at last
    if(index >= 0 && index != DragRowIdx)
    {
        if (DragRowIdx < index)
        {
            // drag down
            int selected = GetNetworkSelectedItemCount();
            MoveNetworkRows(index - selected + 1, true);
        }
        else
        {
            // drag up
            MoveNetworkRows(index, false);
        }
    }

    // restore cursor
    GridNetwork->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    // disconnect both functions
    GridNetwork->Disconnect(wxEVT_LEFT_UP,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd));
    GridNetwork->Disconnect(wxEVT_LEAVE_WINDOW,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit));
    GridNetwork->Disconnect(wxEVT_MOTION,
        wxMouseEventHandler(xLightsFrame::OnGridNetworkMove));
    DragRowIdx = -1;
    _scrollTimer.Stop();
}

void xLightsFrame::OnGridNetworkScrollTimer(wxTimerEvent& event)
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

void xLightsFrame::OnGridNetworkMove(wxMouseEvent& event)
{
    if (DragRowIdx < 0) return;

    static int scrollspersec = 2;
    static wxLongLong last = wxGetLocalTimeMillis() - 10000;
    static int lastitem = 99999;

    wxPoint pos = event.GetPosition();  // must reference the event
    int flags = wxLIST_HITTEST_ONITEM;
    long index = GridNetwork->HitTest(pos, flags, nullptr); // got to use it at last

    // dont scroll too fast
    if (lastitem == index && (wxGetLocalTimeMillis() - last).ToLong() < 1000 / scrollspersec)
    {
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
        return;
    }

    lastitem = index;
    last = wxGetLocalTimeMillis();

    int topitem = GridNetwork->GetTopItem();
    int bottomitem = topitem + GridNetwork->GetCountPerPage() - 1;

    if (index >= 0 && index == topitem && topitem != 0)
    {
        // scroll up
        GridNetwork->EnsureVisible(topitem - 1);
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
    }
    else if (index >= 0 && index == bottomitem && bottomitem < GridNetwork->GetItemCount())
    {
        // scroll down
        GridNetwork->EnsureVisible(bottomitem + 1);
        _scrollTimer.StartOnce(1000 / scrollspersec + 10);
    }
}

void xLightsFrame::OnGridNetworkItemActivated(wxListEvent& event)
{
    ChangeSelectedNetwork();
}

// abort dragging a list item because user has left window
void xLightsFrame::OnGridNetworkDragQuit(wxMouseEvent& event)
{
    _scrollTimer.Stop();
    // restore cursor and disconnect unconditionally
    GridNetwork->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    GridNetwork->Disconnect(wxEVT_LEFT_UP,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd));
    GridNetwork->Disconnect(wxEVT_LEAVE_WINDOW,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit));
    GridNetwork->Disconnect(wxEVT_MOTION,
        wxMouseEventHandler(xLightsFrame::OnGridNetworkMove));
    DragRowIdx = -1;
}

void xLightsFrame::OnGridNetworkBeginDrag(wxListEvent& event)
{
    DragRowIdx = event.GetIndex();	// save the start index
    // do some checks here to make sure valid start
    // ...
    // trigger when user releases left button (drop)
    GridNetwork->Connect(wxEVT_LEFT_UP,
                         wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd), nullptr,this);
    // trigger when user leaves window to abort drag
    GridNetwork->Connect(wxEVT_LEAVE_WINDOW,
                         wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit), nullptr,this);
    // trigger when mouse moves
    GridNetwork->Connect(wxEVT_MOTION,
        wxMouseEventHandler(xLightsFrame::OnGridNetworkMove), nullptr, this);

    // give visual feedback that we are doing something
    GridNetwork->SetCursor(wxCursor(wxCURSOR_HAND));
}

void xLightsFrame::OnButtonAddE131Click(wxCommandEvent& event)
{
    SetupE131(nullptr);
}

void xLightsFrame::OnButton_AddZCPPClick(wxCommandEvent& event)
{
    SetupZCPP(nullptr);
}

void xLightsFrame::OnButtonAddDDPClick(wxCommandEvent& event)
{
    SetupDDP(nullptr);
}
void xLightsFrame::OnButtonArtNETClick(wxCommandEvent& event)
{
    SetupArtNet(nullptr);
}

void xLightsFrame::OnButtonAddNullClick(wxCommandEvent& event)
{
    SetupNullOutput(nullptr);
}

void xLightsFrame::SetupNullOutput(Output* e, int after) {

    Output* null = e;
    if (null == nullptr) null = new NullOutput();
    _outputManager.AddOutput(null, after);

    if (null->Configure(this, &_outputManager, &AllModels) != nullptr)
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupNullOutput", nullptr, null);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupNullOutput", nullptr, null);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupNullOutput", nullptr, null);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupNullOutput", nullptr, null);
    }
    else
    {
        if (e != null)
        {
            _outputManager.DeleteOutput(null);
        }
    }
}

void xLightsFrame::SetupE131(Output* e, int after)
{
    Output* e131 = e;
    if (e131 == nullptr) e131 = new E131Output();
    _outputManager.AddOutput(e131, after);

    if (e131->Configure(this, &_outputManager, &AllModels) != nullptr)
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupE131", nullptr, e131);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupE131", nullptr, e131);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupE131", nullptr, e131);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupE131", nullptr, e131);
    }
    else
    {
        if (e != e131)
        {
            _outputManager.DeleteOutput(e131);
        }
    }
}

void xLightsFrame::SetupZCPP(Output* e, int after)
{
    Output* zcpp = e;
    if (zcpp == nullptr) zcpp = new ZCPPOutput();
    _outputManager.AddOutput(zcpp, after);

    std::string oldDesc = zcpp->GetDescription();

    if (zcpp->Configure(this, &_outputManager, &AllModels) != nullptr)
    {
        std::string newDesc = zcpp->GetDescription();
        if (oldDesc != "" && oldDesc != newDesc)
        {
            AllModels.RenameController(oldDesc, newDesc);
        }

        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupZCPP", nullptr, zcpp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupZCPP", nullptr, zcpp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupZCPP", nullptr, zcpp);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupZCPP", nullptr, zcpp);
    }
    else
    {
        if (e != zcpp)
        {
            _outputManager.DeleteOutput(zcpp);
        }
    }
}

void xLightsFrame::SetupArtNet(Output* e, int after)
{
    Output* artnet = e;
    if (artnet == nullptr) artnet = new ArtNetOutput();
    _outputManager.AddOutput(artnet, after);

    if (artnet->Configure(this, &_outputManager, &AllModels) != nullptr)
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupArtNet", nullptr, artnet);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupArtNet", nullptr, artnet);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupArtNet", nullptr, artnet);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupArtNet", nullptr, artnet);
    }
    else
    {
        if (e != artnet)
        {
            _outputManager.DeleteOutput(artnet);
        }
    }
}

void xLightsFrame::SetupDDP(Output* e, int after)
{
    Output* ddp = e;
    if (ddp == nullptr) ddp = new DDPOutput();
    _outputManager.AddOutput(ddp, after);

    if (ddp->Configure(this, &_outputManager, &AllModels) != nullptr)
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupDDP", nullptr, ddp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupDDP", nullptr, ddp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupDDP", nullptr, ddp);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupDDP", nullptr, ddp);
    }
    else
    {
        if (e != ddp)
        {
            _outputManager.DeleteOutput(ddp);
        }
    }
}

void xLightsFrame::SetupDongle(Output* e, int after)
{
    Output* serial = e;
    if (serial == nullptr) serial = new DMXOutput();
    _outputManager.AddOutput(serial, after);

    Output* newoutput = serial->Configure(this, &_outputManager, &AllModels);

    if (newoutput == nullptr)
    {
        if (e != serial)
        {
            _outputManager.DeleteOutput(serial);
        }
    }
    else if (newoutput != serial)
    {
        _outputManager.Replace(serial, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupDongle", nullptr, newoutput);
    }
    else
    {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupDongle", nullptr, newoutput);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupDongle", nullptr, newoutput);
    }
}

void xLightsFrame::SetupLOR(Output* e, int after)
{
    Output* serial = e;
    if (serial == nullptr) serial = new LOROptimisedOutput();
    _outputManager.AddOutput(serial, after);

    Output* newoutput = serial->Configure(this, &_outputManager, &AllModels);

    if (newoutput == nullptr) {
        if (e != serial) {
            _outputManager.DeleteOutput(serial);
        }
    } else if (newoutput != serial) {
        _outputManager.Replace(serial, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupLOR", nullptr, newoutput);
    } else {
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "SetupLOR", nullptr, newoutput);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "SetupLOR", nullptr, newoutput);
    }
}

void xLightsFrame::OnButtonAddDongleClick(wxCommandEvent& event)
{
    SetupDongle(nullptr);
}

void xLightsFrame::OnButtonAddLORClick(wxCommandEvent& event)
{
    SetupLOR(nullptr);
}

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

void xLightsFrame::OnSpinCtrl_SyncUniverseChange(wxSpinEvent& event)
{
    SetSyncUniverse(SpinCtrl_SyncUniverse->GetValue());
    _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnSpinCtrl_SyncUniverseChange");
}

int xLightsFrame::GetNetworkSelectedItemCount() const
{
    int selected = 0;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1) {
        selected++;
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return selected;
}

bool xLightsFrame::AllSelectedSupportIP()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1) {
        if (!_outputManager.GetOutput(item)->IsIpOutput()) {
            return false;
        }

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return true;
}

static bool CheckAllAreSameIPType(OutputManager &outputManager, wxListCtrl *GridNetwork, bool allowAllMulticasts, bool checkTypes) {
    bool valid = true;
    // check all are multicast or one ip address
    wxString ip;
    wxString type;
    
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    
    while (item != -1) {
        Output* o = outputManager.GetOutput(item);
        
        if (allowAllMulticasts && o->GetIP() == "MULTICAST") {
            //
        } else if (ip != o->GetIP()) {
            if (ip == "") {
                ip = o->GetIP();
            } else {
                valid = false;
                break;
            }
        }
        
        if (checkTypes) {
            if (type == "") {
                type = o->GetType();
            } else {
                if (type != o->GetType()) {
                    valid = false;
                    break;
                }
            }
        }
        
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
    return valid;
}


bool xLightsFrame::AllSelectedSupportChannels()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1) {
        if (!_outputManager.GetOutput(item)->IsOutputable()) {
            return false;
        }

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return true;
}

void xLightsFrame::OnGridNetworkItemRClick(wxListEvent& event)
{
    GridNetwork->SetFocus();

    int selcnt = GridNetwork->GetSelectedItemCount();

    wxMenu mnu;
    wxMenu* mnuAdd = new wxMenu();
    mnuAdd->Append(ID_NETWORK_ADDUSB, "USB")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDNULL, "NULL")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDE131, "E1.31")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDARTNET, "ArtNET")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDLOR, "LOR")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDDDP, "DDP")->Enable(selcnt == 1);
    mnuAdd->Append(ID_NETWORK_ADDZCPP, "ZCPP")->Enable(selcnt == 1);
    mnuAdd->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    
    bool validIpNoType = CheckAllAreSameIPType(_outputManager, GridNetwork, true, false);
    bool allSupportIp = AllSelectedSupportIP();
    bool doEnable = allSupportIp && (selcnt == 1 || validIpNoType);
    bool hasControllerConfigured = false;
    bool hasProxyConfigured = false;
    Output *selected = nullptr;
    if (selcnt == 1) {
        int idx = GridNetwork->GetNextItem(-1,
                                           wxLIST_NEXT_ALL,
                                           wxLIST_STATE_SELECTED);
        selected = _outputManager.GetOutput(idx);
        if (idx >= 0 && selected) {
            if (selected->GetControllerId() != "") {
                hasControllerConfigured = true;
            }
            if (selected->GetFPPProxyIP() != "") {
                hasProxyConfigured = true;
            }
        }
    }
    
    wxMenu* mnuUploadController = new wxMenu();

    wxMenuItem* beMultiUpload = mnuUploadController->Append(ID_NETWORK_MULTIUPLOAD, "Upload To All Controllers");
    if (_outputManager.GetOutputCount() > 0 && _outputManager.GetIps().size() > 0) {
        beMultiUpload->Enable();
    }
    else {
        beMultiUpload->Enable(false);
    }

    if (hasControllerConfigured) {
        auto rules = ControllerRegistry::GetRulesForController(selected->GetControllerId());
        if (selected->GetType() == OUTPUT_E131 && rules->GetControllerManufacturer() != "ESPixelStick") {
            std::string description = rules->GetControllerDescription();
            mnuUploadController->Append(ID_NETWORK_UPLOAD_INPUT_CONTROLLER_CONFIGURED, "E1.31 Input Definition - " + description);
        }
    } else {
        wxMenu* mnuUCInput = new wxMenu();
        wxMenuItem* beUCIFPPB = mnuUCInput->Append(ID_NETWORK_UCIFPPB, "FPP Bridge Mode");
        if (!AllSelectedSupportIP()) {
            beUCIFPPB->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCIFPPB->Enable(true);
            } else {
                beUCIFPPB->Enable(validIpNoType);
            }
        }

        wxMenuItem* beUCIFalcon = mnuUCInput->Append(ID_NETWORK_UCIFALCON, "Falcon");
        if (!AllSelectedSupportIP()) {
            beUCIFalcon->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCIFalcon->Enable(true);
            } else {
                beUCIFalcon->Enable(validIpNoType);
            }
        }

        wxMenuItem* beUCISanDevices = mnuUCInput->Append(ID_NETWORK_UCISANDEVICES, "SanDevices");
        if (!AllSelectedSupportIP()) {
            beUCISanDevices->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCISanDevices->Enable(true);
            } else {
                bool valid = CheckAllAreSameIPType(_outputManager, GridNetwork, false, true);
                beUCISanDevices->Enable(valid);
            }
        }

        mnuUploadController->Append(ID_NETWORK_UCINPUT, "E1.31 Input Definition", mnuUCInput, "");
        mnuUCInput->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);
    }
    if (hasControllerConfigured) {
        auto rules = ControllerRegistry::GetRulesForController(selected->GetControllerId());
        std::string description = rules->GetControllerDescription();
        if (rules->GetControllerManufacturer() == "ESPixelStick") {
            //ESPixelStick uploads inputs and outputs together as one
            mnuUploadController->Append(ID_NETWORK_UPLOAD_CONTROLLER_CONFIGURED, description);
        } else {
            mnuUploadController->Append(ID_NETWORK_UPLOAD_CONTROLLER_CONFIGURED, "Output - " + description);
        }
    } else {
        wxMenu* mnuUCOutput = new wxMenu();
        wxMenuItem* beUCOFalcon = mnuUCOutput->Append(ID_NETWORK_UCOFALCON, "Falcon");
        if (!AllSelectedSupportIP()) {
            beUCOFalcon->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCOFalcon->Enable(true);
            } else {
                beUCOFalcon->Enable(validIpNoType);
            }
        }

        wxMenuItem* beUCOPixlite16 = mnuUCOutput->Append(ID_NETWORK_UCOPIXLITE16, "PixLite/PixCon");
        if (!AllSelectedSupportIP()) {
            beUCOPixlite16->Enable(false);
        } else  {
            if (selcnt == 1) {
                beUCOPixlite16->Enable(true);
            } else {
                beUCOPixlite16->Enable(validIpNoType);
            }
        }

        wxMenuItem* beUCOSanDevices = mnuUCOutput->Append(ID_NETWORK_UCOSANDEVICES, "SanDevices");
        if (!AllSelectedSupportIP()) {
            beUCOSanDevices->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCOSanDevices->Enable(true);
            } else {
                bool valid = CheckAllAreSameIPType(_outputManager, GridNetwork, false, true);
                beUCOSanDevices->Enable(valid);
            }
        }


        wxMenuItem* beUCOJ1SYS = mnuUCOutput->Append(ID_NETWORK_UCOJ1SYS, "J1SYS");
        if (!AllSelectedSupportIP()) {
            beUCOJ1SYS->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCOJ1SYS->Enable(true);
            } else {
                bool valid = CheckAllAreSameIPType(_outputManager, GridNetwork, true, true);
                beUCOJ1SYS->Enable(valid);
            }
        }

        wxMenuItem* beUCOEasyLights = mnuUCOutput->Append(ID_NETWORK_UCOEASYLIGHTS, "EasyLights");
        if(!AllSelectedSupportIP()) {
            beUCOEasyLights->Enable(false);
        }
        else {
            if(selcnt == 1) {
                beUCOEasyLights->Enable(true);
            }
            else {
                bool valid = CheckAllAreSameIPType(_outputManager, GridNetwork, false, true);
                beUCOEasyLights->Enable(valid);
            }
        }

        wxMenu* fppOutput = new wxMenu();
        mnuUCOutput->AppendSubMenu(fppOutput, "FPP Capes/Hats");
        fppOutput->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);
        wxMenuItem* item = fppOutput->Append(ID_NETWORK_UCOFPP_F4B, "F4-B");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B, "F8-B (8 serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B_16, "F8-B (4 serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B_20, "F8-B (No serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B_EXP, "F8-B w/ Expansion (8 serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B_EXP_32, "F8-B w/ Expansion (4 serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F8B_EXP_36, "F8-B w/ Expansion (No serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F16B, "F16-B");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F16B_32, "F16-B w/ 32 outputs");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F16B_48, "F16-B w/ 48 outputs (No Serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F32B, "F32-B");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_F32B_48, "F32-B (No Serial)");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_RGBCape24, "RGBCape24");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_RGBCape48C, "RGBCape48C");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_RGBCape48F, "RGBCape48F");
        item->Enable(doEnable);
        item = fppOutput->Append(ID_NETWORK_UCOFPP_PIHAT, "PiHat");
        item->Enable(doEnable);

        wxMenuItem* beUCOESPixelStick = mnuUCOutput->Append(ID_NETWORK_UCOESPIXELSTICK, "ES Pixel Stick");
        if (!AllSelectedSupportIP()) {
            beUCOESPixelStick->Enable(false);
        } else {
            if (selcnt == 1) {
                beUCOESPixelStick->Enable(true);
            } else {
                bool valid = CheckAllAreSameIPType(_outputManager, GridNetwork, true, true);
                beUCOESPixelStick->Enable(valid);
            }
        }

        mnuUploadController->Append(ID_NETWORK_UCOUTPUT, "Output", mnuUCOutput, "");
        mnuUCOutput->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);
    }
    if (hasProxyConfigured) {
        mnuUploadController->Append(ID_NETWORK_PROXY_OUTPUT, "Upload FPP Proxy Outputs");
    }
    
    mnu.Append(ID_NETWORK_UPLOADCONTROLLER, "Upload To Controller", mnuUploadController, "");
    mnuUploadController->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    wxMenu* mnuBulkEdit = new wxMenu();
    wxMenuItem* beip = mnuBulkEdit->Append(ID_NETWORK_BEIPADDR, "IP Address");
    beip->Enable(selcnt > 0);
    if (!AllSelectedSupportIP()) {
        beip->Enable(false);
    }
    wxMenuItem* bech = mnuBulkEdit->Append(ID_NETWORK_BECHANNELS, "Channels");
    bech->Enable(selcnt > 0);
    if (!AllSelectedSupportChannels()) {
        bech->Enable(false);
    }
    mnuBulkEdit->Append(ID_NETWORK_BECONTROLLERTYPE, "Type")->Enable(selcnt > 0);
    mnuBulkEdit->Append(ID_NETWORK_BEDESCRIPTION, "Description")->Enable(selcnt > 0);

    wxMenu* mnuBulkEditSD = new wxMenu();
    mnuBulkEditSD->Append(ID_NETWORK_BESUPPRESSDUPLICATESYES, "Yes")->Enable(selcnt > 0);
    mnuBulkEditSD->Append(ID_NETWORK_BESUPPRESSDUPLICATESNO, "No")->Enable(selcnt > 0);
    mnuBulkEditSD->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    mnuBulkEdit->Append(ID_NETWORK_BESUPPRESSDUPLICATES, "Suppress duplicate frames", mnuBulkEditSD, "")->Enable(selcnt > 0);
    mnuBulkEdit->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    wxMenuItem* ma =  mnu.Append(ID_NETWORK_ADD, "Insert After", mnuAdd, "");
    wxMenuItem* be = mnu.Append(ID_NETWORK_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
    mnu.AppendSeparator();

    if (!ButtonAddE131->IsEnabled()) {
        ma->Enable(false);
        be->Enable(false);
    }

    wxMenuItem* mid = mnu.Append(ID_NETWORK_DELETE, "Delete");
    wxMenuItem* mia = mnu.Append(ID_NETWORK_ACTIVATE, "Activate");
    wxMenuItem* mide = mnu.Append(ID_NETWORK_DEACTIVATE, "Deactivate");
    wxMenuItem* mideu = mnu.Append(ID_NETWORK_DEACTIVATEUNUSED, "Deactivate Unused");
    wxMenuItem* oc = mnu.Append(ID_NETWORK_OPENCONTROLLER, "Open Controller");
    wxMenuItem* pc = mnu.Append(ID_NETWORK_PINGCONTROLLER, "Ping Controller");
    if (hasControllerConfigured) {
        mnu.Append(ID_NETWORK_VISUALISE, "Visualise Controller");
    }

    mideu->Enable(true);
    mid->Enable(selcnt > 0);
    mia->Enable(selcnt > 0);
    mide->Enable(selcnt > 0);

    oc->Enable(selcnt == 1);

    if (!AllSelectedSupportIP()) {
        oc->Enable(false);
    } else {
        int item2 = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item2 != -1) {
            Output* o = _outputManager.GetOutput(item2);
            if (o->GetIP() == "MULTICAST") {
                oc->Enable(false);
            }
        }
    }

	pc->Enable(false);
	if (selcnt == 1) {
		int item2 = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item2 != -1) {
			Output* o = _outputManager.GetOutput(item2);
			if (o->CanPing()) {
				pc->Enable(true);
			}
		}
	}

    if (!ButtonAddE131->IsEnabled()) {
        mia->Enable(false);
        mid->Enable(false);
        mide->Enable(false);
        mideu->Enable(false);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);
    PopupMenu(&mnu);
    GridNetwork->SetFocus();
}

void xLightsFrame::OnNetworkPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (id == ID_NETWORK_ADDUSB) {
        SetupDongle(nullptr, item+1);
    } else if (id == ID_NETWORK_ADDNULL) {
        SetupNullOutput(nullptr, item+1);
    } else if (id == ID_NETWORK_ADDE131) {
        SetupE131(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDZCPP)
    {
        SetupZCPP(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDARTNET)
    {
        SetupArtNet(nullptr, item+1);
    } else if (id == ID_NETWORK_ADDLOR) {
        SetupLOR(nullptr, item+1);
    } else if (id == ID_NETWORK_ADDDDP) {
        SetupDDP(nullptr, item + 1);
    } else if (id == ID_NETWORK_UCIFPPB) {
        UploadFPPBridgeInput();
    } else if (id == ID_NETWORK_MULTIUPLOAD) {
        MultiControllerUpload();
    } else if (id == ID_NETWORK_UCOFPPB) {
        UploadFPPBridgeOutput();
    } else if (id == ID_NETWORK_UCIFALCON) {
        UploadFalconInput();
    } else if (id == ID_NETWORK_UCOFALCON) {
        UploadFalconOutput();
    } else if (id == ID_NETWORK_UCISANDEVICES) {
        UploadSanDevicesInput();
    } else if (id == ID_NETWORK_UCOSANDEVICES) {
        UploadSanDevicesOutput();
    } else if (id == ID_NETWORK_UCOJ1SYS) {
        UploadJ1SYSOutput();
    } else if (id == ID_NETWORK_UCOESPIXELSTICK) {
        UploadESPixelStickOutput();
    } else if (id == ID_NETWORK_UCOFPP_F4B) {
        UploadFPPStringOuputs("F4-B");
    } else if (id == ID_NETWORK_UCOFPP_F8B) {
        UploadFPPStringOuputs("F8-B");
    } else if (id == ID_NETWORK_UCOFPP_F8B_16) {
        UploadFPPStringOuputs("F8-B-16");
    } else if (id == ID_NETWORK_UCOFPP_F8B_20) {
        UploadFPPStringOuputs("F8-B-20");
    } else if (id == ID_NETWORK_UCOFPP_F8B_EXP) {
        UploadFPPStringOuputs("F8-B-EXP");
    } else if (id == ID_NETWORK_UCOFPP_F8B_EXP_32) {
        UploadFPPStringOuputs("F8-B-EXP-32");
    } else if (id == ID_NETWORK_UCOFPP_F8B_EXP_36) {
        UploadFPPStringOuputs("F8-B-EXP-36");
    } else if (id == ID_NETWORK_UCOFPP_F16B) {
        UploadFPPStringOuputs("F16-B");
    } else if (id == ID_NETWORK_UCOFPP_F16B_32) {
        UploadFPPStringOuputs("F16-B-32");
    } else if (id == ID_NETWORK_UCOFPP_F16B_48) {
        UploadFPPStringOuputs("F16-B-48");
    } else if (id == ID_NETWORK_UCOFPP_F32B) {
        UploadFPPStringOuputs("F32-B");
    } else if (id == ID_NETWORK_UCOFPP_F32B_48) {
        UploadFPPStringOuputs("F32-B-48");
    } else if (id == ID_NETWORK_UCOFPP_RGBCape24) {
        UploadFPPStringOuputs("RGBCape24");
    } else if (id == ID_NETWORK_UCOFPP_RGBCape48C) {
        UploadFPPStringOuputs("RGBCape48C");
    } else if (id == ID_NETWORK_UCOFPP_RGBCape48F) {
        UploadFPPStringOuputs("RGBCape48F");
    } else if (id == ID_NETWORK_UCOFPP_PIHAT) {
        UploadFPPStringOuputs("PiHat");
    } else if (id == ID_NETWORK_UCOPIXLITE16) {
        UploadPixlite16Output();
    } else if (id == ID_NETWORK_PROXY_OUTPUT) {
        UploadFPPProxyOuputs();
    } else if (id == ID_NETWORK_UPLOAD_INPUT_CONTROLLER_CONFIGURED) {
        Output *selected = _outputManager.GetOutput(item);
        const ControllerRules * rules = ControllerRegistry::GetRulesForController(selected->GetControllerId());
        if (rules->GetControllerManufacturer() == "FPP") {
            UploadFPPBridgeInput();
        } else if (rules->GetControllerManufacturer() == "Falcon") {
            UploadFalconInput();
        } else if (rules->GetControllerManufacturer() == "ESPixelStick") {
            UploadESPixelStickOutput();
        } else if (rules->GetControllerManufacturer() == "SanDevices") {
            UploadSanDevicesInput();
        }
        //FIXME - other targets
    } else if (id == ID_NETWORK_UPLOAD_CONTROLLER_CONFIGURED) {
        Output *selected = _outputManager.GetOutput(item);
        const ControllerRules * rules = ControllerRegistry::GetRulesForController(selected->GetControllerId());
        if (rules->GetControllerManufacturer() == "FPP") {
            UploadFPPStringOuputs(rules->GetControllerId());
        } else if (rules->GetControllerManufacturer() == "Falcon") {
            UploadFalconOutput();
        } else if (rules->GetControllerManufacturer() == "ESPixelStick") {
            UploadESPixelStickOutput();
        } else if (rules->GetControllerManufacturer() == "SanDevices") {
            UploadSanDevicesOutput();
        }
        //FIXME - other targets
    } else if (id == ID_NETWORK_VISUALISE) {
        Output *selected = _outputManager.GetOutput(item);
        VisualiseOutput(selected);
    } else if (id == ID_NETWORK_BEIPADDR) {
        UpdateSelectedIPAddresses();
    } else if (id == ID_NETWORK_BECHANNELS) {
        UpdateSelectedChannels();
    }
    else if (id == ID_NETWORK_BEDESCRIPTION) {
    UpdateSelectedDescriptions();
    }
    else if (id == ID_NETWORK_BECONTROLLERTYPE) {
    UpdateSelectedTypes();
    } else if (id == ID_NETWORK_BESUPPRESSDUPLICATESYES) {
        UpdateSelectedSuppressDuplicates(true);
    } else if (id == ID_NETWORK_BESUPPRESSDUPLICATESNO) {
        UpdateSelectedSuppressDuplicates(false);
    } else if (id == ID_NETWORK_DELETE) {
        DeleteSelectedNetworks();
    } else if (id == ID_NETWORK_ACTIVATE) {
        ActivateSelectedNetworks(true);
    } else if (id == ID_NETWORK_DEACTIVATE) {
        ActivateSelectedNetworks(false);
    } else if (id == ID_NETWORK_DEACTIVATEUNUSED) {
        DeactivateUnusedNetworks();
    } else if (id == ID_NETWORK_OPENCONTROLLER) {
        Output* o = _outputManager.GetOutput(item);
        if (o != nullptr) {
            ::wxLaunchDefaultBrowser("http://" + o->GetIP());
        }
    } else if (id == ID_NETWORK_PINGCONTROLLER) {
        Output* o = _outputManager.GetOutput(item);
        PingController(o);
	} else if(id == ID_NETWORK_UCOEASYLIGHTS) {
	 	UploadEasyLightsOutput();
	}
}

void xLightsFrame::OnGridNetworkItemSelect(wxListEvent& event)
{
}

void xLightsFrame::OnGridNetworkItemDeselect(wxListEvent& event)
{
}

void xLightsFrame::OnGridNetworkItemFocused(wxListEvent& event)
{
}

void xLightsFrame::OnGridNetworkKeyDown(wxListEvent& event)
{
    wxChar uc = event.GetKeyCode();
    switch (uc)
    {
    case WXK_DELETE:
        if (GridNetwork->GetSelectedItemCount() > 0) {
            DeleteSelectedNetworks();
        }
        break;
    case 'A':
        if (::wxGetKeyState(WXK_CONTROL)) {
            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL);
            while (item != -1) {
                GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL);
            }
        }
        break;
    default:
        break;
    }
}
std::list<int> xLightsFrame::GetSelectedOutputs(wxString& ip) {
    wxString p;
    return GetSelectedOutputs(ip, p);
}

std::list<int> xLightsFrame::GetSelectedOutputs(wxString& ip, wxString& proxy)
{
    std::list<int> selected;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1) {
        selected.push_back(item);
        if (ip == "") {
            Output* e = _outputManager.GetOutput(item);
            
            if (proxy == "") {
                proxy = e->GetFPPProxyIP();
            }

            if (e->GetIP() == "MULTICAST") {
            } else if (ip != e->GetIP()) {
                ip = e->GetIP();
            }
        }

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return selected;
}

void xLightsFrame::UploadFPPBridgeInput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the input controller configuration for a FPP in Bridge mode running pixels using a PiHat or an RGBCape or similar. It should not be used to upload to your show player. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);
        if (ip == "") {
            wxTextEntryDialog dlg(this, "FPP Bridge Mode Controller IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

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
            } else if (theUser == "fpp") {
                password = "falcon";
            } else {
                wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
                if (ted.ShowModal() == wxID_OK) {
                    password = ted.GetValue();
                }
            }
        } else {
            wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
            if (ted.ShowModal() == wxID_OK) {
                password = ted.GetValue();
            }
        }

        FPP fpp(ip);
        fpp.parent = this;
        fpp.username = theUser;
        fpp.password = password;
        if (fpp.AuthenticateAndUpdateVersions() && !fpp.SetInputUniversesBridge(selected, &_outputManager)) {
            SetStatusText("FPP Input Upload Complete.");
        } else {
            SetStatusText("FPP Input Upload Failed.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::MultiControllerUpload()
{
    MultiControllerUploadDialog dlg(this);

    dlg.ShowModal();
}

void xLightsFrame::UploadFPPBridgeOutput()
{
    SetStatusText("");
    DisplayWarning("Not implemented", this);
}

void xLightsFrame::UploadFalconInput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the input controller configuration for a Falcon controller. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "Falcon IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        Falcon falcon(ip.ToStdString(), proxy.ToStdString());
        if (falcon.IsConnected()) {
            if (falcon.SetInputUniverses(&_outputManager, selected)) {
                SetStatusText("Falcon Input Upload Complete.");
            } else {
                SetStatusText("Falcon Input Upload Failed.");
            }
        } else {
            SetStatusText("Falcon Input Upload Failed.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadFalconOutput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for a Falcon controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "Falcon IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        Falcon falcon(ip.ToStdString(), proxy);
        if (falcon.IsConnected()) {
            if (falcon.SetOutputs(&AllModels, &_outputManager, selected, this)) {
                SetStatusText("Falcon Output Upload Complete.");
            } else {
                SetStatusText("Falcon Output Upload Failed.");
            }
        } else {
            SetStatusText("Falcon Output Upload Failed.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}
void xLightsFrame::UploadFPPProxyOuputs() {
    SetStatusText("");
    if (wxMessageBox("This will upload the output UDP configuration for FPP based on controllers with a FPP Proxy set. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES) {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);
        
        if (ip == "") {
            return;
        }
        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();
        
        std::set<std::string> done;
        for (auto x : selected) {
            Output *o = _outputManager.GetOutput(x);
            if (o->GetFPPProxyIP() != "" && done.find(o->GetFPPProxyIP()) == done.end()) {
                done.emplace(o->GetFPPProxyIP());
                FPP fpp(o->GetFPPProxyIP());
                fpp.AuthenticateAndUpdateVersions();
                fpp.UploadUDPOutputsForProxy(&_outputManager);
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }

}

void xLightsFrame::UploadFPPStringOuputs(const std::string &controller) {
    SetStatusText("");
    const PixelCapeInfo &ci = FPP::GetCapeRules(controller);
    if (wxMessageBox("This will upload the output controller configuration for a " + ci.description
                     + " controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?",
                     "Are you sure?", wxYES_NO, this) == wxYES) {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);
        
        if (ip == "") {
            wxTextEntryDialog dlg(this, "FPP IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }
        
        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        FPP fpp(ip);
        fpp.parent = this;
        fpp.pixelControllerType = controller;
        if (fpp.AuthenticateAndUpdateVersions() && !fpp.UploadPixelOutputs(&AllModels, &_outputManager, selected)) {
            SetStatusText("FPP Upload Complete.");
        } else {
            SetStatusText("FPP Upload Failed.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadPixlite16Output()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for a PixLite/PixCon controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "PixLite/PixCon IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        Pixlite16 pixlite(ip.ToStdString());
        if (pixlite.IsConnected()) {
            if (pixlite.SetOutputs(&AllModels, &_outputManager, selected, this)) {
                SetStatusText("PixLite/PixCon Upload Complete.");
            } else {
                SetStatusText("PixLite/PixCon Upload Failed.");
            }
        }
        else
        {
            SetStatusText("Unable to connect to PixLite/PixCon.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadSanDevicesInput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the input controller configuration for an SanDevices controller. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "SanDevices IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        SanDevices sanDevices(ip.ToStdString(), proxy.ToStdString());
        if (sanDevices.IsConnected()) {
            if (sanDevices.SetInputUniverses(&_outputManager, selected)) {
                SetStatusText("SanDevices Input Upload Complete.");
            } else {
                SetStatusText("SanDevices Input Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadSanDevicesOutput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for an SanDevices controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        wxString proxy;
        std::list<int> selected = GetSelectedOutputs(ip, proxy);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "SanDevices IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        SanDevices sanDevices(ip.ToStdString(), proxy);
        if (sanDevices.IsConnected()) {
            if (sanDevices.SetOutputs(&AllModels, &_outputManager, selected, this)) {
                SetStatusText("SanDevices Output Upload Complete.");
            } else {
                SetStatusText("SanDevices Output Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadJ1SYSOutput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for a J1SYS controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "J1SYS IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        J1Sys j1sys(ip.ToStdString());
        if (j1sys.IsConnected()) {
            if (j1sys.SetOutputs(&AllModels, &_outputManager, selected, this)) {
                SetStatusText("J1SYS Upload Complete.");
            } else {
                SetStatusText("J1SYS Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadESPixelStickOutput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for a ES Pixel Stick controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "") {
            wxTextEntryDialog dlg(this, "ES Pixel Stick IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK) {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        // Recalc all the models to make sure any changes on setup are incorporated
        RecalcModels();

        ESPixelStick esPixelStick(ip.ToStdString());
        if (esPixelStick.IsConnected()) {
            if (esPixelStick.SetOutputs(&AllModels, &_outputManager, selected, this)) {
                SetStatusText("ES Pixel Stick Upload Complete.");
            } else {
                SetStatusText("ES Pixel Stick Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::PingController(Output* e)
{
	if (e != nullptr)
	{
		std::string name;
		if (e->IsIpOutput()) {
			name = e->GetIP() + " " + e->GetDescription();
		} else {
			name = e->GetCommPort() + " " + e->GetDescription();
		}
		switch (e->Ping()) {
			case PING_OK:
			case PING_WEBOK:
				DisplayInfo("Pinging the Controller was Successful: " + name, this);
				break;
			case PING_OPENED:
				DisplayInfo("Serial Port Exists and was Opened: " + name, this);
				break;
			case PING_OPEN:
				DisplayWarning("Serial Port Exists but couldn't be opened: " + name, this);
				break;
			case PING_UNAVAILABLE:
				DisplayWarning("Controller Status is Unavailible: " + name, this);
				break;
			case PING_UNKNOWN:
				DisplayWarning("Controller Status is Unknown: " + name, this);
				break;
			case PING_ALLFAILED:
				DisplayError("Unable to Communicate with the Controller: " + name, this);
				break;
		}
	}
}

int xLightsFrame::SetZCPPPort(std::list<ZCPP_packet_t*>& modelDatas, int index, UDControllerPort* port, int portNum, int virtualString, long baseStart, bool isSerial, ZCPPOutput* zcpp)
{
    static log4cpp::Category &logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));

    auto current = modelDatas.back();
    if (current->Configuration.ports >= ZCPP_CONFIG_MAX_PORT_PER_PACKET)
    {
        ZCPP_packet_t* modelData = new ZCPP_packet_t();
        modelDatas.push_back(modelData);
        current = modelDatas.back();
        ZCPPOutput::InitialiseModelDataPacket(current, modelsChangeCount, zcpp->GetPriority(), zcpp->GetDescription());
    }
    current->Configuration.ports++;

    UDControllerPortModel* m = nullptr;
    if (port != nullptr && port->GetModels().size() > 0) m = port->GetFirstModel();

    UDVirtualString* vs = nullptr;
    if (port != nullptr && port->GetVirtualStringCount() > 0)
    {
        vs = port->GetVirtualString(virtualString);
    }

    logger_zcpp.debug("    Port/String %d/%d", portNum, virtualString);

    ZCPP_PortConfig* p = current->Configuration.PortConfig + (current->Configuration.ports-1);
    wxASSERT((size_t)p < (size_t)current + sizeof(ZCPP_packet_t) - sizeof(ZCPP_PortConfig)); // check pointer has not gone rogue

    p->port = portNum | (isSerial ? 0x80 : 0x00);
    p->string = virtualString;
    std::string protocol = "ws2811";
    if (port != nullptr)
    {
        protocol = port->GetProtocol();
    }
    p->protocol = ZCPPOutput::EncodeProtocol(protocol);
    logger_zcpp.debug("       Protocol %d/%s", ZCPPOutput::EncodeProtocol(protocol), (const char *)protocol.c_str());

    long sc = 0;
    if (vs != nullptr)
    {
        sc = vs->_startChannel - baseStart;
    }
    else if (port != nullptr)
    {
        if (port->GetModels().size() > 0)
        {
            sc = port->GetStartChannel() - baseStart;
        }
    }
    if (sc < 0) sc = 0;
    p->startChannel = ntohl(sc);
    logger_zcpp.debug("       Start Channel %ld", sc);

    long c = 0;
    if (vs != nullptr)
    {
        c = vs->Channels();
    }
    else if (port != nullptr)
    {
        if (port->GetModels().size() > 0)
        {
            c = port->Channels();
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
    static log4cpp::Category &logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
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
    p->port =  portNum;
    p->string = virtualStringNum;
    int len = std::min(255, (int)name.size());
    p->descriptionLength = len;
    strncpy(p->description, name.c_str(), len);
    logger_zcpp.debug("       Extra : %d/%d '%s'", portNum, virtualStringNum, (const char*)name.c_str());
}

void xLightsFrame::SetModelData(ZCPPOutput* zcpp, ModelManager* modelManager, OutputManager* outputManager, std::string showDir)
{
    static log4cpp::Category& logger_zcpp = log4cpp::Category::getInstance(std::string("log_zcpp"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Setting ZCPP model data");

    std::list<int> selected;
    std::string check;
    UDController cud(zcpp->GetIP(), zcpp->GetIP(), modelManager, outputManager, &selected, check);

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
    ZCPPOutput::InitialiseModelDataPacket(currentmd, modelsChangeCount, zcpp->GetPriority(), zcpp->GetDescription());

    int index = 0;
    for (int i = 0; i < cud.GetMaxPixelPort(); i++)
    {
        auto port = cud.GetControllerPixelPort(i + 1);
        port->CreateVirtualStrings(true);

        if (port == nullptr)
        {
            SetZCPPPort(modelDatas, index, nullptr, i, 0, baseStart, false, zcpp);
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
                    SetZCPPPort(modelDatas, index, port, i, string, baseStart, false, zcpp);
                    SetZCPPExtraConfig(extraConfigs, i, string, port->GetVirtualString(j)->_description, zcpp);
                    index++;
                }
            }
            else
            {
                SetZCPPPort(modelDatas, index, port, i, 0, baseStart, false, zcpp);
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
            SetZCPPPort(modelDatas, index, port, i, 0, baseStart, true, zcpp);
            SetZCPPExtraConfig(extraConfigs, 0x80 + i, 0, "", zcpp);
            index++;
        }
        else
        {
            SetZCPPPort(modelDatas, index, port, i, 0, baseStart, true, zcpp);
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

    if (zcpp->SetModelData(modelDatas, extraConfigs, showDir))
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

    for (auto ito : outputManager->GetOutputs())
    {
        if (ito->NeedsControllerConfig())
        {
            if (ito->GetType() == OUTPUT_ZCPP) {
                ZCPPOutput* zcpp = (ZCPPOutput*)(ito);
                SetModelData(zcpp, modelManager, outputManager, CurrentDir.ToStdString());
            }
        }
    }

    return true;
}

void xLightsFrame::OnButton_DiscoverClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Running controller discovery.");

    std::list<std::string> startAddresses;
    std::list<FPP*> instances;
    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        wxString newForce;
        for (auto &a : ips) {
            startAddresses.push_back(a);
        }
    }
    FPP::Discover(startAddresses, instances, true, true);
    std::list<FPP*> consider;
    for (auto fpp : instances) {
        std::list<Output*> outputs = _outputManager.GetAllOutputs(fpp->ipAddress, fpp->hostName);
        if (outputs.size() == 1) {
            Output *o = outputs.front();
            if (o->GetDescription() != fpp->description) {
                if (fpp->description == "") {
                    fpp->SetDescription(o->GetDescription());
                } else if (o->GetDescription() == "") {
                    o->SetDescription(fpp->description);
                } else {
                    //FIXME - descriptions aren't equal, ask what to do....
                }
            }
            if (o->GetControllerId() != fpp->pixelControllerType) {
                if (o->GetControllerId() == "") {
                    o->SetControllerId(fpp->pixelControllerType);
                } else if (fpp->pixelControllerType == "") {
                    // this will get set later at upload time
                } else {
                    //FIXME - controller types don't match, ask what to do
                    //for now, I'll use what FPP is configured for
                     o->SetControllerId(fpp->pixelControllerType);
                }
            }
            delete fpp;
        } else if (outputs.size() > 1) {
            // not sure what to do if multiple outputs match
            delete fpp;
        } else {
            consider.push_back(fpp);
        }
    }
    for (auto fpp : consider) {
        DDPOutput *ddp = new DDPOutput();
        if (fpp->hostName != "") {
            ddp->SetIP(fpp->hostName);
        } else {
            ddp->SetIP(fpp->ipAddress);
        }
        ddp->SetControllerId(fpp->pixelControllerType);
        ddp->SetDescription(fpp->description);
        if (fpp->platform == "ESPixelStick") {
            ddp->SetControllerId("ESPixelStick");
            ddp->KeepChannelNumber(false);
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
        ddp->SetChannels(count);
        _outputManager.AddOutput(ddp, -1);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "OnButton_DiscoverClick", nullptr, ddp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "OnButton_DiscoverClick", nullptr, ddp);
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "OnButton_DiscoverClick", nullptr, ddp);
        _outputModelManager.AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "OnButton_DiscoverClick", nullptr, ddp);
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
    SetStatusText("Discovery complete.");
    logger_base.debug("Controller discovery complete.");
}

void xLightsFrame::DoASAPWork()
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    logger_work.debug("Doing ASAP Work.");
    DoWork(_outputModelManager.GetASAPWork(), "ASAP");
}

void xLightsFrame::DoWork(uint32_t work, const std::string& type, Model* m, const std::string& selected)
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
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
        UpdateNetworkList();
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
        Model* mm = m;
        if (mm == nullptr) mm = _outputModelManager.GetModelToReload();
        if (mm != nullptr)
        {
            mm->ReloadModelXml();
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

void xLightsFrame::VisualiseOutput(Output *e, wxWindow *parent) {
    std::list<int> selected;
    std::string check;
    std::string ip = e->GetIP();
    UDController cud(ip, ip, &AllModels, &_outputManager, &selected, check);
    cud.Check(ControllerRegistry::GetRulesForController(e->GetControllerId()), check);
    if (!parent) {
        parent = this;
    }
    ControllerVisualiseDialog dlg(parent, cud);
    dlg.ShowModal();
}

void xLightsFrame::UploadEasyLightsOutput()
{
	SetStatusText("");
	if(wxMessageBox("This uploads your E131, Port Configurations and Resets your EasyLights controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
	{
		SetCursor(wxCURSOR_WAIT);
		wxString ip;
		std::list<int> selected = GetSelectedOutputs(ip);

		if(ip == "") {
			wxTextEntryDialog dlg(this, "EasyLights IP Address", "IP Address", ip);
			if(dlg.ShowModal() != wxID_OK) {
				SetCursor(wxCURSOR_ARROW);
				return;
			}
			ip = dlg.GetValue();
		}

		// Recalc all the models to make sure any changes on setup are incorporated
		RecalcModels();

		EasyLights EL(ip.ToStdString(), 0);

		if(EL.SetOutputs(&AllModels, &_outputManager, selected, this)) {
			SetStatusText("EasyLights Output Upload Complete.");
        } else {
			SetStatusText("EasyLights Output Upload Failed.");
		}
		
		SetCursor(wxCURSOR_ARROW);
	}
}
