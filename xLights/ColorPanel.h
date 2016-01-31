#ifndef COLORPANEL_H
#define COLORPANEL_H

//(*Headers(ColorPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/colordlg.h>
#include <unordered_map>

wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);

class ColorPanel: public wxPanel
{
	public:

		ColorPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorPanel();

		wxString GetColorString();
        wxString GetRandomColorString();

        bool PaletteChanged;
        bool EffectChanged;

        void SetDefaultPalette();
        wxColour GetPaletteColor(int idx);
        void SetPaletteColor(int idx, const wxColour* c);
        void UpdateSliderText();
        static void SetButtonColor(wxBitmapButton* btn, const wxColour* c);
        static const int PALETTE_SIZE = 6;

		//(*Declarations(ColorPanel)
		wxCheckBox* CheckBox_Palette3;
		wxCheckBox* CheckBox_Palette2;
		wxCheckBox* CheckBox_Palette1;
		wxStaticText* StaticText22;
		wxTextCtrl* txtCtlBrightness;
		wxTextCtrl* txtCtrlSparkleFreq;
		wxScrolledWindow* ColorScrollWindow;
		wxStaticText* StaticText126;
		wxBitmapButton* BitmapButton_normal;
		wxBitmapButton* Button_Palette4;
		wxTextCtrl* txtCtlContrast;
		wxBitmapButton* BitmapButton_SparkleFrequency;
		wxSlider* Slider_Contrast;
		wxSlider* Slider_SparkleFrequency;
		wxBitmapButton* BitmapButton_Contrast;
		wxPanel* Panel_Sizer;
		wxBitmapButton* BitmapButton_Palette2;
		wxBitmapButton* BitmapButton_random;
		wxStaticText* StaticText127;
		wxBitmapButton* BitmapButton_Brightness;
		wxBitmapButton* BitmapButton_Palette1;
		wxBitmapButton* BitmapButton_Palette4;
		wxBitmapButton* Button_Palette1;
		wxBitmapButton* BitmapButton_locked;
		wxCheckBox* CheckBox_Palette4;
		wxBitmapButton* BitmapButton_Palette6;
		wxBitmapButton* BitmapButton_Palette5;
		wxCheckBox* CheckBox_Palette5;
		wxBitmapButton* BitmapButton_Palette3;
		wxBitmapButton* Button_Palette3;
		wxFlexGridSizer* FlexGridSizer_Palette;
		wxBitmapButton* Button_Palette2;
		wxBitmapButton* Button_Palette5;
		wxCheckBox* CheckBox_Palette6;
		wxBitmapButton* Button_Palette6;
		wxSlider* Slider_Brightness;
		//*)

	protected:

		//(*Identifiers(ColorPanel)
		static const long ID_CHECKBOX_Palette1;
		static const long ID_CHECKBOX_Palette2;
		static const long ID_CHECKBOX_Palette3;
		static const long ID_CHECKBOX_Palette4;
		static const long ID_CHECKBOX_Palette5;
		static const long ID_CHECKBOX_Palette6;
		static const long ID_BUTTON_Palette1;
		static const long ID_BUTTON_Palette2;
		static const long ID_BUTTON_Palette3;
		static const long ID_BUTTON_Palette4;
		static const long ID_BUTTON_Palette5;
		static const long ID_BUTTON_Palette6;
		static const long ID_BITMAPBUTTON_BUTTON_Palette1;
		static const long ID_BITMAPBUTTON_BUTTON_Palette2;
		static const long ID_BITMAPBUTTON_BUTTON_Palette3;
		static const long ID_BITMAPBUTTON_BUTTON_Palette4;
		static const long ID_BITMAPBUTTON_BUTTON_Palette5;
		static const long ID_BITMAPBUTTON_BUTTON_Palette6;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT24;
		static const long ID_SLIDER_SparkleFrequency;
		static const long IDL_TEXTCTRL_SPARKLE;
		static const long ID_BITMAPBUTTON_SLIDER_SparkleFrequency;
		static const long ID_STATICTEXT127;
		static const long ID_SLIDER_Brightness;
		static const long IDL_TEXTCTRLBRIGHTNESS;
		static const long ID_BITMAPBUTTON_SLIDER_Brightness;
		static const long ID_STATICTEXT128;
		static const long ID_SLIDER_Contrast;
		static const long IDL_TEXTCTRLCONTRAST;
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
		void OnBitmapButton_Palette2Click(wxCommandEvent& event);
		void OnBitmapButton_Palette1Click(wxCommandEvent& event);
		void OnBitmapButton_Palette3Click(wxCommandEvent& event);
		void OnBitmapButton_Palette4Click(wxCommandEvent& event);
		void OnBitmapButton_Palette5Click(wxCommandEvent& event);
		void OnBitmapButton_Palette6Click(wxCommandEvent& event);
		void OnButton_PaletteNumberClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnBitmapButton_SparkleFrequencyClick(wxCommandEvent& event);
		void OnBitmapButton_BrightnessClick(wxCommandEvent& event);
		void OnBitmapButton_ContrastClick(wxCommandEvent& event);
		void OntxtCtlBrightnessText(wxCommandEvent& event);
		void OntxtCtrlSparkleFreqText(wxCommandEvent& event);
		void OntxtCtlContrastText(wxCommandEvent& event);
		void OnSlider_SparkleFrequencyCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_BrightnessCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_ContrastCmdSliderUpdated(wxScrollEvent& event);
		void OnUpdateColorClick(wxCommandEvent& event);
		//*)
        wxCheckBox* GetPaletteCheckbox(int idx);
        wxButton* GetPaletteButton(int idx);
        wxColourData colorData;
        void setlock(wxButton* button); //, EditState& islocked);
        typedef enum { Normal, Locked, Random } EditState;
        std::unordered_map<std::string, EditState> buttonState;
        bool isRandom_(wxControl* ctl, const char*debug);
        #define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)

		DECLARE_EVENT_TABLE()

};

#endif
