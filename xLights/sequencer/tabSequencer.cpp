#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"
#include "SequenceElements.h"


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
        m_mgr.SetManagedWindow(PanelSequencer);

        int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

        mainSequencer = new MainSequencer(PanelSequencer);

        mainSequencer->SetDoubleBuffered(true);
        mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
        mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));
        fgsSequencer->Add(mainSequencer, 1, wxALL|wxALIGN_LEFT, 2);

        rowHeading = new RowHeading(mainSequencer->PanelRowHeadings);
        rowHeading->SetSequenceElements(&mSequenceElements);
        rowHeading->SetCanvasSize(175,2200);

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

        wave = new Waveform(mainSequencer->PanelWaveForm,PanelSequencer,args);
        mMediaLengthMS = wave->OpenfileMediaFile("c:\\temp\\4.mp3");
        wave->SetCanvasSize(1200,75);

        timeLine = new TimeLine(mainSequencer->PanelTimeLine);
        timeLine->SetTimeLength(mMediaLengthMS);
        timeLine->SetCanvasSize(1200,25);


        effectsGrid = new EffectsGrid(mainSequencer->PanelEffectGrid,args);
        effectsGrid->SetCanvasSize(1200,2200);
        effectsGrid->SetSequenceElements(&mSequenceElements);
        effectsGrid->SetTimeline(timeLine);
        effectsGrid->InitializeGrid();

        sPreview1 = new SequencePreview(PanelSequencer,args);
        sPreview1->SetSize(wxSize(200,200));
        sPreview1->InitializePreview();
        m_mgr.AddPane(sPreview1, wxLEFT, wxT("Model Preview"));

        sPreview2 = new SequencePreview(PanelSequencer,args);
        sPreview2->SetSize(wxSize(200,200));
        sPreview2->InitializePreview();
        m_mgr.AddPane(sPreview2, wxLEFT, wxT("Model Preview"));

        m_mgr.AddPane(mainSequencer,wxCENTER, wxT("Main Sequencer"));
        m_mgr.GetPane("Main Sequencer");


        effectsPnl = new wxPanel(PanelSequencer, ID_PANEL_EFFECTS, wxPoint(40,-11), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS"));
        FlexGridEffects = new wxFlexGridSizer(0, 1, 0, 0);
        FlexGridEffects->AddGrowableCol(0);

        effectsNotebook = new wxNotebook(effectsPnl, ID_NOTEBOOK_EFFECTS, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_EFFECTS"));
        EffectsPanel1 = new EffectsPanel(effectsNotebook, ID_PANEL_EFFECTS1, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS1"));
        EffectsPanel2 = new EffectsPanel(effectsNotebook, ID_PANEL_EFFECTS2, wxPoint(0,0), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_EFFECTS2"));

        effectsNotebook->AddPage(EffectsPanel1, _("Effect1"), false);
        effectsNotebook->AddPage(EffectsPanel2, _("Effect2"), false);

        FlexGridEffects->Add(effectsNotebook, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
        effectsPnl->SetSizer(FlexGridEffects);

        m_mgr.AddPane(effectsPnl, wxLEFT, wxT("Effects"));

        m_mgr.Update();
        sPreview1->Refresh();
        sPreview2->Refresh();
}

void xLightsFrame::Zoom( wxCommandEvent& event)
{
    if(event.GetInt() == ZOOM_IN)
    {
        timeLine->ZoomIn();
    }
    else
    {
        timeLine->ZoomOut();
    }
}

void xLightsFrame::HorizontalScrollChanged( wxCommandEvent& event)
{
}

void xLightsFrame::TimeSelected( wxCommandEvent& event)
{
    // event.GetInt holds position without first pixelOffset
    timeLine->TimeSelected(event.GetInt());
}



void xLightsFrame::TimelineChanged( wxCommandEvent& event)
{
    TimelineChangeArguments *tla = (TimelineChangeArguments*)(event.GetClientData());
    wave->SetZoomLevel(tla->ZoomLevel);
    wave->SetStartPixelOffset(tla->StartPixelOffset);
    wave->PositionSelected(tla->SelectedPosition);
    wave->Refresh();
    mainSequencer->ScrollBarEffectGridHorz->SetRange(((MAX_ZOOM_OUT_INDEX - tla->ZoomLevel)*5)+1);
    mainSequencer->ScrollBarEffectGridHorz->Refresh();

    effectsGrid->SetStartPixelOffset(tla->StartPixelOffset);
    effectsGrid->Refresh();

}


void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    mSequenceElements.PopulateRowInformation();
    int height = DEFAULT_ROW_HEADING_HEIGHT * mSequenceElements.GetRowInformationSize();
    //Effects Grid Height
    effectsGrid->SetCanvasSize(1200,height);
    effectsGrid->Refresh();

    mainSequencer->PanelEffectGrid->SetSize(wxSize(1200,height));
    mainSequencer->PanelEffectGrid->SetMinSize(wxSize(1200,height));
    mainSequencer->PanelEffectGrid->SetMaxSize(wxSize(1200,height));


    // Row heading Height
    rowHeading->SetCanvasSize(175,height);
    rowHeading->Refresh();
    mainSequencer->PanelRowHeadings->SetSize(wxSize(175,height));
    mainSequencer->PanelRowHeadings->SetMinSize(wxSize(175,height));
    m_mgr.Update();
}


void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}
