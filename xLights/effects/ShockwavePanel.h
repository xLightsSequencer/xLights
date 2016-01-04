#ifndef SHOCKWAVEPANEL_H
#define SHOCKWAVEPANEL_H

//(*Headers(ShockwavePanel)
#include <wx/panel.h>
class wxCheckBox;
class wxTextCtrl;
class wxNotebookEvent;
class wxNotebook;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class ShockwavePanel: public wxPanel
{
	public:

		ShockwavePanel(wxWindow* parent);
		virtual ~ShockwavePanel();

		//(*Declarations(ShockwavePanel)
		wxStaticText* StaticText156;
		wxBitmapButton* BitmapButton_Shockwave_End_Width;
		wxBitmapButton* BitmapButton_Shockwave_Start_Width;
		wxCheckBox* CheckBox_Shockwave_Blend_Edges;
		wxBitmapButton* BitmapButton_Shockwave_Accel;
		wxBitmapButton* BitmapButton_Shockwave_CenterX;
		wxStaticText* StaticText158;
		wxBitmapButton* BitmapButton_Shockwave_End_Radius;
		wxBitmapButton* BitmapButton_Shockwave_CenterY;
		wxBitmapButton* BitmapButton_Shockwave_Start_Radius;
		wxStaticText* StaticText159;
		wxStaticText* StaticText157;
		wxNotebook* Notebook3;
		wxStaticText* StaticText155;
		wxStaticText* StaticText166;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
