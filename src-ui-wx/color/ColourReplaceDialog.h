#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(ColourReplaceDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class ColoursList;
class xLightsFrame;
class ColorCurveButton;

class ColourReplaceDialog: public wxDialog
{
    xLightsFrame* _frame = nullptr;
    ColorCurveButton* _ccb = nullptr;

	public:

		ColourReplaceDialog(xLightsFrame* frame, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ColourReplaceDialog();

		//(*Declarations(ColourReplaceDialog)
		ColoursList* BitmapButton_ExistingColour;
		wxButton* Button_Close;
		wxButton* Button_Replace;
		wxCheckBox* CheckBox_SelectedEffectsOnly;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

	protected:

		//(*Identifiers(ColourReplaceDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CUSTOM1;
		static const long ID_STATICTEXT2;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ColourReplaceDialog)
		void OnButton_ReplaceClick(wxCommandEvent& event);
		void OnButton_CloseClick(wxCommandEvent& event);
		void OnCheckBox_SelectedEffectsOnlyClick(wxCommandEvent& event);
		//*)

		void OnCCChanged(wxCommandEvent& event);
        void UpdateExistingColours();

		DECLARE_EVENT_TABLE()
};
