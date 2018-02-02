#ifndef LORCONTROLLERDIALOG_H
#define LORCONTROLLERDIALOG_H

//(*Headers(LorControllerDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class LorController;

class LorControllerDialog: public wxDialog
{
	public:

		LorControllerDialog(wxWindow* parent, LorController* controller, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LorControllerDialog();

        LorController* GetController() { return _controller; }

        void SetNextUnitId(int id) { _next_id = id; }

		//(*Declarations(LorControllerDialog)
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxSpinCtrl* SpinCtrl_Channels;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxSpinCtrl* SpinCtrl_UnitID;
		wxChoice* Choice_Mode;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText_Hex;
		wxButton* Button_Cancel;
		wxChoice* Choice_Type;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(LorControllerDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_TYPE;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL_CHANNELS;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL_UNITID;
		static const long ID_STATICTEXT_HEX;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE_MODE;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL_DESCRIPTION;
		static const long ID_BUTTON_OK;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:
        LorController* _controller;
        int _next_id;

        void ValidateWindow();

		//(*Handlers(LorControllerDialog)
		void OnChoice_TypeSelect(wxCommandEvent& event);
		void OnTextCtrl_ChannelsText(wxCommandEvent& event);
		void OnSpinCtrl_UnitIDChange(wxSpinEvent& event);
		void OnChoice_ModeSelect(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSpinCtrl_ChannelsChange(wxSpinEvent& event);
		void OnTextCtrl_DescriptionText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
