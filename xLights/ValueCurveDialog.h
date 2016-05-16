#ifndef VALUECURVEDIALOG_H
#define VALUECURVEDIALOG_H

//(*Headers(ValueCurveDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ValueCurve;

class ValueCurveDialog: public wxDialog
{
    // these are only used for validation
    int __p1;
    int __p2;
    int __p3;
    ValueCurve* _vc;
    float _grabbedPoint;
    void ValidateWindow();
    public:

		ValueCurveDialog(wxWindow* parent, ValueCurve* vc, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ValueCurveDialog();

		//(*Declarations(ValueCurveDialog)
		wxTextCtrl* TextCtrl_Parameter3;
		wxSlider* Slider_Parameter2;
		wxButton* Button_Ok;
		wxStaticText* StaticText2;
		wxPanel* Panel_Graph;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Parameter2;
		wxStaticText* StaticText_BottomValue;
		wxButton* Button_Cancel;
		wxSlider* Slider_Parameter3;
		wxStaticText* StaticText_TopValue;
		wxChoice* Choice1;
		wxSlider* Slider_Parameter1;
		wxTextCtrl* TextCtrl_Parameter1;
		//*)

	protected:

		//(*Identifiers(ValueCurveDialog)
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_PANEL1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long IDD_SLIDER_Parameter1;
		static const long ID_TEXTCTRL_Parameter1;
		static const long ID_STATICTEXT2;
		static const long IDD_SLIDER_Parameter2;
		static const long ID_TEXTCTRL_Parameter2;
		static const long ID_STATICTEXT5;
		static const long IDD_SLIDER_Parameter3;
		static const long ID_TEXTCTRL_Parameter3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ValueCurveDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnChoice1Select(wxCommandEvent& event);
		void OnPanel_GraphLeftDown(wxMouseEvent& event);
		void OnPanel_GraphLeftUp(wxMouseEvent& event);
		void OnPanel_GraphMouseMove(wxMouseEvent& event);
		void OnTextCtrl_Parameter1Text(wxCommandEvent& event);
		void OnTextCtrl_Parameter2Text(wxCommandEvent& event);
		void OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event);
		void OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event);
		void OnPanel_GraphPaint(wxPaintEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
