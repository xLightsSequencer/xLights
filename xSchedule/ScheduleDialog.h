#ifndef SCHEDULEDIALOG_H
#define SCHEDULEDIALOG_H

//(*Headers(ScheduleDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/datectrl.h>
#include <wx/spinctrl.h>
#include <wx/dateevt.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class Schedule;

class ScheduleDialog: public wxDialog
{
    Schedule* _schedule;
    void ValidateWindow();

	public:

		ScheduleDialog(wxWindow* parent, Schedule* schedule, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ScheduleDialog();

		//(*Declarations(ScheduleDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxCheckBox* CheckBox_Tue;
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Name;
		wxCheckBox* CheckBox_Sat;
		wxCheckBox* CheckBox_Sun;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText6;
		wxCheckBox* CheckBox_EveryYear;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxTextCtrl* TextCtrl_OnTime;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Priority;
		wxCheckBox* CheckBox_Wed;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxButton* Button_Cancel;
		wxCheckBox* CheckBox_Mon;
		wxDatePickerCtrl* DatePickerCtrl_End;
		wxStaticText* StaticText15;
		wxStaticText* StaticText12;
		wxTextCtrl* TextCtrl_OffTime;
		wxDatePickerCtrl* DatePickerCtrl_Start;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_MaxLoops;
		wxCheckBox* CheckBox_Loop;
		wxCheckBox* CheckBox_Thu;
		wxCheckBox* CheckBox_Shuffle;
		wxCheckBox* CheckBox_Fri;
		//*)

	protected:

		//(*Identifiers(ScheduleDialog)
		static const long ID_CHECKBOX11;
		static const long ID_STATICTEXT15;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT1;
		static const long ID_DATEPICKERCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_DATEPICKERCTRL2;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT7;
		static const long ID_STATICTEXT8;
		static const long ID_STATICTEXT9;
		static const long ID_STATICTEXT10;
		static const long ID_CHECKBOX8;
		static const long ID_CHECKBOX7;
		static const long ID_CHECKBOX6;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT12;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT11;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX9;
		static const long ID_STATICTEXT13;
		static const long ID_SPINCTRL1;
		static const long ID_CHECKBOX10;
		static const long ID_STATICTEXT14;
		static const long ID_SPINCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ScheduleDialog)
		void OnDatePickerCtrl_StartChanged(wxDateEvent& event);
		void OnDatePickerCtrl_EndChanged(wxDateEvent& event);
		void OnTextCtrl1Text(wxCommandEvent& event);
		void OnTextCtrl_OnTimeText(wxCommandEvent& event);
		void OnTextCtrl_OffTimeText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnCheckBox6Click(wxCommandEvent& event);
		void OnCheckBox_DOWClick(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
