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

//(*Headers(SequenceFileSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class SequenceFileSettingsPanel: public wxPanel
{
	public:

		SequenceFileSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SequenceFileSettingsPanel();

		//(*Declarations(SequenceFileSettingsPanel)
		wxCheckBox* FSEQSaveCheckBox;
		wxCheckBox* ModelBlendCheckBox;
		wxCheckBox* RenderOnSaveCheckBox;
		wxChoice* AutoSaveIntervalChoice;
		wxChoice* FSEQVersionChoice;
		wxChoice* RenderCacheChoice;
		//*)
        
        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(SequenceFileSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_CHOICE3;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(SequenceFileSettingsPanel)
		void OnRenderOnSaveCheckBoxClick(wxCommandEvent& event);
		void OnFSEQSaveCheckBoxClick(wxCommandEvent& event);
		void OnModelBlendCheckBoxClick(wxCommandEvent& event);
		void OnRenderCacheChoiceSelect(wxCommandEvent& event);
		void OnAutoSaveIntervalChoiceSelect(wxCommandEvent& event);
		void OnFSEQVersionChoiceSelect(wxCommandEvent& event);
		void OnRenderModeChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
