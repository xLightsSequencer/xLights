#ifndef MAINSEQUENCER_H
#define MAINSEQUENCER_H

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>

wxDECLARE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);

class MainSequencer: public wxPanel
{
	public:

		MainSequencer(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MainSequencer();

		//(*Declarations(MainSequencer)
		wxPanel* panelEffectScrollBarSpacer;
		wxPanel* PanelRowHeadings;
		wxScrolledWindow* ScrolledEffectsGrid;
		wxScrollBar* ScrollBarEffectGridHorz;
		wxPanel* PanelTimeLine;
		wxPanel* PanelPlayControls;
		wxPanel* PanelEffectGrid;
		wxPanel* PanelWaveForm;
		//*)

	protected:

		//(*Identifiers(MainSequencer)
		static const long ID_PANEL5;
		static const long ID_PANEL1;
		static const long ID_PANEL3;
		static const long ID_PANEL6;
		static const long ID_PANEL4;
		static const long ID_SCROLLEDWINDOW2;
		static const long ID_PANEL_EFFECT_SCROLLBAR_SPACER;
		static const long ID_SCROLLBAR_EFFECT_GRID_HORZ;
		//*)

	private:

		//(*Handlers(MainSequencer)
		void OnPanelWaveFormPaint(wxPaintEvent& event);
		void OnScrollBarEffectGridHorzScrollChanged(wxScrollEvent& event);
		void OnPaint(wxPaintEvent& event);
		//*)
		DECLARE_EVENT_TABLE()
        void TimeLineChanged( wxCommandEvent& event);
        wxWindow *mParent;
};

#endif
