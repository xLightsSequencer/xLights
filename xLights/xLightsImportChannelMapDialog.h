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

// need to do these manually due to issues with wxSmith
#include <wx/treelist.h>
#include <wx/treectrl.h>
#include <wx/dataview.h>

//(*Headers(xLightsImportChannelMapDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <map>
#include <vector>
#include "Color.h"
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <list>
#include <memory>
#include <optional>
#include "SequencePackage.h"

class SequenceElements;
class xLightsFrame;
class Model;
class wxProgressDialog;

wxDECLARE_EVENT(EVT_MDDROP, wxCommandEvent);

class xLightsImportModelNode;
WX_DEFINE_ARRAY_PTR(xLightsImportModelNode*, xLightsImportModelNodePtrArray);

class MDTextDropTarget : public wxTextDropTarget
{
public:
    MDTextDropTarget(wxWindow *owner, wxListCtrl* list, const wxString &type) { _owner = owner; _list = list; _tree = nullptr; _type = type; };
    MDTextDropTarget(wxWindow *owner, wxDataViewCtrl* tree, const wxString &type) { _owner = owner; _list = nullptr; _tree = tree; _type = type; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow *_owner;
    wxListCtrl* _list;
    wxDataViewCtrl* _tree;
    wxString _type;
};

class xLightsImportModelNode : wxDataViewTreeStoreNode
{
public:
    xLightsImportModelNode(xLightsImportModelNode* parent,
                           const wxString& model, const wxString& strand, const wxString& node,
                           const wxString& mapping, const bool mappingExists, 
                           const std::list<std::string> aliases, const std::string& modelType, 
                           const std::string& groupModels,
                           bool isSubmodel, const std::string& modelClass, int nodeCount, const wxColor& color = *wxWHITE,
                           const wxString& mappingModelType = "", const int effectCount = 0) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model.ToStdString()),
        _strand(strand.ToStdString()),
        _node(node.ToStdString()),
        _mapping(mapping.ToStdString()),
        _color(color),
        _group(false),
        _mappingExists(mappingExists),
        _aliases(aliases),
        _modelType(modelType),
        m_container(false),
        _groupModels(groupModels),
        _isSubmodel(isSubmodel),
        _modelClass(modelClass),
        _nodeCount(nodeCount),
        _effectCount(effectCount),
        _mappingModelType(mappingModelType.ToStdString()) {
    }

    xLightsImportModelNode(xLightsImportModelNode* parent,
                           const wxString& model, const wxString& strand,
                           const wxString& mapping, const bool mappingExists, 
                           const std::list<std::string> aliases, const std::string& modelType, 
                           const std::string& groupModels, 
                           bool isSubmodel, const std::string& modelClass, int nodeCount, const wxColor& color = *wxWHITE,
                           const wxString& mappingModelType = "", const int effectCount = 0) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model.ToStdString()),
        _strand(strand.ToStdString()),
        _node(),
        _mapping(mapping),
        _color(color),
        _group(false),
        _mappingExists(mappingExists),
        _aliases(aliases),
        _modelType(modelType),
        m_container(true),
        _groupModels(groupModels),
        _isSubmodel(isSubmodel),
        _modelClass(modelClass),
        _nodeCount(nodeCount),
        _effectCount(effectCount),
        _mappingModelType(mappingModelType.ToStdString())
    { }

    xLightsImportModelNode(xLightsImportModelNode* parent,
                           const wxString &model,
                           const wxString& mapping, const bool mappingExists, 
                           const std::list<std::string> aliases, const std::string& modelType,
                           const std::string& groupModels,
                           bool isSubmodel, const std::string& modelClass, int nodeCount, const wxColor& color = *wxWHITE,
                           const bool isGroup = false,
                           const wxString& mappingModelType = "", const int effectCount = 0) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model.ToStdString()),
        _strand(),
        _node(),
        _mapping(mapping.ToStdString()),
        _color(color),
        _group(isGroup),
        _mappingExists(mappingExists),
        _aliases(aliases),
        _modelType(modelType),
        m_container(!isGroup),
        _groupModels(groupModels),
        _isSubmodel(isSubmodel),
        _modelClass(modelClass),
        _nodeCount(nodeCount),
        _effectCount(effectCount),
        _mappingModelType(mappingModelType.ToStdString())
    { }

    ~xLightsImportModelNode()
    {
        // free all our children nodes
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; ++i) {
            xLightsImportModelNode *child = m_children[i];
            delete child;
        }
    }

    void ClearMapping()
    {
        _mappingExists = true;
        _mapping = "";
        _color = *wxWHITE;
        _mappingModelType = "";
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; ++i) {
            GetNthChild(i)->ClearMapping();
        }
    }

    bool IsGroup() const { return _group; }

    std::list<std::string> GetAliases() const {
        return _aliases;
    }

    std::string GetModelType() const {
        return _modelType;
    }

    void Map(const std::string& mapTo, const std::string& mappingModelType)
    {
        _mapping = mapTo;
        _mappingExists = true;
        _mappingModelType = mappingModelType;
    }

    // This also considers children
    bool HasMapping() {
        if (!_mapping.empty()) {
            return true;
        } else {
            for (size_t i = 0; i < m_children.size(); ++i) {
                xLightsImportModelNode* c = GetNthChild(i);
                if (c->HasMapping()) {
                    return true;
                }
            }
        }
        return false;
    }

    // This just considers this node
    bool IsMapped() const
    {
        return !_mapping.empty();
    }

    bool IsContainer() wxOVERRIDE {
        return m_container;
    }

    xLightsImportModelNode* GetParent() const
    {
        return m_parent;
    }
    xLightsImportModelNodePtrArray& GetChildren()
    {
        return m_children;
    }
    xLightsImportModelNode* GetNthChild(unsigned int n)
    {
        return m_children.Item(n);
    }
    void Insert(xLightsImportModelNode* child, unsigned int n)
    {
        m_children.Insert(child, n);
    }
    void Append(xLightsImportModelNode* child)
    {
        m_children.Add(child);
    }
    unsigned int GetChildCount() const
    {
        return m_children.GetCount();
    }

    std::string GetModelName() const {
        std::string name = _model;
        if (!_strand.empty()) {
            name += "/" + _strand;
        }
        if (!_node.empty()) {
            name += "/" + _node;
        }
        return name;
    }

    bool IsSubModel() const {
        return _strand != "" && _isSubmodel;
    }

    bool IsStrand() const {
        return _strand != "" && !_isSubmodel;
    }

    bool IsNode() const {
        return _node != "";
    }

public:     // public to avoid getters/setters
    std::string                 _model;
    std::string                 _strand;
    std::string                 _node;
    std::string                 _mapping;
    wxColor                     _color;
    bool                        _group = false;
    bool                        _mappingExists = false;
    std::list<std::string> _aliases;
    std::string _modelType;
    std::string _groupModels;
    bool _isSubmodel = false;
    std::string _modelClass;
    int _nodeCount = 0;
    int _effectCount = 0;
    std::string _mappingModelType;

    // TODO/FIXME:
    // the GTK version of wxDVC (in particular wxDataViewCtrlInternal::ItemAdded)
    // needs to know in advance if a node is or _will be_ a container.
    // Thus implementing:
    //   bool IsContainer() const
    //    { return m_children.GetCount()>0; }
    // doesn't work with wxGTK when MyMusicTreeModel::AddToClassical is called
    // AND the classical node was removed (a new node temporary without children
    // would be added to the control)
    bool m_container;

private:
    xLightsImportModelNode          *m_parent;
    xLightsImportModelNodePtrArray   m_children;
};

class xLightsImportTreeModel : public wxDataViewModel
{
public:
    xLightsImportTreeModel();
    ~xLightsImportTreeModel()
    {
        // free all our children nodes
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; ++i) {
            xLightsImportModelNode *child = m_children[i];
            delete child;
        }
    }

    bool GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const override;
    int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const override;

    void SetMappingExists(const wxDataViewItem &item, bool exists);
    void Insert(xLightsImportModelNode* child, unsigned int n)
    {
        m_children.Insert(child, n);
        ItemAdded(wxDataViewItem(0), wxDataViewItem(child));
    }

    void BulkInsert(xLightsImportModelNode* child, unsigned int n)
    {
        m_children.Insert(child, n);
        _pendingAdditions.Add(wxDataViewItem(child));
    }

    void Append(xLightsImportModelNode* child)
    {
        m_children.Add(child);
        ItemAdded(wxDataViewItem(0), wxDataViewItem(child));
    }
    void ClearMapping();
    unsigned int GetChildCount() const
    {
        return m_children.GetCount();
    }
    unsigned int GetMappedChildCount() const
    {
        size_t count = 0;
        for (size_t i = 0; i < m_children.size(); ++i) {
            xLightsImportModelNode* c = GetNthChild(i);
            if (c->HasMapping()) {
                count++;
            }
        }
        return count;
    }
    xLightsImportModelNodePtrArray& GetChildren()
    {
        return m_children;
    }
    xLightsImportModelNode* GetNthChild(unsigned int n) const
    {
        return m_children.Item(n);
    }
    wxDataViewItem GetNthItem(unsigned int n) const;
    wxString GetModel(const wxDataViewItem &item) const;
    wxString GetStrand(const wxDataViewItem &item) const;
    wxString GetNode(const wxDataViewItem &item) const;
    wxString GetMapping(const wxDataViewItem &item) const;
    wxColor GetColor(const wxDataViewItem &item) const;
    void Delete(const wxDataViewItem &item);
    virtual unsigned int GetColumnCount() const wxOVERRIDE
    {
        return 2;
    }
    virtual bool HasContainerColumns(const wxDataViewItem &item) const wxOVERRIDE
    {
        return true;
    }
    virtual wxString GetColumnType(unsigned int col) const wxOVERRIDE
    {
        return wxT("string");
    }

    virtual void GetValue(wxVariant &variant,
        const wxDataViewItem &item, unsigned int col) const wxOVERRIDE;
    virtual bool SetValue(const wxVariant &variant,
        const wxDataViewItem &item, unsigned int col) wxOVERRIDE;

    virtual wxDataViewItem GetParent(const wxDataViewItem &item) const wxOVERRIDE;
    virtual bool IsContainer(const wxDataViewItem &item) const wxOVERRIDE;
    virtual unsigned int GetChildren(const wxDataViewItem &parent,
        wxDataViewItemArray &array) const wxOVERRIDE;

private:
    xLightsImportModelNodePtrArray   m_children;
    wxDataViewItemArray _pendingAdditions;
};

class StashedMapping
{
public:
    wxString _model;
    wxString _strand;
    wxString _node;
    wxString _mapping;
    wxString _mappingModelType;
    wxColor _color;
    StashedMapping(wxString model, wxString strand, wxString node, wxString mapping, wxColor color) :
        _model(std::move(model)), _strand(std::move(strand)), _node(std::move(node)), _mapping(std::move(mapping)), _color(color)
    { }
};

struct ImportChannel
{
    std::string name;
    std::string type;
    int effectCount{0};
    bool isNode = false;
    bool isUsed = false;
    std::string groupModels;
    std::string modelClass;
    int nodeCount = 0;

    //ImportChannel(std::string name_, std::string type_):
    //    name(std::move(name_)), type(std::move(type_))
    //{ }
    //ImportChannel(std::string name_) :
    //    name(std::move(name_))
    //{ }

    ImportChannel(std::string name_, int count, bool isNode) :
        name(std::move(name_)), effectCount(count), isNode(isNode)
    {}

    bool IsSubModel() const {
        return type == "SubModel";
    }

    bool IsStrand() const {
		return type == "Strand";
	}

    bool IsNode() const {
		return isNode;
	}

    inline bool operator==(const ImportChannel& rhs)
    {
        return name == rhs.name;
    }
};

class xLightsImportChannelMapDialog: public wxDialog
{
    xLightsImportModelNode* TreeContainsModel(std::string const& model, std::string const& strand = "", std::string const& node = "");
    wxDataViewItem FindItem(std::string const& model, std::string const& strand = "", std::string const& node = "");
    void OnSelectionChanged(wxDataViewEvent& event);
    void OnValueChanged(wxDataViewEvent& event);
    void OnItemActivated(wxDataViewEvent& event);

    void OnDragPossible(wxDataViewEvent& event);
    void OnDragDrop(wxDataViewEvent& event);

    void OnBeginDrag(wxDataViewEvent& event);
    void Unmap(const wxDataViewItem& item);
    void Map(const wxDataViewItem& item, const wxString& mapping, const wxString& mappingModelType);
    void OnKeyDown(wxKeyEvent& event);
    void SetCCROn();
    void SetCCROff();
    void PopulateAvailable(bool ccr);
    void MarkUsed();
    std::list<std::unique_ptr<StashedMapping>> _stashedMappings;
    StashedMapping* GetStashedMapping(wxString const& modelName, wxString const& strandName, wxString const& nodeName);
    bool AnyStashedMappingExists(wxString const& modelName, wxString const& strandName);
    bool AIModelMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels);
    bool AISubModelMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels);
    bool AIStrandMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels);
    bool AINodeMap(wxProgressDialog* dlg, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels);
    std::string GetAIPrompt(const std::string& promptType);
    std::string BuildSourceModelPrompt(const std::list<ImportChannel*>& sourceModels, std::function<bool(const ImportChannel*)> filter);
    std::string BuildTargetModelPrompt(const std::list<xLightsImportModelNode*>& targetModels, std::function<bool(const xLightsImportModelNode*)> filter);
    std::string BuildAlreadyMappedPrompt(const std::list<xLightsImportModelNode*>& targetModels, std::function<bool(const xLightsImportModelNode*)> filter);
    bool RunAIPrompt(wxProgressDialog* dlg, const std::string& prompt, const std::list<ImportChannel*>& sourceModels, const std::list<xLightsImportModelNode*>& targetModels);

    bool _dirty;
    wxFileName _filename;
    wxString _mappingFile = "mapping.xmap";
    bool _allowTimingOffset;
    bool _allowTimingTrack;
    bool _allowColorChoice;
    int _sortOrder;
    wxDataViewItem _dragItem;
    bool _allowCCRStrand;
    bool _allowImportBlend;

	public:

		xLightsImportChannelMapDialog(xLightsFrame* parent, const wxFileName &filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, bool allowCCRStrand, bool allowImportBlend, wxWindowID id=wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size=wxDefaultSize);
		virtual ~xLightsImportChannelMapDialog();
        wxDataViewItem GetNextTreeItem(const wxDataViewItem item) const;
        wxDataViewItem GetPriorTreeItem(const wxDataViewItem item) const;
        bool InitImport(std::string checkboxText = "");
        void SetModelBlending(bool enabled);
        [[nodiscard]] bool GetImportModelBlending() const;
        [[nodiscard]] bool IsLockEffects() const;
        void SetXsqPkg(SequencePackage* xsqPkg);
        bool IsConvertRender() const;
        [[nodiscard]] std::vector<std::string> const GetChannelNames() const;
        [[nodiscard]] ImportChannel* GetImportChannel(std::string const& name) const;
        void SortChannels();
        void AddChannel(std::string const& name, int effectCount = 0, bool isNode = false);
        void LoadMappingFile(wxString const& filepath, bool hideWarnings = false);

        xLightsImportTreeModel *_dataModel;

		//(*Declarations(xLightsImportChannelMapDialog)
		wxButton* ButtonImportOptions;
		wxButton* Button_AIMap;
		wxButton* Button_AutoMap;
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxButton* Button_UpdateAliases;
		wxCheckBox* CheckBoxImportMedia;
		wxCheckBox* CheckBox_ConvertRenderStyle;
		wxCheckBox* CheckBox_EraseExistingEffects;
		wxCheckBox* CheckBox_Import_Blend_Mode;
		wxCheckBox* CheckBox_LockEffects;
		wxCheckBox* CheckBox_MapCCRStrand;
		wxCheckListBox* TimingTrackListBox;
		wxFlexGridSizer* FlexGridSizer11;
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* FlexGridSizerImportMedia;
		wxFlexGridSizer* FlexGridSizer_Blend_Mode;
		wxFlexGridSizer* OldSizer;
		wxFlexGridSizer* Sizer1;
		wxFlexGridSizer* Sizer2;
		wxFlexGridSizer* SizerMap;
		wxFlexGridSizer* Sizer_TimeAdjust;
		wxListCtrl* ListCtrl_Available;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxSpinCtrl* TimeAdjustSpinCtrl;
		wxSplitterWindow* SplitterWindow1;
		wxStaticBoxSizer* TimingTrackPanel;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_Blend_Type;
		wxStaticText* StaticText_TimeAdjust;
		wxTextCtrl* TextCtrl_FindFrom;
		wxTextCtrl* TextCtrl_FindTo;
		//*)

        SequenceElements *mSequenceElements = nullptr;
        xLightsFrame * xlights = nullptr;
        wxDataViewCtrl* TreeListCtrl_Mapping = nullptr;

        std::vector<std::string> ccrNames;
        std::map<std::string, xlColor> channelColors;
        std::vector<std::string> timingTracks;
        std::map<std::string, bool> timingTrackAlreadyExists;
        static const long ID_TREELISTCTRL1;
        static const long ID_CHOICE;
protected:

		//(*Identifiers(xLightsImportChannelMapDialog)
		static const wxWindowID ID_SPINCTRL1;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_CHECKBOX11;
		static const wxWindowID ID_CHECKBOX4;
		static const wxWindowID ID_CHECKBOX5;
		static const wxWindowID ID_CHECKBOX2;
		static const wxWindowID ID_STATICTEXT_BLEND_TYPE;
		static const wxWindowID ID_CHECKBOX3;
		static const wxWindowID ID_BUTTON_IMPORT_OPTIONS;
		static const wxWindowID ID_CHECKLISTBOX1;
		static const wxWindowID ID_STATICTEXT2;
		static const wxWindowID ID_TEXTCTRL2;
		static const wxWindowID ID_BUTTON3;
		static const wxWindowID ID_BUTTON4;
		static const wxWindowID ID_BUTTON5;
		static const wxWindowID ID_BUTTON7;
		static const wxWindowID ID_BUTTON6;
		static const wxWindowID ID_BUTTON2;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_PANEL1;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_TEXTCTRL1;
		static const wxWindowID ID_LISTCTRL1;
		static const wxWindowID ID_PANEL2;
		static const wxWindowID ID_SPLITTERWINDOW1;
		//*)

        static const long ID_MNU_SELECTALL;
        static const long ID_MNU_SELECTNONE;
        static const long ID_MNU_COLLAPSEALL;
        static const long ID_MNU_EXPANDALL;
        static const long ID_MNU_SHOWALLMAPPED;
        static const long ID_MNU_AUTOMAPSELECTED;
        static const wxWindowID ID_MNU_CLEARSELECTED;
        static const wxWindowID ID_MNU_CLEARALL;
        static const long ID_MNU_AUTOMAPSELECTED_AVAIL;
        static const wxWindowID ID_MNU_ADD_EMPTY_GROUP;

	private:
        wxString FindTab(wxString &line);
        void AddModel(Model *model, int &cnt);

		//(*Handlers(xLightsImportChannelMapDialog)
		void LoadMapping(wxCommandEvent& event);
		void SaveMapping(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnListCtrl_AvailableBeginDrag(wxListEvent& event);
		void OnListCtrl_AvailableItemSelect(wxListEvent& event);
		void OnListCtrl_AvailableColumnClick(wxListEvent& event);
		void OnCheckBox_MapCCRStrandClick(wxCommandEvent& event);
		void OnButton_AutoMapClick(wxCommandEvent& event);
		void OnButton_AutoMapSelClick(wxCommandEvent& event);
		void OnListCtrl_AvailableItemActivated(wxListEvent& event);
		void OnButtonImportOptionsClick(wxCommandEvent& event);
		void OnCheckBoxImportMediaClick(wxCommandEvent& event);
		void OnTextCtrl_FindFromText(wxCommandEvent& event);
		void OnTextCtrl_FindToText(wxCommandEvent& event);
		void OnButton_UpdateAliasesClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnInit(wxInitDialogEvent& event);
		void OnButton_AIMapClick(wxCommandEvent& event);
		void OnTimingTrackListBoxToggled(wxCommandEvent& event);
		//*)

        void RightClickTimingTracks(wxContextMenuEvent& event);
        void RightClickModels(wxDataViewEvent& event);
        void RightClickModelsAvail(wxDataViewEvent& event);
        void CollapseAll();
        void ExpandAll();
        void ClearAll();
        void ClearSelected();
        void AddEmptyGroup();
        void ShowAllMapped();
        void OnPopupTimingTracks(wxCommandEvent& event);
        void OnPopupModels(wxCommandEvent& event);
        void OnDrop(wxCommandEvent& event);
        void HandleDropAvailable(wxDataViewItem dropTarget, std::string availableModelName, std::string availableModelType);
        void SetImportMediaTooltip();
        void LoadRgbEffectsFile();
        void BulkMapSubmodelsStrands(const std::string& fromModel, wxDataViewItem& toModel);
        void BulkMapNodes(const std::string& fromModel, wxDataViewItem& toModel);
        std::string findModelType(std::string modelName);
        void DoAutoMap(
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>& aliases)> lambda_model,
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>& aliases)> lambda_strand,
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>& aliases)> lambda_node,
            const std::string& extra1, const std::string& extra2, const std::string& mg, const bool& select);
        void DoAIAutoMap(bool select);


        void LoadXMapMapping(wxString const& filename, bool hideWarnings);
        void LoadJSONMapping(wxString const& filename, bool hideWarnings);
        void loadMapHintsFile(wxString const& filename);
        void SaveXMapMapping(wxString const& filename);
        void SaveJSONMapping(wxString const& filename);
        void generateMapHintsFile(wxString const& filename);

        static wxString AggressiveAutomap(const wxString& name);
        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> aggressive =
            [](const std::string& s, const std::string& c, const std::string& extra1, const std::string& extra2, const std::list<std::string>& aliases) {
                if (AggressiveAutomap(wxString(s).Trim(true).Trim(false).Lower()) == AggressiveAutomap(wxString(c))) // alias trimmed and lower at save
                    return true;

                for (const auto& it : aliases) {
                    if (::Lower(::Trim(it)) == "oldname:" + c)
                        return true;
                }

                for (const auto& it : aliases) {
                    if (::Lower(::Trim(it)) == c)
                        return true;
                }

                return false;
            };

        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> norm =
            [](const std::string& s, const std::string& c, const std::string& extra1, const std::string& extra2, const std::list<std::string>& aliases) {
                return (::Lower(::Trim(s)) == c);
            };

        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&, const std::list<std::string>&)> regex =
            [](const std::string& s, const std::string& c, const std::string& pattern, const std::string& replacement, const std::list<std::string>& aliases) {
                static wxRegEx r;
                static std::string lastRegex;

                if (::Lower(::Trim(c)) != ::Lower(::Trim(replacement)))
                    return false;

                // create a regex from extra
                if (pattern != lastRegex) {
                    r.Compile(pattern, wxRE_ADVANCED | wxRE_ICASE);
                    lastRegex = pattern;
                }

                // run is against s ... return true if it matches
                if (r.IsValid()) {
                    return (r.Matches(s));
                }
                return false;
            };

        SequencePackage* _xsqPkg {nullptr};

        std::vector<std::unique_ptr<ImportChannel>> importChannels;
        std::map<int, int> m_iconIndexMap; // Order in list->one we got

		DECLARE_EVENT_TABLE()


};
