
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
#include <wx/dir.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/valnum.h>
#include <wx/artprov.h>

// dialogs
#include "SerialPortWithRate.h"
#include "E131Dialog.h"
#include "NullOutputDialog.h"

// Process Setup Panel Events

void xLightsFrame::OnMenuMRU(wxCommandEvent& event)
{
    int id = event.GetId();
    wxString newdir = MenuFile->GetLabel(id);
    SetDir(newdir);
}

void xLightsFrame::SetDir(const wxString& newdir)
{
    static bool HasMenuSeparator=false;
    int idx, cnt, i;

    // don't change show directories with an open sequence because models won't match
    CloseSequence();

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
        return;
    }

    // Check to see if any show directory files need to be saved
    CheckUnsavedChanges();

    // Force update of Preset dialog
    if( EffectTreeDlg != NULL ) {
        delete EffectTreeDlg;
    }
    EffectTreeDlg = NULL;

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
        if (mru_MenuItem[i] != NULL)
        {
            Disconnect(mru_MenuItem[i]->GetId(), wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xLightsFrame::OnMenuMRU);
            MenuFile->Delete(mru_MenuItem[i]);
            mru_MenuItem[i] = NULL;
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
        return;
    }

    // update UI
    CheckBoxLightOutput->SetValue(false);
    CheckBoxRunSchedule->SetValue(false);
    if (xout)
    {
        delete xout;
        xout=0;
    }
    CurrentDir=newdir;
    showDirectory=newdir;

    long LinkFlag=0;
    config->Read(_("LinkFlag"), &LinkFlag);
    if( LinkFlag ) {
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_LINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(false);
        mediaDirectory = CurrentDir;
        config->Write(_("MediaDir"), mediaDirectory);
        MediaDirectoryLabel->SetLabel(mediaDirectory);
        MediaDirectoryLabel->GetParent()->Layout();
    } else {
        BitmapButton_Link_Dirs->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_UNLINK")),wxART_OTHER));
        Button_Change_Media_Dir->Enable(true);
    }

    TextCtrlLog->Clear();
    while (Notebook1->GetPageCount() > FixedPages)
    {
        Notebook1->DeletePage(FixedPages);
    }
//    ButtonTestLoad->Enable(true);
//    ButtonTestSave->Enable(true);
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
        LoadScheduleFile();
    }
    DisplaySchedule();

    // load sequence effects
//~    EffectsPanel1->SetDefaultPalette();
//~    EffectsPanel2->SetDefaultPalette();
    UpdateNetworkList();
    LoadEffectsFile();

    wxFileName kbf;
    kbf.AssignDir(CurrentDir);
    kbf.SetFullName("xlights_keybindings.xml");
    mainSequencer->keyBindings.Load(kbf);

    EnableSequenceControls(true);

    Notebook1->ChangeSelection(SETUPTAB);
    SetStatusText("");
    FileNameText->SetLabel(newdir);
}

std::string xLightsFrame::GetChannelToControllerMapping(long channel)
{
	wxXmlNode* e = NetworkXML.GetRoot();
	long currentcontrollerstartchannel = 0;
	long currentcontrollerendchannel = 0;
	int nullcount = 1;

	for (e = e->GetChildren(); e != NULL; e = e->GetNext())
	{
		if (e->GetName() == "network")
		{
			currentcontrollerstartchannel = currentcontrollerendchannel + 1;
			wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
			long MaxChannels;
			MaxChannelsStr.ToLong(&MaxChannels);
			currentcontrollerendchannel = currentcontrollerstartchannel + MaxChannels - 1;

			if (channel >= currentcontrollerstartchannel && channel <= currentcontrollerendchannel)
			{
				int channeloffset = channel - currentcontrollerstartchannel + 1;
				// found it
				std::string s = "Channel " + std::string(wxString::Format(wxT("%i"), channel)) + " maps to ...\n";
				if (e->GetAttribute("Description", "") != "")
				{
					s = s + std::string(e->GetAttribute("Description")) + "\n";
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
					std::string u = std::string(e->GetAttribute("BaudRate", ""));
					s = s + "IP: " + ip + "\n";
					s = s + "Universe: " + u + "\n";
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

void xLightsFrame::UpdateNetworkList()
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
    for( e=e->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "network")
        {
            NetName=e->GetAttribute("NetworkType", "");
            newidx = GridNetwork->InsertItem(GridNetwork->GetItemCount(), NetName);
            GridNetwork->SetItem(newidx,1,e->GetAttribute("ComPort", ""));
            int i = wxAtoi(e->GetAttribute("NumUniverses", "1"));

            MaxChannelsStr=e->GetAttribute("MaxChannels", "0");
            MaxChannelsStr.ToLong(&MaxChannels);
            if (i > 1) {
                int u = wxAtoi(e->GetAttribute("BaudRate", "1"));
                GridNetwork->SetItem(newidx,2,wxString::Format("%d-%d",u,(u + i - 1)));
                MaxChannelsStr = MaxChannelsStr + "x" + e->GetAttribute("NumUniverses");
                MaxChannels *= i;
            } else {
                GridNetwork->SetItem(newidx,2,e->GetAttribute("BaudRate", ""));
            }
            GridNetwork->SetItem(newidx,3,MaxChannelsStr);

            NetInfo.AddNetwork(MaxChannels);
            StartChannel=TotChannels+1;
            TotChannels+=MaxChannels;
            NetCnt++;

            // Vixen mapping
            msg=wxString::Format(_("Channels %d to %ld"), StartChannel, TotChannels);
            GridNetwork->SetItem(newidx,4,msg);

            GridNetwork->SetItem(newidx,5,e->GetAttribute("Enabled", "Yes"));
			GridNetwork->SetItem(newidx, 6, e->GetAttribute("Description", ""));
			GridNetwork->SetColumnWidth(6, wxLIST_AUTOSIZE);
		}
    }
    //GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE);
    GridNetwork->SetColumnWidth(1,NetCnt > 0 ? wxLIST_AUTOSIZE : 100);
	_totalChannels = TotChannels;
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

    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
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

void xLightsFrame::PromptForShowDirectory()
{
    wxString newdir;
    if (DirDialog1->ShowModal() == wxID_OK)
    {
        newdir=DirDialog1->GetPath();
        if (newdir == CurrentDir) return;
        SetDir(newdir);
    }
}

// returns -1 if not found
long xLightsFrame::GetNetworkSelection()
{
    return GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void xLightsFrame::MoveNetworkRow(int fromRow, int toRow)
{
    wxXmlNode* root=NetworkXML.GetRoot();
    wxXmlNode* fromNode = NULL;
    wxXmlNode* toNode = NULL;
    int cnt=0;
    //wxMessageBox(wxString::Format("Move from %d to %d",fromRow,toRow));
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
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
    UnsavedNetworkChanges=true;
    UpdateNetworkList();
}

void xLightsFrame::OnButtonNetworkChangeClick(wxCommandEvent& event)
{
    long SelectedItem = GetNetworkSelection();
    if (SelectedItem == -1)
    {
        wxMessageBox(_("Please select a single row first"), _("Error"));
        return;
    }
    wxXmlNode* root=NetworkXML.GetRoot();
    long cnt=0;
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "network")
        {
            if (cnt==SelectedItem)
            {
                if (e->GetAttribute("NetworkType") == "E131")
                {
                    SetupE131(e);
                }
                else if (e->GetAttribute("NetworkType") == "NULL")
                {
                    SetupNullOutput(e);
                }
                else
                {
                    SetupDongle(e);
                }
                break;
            }
            else
            {
                cnt++;
            }
        }
    }
}

void xLightsFrame::OnButtonNetworkDeleteClick(wxCommandEvent& event)
{
    long SelectedItem = GetNetworkSelection();
    if (SelectedItem == -1)
    {
        wxMessageBox(_("Please select a single row first"), _("Error"));
        return;
    }
    wxXmlNode* root=NetworkXML.GetRoot();
    long cnt=0;
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "network")
        {
            if (cnt==SelectedItem)
            {
                root->RemoveChild(e);
                break;
            }
            else
            {
                cnt++;
            }
        }
    }
    UnsavedNetworkChanges=true;
    UpdateNetworkList();
    cnt=GridNetwork->GetItemCount();
    if (cnt > 0)
    {
        GridNetwork->SetItemState(cnt <= SelectedItem ? cnt-1 : SelectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

void xLightsFrame::OnButtonNetworkDeleteAllClick(wxCommandEvent& event)
{
    wxXmlNode* e;
    wxXmlNode* root=NetworkXML.GetRoot();
    while ( (e=root->GetChildren()) != NULL )
    {
        root->RemoveChild(e);
    }
    UnsavedNetworkChanges=true;
    UpdateNetworkList();
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
    long index = GridNetwork->HitTest(pos,flags,NULL); // got to use it at last
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
    int item = GridNetwork->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1) {
        return;
    }
    int i = item;
    wxXmlNode* e=NetworkXML.GetRoot();
    GridNetwork->DeleteAllItems();
    NetInfo.Clear();
    for( e=e->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "network") {
            item--;
            if (item == -1) {
                wxString b = e->GetAttribute("Enabled", "Yes");
                e->DeleteAttribute("Enabled");
                e->AddAttribute("Enabled", b == "Yes" ? "No" : "Yes");
                if (xout != nullptr) {
                    xout->EnableOutput(i, b == "No");
                }
                UpdateNetworkList();
                return;
            }
        }
    }

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
                         wxMouseEventHandler(xLightsFrame::OnGridNetworkDragEnd), NULL,this);
    // trigger when user leaves window to abort drag
    GridNetwork->Connect(wxEVT_LEAVE_WINDOW,
                         wxMouseEventHandler(xLightsFrame::OnGridNetworkDragQuit), NULL,this);
    // give visual feedback that we are doing something
    GridNetwork->SetCursor(wxCursor(wxCURSOR_HAND));
}

void xLightsFrame::OnButtonAddE131Click(wxCommandEvent& event)
{
    SetupE131(0);
}


void xLightsFrame::OnButtonAddNullClick(wxCommandEvent& event)
{
    SetupNullOutput(nullptr);
}

void xLightsFrame::SetupNullOutput(wxXmlNode* e) {
    wxString NetName=_("NULL");

	int numChannels = 512;;
	wxString Description;
    if (e != nullptr) {
		Description = e->GetAttribute("Description", "");
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
            NetworkXML.GetRoot()->AddChild(e);
        } else {
            e->DeleteAttribute("MaxChannels");
        }
        wxString LastChannelStr = wxString::Format("%d", numChannels);
		e->AddAttribute("MaxChannels", LastChannelStr);
		e->DeleteAttribute("Description");
		e->AddAttribute("Description", Description);
		UpdateNetworkList();
        UnsavedNetworkChanges=true;
    }
}

void xLightsFrame::SetupE131(wxXmlNode* e)
{
    int DlgResult,UnivNum,NumUniv,LastChannel;
    bool ok=true;
    wxString NetName=_("E131");
    wxString IpAddr,StartUniverse,LastChannelStr,Description;
    E131Dialog E131Dlg(this);

    if (e)
    {
        IpAddr=e->GetAttribute("ComPort");
        StartUniverse=e->GetAttribute("BaudRate");
        LastChannelStr=e->GetAttribute("MaxChannels");
		Description = e->GetAttribute("Description");

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
					e->AddAttribute("Description", Description);

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
						e->AddAttribute("Description", Description);
                        NetworkXML.GetRoot()->AddChild(e);
                    } else {
                        for (int u=0; u < NumUniv; u++)
                        {
                            e = new wxXmlNode( wxXML_ELEMENT_NODE, "network" );
                            e->AddAttribute("NetworkType",NetName);
                            e->AddAttribute("ComPort",IpAddr);
                            e->AddAttribute("BaudRate",wxString::Format("%d",UnivNum));
                            e->AddAttribute("MaxChannels",LastChannelStr);
							e->AddAttribute("Description", Description);
                            NetworkXML.GetRoot()->AddChild(e);
                            UnivNum++;
                        }
                    }
                }
                UpdateNetworkList();
                UnsavedNetworkChanges=true;
            }
            else
            {
                wxMessageBox(_("All fields must be filled in!"), _("ERROR"));
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
}

void xLightsFrame::SetupDongle(wxXmlNode* e)
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
		Description = e->GetAttribute("Description");
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
                    NetworkXML.GetRoot()->AddChild(e);
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
				e->AddAttribute("Description", Description);
                UpdateNetworkList();
                UnsavedNetworkChanges=true;
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

void xLightsFrame::SaveFPPUniverses(std::string path)
{
    wxFile universes;
    universes.Open(path + "/universes", wxFile::write);

    if (universes.IsOpened())
    {
        wxXmlNode* e = NetworkXML.GetRoot();
        long count = 1;

        for (e = e->GetChildren(); e != NULL; e = e->GetNext())
        {
            if (e->GetName() == "network")
            {
                std::string type = std::string(e->GetAttribute("NetworkType", ""));

                if (type == "E131")
                {
                    std::string ip = std::string(e->GetAttribute("ComPort", ""));
                    std::string universe = std::string(e->GetAttribute("BaudRate", ""));
                    wxString MaxChannelsStr = e->GetAttribute("MaxChannels", "0");
                    long chan;
                    MaxChannelsStr.ToLong(&chan);
                    long end = count + chan - 1;

                    if (ip == "MULTICAST")
                    {
                        universes.Write("1," + universe + "," + std::string(wxString::Format(wxT("%i"), count)) + "," + std::string(wxString::Format(wxT("%i"), chan)) + ",0,,\r\n");
                    }
                    else
                    {
                        universes.Write("1," + universe + "," + std::string(wxString::Format(wxT("%i"), count)) + "," + std::string(wxString::Format(wxT("%i"), chan)) + ",1," + ip + ",\r\n");
                    }

                    count = end + 1;
                }
            }
        }
        universes.Close();
    }
}

bool xLightsFrame::SaveNetworksFile()
{
    SaveFPPUniverses(std::string(networkFile.GetPath().c_str()));
    if (NetworkXML.Save( networkFile.GetFullPath() ))
    {
        UnsavedNetworkChanges=false;
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
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("MediaDir"), mediaDirectory);
        MediaDirectoryLabel->SetLabel(mediaDirectory);
        MediaDirectoryLabel->GetParent()->Layout();
    }
}

