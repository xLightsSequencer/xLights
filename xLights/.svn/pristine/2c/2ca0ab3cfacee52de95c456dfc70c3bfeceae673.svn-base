#ifndef MODELDIALOG_H
#define MODELDIALOG_H

//(*Headers(ModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class ModelDialog: public wxDialog
{
	public:

		ModelDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~ModelDialog();
        void UpdateLabels();

		//(*Declarations(ModelDialog)
		wxSpinCtrl* SpinCtrl_parm2;
		wxSpinCtrl* SpinCtrl_parm1;
		wxStaticText* StaticText_Strings;
		wxTextCtrl* TextCtrl_Name;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxSpinCtrl* SpinCtrl_parm3;
		wxRadioButton* RadioButton_RtoL;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxCheckBox* CheckBox_MyDisplay;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxChoice* Choice_Order;
		wxChoice* Choice_Antialias;
		wxChoice* Choice_DisplayAs;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_StartChannel;
		wxStaticText* StaticText_Strands;
		wxStaticText* StaticText_Nodes;
		wxRadioButton* RadioButton_LtoR;
		//*)

	protected:

		//(*Identifiers(ModelDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT6;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT8;
		static const long ID_RADIOBUTTON1;
		static const long ID_RADIOBUTTON2;
		static const long ID_STATICTEXT9;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT10;
		static const long ID_CHECKBOX1;
		//*)

	private:

		//(*Handlers(ModelDialog)
		void OnChoice_DisplayAsSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
