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
		wxBitmapButton* BitmapButton_Gridlines;
		wxBitmapButton* BitmapButton_Timing2;
		wxBitmapButton* BitmapButton_ReferenceEffect;
		wxStaticText* StaticText13;
		wxStaticText* StaticText2;
		wxStaticText* StaticText14;
		wxStaticText* StaticText6;
		wxStaticBitmap* StaticBitmap1;
		wxStaticText* StaticText19;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxStaticText* StaticText18;
		wxBitmapButton* BitmapButton_TimingDefault;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxBitmapButton* BitmapButton_RowHeader;
		wxButton* Button_Reset;
		wxBitmapButton* BitmapButton_Timing1;
		wxBitmapButton* BitmapButton_EffectSelected;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxBitmapButton* BitmapButton_LabelOutline;
		wxBitmapButton* BitmapButton_Timing4;
		wxButton* Button_Cancel;
		wxBitmapButton* BitmapButton_Phrases;
		wxBitmapButton* BitmapButton_Timing5;
		wxBitmapButton* BitmapButton_Phonemes;
		wxStaticText* StaticText15;
		wxStaticText* StaticText12;
		wxBitmapButton* BitmapButton_Words;
		wxBitmapButton* BitmapButton_RowHeaderSelected;
		wxButton* Button_Close;
		wxStaticText* StaticText17;
		wxStaticText* StaticText4;
		wxBitmapButton* BitmapButton_Timing3;
		wxStaticText* StaticText16;
		wxBitmapButton* BitmapButton_ModelSelected;
		wxBitmapButton* BitmapButton_EffectDefault;
		wxBitmapButton* BitmapButton_Labels;
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
		static const long ID_STATICTEXT19;
		static const long ID_BITMAPBUTTON_TimingDefault;
		static const long ID_STATICTEXT12;
		static const long ID_BITMAPBUTTON_EffectDefault;
		static const long ID_STATICTEXT7;
		static const long ID_BITMAPBUTTON_EffectSelected;
		static const long ID_STATICTEXT9;
		static const long ID_BITMAPBUTTON_ReferenceEffect;
		static const long ID_STATICTEXT10;
		static const long ID_BITMAPBUTTON_RowHeader;
		static const long ID_STATICTEXT11;
		static const long ID_BITMAPBUTTON_RowHeaderSelected;
		static const long ID_STATICTEXT13;
		static const long ID_BITMAPBUTTON_Gridlines;
		static const long ID_STATICTEXT14;
		static const long ID_BITMAPBUTTON_Labels;
		static const long ID_STATICTEXT15;
		static const long ID_BITMAPBUTTON_LabelOutline;
		static const long ID_STATICTEXT16;
		static const long ID_BITMAPBUTTON_Phrases;
		static const long ID_STATICTEXT17;
		static const long ID_BITMAPBUTTON_Words;
		static const long ID_STATICTEXT18;
		static const long ID_BITMAPBUTTON_Phonemes;
		static const long ID_STATICTEXT8;
		static const long ID_BITMAPBUTTON_ModelSelected;
		static const long ID_BUTTON_RESET;
		static const long ID_BUTTON_Cancel;
		static const long ID_BUTTON_Close;
		//*)

	private:

		//(*Handlers(ColorManagerDialog)
		void ColorButtonSelected(wxCommandEvent& event);
		void OnButton_Reset_DefaultsClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		void SetButtonColor(wxBitmapButton* btn, const xlColor* color);
		void SetButtonColor(wxBitmapButton* btn, const wxColour* c);
		void UpdateButtonColors();
        void RefreshColors();

		MainSequencer* main_sequencer;
		ColorManager& color_mgr;
        wxBitmap color_fan;

};

#endif
