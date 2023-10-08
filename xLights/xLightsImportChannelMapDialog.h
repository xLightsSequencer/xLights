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

wxDECLARE_EVENT(EVT_MDDROP, wxCommandEvent);

class xLightsImportModelNode;
WX_DEFINE_ARRAY_PTR(xLightsImportModelNode*, xLightsImportModelNodePtrArray);

class MDTextDropTarget : public wxTextDropTarget
{
public:
    MDTextDropTarget(wxWindow *owner, wxListCtrl* list, wxString type) { _owner = owner; _list = list; _tree = nullptr; _type = type; };
    MDTextDropTarget(wxWindow *owner, wxDataViewCtrl* tree, wxString type) { _owner = owner; _list = nullptr; _tree = tree; _type = type; };

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
        const wxString &model, const wxString &strand, const wxString &node,
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model),
        _strand(strand),
        _node(node),
        _mapping(mapping),
        _color(color),
        _group(false),
        _mappingExists(mappingExists),
        m_container(false)
    { }

    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model, const wxString &strand,
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model),
        _strand(strand),
        _node(wxString()),
        _mapping(mapping),
        _color(color),
        _group(false),
        _mappingExists(mappingExists),
        m_container(true)
    { }

    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model,
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE, const bool isGroup = false) :
        wxDataViewTreeStoreNode(parent, "XXX"),
        m_parent(parent),
        _model(model),
        _strand(wxString()),
        _node(wxString()),
        _mapping(mapping),
        _color(color),
        _group(isGroup),
        _mappingExists(mappingExists),
        m_container(!isGroup)
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
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; ++i) {
            GetNthChild(i)->ClearMapping();
        }
    }

    bool IsGroup() const { return _group; }

    bool HasMapping()
    {
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

    bool IsContainer() wxOVERRIDE
    {
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

public:     // public to avoid getters/setters
    wxString                _model;
    wxString                _strand;
    wxString                _node;
    wxString                _mapping;
    wxColor                 _color;
    bool                    _group;
    bool                    _mappingExists;

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
    //ImportChannel(std::string name_, std::string type_):
    //    name(std::move(name_)), type(std::move(type_))
    //{ }
    //ImportChannel(std::string name_) :
    //    name(std::move(name_))
    //{ }

    ImportChannel(std::string name_, int count) :
        name(std::move(name_)), effectCount(count)
    {}

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
    void Map(const wxDataViewItem& item, const wxString& mapping);
    void OnKeyDown(wxKeyEvent& event);
    void SetCCROn();
    void SetCCROff();
    void PopulateAvailable(bool ccr);
    void MarkUsed();
    std::list<std::unique_ptr<StashedMapping>> _stashedMappings;
    StashedMapping* GetStashedMapping(wxString const& modelName, wxString const& strandName, wxString const& nodeName);
    bool AnyStashedMappingExists(wxString const& modelName, wxString const& strandName);

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

		xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName &filename, bool allowTimingOffset, bool allowTimingTrack, bool allowColorChoice, bool allowCCRStrand, bool allowImportBlend, wxWindowID id=wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size=wxDefaultSize);
		virtual ~xLightsImportChannelMapDialog();
        wxDataViewItem GetNextTreeItem(const wxDataViewItem item) const;
        wxDataViewItem GetPriorTreeItem(const wxDataViewItem item) const;
        bool InitImport(std::string checkboxText = "");
        void SetModelBlending(bool enabled);
        [[nodiscard]] bool GetImportModelBlending() const;
        [[nodiscard]] bool IsLockEffects() const;
        void SetXsqPkg(SequencePackage* xsqPkg);
        [[nodiscard]] std::vector<std::string> const GetChannelNames() const;
        [[nodiscard]] ImportChannel* GetImportChannel(std::string const& name) const;
        void SortChannels();
        void AddChannel(std::string const& name, int effectCount = 0);
        void LoadMappingFile(wxString const& filepath, bool hideWarnings = false);

        xLightsImportTreeModel *_dataModel;

		//(*Declarations(xLightsImportChannelMapDialog)
		wxButton* ButtonImportOptions;
		wxButton* Button_AutoMap;
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBoxImportMedia;
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

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
        wxDataViewCtrl* TreeListCtrl_Mapping;

        std::vector<std::string> ccrNames;
        std::map<std::string, xlColor> channelColors;
        std::vector<std::string> timingTracks;
        std::map<std::string, bool> timingTrackAlreadyExists;
        static const long ID_TREELISTCTRL1;
        static const long ID_CHOICE;
protected:

		//(*Identifiers(xLightsImportChannelMapDialog)
		static const long ID_SPINCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX11;
		static const long ID_CHECKBOX4;
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT_BLEND_TYPE;
		static const long ID_CHECKBOX3;
		static const long ID_BUTTON_IMPORT_OPTIONS;
		static const long ID_CHECKLISTBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_LISTCTRL1;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

        static const long ID_MNU_SELECTALL;
        static const long ID_MNU_SELECTNONE;

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
		void OnListCtrl_AvailableItemActivated(wxListEvent& event);
		void OnButtonImportOptionsClick(wxCommandEvent& event);
		void OnCheckBoxImportMediaClick(wxCommandEvent& event);
		void OnTextCtrl_FindFromText(wxCommandEvent& event);
		void OnTextCtrl_FindToText(wxCommandEvent& event);
		//*)

        void RightClickTimingTracks(wxContextMenuEvent& event);
        void OnPopupTimingTracks(wxCommandEvent& event);
        void OnDrop(wxCommandEvent& event);
        void HandleDropAvailable(wxDataViewItem dropTarget, std::string availableModelName);
        void SetImportMediaTooltip();
        void LoadRgbEffectsFile();
        void BulkMapSubmodelsStrands(const std::string& fromModel, wxDataViewItem& toModel);
        void BulkMapNodes(const std::string& fromModel, wxDataViewItem& toModel);
        void DoAutoMap(
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> lambda_model,
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> lambda_strand,
            std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> lambda_node,
            const std::string& extra1, const std::string& extra2, const std::string& extra3);


        void LoadXMapMapping(wxString const& filename, bool hideWarnings);
        void LoadJSONMapping(wxString const& filename, bool hideWarnings);
        void loadMapHintsFile(wxString const& filename);
        void SaveXMapMapping(wxString const& filename);
        void SaveJSONMapping(wxString const& filename);
        void generateMapHintsFile(wxString const& filename);

        static wxString AggressiveAutomap(const wxString& name);
        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> aggressive =
            [](const std::string& s, const std::string& c, const std::string& extra1, const std::string& extra2)
        {
            return AggressiveAutomap(wxString(s).Trim(true).Trim(false).Lower()) == c;
        };

        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> norm =
            [](const std::string& s, const std::string& c, const std::string& extra1, const std::string& extra2)
        {
            return wxString(s).Trim(true).Trim(false).Lower() == c;
        };

        std::function<bool(const std::string&, const std::string&, const std::string&, const std::string&)> regex =
            [](const std::string& s, const std::string& c, const std::string& pattern, const std::string& replacement)
        {
            static wxRegEx r;
            static std::string lastRegex;

            if (wxString(c).Trim().Lower() != wxString(replacement).Trim().Lower())
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
        std::unique_ptr<wxImageList> m_imageList;
        std::map<int, int> m_iconIndexMap; // Order in list->one we got

		DECLARE_EVENT_TABLE()


};
