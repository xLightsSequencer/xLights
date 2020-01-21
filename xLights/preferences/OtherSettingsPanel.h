#ifndef OTHERSETTINGSPANEL_H
#define OTHERSETTINGSPANEL_H

//(*Headers(OtherSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxGridBagSizer;
class wxStaticBoxSizer;
class wxStaticText;
class wxTextCtrl;
//*)

class xLightsFrame;
class OtherSettingsPanel: public wxPanel
{
	public:

		OtherSettingsPanel(wxWindow* parent,xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OtherSettingsPanel();

		//(*Declarations(OtherSettingsPanel)
		wxCheckBox* ExcludeAudioCheckBox;
		wxCheckBox* ExcludePresetsCheckBox;
		wxCheckBox* HardwareVideoDecodingCheckBox;
		wxTextCtrl* eMailTextControl;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(OtherSettingsPanel)
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(OtherSettingsPanel)
		void OnExcludeAudioCheckBoxClick(wxCommandEvent& event);
		void OnExcludePresetsCheckBoxClick(wxCommandEvent& event);
		void OnHardwareVideoDecodingCheckBoxClick(wxCommandEvent& event);
		void OneMailTextControlTextEnter(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
