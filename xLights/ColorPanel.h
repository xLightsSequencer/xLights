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

#include <wx/dir.h>
#include <wx/colordlg.h>
#include "ValueCurveButton.h"
#include "ColorCurve.h"

#include <vector>
#include <map>
#include "osx_utils/TouchBars.h"

wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);

class ColourList;

class ColorPanel: public wxPanel
{
    void OnVCChanged(wxCommandEvent& event);
    void OnCCChanged(wxCommandEvent& event);

    int __brightness;
    std::list<std::string> _loadedPalettes;
    wxString _lastShowDir;
    bool _supportslinear;
    bool _supportsradial;

public:

		ColorPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorPanel();

        void SetColorCount(int count);
        void SetDefaultSettings();
		wxString GetColorString();
        wxString GetRandomColorString();
        void SetSupports(bool linear, bool radial);

        bool PaletteChanged;
        bool EffectChanged;
    
        void SetDefaultPalette();
    
        ColorPanelTouchBar *SetupTouchBar(xlTouchBarSupport &tbs);
        ColorPanelTouchBar *GetTouchBar() { return touchBar; }
private:
        void ValidateWindow();
        std::string GetCurrentPalette() const;
        wxString FindPaletteFile(const wxString& filename, const wxString& palette) const;
        wxColour GetPaletteColor(int idx) const;
        void LoadPalettes(wxDir& directory, bool subdirs);
        void LoadAllPalettes();

public:
        void SetButtonColor(ColorCurveButton* btn, const std::string &v, bool notify = true);
        void SetButtonColor(int btn, const xlColor &v, bool notify = true);

		//(*Declarations(ColorPanel)
		wxStaticText* StaticText22;
		wxTextCtrl* txtCtlBrightness;
		wxTextCtrl* txtCtrlSparkleFreq;
		wxScrolledWindow* ColorScrollWindow;
		wxStaticText* StaticText126;
		wxBitmapButton* BitmapButton_normal;
		wxBitmapButton* BitmapButton_MusicSparkles;
		wxCheckBox* CheckBox_MusicSparkles;
		wxTextCtrl* txtCtlContrast;
		wxBitmapButton* BitmapButton_SparkleFrequency;
		wxSlider* Slider_Contrast;
		wxSlider* Slider_SparkleFrequency;
		wxBitmapButton* BitmapButton_Contrast;
		ValueCurveButton* BitmapButton_SparkleFrequencyVC;
		wxPanel* Panel_Sizer;
		wxBitmapButton* BitmapButton_random;
		wxStaticText* StaticText127;
		wxBitmapButton* BitmapButton_ShuffleColours;
		wxBitmapButton* BitmapButton_Brightness;
		wxBitmapButton* BitmapButton_locked;
		ValueCurveButton* BitmapButton_VCBrightness;
		wxFlexGridSizer* FlexGridSizer_Palette;
		wxBitmapButton* BitmapButton_DeletePalette;
		ColourList* BitmapButton_ColourChoice;
		wxBitmapButton* BitmapButton_SavePalette;
		wxSlider* Slider_Brightness;
		//*)

	protected:

		//(*Identifiers(ColorPanel)
		static const long ID_BITMAPBUTTON4;
		static const long ID_CUSTOM1;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT24;
		static const long ID_SLIDER_SparkleFrequency;
		static const long ID_VALUECURVE_SparkleFrequency;
		static const long IDD_TEXTCTRL_SparkleFrequency;
		static const long ID_BITMAPBUTTON_SLIDER_SparkleFrequency;
		static const long ID_CHECKBOX_MusicSparkles;
		static const long ID_BITMAPBUTTON_MusicSparkles;
		static const long ID_STATICTEXT127;
		static const long ID_SLIDER_Brightness;
		static const long ID_VALUECURVE_Brightness;
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
		//void OnButton_PaletteNumberClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnUpdateColorClick(wxCommandEvent& event);
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
		void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
		void OnCheckBox_MusicSparklesClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
        void UpdateLinkedSliderFloat2(wxCommandEvent& event);
        void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
        void OnBitmapButton_SavePaletteClick(wxCommandEvent& event);
		void OnBitmapButton_DeletePaletteClick(wxCommandEvent& event);
		void OnBitmapButton_ShuffleColoursClick(wxCommandEvent& event);
        void UpdateTouchBarSlider(wxScrollEvent& event);
		//*)

        void OnCCButtonClick(wxCommandEvent& event);
        wxCheckBox* GetPaletteCheckbox(int idx);
        wxButton* GetPaletteButton(int idx);
        void OnColourChoiceDropDown(wxCommandEvent& event);
        void OnColourChoiceSelect(wxCommandEvent& event);
        wxColourData colorData;
    
        std::vector<ColorCurveButton*> buttons;
        std::vector<wxCheckBox*> checkBoxes;
        std::map<int, std::string> lastColors;
        ColorPanelTouchBar *touchBar;

		DECLARE_EVENT_TABLE()

};

#endif