#ifndef VIEWSETTINGSPANEL_H
#define VIEWSETTINGSPANEL_H

//(*Headers(ViewSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxGridBagSizer;
class wxStaticBoxSizer;
class wxStaticText;
//*)

class xLightsFrame;
class ViewSettingsPanel: public wxPanel
{
	public:

		ViewSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ViewSettingsPanel();

		//(*Declarations(ViewSettingsPanel)
		wxCheckBox* HousePreviewCheckBox;
		wxCheckBox* PlayControlsCheckBox;
		wxChoice* EffectAssistChoice;
		wxChoice* ModelHandleSizeChoice;
		wxChoice* OpenGLRenderOrderChoice;
		wxChoice* OpenGLVersionChoice;
		wxChoice* ToolIconSizeChoice;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ViewSettingsPanel)
		static const long ID_CHOICE3;
		static const long ID_CHOICE1;
		static const long ID_CHOICE2;
		static const long ID_CHOICE4;
		static const long ID_CHOICE5;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
