#ifndef BUFFERSIZEDIALOG_H
#define BUFFERSIZEDIALOG_H

//(*Headers(BufferSizeDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
//*)

class BufferSizeDialog: public wxDialog
{
	public:

		BufferSizeDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferSizeDialog();
        void SetSizes(int top, int left, int bottom, int right);

		//(*Declarations(BufferSizeDialog)
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Top;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSpinCtrl* SpinCtrl_Right;
		wxSpinCtrl* SpinCtrl_Bottom;
		wxStaticText* StaticText4;
		wxSpinCtrl* SpinCtrl_Left;
		//*)

	protected:

		//(*Identifiers(BufferSizeDialog)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL4;
		//*)

	private:

		//(*Handlers(BufferSizeDialog)
		void OnSpinCtrl_TopChange(wxSpinEvent& event);
		void OnSpinCtrl_LeftChange(wxSpinEvent& event);
		void OnSpinCtrl_BottomChange(wxSpinEvent& event);
		void OnSpinCtrl_RightChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
