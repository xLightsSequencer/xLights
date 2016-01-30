#ifndef SHIMMERPANEL_H
#define SHIMMERPANEL_H

//(*Headers(ShimmerPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
class wxCheckBox;
//*)

class ShimmerPanel: public wxPanel
{
	public:

		ShimmerPanel(wxWindow* parent);
		virtual ~ShimmerPanel();

		//(*Declarations(ShimmerPanel)
		wxStaticText* StaticText65;
		wxBitmapButton* BitmapButton5;
		wxStaticText* StaticText66;
		wxBitmapButton* BitmapButton_Shimmer_Duty_Factor;
		wxStaticText* StaticText67;
		wxBitmapButton* BitmapButton_Shimmer_Use_All_Colors;
		wxCheckBox* CheckBox_Shimmer_Use_All_Colors;
		//*)

	protected:

		//(*Identifiers(ShimmerPanel)
		static const long ID_STATICTEXT68;
		static const long ID_SLIDER_Shimmer_Duty_Factor;
		static const long IDD_TEXTCTRL_Shimmer_Duty_Factor;
		static const long ID_BITMAPBUTTON_DutyFactor;
		static const long ID_STATICTEXT9;
		static const long IDD_SLIDER_Shimmer_Cycles;
		static const long ID_TEXTCTRL_Shimmer_Cycles;
		static const long ID_BITMAPBUTTON24;
		static const long ID_STATICTEXT69;
		static const long ID_CHECKBOX_Shimmer_Use_All_Colors;
		static const long ID_BITMAPBUTTON_ShimmerUseAllColors;
		//*)

	public:

		//(*Handlers(ShimmerPanel)
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
