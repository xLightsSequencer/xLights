#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"


#if __WXOSX__
#define BASEPATH wxString("/tmp/")
#else
#define BASEPATH wxString("c:\\temp\\")
#endif

/************************************* New Sequencer Code*****************************************/
void xLightsFrame::CreateSequencer()
{
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
        if(mSequencerInitialize || EffectsPanel1 == NULL)
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
    mainSequencer->ScrolledEffectsGrid->SetScrollbars(0, 100, 0, 10);
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
    Effect* effect = (Effect*)event.GetClientData();
    wxString name = "";
    if(effect!=nullptr)
    {
        SetEffectControls(effect->GetEffectName(),effect->GetSettings(),name);
    }

    wxBitmap bm(GetIconBuffer(EffectsPanel1->Choicebook1->GetSelection()));
    effectsPnl->BitmapButtonSelectedEffect->SetBitmapLabel(bm);

}



void xLightsFrame::SetEffectControls(wxString effectName, wxString settings, const wxString& model_name)
{
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
    timingPanel->CheckBox_LayerMorph->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter1->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter2->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter3->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter4->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->SingleStrandEffectType->SetSelection(0); //Set to first page in case not present

    SetChoicebook(EffectsPanel1->Choicebook1, effectName);
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
                    SetButtonColor((wxButton*)CtrlWin, &color);
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
    wxScrollEvent evt;
//    OnSlider_BrightnessCmdScroll(evt);
//    OnSlider_ContrastCmdScroll(evt);
//    OnSlider_EffectLayerMixCmdScroll(evt);

//    OnSlider_SparkleFrequencyCmdScroll(evt);
//    OnSlider_Model_BrightnessCmdScroll(evt);
//   OnSlider_SparkleSliderCmdScroll(evt);

    EffectsPanel1->UpdateSpeedText();

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
            p_XPM = Off;
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
        case xLightsFrame::RGB_EFFECTS_e::eff_CURTAIN:
            p_XPM = curtain;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_FACES:
            p_XPM = Off;
            //p_XPM = faces;
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
            p_XPM = Off;
           // p_XPM = strobe;
            break;
        case xLightsFrame::RGB_EFFECTS_e::eff_TEXT:
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

