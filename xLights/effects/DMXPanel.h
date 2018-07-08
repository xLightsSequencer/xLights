#ifndef DMXPANEL_H
#define DMXPANEL_H

//(*Headers(DMXPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxFlexGridSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class DMXPanel: public wxPanel
{
	public:

		DMXPanel(wxWindow* parent);
		virtual ~DMXPanel();

		//(*Declarations(DMXPanel)
		BulkEditSlider* Slider_DMX10;
		BulkEditSlider* Slider_DMX11;
		BulkEditSlider* Slider_DMX12;
		BulkEditSlider* Slider_DMX13;
		BulkEditSlider* Slider_DMX14;
		BulkEditSlider* Slider_DMX15;
		BulkEditSlider* Slider_DMX16;
		BulkEditSlider* Slider_DMX17;
		BulkEditSlider* Slider_DMX18;
		BulkEditSlider* Slider_DMX1;
		BulkEditSlider* Slider_DMX2;
		BulkEditSlider* Slider_DMX3;
		BulkEditSlider* Slider_DMX4;
		BulkEditSlider* Slider_DMX5;
		BulkEditSlider* Slider_DMX6;
		BulkEditSlider* Slider_DMX7;
		BulkEditSlider* Slider_DMX8;
		BulkEditSlider* Slider_DMX9;
		BulkEditValueCurveButton* ValueCurve_DMX10;
		BulkEditValueCurveButton* ValueCurve_DMX11;
		BulkEditValueCurveButton* ValueCurve_DMX12;
		BulkEditValueCurveButton* ValueCurve_DMX13;
		BulkEditValueCurveButton* ValueCurve_DMX14;
		BulkEditValueCurveButton* ValueCurve_DMX15;
		BulkEditValueCurveButton* ValueCurve_DMX16;
		BulkEditValueCurveButton* ValueCurve_DMX17;
		BulkEditValueCurveButton* ValueCurve_DMX18;
		BulkEditValueCurveButton* ValueCurve_DMX1;
		BulkEditValueCurveButton* ValueCurve_DMX2;
		BulkEditValueCurveButton* ValueCurve_DMX3;
		BulkEditValueCurveButton* ValueCurve_DMX4;
		BulkEditValueCurveButton* ValueCurve_DMX5;
		BulkEditValueCurveButton* ValueCurve_DMX6;
		BulkEditValueCurveButton* ValueCurve_DMX7;
		BulkEditValueCurveButton* ValueCurve_DMX8;
		BulkEditValueCurveButton* ValueCurve_DMX9;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxFlexGridSizer* FlexGridSizer_Panel1;
		wxFlexGridSizer* FlexGridSizer_Panel2;
		wxNotebook* Notebook7;
		wxPanel* ChannelPanel1;
		wxPanel* ChannelPanel2;
		wxStaticText* Label_DMX10;
		wxStaticText* Label_DMX11;
		wxStaticText* Label_DMX12;
		wxStaticText* Label_DMX13;
		wxStaticText* Label_DMX14;
		wxStaticText* Label_DMX15;
		wxStaticText* Label_DMX16;
		wxStaticText* Label_DMX17;
		wxStaticText* Label_DMX18;
		wxStaticText* Label_DMX1;
		wxStaticText* Label_DMX2;
		wxStaticText* Label_DMX3;
		wxStaticText* Label_DMX4;
		wxStaticText* Label_DMX5;
		wxStaticText* Label_DMX6;
		wxStaticText* Label_DMX7;
		wxStaticText* Label_DMX8;
		wxStaticText* Label_DMX9;
		//*)

	protected:

		//(*Identifiers(DMXPanel)
		static const long ID_STATICTEXT_DMX1;
		static const long ID_SLIDER_DMX1;
		static const long ID_VALUECURVE_DMX1;
		static const long IDD_TEXTCTRL_DMX1;
		static const long ID_STATICTEXT_DMX2;
		static const long ID_SLIDER_DMX2;
		static const long ID_VALUECURVE_DMX2;
		static const long IDD_TEXTCTRL_DMX2;
		static const long ID_STATICTEXT_DMX3;
		static const long ID_SLIDER_DMX3;
		static const long ID_VALUECURVE_DMX3;
		static const long IDD_TEXTCTRL_DMX3;
		static const long ID_STATICTEXT_DMX4;
		static const long ID_SLIDER_DMX4;
		static const long ID_VALUECURVE_DMX4;
		static const long IDD_TEXTCTRL_DMX4;
		static const long ID_STATICTEXT_DMX5;
		static const long ID_SLIDER_DMX5;
		static const long ID_VALUECURVE_DMX5;
		static const long IDD_TEXTCTRL_DMX5;
		static const long ID_STATICTEXT_DMX6;
		static const long ID_SLIDER_DMX6;
		static const long ID_VALUECURVE_DMX6;
		static const long IDD_TEXTCTRL_DMX6;
		static const long ID_STATICTEXT_DMX7;
		static const long ID_SLIDER_DMX7;
		static const long ID_VALUECURVE_DMX7;
		static const long IDD_TEXTCTRL_DMX7;
		static const long ID_STATICTEXT_DMX8;
		static const long ID_SLIDER_DMX8;
		static const long ID_VALUECURVE_DMX8;
		static const long IDD_TEXTCTRL_DMX8;
		static const long ID_STATICTEXT_DMX9;
		static const long ID_SLIDER_DMX9;
		static const long ID_VALUECURVE_DMX9;
		static const long IDD_TEXTCTRL_DMX9;
		static const long ID_PANEL6;
		static const long ID_STATICTEXT_DMX10;
		static const long ID_SLIDER_DMX10;
		static const long ID_VALUECURVE_DMX10;
		static const long IDD_TEXTCTRL_DMX10;
		static const long ID_STATICTEXT_DMX11;
		static const long ID_SLIDER_DMX11;
		static const long ID_VALUECURVE_DMX11;
		static const long IDD_TEXTCTRL_DMX11;
		static const long ID_STATICTEXT_DMX12;
		static const long ID_SLIDER_DMX12;
		static const long ID_VALUECURVE_DMX12;
		static const long IDD_TEXTCTRL_DMX12;
		static const long ID_STATICTEXT_DMX13;
		static const long ID_SLIDER_DMX13;
		static const long ID_VALUECURVE_DMX13;
		static const long IDD_TEXTCTRL_DMX13;
		static const long ID_STATICTEXT_DMX14;
		static const long ID_SLIDER_DMX14;
		static const long ID_VALUECURVE_DMX14;
		static const long IDD_TEXTCTRL_DMX14;
		static const long ID_STATICTEXT_DMX15;
		static const long ID_SLIDER_DMX15;
		static const long ID_VALUECURVE_DMX15;
		static const long IDD_TEXTCTRL_DMX15;
		static const long ID_STATICTEXT_DMX16;
		static const long ID_SLIDER_DMX16;
		static const long ID_VALUECURVE_DMX16;
		static const long IDD_TEXTCTRL_DMX16;
		static const long ID_STATICTEXT_DMX17;
		static const long ID_SLIDER_DMX17;
		static const long ID_VALUECURVE_DMX17;
		static const long IDD_TEXTCTRL_DMX17;
		static const long ID_STATICTEXT_DMX18;
		static const long ID_SLIDER_DMX18;
		static const long ID_VALUECURVE_DMX18;
		static const long IDD_TEXTCTRL_DMX18;
		static const long ID_PANEL28;
		static const long ID_NOTEBOOK1;
		//*)

	public:

		//(*Handlers(DMXPanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
