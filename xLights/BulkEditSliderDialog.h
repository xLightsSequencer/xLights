#ifndef BULKEDITSLIDERDIALOG_H
#define BULKEDITSLIDERDIALOG_H

//(*Headers(BulkEditSliderDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "BulkEditControls.h"
#include "ValueCurveButton.h"

class BulkEditSliderDialog: public wxDialog
{
    ValueCurveButton* _vcb;
    BESLIDERTYPE _type;

	public:

		BulkEditSliderDialog(wxWindow* parent, const std::string& label, int value, int min, int max, int pageSize, BESLIDERTYPE type, ValueCurveButton* vcb, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~BulkEditSliderDialog();

		//(*Declarations(BulkEditSliderDialog)
		wxSlider* Slider_BulkEdit;
		wxButton* Button_Ok;
		wxStaticText* StaticText_Label;
		ValueCurveButton* BitmapButton_VC;
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_BulkEdit;
		//*)

	protected:

		//(*Identifiers(BulkEditSliderDialog)
		static const long ID_STATICTEXT_BulkEdit;
		static const long ID_SLIDER_BulkEdit;
		static const long ID_VALUECURVE_BulkEdit;
		static const long ID_TEXTCTRL_BulkEdit;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(BulkEditSliderDialog)
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSlider_BulkEditCmdSliderUpdated(wxScrollEvent& event);
		void OnTextCtrl_BulkEditText(wxCommandEvent& event);
		void OnBitmapButton_VCClick(wxCommandEvent& event);
		//*)

        void OnVCChanged(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
