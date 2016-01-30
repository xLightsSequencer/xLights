#ifndef GALAXYPANEL_H
#define GALAXYPANEL_H

//(*Headers(GalaxyPanel)
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

class GalaxyPanel: public wxPanel
{
	public:

		GalaxyPanel(wxWindow* parent);
		virtual ~GalaxyPanel();

		//(*Declarations(GalaxyPanel)
		wxStaticText* StaticText139;
		wxBitmapButton* BitmapButton_Galaxy_CenterX;
		wxBitmapButton* BitmapButton_Galaxy_End_Radius;
		wxStaticText* StaticText126;
		wxCheckBox* CheckBox_Galaxy_Reverse;
		wxBitmapButton* BitmapButton_Galaxy_Revolutions;
		wxStaticText* StaticText131;
		wxStaticText* StaticText141;
		wxBitmapButton* BitmapButton_Galaxy_Start_Width;
		wxNotebook* Notebook_Galaxy;
		wxCheckBox* CheckBox_Galaxy_Inward;
		wxStaticText* StaticText127;
		wxStaticText* StaticText138;
		wxBitmapButton* BitmapButton_Galaxy_End_Width;
		wxBitmapButton* BitmapButton_Galaxy_Start_Angle;
		wxBitmapButton* BitmapButton_Galaxy_Start_Radius;
		wxStaticText* StaticText142;
		wxStaticText* StaticText140;
		wxBitmapButton* BitmapButton_Galaxy_Accel;
		wxCheckBox* CheckBox_Galaxy_Blend_Edges;
		wxStaticText* StaticText129;
		wxBitmapButton* BitmapButton_Galaxy_Duration;
		wxBitmapButton* BitmapButton_Galaxy_CenterY;
		//*)

	protected:

		//(*Identifiers(GalaxyPanel)
		static const long ID_SLIDER_Galaxy_CenterX;
		static const long IDD_TEXTCTRL_Galaxy_CenterX;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER_Galaxy_CenterY;
		static const long IDD_TEXTCTRL_Galaxy_CenterY;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY;
		static const long ID_STATICTEXT125;
		static const long ID_SLIDER_Galaxy_Start_Radius;
		static const long IDD_TEXTCTRL_Galaxy_Start_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius;
		static const long ID_STATICTEXT127;
		static const long ID_SLIDER_Galaxy_Start_Width;
		static const long IDD_TEXTCTRL_Galaxy_Start_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width;
		static const long ID_STATICTEXT126;
		static const long ID_SLIDER_Galaxy_Start_Angle;
		static const long IDD_TEXTCTRL_Galaxy_Start_Angle;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle;
		static const long ID_PANEL29;
		static const long ID_STATICTEXT130;
		static const long ID_SLIDER_Galaxy_End_Radius;
		static const long IDD_TEXTCTRL_Galaxy_End_Radius;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius;
		static const long ID_STATICTEXT132;
		static const long ID_SLIDER_Galaxy_End_Width;
		static const long IDD_TEXTCTRL_Galaxy_End_Width;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width;
		static const long ID_STATICTEXT134;
		static const long ID_SLIDER_Galaxy_Revolutions;
		static const long IDD_TEXTCTRL_Galaxy_Revolutions;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions;
		static const long ID_PANEL31;
		static const long ID_STATICTEXT139;
		static const long ID_SLIDER_Galaxy_Duration;
		static const long IDD_TEXTCTRL_Galaxy_Duration;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Duration;
		static const long ID_STATICTEXT140;
		static const long ID_SLIDER_Galaxy_Accel;
		static const long IDD_TEXTCTRL_Galaxy_Accel;
		static const long ID_BITMAPBUTTON_SLIDER_Galaxy_Accel;
		static const long ID_CHECKBOX_Galaxy_Reverse;
		static const long ID_CHECKBOX_Galaxy_Blend_Edges;
		static const long ID_CHECKBOX_Galaxy_Inward;
		static const long ID_PANEL32;
		static const long ID_NOTEBOOK_Galaxy;
		//*)

	public:

		//(*Handlers(GalaxyPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void OnCheckBox_Galaxy_Start_Radius_x10Click(wxCommandEvent& event);
    void OnCheckBox_Galaxy_Start_Width_x10Click(wxCommandEvent& event);
    void OnCheckBox_Galaxy_End_Radius_x10Click(wxCommandEvent& event);
    void OnCheckBox_Galaxy_End_Width_x10Click(wxCommandEvent& event);
    void OnCheckBox_Galaxy_Revs_x2Click(wxCommandEvent& event);

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
