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

#include <wx/dnd.h>

//(*Headers(ViewsModelsPanel)
#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include "wxCheckedListCtrl.h"
#include "SequenceData.h"
#include <list>
#include <map>

class wxXmlNode;
class SequenceElements;
class xLightsFrame;
class Element;
class wxChoice;
class SequenceViewManager;

wxDECLARE_EVENT(EVT_VMDROP, wxCommandEvent);

class MyTextDropTarget : public wxTextDropTarget
{
public:
    MyTextDropTarget(wxWindow *owner, wxListCtrl* list, wxString type) { _owner = owner; _list = list; _type = type; };

    virtual bool OnDropText(wxCoord x, wxCoord y,
        const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow *_owner;
    wxListCtrl* _list;
    wxString _type;
};

class ViewsModelsPanel : public wxPanel
{
    wxChoice* _mainViewsChoice = nullptr;
    xLightsFrame* _xlFrame = nullptr;
    SequenceData* _seqData = nullptr;
    SequenceElements* _sequenceElements = nullptr;
    wxXmlNode* _models = nullptr;
    wxXmlNode* _views = nullptr;
    wxXmlNode* _modelGroups = nullptr;
    SequenceViewManager* _sequenceViewManager = nullptr;
    wxImageList* _imageList = nullptr;
    int _numViews = 0;
    int _numModels = 0;
    int _numNonModels = 0;
    int _sortOrder = 0;
    bool _dragRowModel = false;
    bool _dragRowNonModel = false;
    std::list<std::string> _undo;

    void PopulateViews();
    void SortNonModels();
    void PopulateModels(const std::string& selectModels = "");
    void ValidateWindow();
    void AddSelectedModels(int pos = -1);
    void RemoveSelectedModels();
    void AddTimingToList(Element* element);
    void AddTimingToNotList(Element* element);
    void AddModelToList(Element* element);
    void AddModelToNotList(Element* element);
    int GetViewIndex(const wxString& name);
    void MarkViewsChanged();
    void AddViewToList(const wxString& viewName, bool isChecked);
    bool IsModelAGroup(const std::string& modelname) const;
    int GetPixelCount(const std::string& modelname);
    void DeleteSelectedView();
    int GetTimingCount();
    static bool IsItemSelected(wxListCtrl* ctrl, int item);
    static bool SelectItem(wxListCtrl* ctrl, int item, bool select);
    static bool SelectItem(wxListCtrl* ctrl, const std::string& itemText, int col, bool select);
    void ShowAllModels(bool show);
    void HideUnusedModels();
    void RemoveUnusedModels();
    void SelectUnusedModels();
    void SelectUsedModels();
    void SelectAllModels();
    void SortModelsByName();
    void SortModelsByNameGM(bool sortGroupsBySize = false, bool sortGroupsByCount = false);
    void SortModelsByCPGM(bool sortGroupsBySize = false);
    void SortModelsBySCGM(bool sortGroupsBySize = false);
    void SortModelsByMasterView();
    void SortModelsByType();
    void SortModelsUnderThisGroup(int groupIndex);
    void SortModelsBubbleUpGroups();
    void RenameView(int itemIndex);
    wxArrayString MergeStringArrays(const wxArrayString& arr1, const wxArrayString& arr2);
    void SetMasterViewModels(const wxArrayString& models);
    std::string GetModelType(const std::string& modelname) const;
    wxArrayString GetGroupModels(const std::string& group) const;
    std::string GetModelCPSortString(const std::string& modelname) const;
    uint32_t GetModelSC(const std::string& modelname) const;
    wxString GetMasterViewModels() const;
    void SaveUndo();
    void Undo();
    void ClearUndo();
    void MoveSelectedModelsTo(int indexTo);
    size_t GetSelectedItemCount() const;
    size_t GetSelectedModelCount() const;
    bool GetSelectionIsMixed() const;
    void ImportRGBEffectsView();
    void ImportSequenceMasterView();

    void ImportViewData(std::map<wxString, wxArrayString> const& views, wxArrayString const& timings);

    wxString CreateUniqueName(wxString const& prefix);

public:

    ViewsModelsPanel(xLightsFrame* frame, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~ViewsModelsPanel();

    void Initialize();
    void Clear();
    void SetViewChoice(wxChoice* choice);
    void SelectView(const std::string& view);
    bool HasView(const std::string& view);
    void SetSequenceElementsModelsViews(SequenceData* seqData, SequenceElements* sequenceElements, wxXmlNode* modelsNode, wxXmlNode* modelGroupsNode, SequenceViewManager* sequenceViewManager);
    void OnViewSelect(wxCommandEvent& event);
    void OnListCtrlItemCheck(wxCommandEvent& event);
    void UpdateModelsForSelectedView();
    void RemoveModelFromLists(const std::string& modelName);
    void DoMakeMaster();

    //(*Declarations(ViewsModelsPanel)
    wxButton* ButtonClone;
    wxButton* ButtonImport;
    wxButton* ButtonRename;
    wxButton* Button_AddAll;
    wxButton* Button_AddSelected;
    wxButton* Button_AddView;
    wxButton* Button_Bottom;
    wxButton* Button_DeleteView;
    wxButton* Button_MakeMaster;
    wxButton* Button_MoveDown;
    wxButton* Button_MoveUp;
    wxButton* Button_RemoveAll;
    wxButton* Button_RemoveSelected;
    wxButton* Button_Top;
    wxButton* Button_ViewDown;
    wxButton* Button_ViewUp;
    wxCheckedListCtrl* ListCtrlModels;
    wxCheckedListCtrl* ListCtrlViews;
    wxListCtrl* ListCtrlNonModels;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText3;
    //*)

protected:

    //(*Identifiers(ViewsModelsPanel)
    static const wxWindowID ID_BUTTON3;
    static const wxWindowID ID_BUTTON4;
    static const wxWindowID ID_BUTTON5;
    static const wxWindowID ID_BUTTON6;
    static const wxWindowID ID_BUTTON_TOP;
    static const wxWindowID ID_BUTTON9;
    static const wxWindowID ID_BUTTON10;
    static const wxWindowID ID_BUTTON_BOTTOM;
    static const wxWindowID ID_BUTTON1;
    static const wxWindowID ID_BUTTON2;
    static const wxWindowID ID_BUTTON7;
    static const wxWindowID ID_BUTTON8;
    static const wxWindowID ID_BUTTON_IMPORT;
    static const wxWindowID ID_BUTTON11;
    static const wxWindowID ID_BUTTON_VIEWUP;
    static const wxWindowID ID_BUTTON_VIEWDOWN;
    static const wxWindowID ID_STATICTEXT1;
    static const wxWindowID ID_LISTCTRL_VIEWS;
    static const wxWindowID ID_STATICTEXT2;
    static const wxWindowID ID_LISTCTRL_MODELS;
    static const wxWindowID ID_LISTCTRL1;
    static const wxWindowID ID_STATICTEXT3;
    //*)

    static const long ID_MODELS_UNDO;
    static const long ID_MODELS_HIDEALL;
    static const long ID_MODELS_SHOWALL;
    static const long ID_MODELS_SELECTALL;
    static const long ID_MODELS_HIDEUNUSED;
    static const long ID_MODELS_REMOVEUNUSED;
    static const long ID_MODELS_SELECTUNUSED;
    static const long ID_MODELS_SELECTUSED;
    static const long ID_MODELS_SORT;
    static const long ID_MODELS_SORTBYNAME;
    static const long ID_MODELS_SORTBYNAMEGM;
    static const long ID_MODELS_SORTBYCPGM;
    static const long ID_MODELS_SORTBYSCGM;
    static const long ID_MODELS_SORTBYTYPE;
    static const long ID_MODELS_SORTMODELSUNDERTHISGROUP;
    static const long ID_MODELS_BUBBLEUPGROUPS;
    static const long ID_MODELS_SORTBYNAMEGMSIZE;
    static const long ID_MODELS_SORTBYNAMEGMCOUNT;
    static const long ID_MODELS_SORTBYCPGMSIZE;
    static const long ID_MODELS_SORTBYSCGMSIZE;
    static const long ID_MODELS_SORTBYMASTERVIEW;
    static const long VIEWSMODELS_IMPORT_RGBEFFECTS;
    static const long VIEWSMODELS_IMPORT_SEQUENCE;

private:

    //(*Handlers(ViewsModelsPanel)
    void OnListView_ViewItemsBeginDrag(wxListEvent& event);
    void OnListView_ViewItemsItemSelect(wxListEvent& event);
    void OnListView_ViewItemsKeyDown(wxListEvent& event);
    void OnButton_AddViewClick(wxCommandEvent& event);
    void OnButton_DeleteViewClick(wxCommandEvent& event);
    void OnButton_AddAllClick(wxCommandEvent& event);
    void OnButton_AddSelectedClick(wxCommandEvent& event);
    void OnButton_RemoveSelectedClick(wxCommandEvent& event);
    void OnButton_RemoveAllClick(wxCommandEvent& event);
    void OnListCtrlViewsItemSelect(wxListEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnListCtrlNonModelsItemSelect(wxListEvent& event);
    void OnListCtrlNonModelsBeginDrag(wxListEvent& event);
    void OnListCtrlNonModelsKeyDown(wxListEvent& event);
    void OnListCtrlViewsKeyDown(wxListEvent& event);
    void OnListCtrlModelsItemRClick(wxListEvent& event);
    void OnButtonCloneClick(wxCommandEvent& event);
    void OnButtonRenameClick(wxCommandEvent& event);
    void OnListCtrlViewsItemDClick(wxListEvent& event);
    void OnButton_MoveDownClick(wxCommandEvent& event);
    void OnButton_MoveUpClick(wxCommandEvent& event);
    void OnButton_MakeMasterClick(wxCommandEvent& event);
    void OnListCtrlNonModelsColumnClick(wxListEvent& event);
    void OnListCtrlNonModelsItemActivated(wxListEvent& event);
    void OnListCtrlModelsItemActivated(wxListEvent& event);
    void OnButton_TopClick(wxCommandEvent& event);
    void OnButtonImportClick(wxCommandEvent& event);
    void OnButton_BottomClick(wxCommandEvent& event);
    void OnButtonView_UpClick(wxCommandEvent& event);
    void OnButtonView_DownClick(wxCommandEvent& event);
    //*)

    void OnDrop(wxCommandEvent& event);
    void OnModelsPopup(wxCommandEvent& event);
    void OnImportBtnPopup(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};
