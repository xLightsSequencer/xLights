#ifndef BUFFERTRANSFORMPROPERTIES_H
#define BUFFERTRANSFORMPROPERTIES_H

//(*Headers(BufferTransformProperties)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#include "RotoZoom.h"

class BufferTransformProperties: public wxDialog
{
    RotoZoomParms _backup;

    // these are only used for validation
    int __rotations;
    int __zooms;
    int __start;
    int __zoomminimum;
    int __zoommaximum;
    int __quality;
    int __xcenter;
    int __ycenter;
    RotoZoomParms* _parms;

    public:
		BufferTransformProperties(wxWindow* parent, RotoZoomParms* parms, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BufferTransformProperties();

		//(*Declarations(BufferTransformProperties)
		wxTextCtrl* TextCtrl_ZoomMaximum;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_YCenter;
		wxStaticText* StaticText2;
		wxSlider* Slider_Start;
		wxTextCtrl* TextCtrl_Start;
		wxStaticText* StaticText6;
		wxSlider* Slider_Zooms;
		wxTextCtrl* TextCtrl_XCenter;
		wxTextCtrl* TextCtrl_Rotations;
		wxTextCtrl* TextCtrl_ZoomMinimum;
		wxStaticText* StaticText8;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxSlider* Slider_YCenter;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxButton* Button_Cancel;
		wxSlider* Slider_Rotations;
		wxTextCtrl* TextCtrl_Quality;
		wxSlider* Slider_Quality;
		wxTextCtrl* TextCtrl_Zooms;
		wxSlider* Slider_ZoomMaximum;
		wxStaticText* StaticText4;
		wxSlider* Slider_XCenter;
		wxSlider* Slider_ZoomMinimum;
		//*)

	protected:

		//(*Identifiers(BufferTransformProperties)
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Rotations;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Zooms;
		static const long ID_TEXTCTRL_Zooms;
		static const long ID_STATICTEXT8;
		static const long ID_SLIDER3;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT7;
		static const long ID_SLIDER2;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT4;
		static const long IDD_SLIDER_ZoomMaximum;
		static const long ID_TEXTCTRL_ZoomMaximum;
		static const long ID_STATICTEXT5;
		static const long ID_SLIDER5;
		static const long ID_TEXTCTRL_XCenter;
		static const long ID_STATICTEXT6;
		static const long ID_SLIDER4;
		static const long ID_TEXTCTRL_YCenter;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER1;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(BufferTransformProperties)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSlider_RotationsCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_ZoomsCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_ZoomMaximumCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_XCenterCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_YCenterCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_RotationsText(wxCommandEvent& event);
		void OnTextCtrl_ZoomsText(wxCommandEvent& event);
		void OnTextCtrl_ZoomMaximumText(wxCommandEvent& event);
		void OnTextCtrl_XCenterText(wxCommandEvent& event);
		void OnTextCtrl_YCenterText(wxCommandEvent& event);
		void OnSlider_QualityCmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_ZoomMinimumCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_ZoomMinimumText(wxCommandEvent& event);
		void OnTextCtrl_QualityText(wxCommandEvent& event);
		void OnSlider_StartCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_StartText(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
