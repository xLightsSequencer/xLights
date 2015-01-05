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

        sPreview2 = new SequencePreview(PanelSequencer,args);
        sPreview2->SetSize(wxSize(200,200));
        sPreview2->InitializePreview();
        m_mgr->AddPane(sPreview2, wxLEFT, wxT("Model Preview 2"));

        effectsPnl = new TopEffectsPanel(PanelSequencer);
        EffectsPanel1 = new EffectsPanel(effectsPnl->EffectsNotebook, ID_PANEL_EFFECTS1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1"));
        EffectsPanel2 = new EffectsPanel(effectsPnl->EffectsNotebook, ID_PANEL_EFFECTS2, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS2"));

        effectsPnl->EffectsNotebook->AddPage(EffectsPanel1, _("Effect1"), false);
        effectsPnl->EffectsNotebook->AddPage(EffectsPanel2, _("Effect2"), false);
        effectsPnl->SetBackgroundColour(*wxGREEN);
        effectsPnl->Refresh();
        effectsPnl->Update();


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

        m_mgr->AddPane(effectsPnl,wxAuiPaneInfo().Name(wxT("Effects")).Caption(wxT("Effects")).
                       BestSize(wxSize(175,175)).MinSize(wxSize(200,100)).Left());

        m_mgr->AddPane(mainSequencer,wxAuiPaneInfo().Name(_T("Main Sequencer")).CenterPane().Caption(_("Main Sequencer")));
        m_mgr->Update();

        sPreview1->Refresh();
        sPreview2->Refresh();
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

void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}
