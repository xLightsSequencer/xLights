#ifndef WAVEPANEL_H
#define WAVEPANEL_H

//(*Headers(WavePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class WavePanel: public wxPanel
{
	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		wxBitmapButton* BitmapButton_Wave_Direction;
		wxBitmapButton* BitmapButton_Wave_Height;
		wxStaticText* StaticText50;
		wxBitmapButton* BitmapButton13;
		wxBitmapButton* BitmapButton_Wave_MirrorWave;
		wxStaticText* StaticText56;
		wxCheckBox* CheckBox_Mirror_Wave;
		wxStaticText* StaticText55;
		wxStaticText* StaticText49;
		wxSlider* Slider_Thickness_Percentage;
		wxStaticText* StaticText189;
		wxStaticText* StaticText47;
		wxStaticText* StaticText52;
		wxBitmapButton* BitmapButton_Wave_Thickness;
		wxBitmapButton* BitmapButton_NumberWaves;
		wxBitmapButton* BitmapButton_Wave_FillColors;
		wxSlider* Slider_Number_Waves;
		wxBitmapButton* BitmapButton_WaveType;
		wxStaticText* StaticText48;
		wxSlider* Slider_Wave_Height;
		//*)

	protected:

		//(*Identifiers(WavePanel)
		static const long ID_STATICTEXT49;
		static const long ID_CHOICE_Wave_Type;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT50;
		static const long ID_CHOICE_Fill_Colors;
		static const long ID_BITMAPBUTTON3;
		static const long ID_STATICTEXT52;
		static const long ID_CHECKBOX_Mirror_Wave;
		static const long ID_BITMAPBUTTON4;
		static const long ID_STATICTEXT55;
		static const long ID_SLIDER_Number_Waves;
		static const long IDD_TEXTCTRL_Number_Waves;
		static const long ID_BITMAPBUTTON5;
		static const long ID_STATICTEXT56;
		static const long ID_SLIDER_Thickness_Percentage;
		static const long IDD_TEXTCTRL_Thickness_Percentage;
		static const long ID_BITMAPBUTTON6;
		static const long ID_STATICTEXT58;
		static const long ID_SLIDER_Wave_Height;
		static const long IDD_TEXTCTRL_Wave_Height;
		static const long ID_BITMAPBUTTON7;
		static const long ID_STATICTEXT32;
		static const long IDD_SLIDER_Wave_Speed;
		static const long ID_TEXTCTRL_Wave_Speed;
		static const long ID_BITMAPBUTTON31;
		static const long ID_STATICTEXT59;
		static const long ID_CHOICE_Wave_Direction;
		static const long ID_BITMAPBUTTON8;
		//*)

	public:

		//(*Handlers(WavePanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
