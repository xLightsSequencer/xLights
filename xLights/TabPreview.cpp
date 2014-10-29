#define PREVIEWROTATIONFACTOR 3

void xLightsFrame::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < PreviewModels.size(); i++)
    {
        PreviewModels[i]->UpdateXmlWithScale();
    }
    SaveEffectsFile();
    StatusBar1->SetStatusText(_("Preview layout saved"));
}

/*
void xLightsFrame::OnButtonSetBackgroundClick(wxCommandEvent& event)
{
    wxImage image;
    wxClientDC dc(ScrolledWindowPreview);
    wxSize dcSize=dc.GetSize();
    wxString filename = wxFileSelector( "Choose Background Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty())
    {
        if (image.LoadFile(filename,wxBITMAP_TYPE_ANY))
        {
            image.Rescale(dcSize.GetWidth(),dcSize.GetHeight());
            wxBitmap bitmap(image);
            dc.DrawBitmap(bitmap,0,0);
        }
        else
        {
            wxMessageBox(_("Unable to load background image"));
        }
    }
}

void xLightsFrame::OnButtonClearBackgroundClick(wxCommandEvent& event)
{
    wxClientDC dc(ScrolledWindowPreview);
    dc.Clear();
}
*/

void xLightsFrame::OnButtonPreviewOpenClick(wxCommandEvent& event)
{
    wxArrayString SeqFiles;
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.xseq");
    if (UnsavedChanges && wxNO == wxMessageBox("Sequence changes will be lost.  Do you wish to continue?",
            "Sequence Changed Confirmation", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    wxSingleChoiceDialog dialog(this,_("Select file"),_("Open xLights Sequence"),SeqFiles);
    if (dialog.ShowModal() != wxID_OK) return;
    ResetTimer(NO_SEQ);
    ResetSequenceGrid();
    wxString filename=dialog.GetStringSelection();
    SeqLoadXlightsXSEQ(filename);
    SeqLoadXlightsFile(filename, false);
    SliderPreviewTime->SetValue(0);
    TextCtrlPreviewTime->Clear();
    CompareMyDisplayToSeq();
}

// ask user if they want to reset MyDisplay flags to match sequence
void xLightsFrame::CompareMyDisplayToSeq()
{
    wxString name;
    wxArrayString SeqModels;
    GetSeqModelNames(SeqModels);
    int SeqModelCount=SeqModels.size();
    if (SeqModelCount == 0) return;
    bool match=SeqModelCount == ListBoxElementList->GetCount();
    wxString reason; //tell the user why -DJ
    if (!match) reason = wxString::Format(wxT(", model count mismatch: sequence has %d, preview has %d"), SeqModelCount, ListBoxElementList->GetCount());
    for(int i=0; i < SeqModelCount && match; i++)
    {
        if (ListBoxElementList->FindString(SeqModels[i]) == wxNOT_FOUND)
        {
            match=false;
            reason += wxString::Format(wxT(", '%s' not in preview"), SeqModels[i]);
        }
    }
    if (match) return;
    if (!reason.IsEmpty()) reason = wxT("\nReason: ") + reason.Mid(2);
    int retval = wxMessageBox(_("Reset 'My Display' flags on element models to match sequence?") + reason,_("Adjust Preview"),wxCENTRE | wxYES_NO);
    if (retval != wxYES) return;

    // reset My Display flags
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            name=e->GetAttribute("name");
            ModelClass::SetMyDisplay(e,SeqModels.Index(name) != wxNOT_FOUND);
        }
    }
    UpdateModelsList();
    UpdatePreview();
}

void xLightsFrame::UpdatePreview()
{
    const wxColour *color;
    wxString SelModelName=ListBoxElementList->GetStringSelection();
    //ScrolledWindowPreview->ClearBackground();
    wxClientDC dc(ScrolledWindowPreview);
    dc.Clear();
    for (int i=0; i<PreviewModels.size(); i++)
    {
        color = (PreviewModels[i]->name == SelModelName) ? wxYELLOW : wxLIGHT_GREY;
        PreviewModels[i]->DisplayModelOnWindow(ScrolledWindowPreview,color);
    }
}

void xLightsFrame::OnButtonBuildWholeHouseModelClick(wxCommandEvent& event)
{
    WholeHouseModelNameDialog whDialog(this);
    if (whDialog.ShowModal() != wxID_OK) return;
    wxString modelName = whDialog.Text_WholehouseModelName->GetValue();
    if(modelName.Length()> 0)
    {
        BuildWholeHouseModel(modelName);
    }
}

void xLightsFrame::BuildWholeHouseModel(wxString modelName)
{
    size_t numberOfNodes=0;
    const wxColour *color;
    int w,h,wScaled,hScaled;
    size_t index=0;
    float scale=0;
    wxString WholeHouseData="";

    wxString SelModelName=ListBoxElementList->GetStringSelection();
    for (int i=0; i<PreviewModels.size(); i++)
    {
        numberOfNodes+= PreviewModels[i]->GetNodeCount();
    }
    std::vector<int> xPos;
    std::vector<int> yPos;
    std::vector<int> actChannel;
    xPos.resize(numberOfNodes);
    yPos.resize(numberOfNodes);
    actChannel.resize(numberOfNodes);


    wxClientDC dc(ScrolledWindowPreview);
    dc.Clear();

    // Add node position and channel number to arrays
    for (int i=0; i<PreviewModels.size(); i++)
    {
        PreviewModels[i]->AddToWholeHouseModel(ScrolledWindowPreview,index,xPos,yPos,actChannel);
        index+=PreviewModels[i]->GetNodeCount();
    }

    dc.GetSize(&w, &h);
    // Add WholeHouseData attribute
    if(w>h){scale = (float)400/(float)w;}
    else{scale = (float)400/(float)h;}
    wScaled = (int)(scale*w);
    hScaled = (int)(scale*h);
    int xoffset=wScaled/2;
    // Create a new model node
    wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "model");
    e->AddAttribute("name", modelName);
    e->AddAttribute("DisplayAs", "WholeHouse");
    e->AddAttribute("StringType", "RGB Nodes");
    e->AddAttribute("parm1", wxString::Format(wxT("%i"), wScaled));
    e->AddAttribute("parm2", wxString::Format(wxT("%i"), hScaled));

    for(int i=0;i<numberOfNodes;i++)
    {
        // Scale to 600 px max
        xPos[i] = (int)(scale*(float)xPos[i]);
        yPos[i] = (int)((scale*(float)yPos[i])+hScaled);
        WholeHouseData += wxString::Format(wxT("%i,%i,%i"),actChannel[i],xPos[i],yPos[i]);
        if(i!=numberOfNodes-1)
        {
            WholeHouseData+=";";
        }
    }

    e->AddAttribute("WholeHouseData", WholeHouseData);
    // Delete exisiting wholehouse model with same name
    for(wxXmlNode* n=ModelsNode->GetChildren(); n!=NULL; n=n->GetNext() )
    {
        if (n->GetAttribute("name") == modelName)
        {
            ModelsNode->RemoveChild(n);
            // No break, remove them all if more than one
        }
    }
    // Add model node to models
    ModelsNode->AddChild(e);
    // Save models to effects file
    SaveEffectsFile();
    // Update List on Sequencer page
    UpdateModelsList();

}


void xLightsFrame::OnListBoxElementListSelect(wxCommandEvent& event)
{
    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
    int newscale=m->GetScale()*100.0;
    SliderPreviewScale->SetValue(newscale);
    TextCtrlPreviewElementSize->SetValue(wxString::Format( "%d",newscale));
    SliderPreviewRotate->SetValue(m->GetRotation()/PREVIEWROTATIONFACTOR);
    bool canrotate=m->CanRotate();
    SliderPreviewRotate->Enable(canrotate);
    StaticTextPreviewRotation->Enable(canrotate);
    UpdatePreview();
}

void xLightsFrame::OnScrolledWindowPreviewLeftDown(wxMouseEvent& event)
{
    m_dragging = true;
    m_previous_mouse_x = event.GetPosition().x;
    m_previous_mouse_y = event.GetPosition().y;
    //StatusBar1->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
}

void xLightsFrame::OnScrolledWindowPreviewLeftUp(wxMouseEvent& event)
{
    m_dragging = false;
}

void xLightsFrame::OnScrolledWindowPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
}

void xLightsFrame::OnScrolledWindowPreviewMouseMove(wxMouseEvent& event)
{
    int wi,ht;
    if (m_dragging && event.Dragging())
    {
        int sel=ListBoxElementList->GetSelection();
        if (sel == wxNOT_FOUND) return;
        ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = event.GetPosition().y - m_previous_mouse_y;
        ScrolledWindowPreview->GetClientSize(&wi,&ht);
        if (wi > 0 && ht > 0)
        {
            m->AddOffset(delta_x/wi, delta_y/ht);
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        UpdatePreview();
    }
}

void xLightsFrame::OnScrolledWindowPreviewResize(wxSizeEvent& event)
{
    UpdatePreview();
}

//refactored
void xLightsFrame::PreviewScaleUpdated(int newscale)
{
    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
    m->SetScale(double(newscale)/100.0);
    UpdatePreview();
}

void xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated(wxScrollEvent& event)
{
    int newscale=SliderPreviewScale->GetValue();
    TextCtrlPreviewElementSize->SetValue(wxString::Format( "%d",newscale));
    PreviewScaleUpdated(newscale);
}

void xLightsFrame::OnSliderPreviewRotateCmdSliderUpdated(wxScrollEvent& event)
{
    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
    m->Rotate(PREVIEWROTATIONFACTOR*SliderPreviewRotate->GetValue());
    UpdatePreview();
}

void xLightsFrame::OnButtonModelsPreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < PreviewModels.size(); i++)
    {
        PreviewModels[i]->UpdateXmlWithScale();
    }
    ShowModelsDialog();
    UpdatePreview();
}

void xLightsFrame::OnButtonPlayPreviewClick(wxCommandEvent& event)
{
    if (SeqNumChannels == 0)
    {
        wxMessageBox(_("Nothing to play. Please open a sequence first."),_("Error in Preview"),wxOK | wxCENTRE | wxICON_ERROR);
        return;
    }
    int LastPreviewChannel=0;
    switch (SeqPlayerState)
    {
    case PAUSE_SEQ:
        PlayerDlg->MediaCtrl->Play();
        break;
    case PAUSE_SEQ_ANIM:
        //ResetTimer(PLAYING_SEQ_ANIM, PlaybackPeriod * XTIMER_INTERVAL);
        break;
    default:
        wxString details; //show details to help user -DJ
        for (int i=0; i<PreviewModels.size(); i++)
        {
            if (PreviewModels[i]->GetLastChannel() > SeqNumChannels) details = wxString::Format("Last was model '%s' - ends on channel %d vs. %d channels in the sequence", PreviewModels[i]->name, PreviewModels[i]->GetLastChannel(), SeqNumChannels);
            LastPreviewChannel=std::max(LastPreviewChannel,PreviewModels[i]->GetLastChannel());
        }
        if (LastPreviewChannel >= SeqNumChannels)
        {
            wxMessageBox(_("One or more of the models define channels beyond what is contained in the sequence. Verify your channel numbers and/or resave the sequence.\n" + details),_("Error in Preview"),wxOK | wxCENTRE | wxICON_ERROR);
            return;
        }
        PlayCurrentXlightsFile();
        heartbeat("playback preview", true); //tell fido to start watching -DJ
        break;
    }
}

void xLightsFrame::OnButtonStopPreviewClick(wxCommandEvent& event)
{
    if (mediaFilename.IsEmpty())
    {
        ResetTimer(PAUSE_SEQ_ANIM);
    }
    else
    {
        PlayerDlg->MediaCtrl->Pause();
    }
}

void xLightsFrame::ShowPreviewTime(long ElapsedMSec)
{
    int msec=ElapsedMSec % 1000;
    int seconds=ElapsedMSec / 1000;
    int minutes=seconds / 60;
    seconds=seconds % 60;
    TextCtrlPreviewTime->SetValue(wxString::Format("%d:%02d.%03d",minutes,seconds,msec));
}

void xLightsFrame::PreviewOutput(int period)
{
    size_t m, n, chnum, NodeCnt;
    wxByte intensity;
    TimerOutput(period);
    for (m=0; m<PreviewModels.size(); m++)
    {
        NodeCnt=PreviewModels[m]->GetNodeCount();
        size_t cn=PreviewModels[m]->ChannelsPerNode();
        for(n=0; n<NodeCnt; n++)
        {
            if (cn==1) {
                PreviewModels[m]->GetChanIntensity(n,0,&chnum,&intensity);
                intensity=SeqData[chnum*SeqNumPeriods+period];
                PreviewModels[m]->SetChanIntensityAll(n,intensity);
            } else {
                for(size_t c=0; c<cn; c++)
                {
                    PreviewModels[m]->GetChanIntensity(n,c,&chnum,&intensity);
                    intensity=SeqData[chnum*SeqNumPeriods+period];
                    PreviewModels[m]->SetChanIntensity(n,c,intensity);
                }
            }
        }
        PreviewModels[m]->DisplayModelOnWindow(ScrolledWindowPreview);
    }
    int amtdone = period * SliderPreviewTime->GetMax() / (SeqNumPeriods-1);
    SliderPreviewTime->SetValue(amtdone);
}

void xLightsFrame::OnSliderPreviewTimeCmdSliderUpdated(wxScrollEvent& event)
{
    int newperiod = SliderPreviewTime->GetValue() * (SeqNumPeriods-1) / SliderPreviewTime->GetMax();
    long msec=newperiod * XTIMER_INTERVAL;
    if (mediaFilename.IsEmpty())
    {
        ResetTimer(PLAYING_SEQ_ANIM, msec);
    }
    else
    {
        PlayerDlg->MediaCtrl->Seek(msec);
    }
}
