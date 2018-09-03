#ifndef MODELCHAINDIALOG_H
#define MODELCHAINDIALOG_H

//(*Headers(ModelChainDialog)
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <string>
class ModelManager;

class ModelChainDialog: public wxDialog
{
	public:

		ModelChainDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelChainDialog();

		//(*Declarations(ModelChainDialog)
		wxChoice* ModelChoice;
		wxStaticText* StaticText1;
		//*)
    
        void Set(const wxString &chain, const ModelManager &models);
        std::string Get();

	protected:

		//(*Identifiers(ModelChainDialog)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		//*)

	private:

		//(*Handlers(ModelChainDialog)
		//*)

		DECLARE_EVENT_TABLE()
};
#endif