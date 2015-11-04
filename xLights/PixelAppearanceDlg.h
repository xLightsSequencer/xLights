#ifndef PIXELAPPEARANCEDLG_H
#define PIXELAPPEARANCEDLG_H

//(*Headers(PixelAppearanceDlg)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
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
		wxTextCtrl* BlackTransparencyText;
		wxTextCtrl* TransparencyText;
		wxSlider* TransparencySlider;
		wxSpinCtrl* PixelSizeSpinner;
		wxStaticText* StaticText4;
		wxSlider* BlackTransparency;
		//*)

	protected:

		//(*Identifiers(PixelAppearanceDlg)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_CHOICE1;
		static const long ID_SLIDER_TRANSPARENCY;
		static const long IDD_TEXTCTRL_TRANSPARENCY;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_BLACKT;
		static const long IDD_TEXTCTRL_BLACKT;
		//*)

	private:

		//(*Handlers(PixelAppearanceDlg)
		void OnSpinButton1Change(wxSpinEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
