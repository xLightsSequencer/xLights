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

 //(*Headers(ScheduleDialog)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/choice.h>
 #include <wx/datectrl.h>
 #include <wx/dateevt.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/stattext.h>
 #include <wx/textctrl.h>
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
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_Enabled;
		wxCheckBox* CheckBox_EveryYear;
		wxCheckBox* CheckBox_Fri;
		wxCheckBox* CheckBox_GracefullyInterrupt;
		wxCheckBox* CheckBox_HardStop;
		wxCheckBox* CheckBox_Loop;
		wxCheckBox* CheckBox_Mon;
		wxCheckBox* CheckBox_Sat;
		wxCheckBox* CheckBox_Shuffle;
		wxCheckBox* CheckBox_Sun;
		wxCheckBox* CheckBox_Thu;
		wxCheckBox* CheckBox_Tue;
		wxCheckBox* CheckBox_Wed;
		wxChoice* Choice_FireFrequency;
		wxDatePickerCtrl* DatePickerCtrl_End;
		wxDatePickerCtrl* DatePickerCtrl_Start;
		wxSpinCtrl* SpinCtrl_MaxLoops;
		wxSpinCtrl* SpinCtrl_NthDay;
		wxSpinCtrl* SpinCtrl_NthDayOffset;
		wxSpinCtrl* SpinCtrl_OffOffsetMins;
		wxSpinCtrl* SpinCtrl_OnOffsetMins;
		wxSpinCtrl* SpinCtrl_Priority;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText12;
		wxStaticText* StaticText13;
		wxStaticText* StaticText14;
		wxStaticText* StaticText15;
		wxStaticText* StaticText16;
		wxStaticText* StaticText17;
		wxStaticText* StaticText18;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticText_NextDay;
		wxStaticText* StaticText_OffOffsetMins;
		wxStaticText* StaticText_OnOffsetMins;
		wxTextCtrl* TextCtrl_Name;
		wxTextCtrl* TextCtrl_OffTime;
		wxTextCtrl* TextCtrl_OnTime;
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
		static const long ID_STATICTEXT16;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT17;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT18;
		static const long ID_STATICTEXT12;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT20;
		static const long ID_SPINCTRL5;
		static const long ID_STATICTEXT11;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT21;
		static const long ID_SPINCTRL6;
		static const long ID_CHECKBOX12;
		static const long ID_CHECKBOX13;
		static const long ID_STATICTEXT19;
		static const long ID_CHOICE1;
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
		void OnTextCtrl_OnTimeText(wxCommandEvent& event);
		void OnTextCtrl_OffTimeText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnCheckBox_DOWClick(wxCommandEvent& event);
		void OnTextCtrl_NameText(wxCommandEvent& event);
		void OnSpinCtrl_NthDayChange(wxSpinEvent& event);
		void OnSpinCtrl_NthDayOffsetChange(wxSpinEvent& event);
		void OnChoice_FireFrequencySelect(wxCommandEvent& event);
		//*)

        void OnTextCtrl_OnKillFocus(wxFocusEvent& event);
        void OnTextCtrl_OffKillFocus(wxFocusEvent& event);

		DECLARE_EVENT_TABLE()
};

