#ifndef COLORPANEL_H
#define COLORPANEL_H

//(*Headers(ColorPanel)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
//*)

#include <wx/colordlg.h>

class ColorPanel: public wxPanel
{
	public:

		ColorPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorPanel();

        bool PaletteChanged;
        bool EffectChanged;

        void SetDefaultPalette();
        wxColour GetPaletteColor(int idx);
        void SetPaletteColor(int idx, const wxColour* c);
        void SetButtonColor(wxButton* btn, const wxColour* c);
        static const int PALETTE_SIZE = 6;

		//(*Declarations(ColorPanel)
		wxButton* Button_Palette6;
		wxButton* Button_Palette3;
		wxButton* Button_Palette2;
		wxFlexGridSizer* FlexGridSizer_Palette;
		wxCheckBox* CheckBox_Palette6;
		wxBitmapButton* BitmapButton_Palette3;
		wxButton* Button_Palette5;
		wxBitmapButton* BitmapButton_Palette2;
		wxCheckBox* CheckBox_Palette1;
		wxCheckBox* CheckBox_Palette5;
		wxBitmapButton* BitmapButton_Palette1;
		wxCheckBox* CheckBox_Palette3;
		wxCheckBox* CheckBox_Palette4;
		wxButton* Button_Palette4;
		wxBitmapButton* BitmapButton_Palette4;
		wxBitmapButton* BitmapButton_Palette5;
		wxCheckBox* CheckBox_Palette2;
		wxBitmapButton* BitmapButton_Palette6;
		wxButton* Button_Palette1;
		//*)

	protected:

		//(*Identifiers(ColorPanel)
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON_Palette1;
		static const long ID_BITMAPBUTTON_BUTTON_Palette1;
		static const long ID_CHECKBOX_Palette2;
		static const long ID_BUTTON_Palette2;
		static const long ID_BITMAPBUTTON_BUTTON_Palette2;
		static const long ID_CHECKBOX_Palette3;
		static const long ID_BUTTON_Palette3;
		static const long ID_BITMAPBUTTON_BUTTON_Palette3;
		static const long ID_CHECKBOX_Palette4;
		static const long ID_BUTTON_Palette4;
		static const long ID_BITMAPBUTTON_BUTTON_Palette4;
		static const long ID_CHECKBOX_Palette5;
		static const long ID_BUTTON_Palette5;
		static const long ID_BITMAPBUTTON_BUTTON_Palette5;
		static const long ID_CHECKBOX_Palette6;
		static const long ID_BUTTON_Palette6;
		static const long ID_BITMAPBUTTON_BUTTON_Palette6;
		//*)

	private:

		//(*Handlers(ColorPanel)
		void OnCheckBox_PaletteClick(wxCommandEvent& event);
		void OnBitmapButton_Palette2Click(wxCommandEvent& event);
		void OnBitmapButton_Palette1Click(wxCommandEvent& event);
		void OnBitmapButton_Palette3Click(wxCommandEvent& event);
		void OnBitmapButton_Palette4Click(wxCommandEvent& event);
		void OnBitmapButton_Palette5Click(wxCommandEvent& event);
		void OnBitmapButton_Palette6Click(wxCommandEvent& event);
		void OnButton_PaletteNumberClick(wxCommandEvent& event);
		//*)
        wxCheckBox* GetPaletteCheckbox(int idx);
        wxButton* GetPaletteButton(int idx);
        wxColourData colorData;
        void setlock(wxButton* button); //, EditState& islocked);


		DECLARE_EVENT_TABLE()
};

#endif
