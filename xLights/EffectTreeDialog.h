#ifndef EFFECTTREEDIALOG_H
#define EFFECTTREEDIALOG_H

//(*Headers(EffectTreeDialog)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)
#include <wx/xml/xml.h>
#include <wx/filename.h>
#include "xLightsMain.h"

#define NCCOM_FILE wxT("nutcracker_com_effects.xml")

class EffectTreeDialog : public wxDialog
{
	public:

		EffectTreeDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectTreeDialog();

		//(*Declarations(EffectTreeDialog)
		wxButton* btNewPreset;
		wxButton* btFavorite;
		wxButton* btAddGroup;
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
		static const long ID_BUTTON7;
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
		void OnbtAddGroupClick(wxCommandEvent& event);
		void OnTreeCtrl1ItemActivated(wxTreeEvent& event);
		void OnButton_OKClick(wxCommandEvent& event);
		//*)
        wxWindow* xLightParent;
		wxXmlNode *NcEffectsNode;
		wxXmlNode *XrgbEffectsNode;
		wxXmlDocument NcEffectsXml;
        void AddNCcomEffects();
        void AddTreeElementsRecursive(wxXmlNode *EffectsNode, wxTreeItemId curGroupID);
        void UpdateNcEffectsList();
        bool CheckValidOperation(wxTreeItemId itemID);
        wxXmlNode* CreateEffectGroupNode(wxString& name);
        void FixupEffectsPresets(wxXmlNode *UserGroupNode);
        void ApplyEffect(bool dblClick=false);
        void SaveEffectsFile();

		DECLARE_EVENT_TABLE()

    public:
        static bool PromptForName(wxWindow* parent, wxString *name, wxString prompt, wxString errorMsg); //static to allow re-use elsewhere -DJ
};

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(wxXmlNode* desc, bool isGroup=false) {element=desc; _isGroup=isGroup; }

    wxXmlNode *GetElement() { return element; }
    bool IsGroup() { return _isGroup; }
private:
    wxXmlNode *element;
    bool _isGroup;
};

#endif
