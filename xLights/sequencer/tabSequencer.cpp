#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"
#include "SequenceElements.h"
#include "../TopEffectsPanel.h"


/************************************* New Sequencer Code*****************************************/
void xLightsFrame::InitSequencer()
{
        if(mSequencerInitialize)
        {
            return;
        }

        mSequenceElements.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
        mSequenceElements.SetFrequency(40);
        bool success = mSequenceElements.LoadSequencerFile("c:\\temp\\v4.xml");
        mSequencerInitialize = true;

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

        mMediaLengthMS = mainSequencer->PanelWaveForm->OpenfileMediaFile("c:\\temp\\4.mp3");
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
        EffectsPanel1 = new EffectsPanel(effectsPnl->EffectsNotebook, ID_PANEL_EFFECTS1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1"));
        EffectsPanel2 = new EffectsPanel(effectsPnl->EffectsNotebook, ID_PANEL_EFFECTS2, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS2"));

        effectsPnl->EffectsNotebook->AddPage(EffectsPanel1, _("Effect1"), false);
        effectsPnl->EffectsNotebook->AddPage(EffectsPanel2, _("Effect2"), false);
        effectsPnl->Refresh();


        wxScrolledWindow* w;
        for(int i =0;i<EffectsPanel1->Choicebook1->GetPageCount();i++)
        {
            w = (wxScrolledWindow*)EffectsPanel1->Choicebook1->GetPage(i);
            w->FitInside();
            w->SetScrollRate(5, 5);
        }

        for(int i =0;i<EffectsPanel2->Choicebook1->GetPageCount();i++)
        {
            w = (wxScrolledWindow*)EffectsPanel2->Choicebook1->GetPage(i);
            w->FitInside();
            w->SetScrollRate(5, 5);
        }

        colorPanel = new ColorPanel(PanelSequencer);
        timingPanel = new TimingPanel(PanelSequencer);

        m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effects")).Caption(wxT("Effects")).
                       BestSize(wxSize(175,175)).MinSize(wxSize(175,175)).Left());

        m_mgr->AddPane(colorPanel,wxAuiPaneInfo().Name(wxT("Color")).Caption(wxT("Color")).
                       BestSize(wxSize(175,175)).MinSize(wxSize(175,175)).Left());

        m_mgr->AddPane(timingPanel,wxAuiPaneInfo().Name(wxT("LayerTiming")).Caption(wxT("Layer/Timing")).
                       BestSize(wxSize(175,175)).MinSize(wxSize(175,175)).Left());

        m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));
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
    mainSequencer->ScrollBarEffectGridHorz->SetRange(((MAX_ZOOM_OUT_INDEX - tla->ZoomLevel)*5)+1);
    mainSequencer->ScrollBarEffectGridHorz->Refresh();

    mainSequencer->PanelEffectGrid->SetStartPixelOffset(tla->StartPixelOffset);
    mainSequencer->PanelEffectGrid->Refresh();

}


void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    mSequenceElements.PopulateRowInformation();
    int height = DEFAULT_ROW_HEADING_HEIGHT * mSequenceElements.GetRowInformationSize();
    //Effects Grid Height

    mainSequencer->PanelEffectGrid->SetSize(wxSize(1200,height));
    mainSequencer->PanelEffectGrid->SetMinSize(wxSize(1200,height));
    mainSequencer->PanelEffectGrid->SetMaxSize(wxSize(1200,height));
    mainSequencer->PanelEffectGrid->Refresh();


    // Row heading Height
    mainSequencer->PanelRowHeadings->SetCanvasSize(175,height);
    mainSequencer->PanelRowHeadings->SetSize(wxSize(175,height));
    mainSequencer->PanelRowHeadings->SetMinSize(wxSize(175,height));
    mainSequencer->PanelRowHeadings->Refresh();
    m_mgr->Update();
}

void xLightsFrame::WindowResized( wxCommandEvent& event)
{
    ResizeAndMakeEffectsScroll();
    ResizeMainSequencer();
}

void xLightsFrame::ResizeAndMakeEffectsScroll()
{
    wxSize s;
    effectsPnl->EffectsNotebook->SetSize(effectsPnl->GetSize());
    effectsPnl->EffectsNotebook->SetMinSize(effectsPnl->GetSize());
    effectsPnl->EffectsNotebook->SetMaxSize(effectsPnl->GetSize());

    int w = effectsPnl->GetSize().GetWidth();
    int h = effectsPnl->GetSize().GetHeight();
    if(w>10 && h>50)
    {
        s = wxSize(w-10,h-30);
    }
    else
    {
        s = wxSize(200,200);
    }

    EffectsPanel1->SetSize(s);
    EffectsPanel1->SetMinSize(s);
    EffectsPanel1->SetMaxSize(s);

    EffectsPanel2->SetSize(s);
    EffectsPanel2->SetMinSize(s);
    EffectsPanel2->SetMaxSize(s);

    EffectsPanel1->Choicebook1->SetSize(s);
    EffectsPanel1->Choicebook1->SetMinSize(s);
    EffectsPanel1->Choicebook1->SetMaxSize(s);

    EffectsPanel2->Choicebook1->SetSize(s);
    EffectsPanel2->Choicebook1->SetMinSize(s);
    EffectsPanel2->Choicebook1->SetMaxSize(s);

    effectsPnl->EffectsNotebook->Refresh();
    EffectsPanel1->Refresh();
    EffectsPanel2->Refresh();
    EffectsPanel1->Choicebook1->Refresh();
    EffectsPanel2->Choicebook1->Refresh();

    // Make effects window scroll by updating its container size
    wxScrolledWindow* sw = (wxScrolledWindow*)EffectsPanel1->Choicebook1->GetPage(EffectsPanel1->Choicebook1->GetSelection());
    sw->FitInside();
    sw->SetScrollRate(5, 5);
    sw->Refresh();

    sw = (wxScrolledWindow*)EffectsPanel2->Choicebook1->GetPage(EffectsPanel2->Choicebook1->GetSelection());
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

    mainSequencer->PanelWaveForm->Refresh();
    mainSequencer->PanelTimeLine->Refresh();
    mainSequencer->PanelRowHeadings->Refresh();
    mainSequencer->PanelEffectGrid->Refresh();
    mainSequencer->panelEffectScrollBarSpacer->Refresh();
    mainSequencer->ScrollBarEffectGridHorz->Refresh();
    mainSequencer->ScrolledEffectsGrid->Refresh();
}

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}

void xLightsFrame::SelectedEffectChanged( wxCommandEvent& event)
{
    Element* element = (Element*)event.GetClientData();
    if(element->GetType() == "model")
    {
        int effectIndex = event.GetInt();
        Effect* effect = element->GetElementEffects()->GetEffect(effectIndex);
        //SetEffectControls(effect->Effect,element->GetName());
    }
}



void xLightsFrame::SetEffectControls(wxString settings, const wxString& model_name)
{
    long TempLong;
    wxColour color;
    wxWindow *CtrlWin, *ContextWin;
    wxString before,after,name,value;
    EffectsPanel *efPanel;
    int cnt=0;

//NOTE: the settings loop after this section does not initialize controls.
//For controls that have been added recently, an older version of the XML file will cause initial settings to be incorrect.
//A loop needs to be added to initialize the wx controls to a predictable value.
//For now, a few recently added controls are explicitly initialized here:
//(not sure if there will be side effects to using a full loop) -DJ
//~    CheckBox_LayerMorph->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter1->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter2->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter3->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->CheckBox_TextToCenter4->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel2->CheckBox_TextToCenter1->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel2->CheckBox_TextToCenter2->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel2->CheckBox_TextToCenter3->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel2->CheckBox_TextToCenter4->SetValue(false); //reset in case not present in settings -DJ
    EffectsPanel1->SingleStrandEffectType->SetSelection(0); //Set to first page in case not present


    while (!settings.IsEmpty())
    {
//NOTE: this doesn't handle "," embedded into Text lines (causes "unable to find" error): -DJ
        before=settings.BeforeFirst(',');
        after=settings.AfterFirst(',');
        switch (cnt)
        {
        case 0:
            SetChoicebook(EffectsPanel1->Choicebook1,before);
            break;
        case 1:
            SetChoicebook(EffectsPanel2->Choicebook1,before);
            break;
        case 2:
//~            Choice_LayerMethod->SetStringSelection(before);
            break;
        default:
            name=before.BeforeFirst('=');
            if (name.StartsWith("E1_"))
            {
                ContextWin=EffectsPanel1;
                name="ID_"+name.Mid(3);
                efPanel = EffectsPanel1;
            }
            else if (name.StartsWith("E2_"))
            {
                ContextWin=EffectsPanel2;
                name="ID_"+name.Mid(3);
                efPanel = EffectsPanel2;
            }
            else
            {
                efPanel = NULL;
                ContextWin=SeqPanelLeft;
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
//~                        efPanel->SetButtonColor((wxButton*)CtrlWin, &color);
                    }
                    else
                    {
                        CtrlWin->SetBackgroundColour(color);
                    }
                    //CtrlWin->SetBackgroundColour(color);
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
#if 0 //obsolete
                else if (name.StartsWith("ID_CHECKLISTBOX")) //for Pgo Coro Face element list
                {
                    wxCheckListBox* ctrl = (wxCheckListBox*)CtrlWin;
//                    ctrl->Clear();
//                    if (!model_name.empty())
                    if (model_name != prev_model) //load face elements from current model
                        load_face_elements(model_name, ctrl);
                    debug(10, "set %s from value '%s', model = '%s'", (const char*)name, (const char*)value.c_str(), (const char*)buffer.name.c_str());
                    wxStringTokenizer wtkz(value, "+");
                    while (wtkz.HasMoreTokens())
                    {
                        wxString nextkey = wtkz.GetNextToken();
                        if (nextkey.empty()) break; //continue;
//                        long keyval; //= wxAtoi(nextinx);
//                        if (nextkey.ToLong(&keyval)) continue;
                        debug(10, "on[%s]", (const char*)nextkey.c_str());
                        if (model_name.empty()) //presets effects tree?
                            if (model_name != prev_model)
                            {
                                ctrl->Append(nextkey); //just use value as-is; other values will be filled in when it's copied into grid
                                ctrl->Check(ctrl->GetCount()); //kludge: turn them all on to preserve them
                                continue;
                            }
                        nextkey = nextkey.BeforeFirst(':'); //strip off the part that can change between models
//                        ctrl->Check(key); //wrong!
                        for (int i = 0; i < ctrl->GetCount(); ++i)
                        {
                            debug(10, "vs. val str[%d/%d] = '%s', key '%s'", i, ctrl->GetCount(), (const char*)ctrl->GetString(i).c_str(), (const char*)ctrl->GetString(i).BeforeFirst(':').c_str());
                            if (ctrl->GetString(i).BeforeFirst(':') == nextkey)
                            {
                                ctrl->Check(i); //match by key (doesn't change), not by index
                                break;
                            }
                        }
                    }
                    prev_model = model_name; //remember which model is cached
                }
#endif // 0
                else
                {
                    wxMessageBox("Unknown type: "+name, "Internal Error");
                }
            }
            else
            {
                wxMessageBox("Unable to find: "+name, "Internal Error");
            }
            break;
        }
        settings=after;
        cnt++;
    }
    // set textbox values for sliders that have them
    wxScrollEvent evt;
    OnSlider_BrightnessCmdScroll(evt);
    OnSlider_ContrastCmdScroll(evt);
    OnSlider_EffectLayerMixCmdScroll(evt);

    OnSlider_SparkleFrequencyCmdScroll(evt);
//    OnSlider_Model_BrightnessCmdScroll(evt);
//   OnSlider_SparkleSliderCmdScroll(evt);

    EffectsPanel1->UpdateSpeedText();
    EffectsPanel2->UpdateSpeedText();

    MixTypeChanged=true;
    FadesChanged=true;
    EffectsPanel1->PaletteChanged=true;
    EffectsPanel2->PaletteChanged=true;
    ResetEffectStates(playResetEffectState);
}
