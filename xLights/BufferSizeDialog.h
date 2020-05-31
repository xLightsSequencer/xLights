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

 //(*Headers(BufferSizeDialog)
 #include <wx/bmpbuttn.h>
 #include <wx/button.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/spinctrl.h>
 #include <wx/statline.h>
 #include <wx/stattext.h>
 //*)

#include "ValueCurveButton.h"

class BufferSizeDialog: public wxDialog
{
    void ValidateWindow();

	public:

		BufferSizeDialog(wxWindow* parent, bool usevc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferSizeDialog();
        void SetSizes(double top, double left, double bottom, double right, const std::string& topvc, const std::string& leftvc, const std::string& bottomvc, const std::string& rightvc);
        void OnVCChanged(wxCommandEvent& event); 

		//(*Declarations(BufferSizeDialog)
		ValueCurveButton* ValueCurve_Bottom;
		ValueCurveButton* ValueCurve_Left;
		ValueCurveButton* ValueCurve_Right;
		ValueCurveButton* ValueCurve_Top;
		wxButton* Button_Export;
		wxButton* Button_Load;
		wxSpinCtrlDouble* SpinCtrl_Bottom;
		wxSpinCtrlDouble* SpinCtrl_Left;
		wxSpinCtrlDouble* SpinCtrl_Right;
		wxSpinCtrlDouble* SpinCtrl_Top;
		wxStaticLine* StaticLine1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(BufferSizeDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_VALUECURVE_BufferTop;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_VALUECURVE_BufferLeft;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		static const long ID_VALUECURVE_BufferBottom;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL4;
		static const long ID_VALUECURVE_BufferRight;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_STATICLINE1;
		//*)

	private:

		//(*Handlers(BufferSizeDialog)
		void OnSpinCtrl_TopChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_LeftChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_BottomChange(wxSpinDoubleEvent& event);
		void OnSpinCtrl_RightChange(wxSpinDoubleEvent& event);
		void OnValueCurve_Click(wxCommandEvent& event);
		void OnButton_ExportClick(wxCommandEvent& event);
		void OnButton_LoadClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
