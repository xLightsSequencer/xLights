#include <map>
#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/tipwin.h>
#include "../xLightsMain.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"
#include "../EffectIconPanel.h"
#include "Element.h"
#include "Effect.h"
#include "../SeqSettingsDialog.h"
#include "../DisplayElementsPanel.h"
#include "../BitmapCache.h"


#define PLAY_TYPE_STOPPED 0
#define PLAY_TYPE_EFFECT 1
#define PLAY_TYPE_MODEL  2
#define PLAY_TYPE_EFFECT_PAUSED 3
#define PLAY_TYPE_MODEL_PAUSED  4



/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
    EffectsPanel1 = NULL;
    timingPanel = NULL;

    mainSequencer = new MainSequencer(PanelSequencer);
    mainSequencer->SetSequenceElements(&mSequenceElements);

    mainSequencer->PanelWaveForm->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
    mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));

    mainSequencer->PanelRowHeadings->SetSequenceElements(&mSequenceElements);
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());

    sPreview1 = new ModelPreview(PanelSequencer);
    sPreview1->SetSize(wxSize(200,200));
    m_mgr->AddPane(sPreview1,wxAuiPaneInfo().Name(wxT("ModelPreview")).Caption(wxT("Model Preview")).
                   BestSize(wxSize(200,200)).Left());
    sPreview2 = new ModelPreview(PanelSequencer, PreviewModels, false);
    sPreview2->SetSize(wxSize(200,200));
    m_mgr->AddPane(sPreview2,wxAuiPaneInfo().Name(wxT("HousePreview")).Caption(wxT("House Preview")).
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

    m_mgr->AddPane(displayElementsPanel,wxAuiPaneInfo().Name(wxT("DisplayElements")).Caption(wxT("Sequence Elements"))
                   .Float());
    // Hide the panel on start.
    m_mgr->GetPane("DisplayElements").Hide();


    m_mgr->AddPane(perspectivePanel,wxAuiPaneInfo().Name(wxT("Perspectives")).Caption(wxT("Perspectives")).Left());

    m_mgr->AddPane(effectPalettePanel,wxAuiPaneInfo().Name(wxT("EffectDropper")).Caption(wxT("Effects")).MinSize(wxSize(150,150)).Left());

    m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effect")).Caption(wxT("Effect Settings")).MinSize(wxSize(175,175)).Left());

    m_mgr->AddPane(colorPanel,wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).Left());

    m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer/Timing")).Left());

    m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));

    mainSequencer->Layout();
    mainSequencer->PanelEffectGrid->SetRenderDataSources(this, &SeqData);
    
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
    sPreview2->InitializePreview(mBackgroundImage,mBackgroundBrightness);
    sPreview2->SetScaleBackgroundImage(mScaleBackgroundImage);
}

bool xLightsFrame::InitPixelBuffer(const wxString &modelName, PixelBufferClass &buffer, int layerCount, bool zeroBased) {
    wxXmlNode *model = GetModelNode(modelName);
    if (model == NULL) {
        model = CreateModelNodeFromGroup(modelName);
        if (model == NULL) {
            return false;
        }
        buffer.InitBuffer(model, layerCount, SeqData.FrameTime(), zeroBased);
        delete model;
    } else {
        buffer.InitBuffer(model, layerCount, SeqData.FrameTime(), zeroBased);
    }
    return true;
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
}



void xLightsFrame::CheckForValidModels()
{
    wxArrayString ModelNames;
    GetModelNames(ModelNames, true);
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

void xLightsFrame::LoadAudioData(xLightsXmlFile& xml_file)
{
    mMediaLengthMS = xml_file.GetSequenceDurationMS();

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
            wxString error;
            musicLength = mainSequencer->PanelWaveForm->OpenfileMediaFile(xml_file.GetMediaFile(), error);
            if(musicLength <=0)
            {
                wxMessageBox(wxString::Format("Media File Missing or Corrupted.\n\nDetails: %s", error));
            }
        }
        else
        {
           wxMessageBox("Media File must be specified");
        }
        if (mMediaLengthMS == 0) {
            mMediaLengthMS = musicLength;
        }
    }

    mainSequencer->PanelTimeLine->SetTimeLength(mMediaLengthMS);
    mainSequencer->PanelTimeLine->Initialize();
    int maxZoom = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
    mainSequencer->PanelTimeLine->SetZoomLevel(maxZoom);
    mainSequencer->PanelWaveForm->SetZoomLevel(maxZoom);
    mainSequencer->PanelTimeLine->RaiseChangeTimeline();  // force refresh when new media is loaded
    mainSequencer->PanelWaveForm->UpdatePlayMarker();
}

void xLightsFrame::LoadSequencer(xLightsXmlFile& xml_file)
{
    SetFrequency(xml_file.GetFrequency());
    mSequenceElements.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
    mSequenceElements.SetModelsNode(ModelsNode);
    mSequenceElements.LoadSequencerFile(xml_file);
    mSequenceElements.PopulateRowInformation();

    Menu_Settings_Sequence->Enable(true);
    MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, true);
    if( xml_file.GetRenderMode() == xLightsXmlFile::CANVAS_MODE )
    {
        MenuItemRenderEraseMode->Check(false);
        MenuItemRenderCanvasMode->Check(true);

    }
    else
    {
        MenuItemRenderEraseMode->Check(true);
        MenuItemRenderCanvasMode->Check(false);
    }

    CheckForValidModels();

    LoadAudioData(xml_file);

    mainSequencer->PanelEffectGrid->SetSequenceElements(&mSequenceElements);
    mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
    mainSequencer->PanelEffectGrid->Refresh();
    sPreview1->Refresh();
    sPreview2->Refresh();
    m_mgr->Update();
}

void xLightsFrame::EffectsResize(wxSizeEvent& event)
{
}

void xLightsFrame::EffectsPaint(wxPaintEvent& event)
{
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
        int ts = mainSequencer->ScrollBarEffectsHorizontal->GetThumbSize();
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
    mainSequencer->PanelTimeLine->SetSelectedPositionStartMS(event.GetInt());
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelTimeLine->Update();
    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelWaveForm->Update();
}

void xLightsFrame::TimelineChanged( wxCommandEvent& event)
{
    TimelineChangeArguments *tla = (TimelineChangeArguments*)(event.GetClientData());
    mainSequencer->PanelWaveForm->SetZoomLevel(tla->ZoomLevel);
    mainSequencer->PanelWaveForm->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->UpdateTimeDisplay(tla->CurrentTimeMS);
    mainSequencer->PanelTimeLine->Update();
    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelWaveForm->Update();
    mainSequencer->PanelEffectGrid->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelEffectGrid->Refresh();
    mainSequencer->PanelEffectGrid->Update();
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
    ResizeMainSequencer();
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
    mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
    mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));

    // Set max rows to determine correct row information size
    mSequenceElements.SetMaxRowsDisplayed(mainSequencer->PanelRowHeadings->GetMaxRows());
    mSequenceElements.PopulateVisibleRowInformation();

    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
    mainSequencer->ScrollBarEffectsHorizontal->Refresh();
    mainSequencer->ScrollBarEffectsVertical->Refresh();

    colorPanel->Refresh();
    colorPanel->ColorScrollWindow->Refresh();
    mainSequencer->UpdateEffectGridVerticalScrollBar();
}

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectsHorizontal->Update();
}

void xLightsFrame::UnselectedEffect(wxCommandEvent& event) {
    playType = PLAY_TYPE_STOPPED;
    playStartTime = -1;
    playEndTime = -1;
    playStartMS = -1;
    selectedEffect = NULL;
    selectedEffectString = "";
    selectedEffectPalette = "";
    sPreview1->Refresh();
    sPreview2->Refresh();
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
        bool resetStrings = false;
        if ("Random" == effect->GetEffectName()) {
            wxString settings, palette;
            wxString effectName = CreateEffectStringRandom(settings, palette);
            effect->SetEffectName(effectName);
            effect->SetEffectIndex(Effect::GetEffectIndex(effectName));
            effect->SetPalette(palette);
            effect->SetSettings(settings);
            resetStrings = true;
        }
        SetEffectControls(effect->GetEffectName(), effect->GetSettings(), effect->GetPaletteMap());
        selectedEffectString = GetEffectTextFromWindows(selectedEffectPalette);
        selectedEffect = effect;
        if (effect->GetPaletteMap().empty() || resetStrings) {
            effect->SetPalette(selectedEffectPalette);
            effect->SetSettings(selectedEffectString);
            RenderEffectForModel(effect->GetParentEffectLayer()->GetParentElement()->GetName(),
                                 effect->GetStartTime() * 1000,
                                 effect->GetEndTime() * 1000);
        }

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            mainSequencer->PanelTimeLine->PlayStopped();
            mainSequencer->PanelWaveForm->UpdatePlayMarker();
            mainSequencer->UpdateTimeDisplay(playStartTime);
        }

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = effect->GetStartTime() * 1000;
            playEndTime = effect->GetEndTime() * 1000;
            playStartMS = -1;

            InitPixelBuffer(effect->GetParentEffectLayer()->GetParentElement()->GetName(),
                            playBuffer,
                            effect->GetParentEffectLayer()->GetParentElement()->GetEffectLayerCount());

            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_FIRST_FRAME,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_LAST_FRAME,false);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_REPLAY_SECTION,false);
        }

    }
    wxString tooltip;
    effectsPnl->SetDragIconBuffer(BitmapCache::GetEffectIcon(EffectsPanel1->Choicebook1->GetSelection(), tooltip));
    effectsPnl->BitmapButtonSelectedEffect->SetEffectIndex(EffectsPanel1->Choicebook1->GetSelection());
    mainSequencer->PanelEffectGrid->SetFocus();
}

void xLightsFrame::EffectDroppedOnGrid(wxCommandEvent& event)
{
    int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
    mSequenceElements.UnSelectAllEffects();
    wxString name = EffectsPanel1->Choicebook1->GetPageText(effectIndex);
    wxString palette;
    wxString settings = GetEffectTextFromWindows(palette);
    selectedEffect = NULL;

    for(int i=0;i<mSequenceElements.GetSelectedRangeCount();i++)
    {
        EffectLayer* el = mSequenceElements.GetSelectedRange(i)->Layer;
        if (el->GetParentElement()->GetType() != "model") {
            continue;
        }
        // Delete Effects that are in same time range as dropped effect
        el->SelectEffectsInTimeRange(mSequenceElements.GetSelectedRange(i)->StartTime,
                                     mSequenceElements.GetSelectedRange(i)->EndTime);
        el->DeleteSelectedEffects();
        // Add dropped effect
        el->AddEffect(0,effectIndex,name,settings,palette,
                      mSequenceElements.GetSelectedRange(i)->StartTime,
                      mSequenceElements.GetSelectedRange(i)->EndTime,
                      EFFECT_SELECTED,false);

        if (playType == PLAY_TYPE_MODEL_PAUSED) {
            mainSequencer->PanelTimeLine->PlayStopped();
            mainSequencer->PanelWaveForm->UpdatePlayMarker();
            mainSequencer->UpdateTimeDisplay(playStartTime);
        }

        if (playType != PLAY_TYPE_MODEL) {
            playType = PLAY_TYPE_EFFECT;
            playStartTime = mSequenceElements.GetSelectedRange(i)->StartTime * 1000;
            playEndTime = mSequenceElements.GetSelectedRange(i)->EndTime * 1000;
            playStartMS = -1;
            RenderEffectForModel(el->GetParentElement()->GetName(),playStartTime,playEndTime);

            InitPixelBuffer(el->GetParentElement()->GetName(), playBuffer,
                            el->GetParentElement()->GetEffectLayerCount());

            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_FIRST_FRAME,true);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_LAST_FRAME,false);
            EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_REPLAY_SECTION,false);
        }
    }

    mainSequencer->PanelEffectGrid->Refresh(false);
}

void xLightsFrame::PlayModel(wxCommandEvent& event)
{
    wxString model = event.GetString();
    if (InitPixelBuffer(model, playBuffer, mSequenceElements.GetElement(model)->GetEffectLayerCount())
        && playType != PLAY_TYPE_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::ModelSelected(wxCommandEvent& event)
{
    if (playType == PLAY_TYPE_MODEL)
    {
        wxString model = event.GetString();
        InitPixelBuffer(model, playBuffer, mSequenceElements.GetElement(model)->GetEffectLayerCount());
    }
}

void xLightsFrame::PlaySequence(wxCommandEvent& event)
{
    if( CurrentSeqXmlFile != NULL )
    {
        EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
        EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,true);
        EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_FIRST_FRAME,false);
        EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_LAST_FRAME,false);
        if( playType == PLAY_TYPE_EFFECT_PAUSED )
        {
            playType = PLAY_TYPE_EFFECT;
            playStartMS = -1;
        }
        else
        {
            playType = PLAY_TYPE_MODEL;
            playStartMS = -1;
            playStartTime = mainSequencer->PanelTimeLine->GetNewStartTimeMS();
            playEndTime = mainSequencer->PanelTimeLine->GetNewEndTimeMS();
            if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
                PlayerDlg->MediaCtrl->Seek(playStartTime);
            }
            if( playEndTime == -1 || playEndTime > CurrentSeqXmlFile->GetSequenceDurationMS()) {
                playEndTime = CurrentSeqXmlFile->GetSequenceDurationMS();
            }
            mainSequencer->PanelTimeLine->PlayStarted();
            if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
                PlayerDlg->MediaCtrl->Play();
            }
        }
    }
}

void xLightsFrame::PauseSequence(wxCommandEvent& event)
{
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" )
    {
        if (PlayerDlg->MediaCtrl->GetState() == wxMEDIASTATE_PLAYING) {
            PlayerDlg->MediaCtrl->Pause();
        }
        else if (PlayerDlg->MediaCtrl->GetState() == wxMEDIASTATE_PAUSED) {
            PlayerDlg->MediaCtrl->Play();
        }
    }

    if (playType == PLAY_TYPE_MODEL) {
        playType = PLAY_TYPE_MODEL_PAUSED;
    }
    else if (playType == PLAY_TYPE_MODEL_PAUSED) {
        EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,true);
        playType = PLAY_TYPE_MODEL;
    }
    else if (playType == PLAY_TYPE_EFFECT_PAUSED) {
        playType = PLAY_TYPE_EFFECT;
    }
    else {
        playType = PLAY_TYPE_EFFECT_PAUSED;
    }
}

void xLightsFrame::TogglePlay(wxCommandEvent& event)
{
    if( playType == PLAY_TYPE_MODEL )
    {
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
    else if( playType == PLAY_TYPE_MODEL_PAUSED )
    {
        wxCommandEvent playEvent(EVT_PAUSE_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
    else
    {
        wxCommandEvent playEvent(EVT_PLAY_SEQUENCE);
        wxPostEvent(this, playEvent);
    }
}

void xLightsFrame::StopSequence(wxCommandEvent& event)
{
    if( playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_MODEL_PAUSED )
    {
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
            PlayerDlg->MediaCtrl->Stop();
            PlayerDlg->MediaCtrl->Seek(playStartTime);
        }
        mainSequencer->PanelTimeLine->PlayStopped();
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->UpdateTimeDisplay(playStartTime);
    }
    playType = PLAY_TYPE_STOPPED;
    if( CheckBoxLightOutput->IsChecked() && xout ) {
        xout->alloff();
    }
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PLAY_NOW,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_STOP,false);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_PAUSE,false);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_FIRST_FRAME,true);
    EnableToolbarButton(PlayToolBar,ID_AUITOOLBAR_LAST_FRAME,true);
}

void xLightsFrame::SequenceFirstFrame(wxCommandEvent& event)
{
    if( playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_EFFECT ) {
        playStartMS = -1;
    }
    else
    {
        mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(0);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        wxPostEvent(this, eventScroll);

        mainSequencer->PanelTimeLine->ResetMarkers(0);
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->UpdateTimeDisplay(0);
    }
}

void xLightsFrame::SequenceLastFrame(wxCommandEvent& event)
{
    int limit = mainSequencer->ScrollBarEffectsHorizontal->GetRange();
    mainSequencer->ScrollBarEffectsHorizontal->SetThumbPosition(limit-1);
    wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
    wxPostEvent(this, eventScroll);

    int end_ms = CurrentSeqXmlFile->GetSequenceDurationMS();
    mainSequencer->PanelTimeLine->ResetMarkers(end_ms);
    mainSequencer->PanelWaveForm->UpdatePlayMarker();
    mainSequencer->UpdateTimeDisplay(end_ms);
}

void xLightsFrame::SequenceReplaySection(wxCommandEvent& event)
{
    //FIXME implement this:  use as a loop flag?
}

void xLightsFrame::PlayModelEffect(wxCommandEvent& event)
{
    if( playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
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
        InitPixelBuffer(args->element->GetName(), playBuffer, args->element->GetEffectLayerCount());
    }
}

void xLightsFrame::UpdateEffect(wxCommandEvent& event)
{
    wxString palette;
    wxString effectText = GetEffectTextFromWindows(palette);
    int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
    wxString effectName = EffectsPanel1->Choicebook1->GetPageText(EffectsPanel1->Choicebook1->GetSelection());

    for(int i=0;i<mSequenceElements.GetRowInformationSize();i++)
    {
        Element* element = mSequenceElements.GetRowInformation(i)->element;
        if(element->GetType() == "model" || element->GetType() == "timing")
        {
            EffectLayer* el = mSequenceElements.GetEffectLayer(i);
            for(int j=0;j< el->GetEffectCount();j++)
            {
                if(el->GetEffect(j)->GetSelected() != EFFECT_NOT_SELECTED)
                {
                    el->GetEffect(j)->SetSettings(effectText);
                    el->GetEffect(j)->SetEffectIndex(effectIndex);
                    el->GetEffect(j)->SetEffectName(effectName);
                    el->GetEffect(j)->SetPalette(palette);

                    if(playType != PLAY_TYPE_MODEL && playType != PLAY_TYPE_MODEL_PAUSED)
                    {
                        playType = PLAY_TYPE_EFFECT;
                        playStartTime = (int)(el->GetEffect(j)->GetStartTime() * 1000);
                        playEndTime = (int)(el->GetEffect(j)->GetEndTime() * 1000);
                        playStartMS = -1;
                        RenderEffectForModel(element->GetName(),playStartTime,playEndTime);
                    }
                }
            }
        }
    }
    mainSequencer->PanelEffectGrid->ForceRefresh();
}

void xLightsFrame::TimerRgbSeq(long msec)
{
    // Update play status so sequencer grid can allow dropping timings during playback
    mainSequencer->SetPlayStatus(playType);

    // return if play is stopped
    if (playType == PLAY_TYPE_STOPPED || CurrentSeqXmlFile == NULL) {
        return;
    }

    // return if paused
    if (playType == PLAY_TYPE_EFFECT_PAUSED || playType == PLAY_TYPE_MODEL_PAUSED) {
        playStartMS = msec - playOffsetTime;  // maintain offset so we can restart where we paused
        return;
    }

    // return if we have reset play times
    if (playEndTime == 0) {
        return;
    }

    // capture start time if necessary
    if (playStartMS == -1) {
        playStartMS = msec;
    }

    // record current time
    int curt = (playStartTime + msec - playStartMS);
    playOffsetTime = msec - playStartMS;

    // repeat loop if in play effect mode
    if (curt > playEndTime && playType == PLAY_TYPE_EFFECT) {
        playStartMS = msec;
        curt = playStartTime;
    }

    // reset all if current time is invalid
    if (curt < 0) {
        playStartMS = -1;
        if (playType == PLAY_TYPE_MODEL) {
            playStartTime = playEndTime = 0;
            playType = PLAY_TYPE_STOPPED;
        }
        return;
    }

    if (playType == PLAY_TYPE_MODEL) {
        // see if its time to stop model play
        if ((curt > playEndTime) || (CurrentSeqXmlFile->GetSequenceType() == "Media" && PlayerDlg->MediaCtrl->GetState() != wxMEDIASTATE_PLAYING)) {
            playStartTime = playEndTime = 0;
            playStartMS = -1;
            wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
            wxPostEvent(this, playEvent);
            return;
        }
        int current_play_time = 0;
        if( CurrentSeqXmlFile->GetSequenceType() == "Media" ) {
            current_play_time = PlayerDlg->MediaCtrl->Tell();
        }
        else {
            current_play_time = curt;
        }
        mainSequencer->PanelTimeLine->SetPlayMarkerMS(current_play_time);
        mainSequencer->PanelWaveForm->UpdatePlayMarker();
        mainSequencer->PanelWaveForm->CheckNeedToScroll();
        mainSequencer->UpdateTimeDisplay(current_play_time);
    }

    if (selectedEffect != NULL) {
        wxString palette;
        wxString effectText = GetEffectTextFromWindows(palette);
        if (effectText != selectedEffectString
            || palette != selectedEffectPalette) {

            int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
            wxString name = EffectsPanel1->Choicebook1->GetPageText(effectIndex);
            if (name !=  selectedEffect->GetEffectName()) {
                selectedEffect->SetEffectName(name);
                selectedEffect->SetEffectIndex(EffectsPanel1->Choicebook1->GetSelection());
            }

            selectedEffect->SetSettings(effectText);
            selectedEffect->SetPalette(palette);

            selectedEffectString = effectText;
            selectedEffectPalette = palette;

            Element *el = selectedEffect->GetParentEffectLayer()->GetParentElement();
            playStartTime = (int)(selectedEffect->GetStartTime() * 1000);
            playEndTime = (int)(selectedEffect->GetEndTime() * 1000);
            playStartMS = -1;

            RenderEffectForModel(el->GetName(),playStartTime,playEndTime);
            mainSequencer->PanelEffectGrid->ForceRefresh();
            return;
        }
    }

    int frame = curt / SeqData.FrameTime();
    //have the frame, copy from SeqData
    int nn = playBuffer.GetNodeCount();
    for (int node = 0; node < nn; node++) {
        int start = playBuffer.NodeStartChannel(node);
        playBuffer.SetNodeChannelValues(node, &SeqData[frame][start]);
    }
    TimerOutput(frame);
    playBuffer.DisplayEffectOnWindow(sPreview1, mPointSize);
    sPreview2->Render(&SeqData[frame][0]);
}

void xLightsFrame::SetEffectControls(const wxString &effectName, const MapStringString &settings, const MapStringString &palette) {
    SetChoicebook(EffectsPanel1->Choicebook1, effectName);
    SetEffectControls(settings);
    SetEffectControls(palette);
}
void xLightsFrame::SetEffectControls(const MapStringString &settings) {
    long TempLong;
    wxColour color;
    wxWindow *CtrlWin, *ContextWin;
    wxString before,after,name,value;
    wxPanel *efPanel;
    int cnt=0;

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
    for (std::map<wxString,wxString>::const_iterator it=settings.begin(); it!=settings.end(); ++it) {
//NOTE: this doesn't handle "," embedded into Text lines (causes "unable to find" error): -DJ
        name = it->first;
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
            continue;
            //efPanel = NULL;
            //ContextWin=SeqPanelLeft;
        }
        value=it->second;
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
        cnt++;
    }
    // set textbox values for sliders that have them
    colorPanel->UpdateSliderText();
    EffectsPanel1->UpdateSpeedText();
    timingPanel->UpdateEffectLayerMix();

    MixTypeChanged=true;
    FadesChanged=true;
    EffectsPanel1->PaletteChanged=true;
}

wxString xLightsFrame::GetEffectTextFromWindows(wxString &palette)
{
    wxWindow*  window = (wxWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    // This is needed because of the "Off" effect that does not return any text.
    wxString effectText = EffectsPanel1->GetEffectStringFromWindow(window);
    if (effectText.size() > 0) {
        effectText += ",";
    }
    effectText += timingPanel->GetTimingString();
    palette = colorPanel->GetColorString();
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
    m_mgr->LoadPerspective(settings,true);
    sPreview1->Refresh(false);
    sPreview2->Refresh(false);
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
    displayElementsPanel->Initialize();
    m_mgr->GetPane("DisplayElements").Show();
    m_mgr->Update();
}


void xLightsFrame::ShowHideEffectSettingsWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("Effect").IsShown();
    if (visible) {
        m_mgr->GetPane("Effect").Hide();
    } else {
        m_mgr->GetPane("Effect").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideColorWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("Color").IsShown();
    if (visible) {
        m_mgr->GetPane("Color").Hide();
    } else {
        m_mgr->GetPane("Color").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideLayerTimingWindow(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("LayerTiming").IsShown();
    if (visible) {
        m_mgr->GetPane("LayerTiming").Hide();
    } else {
        m_mgr->GetPane("LayerTiming").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideModelPreview(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("ModelPreview").IsShown();
    if (visible) {
        m_mgr->GetPane("ModelPreview").Hide();
    } else {
        m_mgr->GetPane("ModelPreview").Show();
    }
    m_mgr->Update();
}


void xLightsFrame::ShowHideHousePreview(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("HousePreview").IsShown();
    if (visible) {
        m_mgr->GetPane("HousePreview").Hide();
    } else {
        m_mgr->GetPane("HousePreview").Show();
    }
    m_mgr->Update();
}

void xLightsFrame::ShowHideEffectDropper(wxCommandEvent& event)
{
    bool visible = m_mgr->GetPane("EffectDropper").IsShown();
    if (visible) {
        m_mgr->GetPane("EffectDropper").Hide();
    } else {
        m_mgr->GetPane("EffectDropper").Show();
    }
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



