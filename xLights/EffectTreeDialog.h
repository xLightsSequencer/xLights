#ifndef EFFECTTREEDIALOG_H
#define EFFECTTREEDIALOG_H

//(*Headers(EffectTreeDialog)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#include <wx/xml/xml.h>
#include "xLightsMain.h"

#define NCCOM_FILE wxT("nutcracker_com_effects.xml")

class EffectTreeDialog: public wxDialog
{
	public:

		EffectTreeDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectTreeDialog();

		//(*Declarations(EffectTreeDialog)
		wxButton* btNewPreset;
		wxButton* btFavorite;
		wxButton* btUpdate;
		wxButton* btApply;
		wxButton* btRename;
		wxTreeCtrl* TreeCtrl1;
		wxButton* btDelete;
		//*)
        wxTreeItemId treeRootID;
        wxTreeItemId treeFavoritesGroupID;
        wxTreeItemId treeUserGroupID;
        wxTreeItemId treeNCcomGroupID;
        void InitItems(wxXmlNode *e);
	protected:

		//(*Identifiers(EffectTreeDialog)
		static const long ID_TREECTRL1;
		static const long ID_BUTTON6;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON5;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		//*)

	private:

		//(*Handlers(EffectTreeDialog)
		void OnbtApplyClick(wxCommandEvent& event);
		void OnbtNewPresetClick(wxCommandEvent& event);
		void OnbtUpdateClick(wxCommandEvent& event);
		void OnbtFavoriteClick(wxCommandEvent& event);
		void OnbtRenameClick(wxCommandEvent& event);
		void OnbtDeleteClick(wxCommandEvent& event);
		//*)
        wxWindow* xLightParent;
		wxXmlNode *NcEffectsNode;
		wxXmlNode *XrgbEffectsNode;
        void AddNCcomEffects();
        void AddTreeElementsRecursive(wxXmlNode *EffectsNode, wxTreeItemId curGroupID);
        void EffectTreeDialog::UpdateNcEffectsList();

		DECLARE_EVENT_TABLE()

};

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(wxXmlNode* desc) {element=desc; }

    wxXmlNode *GetElement() { return element; }

private:
    wxXmlNode *element;
};

#endif
