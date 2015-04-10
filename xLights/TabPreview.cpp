#include "xLightsMain.h"
#include "ModelDialog.h" //Cheating to avoid full recompile by adding this in main.h
#include "heartbeat.h"
#include "DrawGLUtils.h"
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

void xLightsFrame::OnButtonPreviewOpenClick(wxCommandEvent& event)
{
    wxArrayString SeqFiles;
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.fseq");
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.xseq");
    if (UnsavedChanges && wxNO == wxMessageBox("Sequence changes will be lost.  Do you wish to continue?",
            "Sequence Changed Confirmation", wxICON_QUESTION | wxYES_NO))
    {
        return;
    }
    wxSingleChoiceDialog dialog(this,_("Select file"),_("Open xLights Sequence"),SeqFiles);
    if (dialog.ShowModal() != wxID_OK) return;
    previewLoaded = false;
    previewPlaying = false;
    ResetTimer(NO_SEQ);
    ResetSequenceGrid();
    wxString filename=dialog.GetStringSelection();
    SeqLoadXlightsXSEQ(filename);
    SeqLoadXlightsFile(filename, false);
    bbPlayPause->SetBitmap(playIcon);
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
    if(!modelPreview->StartDrawing(mPointSize)) return;
    if(m_creating_bound_rect)
    {
        DrawGLUtils::DrawRectangle(xlYELLOW,true,m_bound_start_x,m_bound_start_y,m_bound_end_x,m_bound_end_y);
    }
    modelPreview->Render();
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
    std::vector<wxString> nodeType;

    modelPreview->GetVirtualCanvasSize(w, h);

    // Add node position and channel number to arrays
    for (int i=0; i<PreviewModels.size(); i++)
    {
        PreviewModels[i]->AddToWholeHouseModel(modelPreview,xPos,yPos,actChannel,nodeType);
        index+=PreviewModels[i]->GetNodeCount();
        StatusBar1->SetStatusText(wxString::Format("Processing %d of %d models",i+1,PreviewModels.size()));
    }

    // Add WholeHouseData attribute
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
        WholeHouseData += wxString::Format(wxT("%i,%i,%i,%s"),actChannel[i],xPos[i],yPos[i],(const char *)nodeType[i].c_str());
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
    int y = event.GetY();
    if (event.ControlDown())
    {
        SelectMultipleModels(event.GetX(),y);
        m_dragging = true;
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetX();
        m_bound_start_y = modelPreview->GetVirtualCanvasHeight() - y;
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

        if(SelectSingleModel(event.GetX(),y))
        {
            m_dragging = true;
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
            StatusBar1->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        }
    }
}

void xLightsFrame::UnSelectAllModels()
{
   for (int i=0; i<PreviewModels.size(); i++)
    {
        PreviewModels[i]->Selected = false;
        PreviewModels[i]->GroupSelected = false;
    }
    UpdatePreview();
}


void xLightsFrame::OnScrolledWindowPreviewRightDown(wxMouseEvent& event)
{

    wxMenu mnu;
    wxMenu *mnuAlign;
    wxMenu *mnuDistribute;
    int selectedModelCnt = ModelsSelectedCount();
    if (selectedModelCnt > 1)
    {
        mnuAlign = new wxMenu();
        mnuAlign->Append(ID_PREVIEW_ALIGN_TOP,"Top");
        mnuAlign->Append(ID_PREVIEW_ALIGN_BOTTOM,"Bottom");
        mnuAlign->Append(ID_PREVIEW_ALIGN_LEFT,"Left");
        mnuAlign->Append(ID_PREVIEW_ALIGN_RIGHT,"Right");
        mnuAlign->Append(ID_PREVIEW_ALIGN_H_CENTER,"Horizontal Center");
        mnuAlign->Append(ID_PREVIEW_ALIGN_V_CENTER,"Vertical Center");

        mnuDistribute = new wxMenu();
        mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
        mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");

        mnu.Append(ID_PREVIEW_ALIGN, 	        "Align", mnuAlign,"");
        mnu.Append(ID_PREVIEW_DISTRIBUTE,"Distribute", mnuDistribute,"");
        mnu.AppendSeparator();
    }
    else if (selectedModelCnt == 0)
    {
        return;
    }
    mnu.Append(ID_PREVIEW_MODEL_PROPERTIES,"Model Properties");
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xLightsFrame::OnPreviewModelPopup, NULL, this);
    PopupMenu(&mnu);
}

void xLightsFrame::OnPreviewModelPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_PREVIEW_ALIGN_TOP)
    {
        PreviewModelAlignTops();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM)
    {
        PreviewModelAlignBottoms();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT)
    {
        PreviewModelAlignLeft();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT)
    {
        PreviewModelAlignRight();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER)
    {
        PreviewModelAlignHCenter();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER)
    {
        PreviewModelAlignVCenter();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_PROPERTIES)
    {
        ShowModelProperties();
    }

}

void xLightsFrame::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int top = PreviewModels[selectedindex]->GetTop(modelPreview);
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetTop(modelPreview,top);
        }
    }
    UpdatePreview();
}

void xLightsFrame::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int bottom = PreviewModels[selectedindex]->GetBottom(modelPreview);
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetBottom(modelPreview,bottom);
        }
    }
    UpdatePreview();
}

void xLightsFrame::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int left = PreviewModels[selectedindex]->GetLeft(modelPreview);
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetLeft(modelPreview,left);
        }
    }
    UpdatePreview();
}

void xLightsFrame::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int right = PreviewModels[selectedindex]->GetRight(modelPreview);
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetRight(modelPreview,right);
        }
    }
    UpdatePreview();
}

void xLightsFrame::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    float center = PreviewModels[selectedindex]->GetHcenterOffset();
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetHcenterOffset(center);
        }
    }
    UpdatePreview();
}

void xLightsFrame::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    float center = PreviewModels[selectedindex]->GetVcenterOffset();
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->GroupSelected)
        {
            PreviewModels[i]->SetVcenterOffset(center);
        }
    }
    UpdatePreview();
}




int xLightsFrame::GetSelectedModelIndex()
{
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->Selected)
        {
            return i;
        }
    }
    return -1;
}

int xLightsFrame::ModelsSelectedCount()
{
    int selectedModelCount=0;
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->Selected || PreviewModels[i]->GroupSelected)
        {
            selectedModelCount++;
        }
    }
    return selectedModelCount;
}

void xLightsFrame::ShowModelProperties()
{
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
                SaveEffectsFile();
                UpdateModelsList();

            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    UpdatePreview();
    delete dialog;
}



int xLightsFrame::FindModelsClicked(int x,int y,wxArrayInt* found)
{
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->HitTest(modelPreview,x,y))
        {
            found->push_back(i);
        }
    }
    return found->GetCount();
}


bool xLightsFrame::SelectSingleModel(int x,int y)
{
    wxArrayInt found;
    int modelCount = FindModelsClicked(x,y,&found);
    if (modelCount==0)
    {
        return false;
    }
    else if(modelCount==1)
    {
        SelectModel(PreviewModels[found[0]]->name);
        mHitTestNextSelectModelIndex = 0;
        return true;
    }
    else if (modelCount>1)
    {
        for(int i=0;i<modelCount;i++)
        {
            if(mHitTestNextSelectModelIndex==i)
            {
                SelectModel(PreviewModels[found[i]]->name);
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= modelCount;
                return true;
            }
        }
    }
    return false;
}

bool xLightsFrame::SelectMultipleModels(int x,int y)
{
    wxArrayInt found;
    int modelCount = FindModelsClicked(x,y,&found);
    if (modelCount==0)
    {
        return false;
    }
    else if(modelCount>0)
    {
        if(PreviewModels[found[0]]->Selected)
        {
            PreviewModels[found[0]]->Selected = false;
            PreviewModels[found[0]]->GroupSelected = false;
        }
        else if (PreviewModels[found[0]]->GroupSelected)
        {
            SetSelectedModelToGroupSelected();
            PreviewModels[found[0]]->Selected = true;
            SelectModel(PreviewModels[found[0]]->name);
        }
        else
        {
            PreviewModels[found[0]]->GroupSelected = true;
        }
        UpdatePreview();
        return true;
    }
    return false;
}

void xLightsFrame::SetSelectedModelToGroupSelected()
{
    for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->Selected)
        {
            PreviewModels[i]->Selected = false;
            PreviewModels[i]->GroupSelected = true;
        }
    }
}


void xLightsFrame::OnScrolledWindowPreviewLeftUp(wxMouseEvent& event)
{
    int y = event.GetY();

    m_rotating = false;
    m_dragging = false;
    m_resizing = false;
    if(m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        SelectAllInBoundingRect();
        m_creating_bound_rect = false;
        UpdatePreview();
    }
}

void xLightsFrame::SelectAllInBoundingRect()
{
   for (int i=0; i<PreviewModels.size(); i++)
    {
        if(PreviewModels[i]->IsContained(modelPreview,m_bound_start_x,m_bound_start_y,
                                         m_bound_end_x,m_bound_end_y))
        {
            PreviewModels[i]->GroupSelected = true;
        }
    }
}

void xLightsFrame::OnScrolledWindowPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
}

void xLightsFrame::OnScrolledWindowPreviewMouseMove(wxMouseEvent& event)
{
    int y = event.GetY();
    int wi,ht;

    if (m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        UpdatePreview();
        return;
    }

    int sel=ListBoxElementList->GetSelection();
    if (sel == wxNOT_FOUND) return;
    ModelClass* m=(ModelClass*)ListBoxElementList->GetClientData(sel);

    if(m_rotating)
    {
        m->RotateWithHandles(modelPreview,event.ShiftDown(), event.GetPosition().x,y);
        TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",(int)(m->GetPreviewRotation())));
        UpdatePreview();
    }
    else if(m_resizing)
    {
        m->ResizeWithHandles(modelPreview,event.GetPosition().x,y);
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
            for (int i=0; i<PreviewModels.size(); i++)
            {
                if(PreviewModels[i]->Selected || PreviewModels[i]->GroupSelected)
                {
                   PreviewModels[i]->AddOffset(delta_x/wi, delta_y/ht);
                }
            }
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
            m_over_handle = m->CheckIfOverHandles(modelPreview,event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y);
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
    if (SeqData.NumChannels() == 0)
    {
        wxMessageBox(_("Nothing to play. Please open a sequence first."),_("Error in Preview"),wxOK | wxCENTRE | wxICON_ERROR);
        return;
    }
    int LastPreviewChannel=0;
    if(!previewPlaying)
    {
        bbPlayPause->SetBitmap(pauseIcon);
        previewPlaying = true;
    }
    else
    {
        bbPlayPause->SetBitmap(playIcon);
        previewPlaying = false;
    }
    switch (SeqPlayerState)
    {
    case PAUSE_SEQ:
        PlayerDlg->MediaCtrl->Play();
        break;
    case PAUSE_SEQ_ANIM:
        ResetTimer(PLAYING_SEQ_ANIM, PlaybackPeriod * SeqData.FrameTime());
        break;
    case PLAYING_SEQ_ANIM:
        ResetTimer(PAUSE_SEQ_ANIM, PlaybackPeriod * SeqData.FrameTime());
        break;
    default:
        wxString details; //show details to help user -DJ
        for (int i=0; i<PreviewModels.size(); i++)
        {
            if (PreviewModels[i]->GetLastChannel() > SeqData.NumChannels()) details = wxString::Format("\nLast was model '%s' - ends on channel %d vs. %d channels in the sequence", PreviewModels[i]->name, PreviewModels[i]->GetLastChannel(), SeqData.NumChannels());
            LastPreviewChannel=std::max(LastPreviewChannel,PreviewModels[i]->GetLastChannel());
        }
        if (LastPreviewChannel >= SeqData.NumChannels())
        {
//            wxMessageBox(_("One or more of the models define channels beyond what is contained in the sequence. Verify your channel numbers and/or resave the sequence.\n" + details),_("Error in Preview"),wxOK | wxCENTRE | wxICON_ERROR);
            if (wxMessageBox(_("One or more of the models define channels beyond what is contained in the sequence. Verify your channel numbers and/or resave the sequence." + details + "\nContinue?"),_("Error in Preview"),wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_ERROR) != wxYES)
                return;
        }
        if (!previewLoaded)
        {
            PlayCurrentXlightsFile();
            previewLoaded = true;
        }
        else
        {
            PlayerDlg->MediaCtrl->Pause();

        }
        heartbeat("playback preview", true); //tell fido to start watching -DJ
        break;
    }
}

void xLightsFrame::OnButtonStopPreviewClick(wxCommandEvent& event)
{
    if (mediaFilename.IsEmpty())
    {
        ResetTimer(PAUSE_SEQ_ANIM);
        PlaybackPeriod = 0;
        previewPlaying = false;
        SliderPreviewTime->SetValue(0);
        ShowPreviewTime(0);
        bbPlayPause->SetBitmap(playIcon);
    }
    else
    {
        StopPreviewPlayback();
    }
}

void xLightsFrame::StopPreviewPlayback()
{
    bbPlayPause->SetBitmap(playIcon);
    PlayerDlg->MediaCtrl->Pause();
    PlayerDlg->MediaCtrl->Seek(0);
    previewPlaying = false;
    SliderPreviewTime->SetValue(0);
    ShowPreviewTime(0);
}
void xLightsFrame::ShowPreviewTime(long ElapsedMSec)
{
    int msec=ElapsedMSec % 1000;
    int seconds=ElapsedMSec / 1000;
    int minutes=seconds / 60;
    seconds=seconds % 60;
    float ElaspedSeconds = ElapsedMSec/1000.0;
    TextCtrlPreviewTime->SetValue(wxString::Format("%d:%02d.%03d  %7.3fs",minutes,seconds,msec,ElaspedSeconds));
}

void xLightsFrame::PreviewOutput(int period)
{
    TimerOutput(period);
    modelPreview->Render(&SeqData[period][0]);

    int amtdone = period * SliderPreviewTime->GetMax() / (SeqData.NumFrames()-1);
    SliderPreviewTime->SetValue(amtdone);
}

void xLightsFrame::OnSliderPreviewTimeCmdSliderUpdated(wxScrollEvent& event)
{
    int newperiod = SliderPreviewTime->GetValue() * (SeqData.NumFrames()-1) / SliderPreviewTime->GetMax();
    long msec=newperiod * SeqData.FrameTime();
    if (mediaFilename.IsEmpty())
    {
        ResetTimer(PAUSE_SEQ_ANIM, msec);
        ShowPreviewTime(msec);
        PlaybackPeriod = newperiod;
    }
    else
    {
        PlayerDlg->MediaCtrl->Seek(msec);
        ShowPreviewTime(msec);
        //PreviewOutput(newperiod);
    }
}

void xLightsFrame::OnSliderPreviewTimeCmdScrollThumbTrack(wxScrollEvent& event)
{
    //when drag event starts stop the timer till the drag event ends.
    Timer1.Stop();
    int newperiod = SliderPreviewTime->GetValue() * (SeqData.NumFrames()-1) / SliderPreviewTime->GetMax();
    long msec=newperiod * SeqData.FrameTime();
    if (mediaFilename.IsEmpty())
    {
        //ResetTimer(PAUSE_SEQ_ANIM, msec);
        ShowPreviewTime(msec);
        PreviewOutput(newperiod);
        PlaybackPeriod = newperiod;
    }
    else
    {
/*        PlayerDlg->MediaCtrl->Seek(msec);
        ShowPreviewTime(msec);
        if(PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING)
        {
            PlayerDlg->MediaCtrl->Play();
        }
        PreviewOutput(newperiod);
        seekPoint = msec;*/
    }
}

void xLightsFrame::OnSliderPreviewTimeCmdScrollThumbRelease(wxScrollEvent& event)
{
    int newperiod = SliderPreviewTime->GetValue() * (SeqData.NumFrames()-1) / SliderPreviewTime->GetMax();
    long msec=newperiod * SeqData.FrameTime();
    if (mediaFilename.IsEmpty())
    {
        ResetTimer(PAUSE_SEQ_ANIM, msec);
        ShowPreviewTime(msec);
        bbPlayPause->SetBitmap(playIcon);
        previewPlaying = false;
        PlaybackPeriod=newperiod;
    }
    else if(SeqPlayerState != PLAYING_SEQ_ANIM)
    {
 /*       ShowPreviewTime(msec);
        PlayerDlg->MediaCtrl->Seek(msec);

        wxSleep(1);

        PlayerDlg->MediaCtrl->Stop();
        PlayerDlg->MediaCtrl->Seek(msec);
        //Update the slider back to where the user last selected since it played past that point
        int frame = msec / SeqData.FrameTime();
        SliderPreviewTime->SetValue(frame*SliderPreviewTime->GetMax()/(SeqData.NumFrames()-1));
        //Update the time box.

        bbPlayPause->SetBitmap(playIcon);
        previewPlaying = false;*/
    }
    Timer1.Start(SeqData.FrameTime(), wxTIMER_CONTINUOUS);
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
    CurrentPreviewModels dialog(this,ModelGroupsNode,ModelsNode);
    dialog.ShowModal();
    SaveEffectsFile();
    ShowSelectedModelGroups();
}

void xLightsFrame::ShowSelectedModelGroups()
{
    // Remove all models from display
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            e->DeleteAttribute("MyDisplay");
            e->AddAttribute("MyDisplay","0");
        }
    }
    // Set models in selected modelgroups as part of display.
    for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "modelGroup")
        {
            if(e->GetAttribute("selected") == "1")
            {
                wxArrayString ModelsInGroup=wxSplit(e->GetAttribute("models"),',');
                for(int i=0;i<ModelsInGroup.size();i++)
                {
                    SetModelAsPartOfDisplay(ModelsInGroup[i]);
                }
            }
        }
    }
    UpdateModelsList();
    UpdatePreview();

}

void xLightsFrame::SetModelAsPartOfDisplay(wxString& model)
{
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if(e->GetAttribute("name")== model)
            {
                e->DeleteAttribute("MyDisplay");
                e->AddAttribute("MyDisplay","1");
            }
        }
    }
}

void xLightsFrame::OnButtonSetBackgroundImageClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Background Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty())
    {
        mBackgroundImage = filename;
        SetXmlSetting("backgroundImage",mBackgroundImage);
        modelPreview->SetbackgroundImage(mBackgroundImage);
        sPreview2->SetbackgroundImage(mBackgroundImage);
        SaveEffectsFile();
        UpdatePreview();
    }
}

void xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    mBackgroundBrightness = Slider_BackgroundBrightness->GetValue();
    SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    SaveEffectsFile();
    UpdatePreview();
}


