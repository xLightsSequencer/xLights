#ifndef BUFFERSIZEDIALOG_H
#define BUFFERSIZEDIALOG_H

//(*Headers(BufferSizeDialog)
#include <wx/bmpbuttn.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
//*)

#include "ValueCurveButton.h"

class BufferSizeDialog: public wxDialog
{
    void ValidateWindow();

	public:

		BufferSizeDialog(wxWindow* parent, bool usevc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferSizeDialog();
        void SetSizes(int top, int left, int bottom, int right, const std::string& topvc, const std::string& leftvc, const std::string& bottomvc, const std::string& rightvc);
        void OnVCChanged(wxCommandEvent& event); 

		//(*Declarations(BufferSizeDialog)
		ValueCurveButton* ValueCurve_Bottom;
		ValueCurveButton* ValueCurve_Left;
		ValueCurveButton* ValueCurve_Right;
		ValueCurveButton* ValueCurve_Top;
		wxSpinCtrl* SpinCtrl_Bottom;
		wxSpinCtrl* SpinCtrl_Left;
		wxSpinCtrl* SpinCtrl_Right;
		wxSpinCtrl* SpinCtrl_Top;
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
		//*)

	private:

		//(*Handlers(BufferSizeDialog)
		void OnSpinCtrl_TopChange(wxSpinEvent& event);
		void OnSpinCtrl_LeftChange(wxSpinEvent& event);
		void OnSpinCtrl_BottomChange(wxSpinEvent& event);
		void OnSpinCtrl_RightChange(wxSpinEvent& event);
		void OnValueCurve_Click(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
