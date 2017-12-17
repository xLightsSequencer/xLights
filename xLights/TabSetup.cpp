
/***************************************************************
 * Name:      xLightsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-11-03
 * Copyright: Matt Brown ()
 * License:
 **************************************************************/

#include "xLightsMain.h"
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/persist.h>
#include <wx/artprov.h>
#include <wx/regex.h>

#include "LayoutPanel.h"
#include "xLightsXmlFile.h"
#include "controllers/FPP.h"
#include "controllers/Falcon.h"
#include "controllers/Pixlite16.h"
#include "controllers/SanDevices.h"
#include "controllers/J1Sys.h"

// dialogs
#include "outputs/Output.h"
#include "outputs/NullOutput.h"
#include "outputs/E131Output.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/DDPOutput.h"
#include "outputs/DMXOutput.h"

// Process Setup Panel Events

#include "osxMacUtils.h"

const long xLightsFrame::ID_NETWORK_ADDUSB = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDNULL = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDE131 = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDARTNET = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDDDP = wxNewId();
const long xLightsFrame::ID_NETWORK_BEIPADDR = wxNewId();
const long xLightsFrame::ID_NETWORK_BECHANNELS = wxNewId();
const long xLightsFrame::ID_NETWORK_BEDESCRIPTION = wxNewId();
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
const long xLightsFrame::ID_NETWORK_UCOFPPB = wxNewId();
const long xLightsFrame::ID_NETWORK_UCIFALCON = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOFALCON = wxNewId();
const long xLightsFrame::ID_NETWORK_UCISanDevices = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOSanDevices = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOPixlite16 = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOJ1SYS = wxNewId();

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
    static bool HasMenuSeparator=false;
    int idx, cnt, i;

    // don't change show directories with an open sequence because models won't match
    if (!CloseSequence()) {
        return false;
    }

    // delete any views that were added to the menu
    for (auto it = LayoutGroups.begin(); it != LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if (grp != nullptr) {
            RemovePreviewOption(grp);
        }
    }
    PreviewWindows.clear();

    if (newdir != CurrentDir && "" != CurrentDir) {
        wxFileName kbf;
        kbf.AssignDir(CurrentDir);
        kbf.SetFullName("xlights_keybindings.xml");
        mainSequencer->keyBindings.Save(kbf);
    }

    // reject change if something is playing
    if (play_mode == play_list || play_mode == play_single)
    {
        wxMessageBox(_("Cannot change directories during playback"),_("Error"));
        return false;
    }

    // Check to see if any show directory files need to be saved
    CheckUnsavedChanges();

    // Force update of Preset dialog
    if( EffectTreeDlg != nullptr ) {
        delete EffectTreeDlg;
    }
    EffectTreeDlg = nullptr;

    // update most recently used array
    idx=mru.Index(newdir);
    if (idx != wxNOT_FOUND) mru.RemoveAt(idx);
    if (!CurrentDir.IsEmpty())
    {
        idx=mru.Index(CurrentDir);
        if (idx != wxNOT_FOUND) mru.RemoveAt(idx);
        mru.Insert(CurrentDir,0);
    }
    cnt=mru.GetCount();
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
    bool DirExists=wxFileName::DirExists(newdir);
    wxString mru_name, value;
    wxConfigBase* config = wxConfigBase::Get();
    if (DirExists) config->Write(_("LastDir"), newdir);
    for (i=0; i<MRU_LENGTH; i++)
    {
        mru_name=wxString::Format("mru%d",i);
        if (mru_MenuItem[i] != nullptr)
        {
            Disconnect(mru_MenuItem[i]->GetId(), wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
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
    cnt=mru.GetCount();
    if (!HasMenuSeparator && cnt > 0)
    {
        MenuFile->AppendSeparator();
        HasMenuSeparator=true;
    }
    for (i=0; i<cnt; i++)
    {
        int menuID = wxNewId();
        mru_MenuItem[i] = new wxMenuItem(MenuFile, menuID, mru[i]);
        Connect(menuID,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
        MenuFile->Append(mru_MenuItem[i]);
    }
    MenuFile->UpdateUI();

    if (!DirExists)
    {
        wxString msg=_("The show directory '") + newdir + ("' no longer exists.\nPlease choose a new show directory.");
        wxMessageBox(msg);
        return false;
    }

    ObtainAccessToURL(newdir.ToStdString());
    
    // update UI
    CheckBoxLightOutput->SetValue(false);
    _outputManager.StopOutput();
    _outputManager.DeleteAllOutputs();
    CurrentDir=newdir;
    showDirectory=newdir;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Show directory set to : %s.", (const char *)showDirectory.c_str());

    if (mBackupOnLaunch)
    {
        logger_base.debug("Backing up show directory before we do anything this session in this folder : %s.", (const char *)CurrentDir.c_str());
        DoBackup(false, true);
        logger_base.debug("Backup completed.");
    }

    long LinkFlag=0;
    config->Read(_("LinkFlag"), &LinkFlag);
    if( LinkFlag ) {
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(false);
        mediaDirectory = CurrentDir;
        config->Write(_("MediaDir"), mediaDirectory);
        MediaDirectoryLabel->SetLabel(mediaDirectory);
        MediaDirectoryLabel->GetParent()->Layout();
        logger_base.debug("Media Directory set to : %s.", (const char *)mediaDirectory.c_str());
        BitmapButton_Link_Dirs->SetToolTip("Unlink Directories");
    } else {
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_UNLINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(true);
        BitmapButton_Link_Dirs->SetToolTip("Link Directories");
    }

    while (Notebook1->GetPageCount() > FixedPages)
    {
        Notebook1->DeletePage(FixedPages);
    }

    EnableNetworkChanges();
    DisplayXlightsFilename(wxEmptyString);

    // load network
    networkFile.AssignDir( CurrentDir );
    networkFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
    if (networkFile.FileExists())
    {
        if (!_outputManager.Load(CurrentDir.ToStdString()))
        {
            logger_base.warn("Unable to load network config %s", (const char*)networkFile.GetFullPath().ToStdString().c_str());
            wxMessageBox(_("Unable to load network definition file"), _("Error"));
        }
        else
        {
            logger_base.debug("Loaded network config %s", (const char*)networkFile.GetFullPath().ToStdString().c_str());
        }
    }
    else
    {
        _outputManager.SetShowDir(CurrentDir.ToStdString());
    }

    UnsavedNetworkChanges = false;
    ShowDirectoryLabel->SetLabel(showDirectory);
    ShowDirectoryLabel->GetParent()->Layout();

    // load sequence effects
//~    EffectsPanel1->SetDefaultPalette();
//~    EffectsPanel2->SetDefaultPalette();
    UpdateNetworkList(false);
    LoadEffectsFile();

    wxFileName kbf;
    kbf.AssignDir(CurrentDir);
    kbf.SetFullName("xlights_keybindings.xml");
    mainSequencer->keyBindings.Load(kbf);

    EnableSequenceControls(true);
    layoutPanel->RefreshLayout();

    Notebook1->ChangeSelection(SETUPTAB);
    SetStatusText("");
    FileNameText->SetLabel(newdir);
    return true;
}

void xLightsFrame::GetControllerDetailsForChannel(long channel, std::string& type, std::string& description, long& channeloffset, std::string &ip, std::string& u, std::string& inactive, int& output)
{
    long ch = 0;
    Output* o = _outputManager.GetOutput(channel, ch);
    channeloffset = ch;

    type = "Unknown";
    description = "";
    ip = "";
    u = "";
    inactive = "";

    if (o != nullptr)
    {
        type = o->GetType();
        description = o->GetDescription();
        if (o->IsIpOutput())
        {
            ip = o->GetIP();
            u = o->GetUniverseString();
        }
        else
        {
            ip = o->GetCommPort();
            u = wxString::Format(wxT("%i"), o->GetBaudRate()).ToStdString();
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
    Output* o = _outputManager.GetOutput(channel, stch);

    if (o != nullptr)
    {
        return o->GetChannelMapping(channel);
    }
    else
    {
        return wxString::Format("Channel %i could not be mapped to a controller.", channel).ToStdString();
    }
}

void xLightsFrame::UpdateNetworkList(bool updateModels)
{
    long newidx;

    if (updateModels) _setupChanged = true;

    int item = GridNetwork->GetTopItem() + GridNetwork->GetCountPerPage() - 1;
    int itemselected = GetNetworkSelection();

    auto outputs = _outputManager.GetOutputs();

    GridNetwork->Freeze();
    GridNetwork->DeleteAllItems();
    for (auto e = outputs.begin(); e != outputs.end(); ++e)
    {
        newidx = GridNetwork->InsertItem(GridNetwork->GetItemCount(), wxString::Format(wxT("%i"), (*e)->GetOutputNumber()));
        GridNetwork->SetItem(newidx, 1, (*e)->GetType());
        if ((*e)->IsIpOutput())
        {
            GridNetwork->SetItem(newidx, 2, (*e)->GetIP());
            GridNetwork->SetItem(newidx, 3, (*e)->GetUniverseString());
        }
        else if ((*e)->IsSerialOutput())
        {
            GridNetwork->SetItem(newidx, 2, (*e)->GetCommPort());
            GridNetwork->SetItem(newidx, 3, (*e)->GetBaudRateString());
        }
        GridNetwork->SetItem(newidx, 4, wxString::Format(wxT("%ld"), (*e)->GetChannels()));
        GridNetwork->SetItem(newidx, 5, wxString::Format(wxT("Channels %ld to %ld"), (*e)->GetStartChannel(), (*e)->GetEndChannel()));
        if ((*e)->IsEnabled())
        {
            GridNetwork->SetItem(newidx, 6, "Yes");
        }
        else
        {
            GridNetwork->SetItem(newidx, 6, "No");
        }
        GridNetwork->SetItem(newidx, 7, (*e)->GetDescription());
        GridNetwork->SetColumnWidth(7, wxLIST_AUTOSIZE);
        GridNetwork->SetItem(newidx, 8, (*e)->IsSuppressDuplicateFrames() ? "Y" : "");
        if (!(*e)->IsEnabled())
        {
            GridNetwork->SetItemTextColour(newidx, *wxLIGHT_GREY);
        }
    }

    GridNetwork->SetColumnWidth(2, _outputManager.GetOutputCount() > 0 ? wxLIST_AUTOSIZE : 100);

    // try to ensure what should be visible is visible in roughly the same part of the screen
    if (item >= GridNetwork->GetItemCount()) item = GridNetwork->GetItemCount() - 1;
    if (item != -1)
    {
        GridNetwork->EnsureVisible(item);
    }

    if (itemselected >= GridNetwork->GetItemCount()) itemselected = GridNetwork->GetItemCount() - 1;

    if (itemselected != -1)
    {
        GridNetwork->EnsureVisible(itemselected);
    }

    GridNetwork->Thaw();
    GridNetwork->Refresh();
}

// reset test channel listbox
void xLightsFrame::UpdateChannelNames()
{
    wxString FormatSpec,RGBFormatSpec;
    int NodeNum;
    size_t ChannelNum, NodeCount,n,c, ChanPerNode;

    ChNames.clear();
    ChNames.resize(_outputManager.GetTotalChannels());
    // update names with RGB models where MyDisplay is checked

	// KW left as some of the conversions seem to use this
    for (auto it = AllModels.begin(); it != AllModels.end(); ++it) {
        Model *model = it->second;
        NodeCount=model->GetNodeCount();
        ChanPerNode = model->GetChanCountPerNode();
        FormatSpec = "Ch %ld: "+model->name+" #%d";
        for(n=0; n < NodeCount; n++)
        {
            ChannelNum=model->NodeStartChannel(n);

            NodeNum=n+1;
            if (ChanPerNode==1)
            {
                if (ChannelNum < ChNames.Count())
                {
                    if( ChNames[ChannelNum] == "" )
                    {
                        ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,NodeNum);
                    }
                }
            }
            else
            {
                for(c=0; c < ChanPerNode; c++)
                {
                    if (ChannelNum < ChNames.Count())
                    {
                        if( ChNames[ChannelNum] == "" )
                        {
                            ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,NodeNum)+model->GetChannelColorLetter(c);
                        }
                    }
                    ChannelNum++;
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
            wxMessageBox("WARNING: Opening a show folder inside a backup folder. This is ok but please make sure you change back to your proper show folder and dont make changes in this folder.", "WARNING");
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

    NetworkChange();
    UpdateNetworkList(true);

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
        wxMessageBox(_("Please select a single row first"), _("Error"));
        return;
    }

    Output* o = _outputManager.GetOutput(item);
    Output* newoutput = o->Configure(this, &_outputManager);
    if (newoutput != nullptr)
    {
        if (newoutput != o)
        {
            _outputManager.Replace(o, newoutput);
        }

        NetworkChange();
        UpdateNetworkList(true);
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
        if (!IPOutput::IsIPValid(dlg.GetValue().ToStdString()) && dlg.GetValue().ToStdString() != "MULTICAST")
        {
            wxMessageBox("Illegal ip address " + dlg.GetValue().ToStdString());
        }
        else
        {
            while (item != -1)
            {
                _outputManager.GetOutput(item)->SetIP(dlg.GetValue().ToStdString());

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }

            NetworkChange();
            UpdateNetworkList(false);

            item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            while (item != -1)
            {
                GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

                item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
        }
    }
}

void xLightsFrame::UpdateSelectedSuppressDuplicates(bool suppressDuplicates)
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        _outputManager.GetOutput(item)->SetSuppressDuplicateFrames(suppressDuplicates);
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    NetworkChange();
    UpdateNetworkList(false);

    item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

        item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
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
            _outputManager.GetOutput(item)->SetDescription(dlg.GetValue().ToStdString());

            item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }

        NetworkChange();
        UpdateNetworkList(false);

        item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        while (item != -1)
        {
            GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

            item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
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
                wxMessageBox(wxString::Format("%i Too many channels for %s controller %s. Channels not changed.", dlg.GetValue(), o->GetType(), o->GetDescription()));
            }
            else
            {
                o->SetChannels(dlg.GetValue());
            }

            item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }

        NetworkChange();
        UpdateNetworkList(true);

        item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        while (item != -1)
        {
            GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

            item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
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
            changed = true;
            o->Enable(false);
        }
    }

    if (changed)
    {
        NetworkChange();
        UpdateNetworkList(false);

        int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        while (item != -1)
        {
            GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

            item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        }
    }
}

void xLightsFrame::ActivateSelectedNetworks(bool active)
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        _outputManager.GetOutput(item)->Enable(active);

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    NetworkChange();
    UpdateNetworkList(false);

    item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

        item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::DeleteSelectedNetworks()
{
    int removed = 0;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        _outputManager.DeleteOutput(_outputManager.GetOutput(item - removed));

        removed++;
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    NetworkChange();
    UpdateNetworkList(true);

    item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

        item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::OnButtonNetworkDeleteClick(wxCommandEvent& event)
{
    DeleteSelectedNetworks();
}

void xLightsFrame::OnButtonNetworkDeleteAllClick(wxCommandEvent& event)
{
    _outputManager.DeleteAllOutputs();
    NetworkChange();
    UpdateNetworkList(true);
}

void xLightsFrame::OnButtonNetworkMoveUpClick(wxCommandEvent& event)
{
    long SelectedItem = GetNetworkSelection();
    if (SelectedItem == -1)
    {
        wxMessageBox(_("Please select a single row first"), _("Error"));
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
        wxMessageBox(_("Please select a single row first"), _("Error"));
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

void xLightsFrame::OnButtonArtNETClick(wxCommandEvent& event)
{
    SetupArtNet(nullptr);
}

void xLightsFrame::OnButton_DDPClick(wxCommandEvent& event)
{
    SetupDDP(nullptr);
}

void xLightsFrame::OnButtonAddNullClick(wxCommandEvent& event)
{
    SetupNullOutput(nullptr);
}

void xLightsFrame::SetupNullOutput(Output* e, int after) {

    Output* null = e;
    if (null == nullptr) null = new NullOutput();
    _outputManager.AddOutput(null, after);

    if (null->Configure(this, &_outputManager) != nullptr)
    {
        NetworkChange();
        UpdateNetworkList(true);
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

    if (e131->Configure(this, &_outputManager) != nullptr)
    {
        NetworkChange();
        UpdateNetworkList(true);
    }
    else
    {
        if (e != e131)
        {
            _outputManager.DeleteOutput(e131);
        }
    }
}

void xLightsFrame::SetupArtNet(Output* e, int after)
{
    Output* artnet = e;
    if (artnet == nullptr) artnet = new ArtNetOutput();
    _outputManager.AddOutput(artnet, after);

    if (artnet->Configure(this, &_outputManager) != nullptr)
    {
        NetworkChange();
        UpdateNetworkList(true);
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

    if (ddp->Configure(this, &_outputManager) != nullptr)
    {
        NetworkChange();
        UpdateNetworkList(true);
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

    Output* newoutput = serial->Configure(this, &_outputManager);

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
        NetworkChange();
        UpdateNetworkList(true);
    }
    else
    {
        NetworkChange();
        UpdateNetworkList(true);
    }
}

void xLightsFrame::OnButtonAddDongleClick(wxCommandEvent& event)
{
    SetupDongle(nullptr);
}

void xLightsFrame::NetworkChange()
{
    _outputManager.SomethingChanged();
    UnsavedNetworkChanges = true;
#ifdef __WXOSX__
    ButtonSaveSetup->SetForegroundColour(wxColour(255, 0, 0));
    ButtonSaveSetup->SetBackgroundColour(wxColour(255, 0, 0));
#else
    ButtonSaveSetup->SetBackgroundColour(wxColour(255, 108, 108));
#endif
}

bool xLightsFrame::SaveNetworksFile()
{
    if (_outputManager.Save())
    {
        UnsavedNetworkChanges = false;
#ifdef __WXOSX__
        ButtonSaveSetup->SetForegroundColour(*wxBLACK);
        ButtonSaveSetup->SetBackgroundColour(mDefaultNetworkSaveBtnColor);
#else
        ButtonSaveSetup->SetBackgroundColour(mDefaultNetworkSaveBtnColor);
#endif
        return true;
    }
    else
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("Unable to save network definition file");
        wxMessageBox(_("Unable to save network definition file"), _("Error"));
        return false;
    }
}

void xLightsFrame::OnButtonSaveSetupClick(wxCommandEvent& event)
{
    SaveNetworksFile();
}


void xLightsFrame::ChangeMediaDirectory(wxCommandEvent& event)
{
    wxDirDialog dialog(this);
    dialog.SetPath(mediaDirectory);
    if (dialog.ShowModal() == wxID_OK)
    {
        mediaDirectory = dialog.GetPath();
        ObtainAccessToURL(mediaDirectory.ToStdString());
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("MediaDir"), mediaDirectory);
        MediaDirectoryLabel->SetLabel(mediaDirectory);
        MediaDirectoryLabel->GetParent()->Layout();
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Media directory set to : %s.", (const char *)mediaDirectory.c_str());
    }
}

void xLightsFrame::SetSyncUniverse(int syncUniverse)
{
    _outputManager.SetSyncUniverse(syncUniverse);
}

void xLightsFrame::OnSpinCtrl_SyncUniverseChange(wxSpinEvent& event)
{
    SetSyncUniverse(SpinCtrl_SyncUniverse->GetValue());
    NetworkChange();
}

int xLightsFrame::GetNetworkSelectedItemCount() const
{
    int selected = 0;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        selected++;
        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return selected;
}

bool xLightsFrame::AllSelectedSupportIP()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        if (!_outputManager.GetOutput(item)->IsIpOutput())
        {
            return false;
        }

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return true;
}

bool xLightsFrame::AllSelectedSupportChannels()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        if (!_outputManager.GetOutput(item)->IsOutputable())
        {
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
    mnuAdd->Append(ID_NETWORK_ADDDDP, "DDP")->Enable(selcnt == 1);
    mnuAdd->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    wxMenu* mnuUploadController = new wxMenu();

    wxMenu* mnuUCInput = new wxMenu();

    wxMenuItem* beUCIFPPB = mnuUCInput->Append(ID_NETWORK_UCIFPPB, "FPP Bridge Mode");
    if (!AllSelectedSupportIP())
    {
        beUCIFPPB->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCIFPPB->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (o->GetIP() == "MULTICAST")
                {
                }
                else if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCIFPPB->Enable(valid);
        }
    }

    wxMenuItem* beUCIFalcon = mnuUCInput->Append(ID_NETWORK_UCIFALCON, "Falcon");
    if (!AllSelectedSupportIP())
    {
        beUCIFalcon->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCIFalcon->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (o->GetIP() == "MULTICAST")
                {
                }
                else if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCIFalcon->Enable(valid);
        }
    }

    wxMenuItem* beUCISanDevices = mnuUCInput->Append(ID_NETWORK_UCISanDevices, "SanDevices");
    if (!AllSelectedSupportIP())
    {
        beUCISanDevices->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCISanDevices->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;
            wxString type;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                if (type == "")
                {
                    type = o->GetType();
                }
                else
                {
                    if (type != o->GetType())
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCISanDevices->Enable(valid);
        }
    }

    mnuUploadController->Append(ID_NETWORK_UCINPUT, "E1.31 Input Defintion", mnuUCInput, "");
    mnuUCInput->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    wxMenu* mnuUCOutput = new wxMenu();

#if 0 // controller output upload ... not built yet but coming
    wxMenuItem* beUCOFPPB = mnuUCOutput->Append(ID_NETWORK_UCOFPPB, "FPP Bridge Mode");
    beUCOFPPB->Enable(selcnt == 1);
    if (!AllSelectedSupportIP())
    {
        beUCOFPPB->Enable(false);
    }
#endif

    wxMenuItem* beUCOFalcon = mnuUCOutput->Append(ID_NETWORK_UCOFALCON, "Falcon");
    if (!AllSelectedSupportIP())
    {
        beUCOFalcon->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCOFalcon->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (o->GetIP() == "MULTICAST")
                {
                }
                else if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCOFalcon->Enable(valid);
        }
    }

    wxMenuItem* beUCOPixlite16 = mnuUCOutput->Append(ID_NETWORK_UCOPixlite16, "Pixlite");
    if (!AllSelectedSupportIP())
    {
        beUCOPixlite16->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCOPixlite16->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (o->GetIP() == "MULTICAST")
                {
                }
                else if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCOPixlite16->Enable(valid);
        }
    }

    wxMenuItem* beUCOSanDevices = mnuUCOutput->Append(ID_NETWORK_UCOSanDevices, "SanDevices");
    if (!AllSelectedSupportIP())
    {
        beUCOSanDevices->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCOSanDevices->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;
            wxString type;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                if (type == "")
                {
                    type = o->GetType();
                }
                else
                {
                    if (type != o->GetType())
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCOSanDevices->Enable(valid);
        }
    }

    wxMenuItem* beUCOJ1SYS = mnuUCOutput->Append(ID_NETWORK_UCOJ1SYS, "J1SYS");
    if (!AllSelectedSupportIP())
    {
        beUCOJ1SYS->Enable(false);
    }
    else
    {
        if (selcnt == 1)
        {
            beUCOJ1SYS->Enable(true);
        }
        else
        {
            bool valid = true;
            // check all are multicast or one ip address
            wxString ip;
            wxString type;

            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

            while (item != -1)
            {
                Output* o = _outputManager.GetOutput(item);

                if (ip != o->GetIP())
                {
                    if (ip == "")
                    {
                        ip = o->GetIP();
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                if (type == "")
                {
                    type = o->GetType();
                }
                else
                {
                    if (type != o->GetType())
                    {
                        valid = false;
                        break;
                    }
                }

                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            beUCOJ1SYS->Enable(valid);
        }
    }

    mnuUploadController->Append(ID_NETWORK_UCOUTPUT, "Output", mnuUCOutput, "");
    mnuUCOutput->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    mnu.Append(ID_NETWORK_UPLOADCONTROLLER, "Upload To Controller", mnuUploadController, "");
    mnuUploadController->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);

    wxMenu* mnuBulkEdit = new wxMenu();
    wxMenuItem* beip = mnuBulkEdit->Append(ID_NETWORK_BEIPADDR, "IP Address");
    beip->Enable(selcnt > 0);
    if (!AllSelectedSupportIP())
    {
        beip->Enable(false);
    }
    wxMenuItem* bech = mnuBulkEdit->Append(ID_NETWORK_BECHANNELS, "Channels");
    bech->Enable(selcnt > 0);
    if (!AllSelectedSupportChannels())
    {
        bech->Enable(false);
    }
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

    if (!ButtonAddE131->IsEnabled())
    {
        ma->Enable(false);
        be->Enable(false);
    }

    wxMenuItem* mid = mnu.Append(ID_NETWORK_DELETE, "Delete");
    wxMenuItem* mia = mnu.Append(ID_NETWORK_ACTIVATE, "Activate");
    wxMenuItem* mide = mnu.Append(ID_NETWORK_DEACTIVATE, "Deactivate");
    wxMenuItem* mideu = mnu.Append(ID_NETWORK_DEACTIVATEUNUSED, "Deactivate Unused");
    wxMenuItem* oc = mnu.Append(ID_NETWORK_OPENCONTROLLER, "Open Controller");

    mideu->Enable(true);
    mid->Enable(selcnt > 0);
    mia->Enable(selcnt > 0);
    mide->Enable(selcnt > 0);

    oc->Enable(selcnt == 1);
    if (!AllSelectedSupportIP())
    {
        oc->Enable(false);
    }
    else
    {
        int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item != -1) {
            Output* o = _outputManager.GetOutput(item);
            if (o->GetIP() == "MULTICAST")
            {
                oc->Enable(false);
            }
        }
    }

    if (!ButtonAddE131->IsEnabled())
    {
        mia->Enable(false);
        mid->Enable(false);
        mide->Enable(false);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, nullptr, this);
    PopupMenu(&mnu);
    GridNetwork->SetFocus();
}

void xLightsFrame::OnNetworkPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (id == ID_NETWORK_ADDUSB)
    {
        SetupDongle(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDNULL)
    {
        SetupNullOutput(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDE131)
    {
        SetupE131(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDARTNET)
    {
        SetupArtNet(nullptr, item+1);
    }
    else if (id == ID_NETWORK_ADDDDP)
    {
        SetupDDP(nullptr, item + 1);
    }
    else if (id == ID_NETWORK_UCIFPPB)
    {
        UploadFPPBridgeInput();
    }
    else if (id == ID_NETWORK_UCOFPPB)
    {
        UploadFPPBridgeOutput();
    }
    else if (id == ID_NETWORK_UCIFALCON)
    {
        UploadFalconInput();
    }
    else if (id == ID_NETWORK_UCOFALCON)
    {
        UploadFalconOutput();
    }
    else if (id == ID_NETWORK_UCISanDevices)
    {
        UploadSanDevicesInput();
    }
    else if (id == ID_NETWORK_UCOSanDevices)
    {
        UploadSanDevicesOutput();
    }
    else if (id == ID_NETWORK_UCOJ1SYS)
    {
        UploadJ1SYSOutput();
    }
    else if (id == ID_NETWORK_UCOPixlite16)
    {
        UploadPixlite16Output();
    }
    else if (id == ID_NETWORK_BEIPADDR)
    {
        UpdateSelectedIPAddresses();
    }
    else if (id == ID_NETWORK_BECHANNELS)
    {
        UpdateSelectedChannels();
    }
    else if (id == ID_NETWORK_BEDESCRIPTION)
    {
        UpdateSelectedDescriptions();
    }
    else if (id == ID_NETWORK_BESUPPRESSDUPLICATESYES)
    {
        UpdateSelectedSuppressDuplicates(true);
    }
    else if (id == ID_NETWORK_BESUPPRESSDUPLICATESNO)
    {
        UpdateSelectedSuppressDuplicates(false);
    }
    else if (id == ID_NETWORK_DELETE)
    {
        DeleteSelectedNetworks();
    }
    else if (id == ID_NETWORK_ACTIVATE)
    {
        ActivateSelectedNetworks(true);
    }
    else if (id == ID_NETWORK_DEACTIVATE)
    {
        ActivateSelectedNetworks(false);
    }
    else if (id == ID_NETWORK_DEACTIVATEUNUSED)
    {
        DeactivateUnusedNetworks();
    }
    else if (id == ID_NETWORK_OPENCONTROLLER)
    {
        Output* o = _outputManager.GetOutput(item);
        if (o != nullptr) {
            ::wxLaunchDefaultBrowser("http://" + o->GetIP());
        }
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
        if (GridNetwork->GetSelectedItemCount() > 0)
        {
            DeleteSelectedNetworks();
        }
        break;
    case 'A':
        if (::wxGetKeyState(WXK_CONTROL))
        {
            int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL);
            while (item != -1)
            {
                GridNetwork->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL);
            }
        }
        break;
    }
}

std::list<int> xLightsFrame::GetSelectedOutputs(wxString& ip)
{
    std::list<int> selected;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        selected.push_back(item);
        if (ip == "")
        {
            Output* e = _outputManager.GetOutput(item);

            if (e->GetIP() == "MULTICAST")
            {
            }
            else if (ip != e->GetIP())
            {
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
        std::list<int> selected = GetSelectedOutputs(ip);
        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "FPP Bridge Mode Controller IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
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
        for (int i = 0; i < count; i++)
        {
            if (ips[i] == ip)
            {
                theUser = users[i];
                thePassword = passwords[i];
            }
        }

        if (thePassword == "true")
        {
            if (theUser == "pi")
            {
                password = "raspberry";
            }
            else if (theUser == "fpp")
            {
                password = "falcon";
            }
            else
            {
                wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
                if (ted.ShowModal() == wxID_OK)
                {
                    password = ted.GetValue();
                }
            }
        }
        else
        {
            wxTextEntryDialog ted(this, "Enter password for " + theUser, "Password", ip);
            if (ted.ShowModal() == wxID_OK)
            {
                password = ted.GetValue();
            }
        }

        FPP fpp(&_outputManager, ip.ToStdString(), theUser.ToStdString(), password.ToStdString());

        if (fpp.IsConnected())
        {
            if (fpp.SetInputUniversesBridge(selected, this))
            {
                SetStatusText("FPP Input Upload Complete.");
            }
            else
            {
                SetStatusText("FPP Input Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadFPPBridgeOutput()
{
    SetStatusText("");
    wxMessageBox("Not implemented");
}

void xLightsFrame::UploadFalconInput()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the input controller configuration for a Falcon controller. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "Falcon IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        Falcon falcon(ip.ToStdString());
        if (falcon.IsConnected())
        {
            if (falcon.SetInputUniverses(&_outputManager, selected))
            {
                SetStatusText("Falcon Input Upload Complete.");
            }
            else
            {
                SetStatusText("Falcon Input Upload Failed.");
            }
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
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "Falcon IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        Falcon falcon(ip.ToStdString());
        if (falcon.IsConnected())
        {
            if (falcon.SetOutputs(&AllModels, &_outputManager, selected, this))
            {
                SetStatusText("Falcon Output Upload Complete.");
            }
            else
            {
                SetStatusText("Falcon Output Upload Failed.");
            }
        }
        else
        {
            SetStatusText("Falcon Output Upload Failed.");
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadPixlite16Output()
{
    SetStatusText("");
    if (wxMessageBox("This will upload the output controller configuration for a Pixlite controller. It requires that you have setup the controller connection on your models. Do you want to proceed with the upload?", "Are you sure?", wxYES_NO, this) == wxYES)
    {
        SetCursor(wxCURSOR_WAIT);
        wxString ip;
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "Pixlite IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        Pixlite16 pixlite(ip.ToStdString());
        if (pixlite.IsConnected())
        {
            if (pixlite.SetOutputs(&AllModels, &_outputManager, selected, this))
            {
                SetStatusText("Pixlite Upload Complete.");
            }
            else
            {
                SetStatusText("Pixlite Upload Failed.");
            }
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
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "SanDevices IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        SanDevices sanDevices(ip.ToStdString());
        if (sanDevices.IsConnected())
        {
            if (sanDevices.SetInputUniverses(&_outputManager, selected))
            {
                SetStatusText("SanDevices Input Upload Complete.");
            }
            else
            {
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
        std::list<int> selected = GetSelectedOutputs(ip);

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "SanDevices IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        SanDevices sanDevices(ip.ToStdString());
        if (sanDevices.IsConnected())
        {
            if (sanDevices.SetOutputs(&AllModels, &_outputManager, selected, this))
            {
                SetStatusText("SanDevices Output Upload Complete.");
            }
            else
            {
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

        if (ip == "")
        {
            wxTextEntryDialog dlg(this, "J1SYS IP Address", "IP Address", ip);
            if (dlg.ShowModal() != wxID_OK)
            {
                SetCursor(wxCURSOR_ARROW);
                return;
            }
            ip = dlg.GetValue();
        }

        J1Sys j1sys(ip.ToStdString());
        if (j1sys.IsConnected())
        {
            if (j1sys.SetOutputs(&AllModels, &_outputManager, selected, this))
            {
                SetStatusText("J1SYS Upload Complete.");
            }
            else
            {
                SetStatusText("J1SYS Upload Failed.");
            }
        }
        SetCursor(wxCURSOR_ARROW);
    }
}
