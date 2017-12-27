#ifndef COLORPANEL_H
#define COLORPANEL_H

//(*Headers(ColorPanel)
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
//*)

#include <wx/dir.h>
#include "ColorCurve.h"
#include "BulkEditControls.h"

#include <vector>
#include <map>
#include "osx_utils/TouchBars.h"

wxDECLARE_EVENT(EVT_EFFECT_PALETTE_UPDATED, wxCommandEvent);

#define COLORPANEL_BRIGHTNESS_MIN 0
#define COLORPANEL_BRIGHTNESS_MAX 400
#define COLORPANEL_SPARKLE_MIN 0
#define COLORPANEL_SPARKLE_MAX 200
#define COLORPANEL_VALUE_MIN -100
#define COLORPANEL_VALUE_MAX 100
#define COLORPANEL_HUE_MIN -100
#define COLORPANEL_HUE_MAX 100
#define COLORPANEL_SATURATION_MIN -100
#define COLORPANEL_SATURATION_MAX 100

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

		ColorPanel(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorPanel();

        void SetColorCount(int count);
        void SetDefaultSettings(bool optionbased = false);
        wxString GetColorString();
        wxString GetRandomColorString();
        void SetSupports(bool linear, bool radial);

        bool PaletteChanged;
        bool EffectChanged;
    
        void SetDefaultPalette();
    
        ColorPanelTouchBar *SetupTouchBar(xlTouchBarSupport &tbs);
        ColorPanelTouchBar *GetTouchBar() const { return touchBar; }
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
		BulkEditSlider* Slider_Color_ValueAdjust;
		wxStaticText* StaticText22;
		wxCheckBox* CheckBox_ResetColorPanel;
		wxBitmapButton* BitmapButton_Contrast;
		wxBitmapButton* BitmapButton_Brightness;
		wxFlexGridSizer* FlexGridSizer_Palette;
		BulkEditTextCtrl* txtCtlBrightness;
		wxBitmapButton* BitmapButton_ShuffleColours;
		wxStaticText* StaticText2;
		BulkEditValueCurveButton* BitmapButton_VCBrightness;
		wxBitmapButton* BitmapButton_SavePalette;
		wxBitmapButton* BitmapButton_random;
		BulkEditSlider* Slider_Color_HueAdjust;
		wxBitmapButton* BitmapButton_normal;
		BulkEditSlider* Slider_Color_SaturationAdjust;
		wxPanel* Panel_Sizer;
		BulkEditSlider* Slider_SparkleFrequency;
		ColourList* BitmapButton_ColourChoice;
		wxBitmapButton* BitmapButton_SparkleFrequency;
		wxStaticText* StaticText1;
		BulkEditTextCtrl* TextCtrl_Color_HueAdjust;
		wxStaticText* StaticText3;
		wxStaticText* StaticText127;
		BulkEditSlider* Slider_Brightness;
		wxBitmapButton* BitmapButton_locked;
		BulkEditValueCurveButton* BitmapButton_Color_HueAdjust;
		BulkEditTextCtrl* txtCtrlSparkleFreq;
		wxBitmapButton* BitmapButton_MusicSparkles;
		BulkEditValueCurveButton* BitmapButton_Color_SaturationAdjust;
		BulkEditTextCtrl* TextCtrl_Color_ValueAdjust;
		BulkEditValueCurveButton* BitmapButton_SparkleFrequencyVC;
		BulkEditSlider* Slider_Contrast;
		wxStaticText* StaticText126;
		BulkEditValueCurveButton* BitmapButton_Color_ValueAdjust;
		wxScrolledWindow* ColorScrollWindow;
		BulkEditTextCtrl* TextCtrl_Color_SaturationAdjust;
		wxBitmapButton* BitmapButton_DeletePalette;
		BulkEditTextCtrl* txtCtlContrast;
		wxStaticText* StaticText4;
		BulkEditCheckBox* CheckBox_MusicSparkles;
		//*)

	protected:

		//(*Identifiers(ColorPanel)
		static const long ID_BITMAPBUTTON4;
		static const long ID_CUSTOM1;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_CHECKBOX_ResetColorPanel;
		static const long ID_STATICTEXT_SparkleFrequency;
		static const long ID_SLIDER_SparkleFrequency;
		static const long ID_VALUECURVE_SparkleFrequency;
		static const long IDD_TEXTCTRL_SparkleFrequency;
		static const long ID_BITMAPBUTTON_SLIDER_SparkleFrequency;
		static const long ID_CHECKBOX_MusicSparkles;
		static const long ID_BITMAPBUTTON_MusicSparkles;
		static const long ID_STATICTEXT_Brightness;
		static const long ID_SLIDER_Brightness;
		static const long ID_VALUECURVE_Brightness;
		static const long IDD_TEXTCTRL_Brightness;
		static const long ID_BITMAPBUTTON_SLIDER_Brightness;
		static const long ID_STATICTEXT_Contrast;
		static const long ID_SLIDER_Contrast;
		static const long IDD_TEXTCTRL_Contrast;
		static const long ID_BITMAPBUTTON_SLIDER_Contrast;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT_Color_HueAdjust;
		static const long ID_SLIDER_Color_HueAdjust;
		static const long ID_VALUECURVE_Color_HueAdjust;
		static const long IDD_TEXTCTRL_Color_HueAdjust;
		static const long ID_STATICTEXT_Color_SaturationAdjust;
		static const long ID_SLIDER_Color_SaturationAdjust;
		static const long ID_VALUECURVE_Color_SaturationAdjust;
		static const long IDD_TEXTCTRL_Color_SaturationAdjust;
		static const long ID_STATICTEXT_Color_ValueAdjust;
		static const long ID_SLIDER_Color_ValueAdjust;
		static const long ID_VALUECURVE_Color_ValueAdjust;
		static const long IDD_TEXTCTRL_Color_ValueAdjust;
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
		void OnLockButtonClick(wxCommandEvent& event);
		void OnCheckBox_MusicSparklesClick(wxCommandEvent& event);
		void OnVCButtonClick(wxCommandEvent& event);
		void OnBitmapButton_SavePaletteClick(wxCommandEvent& event);
		void OnBitmapButton_DeletePaletteClick(wxCommandEvent& event);
		void OnBitmapButton_ShuffleColoursClick(wxCommandEvent& event);
		void UpdateTouchBarSlider(wxScrollEvent& event);
		void OnCheckBox_ResetColorPanelClick(wxCommandEvent& event);
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