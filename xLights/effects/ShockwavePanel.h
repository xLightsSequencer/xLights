#ifndef SHOCKWAVEPANEL_H
#define SHOCKWAVEPANEL_H

//(*Headers(ShockwavePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxNotebook;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxNotebookEvent;
class wxStaticText;
class wxCheckBox;
//*)

class ShockwavePanel: public wxPanel
{
	public:

		ShockwavePanel(wxWindow* parent);
		virtual ~ShockwavePanel();

		//(*Declarations(ShockwavePanel)
		wxBitmapButton* BitmapButton_Shockwave_End_Radius;
		wxStaticText* StaticText155;
		wxCheckBox* CheckBox_Shockwave_Blend_Edges;
		wxStaticText* StaticText156;
		wxStaticText* StaticText159;
		wxBitmapButton* BitmapButton_Shockwave_CenterX;
		wxBitmapButton* BitmapButton_Shockwave_CenterY;
		wxBitmapButton* BitmapButton_Shockwave_Accel;
		wxStaticText* StaticText157;
		wxStaticText* StaticText166;
		wxNotebook* Notebook3;
		wxBitmapButton* BitmapButton_Shockwave_End_Width;
		wxBitmapButton* BitmapButton_Shockwave_Start_Width;
		wxStaticText* StaticText158;
		wxBitmapButton* BitmapButton_Shockwave_Start_Radius;
		//*)

	protected:

		//(*Identifiers(ShockwavePanel)
		static const long ID_SLIDER_Shockwave_CenterX;
		static const long IDD_TEXTCTRL_Shockwave_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX;
		static const long ID_STATICTEXT151;
		static const long ID_SLIDER_Shockwave_CenterY;
		static const long IDD_TEXTCTRL_Shockwave_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY;
		static const long ID_STATICTEXT152;
		static const long ID_SLIDER_Shockwave_Start_Radius;
		static const long IDD_TEXTCTRL_Shockwave_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius;
		static const long ID_STATICTEXT154;
		static const long ID_SLIDER_Shockwave_End_Radius;
		static const long IDD_TEXTCTRL_Shockwave_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius;
		static const long ID_STATICTEXT153;
		static const long ID_SLIDER_Shockwave_Start_Width;
		static const long IDD_TEXTCTRL_Shockwave_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width;
		static const long ID_STATICTEXT155;
		static const long ID_SLIDER_Shockwave_End_Width;
		static const long IDD_TEXTCTRL_Shockwave_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width;
		static const long ID_PANEL40;
		static const long ID_STATICTEXT162;
		static const long ID_SLIDER_Shockwave_Accel;
		static const long IDD_TEXTCTRL_Shockwave_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Shockwave_Accel;
		static const long ID_CHECKBOX_Shockwave_Blend_Edges;
		static const long ID_PANEL44;
		static const long ID_NOTEBOOK_Shockwave;
		//*)

	public:

		//(*Handlers(ShockwavePanel)
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
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
