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
#include "../osx_utils/TouchBars.h"

wxDECLARE_EVENT(EVT_HORIZ_SCROLL, wxCommandEvent);
wxDECLARE_EVENT(EVT_WINDOW_RESIZED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TOGGLE_PLAY, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_LINE_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SEQUENCE_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCROLL_RIGHT, wxCommandEvent);

class TimeDisplayControl;

class MainSequencer: public wxPanel
{
	public:

		MainSequencer(wxWindow* parent, bool smallWaveform, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MainSequencer();
		void SetSequenceElements(SequenceElements* elements);
        void UpdateEffectGridVerticalScrollBar();
        void UpdateTimeDisplay(int time_ms, float fps);
        void UpdateSelectedDisplay(int selected);
        int GetPlayStatus() const { return mPlayType; }
        void SetPlayStatus(int play_type);
        void GetSelectedEffectsData(wxString& copy_data);
        void GetACEffectsData(wxString& copy_data);
        void GetPresetData(wxString& copy_data);
        bool CopySelectedEffects();
        bool PasteByCellActive() const { return mPasteByCell; }
        void SetPasteByCell(bool state) { mPasteByCell = state; }
        void HorizontalScrollChanged( wxCommandEvent& event);
        void TagAllSelectedEffects();
        void UnTagAllEffects();
        Effect* GetSelectedEffect();
        void CancelRender();
        int GetSelectedEffectCount(const std::string effectName) const;
        bool AreAllSelectedEffectsOnTheSameElement() const;
        void ApplyEffectSettingToSelected(const std::string effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid);
        void SelectEffectUsingDescription(std::string description);
        void SelectEffectUsingElementLayerTime(std::string element, int layer, int time);
        std::list<std::string> GetAllEffectDescriptions();
        std::list<std::string> GetAllElementNamesWithEffects();
        int GetElementLayerCount(std::string elementName, std::list<int>* layers = nullptr);
        std::list<Effect*> GetElementLayerEffects(std::string elementName, int layer);
        void SetChanged();
        void UnselectAllEffects();
        bool HandleSequencerKeyBinding(wxKeyEvent& event);

        void Cut();
        void Copy();
        void Paste(bool row_paste = false);

        void DoCopy(wxCommandEvent& event);
        void DoCut(wxCommandEvent& event);
        void DoPaste(wxCommandEvent& event);
        void DoUndo(wxCommandEvent& event);
        void DoRedo(wxCommandEvent& event);

        void SetLargeWaveform();
        void SetSmallWaveform();

        void TouchButtonEvent(wxCommandEvent &event);
        void ToggleHousePreview();
        void ToggleModelPreview();
        void TouchPlayControl(const std::string &event);
        void SetupTouchBar(EffectManager &m, ColorPanelTouchBar *colorTouchBar);

		//(*Declarations(MainSequencer)
		EffectsGrid* PanelEffectGrid;
		RowHeading* PanelRowHeadings;
		TimeLine* PanelTimeLine;
		Waveform* PanelWaveForm;
		wxChoice* ViewChoice;
		wxScrollBar* ScrollBarEffectsHorizontal;
		wxScrollBar* ScrollBarEffectsVertical;
		//*)

        KeyBindingMap keyBindings;
        TimeDisplayControl *timeDisplay;
        xlTouchBarSupport touchBarSupport;
        EffectGridTouchBar *effectGridTouchbar;
	protected:

		//(*Identifiers(MainSequencer)
		static const long ID_CHOICE_VIEW_CHOICE;
		static const long ID_PANEL1;
		static const long ID_PANEL3;
		static const long ID_PANEL6;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR_EFFECTS_VERTICAL;
		static const long ID_SCROLLBAR_EFFECT_GRID_HORZ;
		//*)

	private:

		//(*Handlers(MainSequencer)
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
        void TimeLineSelectionChanged(wxCommandEvent& event);
        void InsertTimingMarkFromRange();
        void SplitTimingMark();
        void SetHandlers(wxWindow *);

        void ScrollRight( wxCommandEvent& event);
        void TimelineChanged( wxCommandEvent& event);
        void SequenceChanged( wxCommandEvent& event);
        void UpdateEffectGridHorizontalScrollBar();

        void SavePosition();
        void RestorePosition();
        void ScrollToRow(int row);

        wxWindow *mParent;
        SequenceElements* mSequenceElements;
        int mPlayType;
        bool mCanUndo;
        bool mPasteByCell;
        std::string _savedTopModel;
};

#endif
