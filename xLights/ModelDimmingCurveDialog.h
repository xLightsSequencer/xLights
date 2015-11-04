#ifndef MODELDIMMINGCURVEDIALOG_H
#define MODELDIMMINGCURVEDIALOG_H

//(*Headers(ModelDimmingCurveDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/glcanvas.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/dialog.h>
#include <wx/choicebk.h>
//*)


#include <map>
#include "DimmingCurvePanel.h"

class ModelDimmingCurveDialog: public wxDialog
{
	public:

		ModelDimmingCurveDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelDimmingCurveDialog();
    
        void Init(std::map<wxString, std::map<wxString,wxString>> &dimmingInfo);
        void Update(std::map<wxString, std::map<wxString,wxString>> &dimmingInfo);

		//(*Declarations(ModelDimmingCurveDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxPanel* RGBGammaPanel;
		wxPanel* FromFilePanel;
		wxPanel* RGBFromFilePanel;
		wxPanel* RedPanel;
		wxTextCtrl* RGBGreenGammaTextCtrl;
		wxFilePickerCtrl* SingleFilePicker;
		DimmingCurvePanel* redDCPanel;
		wxStaticText* StaticText6;
		wxTextCtrl* RGBRedTextCtrl;
		wxPanel* GammaPanel;
		wxStaticText* StaticText8;
		wxStaticText* StaticText11;
		wxStaticText* StaticText3;
		wxSlider* SingleBrightnessSlider;
		wxFilePickerCtrl* RGBRedFilePicker;
		wxSlider* RGBBlueSlider;
		DimmingCurvePanel* greenDCPanel;
		wxSlider* RGBRedSlider;
		wxTextCtrl* RGBGreenTextCtrl;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxFilePickerCtrl* RGBGreenFilePicker;
		wxChoicebook* DimmingTypeChoice;
		wxPanel* GreenPanel;
		wxTextCtrl* SingleBrightnessBox;
		wxTextCtrl* SingleGammaText;
		wxFilePickerCtrl* RGBBlueFilePicker;
		wxSlider* RGBGreenSlider;
		wxPanel* BluePanel;
		wxStaticText* StaticText4;
		wxTextCtrl* RGBBlueTextCtrl;
		wxTextCtrl* RGBBlueGammaTextCtrl;
		wxTextCtrl* RGBRedGammaTextCtrl;
		DimmingCurvePanel* blueDCPanel;
		//*)

    
	protected:

		//(*Identifiers(ModelDimmingCurveDialog)
		static const long ID_TEXTCTRL1;
		static const long IDD_SLIDER_SINGLEBRIGHTNESS;
		static const long ID_TEXTCTRL_SINGLEBRIGHTNESS;
		static const long ID_PANEL1;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_PANEL2;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT7;
		static const long IDD_SLIDER_RGBRED;
		static const long ID_TEXTCTRL_RGBRED;
		static const long ID_STATICTEXT8;
		static const long ID_TEXTCTRL4;
		static const long ID_STATICTEXT9;
		static const long IDD_SLIDER_RGBGREEN;
		static const long ID_TEXTCTRL_RGBGREEN;
		static const long ID_STATICTEXT10;
		static const long ID_TEXTCTRL6;
		static const long ID_STATICTEXT11;
		static const long IDD_SLIDER_RGBBLUE;
		static const long ID_TEXTCTRL_RGBBLUE;
		static const long ID_PANEL3;
		static const long ID_STATICTEXT3;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_STATICTEXT4;
		static const long ID_FILEPICKERCTRL3;
		static const long ID_STATICTEXT5;
		static const long ID_FILEPICKERCTRL4;
		static const long ID_PANEL4;
		static const long ID_CHOICEBOOK1;
		static const long ID_GLCANVAS2;
		static const long ID_PANEL5;
		static const long ID_GLCANVAS1;
		static const long ID_PANEL6;
		static const long ID_GLCANVAS3;
		static const long ID_PANEL7;
		//*)

	private:

		//(*Handlers(ModelDimmingCurveDialog)
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void OnSingleGammaText(wxCommandEvent& event);
		void OnSingleBrightnessBoxText(wxCommandEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnRGBGammaText(wxCommandEvent& event);
		void OnRGBTextCtrlText(wxCommandEvent& event);
		void OnRGBFilePickerFileChanged(wxFileDirPickerEvent& event);
		void OnSingleFilePickerFileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
