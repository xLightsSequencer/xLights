#ifndef EFFECTDIALOGV2_H
#define EFFECTDIALOGV2_H

//(*Headers(EffectDialogv2)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include "scrollingdialog.h"
//*)

class EffectDialogv2: public wxScrollingDialog
{
	public:

		EffectDialogv2(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectDialogv2();

		//(*Declarations(EffectDialogv2)
		wxButton* Button4;
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxTreeCtrl* TreeCtrl1;
		//*)

        void EffectDialogv2::PrepItems(wxXmlNode* e);
        wxTreeItemId treeRootID;

	protected:

		//(*Identifiers(EffectDialogv2)
		static const long ID_TREECTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(EffectDialogv2)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
