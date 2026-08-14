#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(ImportPreviewsModelsDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

#include <wx/treelist.h>
#include <wx/srchctrl.h>
#include <wx/timer.h>
#include <pugixml.hpp>

#include <set>
#include <string>

#include "models/ModelManager.h"

class LayoutGroup;

enum class ImpItemKind { Model, ModelGroup, Viewpoint };

class impTreeItemData : public wxClientData
{
    pugi::xml_node _modelNode;
    ImpItemKind _kind;
    const wxString _name;
public:
    impTreeItemData(wxString name, pugi::xml_node n, ImpItemKind kind) : _modelNode(n), _kind(kind), _name(name)
    {}
    wxString GetName() const { return _name; };
    pugi::xml_node GetModelNode() const { return _modelNode; }
    bool IsModelGroup() const { return _kind == ImpItemKind::ModelGroup; }
    bool IsViewpoint() const { return _kind == ImpItemKind::Viewpoint; }
    ImpItemKind GetKind() const { return _kind; }
};

class ImportPreviewsModelsDialog: public wxDialog
{
    wxTreeListCtrl* TreeListCtrl1;
    pugi::xml_document _doc;
    wxTreeListItem _item;
    ModelManager& _allModels;
    const std::map<std::string, std::unique_ptr<LayoutGroup>>& _layoutGroups;

    void ValidateWindow();
    void PopulateTree();
    void AddModels(wxTreeListCtrl* tree, wxTreeListItem item, pugi::xml_node models, pugi::xml_node modelgroups, wxString preview, const wxString& filter);
    void AddViewpoints(wxTreeListCtrl* tree, wxTreeListItem item, pugi::xml_node viewpoints, const wxString& filter);
    static bool MatchesFilter(const wxString& name, const wxString& filterLower);
    // MatchesFilter, plus rows the user has already ticked - those stay in the
    // tree so a selection assembled across several filter terms remains visible.
    bool KeepInFilteredTree(const wxString& name, ImpItemKind kind, const wxString& filterLower) const;
    bool IsViewpointsRow(wxTreeListItem it) const;
    // Filtering rebuilds the tree, so checked state is kept in these sets
    // (which survive filtered-out rows) and synced to/from the visible tree.
    void SyncCheckedFromTree();
    void RestoreChecksToTree();
    void ClearFilter();
    void SelectAll(bool checked);
    void SelectHighlighted(bool checked);
    void SelectRecursiveModel(wxString m, bool checked);
    void SelectSiblings(wxTreeListItem item, bool checked);
    void ExpandAll(bool expand);
    void DeselectExistingModels();
    void SelectAllModel(bool checked);
    void SelectAllModelGroups(bool checked);
    void SelectAllViewpoints(bool checked);
    bool ModelExists(const std::string& modelName) const;
    bool LayoutExists(const std::string& layoutName) const;

	public:

		ImportPreviewsModelsDialog(wxWindow* parent, const wxString& filename, ModelManager& allModels, const std::map<std::string, std::unique_ptr<LayoutGroup>>& layoutGroups, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ImportPreviewsModelsDialog();
        bool GetIncludeEmptyGroups() const;
        wxArrayString GetPreviews() const;
        std::list<impTreeItemData*> GetModelsInPreview(wxString preview) const;
        std::list<impTreeItemData*> GetViewpoints() const;
        float GetSourceRulerPerUnit() const;
		//(*Declarations(ImportPreviewsModelsDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_IncludeEmptyModelGroups;
		wxFlexGridSizer* FlexGridSizer2;
		//*)

	protected:

		//(*Identifiers(ImportPreviewsModelsDialog)
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

        static const long ID_MNU_IPM_EXPANDALL;
        static const long ID_MNU_IPM_COLLAPSEALL;
        static const long ID_MNU_IPM_SELECTALL;
        static const long ID_MNU_IPM_DESELECTALL;
        static const long ID_MNU_IPM_SELECTHIGH;
        static const long ID_MNU_IPM_DESELECTHIGH;
        static const long ID_MNU_IPM_SELECTSIBLINGS;
        static const long ID_MNU_IPM_DESELECTSIBLINGS;
        static const long ID_MNU_IPM_DESELECTEXISTING;
        static const long ID_MNU_IPM_SELECTALLMODELS;
        static const long ID_MNU_IPM_SELECTALLMODELSGROUPS;
        static const long ID_MNU_IPM_SELECTALLVIEWPOINTS;

	private:

		//(*Handlers(ImportPreviewsModelsDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		//*)

        void OnContextMenu(wxTreeListEvent& event);
        void OnListPopup(wxCommandEvent& event);
        void OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event);

        struct CheckedModel {
            std::string name;
            ImpItemKind kind;
            bool operator<(const CheckedModel& o) const {
                return kind != o.kind ? kind < o.kind : name < o.name;
            }
        };

        wxSearchCtrl* _filterCtrl = nullptr;
        wxString _filter;        // lower-cased; whitespace-tokenised AND match
        wxString _appliedFilter; // what the tree currently shows; lags _filter by the debounce
        wxTimer _filterTimer;    // debounce tree rebuilds while typing
        std::set<CheckedModel> _checkedModels;
        std::set<std::string> _checkedPreviews; // checked preview/layout-group rows
        bool _viewpointsRootChecked = false; // the Viewpoints row's own checkbox

		DECLARE_EVENT_TABLE()
};
