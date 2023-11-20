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

//(*Headers(CheckSequenceSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class CheckSequenceSettingsPanel: public wxPanel
{
	public:

		CheckSequenceSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~CheckSequenceSettingsPanel();

		//(*Declarations(CheckSequenceSettingsPanel)
		wxCheckBox* CheckBox_CustomSizeCheck;
		wxCheckBox* CheckBox_DisableSketch;
		wxCheckBox* CheckBox_DupNodeMG;
		wxCheckBox* CheckBox_DupUniv;
		wxCheckBox* CheckBox_NonContigChOnPort;
		wxCheckBox* CheckBox_PreviewGroup;
		wxCheckBox* CheckBox_TransTime;
		wxStaticText* StaticText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(CheckSequenceSettingsPanel)
		static const long ID_STATICTEXT1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX5;
		static const long ID_CHECKBOX6;
		static const long ID_CHECKBOX7;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(CheckSequenceSettingsPanel)
		void OnCheckBox_DupUnivClick(wxCommandEvent& event);
		void OnCheckBox_NonContigChOnPortClick(wxCommandEvent& event);
		void OnCheckBox_PreviewGroupClick(wxCommandEvent& event);
		void OnCheckBox_DupNodeMGClick(wxCommandEvent& event);
		void OnCheckBox_TransTimeClick(wxCommandEvent& event);
		void OnCheckBox_CustomSizeCheckClick(wxCommandEvent& event);
		void OnCheckBox_DisableSketchClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
