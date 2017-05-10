#ifndef ColorManagerDialog_H
#define ColorManagerDialog_H

#include "Color.h"
#include <wx/bmpbuttn.h>

//(*Headers(ColorManagerDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
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
		wxFlexGridSizer* Sizer_Timing_Tracks;
		wxFlexGridSizer* Sizer_Effect_Grid;
		wxStaticBitmap* StaticBitmap1;
		wxStaticText* StaticText1;
		wxButton* Button_Reset;
		wxFlexGridSizer* Sizer_Layout_Tab;
		wxButton* Button_Cancel;
		wxButton* Button_Close;
		//*)

	protected:

		//(*Identifiers(ColorManagerDialog)
		static const long ID_STATICTEXT1;
		static const long ID_STATICBITMAP1;
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

		void SetButtonColor(wxBitmapButton* btn, const xlColor color);
		void SetButtonColor(wxBitmapButton* btn, const wxColour c);
        void AddButtonsToDialog();
		void UpdateButtonColors();
        void RefreshColors();

		MainSequencer* main_sequencer;
		ColorManager& color_mgr;
        wxBitmap color_fan;

};

#endif
