#ifndef SNOWFLAKESPANEL_H
#define SNOWFLAKESPANEL_H

//(*Headers(SnowflakesPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
class wxChoice;
//*)

class SnowflakesPanel: public wxPanel
{
	public:

		SnowflakesPanel(wxWindow* parent);
		virtual ~SnowflakesPanel();

		//(*Declarations(SnowflakesPanel)
		wxChoice* Choice_Falling;
		wxSlider* Slider_Snowflakes_Type;
		wxStaticText* StaticText80;
		wxBitmapButton* BitmapButton16;
		wxBitmapButton* BitmapButton_SnowflakesType;
		wxStaticText* StaticText2;
		wxBitmapButton* BitmapButton_SnowflakesCount;
		wxSlider* Slider_Snowflakes_Count;
		wxStaticText* StaticText1;
		wxBitmapButton* BitmapButton1;
		wxSlider* Slider_Snowflakes_Speed;
		wxStaticText* StaticText79;
		//*)

	protected:

		//(*Identifiers(SnowflakesPanel)
		static const long ID_STATICTEXT80;
		static const long ID_SLIDER_Snowflakes_Count;
		static const long IDD_TEXTCTRL_Snowflakes_Count;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Count;
		static const long ID_STATICTEXT81;
		static const long ID_SLIDER_Snowflakes_Type;
		static const long IDD_TEXTCTRL_Snowflakes_Type;
		static const long ID_BITMAPBUTTON_SLIDER_Snowflakes_Type;
		static const long ID_SLIDER_Snowflakes_Speed;
		static const long IDD_TEXTCTRL_Snowflakes_Speed;
		static const long ID_BITMAPBUTTON34;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Falling;
		static const long ID_STATICTEXT2;
		static const long ID_BITMAPBUTTON1;
		//*)

	public:

		//(*Handlers(SnowflakesPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
