#ifndef SERVICESPANEL_H
#define SERVICESPANEL_H

//(*Headers(ServicesPanel)
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

class xLightsFrame;
class EditableListCtrl;
class ServicesPanel : public wxPanel {
	public:

		ServicesPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~ServicesPanel();

		//(*Declarations(ServicesPanel)
		EditableListCtrl* servicesList;
		wxButton* ButtonTest;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(ServicesPanel)
		static const long ID_LISTCTRL1;
		static const long ID_BUTTON1;
		//*)

	private:
        xLightsFrame* frame;

		//(*Handlers(ServicesPanel)
		void OnservicesListItemSelect(wxListEvent& event);
		void OnservicesListDeleteItem(wxListEvent& event);
		void OnservicesListBeginLabelEdit(wxListEvent& event);
		void OnservicesListEndLabelEdit(wxListEvent& event);
		void OnButtonTestClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
