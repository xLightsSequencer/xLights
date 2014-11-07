#include "ModelDialog.h" //Cheating to avoid full recompile by adding this in main.h
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
    const wxColour* color;
    wxString SelModelName=ListBoxElementList->GetStringSelection();
    modelPreview->StartDrawing(mPointSize);
    if(m_creating_bound_rect)
    {
        modelPreview->DrawRectangle(*wxYELLOW,true,m_bound_start_x,m_bound_start_y,m_bound_end_x,m_bound_end_y);
    }
    for (int i=0; i<PreviewModels.size(); i++)
    {
        color = (PreviewModels[i]->Selected) ? wxYELLOW : wxLIGHT_GREY;
        PreviewModels[i]->DisplayModelOnWindow(modelPreview,color);
    }
    modelPreview->EndDrawing();
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

    modelPreview->GetSize(&w, &h);

    // Add node position and channel number to arrays
    for (int i=0; i<PreviewModels.size(); i++)
    {
        PreviewModels[i]->AddToWholeHouseModel(modelPreview,xPos,yPos,actChannel);
        index+=PreviewModels[i]->GetNodeCount();
        StatusBar1->SetStatusText(wxString::Format("Processing %d of %d models",i+1,PreviewModels.size()));
    }

    // Add WholeHouseData attribute
    int ii=0;

    if(w>h){scale = (float)400/(float)w;}
    else{scale = (float)400/(float)h;}
    wScaled = (int)(scale*w);
    hScaled = (int)(scale*h);
    // Create a new model node
    wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "model");
    e->AddAttribute("name", modelName);
    e->AddAttribute("DisplayAs", "WholeHouse");
    e->AddAttribute("StringType", "RGB Nodes");
    e->AddAttribute("parm1", wxString::Format(wxT("%i"), wScaled));
    e->AddAttribute("parm2", wxString::Format(wxT("%i"), hScaled));

    for(int i=0;i<xPos.size();i++)
    {
        xPos[i] = (int)(scale*(float)xPos[i]);
        yPos[i] = (int)((scale*(float)yPos[i]));
        WholeHouseData += wxString::Format(wxT("%i,%i,%i"),actChannel[i],xPos[i],yPos[i]);
        if(i!=xPos.size()-1)
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
    StatusBar1->SetStatusText(wxString::Format("Completed creating '%s' whole house model",modelName));
}


void xLightsFrame::OnListBoxElementListSelect(wxCommandEvent& event)
{
    UnSelectAllModels();
    SelectModel(ListBoxElementList->GetString(ListBoxElementList->GetSelection()));
}

void xLightsFrame::SelectModel(wxString name)
{
    for(int i=0;i<ListBoxElementList->GetCount();i++)
    {
        if (name == ListBoxElementList->GetString(i))
        {
            ListBoxElementList->SetSelection(i);
            ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(i);
            m->Selected = true;
            int newscale=m->GetScale()*100.0;
            SliderPreviewScale->SetValue(newscale);
            TextCtrlPreviewElementSize->SetValue(wxString::Format( "%d",newscale));
            SliderPreviewRotate->SetValue(m->GetRotation());
            TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",m->GetRotation()));
            bool canrotate=m->CanRotate();
            SliderPreviewRotate->Enable(canrotate);
            StaticTextPreviewRotation->Enable(canrotate);
            UpdatePreview();
            break;
        }
    }

}

void xLightsFrame::OnScrolledWindowPreviewLeftDown(wxMouseEvent& event)
{
    if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetPosition().x;
        m_bound_start_y = modelPreview->getHeight() - event.GetPosition().y;
    }
    else if (m_over_handle == OVER_ROTATE_HANDLE)
    {
        m_rotating = true;
    }
    else if (m_over_handle != OVER_NO_HANDLE)
    {
        m_resizing = true;
    }
    else
    {
        m_rotating = false;
        m_resizing = false;
        m_creating_bound_rect = false;

        if(!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModels();
        }

        if(FindSelectedModel(event.GetX(),event.GetY())!=0)
        {
            m_dragging = true;
            m_previous_mouse_x = event.GetPosition().x;
            m_previous_mouse_y = event.GetPosition().y;
            StatusBar1->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        }
    }
}

void xLightsFrame::UnSelectAllModels()
{
   for (int i=0; i<PreviewModels.size(); i++)
    {
        PreviewModels[i]->Selected = false;
    }
    UpdatePreview();
}

void xLightsFrame::OnScrolledWindowPreviewRightDown(wxMouseEvent& event)
{
    if ( FindSelectedModel(event.GetX(),event.GetY()) == 0 )
    {
        return;
    }
    ListBoxElementList->GetSelection();
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(ListBoxElementList->GetSelection());

    wxXmlNode* e=m->GetModelXml();
    int DlgResult;
    bool ok;
    ModelDialog *dialog = new ModelDialog(this);
    dialog->SetFromXml(e);
    dialog->TextCtrl_Name->Enable(false); // do not allow name changes; -why? -DJ
    do
    {
        ok=true;
        DlgResult=dialog->ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            if (ok)
            {
                dialog->UpdateXml(e);
            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    UpdatePreview();
    delete dialog;
}

int xLightsFrame::FindSelectedModel(int x,int y)
{
    wxArrayInt found;
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->HitTest(modelPreview,x,y))
        {
            found.push_back(i);
        }
    }
    if (found.GetCount()==0)
    {
        return 0;
    }
    else if(found.GetCount()==1)
    {
        SelectModel(PreviewModels[found[0]]->name);
        mHitTestNextSelectModelIndex = 0;
    }
    else if (found.GetCount()>1)
    {
        for(int i=0;i<found.GetCount();i++)
        {
            if(mHitTestNextSelectModelIndex==i)
            {
                SelectModel(PreviewModels[found[i]]->name);
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= found.GetCount();
                break;
            }
        }
    }
    return found.GetCount();
}

void xLightsFrame::OnScrolledWindowPreviewLeftUp(wxMouseEvent& event)
{
    m_rotating = false;
    m_dragging = false;
    m_resizing = false;
    if(m_creating_bound_rect)
    {
        m_creating_bound_rect = false;
        UpdatePreview();
    }
}

void xLightsFrame::OnScrolledWindowPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
}

void xLightsFrame::OnScrolledWindowPreviewMouseMove(wxMouseEvent& event)
{
    int wi,ht;

    if (m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->getHeight() - event.GetPosition().y;
        UpdatePreview();
        return;
    }

    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);

    if(m_rotating)
    {
        m->RotateWithHandles(modelPreview,event.ShiftDown(), event.GetPosition().x,event.GetPosition().y);
        TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",(int)(m->GetPreviewRotation())));
        UpdatePreview();
    }
    else if(m_resizing)
    {
        m->ResizeWithHandles(modelPreview,event.GetPosition().x,event.GetPosition().y);
        TextCtrlPreviewElementSize->SetValue(wxString::Format( "%d",(int)(m->GetScale()*100)));
        UpdatePreview();
    }
    else if (m_dragging && event.Dragging())
    {
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = -(event.GetPosition().y - m_previous_mouse_y);
        modelPreview->GetSize(&wi,&ht);
        if (wi > 0 && ht > 0)
        {
            m->AddOffset(delta_x/wi, delta_y/ht);
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        StatusBar1->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        UpdatePreview();
    }
    else
    {
        if(m->Selected)
        {
            m_over_handle = m->CheckIfOverHandles(modelPreview,event.GetPosition().x,modelPreview->getHeight() - event.GetPosition().y);
        }
    }
}

void xLightsFrame::OnScrolledWindowPreviewResize(wxSizeEvent& event)
{
    UpdatePreview();
}

void xLightsFrame::OnScrolledWindowPreviewPaint(wxPaintEvent& event)
{
    UpdatePreview();
}


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

void xLightsFrame::PreviewRotationUpdated(int newRotation)
{
    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);
    m->SetModelCoord(newRotation);
    UpdatePreview();
}

void xLightsFrame::OnSliderPreviewRotateCmdSliderUpdated(wxScrollEvent& event)
{
    int newRotation=SliderPreviewRotate->GetValue();
    TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",newRotation));
    PreviewRotationUpdated(newRotation);
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
    modelPreview->StartDrawing(mPointSize);
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
        PreviewModels[m]->DisplayModelOnWindow(modelPreview);
    }
    modelPreview->EndDrawing();
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
void xLightsFrame::OnTextCtrlModelRotationDegreesText(wxCommandEvent& event)
{
    int newRotDegrees = wxAtoi(TextCtrlModelRotationDegrees->GetValue());
    SliderPreviewRotate->SetValue(newRotDegrees);
    PreviewRotationUpdated(newRotDegrees);
}

void xLightsFrame::OnTextCtrlPreviewElementSizeText(wxCommandEvent& event)
{
    int newscale = wxAtoi(TextCtrlPreviewElementSize->GetValue()); //SliderPreviewScale->GetValue();
    SliderPreviewScale->SetValue(newscale);
    PreviewScaleUpdated(newscale); //slider event not called automatically, so force it here
}

void xLightsFrame::OnButtonSelectModelGroupsClick(wxCommandEvent& event)
{
    wxString name;
    bool checked;
    wxXmlNode* e;
    CurrentPreviewModels dialog(this);
    for(e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            name=e->GetAttribute("name");
            if (!name.IsEmpty())
            {
                dialog.CheckListBoxCurrentGroups->Append(name,e);
                bool isChecked = e->GetAttribute("selected")=="1"?true:false;
                dialog.CheckListBoxCurrentGroups->Check(dialog.CheckListBoxCurrentGroups->GetCount()-1,isChecked);
            }
        }
    }
    dialog.ShowModal();
    SaveEffectsFile();
}

void xLightsFrame::OnButtonSetBackgroundImageClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Background Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty())
    {
        mBackgroundImage = filename;
        SetXmlSetting("backgroundImage",mBackgroundImage);
        modelPreview->SetbackgroundImage(mBackgroundImage);
        SaveEffectsFile();
        UpdatePreview();
    }
}

void xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    mBackgroundBrightness = Slider_BackgroundBrightness->GetValue();
    SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    SaveEffectsFile();
    UpdatePreview();
}


