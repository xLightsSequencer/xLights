#ifndef SERVICESPANEL_H
#define SERVICESPANEL_H

//(*Headers(ServicesPanel)
#include "wx/propgrid/propgrid.h"
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class ServiceManager;
class EditableListCtrl;
class ServicesPanel : public wxPanel {
	public:

		ServicesPanel(wxWindow* parent, ServiceManager* sm, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ServicesPanel();

		//(*Declarations(ServicesPanel)
		wxButton* ButtonTest;
		wxChoice* ChoiceServicesTest;
		wxPropertyGrid* servicesGrid;
		wxStaticText* StatictText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ServicesPanel)
		static const wxWindowID ID_PROPERYMANAGER_SERVICES;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_CHOICE_SERVICES;
		static const wxWindowID ID_BUTTON_TEST;
		//*)

	private:
        ServiceManager* m_serviceManager;

		//(*Handlers(ServicesPanel)
		void OnButtonTestClick(wxCommandEvent& event);
		void OnChoiceServicesTestSelect(wxCommandEvent& event);
		//*)

		void OnPropertyGridChange(wxPropertyGridEvent& event);
    
        void SetupTests();

		DECLARE_EVENT_TABLE()
};

#endif
