#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolbar.h>
#include "RowHeading.h"
#include "EffectsGrid.h"
#include "Waveform.h"
#include "../KeyBindings.h"

#if __has_include("osxUtils/TouchBars.h")
#include "osxUtils/TouchBars.h"
#endif

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

		MainSequencer(wxWindow* parent, bool smallWaveform, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~MainSequencer();
		void SetSequenceElements(SequenceElements* elements);
        void UpdateEffectGridVerticalScrollBar();
        void UpdateTimeDisplay(int time_ms, float fps);
        void UpdateSelectedDisplay(int selected);
        bool GetSelectedEffectsData(wxString& copy_data);
        bool GetACEffectsData(wxString& copy_data);
        void GetPresetData(wxString& copy_data);
        bool CopySelectedEffects();
        bool PasteByCellActive() const { return mPasteByCell; }
        void SetPasteByCell(bool state) { mPasteByCell = state; }
        void HorizontalScrollChanged( wxCommandEvent& event);
        void TagAllSelectedEffects();
        void UnTagAllEffects();
        Effect* GetSelectedEffect();
        void CancelRender();
        void ToggleRender(bool off);
        int GetSelectedEffectCount(const std::string effectName) const;
        bool AreAllSelectedEffectsOnTheSameElement() const;
        void ApplyEffectSettingToSelected(const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid);
        std::list<std::string> GetUniqueEffectPropertyValues(const std::string& id);
        void ApplyButtonPressToSelected(const std::string& effectName, const std::string id);
        void RemapSelectedDMXEffectValues(const std::vector<std::tuple<int, int, float, int>>& dmxmappings);
        void ConvertSelectedEffectsTo(const std::string& effectName);
        Effect* SelectEffectUsingDescription(std::string description);
        Effect* SelectEffectUsingElementLayerTime(std::string element, int layer, int time);
        std::list<std::string> GetAllEffectDescriptions();
        std::list<std::string> GetAllElementNamesWithEffects();
        int GetElementLayerCount(std::string elementName, std::list<int>* layers = nullptr);
        std::list<Effect*> GetElementLayerEffects(std::string elementName, int layer);
        void SetChanged();
        void UnselectAllEffects();
        void SelectEffect(Effect* ef);
        bool HandleSequencerKeyBinding(wxKeyEvent& event);
        void ScrollToRow(int row);
        void UpdateEffectGridHorizontalScrollBar();
        void SetEffectDuration(const std::string& effectType, const uint32_t durationMS);

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

    //(*Declarations(MainSequencer)
    EffectsGrid* PanelEffectGrid;
    RowHeading* PanelRowHeadings;
    TimeLine* PanelTimeLine;
    Waveform* PanelWaveForm;
    wxCheckBox* CheckBox_SuspendRender;
    wxChoice* ViewChoice;
    wxScrollBar* ScrollBarEffectsHorizontal;
    wxScrollBar* ScrollBarEffectsVertical;
    //*)

        KeyBindingMap keyBindings;
        TimeDisplayControl *timeDisplay;

#ifdef __XLIGHTS_HAS_TOUCHBARS__
        void SetupTouchBar(EffectManager &m, ColorPanelTouchBar *colorTouchBar);
        xlTouchBarSupport touchBarSupport;
        std::unique_ptr<EffectGridTouchBar> effectGridTouchbar;
#endif
	protected:

		//(*Identifiers(MainSequencer)
		static const long ID_CHOICE_VIEW_CHOICE;
		static const long ID_PANEL1;
		static const long ID_PANEL3;
		static const long ID_PANEL6;
		static const long ID_PANEL2;
		static const long ID_SCROLLBAR_EFFECTS_VERTICAL;
		static const long ID_CHECKBOX1;
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
		void OnCheckBox_SuspendRenderClick(wxCommandEvent& event);
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

        void SavePosition();
        void RestorePosition();

        wxWindow *mParent;
        SequenceElements* mSequenceElements;
        bool mCanUndo;
        bool mPasteByCell;
        std::string _savedTopModel;
};
