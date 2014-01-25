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
    for(int i=0; i < SeqModelCount && match; i++)
    {
        if (ListBoxElementList->FindString(SeqModels[i]) == wxNOT_FOUND) match=false;
    }
    if (match) return;
    int retval = wxMessageBox(_("Reset 'My Display' flags on element models to match sequence?"),_("Adjust Preview"),wxCENTRE | wxYES_NO);
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

void xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated(wxScrollEvent& event)
{
    int newscale=SliderPreviewScale->GetValue();
    TextCtrlPreviewElementSize->SetValue(wxString::Format( "%d",newscale));
    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
    m->SetScale(double(newscale)/100.0);
    UpdatePreview();
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
    TextCtrlPreviewTime->SetValue(wxString::Format("%d:%02d:%03d",minutes,seconds,msec));
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
