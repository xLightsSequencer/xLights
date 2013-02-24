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

    // reject change if something is playing
    if (play_mode == play_sched || play_mode == play_list || play_mode == play_single)
    {
        wxMessageBox(_("Cannot change directories during playback"),_("Error"));
        return;
    }

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
    wxString msg=wxT("UpdateMRU:\n");
    for (int i=0; i<mru.GetCount(); i++) msg+=wxT("\n") + mru[i];
    wxMessageBox(msg);
    */

    // save config
    bool DirExists=wxFileName::DirExists(newdir);
    wxString mru_name, value;
    wxConfig* config = new wxConfig(_(XLIGHTS_CONFIG_ID));
    if (DirExists) config->Write(_("LastDir"), newdir);
    for (i=0; i<MRU_LENGTH; i++)
    {
        mru_name=wxString::Format(wxT("mru%d"),i);
        if (MenuFile->FindItem(mru_MenuItem[i]->GetItemLabel()) != wxNOT_FOUND)
        {
            MenuFile->Remove(mru_MenuItem[i]);
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
    delete config;

    // append mru items to menu
    cnt=mru.GetCount();
    if (!HasMenuSeparator && cnt > 0)
    {
        MenuFile->AppendSeparator();
        HasMenuSeparator=true;
    }
    for (i=0; i<cnt; i++)
    {
        mru_MenuItem[i]->SetItemLabel(mru[i]);
        MenuFile->Append(mru_MenuItem[i]);
    }

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
    StaticTextDirName->SetLabel(CurrentDir);
    UnsavedChanges=false;
    TextCtrlLog->Clear();
    while (Notebook1->GetPageCount() > FixedPages)
    {
        Notebook1->DeletePage(FixedPages);
    }
    ButtonTestLoad->Enable(true);
    ButtonTestSave->Enable(true);
    EnableNetworkChanges();
    DisplayXlightsFilename(wxT(""));

    // load network
    wxXmlNode* root = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("Networks") );
    root->AddAttribute( wxT("computer"), wxGetHostName());
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
    wxWindow* w;
    int ColorIdx;
    for(i=0; i<12; i++)
    {
        w=wxWindow::FindWindowByName(wxString::Format(wxT("ID_BUTTON_Palette%d_%d"),int(i/6+1),int(i%6+1)));
        switch (i%6)
        {
        case 0:
            w->SetBackgroundColour(*wxRED);
            break;
        case 1:
            w->SetBackgroundColour(*wxGREEN);
            break;
        case 2:
            w->SetBackgroundColour(*wxBLUE);
            break;
        case 3:
            w->SetBackgroundColour(*wxYELLOW);
            break;
        case 4:
            w->SetBackgroundColour(*wxWHITE);
            break;
        case 5:
            w->SetBackgroundColour(*wxBLACK);
            break;
        }
        SetTextColor(w);
    }
    LoadEffectsFile();
    PresetsSelect();
    EnableSequenceControls(true);
    UpdateNetworkList();

    Notebook1->ChangeSelection(SETUPTAB);
}

void xLightsFrame::UpdateNetworkList()
{
    long newidx,MaxChannels;
    long TotChannels=0;
    int NetCnt=0;
    //int MaxLorChannels=240*16;
    int MaxDmxChannels=512;
    int StartChannel,ch;
    char c;
    wxArrayString ChNames;

    wxString MaxChannelsStr,NetName,msg;
    wxXmlNode* e=NetworkXML.GetRoot();
    GridNetwork->DeleteAllItems();
    NetInfo.Clear();
    for( e=e->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("network"))
        {
            NetName=e->GetAttribute(wxT("NetworkType"), wxT(""));
            newidx = GridNetwork->InsertItem(GridNetwork->GetItemCount(), NetName);
            GridNetwork->SetItem(newidx,1,e->GetAttribute(wxT("ComPort"), wxT("")));
            GridNetwork->SetItem(newidx,2,e->GetAttribute(wxT("BaudRate"), wxT("")));
            MaxChannelsStr=e->GetAttribute(wxT("MaxChannels"), wxT("0"));
            GridNetwork->SetItem(newidx,3,MaxChannelsStr);
            MaxChannelsStr.ToLong(&MaxChannels);
            NetInfo.AddNetwork(MaxChannels);
            StartChannel=TotChannels+1;
            TotChannels+=MaxChannels;
            NetCnt++;

            // LOR mapping
            if (NetCnt==1)
            {
                msg=_("LOR Regular");
                if (TotChannels <= MaxDmxChannels) msg+=_(" or Univ 1");
            }
            else if (NetCnt <= 16)
            {
                c = NetCnt - 2 + 'A';
                msg=wxString::Format(_("LOR Aux %c"),c);
                if (MaxChannels <= MaxDmxChannels) msg+=wxString::Format(_(" or Univ %d"), NetCnt);
            }
            else if (MaxChannels <= MaxDmxChannels)
            {
                msg=wxString::Format(_("Univ %d"), NetCnt);
            }
            else
            {
                msg=_("Not mappable");
            }
            GridNetwork->SetItem(newidx,4,msg);

            // Vixen mapping
            msg=wxString::Format(_("Channels %d to %d"), StartChannel, TotChannels);
            GridNetwork->SetItem(newidx,5,msg);
        }
    }
    //GridNetwork->SetColumnWidth(0,wxLIST_AUTOSIZE);
    GridNetwork->SetColumnWidth(1,NetCnt > 0 ? wxLIST_AUTOSIZE : 100);

    // reset test channel listbox
    NetInfo.GetAllChannelNames(ChNames);
    // update names with RGB models where MyDisplay is checked
    wxString MyDisplay;
    for(e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == wxT("model"))
        {
            MyDisplay=e->GetAttribute(wxT("MyDisplay"));
            if (MyDisplay == wxT("1"))
            {
                SetChannelNamesForRgbModel(ChNames,e);
            }
        }
    }
    CheckListBoxTestChannels->Set(ChNames);
}

void xLightsFrame::SetChannelNamesForRgbModel(wxArrayString& ChNames, wxXmlNode* ModelNode)
{
    ModelClass model;
    model.SetFromXml(ModelNode);
    size_t ChannelNum=model.StartChannel-1;
    size_t NodeCount=model.GetNodeCount();
    wxString FormatSpec = wxT("Ch %d: ")+model.name+wxT(" node %d %c");
    for(size_t i=0; i < NodeCount && ChannelNum+2 < ChNames.Count(); i++)
    {
        for(size_t j=0; j < 3; j++)
        {
            ChNames[ChannelNum] = wxString::Format(FormatSpec,ChannelNum+1,i+1,model.RGBorder[j]);
            ChannelNum++;
        }
    }
}

void xLightsFrame::OnMenuOpenFolderSelected(wxCommandEvent& event)
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
    wxXmlNode* fromNode;
    wxXmlNode* toNode;
    int cnt=0;
    //wxMessageBox(wxString::Format(wxT("Move from %d to %d"),fromRow,toRow));
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (cnt==fromRow) fromNode=e;
        if (cnt==toRow) toNode=e;
        cnt++;
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
    UnsavedChanges=true;
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
        if (cnt==SelectedItem)
        {
            if (e->GetAttribute(wxT("NetworkType")) == wxT("E131"))
            {
                SetupE131(e);
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
    UnsavedChanges=true;
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
    while ( e=root->GetChildren() )
    {
        root->RemoveChild(e);
    }
    UnsavedChanges=true;
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

void xLightsFrame::SetupE131(wxXmlNode* e)
{
    int DlgResult,UnivNum,NumUniv,LastChannel;
    bool ok=true;
    wxString NetName=_("E131");
    wxString IpAddr,StartUniverse,LastChannelStr;
    E131Dialog E131Dlg(this);

    if (e)
    {
        IpAddr=e->GetAttribute(wxT("ComPort"));
        StartUniverse=e->GetAttribute(wxT("BaudRate"));
        LastChannelStr=e->GetAttribute(wxT("MaxChannels"));
        E131Dlg.SpinCtrl_StartUniv->SetValue(StartUniverse);
        E131Dlg.SpinCtrl_NumUniv->SetValue(1);
        E131Dlg.SpinCtrl_NumUniv->Enable(false);
        E131Dlg.SpinCtrl_LastChannel->SetValue(LastChannelStr);

        if (IpAddr.StartsWith( wxT("239.255.") ) || IpAddr == wxT("MULTICAST"))
        {
            E131Dlg.TextCtrlIpAddr->SetValue(wxT("MULTICAST"));
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
            //LastChannel = wxString::Format(wxT("%d"), E131Dlg.SpinCtrl_NumUniv->GetValue() * 512);
            ok=!IpAddr.IsEmpty() && (LastChannel >= 1) && (LastChannel <= 512);
            if (ok)
            {
                LastChannelStr=wxString::Format(wxT("%d"),LastChannel);
                if (e)
                {
                    e->DeleteAttribute(wxT("ComPort"));
                    e->AddAttribute(wxT("ComPort"),IpAddr);
                    e->DeleteAttribute(wxT("BaudRate"));
                    e->AddAttribute(wxT("BaudRate"),wxString::Format(wxT("%d"),UnivNum));
                    e->DeleteAttribute(wxT("MaxChannels"));
                    e->AddAttribute(wxT("MaxChannels"),LastChannelStr);
                }
                else
                {
                    NumUniv = E131Dlg.SpinCtrl_NumUniv->GetValue();
                    for (int u=0; u < NumUniv; u++)
                    {
                        e = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("network") );
                        e->AddAttribute(wxT("NetworkType"),NetName);
                        e->AddAttribute(wxT("ComPort"),IpAddr);
                        e->AddAttribute(wxT("BaudRate"),wxString::Format(wxT("%d"),UnivNum));
                        e->AddAttribute(wxT("MaxChannels"),LastChannelStr);
                        NetworkXML.GetRoot()->AddChild(e);
                        UnivNum++;
                    }
                }
                UpdateNetworkList();
                UnsavedChanges=true;
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
    wxString NetName,Port,BaudRate,LastChannel;
    SerialPortWithRate SerialDlg(this);

    if (e)
    {
        NetName=e->GetAttribute(wxT("NetworkType"));
        Port=e->GetAttribute(wxT("ComPort"));
        BaudRate=e->GetAttribute(wxT("BaudRate"));
        LastChannel=e->GetAttribute(wxT("MaxChannels"));
        SerialDlg.ChoicePort->SetStringSelection(Port);
        SerialDlg.ChoiceBaudRate->SetStringSelection(BaudRate);
        SerialDlg.TextCtrlLastChannel->SetValue(LastChannel);
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
                    e = new wxXmlNode( wxXML_ELEMENT_NODE, wxT("network") );
                    NetworkXML.GetRoot()->AddChild(e);
                }
                e->DeleteAttribute(wxT("NetworkType"));
                e->AddAttribute(wxT("NetworkType"),NetName);
                e->DeleteAttribute(wxT("ComPort"));
                e->AddAttribute(wxT("ComPort"),Port);
                e->DeleteAttribute(wxT("BaudRate"));
                e->AddAttribute(wxT("BaudRate"),BaudRate);
                e->DeleteAttribute(wxT("MaxChannels"));
                e->AddAttribute(wxT("MaxChannels"),LastChannel);
                UpdateNetworkList();
                UnsavedChanges=true;
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

void xLightsFrame::OnButtonSaveSetupClick(wxCommandEvent& event)
{
    if (NetworkXML.Save( networkFile.GetFullPath() ))
    {
        UnsavedChanges=false;
    }
    else
    {
        wxMessageBox(_("Unable to save network definition file"), _("Error"));
    }
}
