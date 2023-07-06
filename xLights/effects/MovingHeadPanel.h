#ifndef MOVINGHEADPANEL_H
#define MOVINGHEADPANEL_H

//(*Headers(MovingHeadPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class MovingHeadPanel: public xlEffectPanel
{
	public:

        MovingHeadPanel(wxWindow* parent);
		virtual ~MovingHeadPanel();
        virtual void ValidateWindow() override;

		//(*Declarations(MovingHeadPanel)
		BulkEditSliderF1* Slider_Pan;
		BulkEditSliderF1* Slider_Tilt;
		BulkEditValueCurveButton* ValueCurve_Pan;
		BulkEditValueCurveButton* ValueCurve_Tilt;
		wxCheckBox* CheckBox_MH1;
		wxCheckBox* CheckBox_MH2;
		wxCheckBox* CheckBox_MH3;
		wxCheckBox* CheckBox_MH4;
		wxCheckBox* CheckBox_MH5;
		wxCheckBox* CheckBox_MH6;
		wxCheckBox* CheckBox_MH7;
		wxCheckBox* CheckBox_MH8;
		wxFlexGridSizer* FlexGridSizer_Main;
		wxNotebook* Notebook1;
		wxPanel* PanelControl;
		wxPanel* PanelFan;
		wxPanel* PanelMovement;
		wxPanel* PanelPosition;
		wxStaticText* Label_Pan;
		wxStaticText* Label_Tilt;
		wxStaticText* StaticTextFixtures;
		//*)

	protected:

		//(*Identifiers(MovingHeadPanel)
		static const long ID_STATICTEXT_Fixtures;
		static const long ID_CHECKBOX_MH1;
		static const long ID_CHECKBOX_MH2;
		static const long ID_CHECKBOX_MH3;
		static const long ID_CHECKBOX_MH4;
		static const long ID_CHECKBOX_MH5;
		static const long ID_CHECKBOX_MH6;
		static const long ID_CHECKBOX_MH7;
		static const long ID_CHECKBOX_MH8;
		static const long ID_STATICTEXT_Pan;
		static const long IDD_SLIDER_Pan;
		static const long ID_VALUECURVE_Pan;
		static const long ID_TEXTCTRL_Pan;
		static const long ID_STATICTEXT_Tilt;
		static const long IDD_SLIDER_Tilt;
		static const long ID_VALUECURVE_Tilt;
		static const long ID_TEXTCTRL_Tilt;
		static const long ID_PANEL_Position;
		static const long ID_PANEL_Fan;
		static const long ID_PANEL_Movement;
		static const long ID_PANEL_Control;
		static const long ID_NOTEBOOK1;
		//*)

	private:

		//(*Handlers(MovingHeadPanel)
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnCheckBox_MH2Click(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
