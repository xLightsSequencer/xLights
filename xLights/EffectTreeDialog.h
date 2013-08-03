#ifndef EFFECTTREEDIALOG_H
#define EFFECTTREEDIALOG_H

//(*Headers(EffectTreeDialog)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#include <wx/xml/xml.h>

class EffectTreeDialog: public wxDialog
{
	public:

		EffectTreeDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectTreeDialog();

		//(*Declarations(EffectTreeDialog)
		wxButton* Button4;
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button5;
		wxButton* Button3;
		wxTreeCtrl* TreeCtrl1;
		//*)
        wxTreeItemId treeRootID;
        wxTreeItemId treeFavoritesGroupID;
        wxTreeItemId treeUserGroupID;
        wxTreeItemId treeNCcomGroupID;
        void InitItems(wxXmlNode *e);
	protected:

		//(*Identifiers(EffectTreeDialog)
		static const long ID_TREECTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		//*)

	private:

		//(*Handlers(EffectTreeDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
