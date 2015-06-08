#ifndef PERSPECTIVESPANEL_H
#define PERSPECTIVESPANEL_H

//(*Headers(PerspectivesPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

#include <wx/xml/xml.h>

wxDECLARE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(EVT_LOAD_PERSPECTIVE, wxCommandEvent);
wxDECLARE_EVENT(EVT_PERSPECTIVES_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SAVE_PERSPECTIVES, wxCommandEvent);

class PerspectivesPanel: public wxPanel
{
	public:

		PerspectivesPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		void SetPerspectives(wxXmlNode* perspectivesNode);
		virtual ~PerspectivesPanel();

		//(*Declarations(PerspectivesPanel)
		wxListBox* ListBoxPerspectives;
		wxButton* ButtonAddPerspective;
		wxStaticText* StaticText1;
		wxButton* ButtonRenamePerspective;
		wxButton* ButtonDeletePerspective;
		wxButton* ButtonSavePerspective;
		//*)

	protected:

		//(*Identifiers(PerspectivesPanel)
		static const long ID_BUTTON_ADD_PERSPECTIVE;
		static const long D_BUTTON_DELETE_PERSPECTIVE;
		static const long ID_BUTTON_RENAME_PERSPECTIVE;
		static const long ID_BUTTON_SAVE_PERSPECTIVE;
		static const long ID_LISTBOX_PERSPECTIVES;
		static const long ID_STATICTEXT1;
		//*)

	private:

		//(*Handlers(PerspectivesPanel)
		void OnButtonAddPerspectiveClick(wxCommandEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnListBoxPerspectivesDClick(wxCommandEvent& event);
		void OnButtonRenamePerspectiveClick(wxCommandEvent& event);
		void OnButtonDeletePerspectiveClick(wxCommandEvent& event);
		void OnButtonSavePerspectiveClick(wxCommandEvent& event);
		//*)

		bool CheckForDuplicates(const wxString& perspective_name);

		DECLARE_EVENT_TABLE()

		wxXmlNode* mPerspectivesNode;
};

#endif
