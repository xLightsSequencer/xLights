#ifndef ColorManagerDialog_H
#define ColorManagerDialog_H

#include "Color.h"

//(*Headers(ColorManagerDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ColorManager;
class MainSequencer;

class ColorManagerDialog: public wxDialog
{
	public:

		ColorManagerDialog(wxWindow* parent, ColorManager& color_mgr_, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorManagerDialog();

        void SetMainSequencer(MainSequencer* sequencer);

		//(*Declarations(ColorManagerDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxBitmapButton* BitmapButton_Timing2;
		wxBitmapButton* BitmapButton_ReferenceEffect;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_HeaderSelectedColor;
		wxStaticText* StaticText6;
		wxStaticBitmap* StaticBitmap1;
		wxBitmapButton* BitmapButton_HeaderColor;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Reset;
		wxBitmapButton* BitmapButton_Timing1;
		wxBitmapButton* BitmapButton_EffectSelected;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_Timing4;
		wxBitmapButton* BitmapButton_Timing5;
		wxButton* Button_Close;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_Timing3;
		wxBitmapButton* BitmapButton_ModelSelected;
		//*)

	protected:

		//(*Identifiers(ColorManagerDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICBITMAP1;
		static const long ID_STATICTEXT2;
		static const long ID_BITMAPBUTTON_Timing1;
		static const long ID_STATICTEXT3;
		static const long ID_BITMAPBUTTON_Timing2;
		static const long ID_STATICTEXT4;
		static const long ID_BITMAPBUTTON_Timing3;
		static const long ID_STATICTEXT5;
		static const long ID_BITMAPBUTTON_Timing4;
		static const long ID_STATICTEXT6;
		static const long ID_BITMAPBUTTON_Timing5;
		static const long ID_STATICTEXT7;
		static const long ID_BITMAPBUTTON_EffectSelected;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_ReferenceEffect;
		static const long ID_STATICTEXT10;
		static const long ID_BITMAPBUTTON_HeaderColor;
		static const long ID_STATICTEXT11;
		static const long ID_BITMAPBUTTON_HeaderSelectedColor;
		static const long ID_STATICTEXT8;
		static const long ID_BITMAPBUTTON_ModelSelected;
		static const long ID_BUTTON_RESET;
		static const long ID_BUTTON_Close;
		//*)

	private:

		//(*Handlers(ColorManagerDialog)
		void ColorButtonSelected(wxCommandEvent& event);
		void OnButton_Reset_DefaultsClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		void SetButtonColor(wxBitmapButton* btn, const xlColor* color);
		void SetButtonColor(wxBitmapButton* btn, const wxColour* c);
		void UpdateButtonColors();

		MainSequencer* main_sequencer;
		ColorManager& color_mgr;
        wxBitmap color_fan;

};

#endif
