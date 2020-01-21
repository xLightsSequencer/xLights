#ifndef OUTPUTSETTINGSPANEL_H
#define OUTPUTSETTINGSPANEL_H

//(*Headers(OutputSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class OutputSettingsPanel: public wxPanel
{
	public:

		OutputSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OutputSettingsPanel();

		//(*Declarations(OutputSettingsPanel)
		wxCheckBox* FrameSyncCheckBox;
		wxChoice* DuplicateSuppressChoice;
		wxChoice* ForceLocalIPChoice;
		wxChoice* xFadexScheduleChoice;
		wxStaticText* StaticText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(OutputSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_CHOICE3;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(OutputSettingsPanel)
		void OnFrameSyncCheckBoxClick(wxCommandEvent& event);
		void OnForceLocalIPChoiceSelect(wxCommandEvent& event);
		void OnDuplicateSuppressChoiceSelect(wxCommandEvent& event);
		void OnxFadexScheduleChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
