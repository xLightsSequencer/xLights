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

 //(*Headers(ModelChainDialog)
 #include <wx/choice.h>
 #include <wx/dialog.h>
 #include <wx/sizer.h>
 #include <wx/stattext.h>
 //*)

#include <string>

class ModelManager;
class Model;

class ModelChainDialog: public wxDialog
{
	public:

		ModelChainDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelChainDialog();

		//(*Declarations(ModelChainDialog)
		wxChoice* ModelChoice;
		wxStaticText* StaticText1;
		//*)
    
        void Set(Model* m, const ModelManager &models);
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

