
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
#include "FPP.h"
#include "Falcon.h"

// dialogs
#include "SerialPortWithRate.h"
#include "E131Dialog.h"
#include "NullOutputDialog.h"
#include "ArtNetDialog.h"
#include "xlights_out.h"
#include "SimpleFTP.h"

// Process Setup Panel Events

#include "osxMacUtils.h"

const long xLightsFrame::ID_NETWORK_ADDUSB = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDNULL = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDE131 = wxNewId();
const long xLightsFrame::ID_NETWORK_ADDARTNET = wxNewId();
const long xLightsFrame::ID_NETWORK_BEIPADDR = wxNewId();
const long xLightsFrame::ID_NETWORK_BECHANNELS = wxNewId();
const long xLightsFrame::ID_NETWORK_BEDESCRIPTION = wxNewId();
const long xLightsFrame::ID_NETWORK_ADD = wxNewId();
const long xLightsFrame::ID_NETWORK_BULKEDIT = wxNewId();
const long xLightsFrame::ID_NETWORK_DELETE = wxNewId();
const long xLightsFrame::ID_NETWORK_ACTIVATE = wxNewId();
const long xLightsFrame::ID_NETWORK_DEACTIVATE = wxNewId();
const long xLightsFrame::ID_NETWORK_OPENCONTROLLER = wxNewId();
const long xLightsFrame::ID_NETWORK_UPLOADCONTROLLER = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOUTPUT = wxNewId();
const long xLightsFrame::ID_NETWORK_UCINPUT = wxNewId();
const long xLightsFrame::ID_NETWORK_UCIFPPB = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOFPPB = wxNewId();
const long xLightsFrame::ID_NETWORK_UCIFALCON = wxNewId();
const long xLightsFrame::ID_NETWORK_UCOFALCON = wxNewId();

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
    if (play_mode == play_sched || play_mode == play_list || play_mode == play_single)
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
    CheckBoxRunSchedule->SetValue(false);
    if (xout)
    {
        delete xout;
        xout = nullptr;
    }
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

    TextCtrlLog->Clear();
    while (Notebook1->GetPageCount() > FixedPages)
    {
        Notebook1->DeletePage(FixedPages);
    }

    EnableNetworkChanges();
    DisplayXlightsFilename(wxEmptyString);

    // load network
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, "Networks" );
    root->AddAttribute( "computer", wxGetHostName());
    NetworkXML.SetRoot( root ); // reset xml
    networkFile.AssignDir( CurrentDir );
    networkFile.SetFullName(_(XLIGHTS_NETWORK_FILE));
    if (networkFile.FileExists())
    {
        if (!NetworkXML.Load( networkFile.GetFullPath() ))
        {
            wxMessageBox(_("Unable to load network definition file"), _("Error"));
        } else {
            logger_base.debug("Loaded network config %s", (const char*)networkFile.GetFullPath().ToStdString().c_str());
        }
    }

    ShowDirectoryLabel->SetLabel(showDirectory);
    ShowDirectoryLabel->GetParent()->Layout();

    // load schedule
    UpdateShowDates(wxDateTime::Now(),wxDateTime::Now());
    ShowEvents.Clear();
    scheduleFile.AssignDir( CurrentDir );
    scheduleFile.SetFullName(_(XLIGHTS_SCHEDULE_FILE));
    if (scheduleFile.FileExists())
    {
        logger_base.debug("Loading schedule %s", (const char *)scheduleFile.GetFullPath().ToStdString().c_str());
        LoadScheduleFile();
        logger_base.debug("Loaded schedule %s", (const char *)scheduleFile.GetFullPath().ToStdString().c_str());
    }
    DisplaySchedule();

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

void xLightsFrame::GetControllerDetailsForChannel(long channel, std::string& type, std::string& description, int& channeloffset, std::string &ip, std::string& u, std::string& inactive, int& output)
{
    type = "Unknown";
    description = "";
    channeloffset = -1;
    ip = "";
    u = "";
    inactive = "";
    output = 0;

    wxXmlNode* e = NetworkXML.GetRoot();
    long currentcontrollerstartchannel = 0;
    long currentcontrollerendchannel = 0;

    for (e = e->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "network")
        {
            output++;
            currentcontrollerstartchannel = currentcontrollerendchannel + 1;
            wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
            long MaxChannels;
            MaxChannelsStr.ToLong(&MaxChannels);
            int ucount = wxAtoi(e->GetAttribute("NumUniverses", "1"));
            currentcontrollerendchannel = currentcontrollerstartchannel + (MaxChannels * ucount) - 1;

            if (channel >= currentcontrollerstartchannel && channel <= currentcontrollerendchannel)
            {
                channeloffset = channel - currentcontrollerstartchannel + 1;
                // found it
                description =  xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description")).ToStdString();
                type = std::string(e->GetAttribute("NetworkType", ""));
                if (type == "NULL")
                {
                    // nothing interesting
                }
                else if (type == "E131")
                {
                    ip = std::string(e->GetAttribute("ComPort", ""));
                    int uu = wxAtoi(e->GetAttribute("BaudRate", ""));
                    if (ucount > 1)
                    {
                        uu += (channel - currentcontrollerstartchannel) / MaxChannels;
                        channeloffset -= (channel - currentcontrollerstartchannel) / MaxChannels * MaxChannels;
                    }
                    u = wxString::Format("%d", uu).ToStdString();
                }
                else if (type == "ArtNet")
                {
                    ip = std::string(e->GetAttribute("ComPort", ""));
                    int uu = wxAtoi(e->GetAttribute("BaudRate", ""));
                    u = wxString::Format("%d:%d:%d", ARTNET_NET(uu), ARTNET_SUBNET(uu), ARTNET_UNIVERSE(uu)).ToStdString();
                }
                else if (type == "DMX")
                {
                    ip = std::string(e->GetAttribute("ComPort", ""));
                    u = std::string(e->GetAttribute("BaudRate", ""));
                    // adjust end channel because this has multiple universes
                    currentcontrollerendchannel = currentcontrollerendchannel + (ucount - 1) * MaxChannels;
                    if (ucount > 1)
                    {
                        int startu = wxAtoi(e->GetAttribute("BaudRate", "1"));
                        int uoffset = channeloffset % MaxChannels;
                        int uu = startu + uoffset;
                        u = std::string(wxString::Format(wxT("%i"), uu));
                        channeloffset = channeloffset - uoffset * MaxChannels;
                    }
                }
                if (e->GetAttribute("Enabled", "Yes") != "Yes")
                {
                    inactive = "TRUE";
                }
                else
                {
                    inactive = "FALSE";
                }
                return;
            }
        }
    }

    output = -1;
}

std::string xLightsFrame::GetChannelToControllerMapping(long channel)
{
	wxXmlNode* e = NetworkXML.GetRoot();
	long currentcontrollerstartchannel = 0;
	long currentcontrollerendchannel = 0;
	int nullcount = 1;

	for (e = e->GetChildren(); e != nullptr; e = e->GetNext())
	{
		if (e->GetName() == "network")
		{
			currentcontrollerstartchannel = currentcontrollerendchannel + 1;
			wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
			long MaxChannels;
			MaxChannelsStr.ToLong(&MaxChannels);
            int universes = wxAtoi(e->GetAttribute("NumUniverses", "1"));
            currentcontrollerendchannel = currentcontrollerstartchannel + (MaxChannels * universes) - 1;

			if (channel >= currentcontrollerstartchannel && channel <= currentcontrollerendchannel)
			{
				int channeloffset = channel - currentcontrollerstartchannel + 1;
				// found it
				std::string s = "Channel " + std::string(wxString::Format(wxT("%i"), (int)channel)) + " maps to ...\n";
				if (e->GetAttribute("Description", "") != "")
				{
					s = s + std::string(xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description")) + "\n");
				}
				std::string type = std::string(e->GetAttribute("NetworkType", ""));
				if (type == "NULL")
				{
					// nothing interesting
					s = s + "Type: NULL ("+ std::string(wxString::Format(wxT("%i"), nullcount++)) +")\nChannel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";
				}
				else if (type == "E131")
				{
					s = s + "Type: E1.31\n";
					std::string ip = std::string(e->GetAttribute("ComPort", ""));
					int u = wxAtoi(e->GetAttribute("BaudRate", ""));
                    if (universes > 1)
                    {
                        u += (channel - currentcontrollerstartchannel) / MaxChannels;
                        channeloffset -= (channel - currentcontrollerstartchannel) / MaxChannels * MaxChannels;
                    }
                    s = s + "IP: " + ip + "\n";
					s = s + "Universe: " + wxString::Format("%d", u).ToStdString() + "\n";
					s = s + "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";
				}
                else if (type == "ArtNet")
                {
                    s = s + "Type: ArtNet\n";
                    std::string ip = std::string(e->GetAttribute("ComPort", ""));
                    int u = wxAtoi(e->GetAttribute("BaudRate", ""));
                    s = s + "IP: " + ip + "\n";
                    s = s + "Net: " + wxString::Format("%d", ARTNET_NET(u)).ToStdString() + "\n";
                    s = s + "Subnet: " + wxString::Format("%d", ARTNET_SUBNET(u)).ToStdString() + "\n";
                    s = s + "Universe: " + wxString::Format("%d", ARTNET_UNIVERSE(u)).ToStdString() + "\n";
                    s = s + "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";
                }
                else if (type == "DMX")
				{
					s = s + "Type: DMX\nComPort: " + std::string(e->GetAttribute("ComPort", "")) + "\n";
					int ucount = wxAtoi(e->GetAttribute("NumUniverses", "1"));
					// adjust end channel because this has multiple universes
					currentcontrollerendchannel = currentcontrollerendchannel + (ucount - 1) * MaxChannels;
					if (ucount > 1)
					{
						int startu = wxAtoi(e->GetAttribute("BaudRate", "1"));
						int uoffset = channeloffset % MaxChannels;
						int u = startu + uoffset;
						s = s + "Universe: " + std::string(wxString::Format(wxT("%i"), u)) + "\n";
						int c = channeloffset - uoffset * MaxChannels;
						s = s + "Channel: " + std::string(wxString::Format(wxT("%i"), c)) + "\n";
					}
					else
					{
						s = s + "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";
					}
				}
				if (e->GetAttribute("Enabled", "Yes") != "Yes")
				{
					s = s + "INACTIVE\n";
				}
				return s;
			}
		}
	}

	return "Channel does not map to a controller.";
}

void xLightsFrame::UpdateNetworkList(bool updateModels)
{
    long newidx,MaxChannels;
    long TotChannels=0;
    int NetCnt=0;
    int StartChannel;
	_totalChannels = 0;

    wxString MaxChannelsStr,NetName,msg;
    wxXmlNode* e=NetworkXML.GetRoot();
    GridNetwork->DeleteAllItems();
    NetInfo.Clear();
    size_t output = 0;
    for( e=e->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "e131sync")
        {
            SpinCtrl_SyncUniverse->SetValue(e->GetAttribute("universe"));
            SetSyncUniverse(SpinCtrl_SyncUniverse->GetValue());
        }
        else if (e->GetName() == "network")
        {
            output++;
            newidx = GridNetwork->InsertItem(GridNetwork->GetItemCount(), wxString::Format("%d", (int)output));
            NetName=e->GetAttribute("NetworkType", "");
            GridNetwork->SetItem(newidx, 1, NetName);
            wxString ip = e->GetAttribute("ComPort", "");
            GridNetwork->SetItem(newidx,2,ip);
            int i = wxAtoi(e->GetAttribute("NumUniverses", "1"));

            MaxChannelsStr=e->GetAttribute("MaxChannels", "0");
            MaxChannelsStr.ToLong(&MaxChannels);
            if (i > 1) {
                int u = wxAtoi(e->GetAttribute("BaudRate", "1"));
                GridNetwork->SetItem(newidx,3,wxString::Format("%d-%d",u,(u + i - 1)));
                MaxChannelsStr = MaxChannelsStr + "x" + e->GetAttribute("NumUniverses");
                if (NetName == "E131")
                {
                    for (int x = 0; x < i; x++)
                    {
                        NetInfo.AddUniverseNetwork(ip, u + x, MaxChannels);
                    }
                }
                MaxChannels *= i;
            } else {
                if (NetName == "ArtNet")
                {
                    int u = wxAtoi(e->GetAttribute("BaudRate", ""));
                    GridNetwork->SetItem(newidx, 3, wxString::Format("%d:%d:%d", ARTNET_NET(u), ARTNET_SUBNET(u), ARTNET_UNIVERSE(u)));
                }
                else
                {
                    GridNetwork->SetItem(newidx, 3, e->GetAttribute("BaudRate", ""));
                    if (NetName == "E131")
                    {
                        int u = wxAtoi(e->GetAttribute("BaudRate", "1"));
                        NetInfo.AddUniverseNetwork(ip, u, MaxChannels);
                    }
                    else
                    {
                        NetInfo.AddUniverseNetwork("", -1, MaxChannels);
                    }
                }
            }
            GridNetwork->SetItem(newidx,4,MaxChannelsStr);

            NetInfo.AddNetwork(MaxChannels);
            StartChannel=TotChannels+1;
            TotChannels+=MaxChannels;
            NetCnt++;

            // Vixen mapping
            msg=wxString::Format(_("Channels %d to %ld"), StartChannel, TotChannels);
            GridNetwork->SetItem(newidx,5,msg);

            GridNetwork->SetItem(newidx,6,e->GetAttribute("Enabled", "Yes"));
			GridNetwork->SetItem(newidx, 7, xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description", "")));
			GridNetwork->SetColumnWidth(7, wxLIST_AUTOSIZE);
            if (e->GetAttribute("Enabled", "Yes") == "No")
            {
                GridNetwork->SetItemTextColour(newidx, *wxLIGHT_GREY);
            }
        }
    }

    GridNetwork->SetColumnWidth(2, NetCnt > 0 ? wxLIST_AUTOSIZE : 100);
	_totalChannels = TotChannels;

    // Now notify the layout as the model start numbers may have been impacted
    if (updateModels) {
        AllModels.RecalcStartChannels();
        layoutPanel->RefreshLayout();
    }
}

// reset test channel listbox
void xLightsFrame::UpdateChannelNames()
{
    wxString FormatSpec,RGBFormatSpec;
    int NodeNum,AbsoluteNodeNum;
    size_t ChannelNum, NodeCount,n,c, ChanPerNode;

    ChNames.clear();
    ChNames.resize(NetInfo.GetTotChannels());
    // update names with RGB models where MyDisplay is checked
#if 0 // Seans code to show absolute channel number

    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "model" && ModelClass::IsMyDisplay(e))
        {
            model.SetFromXml(e);
            NodeCount=model.GetNodeCount();
            ChanPerNode = model.ChannelsPerNode();
            FormatSpec = "Ch %d (RGB Node %d): " + model.name;
            RGBFormatSpec = "Ch %d (RGB Node %d-";
            for(n=0; n < NodeCount; n++)
            {
                ChannelNum=model.NodeStartChannel(n);
                NodeNum=n+1;
                if (ChanPerNode==1) // just skip the one channel code, force us always to go to rgb display
                {
                    if (ChannelNum < ChNames.Count())
                    {
                        AbsoluteNodeNum=ChannelNum+1;
                        AbsoluteNodeNum=(ChannelNum/3)+1; // even on single channel  lights show the rgb node#
                        //   ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,NodeNum);
                        ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,AbsoluteNodeNum);
                    }
                }
                else
                {
                    for(c=0; c < ChanPerNode; c++)
                    {
                        if (ChannelNum < ChNames.Count())
                        {
                            AbsoluteNodeNum=(ChannelNum/3)+1;
                            // ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,NodeNum)+model.GetChannelColorLetter(c);
                            ChNames[ChannelNum] = wxString::Format(RGBFormatSpec,ChannelNum+1,AbsoluteNodeNum) +
                                                  model.GetChannelColorLetter(c) + ") " + model.name;
                        }
                        ChannelNum++;
                    }
                }
            }
        }
    }
    CheckListBoxTestChannels->Set(ChNames);
#endif //

	// KW left as some of the conversions seem to use this
    for (auto it = AllModels.begin(); it != AllModels.end(); it++) {
        Model *model = it->second;
        NodeCount=model->GetNodeCount();
        ChanPerNode = model->GetChanCountPerNode();
        FormatSpec = "Ch %d: "+model->name+" #%d";
        for(n=0; n < NodeCount; n++)
        {
            ChannelNum=model->NodeStartChannel(n);

            NodeNum=n+1;
            if (ChanPerNode==1)
            {
                if (ChannelNum < ChNames.Count())
                {
                    AbsoluteNodeNum=ChannelNum+1;
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
                        AbsoluteNodeNum=(ChannelNum/3)+1;
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
    //CheckListBoxTestChannels->Set(ChNames);
}

void xLightsFrame::OnMenuOpenFolderSelected(wxCommandEvent& event)
{
    PromptForShowDirectory();
}

bool xLightsFrame::PromptForShowDirectory()
{
    wxString newdir;
    if (DirDialog1->ShowModal() == wxID_OK)
    {
        newdir=DirDialog1->GetPath();
        if (newdir == CurrentDir) return true;
        return SetDir(newdir);
    }
    return false;
}

// returns -1 if not found
long xLightsFrame::GetNetworkSelection()
{
    return GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void xLightsFrame::MoveNetworkRow(int fromRow, int toRow)
{
    wxXmlNode* root=NetworkXML.GetRoot();
    wxXmlNode* fromNode = nullptr;
    wxXmlNode* toNode = nullptr;
    int cnt=0;
    //wxMessageBox(wxString::Format("Move from %d to %d",fromRow,toRow));
    for( wxXmlNode* e=root->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        //Network XML can have nodes other than "network" nodes. (like "testpreset") We
        //need to make sure we only consider the "network" nodes.
        if (e->GetName() == "network")
        {
            if (cnt==fromRow) fromNode=e;
            if (cnt==toRow) toNode=e;
            cnt++;
        }
    }
    root->RemoveChild(fromNode);
    if (!toNode)
    {
        root->AddChild(fromNode);
    }
    else if (toRow < fromRow)
    {
        // move up
        root->InsertChild(fromNode,toNode);
    }
    else
    {
        // move down
        root->InsertChildAfter(fromNode,toNode);
    }
    NetworkChange();
    UpdateNetworkList(true);
}

void xLightsFrame::ChangeSelectedNetwork()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (item == -1 || GridNetwork->GetSelectedItemCount() != 1)
    {
        wxMessageBox(_("Please select a single row first"), _("Error"));
        return;
    }

    wxXmlNode* e = GetOutput(item);
    if (e->GetAttribute("NetworkType") == "E131")
    {
        SetupE131(e);
    }
    else if (e->GetAttribute("NetworkType") == "NULL")
    {
        SetupNullOutput(e);
    }
    else if (e->GetAttribute("NetworkType") == "ArtNet")
    {
        SetupArtNet(e);
    }
    else
    {
        SetupDongle(e);
    }
}

void xLightsFrame::OnButtonNetworkChangeClick(wxCommandEvent& event)
{
    ChangeSelectedNetwork();
}

void xLightsFrame::UpdateSelectedIPAddresses()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    wxXmlNode* f = GetOutput(item);
    wxString ip = f->GetAttribute("ComPort", "MULTICAST");

    wxTextEntryDialog dlg(this, "Change controller IP Address", "IP Address", ip);
    if (dlg.ShowModal() == wxID_OK)
    {
        ip = dlg.GetValue();
        CleanupIpAddress(ip);
        while (item != -1)
        {
            wxXmlNode* e = GetOutput(item);
            e->DeleteAttribute("ComPort");
            e->AddAttribute("ComPort", ip);
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

void xLightsFrame::UpdateSelectedDescriptions()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    wxXmlNode* f = GetOutput(item);
    wxString description = f->GetAttribute("Description", "");
    
    wxTextEntryDialog dlg(this, "Change controller description", "Description", description);
    if (dlg.ShowModal() == wxID_OK)
    {
        description = dlg.GetValue();
        while (item != -1)
        {
            wxXmlNode* e = GetOutput(item);
            e->DeleteAttribute("Description");
            e->AddAttribute("Description", description);
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

void xLightsFrame::UpdateSelectedChannels()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    wxXmlNode* f = GetOutput(item);
    int channels = wxAtoi(f->GetAttribute("MaxChannels", "0"));
    wxNumberEntryDialog dlg(this, "Change channels per controller", "Channels", wxEmptyString, channels, 1, 512);
    if (dlg.ShowModal() == wxID_OK)
    {
        channels = dlg.GetValue();
        while (item != -1)
        {
            wxXmlNode* e = GetOutput(item);
            e->DeleteAttribute("MaxChannels");
            e->AddAttribute("MaxChannels", wxString::Format("%d", channels));
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

void xLightsFrame::ActivateSelectedNetworks(bool active)
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        wxXmlNode* e = GetOutput(item);
        e->DeleteAttribute("Enabled");
        e->AddAttribute("Enabled", active ? "Yes" : "No");
        if (xout != nullptr) {
            xout->EnableOutput(item, active);
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

void xLightsFrame::DeleteSelectedNetworks()
{
    int removed = 0;
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        wxXmlNode* e = GetOutput(item - removed);
        NetworkXML.GetRoot()->RemoveChild(e);
        
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
    wxXmlNode* e;
    wxXmlNode* root=NetworkXML.GetRoot();
    while ( (e=root->GetChildren()) != nullptr )
    {
        if (e->GetName() == "network")
        {
            root->RemoveChild(e);
        }
    }
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
    MoveNetworkRow(SelectedItem, SelectedItem-1);
    GridNetwork->SetItemState(SelectedItem-1,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
}

void xLightsFrame::OnButtonNetworkMoveDownClick(wxCommandEvent& event)
{
    long SelectedItem = GetNetworkSelection();
    if (SelectedItem == -1)
    {
        wxMessageBox(_("Please select a single row first"), _("Error"));
        return;
    }
    if (SelectedItem == GridNetwork->GetItemCount()-1) return;
    MoveNetworkRow(SelectedItem, SelectedItem+1);
    GridNetwork->SetItemState(SelectedItem+1,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);
}

// drop a list item (start row is in DragRowIdx)
void xLightsFrame::OnGridNetworkDragEnd(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();  // must reference the event
    int flags = wxLIST_HITTEST_ONITEM;
    long index = GridNetwork->HitTest(pos,flags,nullptr); // got to use it at last
    if(index >= 0 && index != DragRowIdx)
    {
        MoveNetworkRow(DragRowIdx, index);
    }
    // restore cursor
    GridNetwork->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    // disconnect both functions
    GridNetwork->Disconnect(wxEVT_LEFT_UP,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd));
    GridNetwork->Disconnect(wxEVT_LEAVE_WINDOW,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit));
}

void xLightsFrame::OnGridNetworkItemActivated(wxListEvent& event)
{
    ChangeSelectedNetwork();
}


// abort dragging a list item because user has left window
void xLightsFrame::OnGridNetworkDragQuit(wxMouseEvent& event)
{
    // restore cursor and disconnect unconditionally
    GridNetwork->SetCursor(wxCursor(*wxSTANDARD_CURSOR));
    GridNetwork->Disconnect(wxEVT_LEFT_UP,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd));
    GridNetwork->Disconnect(wxEVT_LEAVE_WINDOW,
                            wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit));
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
    // give visual feedback that we are doing something
    GridNetwork->SetCursor(wxCursor(wxCURSOR_HAND));
}

void xLightsFrame::OnButtonAddE131Click(wxCommandEvent& event)
{
    SetupE131(0);
}

void xLightsFrame::OnButtonArtNETClick(wxCommandEvent& event)
{
    SetupArtNet(0);
}

void xLightsFrame::OnButtonAddNullClick(wxCommandEvent& event)
{
    SetupNullOutput(nullptr);
}

wxXmlNode* xLightsFrame::GetOutput(int num)
{
    wxXmlNode* root = NetworkXML.GetRoot();
    long cnt = 0;
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "network")
        {
            if (cnt == num)
            {
                return e;
            }
            else
            {
                cnt++;
            }
        }
    }

    return nullptr;
}

void xLightsFrame::SetupNullOutput(wxXmlNode* e, int after) {
    wxString NetName=_("NULL");

	int numChannels = 512;;
	wxString Description;
    if (e != nullptr) {
		Description = xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description", ""));
		numChannels = wxAtoi(e->GetAttribute("MaxChannels", "512"));
    }
    NullOutputDialog dlg(this);
    dlg.NumChannelsSpinCtrl->SetValue(numChannels);
	dlg.TextCtrl_Description->SetValue(Description);

    if (dlg.ShowModal() == wxID_OK) {
		Description = dlg.TextCtrl_Description->GetValue();
        numChannels = dlg.NumChannelsSpinCtrl->GetValue();
        if (e == nullptr) {
            e = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
            e->AddAttribute("NetworkType", NetName);
            if (after == -1)
            {
                NetworkXML.GetRoot()->AddChild(e);
            }
            else
            {
                NetworkXML.GetRoot()->InsertChildAfter(e, GetOutput(after));
            }
        } else {
            e->DeleteAttribute("MaxChannels");
        }
        wxString LastChannelStr = wxString::Format("%d", numChannels);
		e->AddAttribute("MaxChannels", LastChannelStr);
		e->DeleteAttribute("Description");
		e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
		UpdateNetworkList(true);
        NetworkChange();
    }
}

void xLightsFrame::SetupE131(wxXmlNode* e, int after)
{
    int afterworking = after;
    int DlgResult,UnivNum,NumUniv,LastChannel;
    bool ok=true;
    wxString NetName=_("E131");
    wxString IpAddr,StartUniverse,LastChannelStr,Description;
    E131Dialog E131Dlg(this);

    if (e)
    {
        IpAddr=e->GetAttribute("ComPort");
        CleanupIpAddress(IpAddr);
        StartUniverse=e->GetAttribute("BaudRate");
        LastChannelStr=e->GetAttribute("MaxChannels");
		Description = xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description"));

        NumUniv = wxAtoi(e->GetAttribute("NumUniverses", "1"));
        E131Dlg.SpinCtrl_StartUniv->SetValue(StartUniverse);
        E131Dlg.SpinCtrl_NumUniv->SetValue(NumUniv);
        E131Dlg.MultiE131CheckBox->SetValue(NumUniv > 1);
        E131Dlg.MultiE131CheckBox->Enable(false);
        E131Dlg.SpinCtrl_NumUniv->Enable(NumUniv > 1);
        E131Dlg.SpinCtrl_LastChannel->SetValue(LastChannelStr);
		E131Dlg.TextCtrl_Description->SetValue(Description);

        if (IpAddr.StartsWith( "239.255." ) || IpAddr == "MULTICAST")
        {
            E131Dlg.TextCtrlIpAddr->SetValue("MULTICAST");
            E131Dlg.TextCtrlIpAddr->Enable(false);
            E131Dlg.RadioButtonMulticast->SetValue(true);
        }
        else
        {
            E131Dlg.TextCtrlIpAddr->SetValue(IpAddr);
            E131Dlg.TextCtrlIpAddr->Enable(true);
            E131Dlg.RadioButtonUnicast->SetValue(true);
        }
    }
    do
    {
        DlgResult=E131Dlg.ShowModal();
        if (DlgResult == wxID_OK)
        {
            IpAddr=E131Dlg.TextCtrlIpAddr->GetValue();
            CleanupIpAddress(IpAddr);
            UnivNum = E131Dlg.SpinCtrl_StartUniv->GetValue();
            LastChannel = E131Dlg.SpinCtrl_LastChannel->GetValue();
			Description = E131Dlg.TextCtrl_Description->GetValue();
            //LastChannel = wxString::Format("%d", E131Dlg.SpinCtrl_NumUniv->GetValue() * 512);
            ok=!IpAddr.IsEmpty() && (LastChannel >= 1) && (LastChannel <= 512);
            if (ok)
            {
                LastChannelStr=wxString::Format("%d",LastChannel);
                if (e)
                {
                    e->DeleteAttribute("ComPort");
                    e->AddAttribute("ComPort",IpAddr);
                    e->DeleteAttribute("BaudRate");
                    e->AddAttribute("BaudRate",wxString::Format("%d",UnivNum));
                    e->DeleteAttribute("MaxChannels");
                    e->AddAttribute("MaxChannels",LastChannelStr);
					e->DeleteAttribute("Description");
					e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));

                    e->DeleteAttribute("NumUniverses");
                    if (E131Dlg.MultiE131CheckBox->GetValue()) {
                        e->AddAttribute("NumUniverses", wxString::Format("%d",E131Dlg.SpinCtrl_NumUniv->GetValue()));
                    }
                }
                else
                {
                    NumUniv = E131Dlg.SpinCtrl_NumUniv->GetValue();
                    if (E131Dlg.MultiE131CheckBox->GetValue()) {
                        e = new wxXmlNode( wxXML_ELEMENT_NODE, "network" );
                        e->AddAttribute("NetworkType",NetName);
                        e->AddAttribute("ComPort",IpAddr);
                        e->AddAttribute("BaudRate",wxString::Format("%d",UnivNum));
                        e->AddAttribute("MaxChannels",LastChannelStr);
                        e->AddAttribute("NumUniverses", wxString::Format("%d", NumUniv));
						e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
                        if (after == -1)
                        {
                            NetworkXML.GetRoot()->AddChild(e);
                        }
                        else
                        {
                            NetworkXML.GetRoot()->InsertChildAfter(e, GetOutput(afterworking++));
                        }
                    } else {
                        for (int u=0; u < NumUniv; u++)
                        {
                            e = new wxXmlNode( wxXML_ELEMENT_NODE, "network" );
                            e->AddAttribute("NetworkType",NetName);
                            e->AddAttribute("ComPort",IpAddr);
                            e->AddAttribute("BaudRate",wxString::Format("%d",UnivNum));
                            e->AddAttribute("MaxChannels",LastChannelStr);
							e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
                            if (after == -1)
                            {
                                NetworkXML.GetRoot()->AddChild(e);
                            }
                            else
                            {
                                NetworkXML.GetRoot()->InsertChildAfter(e, GetOutput(afterworking++));
                            }
                            UnivNum++;
                        }
                    }
                }
                UpdateNetworkList(true);
                NetworkChange();
            }
            else
            {
                wxMessageBox(_("All fields must be filled in!"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void xLightsFrame::SetupArtNet(wxXmlNode* e, int after)
{
    int DlgResult;
    //int DlgResult, UnivNum, NumUniv, LastChannel;
    bool ok = true;
    wxString NetName = _("ArtNet");
    wxString IpAddr, StartUniverse, LastChannelStr, Description;
    ArtNetDialog ArtNetDlg(this);

    if (e)
    {
        IpAddr = e->GetAttribute("ComPort");
        CleanupIpAddress(IpAddr);
        StartUniverse = e->GetAttribute("BaudRate");
        LastChannelStr = e->GetAttribute("MaxChannels");
        Description = xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description"));
        int raw = wxAtoi(StartUniverse);
        int net = ARTNET_NET(raw);
        int subnet = ARTNET_SUBNET(raw);
        int universe = ARTNET_UNIVERSE(raw);

        int NumUniv = wxAtoi(e->GetAttribute("NumUniverses", "1"));
        ArtNetDlg.SpinCtrlNet->SetValue(net);
        ArtNetDlg.SpinCtrlSubnet->SetValue(subnet);
        ArtNetDlg.SpinCtrlUniverse->SetValue(universe);
        ArtNetDlg.SpinCtrlChannels->SetValue(LastChannelStr);
        ArtNetDlg.TextCtrlDescription->SetValue(Description);
        ArtNetDlg.TextCtrlIPAddress->SetValue(IpAddr);
        ArtNetDlg.SpinCtrlUniverseOnly->SetValue(ARTNET_MAKEU(net, subnet, universe));
        ArtNetDlg.SpinCtrl_NumUniv->SetValue(NumUniv);
        ArtNetDlg.SpinCtrl_NumUniv->Enable(NumUniv > 1);
    }

    do
    {
        DlgResult = ArtNetDlg.ShowModal();
        if (DlgResult == wxID_OK)
        {
            IpAddr = ArtNetDlg.TextCtrlIPAddress->GetValue();
            CleanupIpAddress(IpAddr);
            int Net = ArtNetDlg.SpinCtrlNet->GetValue();
            int Subnet = ArtNetDlg.SpinCtrlSubnet->GetValue();
            int Universe = ArtNetDlg.SpinCtrlUniverse->GetValue();
            int LastChannel = ArtNetDlg.SpinCtrlChannels->GetValue();
            Description = ArtNetDlg.TextCtrlDescription->GetValue();
            ok = !IpAddr.IsEmpty() && (LastChannel >= 1) && (LastChannel <= 512);
            if (ok)
            {
                LastChannelStr = wxString::Format("%d", LastChannel);
                if (e)
                {
                    e->DeleteAttribute("ComPort");
                    e->AddAttribute("ComPort", IpAddr);
                    e->DeleteAttribute("BaudRate");
                    e->AddAttribute("BaudRate", wxString::Format("%d", ARTNET_MAKEU(Net, Subnet, Universe)));
                    e->DeleteAttribute("MaxChannels");
                    e->AddAttribute("MaxChannels", LastChannelStr);
                    e->DeleteAttribute("Description");
                    e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
                }
                else
                {
                    int NumUniv = ArtNetDlg.SpinCtrl_NumUniv->GetValue();
                    int univ = ARTNET_MAKEU(Net, Subnet, Universe);
                    for (int u = 0; u < NumUniv; u++)
                    {
                        e = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
                        e->AddAttribute("NetworkType", NetName);
                        e->AddAttribute("ComPort", IpAddr);
                        e->AddAttribute("MaxChannels", LastChannelStr);
                        e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
                        if (after == -1)
                        {
                            e->AddAttribute("BaudRate", wxString::Format("%d", univ + u));
                            NetworkXML.GetRoot()->AddChild(e);
                        }
                        else
                        {
                            e->AddAttribute("BaudRate", wxString::Format("%d", univ + NumUniv - 1 - u));
                            NetworkXML.GetRoot()->InsertChildAfter(e, GetOutput(after));
                        }
                    }
                }
                UpdateNetworkList(true);
                NetworkChange();
            }
            else
            {
                wxMessageBox(_("All fields must be filled in!"), _("ERROR"));
            }
        }
    } while (DlgResult == wxID_OK && !ok);
}

void xLightsFrame::SetupDongle(wxXmlNode* e, int after)
{
    int DlgResult;
    bool ok=true;
    wxString NetName,Port,BaudRate,LastChannel,Description;
    SerialPortWithRate SerialDlg(this);

    if (e)
    {
        NetName=e->GetAttribute("NetworkType");
        Port=e->GetAttribute("ComPort");
        BaudRate=e->GetAttribute("BaudRate");
        LastChannel=e->GetAttribute("MaxChannels");
		Description = xLightsXmlFile::UnXmlSafe(e->GetAttribute("Description"));
        SerialDlg.ChoiceProtocol->SetStringSelection(NetName); //preserve network type -DJ
        SerialDlg.ChoicePort->SetStringSelection(Port);
        SerialDlg.ChoiceBaudRate->SetStringSelection(BaudRate);
		SerialDlg.TextCtrlLastChannel->SetValue(LastChannel);
		SerialDlg.TextCtrl_Description->SetValue(Description);
	}
    SerialDlg.ProtocolChange();

    do
    {
        DlgResult=SerialDlg.ShowModal();
        if (DlgResult == wxID_OK)
        {
            ok=false;
            NetName=SerialDlg.ChoiceProtocol->GetStringSelection();
            Port=SerialDlg.ChoicePort->GetStringSelection();
            BaudRate=SerialDlg.GetRateString();
            LastChannel=SerialDlg.TextCtrlLastChannel->GetValue();
			Description = SerialDlg.TextCtrl_Description->GetValue();
            if (Port.IsEmpty() || BaudRate.IsEmpty() || LastChannel.IsEmpty())
            {
                wxMessageBox(_("All fields must be filled in!"), _("ERROR"));
            }
            else if (!LastChannel.IsNumber() || LastChannel[0]=='-')
            {
                wxMessageBox(_("Last Channel must be a positive number!"), _("ERROR"));
            }
            else
            {
                if (!e)
                {
                    e = new wxXmlNode( wxXML_ELEMENT_NODE, "network" );
                    if (after == -1)
                    {
                        NetworkXML.GetRoot()->AddChild(e);
                    }
                    else
                    {
                        NetworkXML.GetRoot()->InsertChildAfter(e, GetOutput(after));
                    }
                }
                e->DeleteAttribute("NetworkType");
                e->AddAttribute("NetworkType",NetName);
                e->DeleteAttribute("ComPort");
                e->AddAttribute("ComPort",Port);
                e->DeleteAttribute("BaudRate");
                e->AddAttribute("BaudRate",BaudRate);
                e->DeleteAttribute("MaxChannels");
                e->AddAttribute("MaxChannels",LastChannel);
				e->DeleteAttribute("Description");
				e->AddAttribute("Description", xLightsXmlFile::XmlSafe(Description));
                UpdateNetworkList(true);
                NetworkChange();
                ok=true;
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void xLightsFrame::OnButtonAddDongleClick(wxCommandEvent& event)
{
    SetupDongle(0);
}

std::list<wxXmlNode> xLightsFrame::GetOutputsForController(const std::string onlyip)
{
    std::list<wxXmlNode> res;

    wxXmlNode* e = NetworkXML.GetRoot();
    long count = 1;

    for (e = e->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "network")
        {
            std::string type = std::string(e->GetAttribute("NetworkType", ""));

            if (type == "E131" || type == "ArtNet")
            {
                std::string ip = std::string(e->GetAttribute("ComPort", ""));
                if (onlyip == "" || ip == onlyip)
                {
                    res.push_back(*e);
                }
            }
        }
    }

    return res;
}

void xLightsFrame::NetworkChange()
{
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
    if (NetworkXML.Save( networkFile.GetFullPath() ))
    {
        UnsavedNetworkChanges=false;
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
    if (xout != nullptr)
    {
        xout->SetSyncUniverse(syncUniverse);
    }
    else
    {
        xNetwork_E131::SetSyncUniverseStatic(syncUniverse);
    }

    bool addneeded = true;
    wxXmlNode* root = NetworkXML.GetRoot();
    if (root == nullptr) {
        return;
    }
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
    {
        if (e->GetName() == "e131sync")
        {
            if (e->GetAttribute("universe") == wxString::Format("%d", syncUniverse))
            {
                addneeded = false;
                break;
            }
            else
            {
                root->RemoveChild(e);
                NetworkChange();
                break;
            }
        }
    }

    if (addneeded && syncUniverse != 0)
    {
        wxXmlNode* newNode = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "e131sync");
        newNode->AddAttribute("universe", wxString::Format("%d", syncUniverse));
        root->AddChild(newNode);
        NetworkChange();
    }
}

void xLightsFrame::OnSpinCtrl_SyncUniverseChange(wxSpinEvent& event)
{
    SetSyncUniverse(SpinCtrl_SyncUniverse->GetValue());
}

bool xLightsFrame::AllSelectedSupportIP()
{
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    while (item != -1)
    {
        wxXmlNode* e = GetOutput(item);

        wxString type = e->GetAttribute("NetworkType", "");

        if (type != "E131" && type != "ArtNet")
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
        wxXmlNode* e = GetOutput(item);

        wxString type = e->GetAttribute("NetworkType", "");

        if (type == "NULL")
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
                    wxXmlNode* e = GetOutput(item);
                    wxString thisip = e->GetAttribute("ComPort", "");

                    if (thisip == "MULTICAST")
                    {
                    }
                    else if (ip != thisip)
                    {
                        if (ip == "")
                        {
                            ip = thisip;
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
                    wxXmlNode* e = GetOutput(item);
                    wxString thisip = e->GetAttribute("ComPort", "");

                    if (thisip == "MULTICAST")
                    {                            
                    }
                    else if (ip != thisip)
                    {
                        if (ip == "")
                        {
                            ip = thisip;
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

        mnuUploadController->Append(ID_NETWORK_UCINPUT, "E1.31 Input Defintion", mnuUCInput, "");
        
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
                    wxXmlNode* e = GetOutput(item);
                    wxString thisip = e->GetAttribute("ComPort", "");

                    if (thisip == "MULTICAST")
                    {
                    }
                    else if (ip != thisip)
                    {
                        if (ip == "")
                        {
                            ip = thisip;
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

        mnuUploadController->Append(ID_NETWORK_UCOUTPUT, "Output", mnuUCOutput, "");
     
    mnu.Append(ID_NETWORK_UPLOADCONTROLLER, "Upload To Controller", mnuUploadController, "");
    
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

    mnu.Append(ID_NETWORK_ADD, "Insert After", mnuAdd, "");
    mnu.Append(ID_NETWORK_BULKEDIT, "Bulk Edit", mnuBulkEdit, "");
    mnu.AppendSeparator();

    mnu.Append(ID_NETWORK_DELETE, "Delete")->Enable(selcnt > 0);
    mnu.Append(ID_NETWORK_ACTIVATE, "Activate")->Enable(selcnt > 0);
    mnu.Append(ID_NETWORK_DEACTIVATE, "Deactivate")->Enable(selcnt > 0);
    wxMenuItem* oc = mnu.Append(ID_NETWORK_OPENCONTROLLER, "Open Controller");
    oc->Enable(selcnt == 1);
    if (!AllSelectedSupportIP())
    {
        oc->Enable(false);
    }
    else
    {
        int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item != -1) {
            wxXmlNode* e = GetOutput(item);
            if (e->GetAttribute("ComPort", "") == "MULTICAST")
            {
                oc->Enable(false);
            }
        }
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnNetworkPopup, NULL, this);
    PopupMenu(&mnu);
    GridNetwork->SetFocus();
}

void xLightsFrame::OnNetworkPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (id == ID_NETWORK_ADDUSB)
    {
        SetupDongle(0, item);
    }
    else if (id == ID_NETWORK_ADDNULL)
    {
        SetupNullOutput(0, item);
    }
    else if (id == ID_NETWORK_ADDE131)
    {
        SetupE131(0, item);
    }
    else if (id == ID_NETWORK_ADDARTNET)
    {
        SetupArtNet(0, item);
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
    else if (id == ID_NETWORK_OPENCONTROLLER)
    {
        wxXmlNode* e = GetOutput(item);
        wxString ip = e->GetAttribute("ComPort", "");
        ::wxLaunchDefaultBrowser("http://" + ip);
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
        wxXmlNode* e = GetOutput(item);
        wxString thisip = e->GetAttribute("ComPort", "");

        if (thisip == "MULTICAST")
        {
        }
        else if (ip != thisip)
        {
            if (ip == "")
            {
                ip = thisip;
            }
        }

        item = GridNetwork->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return selected;
}

void xLightsFrame::UploadFPPBridgeInput()
{
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

        // now upload it
        wxConfigBase* config = wxConfigBase::Get();
        wxString user;
        config->Read("xLightsPiUser", &user, "fpp");

        wxString password = "";
        bool usedefaultpwd;
        config->Read("xLightsPiDefaultPassword", &usedefaultpwd, true);

        if (usedefaultpwd)
        {
            if (user == "pi")
            {
                password = "raspberry";
            }
            else if (user == "fpp")
            {
                password = "falcon";
            }
            else
            {
                wxTextEntryDialog ted(this, "Enter password for " + user, "Password", ip);
                if (ted.ShowModal() == wxID_OK)
                {
                    password = ted.GetValue();
                }
            }
        }
        else
        {
            wxTextEntryDialog ted(this, "Enter password for " + user, "Password", ip);
            if (ted.ShowModal() == wxID_OK)
            {
                password = ted.GetValue();
            }
        }

        FPP fpp(ip.ToStdString(), user.ToStdString(), password.ToStdString());

        if (fpp.IsConnected())
        {
            fpp.SetInputUniversesBridge(NetworkXML.GetRoot(), selected, this);
        }
        SetCursor(wxCURSOR_ARROW);
    }
}

void xLightsFrame::UploadFPPBridgeOutput()
{
    wxMessageBox("Not implemented");
}

void xLightsFrame::UploadFalconInput()
{
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
            falcon.SetInputUniverses(NetworkXML.GetRoot(), selected);
        }
        SetCursor(wxCURSOR_ARROW);
    }
}
void xLightsFrame::UploadFalconOutput()
{
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
            falcon.SetOutputs(&AllModels, NetworkXML.GetRoot(), selected, this);
        }
        SetCursor(wxCURSOR_ARROW);
    }
}
