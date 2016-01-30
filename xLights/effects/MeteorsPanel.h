#ifndef METEORSPANEL_H
#define METEORSPANEL_H

//(*Headers(MeteorsPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxChoice;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class MeteorsPanel: public wxPanel
{
	public:

		MeteorsPanel(wxWindow* parent);
		virtual ~MeteorsPanel();

		//(*Declarations(MeteorsPanel)
		wxStaticText* StaticText43;
		wxSlider* Slider_Meteors_Count;
		wxBitmapButton* BitmapButton_MeteorsEffect;
		wxStaticText* StaticText130;
		wxBitmapButton* BitmapButton_MeteorsType;
		wxStaticText* StaticText41;
		wxBitmapButton* BitmapButton_MeteorsSwirlIntensity;
		wxStaticText* StaticText128;
		wxSlider* Slider_Meteors_Length;
		wxSlider* Slider_Meteors_Swirl_Intensity;
		wxBitmapButton* BitmapButton_MeteorsLength;
		wxStaticText* StaticText39;
		wxBitmapButton* BitmapButton_MeteorsCount;
		//*)

	protected:

		//(*Identifiers(MeteorsPanel)
		static const long ID_STATICTEXT39;
		static const long ID_CHOICE_Meteors_Type;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Type;
		static const long ID_STATICTEXT129;
		static const long ID_CHOICE_Meteors_Effect;
		static const long ID_BITMAPBUTTON_CHOICE_Meteors_Effect;
		static const long ID_STATICTEXT41;
		static const long ID_SLIDER_Meteors_Count;
		static const long IDD_TEXTCTRL_Meteors_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Count;
		static const long ID_STATICTEXT43;
		static const long ID_SLIDER_Meteors_Length;
		static const long IDD_TEXTCTRL_Meteors_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Length;
		static const long ID_STATICTEXT131;
		static const long ID_SLIDER_Meteors_Swirl_Intensity;
		static const long IDD_TEXTCTRL_Meteors_Swirl_Intensity;
		static const long ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity;
		static const long ID_SLIDER_Meteors_Speed;
		static const long IDD_TEXTCTRL_Meteors_Speed;
		//*)

	public:

		//(*Handlers(MeteorsPanel)
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
