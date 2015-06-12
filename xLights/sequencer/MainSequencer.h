#ifndef MAINSEQUENCER_H
#define MAINSEQUENCER_H

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include "RowHeading.h"
#include "EffectsGrid.h"
#include "Waveform.h"
#include "../KeyBindings.h"

wxDECLARE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);

class MainSequencer: public wxPanel
{
	public:

		MainSequencer(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MainSequencer();
		void SetSequenceElements(SequenceElements* elements);
        void UpdateEffectGridVerticalScrollBar();
        void UpdateTimeDisplay(int time_ms);
        void SetPlayStatus(int play_type);
        void GetSelectedEffectsData(wxString& copy_data);
        void CopySelectedEffects();
        void Paste();
        void DeleteAllSelectedEffects();

		//(*Declarations(MainSequencer)
		wxScrollBar* ScrollBarEffectsHorizontal;
		wxScrollBar* ScrollBarEffectsVertical;
		EffectsGrid* PanelEffectGrid;
		wxStaticText* StaticText_SeqTime;
		RowHeading* PanelRowHeadings;
		Waveform* PanelWaveForm;
		TimeLine* PanelTimeLine;
		//*)

        KeyBindingMap keyBindings;

	protected:

		//(*Identifiers(MainSequencer)
		static const long ID_STATICTEXT_Time;
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
		void OnChar(wxKeyEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		void OnCharHook(wxKeyEvent& event);
		void OnScrollBarEffectsHorizontalScrollLineUp(wxScrollEvent& event);
		void OnScrollBarEffectsHorizontalScrollLineDown(wxScrollEvent& event);
		//*)
		DECLARE_EVENT_TABLE()

        void mouseWheelMoved(wxMouseEvent& event);
        void TimeLineChanged( wxCommandEvent& event);
        void InsertTimingMarkFromRange();
        void SplitTimingMark();
        void SetHandlers(wxWindow *);

        wxWindow *mParent;
        SequenceElements* mSequenceElements;
        int mPlayType;
        bool mCanUndo;
};

#endif
