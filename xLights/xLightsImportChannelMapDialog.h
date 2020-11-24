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
//*)

#include <map>
#include <vector>
#include "Color.h"
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <list>

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
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE) : wxDataViewTreeStoreNode(parent, "XXX")
    {
        m_parent = parent;

        _model = model;
        _strand = strand;
        _node = node;
        _mapping = mapping;
        _color = color;
        _group = false;
        _mappingExists = mappingExists;

        m_container = false;
    }

    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model, const wxString &strand,
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE) : wxDataViewTreeStoreNode(parent, "XXX")
    {
        m_parent = parent;

        _model = model;
        _strand = strand;
        _node = "";
        _mapping = mapping;
        _color = color;
        _group = false;
        _mappingExists = mappingExists;

        m_container = true;
    }

    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model,
        const wxString &mapping, const bool mappingExists, const wxColor& color = *wxWHITE, const bool isGroup = false) : wxDataViewTreeStoreNode(parent, "XXX")
    {
        m_parent = parent;

        _model = model;
        _strand = "";
        _node = "";
        _mapping = mapping;
        _color = color;
        _group = isGroup;
        _mappingExists = mappingExists;

        m_container = !isGroup;
    }

    ~xLightsImportModelNode()
    {
        // free all our children nodes
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; i++)
        {
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
        for (size_t i = 0; i < count; i++)
        {
            GetNthChild(i)->ClearMapping();
        }
    }

    bool IsGroup() const { return _group; }

    bool HasMapping()
    {
        if (_mapping != "")
        {
            return true;
        }
        else
        {
            for (size_t i = 0; i < m_children.size(); i++)
            {
                xLightsImportModelNode* c = GetNthChild(i);
                if (c->HasMapping())
                {
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
        for (size_t i = 0; i < count; i++)
        {
            xLightsImportModelNode *child = m_children[i];
            delete child;
        }
    }

    virtual bool GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const override;
    virtual int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const override;

    void SetMappingExists(const wxDataViewItem &item, bool exists);
    void Insert(xLightsImportModelNode* child, unsigned int n)
    {
        m_children.Insert(child, n);
        ItemAdded(wxDataViewItem(0), wxDataViewItem(child));
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
        for (size_t i = 0; i < m_children.size(); i++)
        {
            xLightsImportModelNode* c = GetNthChild(i);
            if (c->HasMapping())
            {
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
};

class StashedMapping
{
public:
    wxString _model;
    wxString _strand;
    wxString _node;
    wxString _mapping;
    wxColor _color;
    StashedMapping(wxString model, wxString strand, wxString node, wxString mapping, wxColor color)
    {
        _model = model;
        _strand = strand;
        _node = node;
        _mapping = mapping;
        _color = color;
    }
};

class xLightsImportChannelMapDialog: public wxDialog
{
    xLightsImportModelNode* TreeContainsModel(std::string model, std::string strand = "", std::string node = "");
    wxDataViewItem FindItem(std::string model, std::string strand = "", std::string node = "");
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
    std::list<StashedMapping*> _stashedMappings;
    StashedMapping* GetStashedMapping(wxString modelName, wxString strandName, wxString nodeName);
    bool AnyStashedMappingExists(wxString modelName, wxString strandName);

    bool _dirty;
    wxFileName _filename;
    wxString _mappingFile = "mapping";
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
        bool GetImportModelBlending();
        xLightsImportTreeModel *_dataModel;

		//(*Declarations(xLightsImportChannelMapDialog)
		wxButton* Button_AutoMap;
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_EraseExistingEffects;
		wxCheckBox* CheckBox_Import_Blend_Mode;
		wxCheckBox* CheckBox_MapCCRStrand;
		wxCheckListBox* TimingTrackListBox;
		wxFlexGridSizer* FlexGridSizer11;
		wxFlexGridSizer* FlexGridSizer1;
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
		wxStaticText* StaticText_Blend_Type;
		wxStaticText* StaticText_TimeAdjust;
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
        wxDataViewCtrl* TreeListCtrl_Mapping;

        std::vector<std::string> channelNames;
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
		static const long ID_CHECKBOX2;
		static const long ID_STATICTEXT_BLEND_TYPE;
		static const long ID_CHECKLISTBOX1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL1;
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
		//*)

        void RightClickTimingTracks(wxContextMenuEvent& event);
        void OnPopupTimingTracks(wxCommandEvent& event);
        void OnDrop(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};
