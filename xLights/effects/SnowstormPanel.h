#ifndef SNOWSTORMPANEL_H
#define SNOWSTORMPANEL_H

//(*Headers(SnowstormPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class SnowstormPanel: public wxPanel
{
	public:

		SnowstormPanel(wxWindow* parent);
		virtual ~SnowstormPanel();

		//(*Declarations(SnowstormPanel)
		wxSlider* Slider_Snowstorm_Length;
		wxSlider* Slider_Snowstorm_Count;
		wxBitmapButton* BitmapButton_SnowstormCount;
		wxBitmapButton* BitmapButton_SnowstormLength;
		wxStaticText* StaticText45;
		wxStaticText* StaticText51;
		//*)

	protected:

		//(*Identifiers(SnowstormPanel)
		static const long ID_STATICTEXT45;
		static const long ID_SLIDER_Snowstorm_Count;
		static const long IDD_TEXTCTRL_Snowstorm_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Snowstorm_Count;
		static const long ID_STATICTEXT51;
		static const long ID_SLIDER_Snowstorm_Length;
		static const long IDD_TEXTCTRL_Snowstorm_Length;
		static const long ID_BITMAPBUTTON_SLIDER_Snowstorm_Length;
		static const long ID_SLIDER_Snowstorm_Speed;
		static const long IDD_TEXTCTRL_Snowstorm_Speed;
		//*)

	public:

		//(*Handlers(SnowstormPanel)
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
