#ifndef FANPANEL_H
#define FANPANEL_H

//(*Headers(FanPanel)
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

class FanPanel: public wxPanel
{
	public:

		FanPanel(wxWindow* parent);
		virtual ~FanPanel();

		//(*Declarations(FanPanel)
		wxBitmapButton* BitmapButton_Fan_Element_Width;
		wxStaticText* StaticText144;
		wxCheckBox* CheckBox_Fan_Reverse;
		wxBitmapButton* BitmapButton_Fan_Blade_Width;
		wxStaticText* StaticText149;
		wxStaticText* StaticText147;
		wxStaticText* StaticText152;
		wxBitmapButton* BitmapButton_Fan_CenterY;
		wxStaticText* StaticText153;
		wxStaticText* StaticText145;
		wxStaticText* StaticText146;
		wxBitmapButton* BitmapButton_Fan_End_Radius;
		wxCheckBox* CheckBox_Fan_Blend_Edges;
		wxStaticText* StaticText150;
		wxBitmapButton* BitmapButton_Fan_Accel;
		wxBitmapButton* BitmapButton_Fan_Blade_Angle;
		wxBitmapButton* BitmapButton_Fan_Num_Elements;
		wxBitmapButton* BitmapButton_Fan_Start_Angle;
		wxNotebook* Notebook1;
		wxBitmapButton* BitmapButton_Fan_Revolutions;
		wxStaticText* StaticText143;
		wxStaticText* StaticText135;
		wxStaticText* StaticText148;
		wxBitmapButton* BitmapButton_Fan_Num_Blades;
		wxBitmapButton* BitmapButton_Fan_Start_Radius;
		wxBitmapButton* BitmapButton_Fan_CenterX;
		wxStaticText* StaticText151;
		wxBitmapButton* BitmapButton_Fan_Duration;
		//*)

	protected:

		//(*Identifiers(FanPanel)
		static const long ID_SLIDER_Fan_CenterX;
		static const long IDD_TEXTCTRL_Fan_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterX;
		static const long ID_STATICTEXT128;
		static const long ID_SLIDER_Fan_CenterY;
		static const long IDD_TEXTCTRL_Fan_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_CenterY;
		static const long ID_STATICTEXT136;
		static const long ID_SLIDER_Fan_Start_Radius;
		static const long IDD_TEXTCTRL_Fan_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius;
		static const long ID_STATICTEXT142;
		static const long ID_SLIDER_Fan_Start_Angle;
		static const long IDD_TEXTCTRL_Fan_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle;
		static const long ID_STATICTEXT141;
		static const long ID_SLIDER_Fan_End_Radius;
		static const long IDD_TEXTCTRL_Fan_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_End_Radius;
		static const long ID_STATICTEXT145;
		static const long ID_SLIDER_Fan_Revolutions;
		static const long IDD_TEXTCTRL_Fan_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Revolutions;
		static const long ID_PANEL38;
		static const long ID_STATICTEXT148;
		static const long ID_SLIDER_Fan_Num_Blades;
		static const long IDD_TEXTCTRL_Fan_Num_Blades;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades;
		static const long ID_STATICTEXT149;
		static const long ID_SLIDER_Fan_Blade_Width;
		static const long IDD_TEXTCTRL_Fan_Blade_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width;
		static const long ID_STATICTEXT150;
		static const long ID_SLIDER_Fan_Blade_Angle;
		static const long IDD_TEXTCTRL_Fan_Blade_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle;
		static const long ID_STATICTEXT143;
		static const long ID_SLIDER_Fan_Num_Elements;
		static const long IDD_TEXTCTRL_Fan_Num_Elements;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements;
		static const long ID_STATICTEXT144;
		static const long ID_SLIDER_Fan_Element_Width;
		static const long IDD_TEXTCTRL_Fan_Element_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Element_Width;
		static const long ID_PANEL42;
		static const long ID_STATICTEXT146;
		static const long ID_SLIDER_Fan_Duration;
		static const long IDD_TEXTCTRL_Fan_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Duration;
		static const long ID_STATICTEXT147;
		static const long ID_SLIDER_Fan_Accel;
		static const long IDD_TEXTCTRL_Fan_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Fan_Accel;
		static const long ID_CHECKBOX_Fan_Reverse;
		static const long ID_CHECKBOX_Fan_Blend_Edges;
		static const long ID_PANEL41;
		static const long ID_NOTEBOOK_Fan;
		//*)

	public:

		//(*Handlers(FanPanel)
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
