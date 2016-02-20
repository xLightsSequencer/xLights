#ifndef COLORPANEL_H
#define COLORPANEL_H

//(*Headers(ColorPanel)
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/colordlg.h>
#include <wx/checkbox.h>
#include <unordered_map>

#include <vector>
#include <map>

wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);

class ColorPanel: public wxPanel
{
	public:

		ColorPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorPanel();

        void SetDefaultSettings();
		wxString GetColorString();
        wxString GetRandomColorString();

        bool PaletteChanged;
        bool EffectChanged;

    
        void SetDefaultPalette();
private:
        wxColour GetPaletteColor(int idx);
public:
        void SetButtonColor(wxBitmapButton* btn, const std::string &v);
        static const int PALETTE_SIZE = 8;

		//(*Declarations(ColorPanel)
		wxStaticText* StaticText22;
		wxScrolledWindow* ColorScrollWindow;
		wxStaticText* StaticText126;
		wxBitmapButton* BitmapButton_normal;
		wxBitmapButton* BitmapButton_SparkleFrequency;
		wxSlider* Slider_Contrast;
		wxSlider* Slider_SparkleFrequency;
		wxBitmapButton* BitmapButton_Contrast;
		wxPanel* Panel_Sizer;
		wxBitmapButton* BitmapButton_random;
		wxStaticText* StaticText127;
		wxBitmapButton* BitmapButton_Brightness;
		wxBitmapButton* BitmapButton_locked;
		wxFlexGridSizer* FlexGridSizer_Palette;
		wxSlider* Slider_Brightness;
		//*)

	protected:

		//(*Identifiers(ColorPanel)
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT24;
		static const long ID_SLIDER_SparkleFrequency;
		static const long IDD_TEXTCTRL_SparkleFrequency;
		static const long ID_BITMAPBUTTON_SLIDER_SparkleFrequency;
		static const long ID_STATICTEXT127;
		static const long ID_SLIDER_Brightness;
		static const long IDD_TEXTCTRL_Brightness;
		static const long ID_BITMAPBUTTON_SLIDER_Brightness;
		static const long ID_STATICTEXT128;
		static const long ID_SLIDER_Contrast;
		static const long IDD_TEXTCTRL_Contrast;
		static const long ID_BITMAPBUTTON_SLIDER_Contrast;
		static const long ID_BITMAPBUTTON87;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON88;
		static const long ID_SCROLLED_ColorScroll;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(ColorPanel)
		void OnCheckBox_PaletteClick(wxCommandEvent& event);
		void OnButton_PaletteNumberClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnUpdateColorClick(wxCommandEvent& event);
    
    
        void UpdateLinkedSliderFloat(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
        void UpdateLinkedTextCtrl360(wxScrollEvent& event);
        void UpdateLinkedSlider360(wxCommandEvent& event);
        void UpdateLinkedTextCtrl(wxScrollEvent& event);
        void UpdateLinkedSlider(wxCommandEvent& event);
        void OnLockButtonClick(wxCommandEvent& event);
		//*)
        wxCheckBox* GetPaletteCheckbox(int idx);
        wxButton* GetPaletteButton(int idx);
        wxColourData colorData;
    
        std::vector<wxBitmapButton*> buttons;
        std::vector<wxCheckBox*> checkBoxes;
        std::map<int, std::string> lastColors;

		DECLARE_EVENT_TABLE()

};

#endif
