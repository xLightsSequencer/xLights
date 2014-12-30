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
//        mainSequencer->PanelRowHeadings->SetSize(wxSize(250,900));
//        mainSequencer->PanelRowHeadings->SetMinSize(wxSize(250,900));
//        mainSequencer->PanelRowHeadings->SetBackgroundColour(*wxGREEN);

        mainSequencer->PanelEffectGrid->SetSize(wxSize(1200,2200));
        mainSequencer->PanelEffectGrid->SetMinSize(wxSize(1200,2200));
        effectsGrid = new EffectsGrid(mainSequencer->PanelEffectGrid,args);
        effectsGrid->SetCanvasSize(1200,2200);
        effectsGrid->SetSequenceElements(&mSequenceElements);
        effectsGrid->InitializeGrid();

        mainSequencer->ScrolledEffectsGrid->SetSize(wxSize(1525,800));
        mainSequencer->ScrolledEffectsGrid->SetMinSize(wxSize(1525,800));
        mainSequencer->ScrolledEffectsGrid->SetMaxSize(wxSize(1525,800));
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
    timeLine->PositionPercentageMoved(event.GetInt());
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

    this->SetLabel(wxString::Format("Zoom=%d, StartOffset=%d",tla->ZoomLevel,tla->StartPixelOffset));
}


void xLightsFrame::RowHeadingsChanged( wxCommandEvent& event)
{
    mSequenceElements.PopulateRowInformation();
    rowHeading->Refresh();
    effectsGrid->Refresh();
}


void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}
