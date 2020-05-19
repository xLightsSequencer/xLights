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

//(*Headers(ImportPreviewsModelsDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

#include <wx/treelist.h>
#include <wx/xml/xml.h>

#include "models/ModelManager.h"

class LayoutGroup;

class impTreeItemData : public wxClientData
{
    wxXmlNode* _modelNode;
    bool _modelGroup;
    const wxString _name;
public:
    impTreeItemData(wxString name, wxXmlNode* n, bool mg) : _name(name), _modelNode(n), _modelGroup(mg)
    {}
    wxString GetName() const { return _name; };
    wxXmlNode* GetModelXml() const { return _modelNode; }
    bool IsModelGroup() const { return _modelGroup; }
};

class ImportPreviewsModelsDialog: public wxDialog
{
    wxTreeListCtrl* TreeListCtrl1;
    wxXmlDocument _doc;
    wxTreeListItem _item;
    ModelManager& _allModels;
    std::vector<LayoutGroup*>& _layoutGroups;

    void ValidateWindow();
    void AddModels(wxTreeListCtrl* tree, wxTreeListItem item, wxXmlNode* models, wxXmlNode* modelgroups, wxString preview);
    void SelectAll(bool checked);
    void SelectSiblings(wxTreeListItem item, bool checked);
    void ExpandAll(bool expand);
    void DeselectExistingModels();
    void SelectAllModel(bool checked);
    void SelectAllModelGroups(bool checked);
    bool ModelExists(const std::string& modelName) const;
    bool LayoutExists(const std::string& layoutName) const;

	public:

		ImportPreviewsModelsDialog(wxWindow* parent, const wxString& filename, ModelManager& allModels, std::vector<LayoutGroup*>& layoutGroups, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ImportPreviewsModelsDialog();
        wxArrayString GetPreviews() const;
        std::list<impTreeItemData*> GetModelsInPreview(wxString preview) const;
		//(*Declarations(ImportPreviewsModelsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxFlexGridSizer* FlexGridSizer2;
		//*)

	protected:

		//(*Identifiers(ImportPreviewsModelsDialog)
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

        static const long ID_MNU_IPM_EXPANDALL;
        static const long ID_MNU_IPM_COLLAPSEALL;
        static const long ID_MNU_IPM_SELECTALL;
        static const long ID_MNU_IPM_DESELECTALL;
        static const long ID_MNU_IPM_SELECTSIBLINGS;
        static const long ID_MNU_IPM_DESELECTSIBLINGS;
        static const long ID_MNU_IPM_DESELECTEXISTING;
        static const long ID_MNU_IPM_SELECTALLMODELS;
        static const long ID_MNU_IPM_SELECTALLMODELSGROUPS;

	private:

		//(*Handlers(ImportPreviewsModelsDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

        void OnContextMenu(wxTreeListEvent& event);
        void OnListPopup(wxCommandEvent& event);
        void OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event);

		DECLARE_EVENT_TABLE()
};
