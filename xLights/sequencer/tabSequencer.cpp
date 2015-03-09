#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/tipwin.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"
#include "../EffectIconPanel.h"
#include "Element.h"
#include "Effect.h"
#include "../SeqSettingsDialog.h"
#include "../DisplayElementsPanel.h"


#define PLAY_TYPE_EFFECT 1
#define PLAY_TYPE_MODEL  2



/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
    EffectsPanel1 = NULL;
    timingPanel = NULL;

    mSequenceElements.SetFrequency(40);
    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    mainSequencer = new MainSequencer(PanelSequencer);
    mainSequencer->SetSequenceElements(&mSequenceElements);

    DOUBLE_BUFFER(mainSequencer);
    mainSequencer->PanelWaveForm->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
    mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));

    mainSequencer->PanelRowHeadings->SetSequenceElements(&mSequenceElements);
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());

    sPreview1 = new SequencePreview(PanelSequencer,args);
    sPreview1->SetSize(wxSize(200,200));
    sPreview1->InitializePreview();
    m_mgr->AddPane(sPreview1,wxAuiPaneInfo().Name(wxT("ModelPreview")).Caption(wxT("Model Preview")).
                   BestSize(wxSize(200,200)).Left());

    effectsPnl = new TopEffectsPanel(PanelSequencer);
    EffectsPanel1 = new EffectsPanel(effectsPnl->Panel_EffectContainer, ID_PANEL_EFFECTS1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1"));
    effectsPnl->Refresh();

    wxScrolledWindow* w;
    for(int i =0;i<EffectsPanel1->Choicebook1->GetPageCount();i++)
    {
        w = (wxScrolledWindow*)EffectsPanel1->Choicebook1->GetPage(i);
        w->FitInside();
        w->SetScrollRate(5, 5);
    }

    colorPanel = new ColorPanel(PanelSequencer);
    timingPanel = new TimingPanel(PanelSequencer);

    perspectivePanel = new PerspectivesPanel(PanelSequencer);

    effectPalettePanel = new EffectIconPanel(PanelSequencer);

    // DisplayElements Panel
    displayElementsPanel = new DisplayElementsPanel(PanelSequencer);

    m_mgr->AddPane(displayElementsPanel,wxAuiPaneInfo().Name(wxT("DisplayElements")).Caption(wxT("Sequence Elements")).
                   BestSize(wxSize(200,250)).Float());
    // Hide the panel on start.
    m_mgr->GetPane("DisplayElements").Hide();


    m_mgr->AddPane(perspectivePanel,wxAuiPaneInfo().Name(wxT("Perspectives")).Caption(wxT("Perspectives")).
                   BestSize(wxSize(175,175)).Left());

    m_mgr->AddPane(effectPalettePanel,wxAuiPaneInfo().Name(wxT("EffectDropper")).Caption(wxT("Effects")).
                   BestSize(wxSize(150,150)).MinSize(wxSize(150,150)).Left());

    m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effect")).Caption(wxT("Effect Settings")).
                   BestSize(wxSize(175,175)).MinSize(wxSize(175,175)).Left());

    m_mgr->AddPane(colorPanel,wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).
                   BestSize(wxSize(175,175)).Left());

    m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer/Timing")).
                   BestSize(wxSize(175,175)).Left());

    m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));

    mainSequencer->Layout();
    m_mgr->Update();
}

void xLightsFrame::InitSequencer()
{
    if(mSequencerInitialize || EffectsPanel1 == NULL || timingPanel == NULL)
    {
        return;
    }
    // Load perspectives
    mSequencerInitialize = true;
    CheckForAndCreateDefaultPerpective();
    perspectivePanel->SetPerspectives(PerspectivesNode);
}

void xLightsFrame::CheckForAndCreateDefaultPerpective()
{
    wxXmlNode* prospectives = PerspectivesNode->GetChildren();
    mCurrentPerpective = NULL;
    if (prospectives==NULL)
    {
        if(PerspectivesNode->HasAttribute("current"))
        {
            PerspectivesNode->DeleteAttribute("current");
        }
        PerspectivesNode->AddAttribute("current","Default Perspective");
        wxXmlNode* p=new wxXmlNode(wxXML_ELEMENT_NODE, "perspective");
        p->AddAttribute("name", "Default Perspective");
        p->AddAttribute("settings",m_mgr->SavePerspective());
        PerspectivesNode->AddChild(p);
        mCurrentPerpective = p;
        SaveEffectsFile();
    }
    else
    {
        wxString currentName = PerspectivesNode->GetAttribute("current");
        for(wxXmlNode* p=PerspectivesNode->GetChildren(); p!=NULL; p=p->GetNext())
        {
            if (p->GetName() == "perspective")
            {
                wxString name=p->GetAttribute("name");
                if (!name.IsEmpty() && currentName == name)
                {
                    mCurrentPerpective = p;
                }
            }
        }
    }

    if(mCurrentPerpective!=NULL)
    {
        wxString settings = mCurrentPerpective->GetAttribute("settings");
        m_mgr->LoadPerspective(settings);
    }
    m_mgr->GetPane(wxT("ModelPreview")).Show(true);

}



void xLightsFrame::CheckForValidModels()
{
    wxArrayString ModelNames;
    GetModelNames(ModelNames);
    SeqElementMismatchDialog dialog(this);
    dialog.ChoiceModels->Set(ModelNames);
    for (int x = mSequenceElements.GetElementCount()-1; x >= 0; x--) {
        if ("model" == mSequenceElements.GetElement(x)->GetType()) {
            wxString name = mSequenceElements.GetElement(x)->GetName();
            if (ModelNames.Index(name) == wxNOT_FOUND) {
                dialog.StaticTextMessage->SetLabel("Model '"+name+"'\ndoes not exist in your list of models");
                dialog.Fit();
                dialog.ShowModal();
                if (dialog.RadioButtonAdd->GetValue()) {
                } else if (dialog.RadioButtonDelete->GetValue()) {
                    mSequenceElements.DeleteElement(name);
                } else {
                    wxString newName = dialog.ChoiceModels->GetStringSelection();
                    mSequenceElements.GetElement(x)->SetName(newName);
                }
            }
        }
    }
}

void xLightsFrame::LoadSequencer(xLightsXmlFile& xml_file)
{
    mSequenceElements.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
    mSequenceElements.LoadSequencerFile(xml_file);

    CheckForValidModels();

    mMediaLengthMS = xml_file.GetSequenceDurationMS();

        mainSequencer->PanelWaveForm->CloseMediaFile();
        if(xml_file.GetSequenceType()=="Media")
        {
            int musicLength = 0;
            mediaFilename = xml_file.GetMediaFile();
            if( mediaFilename == wxEmptyString )
            {
                SeqSettingsDialog setting_dlg(this, &xml_file, mediaDirectory, wxT("Please select Media file!!!"));
                setting_dlg.Fit();
                setting_dlg.ShowModal();
                mediaFilename = xml_file.GetMediaFile();
            }
            if( mediaFilename != wxEmptyString )
            {
                PlayerDlg->Load(mediaFilename);
                musicLength = mainSequencer->PanelWaveForm->OpenfileMediaFile(xml_file.GetMediaFile());
                if(musicLength <=0)
                {
                    wxMessageBox("Invalid Media File");
                }
            }
            else
            {
               wxMessageBox("Media File must be specified");
            }
            if (mMediaLengthMS == 0) {
                mMediaLengthMS = musicLength;
            }
            //mainSequencer->PanelWaveForm->SetCanvasSize(1200,75);
        }

//        wxString s = wxString::Format("Length=%d",mMediaLengthMS);
//        wxMessageBox(s);
        //mainSequencer->PanelTimeLine->SetTimeLength(230000);
        mainSequencer->PanelTimeLine->SetTimeLength(mMediaLengthMS);
        //mainSequencer->PanelTimeLine->SetCanvasSize(1200,25);
        mainSequencer->PanelTimeLine->Initialize();
        //mainSequencer->PanelEffectGrid->SetCanvasSize(1200,2200);
        mainSequencer->PanelEffectGrid->SetSequenceElements(&mSequenceElements);
        mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);
        //mainSequencer->PanelEffectGrid->InitializeGrid();

        ResizeAndMakeEffectsScroll();
        ResizeMainSequencer();
        int maxZoom = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
        mainSequencer->PanelTimeLine->SetZoomLevel(maxZoom);
        mainSequencer->PanelWaveForm->SetZoomLevel(maxZoom);
        mainSequencer->PanelWaveForm->Refresh();
        mainSequencer->PanelEffectGrid->Refresh();
        sPreview1->Refresh();
        m_mgr->Update();
}

void xLightsFrame::EffectsResize(wxSizeEvent& event)
{
    int k=0;
}

void xLightsFrame::EffectsPaint(wxPaintEvent& event)
{
    int k=0;
}

void xLightsFrame::Zoom( wxCommandEvent& event)
{
    if(event.GetInt() == ZOOM_IN)
    {
        mainSequencer->PanelTimeLine->ZoomIn();
    }
    else
    {
        mainSequencer->PanelTimeLine->ZoomOut();
    }
}

void xLightsFrame::HorizontalScrollChanged( wxCommandEvent& event)
{
    int position = mainSequencer->ScrollBarEffectsHorizontal->GetThumbPosition();
    int timeLength = mainSequencer->PanelTimeLine->GetTimeLength();

    int startTime = (int)(((double)position/(double)timeLength) * (double)timeLength);
    mainSequencer->PanelTimeLine->SetStartTimeMS(startTime);
}

void xLightsFrame::ScrollRight(wxCommandEvent& event)
{
    int position = mainSequencer->ScrollBarEffectsHorizontal->GetThumbPosition();
    int limit = mainSequencer->ScrollBarEffectsHorizontal->GetRange();
    if( position < limit-1 )
    {
        int ts = mainSequencer->ScrollBarEffectsHorizontal->GetThumbSize() / 2;
        if (ts == 0) {
            ts = 1;
        }
        position += ts;
        if (position >= limit) {
            position = limit - 1;
        }
        mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(position);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        wxPostEvent(this, eventScroll);
    }
}

void xLightsFrame::TimeSelected( wxCommandEvent& event)
{
    // event.GetInt holds position without first pixelOffset
    int newPlayTime = mainSequencer->PanelTimeLine->TimeSelected(event.GetInt());
    if( mainSequencer->GetIsPlaying() ) {
        PlayerDlg->MediaCtrl->Seek(newPlayTime);
    }
}



void xLightsFrame::TimelineChanged( wxCommandEvent& event)
{
    TimelineChangeArguments *tla = (TimelineChangeArguments*)(event.GetClientData());
    mainSequencer->PanelWaveForm->SetZoomLevel(tla->ZoomLevel);
    mainSequencer->PanelWaveForm->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelWaveForm->RecalcSelectedPosition();
    mainSequencer->UpdateTimeDisplay(tla->SelectedTime);
    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelEffectGrid->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelEffectGrid->Refresh();
    UpdateEffectGridHorizontalScrollBar();
    // Set text entry to timing for "T" insertion
    mTextEntryContext = TEXT_ENTRY_TIMING;
    delete tla;
}

void xLightsFrame::UpdateEffectGridHorizontalScrollBar()
{
    int zoomLevel = mainSequencer->PanelTimeLine->GetZoomLevel();
    int maxZoomLevel = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
    if(zoomLevel == maxZoomLevel)
    {
        // Max Zoom so scrollbar is same size as window.
        int range = mainSequencer->PanelTimeLine->GetSize().x;
        int pageSize =range;
        int thumbSize = range;
        mainSequencer->ScrollBarEffectsHorizontal->SetScrollbar(0,thumbSize,range,pageSize);
    }
    else
    {
        double startTime;
        double endTime;
        int range = mainSequencer->PanelTimeLine->GetTimeLength();
        mainSequencer->PanelTimeLine->GetViewableTimeRange(startTime,endTime);

        double diff = endTime - startTime;
        int thumbSize = (int)(diff*(double)1000);
        int pageSize = thumbSize;
        int position = (int)(startTime * (double)1000);
        mainSequencer->ScrollBarEffectsHorizontal->SetScrollbar(position,thumbSize,range,pageSize);
    }

    mainSequencer->ScrollBarEffectsHorizontal->Refresh();
}

void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    mSequenceElements.PopulateRowInformation();
    int height = DEFAULT_ROW_HEADING_HEIGHT * mSequenceElements.GetRowInformationSize();
    ResizeMainSequencer();
//    m_mgr->Update();
}

void xLightsFrame::WindowResized( wxCommandEvent& event)
{
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
}

void xLightsFrame::ResizeAndMakeEffectsScroll()
{
    wxSize s;

    int w = effectsPnl->GetSize().GetWidth();
    int h = effectsPnl->GetSize().GetHeight();
    if(w>10 && h>50)
    {
        s = wxSize(w-10,h-60);
    }
    else
    {
        s = wxSize(200,200);
    }

    effectsPnl->Panel_EffectContainer->SetSize(s);
    effectsPnl->Panel_EffectContainer->SetMinSize(s);
    effectsPnl->Panel_EffectContainer->SetMaxSize(s);

    EffectsPanel1->SetSize(s);
    EffectsPanel1->SetMinSize(s);
    EffectsPanel1->SetMaxSize(s);


    EffectsPanel1->Choicebook1->SetSize(s);
    EffectsPanel1->Choicebook1->SetMinSize(s);
    EffectsPanel1->Choicebook1->SetMaxSize(s);

    EffectsPanel1->Refresh();
    EffectsPanel1->Choicebook1->Refresh();

    // Make effects window scroll by updating its container size
    wxScrolledWindow* sw = (wxScrolledWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    sw->FitInside();
    sw->SetScrollRate(5, 5);
    sw->Refresh();

}

void xLightsFrame::ResizeMainSequencer()
{
    //Play Controls
    wxSize s = mainSequencer->GetSize();
    int w = s.GetX();
    int h = s.GetY();

    mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
    mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));

    // Wave Form and Timeline
    //mainSequencer->PanelWaveForm->SetCanvasSize(w-175,75);
    //mainSequencer->PanelTimeLine->SetCanvasSize(w-175,25);

    int effectHeight = h-120;

    int effectWidth = w-20; // Width - vertical scrollbar width

    //mainSequencer->PanelRowHeadings->SetSize(wxSize(175,effectHeight));
    //mainSequencer->PanelRowHeadings->SetMinSize(wxSize(175,effectHeight));
    //mainSequencer->PanelRowHeadings->SetMaxSize(wxSize(175,effectHeight));

    // Set max rows to determine correct row information size
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());



    //mainSequencer->PanelEffectGrid->SetSize(wxSize(effectWidth-175,effectHeight));
    //mainSequencer->PanelEffectGrid->SetMinSize(wxSize(effectWidth-175,effectHeight));
    //mainSequencer->PanelEffectGrid->SetMaxSize(wxSize(effectWidth-175,effectHeight));

//    mainSequencer->ScrollBarEffectsVertical->SetSize(20,effectHeight);
//    mainSequencer->ScrollBarEffectsVertical->SetMinSize(wxSize(20,effectHeight));
//    mainSequencer->ScrollBarEffectsVertical->SetMaxSize(wxSize(20,effectHeight));

//    mainSequencer->panelEffectScrollBarSpacer->SetSize(175,20);
//    mainSequencer->panelEffectScrollBarSpacer->SetMinSize(wxSize(175,20));
//    mainSequencer->panelEffectScrollBarSpacer->SetMaxSize(wxSize(175,20));

//    mainSequencer->ScrollBarEffectGridHorz->SetSize(w-175,20);
//    mainSequencer->ScrollBarEffectGridHorz->SetMinSize(wxSize(w-175,20));
//    mainSequencer->ScrollBarEffectGridHorz->SetMaxSize(wxSize(w-175,20));


    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
//    mainSequencer->panelEffectScrollBarSpacer->Refresh();
    mainSequencer->ScrollBarEffectsHorizontal->Refresh();
    mainSequencer->ScrollBarEffectsVertical->Refresh();

    colorPanel->Refresh();
    colorPanel->ColorScrollWindow->Refresh();
    mainSequencer->UpdateEffectGridVerticalScrollBar();
}

const wxString& xLightsFrame::GetColorPalette(int i) {
    return mSequenceElements.getPalette(i);
}

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectsHorizontal->Update();
}

void xLightsFrame::SelectedEffectChanged(wxCommandEvent& event)
{
    bool OnlyChoiceBookPage = event.GetClientData()==nullptr?true:false;
    if(OnlyChoiceBookPage)
    {
        int pageIndex = event.GetInt();
        // Dont change page if it is already on correct page
        if (EffectsPanel1->Choicebook1->GetSelection()!=pageIndex)
        {
            EffectsPanel1->Choicebook1->SetSelection(pageIndex);
        }
    }
    else
    {
        Effect* effect = (Effect*)event.GetClientData();
        SetEffectControls(effect->GetEffectName(), effect->GetSettings(), effect->GetPalette());
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = effect;
    }
    wxString tooltip;
    effectsPnl->SetDragIconBuffer(GetIconBuffer(EffectsPanel1->Choicebook1->GetSelection(), tooltip));
    effectsPnl->BitmapButtonSelectedEffect->SetEffectIndex(EffectsPanel1->Choicebook1->GetSelection());
}

void xLightsFrame::EffectDroppedOnGrid(wxCommandEvent& event)
{
    int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
    mSequenceElements.UnSelectAllEffects();
    wxString name = EffectsPanel1->Choicebook1->GetPageText(effectIndex);
    int palette;
    wxString settings = GetEffectTextFromWindows(palette);
    for(int i=0;i<mSequenceElements.GetSelectedRangeCount();i++)
    {
        EffectLayer* el = mSequenceElements.GetSelectedRange(i)->Layer;
        // Delete Effects that are in same time range as dropped effect
        el->SelectEffectsInTimeRange(mSequenceElements.GetSelectedRange(i)->StartTime,
                                     mSequenceElements.GetSelectedRange(i)->EndTime);
        el->DeleteSelectedEffects();
        // Add dropped effect
        el->AddEffect(0,effectIndex,name,settings,palette,
                      mSequenceElements.GetSelectedRange(i)->StartTime,
                      mSequenceElements.GetSelectedRange(i)->EndTime,
                      EFFECT_SELECTED,false);

        playType = PLAY_TYPE_EFFECT;
        playStartTime = mSequenceElements.GetSelectedRange(i)->StartTime * 1000;
        playEndTime = mSequenceElements.GetSelectedRange(i)->EndTime * 1000;
        playStartMS = -1;
        RenderEffectForModel(el->GetParentElement()->GetName(),playStartTime,playEndTime);
    }

    mainSequencer->PanelEffectGrid->Refresh(false);
}

void xLightsFrame::PlayModel(wxCommandEvent& event)
{
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,false);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,true);

    playType = PLAY_TYPE_MODEL;
    playStartTime = mainSequencer->PanelTimeLine->GetSelectedTimeMS();
    wxString model = event.GetString();

    playBuffer.InitBuffer(GetModelNode(model),
                          mSequenceElements.GetElement(model)->GetEffectLayerCount(),
                          SeqData.FrameTime());

    playEndTime = SeqData.NumFrames() * SeqData.FrameTime();
    playStartMS = -1;
    PlayerDlg->MediaCtrl->Seek(playStartTime);
    PlayerDlg->MediaCtrl->Play();
}

void xLightsFrame::PlaySequence(wxCommandEvent& event)
{
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,false);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,true);

    playType = PLAY_TYPE_MODEL;
    playStartTime = mainSequencer->PanelTimeLine->GetSelectedTimeMS();

    playEndTime = SeqData.NumFrames() * SeqData.FrameTime();
    playStartMS = -1;
    PlayerDlg->MediaCtrl->Seek(playStartTime);
    PlayerDlg->MediaCtrl->Play();
    mainSequencer->SetIsPlaying(true);
}

void xLightsFrame::PauseSequence(wxCommandEvent& event)
{
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,false);

    PlayerDlg->MediaCtrl->Pause();
    mainSequencer->SetIsPlaying(false);
}

void xLightsFrame::StopSequence(wxCommandEvent& event)
{
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,false);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,false);

    PlayerDlg->MediaCtrl->Stop();
    PlayerDlg->MediaCtrl->Seek(playStartTime);
    mainSequencer->SetIsPlaying(false);
}

void xLightsFrame::PlayModelEffect(wxCommandEvent& event)
{
    EventPlayEffectArgs* args = (EventPlayEffectArgs*)event.GetClientData();
    playType = PLAY_TYPE_EFFECT;
    playStartTime = (int)(args->effect->GetStartTime() * 1000);
    playEndTime = (int)(args->effect->GetEndTime() * 1000);
    if(args->renderEffect)
    {
        RenderEffectForModel(args->element->GetName(),playStartTime,playEndTime);
    }
    playStartMS = -1;
    playBuffer.InitBuffer(GetModelNode(args->element->GetName()),
                          args->element->GetEffectLayerCount(),
                          SeqData.FrameTime());
}

void xLightsFrame::UpdateEffect(wxCommandEvent& event)
{
    int palette = -1;
    wxString effectText = GetEffectTextFromWindows(palette);
    int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
    wxString effectName = EffectsPanel1->Choicebook1->GetPageText(EffectsPanel1->Choicebook1->GetSelection());

    for(int i=0;i<mSequenceElements.GetRowInformationSize();i++)
    {
        Element* element = mSequenceElements.GetRowInformation(i)->element;
        if(element->GetType() == "model" || element->GetType() == "timing")
        {
            int layerIndex = mSequenceElements.GetRowInformation(i)->layerIndex;
            EffectLayer* el = element->GetEffectLayer(layerIndex);
            for(int j=0;j< el->GetEffectCount();j++)
            {
                if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)
                {
                    el->GetEffect(j)->SetSettings(effectText);
                    el->GetEffect(j)->SetEffectIndex(effectIndex);
                    el->GetEffect(j)->SetEffectName(effectName);
                    el->GetEffect(j)->SetPalette(palette);

                    playType = PLAY_TYPE_EFFECT;
                    playStartTime = (int)(el->GetEffect(j)->GetStartTime() * 1000);
                    playEndTime = (int)(el->GetEffect(j)->GetEndTime() * 1000);
                    playStartMS = -1;
                    RenderEffectForModel(element->GetName(),playStartTime,playEndTime);

                }
            }
        }
    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    if (playStartTime == playEndTime) {
        return;
    }
    if (playStartMS == -1) {
        playStartMS = msec;
    }
    int curt = (playStartTime + msec - playStartMS);
    if (curt > playEndTime) {
        playStartMS = msec;
        curt = (playStartTime + msec - playStartMS);
    }
    if (curt < 0) {
        playStartMS = -1;
        if (playType == PLAY_TYPE_MODEL) {
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,false);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,false);
            playStartTime = playEndTime = 0;
            playType = 0;
        }
        return;
    }
    if (playType == PLAY_TYPE_MODEL) {
        if (PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING) {
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,false);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,false);
            playStartTime = playEndTime = 0;
            playStartMS = -1;
            playType = 0;
            return;
        }
        double ms = PlayerDlg->MediaCtrl->Tell();
        ms /= 1000.0;
        int i = mainSequencer->PanelTimeLine->GetPositionFromTime(ms);
        mainSequencer->PanelWaveForm->PositionSelected(i);
        mainSequencer->PanelTimeLine->TimeSelected(i);
        mainSequencer->PanelWaveForm->CheckNeedToScroll();
    }

    if (selectedEffect != NULL) {
        int palette = -1;
        wxString effectText = GetEffectTextFromWindows(palette);
        if (effectText != selectedEffectString
            || palette != selectedEffectPalette) {
            selectedEffect->SetSettings(effectText);
            selectedEffect->SetPalette(palette);

            selectedEffectString = effectText;
            selectedEffectPalette = palette;

            Element *el = selectedEffect->GetParentEffectLayer()->GetParentElement();
            playStartTime = (int)(selectedEffect->GetStartTime() * 1000);
            playEndTime = (int)(selectedEffect->GetEndTime() * 1000);
            playStartMS = -1;

            RenderEffectForModel(el->GetName(),playStartTime,playEndTime);
            return;
        }
    }

    bool output = CheckBoxLightOutput->IsChecked() && xout;
    int frame = curt / SeqData.FrameTime();
    //have the frame, copy from SeqData
    int nn = playBuffer.GetNodeCount();
    size_t cn=playBuffer.ChannelsPerNode();
    for (int node = 0; node < nn; node++) {
        int start = playBuffer.NodeStartChannel(node);
        playBuffer.SetNodeChannelValues(node, &SeqData[frame][start]);
        if (output) {
            xout->SetIntensities(start,&SeqData[frame][start],cn);
        }
    }

    playBuffer.DisplayEffectOnWindow(sPreview1, mPointSize);
}


void xLightsFrame::SetEffectControls(const wxString &effectName, const wxString &origSettings, int palette)
{
    long TempLong;
    wxColour color;
    wxWindow *CtrlWin, *ContextWin;
    wxString before,after,name,value;
    wxPanel *efPanel;
    int cnt=0;

    wxString settings(origSettings);
    if (palette != -1) {
        settings += "," + mSequenceElements.getPalette(palette);
    }

    SetChoicebook(EffectsPanel1->Choicebook1, effectName);
//NOTE: the settings loop after this section does not initialize controls.
//For controls that have been added recently, an older version of the XML file will cause initial settings to be incorrect.
//A loop needs to be added to initialize the wx controls to a predictable value.
//For now, a few recently added controls are explicitly initialized here:
//(not sure if there will be side effects to using a full loop) -DJ
    timingPanel->CheckBox_LayerMorph->SetValue(false);
    EffectsPanel1->CheckBox_TextToCenter1->SetValue(false);
    EffectsPanel1->CheckBox_TextToCenter2->SetValue(false);
    EffectsPanel1->CheckBox_TextToCenter3->SetValue(false);
    EffectsPanel1->CheckBox_TextToCenter4->SetValue(false);
    EffectsPanel1->SingleStrandEffectType->SetSelection(0);
    while (!settings.IsEmpty())
    {
//NOTE: this doesn't handle "," embedded into Text lines (causes "unable to find" error): -DJ
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');

        name=before.BeforeFirst('=');
        if (name.StartsWith("E_"))
        {
            ContextWin=EffectsPanel1;
            name="ID_"+name.Mid(2);
            efPanel = EffectsPanel1;
        }
        else if (name.StartsWith("T_"))
        {
            ContextWin=timingPanel;
            name="ID_"+name.Mid(2);
            efPanel = EffectsPanel1;
        }
        else if (name.StartsWith("C_"))
        {
            ContextWin=colorPanel;
            name="ID_"+name.Mid(2);
            efPanel = EffectsPanel1;
        }
        else
        {
            settings=after;
            continue;
            //efPanel = NULL;
            //ContextWin=SeqPanelLeft;
        }
        value=before.AfterFirst('=');
        CtrlWin=wxWindow::FindWindowByName(name,ContextWin);
        if (CtrlWin)
        {
            if (name.StartsWith("ID_SLIDER"))
            {
                wxSlider* ctrl=(wxSlider*)CtrlWin;
                if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong);
            }
            else if (name.StartsWith("ID_TEXTCTRL"))
            {
                value.Replace("&comma;", ",", true); //kludge: remove escape code for "," -DJ
                wxTextCtrl* ctrl=(wxTextCtrl*)CtrlWin;
                ctrl->SetValue(value);
            }
            else if (name.StartsWith("ID_CHOICE"))
            {
                wxChoice* ctrl=(wxChoice*)CtrlWin;
                ctrl->SetStringSelection(value);
            }
            else if (name.StartsWith("ID_BUTTON"))
            {
                color.Set(value);
                if (efPanel != NULL)
                {
                    ColorPanel::SetButtonColor((wxBitmapButton*)CtrlWin, &color);
                }
                else
                {
                    CtrlWin->SetBackgroundColour(color);
                }
                CtrlWin->SetBackgroundColour(color);
                //SetTextColor(CtrlWin);
            }
            else if (name.StartsWith("ID_CHECKBOX"))
            {
                wxCheckBox* ctrl=(wxCheckBox*)CtrlWin;
                if (value.ToLong(&TempLong)) ctrl->SetValue(TempLong!=0);
            }
            else if (name.StartsWith("ID_NOTEBOOK"))
            {
                wxNotebook* ctrl=(wxNotebook*)CtrlWin;
                for (int z = 0 ; z < ctrl->GetPageCount() ; z++)
                {
                    if (value == ctrl->GetPageText(z))
                    {
                        ctrl->SetSelection(z);
                    }
                }
            }
            else
            {
                wxMessageBox("Unknown type: "+name, "Internal Error");
            }
        }
        else
        {
            wxMessageBox("Unable to find: "+name, "Internal Error");
        }
        settings=after;
        cnt++;
    }
    // set textbox values for sliders that have them
    colorPanel->UpdateSliderText();
    EffectsPanel1->UpdateSpeedText();
    timingPanel->UpdateEffectLayerMix();

    MixTypeChanged=true;
    FadesChanged=true;
    EffectsPanel1->PaletteChanged=true;
    ResetEffectStates(playResetEffectState);
}

const char** xLightsFrame::GetIconBuffer(int effectID, wxString &toolTip)
{
    const char** p_XPM;
    switch(effectID)
    {
        case xLightsFrame::RGB_EFFECTS_e::eff_OFF:
            p_XPM = Off;
            toolTip = "Off";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_ON:
            p_XPM = On;
            toolTip = "On";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_BARS:
            p_XPM = bars;
            toolTip = "Bars";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_BUTTERFLY:
            p_XPM = butterfly;
            toolTip = "Butterfly";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_CIRCLES:
            p_XPM = circles;
            toolTip = "Circles";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_COLORWASH:
            p_XPM = ColorWash;
            toolTip = "ColorWash";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_COROFACES:
            p_XPM = corofaces;
            toolTip = "Coro Faces";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_CURTAIN:
            p_XPM = curtain;
            toolTip = "Curtain";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FACES:
            p_XPM = faces;
            toolTip = "Matrix Faces";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FIRE:
            p_XPM = fire;
            toolTip = "Fire";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FIREWORKS:
            p_XPM = fireworks;
            toolTip = "Fireworks";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_GARLANDS:
            p_XPM = garlands;
            toolTip = "Garlands";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_GLEDIATOR:
            p_XPM = glediator;
            toolTip = "Glediator";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_LIFE:
            p_XPM = life;
            toolTip = "Life";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_METEORS:
            p_XPM = meteors;
            toolTip = "Meteors";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_MORPH:
            p_XPM = morph;
            toolTip = "Morph";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PIANO:
            p_XPM = piano;
            toolTip = "Piano";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PICTURES:
//~ Fix this missing "pictures" xpm
            p_XPM = Off;
            toolTip = "Pictures";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PINWHEEL:
            p_XPM = pinwheel;
            toolTip = "Pinwheel";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_RIPPLE:
            p_XPM = ripple;
            toolTip = "Ripple";
            break;
//~ Fix this missing "shimmer" xpm
        case xLightsFrame::RGB_EFFECTS_e::eff_SHIMMER:
            p_XPM = Off;
            toolTip = "Shimmer";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SINGLESTRAND:
            p_XPM = singleStrand;
            toolTip = "Single Strand";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWFLAKES:
            p_XPM = snowflakes;
            toolTip = "Snow Flakes";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWSTORM:
            p_XPM = snowstorm;
            toolTip = "Snow Storm";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIRALS:
            p_XPM = spirals;
            toolTip = "Spirals";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIROGRAPH:
            p_XPM = spirograph;
            toolTip = "Spirograph";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_STROBE:
            p_XPM = strobe;
            toolTip = "Strobe";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TEXT:
            p_XPM = text;
            toolTip = "Text";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TREE:
            p_XPM = tree;
            toolTip = "Tree";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TWINKLE:
            p_XPM = twinkle;
            toolTip = "Twinkle";
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_WAVE:
            p_XPM = wave;
            toolTip = "Wave";
            break;
        default:
            p_XPM = Off;
            toolTip = "Off";
            break;
    }
    return p_XPM;
}


wxString xLightsFrame::GetEffectTextFromWindows(int &palette)
{
    wxWindow*  window = (wxWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    // This is needed because of the "Off" effect that does not return any text.
    wxString effectText = EffectsPanel1->GetEffectStringFromWindow(window);
    if (effectText.size() > 0) {
        effectText += ",";
    }
    effectText += timingPanel->GetTimingString();
    wxString colorString = colorPanel->GetColorString();
    palette = mSequenceElements.getPaletteIndex(colorString);
    return effectText;
}

void xLightsFrame::ForceSequencerRefresh(wxCommandEvent& event)
{
    mSequenceElements.PopulateRowInformation();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::LoadPerspective(wxCommandEvent& event)
{
    wxXmlNode* perspective = (wxXmlNode*)(event.GetClientData());
    mCurrentPerpective = perspective;
    wxString name = perspective->GetAttribute("name");
    wxString settings = perspective->GetAttribute("settings");
    PerspectivesNode->DeleteAttribute("current");
    PerspectivesNode->AddAttribute("current",name);
    SaveEffectsFile();
    if(settings.size()==0)
    {
        settings = m_mgr->SavePerspective();
        mCurrentPerpective->DeleteAttribute("settings");
        mCurrentPerpective->AddAttribute("settings",settings);
    }
    bool success = m_mgr->LoadPerspective(settings,true);
    sPreview1->Refresh(false);
    m_mgr->GetPane(wxT("ModelPreview")).Show(true);
    m_mgr->Update();
}

void xLightsFrame::OnMenuItemViewSavePerspectiveSelected(wxCommandEvent& event)
{
    if(mCurrentPerpective!=NULL)
    {
        if(mCurrentPerpective->HasAttribute("settings"))
        {
            mCurrentPerpective->DeleteAttribute("settings");
        }
        mCurrentPerpective->AddAttribute("settings",m_mgr->SavePerspective());
        SaveEffectsFile();
    }
}

void xLightsFrame::PerspectivesChanged(wxCommandEvent& event)
{
    SaveEffectsFile();
}

void xLightsFrame::ShowDisplayElements(wxCommandEvent& event)
{
    displayElementsPanel->SetSequenceElementsModelsViews(&mSequenceElements,ModelsNode, ViewsNode);
    displayElementsPanel->Initialize();
    m_mgr->GetPane("DisplayElements").Show();
    m_mgr->Update();
}

Element* xLightsFrame::AddTimingElement(wxString& name)
{
    // Deactivate active timing mark so new one is selected;
    mSequenceElements.DeactivateAllTimingElements();
    int timingCount = mSequenceElements.GetNumberOfTimingRows();
    wxString type = "timing";
    Element* e = mSequenceElements.AddElement(timingCount,name,type,true,false,true,false);
    e->AddEffectLayer();
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
    return e;
}

void xLightsFrame::DeleteTimingElement(wxString& name)
{
    mSequenceElements.DeleteElement(name);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::RenameTimingElement(wxString& old_name, wxString& new_name)
{
    Element* element = mSequenceElements.GetElement(old_name);
    if( element ) element->SetName(new_name);
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(this, eventRowHeaderChanged);
}

void xLightsFrame::ExecuteImportTimingElement(wxCommandEvent &command) {
    ImportTimingElement();
}

wxArrayString xLightsFrame::ImportTimingElement()
{
    wxFileDialog* OpenDialog = new wxFileDialog( this, "Choose Audacity timing file(s)", wxEmptyString, wxEmptyString, "Text files (*.txt)|*.txt", wxFD_OPEN | wxFD_MULTIPLE, wxDefaultPosition);
    wxString fDir;
    wxArrayString new_timings;
    if (OpenDialog->ShowModal() == wxID_OK)
    {
        fDir =	OpenDialog->GetDirectory();
        wxArrayString filenames;
        OpenDialog->GetFilenames(filenames);
        CurrentSeqXmlFile->ProcessAudacityTimingFiles(fDir, filenames, this);
        for(int i = 0; i < filenames.GetCount(); ++i)
        {
            new_timings.push_back(filenames[i]);
        }
    }

    OpenDialog->Destroy();
    return new_timings;
}

void xLightsFrame::OnMenuItemLoadEditPerspectiveSelected(wxCommandEvent& event)
{
    m_mgr->GetPane(wxT("Perspectives")).Show(true);
    m_mgr->Update();
}



