#include <wx/utils.h> //check keyboard state -DJ
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include "../xLightsMain.h"
#include "../SequencePreview.h"


/************************************* New Sequencer Code*****************************************/
void xLightsFrame::InitSequencer()
{
        if(mSequencerInitialize)
        {
            return;
        }
        bool success = LoadSequencerFile("e:\\v4.xml");
        mSequencerInitialize = true;
        m_mgr.SetManagedWindow(PanelSequencer);

        int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

        mainSequencer = new MainSequencer(PanelSequencer);

        mainSequencer->SetDoubleBuffered(true);
        mainSequencer->PanelPlayControls->SetSize(wxSize(175,100));
        mainSequencer->PanelPlayControls->SetMinSize(wxSize(175,100));
        fgsSequencer->Add(mainSequencer, 1, wxALL|wxALIGN_LEFT, 2);

        rowHeading = new RowHeading(mainSequencer->PanelRowHeadings);

        rowHeading->SetElements(mDisplayElements);
        rowHeading->SetCanvasSize(175,2200);
//        mainSequencer->PanelRowHeadings->SetSize(wxSize(250,900));
//        mainSequencer->PanelRowHeadings->SetMinSize(wxSize(250,900));
//        mainSequencer->PanelRowHeadings->SetBackgroundColour(*wxGREEN);

        mainSequencer->PanelEffectGrid->SetSize(wxSize(1200,2200));
        mainSequencer->PanelEffectGrid->SetMinSize(wxSize(1200,2200));
        effectsGrid = new EffectsGrid(mainSequencer->PanelEffectGrid,args);
        effectsGrid->SetCanvasSize(1200,2200);
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
        mMediaLengthMS = wave->OpenfileMediaFile("e:\\4.mp3");
        wave->SetCanvasSize(1200,75);

        timeLine = new TimeLine(mainSequencer->PanelTimeLine);
        timeLine->SetTimeLength(mMediaLengthMS);
        timeLine->SetCanvasSize(1200,25);

        sPreview1 = new SequencePreview(mainSequencer,args);
        sPreview1->InitializePreview();
        sPreview1->SetSize(wxSize(200,200));
        m_mgr.AddPane(sPreview1, wxLEFT, wxT("Model Preview"));

        sPreview2 = new SequencePreview(mainSequencer,args);
        sPreview2->InitializePreview();
        sPreview2->SetSize(wxSize(200,200));
        m_mgr.AddPane(sPreview2, wxLEFT, wxT("Model Preview"));

        m_mgr.AddPane(mainSequencer,wxCENTER, wxT("Main Sequencer"));
        m_mgr.GetPane("Main Sequencer");

        m_mgr.Update();
        SequenceElements *sequenceElements = new SequenceElements();
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

    this->SetLabel(wxString::Format("Zoom=%d, StartOffset=%d",tla->ZoomLevel,tla->StartPixelOffset));
}


void xLightsFrame::OnPanelSequencerPaint(wxPaintEvent& event)
{
    mainSequencer->ScrollBarEffectGridHorz->Update();
}

bool xLightsFrame::LoadSequencerFile(wxString filename)
{
    wxString tmpStr;
    // read xml sequence info
    wxFileName FileObj(filename);
    FileObj.SetExt("xml");
    wxString SeqXmlFileName=FileObj.GetFullPath();
    int gridCol;
    if (!FileObj.FileExists())
    {
        //if (ChooseModels) ChooseModelsForSequence();
        return false;
    }
    // read xml
    //  first fix any version specific changes
    //FixVersionDifferences(SeqXmlFileName);
    if (!mSequenceDocument.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return false;
    }
    wxXmlNode* root=mSequenceDocument.GetRoot();
    wxString tempstr=root->GetAttribute("BaseChannel", "1");
    tempstr.ToLong(&SeqBaseChannel);
    tempstr=root->GetAttribute("ChanCtrlBasic", "0");
    SeqChanCtrlBasic=tempstr!="0";
    tempstr=root->GetAttribute("ChanCtrlColor", "0");
    SeqChanCtrlColor=tempstr!="0";

    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "DisplayElements")
        {
            mDisplayElements=e;
            wxXmlNode* j=mDisplayElements->GetChildren();
            if(j->GetName()=="Element")
            {
                int h=0;
            }
        }
       if (e->GetName() == "ElementEffects")
        {
            mElementEffects=e;
        }
    }
    if(mDisplayElements ==0)
    {
        int k=0;
    }

    if(mElementEffects==0)
    {
    }
    return true;
}
