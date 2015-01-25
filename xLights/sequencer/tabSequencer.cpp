#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"
#include "../EffectIconPanel.h"


#ifdef __WXMSW__
#define BASEPATH wxString("c:\\temp\\")
#else
#define BASEPATH wxString("/tmp/")
#endif

/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
    EffectsPanel1 = NULL;
    timingPanel = NULL;

    mSequenceElements.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
    mSequenceElements.SetFrequency(40);
    bool success = mSequenceElements.LoadSequencerFile(BASEPATH + "v4.xml");

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    mainSequencer = new MainSequencer(PanelSequencer);

    DOUBLE_BUFFER(mainSequencer);
    mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
    mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));

    mainSequencer->PanelRowHeadings->SetSequenceElements(&mSequenceElements);
    mainSequencer->PanelRowHeadings->SetCanvasSize(175,2200);

    mainSequencer->PanelRowHeadings->SetSize(wxSize(175,2200));
    mainSequencer->PanelRowHeadings->SetMinSize(wxSize(175,2200));

    mainSequencer->PanelEffectGrid->SetSize(wxSize(1200,2200));
    mainSequencer->PanelEffectGrid->SetMinSize(wxSize(1200,2200));

    mainSequencer->ScrolledEffectsGrid->SetSize(wxSize(1525,700));
    mainSequencer->ScrolledEffectsGrid->SetMinSize(wxSize(1525,700));
    mainSequencer->ScrolledEffectsGrid->SetMaxSize(wxSize(1525,700));
    mainSequencer->ScrolledEffectsGrid->SetScrollbars(0, 100, 0, 10);

    mainSequencer->panelEffectScrollBarSpacer->SetSize(175,20);
    mainSequencer->panelEffectScrollBarSpacer->SetMinSize(wxSize(175,20));
    mainSequencer->panelEffectScrollBarSpacer->SetMaxSize(wxSize(175,20));

    mainSequencer->ScrollBarEffectGridHorz->SetSize(1200,20);
    mainSequencer->ScrollBarEffectGridHorz->SetMinSize(wxSize(1200,20));
    mainSequencer->ScrollBarEffectGridHorz->SetMaxSize(wxSize(1200,20));

    mMediaLengthMS = mainSequencer->PanelWaveForm->OpenfileMediaFile(BASEPATH + "4.mp3");
    mainSequencer->PanelWaveForm->SetCanvasSize(1200,75);

    mainSequencer->PanelTimeLine->SetTimeLength(mMediaLengthMS);

    mainSequencer->PanelTimeLine->SetCanvasSize(1200,25);
    mainSequencer->PanelTimeLine->Initialize();

    mainSequencer->PanelEffectGrid->SetCanvasSize(1200,2200);
    mainSequencer->PanelEffectGrid->SetSequenceElements(&mSequenceElements);
    mainSequencer->PanelEffectGrid->SetTimeline(mainSequencer->PanelTimeLine);
    mainSequencer->PanelEffectGrid->InitializeGrid();

    sPreview1 = new SequencePreview(PanelSequencer,args);
    sPreview1->SetSize(wxSize(200,200));
    sPreview1->InitializePreview();
    m_mgr->AddPane(sPreview1, wxLEFT, wxT("Model Preview 1"));

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

    EffectIconPanel* effectIconPanel1 = new EffectIconPanel(PanelSequencer);

    m_mgr->AddPane(effectIconPanel1,wxAuiPaneInfo().Name(wxT("EffectIcons1")).Caption(wxT("Effects")).
                   BestSize(wxSize(150,150)).MinSize(wxSize(150,150)).Left());

    m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effect")).Caption(wxT("Effects")).
                   BestSize(wxSize(175,175)).MinSize(wxSize(175,175)).Left());

    m_mgr->AddPane(colorPanel,wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).
                   BestSize(wxSize(175,175)).Left());

    m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer/Timing")).
                   BestSize(wxSize(175,175)).Left());

    m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));
    m_mgr->Update();
}

void xLightsFrame::InitSequencer()
{
        if(mSequencerInitialize || EffectsPanel1 == NULL || timingPanel == NULL)
        {
            return;
        }
        mSequencerInitialize = true;
        ResizeAndMakeEffectsScroll();
        ResizeMainSequencer();
        int maxZoom = mainSequencer->PanelTimeLine->GetMaxZoomLevel();
        mainSequencer->PanelTimeLine->SetZoomLevel(maxZoom);
        mainSequencer->PanelWaveForm->SetZoomLevel(maxZoom);
        mainSequencer->PanelWaveForm->Refresh();
        mainSequencer->PanelEffectGrid->Refresh();
        m_mgr->Update();
        PlayerDlg->MediaCtrl->Load(BASEPATH + "4.mp3");


        sPreview1->Refresh();
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

    int position = mainSequencer->ScrollBarEffectGridHorz->GetThumbPosition();
    int timeLength = mainSequencer->PanelTimeLine->GetTimeLength();

    int startTime = (int)(((double)position/(double)timeLength) * (double)timeLength);
    mainSequencer->PanelTimeLine->SetStartTimeMS(startTime);

//    mainSequencer->PanelTimeLine->GetViewableTimeRange(startTime,endTime);

}

void xLightsFrame::TimeSelected( wxCommandEvent& event)
{
    // event.GetInt holds position without first pixelOffset
    mainSequencer->PanelTimeLine->TimeSelected(event.GetInt());
}



void xLightsFrame::TimelineChanged( wxCommandEvent& event)
{
    TimelineChangeArguments *tla = (TimelineChangeArguments*)(event.GetClientData());
    mainSequencer->PanelWaveForm->SetZoomLevel(tla->ZoomLevel);
    mainSequencer->PanelWaveForm->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelWaveForm->PositionSelected(tla->SelectedPosition);
    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelEffectGrid->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelEffectGrid->Refresh();
    UpdateEffectGridHorizontalScrollBar();
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
        mainSequencer->ScrollBarEffectGridHorz->SetScrollbar(0,thumbSize,range,pageSize);
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
        mainSequencer->ScrollBarEffectGridHorz->SetScrollbar(position,thumbSize,range,pageSize);
    }

    mainSequencer->ScrollBarEffectGridHorz->Refresh();
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
    mainSequencer->PanelWaveForm->SetCanvasSize(w-175,75);
    mainSequencer->PanelTimeLine->SetCanvasSize(w-175,25);

    int effectHeight = mSequenceElements.GetRowInformationSize()* DEFAULT_ROW_HEADING_HEIGHT > h-120?
                       mSequenceElements.GetRowInformationSize()* DEFAULT_ROW_HEADING_HEIGHT:h-120;
    mainSequencer->PanelRowHeadings->SetSize(wxSize(175,effectHeight));
    mainSequencer->PanelRowHeadings->SetMinSize(wxSize(175,effectHeight));
    mainSequencer->PanelRowHeadings->SetMaxSize(wxSize(175,effectHeight));

    mainSequencer->PanelEffectGrid->SetSize(wxSize(w-175,effectHeight));
    mainSequencer->PanelEffectGrid->SetMinSize(wxSize(w-175,effectHeight));
    mainSequencer->PanelEffectGrid->SetMaxSize(wxSize(w-175,effectHeight));

    mainSequencer->panelEffectScrollBarSpacer->SetSize(175,20);
    mainSequencer->panelEffectScrollBarSpacer->SetMinSize(wxSize(175,20));
    mainSequencer->panelEffectScrollBarSpacer->SetMaxSize(wxSize(175,20));

    mainSequencer->ScrollBarEffectGridHorz->SetSize(w-175,20);
    mainSequencer->ScrollBarEffectGridHorz->SetMinSize(wxSize(w-175,20));
    mainSequencer->ScrollBarEffectGridHorz->SetMaxSize(wxSize(w-175,20));

    mainSequencer->ScrolledEffectsGrid->SetSize(wxSize(w,h-120));
    mainSequencer->ScrolledEffectsGrid->SetMinSize(wxSize(w,h-120));
    mainSequencer->ScrolledEffectsGrid->SetMaxSize(wxSize(w,h-120));
    mainSequencer->ScrolledEffectsGrid->SetScrollbars(0, 10, 0, 2);
    mainSequencer->ScrolledEffectsGrid->FitInside();

    //colorPanel->SetSize(wxSize(50,50));
    //colorPanel->SetSize(wxSize(50,50));
    //colorPanel->SetSize(wxSize(50,50));

    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
    mainSequencer->panelEffectScrollBarSpacer->Refresh();
    mainSequencer->ScrollBarEffectGridHorz->Refresh();
    mainSequencer->ScrolledEffectsGrid->Refresh();
    colorPanel->Refresh();
    colorPanel->ColorScrollWindow->Refresh();
}

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}

void xLightsFrame::SelectedEffectChanged( wxCommandEvent& event)
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
        SetEffectControls(effect->GetEffectName(),effect->GetSettings());
    }
    effectsPnl->SetDragIconBuffer(GetIconBuffer(EffectsPanel1->Choicebook1->GetSelection()));
    effectsPnl->BitmapButtonSelectedEffect->SetEffectIndex(EffectsPanel1->Choicebook1->GetSelection());
}

void xLightsFrame::EffectDroppedOnGrid(wxCommandEvent& event)
{
    Effect* effect;
    int effectIndex = EffectsPanel1->Choicebook1->GetSelection();
    mSequenceElements.UnSelectAllEffects();
    wxString name = EffectsPanel1->Choicebook1->GetPageText(effectIndex);
    wxString settings = GetEffectTextFromWindows();
    for(int i=0;i<mSequenceElements.GetSelectedRangeCount();i++)
    {
       mSequenceElements.GetSelectedRange(i)->Layer->AddEffect(0,effectIndex,name,settings,
                                   mSequenceElements.GetSelectedRange(i)->StartTime,
                                   mSequenceElements.GetSelectedRange(i)->EndTime,EFFECT_SELECTED,false);
    }

    mainSequencer->PanelEffectGrid->Refresh(false);
}

void xLightsFrame::PlayModelEffect(wxCommandEvent& event)
{
    EventPlayEffectArgs* args = (EventPlayEffectArgs*)event.GetClientData();
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
    wxWindow*  window = (wxWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    wxString effectText = GetEffectTextFromWindows();
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
    int frame = curt / SeqData.FrameTime();
    //have the frame, copy from SeqData
    int nn = playBuffer.GetNodeCount();
    unsigned char intensity;
    size_t chnum;
    size_t cn = playBuffer.ChannelsPerNode();
    for (int node = 0; node < nn; node++) {
        if (cn == 1) {
            playBuffer.GetChanIntensity(node, 0, &chnum, &intensity);
            intensity = SeqData[frame][chnum];
            playBuffer.SetChanIntensityAll(node, intensity);
        } else {
            for(size_t c = 0; c < cn; c++)
            {
                playBuffer.GetChanIntensity(node, c, &chnum, &intensity);
                intensity = SeqData[frame][chnum];
                playBuffer.SetChanIntensity(node, c, intensity);
            }
        }
    }

    playBuffer.DisplayEffectOnWindow(sPreview1, mPointSize);
}


void xLightsFrame::SetEffectControls(wxString effectName, wxString settings)
{
    long TempLong;
    wxColour color;
    wxWindow *CtrlWin, *ContextWin;
    wxString before,after,name,value;
    wxPanel *efPanel;
    int cnt=0;

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

const char** xLightsFrame::GetIconBuffer(int effectID)
{
    const char** p_XPM;
    switch(effectID)
    {
        case xLightsFrame::RGB_EFFECTS_e::eff_OFF:
            p_XPM = Off;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_ON:
            p_XPM = On;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_BARS:
            p_XPM = bars;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_BUTTERFLY:
            p_XPM = butterfly;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_CIRCLES:
            p_XPM = circles;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_COLORWASH:
            p_XPM = ColorWash;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_COROFACES:
            p_XPM = corofaces;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_CURTAIN:
            p_XPM = curtain;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FACES:
            p_XPM = faces;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FIRE:
            p_XPM = fire;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FIREWORKS:
            p_XPM = fireworks;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_GARLANDS:
            p_XPM = garlands;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_GLEDIATOR:
            p_XPM = glediator;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_LIFE:
            p_XPM = life;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_METEORS:
            p_XPM = meteors;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PIANO:
            p_XPM = pinwheel;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PICTURES:
//~ Fix this missing "pictures" xpm
            p_XPM = Off;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_PINWHEEL:
            p_XPM = pinwheel;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_RIPPLE:
            p_XPM = ripple;
            break;
//~ Fix this missing "shimmer" xpm
        case xLightsFrame::RGB_EFFECTS_e::eff_SHIMMER:
            p_XPM = Off;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SINGLESTRAND:
            p_XPM = singleStrand;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWFLAKES:
            p_XPM = snowflakes;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SNOWSTORM:
            p_XPM = snowstorm;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIRALS:
            p_XPM = spirals;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_SPIROGRAPH:
            p_XPM = spirograph;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_STROBE:
  //         p_XPM = Off;
          p_XPM = strobe;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TEXT:
            p_XPM = text;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TREE:
            p_XPM = tree;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TWINKLE:
            p_XPM = twinkle;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_WAVE:
            p_XPM = wave;
            break;
        default:
            p_XPM = Off;
            break;
    }
    return p_XPM;
}


wxString xLightsFrame::GetEffectTextFromWindows()
{
    wxWindow*  window = (wxWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    // This is needed because of the "Off" effect that does not return any text.
    wxString comma = EffectsPanel1->GetEffectStringFromWindow(window).size()>0?",":"";
    wxString effectText = EffectsPanel1->GetEffectStringFromWindow(window) + comma +
                          colorPanel->GetColorString() + "," + timingPanel->GetTimingString();
    return effectText;
}

