#include "xLightsMain.h"
#include "ModelDialog.h" //Cheating to avoid full recompile by adding this in main.h
#include "heartbeat.h"
#include "DrawGLUtils.h"
#include "SaveChangesDialog.h"
#include "models/Model.h"
#ifndef USE_WXMEDIAPLAYER
#include "xLightsXmlFile.h"
#endif
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
    if( mSavedChangeCount !=  mSequenceElements.GetChangeCount() )
    {
        SaveChangesDialog* dlg = new SaveChangesDialog(this);
        if( dlg->ShowModal() == wxID_CANCEL )
        {
            return;
        }
        if( dlg->GetSaveChanges() )
        {
            SaveSequence();
        }
    }

    wxArrayString SeqFiles;
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.fseq");
    wxDir::GetAllFiles(CurrentDir,&SeqFiles,"*.xseq");
    wxSingleChoiceDialog dialog(this,_("Select file"),_("Open xLights Sequence"),SeqFiles);
    if (dialog.ShowModal() != wxID_OK) return;
    previewLoaded = false;
    previewPlaying = false;
    ResetTimer(NO_SEQ);
    wxString filename=dialog.GetStringSelection();
    SeqLoadXlightsXSEQ(filename);
    SeqLoadXlightsFile(filename, false);
    bbPlayPause->SetBitmap(playIcon);
    SliderPreviewTime->SetValue(0);
    TextCtrlPreviewTime->Clear();
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

void xLightsFrame::OnListBoxElementListItemSelect(wxListEvent& event)
{
    UnSelectAllModels();
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    SelectModel(ListBoxElementList->GetItemText(sel).ToStdString());
}

void xLightsFrame::SelectModel(const std::string & name)
{
	int foundStart = 0;
	int foundEnd = 0;
	for(int i=0;i<ListBoxElementList->GetItemCount();i++)
    {
        if (name == ListBoxElementList->GetItemText(i))
        {
            if (ListBoxElementList->GetItemState(i, wxLIST_STATE_SELECTED) == false) {
                ListBoxElementList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                return;
            }
            Model* m=(Model*)ListBoxElementList->GetItemData(i);
            m->Selected = true;
            double newscalex, newscaley;
            m->GetScales(newscalex, newscaley);
            newscalex *= 100.0;
            newscaley *= 100.0;
            SliderPreviewScaleWidth->SetValue(newscalex*10);
            SliderPreviewScaleHeight->SetValue(newscaley*10);
            TextCtrlPreviewElementHeight->SetValue("-1");
            TextCtrlPreviewElementWidth->SetValue(wxString::Format( "%0.1f",newscalex));
            TextCtrlPreviewElementHeight->SetValue(wxString::Format( "%0.1f",newscaley));
            SliderPreviewRotate->SetValue(m->GetRotation());
            TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",m->GetRotation()));
            SliderPreviewRotate->Enable(true);
            StaticTextPreviewRotation->Enable(true);
            TextCtrlModelStartChannel->SetValue(m->ModelStartChannel);
            if (CheckBoxOverlap->GetValue()== true) {
                foundStart = m->GetNumberFromChannelString(m->ModelStartChannel);
                foundEnd = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,2).ToStdString());
            }
            break;
        }
    }

    if (CheckBoxOverlap->GetValue()) {
        for(int i=0;i<ListBoxElementList->GetItemCount();i++)
        {
            if (name != ListBoxElementList->GetItemText(i)) {
                Model* m=(Model*)ListBoxElementList->GetItemData(i);
                if (m != NULL) {
                    int startChan = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,1).ToStdString());
                    int endChan = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,2).ToStdString());
                    if ((startChan >= foundStart) && (endChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else if ((startChan >= foundStart) && (startChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else if ((endChan >= foundStart) && (endChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else {
                        m->Overlapping = false;
                    }
                }
            }
        }
	}
	UpdatePreview();

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
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnPreviewModelPopup, NULL, this);

        mnuDistribute = new wxMenu();
        mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
        mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnPreviewModelPopup, NULL, this);

        mnu.Append(ID_PREVIEW_ALIGN, 	        "Align", mnuAlign,"");
        mnu.Append(ID_PREVIEW_DISTRIBUTE,"Distribute", mnuDistribute,"");
        mnu.AppendSeparator();
    }
    else if (selectedModelCnt == 0)
    {
        return;
    }
    mnu.Append(ID_PREVIEW_MODEL_PROPERTIES,"Model Properties");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xLightsFrame::OnPreviewModelPopup, NULL, this);
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
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);

    wxXmlNode* e=m->GetModelXml();
    int DlgResult;
    bool ok;
    ModelDialog *dialog = new ModelDialog(this);
    dialog->SetFromXml(e, &NetInfo);
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
                UnsavedRgbEffectsChanges=true;
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

    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);

    if(m_rotating)
    {
        m->RotateWithHandles(modelPreview,event.ShiftDown(), event.GetPosition().x,y);
        m->UpdateXmlWithScale();
        UnsavedRgbEffectsChanges = true;
        TextCtrlModelRotationDegrees->SetValue(wxString::Format( "%d",(int)(m->GetPreviewRotation())));
        UpdatePreview();
    }
    else if(m_resizing)
    {
        m->ResizeWithHandles(modelPreview,event.GetPosition().x,y);
        double scalex, scaley;
        m->GetScales(scalex, scaley);
        TextCtrlPreviewElementWidth->SetValue(wxString::Format( "%0.1f",scalex*100.0));
        TextCtrlPreviewElementHeight->SetValue(wxString::Format( "%0.1f",scaley*100.0));
        m->UpdateXmlWithScale();
        UnsavedRgbEffectsChanges = true;
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
                   PreviewModels[i]->UpdateXmlWithScale();
                   UnsavedRgbEffectsChanges = true;
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


void xLightsFrame::PreviewScaleUpdated(float xscale, float yscale)
{
    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);
    m->SetScale(xscale/100.0, yscale/100.0);
    m->UpdateXmlWithScale();
    UnsavedRgbEffectsChanges = true;
    UpdatePreview();
}

void xLightsFrame::OnSliderPreviewScaleCmdSliderUpdated(wxScrollEvent& event)
{
    double newscalex=SliderPreviewScaleWidth->GetValue();
    double newscaley=SliderPreviewScaleHeight->GetValue();
    newscalex /= 10.0;
    newscaley /= 10.0;
    TextCtrlPreviewElementWidth->SetValue(wxString::Format( "%.1f",newscalex));
    TextCtrlPreviewElementHeight->SetValue(wxString::Format( "%.1f",newscaley));
    PreviewScaleUpdated(newscalex, newscaley);
}

void xLightsFrame::PreviewRotationUpdated(int newRotation)
{
    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);
    m->SetModelCoord(newRotation);
    m->UpdateXmlWithScale();
    UnsavedRgbEffectsChanges = true;
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
#ifdef USE_WXMEDIAPLAYER
		PlayerDlg->Play();
#else
		if (CurrentSeqXmlFile->GetMedia() != NULL)
		{
			CurrentSeqXmlFile->GetMedia()->Play();
		}
#endif
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
#ifdef USE_WXMEDIAPLAYER
			PlayerDlg->Pause();
#else
			if (CurrentSeqXmlFile->GetMedia() != NULL)
			{
				CurrentSeqXmlFile->GetMedia()->Pause();
			}
#endif

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
#ifdef USE_WXMEDIAPLAYER
	PlayerDlg->Pause();
	PlayerDlg->Seek(0);
#else
	if (CurrentSeqXmlFile->GetMedia() != NULL)
	{
		CurrentSeqXmlFile->GetMedia()->Pause();
		CurrentSeqXmlFile->GetMedia()->Seek(0);
	}
#endif
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
#ifdef USE_WXMEDIAPLAYER
		PlayerDlg->Seek(msec);
#else
		if (CurrentSeqXmlFile->GetMedia() != NULL)
		{
			CurrentSeqXmlFile->GetMedia()->Seek(msec);
		}
#endif
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
    float newscalex = wxAtof(TextCtrlPreviewElementWidth->GetValue());
    float newscaley = wxAtof(TextCtrlPreviewElementHeight->GetValue());
    if (newscaley == -1) {return;}
    SliderPreviewScaleWidth->SetValue(newscalex * 10);
    SliderPreviewScaleHeight->SetValue(newscaley * 10);
    PreviewScaleUpdated(newscalex, newscaley); //slider event not called automatically, so force it here
}

void xLightsFrame::OnButtonSelectModelGroupsClick(wxCommandEvent& event)
{
    CurrentPreviewModels dialog(this,ModelGroupsNode,ModelsNode);
    dialog.ShowModal();

    for (wxXmlNode *node = ModelGroupsNode->GetChildren(); node != nullptr; node = node->GetNext()) {
        wxString oldName = node->GetAttribute("oldName", "");
        node->DeleteAttribute("oldName");
        if (oldName != "") {
            Element* elem_to_rename = mSequenceElements.GetElement(oldName.ToStdString());
            if( elem_to_rename != NULL ) {
                elem_to_rename->SetName(node->GetAttribute("name").ToStdString());
            }
        }
    }

    UnsavedRgbEffectsChanges=true;
    ShowSelectedModelGroups();
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    RowHeadingsChanged(eventRowHeaderChanged);
}

void xLightsFrame::ShowSelectedModelGroups()
{
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
    wxString filename = wxFileSelector( "Choose Background Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    if (!filename.IsEmpty())
    {
        mBackgroundImage = filename;
        SetXmlSetting("backgroundImage",mBackgroundImage);
        modelPreview->SetbackgroundImage(mBackgroundImage);
        sPreview2->SetbackgroundImage(mBackgroundImage);
        UnsavedRgbEffectsChanges=true;
        UpdatePreview();
    }
}

void xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    mBackgroundBrightness = Slider_BackgroundBrightness->GetValue();
    SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    UnsavedRgbEffectsChanges=true;
    UpdatePreview();
}


void xLightsFrame::OnScaleImageCheckboxClick(wxCommandEvent& event)
{
    SetXmlSetting("scaleImage",wxString::Format("%d",ScaleImageCheckbox->IsChecked()));
    modelPreview->SetScaleBackgroundImage(ScaleImageCheckbox->IsChecked());
    sPreview2->SetScaleBackgroundImage(ScaleImageCheckbox->IsChecked());
    UnsavedRgbEffectsChanges=true;
}


void xLightsFrame::OnTextCtrlModelStartChannelText(wxCommandEvent& event)
{
    std::string newStartChannel = TextCtrlModelStartChannel->GetValue().ToStdString();
	int sel = ListBoxElementList->GetFirstSelected();
	if (sel == wxNOT_FOUND) return;
	Model* m = (Model*)ListBoxElementList->GetItemData(sel);
	wxString name = ListBoxElementList->GetItemText(sel);
	int oldStart = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(sel,1).ToStdString());
    int oldEnd = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(sel,2).ToStdString());
    int newEnd = (m->GetNumberFromChannelString(newStartChannel) - oldStart) + oldEnd;
	m->SetModelStartChan(newStartChannel);
	for(wxXmlNode* e=ModelGroupsNode->GetChildren(); e!=NULL; e=e->GetNext() ) {
		if (e->GetName().Cmp("modelGroup") == 0) {
			if (name.Cmp(e->GetAttribute("name")) == 0) {
				e->DeleteAttribute("StartChannel");
				e->AddAttribute("StartChannel",wxString::Format("%d",newStartChannel));
			}
		}
	}
	ListBoxElementList->SetItem(sel, 1, newStartChannel);
	ListBoxElementList->SetItem(sel, 2, wxString::Format("%d",newEnd));
	if (newEnd > NetInfo.GetTotChannels()) {
        TextCtrlModelStartChannel->SetBackgroundColour(wxColour("#ff0000"));
	} else {
        TextCtrlModelStartChannel->SetBackgroundColour(wxColour("#ffffff"));
    }
	UpdatePreview();
}

int wxCALLBACK SortElementsFunctionASC(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortColumn)
{
    Model* a = (Model *)item1;
    Model* b = (Model *)item2;

    if (sortColumn == 1) {
        int ia = a->GetNumberFromChannelString(a->ModelStartChannel);
        int ib = b->GetNumberFromChannelString(b->ModelStartChannel);
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else if (sortColumn == 2) {
        int ia = a->GetLastChannel();
        int ib = b->GetLastChannel();
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else {
        return strcasecmp(a->name.c_str(), b->name.c_str());
    }
	return 0;
}

int wxCALLBACK SortElementsFunctionDESC(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortColumn)
{
    return SortElementsFunctionASC(item2, item1, sortColumn);
}


void xLightsFrame::OnListBoxElementListColumnClick(wxListEvent& event)
{
    int col = event.GetColumn();
    static bool x = false;
    x = !x;
    x ? ListBoxElementList->SortItems(SortElementsFunctionASC,col):ListBoxElementList->SortItems(SortElementsFunctionDESC,col);
}


void xLightsFrame::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        for(int i=0;i<ListBoxElementList->GetItemCount();i++)
        {
            Model* m=(Model*)ListBoxElementList->GetItemData(i);
            if (m != NULL) {
                m->Overlapping = false;
            }
        }
    }
}
