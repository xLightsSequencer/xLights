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

//(*Headers(NoteRangeDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class NoteRangeDialog: public wxDialog
{
    int& _lowNote;
    int& _highNote;
    void UpdateNotes();

	public:

		NoteRangeDialog(wxWindow* parent, int&lowNote, int&highNote, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NoteRangeDialog();

		//(*Declarations(NoteRangeDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxSlider* Slider_High;
		wxSlider* Slider_Low;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_HighHz;
		wxStaticText* StaticText_LowHz;
		wxTextCtrl* TextCtrl_High;
		wxTextCtrl* TextCtrl_Low;
		//*)

	protected:

		//(*Identifiers(NoteRangeDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER2;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_BUTTON_Ok;
		static const long ID_BUTTON_CANCEL;
		//*)

	private:

		//(*Handlers(NoteRangeDialog)
		void OnSlider_LowCmdScroll(wxScrollEvent& event);
		void OnSlider_HighCmdScroll(wxScrollEvent& event);
		void OnTextCtrl_HighText(wxCommandEvent& event);
		void OnTextCtrl_LowText(wxCommandEvent& event);
		void OnTextCtrl_LowTextEnter(wxCommandEvent& event);
		void OnTextCtrl_HighTextEnter(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

