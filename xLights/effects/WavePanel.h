#ifndef WAVEPANEL_H
#define WAVEPANEL_H

//(*Headers(WavePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class WavePanel: public wxPanel
{
	public:

		WavePanel(wxWindow* parent);
		virtual ~WavePanel();

		//(*Declarations(WavePanel)
		wxSlider* Slider_Thickness_Percentage;
		wxStaticText* StaticText50;
		wxSlider* Slider_Wave_Height;
		wxStaticText* StaticText49;
		wxStaticText* StaticText47;
		wxBitmapButton* BitmapButton_Wave_Direction;
		wxStaticText* StaticText48;
		wxStaticText* StaticText56;
		wxBitmapButton* BitmapButton13;
		wxBitmapButton* BitmapButton_Wave_Height;
		wxStaticText* StaticText55;
		wxSlider* Slider_Number_Waves;
		wxBitmapButton* BitmapButton_Wave_Thickness;
		wxBitmapButton* BitmapButton_WaveType;
		wxBitmapButton* BitmapButton_NumberWaves;
		wxStaticText* StaticText189;
		wxCheckBox* CheckBox_Mirror_Wave;
		wxBitmapButton* BitmapButton_Wave_FillColors;
		wxStaticText* StaticText52;
		wxBitmapButton* BitmapButton_Wave_MirrorWave;
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
