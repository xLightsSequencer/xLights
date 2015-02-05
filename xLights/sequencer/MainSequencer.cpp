#include "MainSequencer.h"
#include "TimeLine.h"
#include <wx/event.h>

//(*InternalHeaders(MainSequencer)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MainSequencer)
const long MainSequencer::ID_PANEL5 = wxNewId();
const long MainSequencer::ID_PANEL1 = wxNewId();
const long MainSequencer::ID_PANEL3 = wxNewId();
const long MainSequencer::ID_PANEL6 = wxNewId();
const long MainSequencer::ID_PANEL2 = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECTS_VERTICAL = wxNewId();
const long MainSequencer::ID_SCROLLBAR_EFFECT_GRID_HORZ = wxNewId();
//*)

BEGIN_EVENT_TABLE(MainSequencer,wxPanel)
    EVT_MOUSEWHEEL(MainSequencer::mouseWheelMoved)

END_EVENT_TABLE()


MainSequencer::MainSequencer(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(MainSequencer)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(1);
	PanelPlayControls = new wxPanel(this, ID_PANEL5, wxDefaultPosition, wxSize(175,100), wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer1->Add(PanelPlayControls, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0);
	FlexGridSizer4 = new wxFlexGridSizer(2, 0, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	PanelTimeLine = new TimeLine(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	PanelTimeLine->SetMinSize(wxSize(-1,25));
	PanelTimeLine->SetMaxSize(wxSize(-1,25));
	FlexGridSizer4->Add(PanelTimeLine, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	PanelWaveForm = new Waveform(this, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	PanelWaveForm->SetMinSize(wxSize(-1,75));
	PanelWaveForm->SetMaxSize(wxSize(-1,75));
	FlexGridSizer4->Add(PanelWaveForm, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PanelRowHeadings = new RowHeading(this, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer1->Add(PanelRowHeadings, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
	PanelEffectGrid = new EffectsGrid(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("ID_PANEL2"));
	FlexGridSizer1->Add(PanelEffectGrid, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	ScrollBarEffectsVertical = new wxScrollBar(this, ID_SCROLLBAR_EFFECTS_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECTS_VERTICAL"));
	ScrollBarEffectsVertical->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer1->Add(ScrollBarEffectsVertical, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ScrollBarEffectsHorizontal = new wxScrollBar(this, ID_SCROLLBAR_EFFECT_GRID_HORZ, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_SCROLLBAR_EFFECT_GRID_HORZ"));
	ScrollBarEffectsHorizontal->SetScrollbar(0, 1, 100, 1);
	FlexGridSizer1->Add(ScrollBarEffectsHorizontal, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECTS_VERTICAL,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectsVerticalScrollChanged);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_TOP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_BOTTOM,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEUP,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_PAGEDOWN,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	Connect(ID_SCROLLBAR_EFFECT_GRID_HORZ,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&MainSequencer::OnScrollBarEffectGridHorzScroll);
	//*)

    mParent = parent;
}

MainSequencer::~MainSequencer()
{
	//(*Destroy(MainSequencer)
	//*)
}

void MainSequencer::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void MainSequencer::UpdateEffectGridVerticalScrollBar()
{
    int position = mSequenceElements->GetFirstVisibleModelRow();
    int range = mSequenceElements->GetTotalNumberOfModelRows();
    int pageSize = mSequenceElements->GetMaxModelsDisplayed();
    int thumbSize = mSequenceElements->GetMaxModelsDisplayed();
    ScrollBarEffectsVertical->SetScrollbar(position,thumbSize,range,pageSize);
    ScrollBarEffectsVertical->Refresh();
}


void MainSequencer::OnPanelWaveFormPaint(wxPaintEvent& event)
{
}


void MainSequencer::OnScrollBarEffectGridHorzScroll(wxScrollEvent& event)
{
    wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
    wxPostEvent(mParent, eventScroll);
}

void MainSequencer::OnScrollBarEffectsVerticalScrollChanged(wxScrollEvent& event)
{
    int position = ScrollBarEffectsVertical->GetThumbPosition();
    mSequenceElements->SetFirstVisibleModelRow(position);
    Refresh();
}

void MainSequencer::mouseWheelMoved(wxMouseEvent& event)
{
    int i = event.GetWheelRotation();
    if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
        int position = ScrollBarEffectsHorizontal->GetThumbPosition();
        int ts = ScrollBarEffectsHorizontal->GetThumbSize() / 10;
        if (ts ==0) {
            ts = 1;
        }
        if (i < 0) {
            if (position > 0) {
                position -= ts;
            }
        } else {
            position += ts;
            if (position >= ScrollBarEffectsHorizontal->GetRange()) {
                position = ScrollBarEffectsHorizontal->GetRange() - 1;
            }
        }
        ScrollBarEffectsHorizontal->SetThumbPosition(position);
        wxCommandEvent eventScroll(EVT_HORIZ_SCROLL);
        wxPostEvent(mParent, eventScroll);
    } else {
        int position = ScrollBarEffectsVertical->GetThumbPosition();
        if(i<0)
        {
            if(position < ScrollBarEffectsVertical->GetRange()-1)
            {
                position++;
                ScrollBarEffectsVertical->SetThumbPosition(position);
                mSequenceElements->SetFirstVisibleModelRow(position);
            }
        }
        else
        {
            if(position > 0)
            {
                position--;
                ScrollBarEffectsVertical->SetThumbPosition(position);
                mSequenceElements->SetFirstVisibleModelRow(position);
            }
        }
        mSequenceElements->PopulateVisibleRowInformation();
        PanelEffectGrid->Refresh();
        PanelRowHeadings->Refresh();
    }
}
