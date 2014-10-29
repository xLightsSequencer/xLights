#ifndef WHOLEHOUSEMODELNAMEDIALOG_H
#define WHOLEHOUSEMODELNAMEDIALOG_H

//(*Headers(WholeHouseModelNameDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
//*)

class WholeHouseModelNameDialog: public wxDialog
{
	public:

		WholeHouseModelNameDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~WholeHouseModelNameDialog();

		//(*Declarations(WholeHouseModelNameDialog)
		wxStaticText* StaticText1;
		wxTextCtrl* Text_WholehouseModelName;
		//*)

	protected:

		//(*Identifiers(WholeHouseModelNameDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXT_WHOLE_HOUSE_MODEL_NAME;
		//*)

	private:

		//(*Handlers(WholeHouseModelNameDialog)
		void OnInit(wxInitDialogEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
