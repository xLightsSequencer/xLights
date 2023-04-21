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

//(*Headers(ColorManagerSettingsPanel)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

#include "Color.h"

class xLightsFrame;
class wxBitmapButton;
class ColorManagerSettingsPanel: public wxPanel
{
	public:

		ColorManagerSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ColorManagerSettingsPanel();

		//(*Declarations(ColorManagerSettingsPanel)
		wxButton* ButtonExport;
		wxButton* ButtonImport;
		wxButton* Button_Reset;
		wxCheckBox* CheckBox_SuppressDarkMode;
		wxFlexGridSizer* Sizer_Effect_Grid;
		wxFlexGridSizer* Sizer_Layout_Tab;
		wxFlexGridSizer* Sizer_Timing_Tracks;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ColorManagerSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON_IMPORT;
		static const long ID_BUTTON_EXPORT;
		static const long ID_BUTTON_RESET;
		//*)

	private:
        xLightsFrame *frame;
    
        void UpdateButtonColors();
        void AddButtonsToDialog();
        void RefreshColors();
        void SetButtonColor(wxBitmapButton* btn, const wxColour &c);
        void SetButtonColor(wxBitmapButton* btn, const xlColor &color);
    
		//(*Handlers(ColorManagerSettingsPanel)
		void OnButtonImportClick(wxCommandEvent& event);
		void OnButtonExportClick(wxCommandEvent& event);
		void OnButton_ResetClick(wxCommandEvent& event);
		void ColorButtonSelected(wxCommandEvent& event);
		void OnCheckBox_SuppressDarkModeClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
