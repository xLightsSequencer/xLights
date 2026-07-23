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

#include <wx/panel.h>
#include <wx/regex.h>
#include <wx/sizer.h>
#include <wx/treelist.h>

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "layout/ControllerTreeUtils.h"

using namespace ControllerTree;

class xLightsFrame;
class LayoutPanel;
class Controller;
class wxDataViewEvent;
class ControllerPropertyAdapter;
class xlPropertyGrid;
class wxButton;
class wxPropertyGridEvent;
class wxStaticText;
class wxStaticBitmap;
class wxSearchCtrl;
class wxWrapSizer;

class ControllerListPanel : public wxPanel {
public:
    ControllerListPanel(wxWindow* parent, xLightsFrame* frame, LayoutPanel* layoutPanel);
    virtual ~ControllerListPanel();

    void UpdateControllerList();
    void RefreshStatusColumn();
    std::list<std::string> GetSelectedControllerNames() const;
    Controller* GetFirstSelectedController() const;
    void SelectController(const std::string& name);
    wxTreeListCtrl* GetTree() const { return _tree; }
    void SaveColumnOrder();
    wxWindow* CreatePropertiesPanel(wxWindow* parent);
    void UpdateControllerProperties();
    void ClearPreviewHighlights();

private:
    void OnControllerPropertyGridChange(wxPropertyGridEvent& event);
    void OnControllerPropertyGridCollapsed(wxPropertyGridEvent& event);
    void OnControllerPropertyGridExpanded(wxPropertyGridEvent& event);
    void CreateTree();
    int ColIndex(const wxString& title) const;
    void OnSelectionChanged(wxTreeListEvent& event);
    void OnItemActivated(wxTreeListEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void ExpandAllControllers();
    void CollapseAll();
    wxTreeListItem TopLevelItem(wxTreeListItem item) const;
    void RefreshPingIndicator(const Controller* controller);
    void PopulatePorts(wxTreeListItem ctrlItem);
    void OnItemExpanding(wxTreeListEvent& event);
    void UpdatePreviewHighlights();
    void OnBeginDrag(wxDataViewEvent& event);
    void OnDragPossible(wxDataViewEvent& event);
    void OnDragDrop(wxDataViewEvent& event);
    void OnPopup(wxCommandEvent& event);
    void OnCharHook(wxKeyEvent& event);
    void DeleteSelectedControllers();
    void ActivateSelectedControllers(const std::string& active);
    void UnlinkSelectedControllers();
    bool NetworkChangesAllowed() const;
    void OnControllerFilterTextChanged(wxCommandEvent& event);
    void OnControllerFilterCancelBtn(wxCommandEvent& event);
    bool ControllerMatchesFilter(const Controller* controller) const;
    void OnFullColumnsClick(wxCommandEvent& event);

    xLightsFrame* _frame = nullptr;
    LayoutPanel* _layoutPanel = nullptr;
    wxTreeListCtrl* _tree = nullptr;
    std::vector<int> _colIdx;

    wxSearchCtrl* _controllerFilterCtrl = nullptr;
    wxString _controllerFilterString;
    wxRegEx  _controllerFilterRegex;
    bool     _controllerFilterRegexValid = false;
    wxButton* _btnFullColumns = nullptr;
    ControllerTree::ControllerLedIcons _ledIcons;

    xlPropertyGrid* _propGrid = nullptr;
    std::unique_ptr<ControllerPropertyAdapter> _adapter;
    wxPanel* _propsPanel = nullptr;
    wxStaticText* _noSelectionHint = nullptr;
    wxStaticBitmap* _ledPing = nullptr;
    ControllerTree::ControllerLedBitmaps _ledPingBitmaps;
    wxButton* _btnAddController = nullptr;
    wxButton* _btnVisualise = nullptr;
    wxButton* _btnUploadInput = nullptr;
    wxButton* _btnUploadOutput = nullptr;
    wxButton* _btnOpen = nullptr;
    wxButton* _btnOpenProxy = nullptr;
    wxButton* _btnFPPConnect = nullptr;
    wxButton* _btnDiscover = nullptr;

    static const long ID_TREELIST_CONTROLLERS;
};
