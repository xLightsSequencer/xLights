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

//(*Headers(ViewSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;
class ViewSettingsPanel: public wxPanel
{
	public:

		ViewSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ViewSettingsPanel();

		//(*Declarations(ViewSettingsPanel)
		wxCheckBox* CheckBox_BaseShowFolder;
		wxCheckBox* HousePreviewCheckBox;
		wxCheckBox* PlayControlsCheckBox;
		wxChoice* Choice_TimelineZooming;
		wxChoice* EffectAssistChoice;
		wxChoice* ModelHandleSizeChoice;
		wxChoice* ToolIconSizeChoice;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ViewSettingsPanel)
		static const long ID_CHOICE3;
		static const long ID_CHOICE4;
		static const long ID_CHOICE5;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE_TIMELINEZOOMING;
		//*)

	private:
        xLightsFrame *frame;

		//(*Handlers(ViewSettingsPanel)
		void OnToolIconSizeChoiceSelect(wxCommandEvent& event);
		void OnHousePreviewCheckBoxClick(wxCommandEvent& event);
		void OnPlayControlsCheckBoxClick(wxCommandEvent& event);
		void OnEffectAssistChoiceSelect(wxCommandEvent& event);
		void OnModelHandleSizeChoiceSelect(wxCommandEvent& event);
		void OnOpenGLRenderOrderChoiceSelect(wxCommandEvent& event);
		void OnOpenGLVersionChoiceSelect(wxCommandEvent& event);
		void OnCheckBox_BaseShowFolderClick(wxCommandEvent& event);		
		void OnChoice_TimelineZoomingSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
