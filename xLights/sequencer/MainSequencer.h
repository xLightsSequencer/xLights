#ifndef MAINSEQUENCER_H
#define MAINSEQUENCER_H

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include "RowHeading.h"
#include "EffectsGrid.h"
#include "Waveform.h"


wxDECLARE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);

class MainSequencer: public wxPanel
{
	public:

		MainSequencer(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MainSequencer();
		void SetSequenceElements(SequenceElements* elements);
        void UpdateEffectGridVerticalScrollBar();


		//(*Declarations(MainSequencer)
		wxScrollBar* ScrollBarEffectsHorizontal;
		wxScrollBar* ScrollBarEffectsVertical;
		wxStaticText* StaticText1;
		EffectsGrid* PanelEffectGrid;
		RowHeading* PanelRowHeadings;
		wxPanel* PanelPlayControls;
		Waveform* PanelWaveForm;
		TimeLine* PanelTimeLine;
		//*)

	protected:

		//(*Identifiers(MainSequencer)
		static const long ID_STATICTEXT1;
		static const long ID_PANEL5;
		static const long ID_PANEL1;
		static const long ID_PANEL3;
		static const long ID_PANEL6;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR_EFFECTS_VERTICAL;
		static const long ID_SCROLLBAR_EFFECT_GRID_HORZ;
		//*)

	private:

		//(*Handlers(MainSequencer)
		void OnPanelWaveFormPaint(wxPaintEvent& event);
		void OnScrollBarEffectGridHorzScroll(wxScrollEvent& event);
		void OnScrollBarEffectsVerticalScrollChanged(wxScrollEvent& event);
		//*)
		DECLARE_EVENT_TABLE()

        void mouseWheelMoved(wxMouseEvent& event);
        void TimeLineChanged( wxCommandEvent& event);

        wxWindow *mParent;
        SequenceElements* mSequenceElements;
};

#endif
