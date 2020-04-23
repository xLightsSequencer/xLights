#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(ModelDimmingCurveDialog)
#include <wx/glcanvas.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/choicebk.h>
#include <wx/filepicker.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/valnum.h>

#include <map>
#include "DimmingCurvePanel.h"

class ModelDimmingCurveDialog: public wxDialog
{
	public:

		ModelDimmingCurveDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelDimmingCurveDialog();
    
        void Init(std::map<std::string, std::map<std::string,std::string>> &dimmingInfo);
        void Update(std::map<std::string, std::map<std::string,std::string>> &dimmingInfo);

		//(*Declarations(ModelDimmingCurveDialog)
		DimmingCurvePanel* redDCPanel;
		DimmingCurvePanel* greenDCPanel;
		DimmingCurvePanel* blueDCPanel;
		wxSlider* RGBBlueSlider;
		wxFilePickerCtrl* SingleFilePicker;
		wxTextCtrl* SingleBrightnessBox;
		wxSlider* RGBRedSlider;
		wxPanel* BluePanel;
		wxPanel* RGBGammaPanel;
		wxPanel* RGBFromFilePanel;
		wxPanel* RedPanel;
		wxFilePickerCtrl* RGBRedFilePicker;
		wxStaticText* StaticText10;
		wxTextCtrl* RGBRedTextCtrl;
		wxPanel* FromFilePanel;
		wxSlider* SingleBrightnessSlider;
		wxStaticText* StaticText3;
		wxFilePickerCtrl* RGBGreenFilePicker;
		wxPanel* GammaPanel;
		wxStaticText* StaticText8;
		wxStaticText* StaticText7;
		wxTextCtrl* SingleGammaText;
		wxTextCtrl* RGBRedGammaTextCtrl;
		wxStaticText* StaticText4;
		wxChoicebook* DimmingTypeChoice;
		wxStaticText* StaticText5;
		wxFilePickerCtrl* RGBBlueFilePicker;
		wxStaticText* StaticText6;
		wxSlider* RGBGreenSlider;
		wxTextCtrl* RGBGreenGammaTextCtrl;
		wxTextCtrl* RGBBlueGammaTextCtrl;
		wxStaticText* StaticText9;
		wxPanel* GreenPanel;
		wxTextCtrl* RGBBlueTextCtrl;
		wxTextCtrl* RGBGreenTextCtrl;
		wxStaticText* StaticText11;
		//*)

        wxIntegerValidator<int> brightnessValidator;
        wxFloatingPointValidator<float> gammaValidator;
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

