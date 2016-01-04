#ifndef STROBEPANEL_H
#define STROBEPANEL_H

//(*Headers(StrobePanel)
#include <wx/panel.h>
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class StrobePanel: public wxPanel
{
	public:

		StrobePanel(wxWindow* parent);
		virtual ~StrobePanel();

		//(*Declarations(StrobePanel)
		wxBitmapButton* BitmapButton_StrobeDuration;
		wxStaticText* StaticText111;
		wxStaticText* StaticText110;
		wxSlider* Slider_Strobe_Type;
		wxBitmapButton* BitmapButton_Strobe_Type;
		wxStaticText* StaticText112;
		wxBitmapButton* BitmapButton_NumberStrobes;
		wxSlider* Slider_Number_Strobes;
		wxSlider* Slider_Strobe_Duration;
		//*)

	protected:

		//(*Identifiers(StrobePanel)
		static const long ID_STATICTEXT112;
		static const long ID_SLIDER_Number_Strobes;
		static const long ID_BITMAPBUTTON49;
		static const long ID_STATICTEXT113;
		static const long ID_SLIDER_Strobe_Duration;
		static const long ID_BITMAPBUTTON51;
		static const long ID_STATICTEXT114;
		static const long ID_SLIDER_Strobe_Type;
		static const long ID_BITMAPBUTTON50;
		//*)

	public:

		//(*Handlers(StrobePanel)
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
