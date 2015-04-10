#ifndef PIXELAPPEARANCEDLG_H
#define PIXELAPPEARANCEDLG_H

//(*Headers(PixelAppearanceDlg)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class PixelAppearanceDlg: public wxDialog
{
	public:

		PixelAppearanceDlg(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PixelAppearanceDlg();

		//(*Declarations(PixelAppearanceDlg)
		wxChoice* PixelStyleBox;
		wxStaticText* StaticText1;
		wxSlider* TransparencySlider;
		wxSpinCtrl* PixelSizeSpinner;
		//*)

	protected:

		//(*Identifiers(PixelAppearanceDlg)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_CHOICE1;
		static const long ID_SLIDER1;
		//*)

	private:

		//(*Handlers(PixelAppearanceDlg)
		void OnSpinButton1Change(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
