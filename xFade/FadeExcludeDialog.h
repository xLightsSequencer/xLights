#ifndef FADEEXCLUDEDIALOG_H
#define FADEEXCLUDEDIALOG_H

//(*Headers(FadeExcludeDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

class Settings;

class FadeExcludeDialog: public wxDialog
{
    Settings* _settings;

	public:

		FadeExcludeDialog(wxWindow* parent, Settings* settings, std::string ch, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FadeExcludeDialog();
        std::string GetChannel() const;

		//(*Declarations(FadeExcludeDialog)
		wxButton* ButtonCancel;
		wxButton* Button_Ok;
		wxChoice* Choice_Universe;
		wxSpinCtrl* SpinCtrl_Channel;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(FadeExcludeDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(FadeExcludeDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButtonCancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
