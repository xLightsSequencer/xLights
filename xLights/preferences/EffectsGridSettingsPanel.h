#ifndef EFFECTSGRIDSETTINGSPANEL_H
#define EFFECTSGRIDSETTINGSPANEL_H

//(*Headers(EffectsGridSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class EffectsGridSettingsPanel: public wxPanel
{
	public:

		EffectsGridSettingsPanel(wxWindow* parent,xLightsFrame *f,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectsGridSettingsPanel();

		//(*Declarations(EffectsGridSettingsPanel)
		wxCheckBox* IconBackgroundsCheckBox;
		wxCheckBox* NodeValuesCheckBox;
		wxCheckBox* SmallWaveformCheckBox;
		wxCheckBox* SnapToTimingCheckBox;
		wxCheckBox* TimingDoubleClickCheckbox;
		wxCheckBox* TransistionMarksCheckBox;
		wxChoice* GridSpacingChoice;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(EffectsGridSettingsPanel)
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX6;
		//*)

	private:
        xLightsFrame *frame;


		//(*Handlers(EffectsGridSettingsPanel)
		void OnIconBackgroundsCheckBoxClick(wxCommandEvent& event);
		void OnNodeValuesCheckBoxClick(wxCommandEvent& event);
		void OnSnapToTimingCheckBoxClick(wxCommandEvent& event);
		void OnTimingDoubleClickChoiceSelect(wxCommandEvent& event);
		void OnSmallWaveformCheckBoxClick(wxCommandEvent& event);
		void OnGridSpacingChoiceSelect(wxCommandEvent& event);
		void OnTransistionMarksCheckBoxClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
