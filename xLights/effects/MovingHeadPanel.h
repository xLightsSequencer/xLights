#ifndef MOVINGHEADPANEL_H
#define MOVINGHEADPANEL_H

//(*Headers(MovingHeadPanel)
#include <wx/bmpbuttn.h>
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
		wxFlexGridSizer* FlexGridSizer_Main;
		wxStaticText* Label_Pan;
		wxStaticText* Label_Tilt;
		//*)

	protected:

		//(*Identifiers(MovingHeadPanel)
		static const long ID_STATICTEXT_Pan;
		static const long IDD_SLIDER_Pan;
		static const long ID_VALUECURVE_Pan;
		static const long ID_TEXTCTRL_Pan;
		static const long ID_STATICTEXT_Tilt;
		static const long IDD_SLIDER_Tilt;
		static const long ID_VALUECURVE_Tilt;
		static const long ID_TEXTCTRL_Tilt;
		//*)

	private:

		//(*Handlers(MovingHeadPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
