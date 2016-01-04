#ifndef SNOWFLAKESPANEL_H
#define SNOWFLAKESPANEL_H

//(*Headers(SnowflakesPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class SnowflakesPanel: public wxPanel
{
	public:

		SnowflakesPanel(wxWindow* parent);
		virtual ~SnowflakesPanel();

		//(*Declarations(SnowflakesPanel)
		wxSlider* Slider_Snowflakes_Type;
		wxStaticText* StaticText80;
		wxBitmapButton* BitmapButton16;
		wxBitmapButton* BitmapButton_SnowflakesType;
		wxBitmapButton* BitmapButton_SnowflakesCount;
		wxCheckBox* CheckBox5;
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
		static const long ID_CHECKBOX_Snowflakes_Accumulate;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
