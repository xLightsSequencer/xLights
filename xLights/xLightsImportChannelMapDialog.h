#ifndef XLIGHTSIMPORTCHANNELMAPDIALOG_H
#define XLIGHTSIMPORTCHANNELMAPDIALOG_H

// need to do these manually due to issues with wxSmith
#include <wx/treelist.h>
#include <wx/treectrl.h>
#include <wx/dataview.h>

//(*Headers(xLightsImportChannelMapDialog)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <map>
#include <vector>
#include "Color.h"
#include <wx/arrstr.h>
#include <wx/filename.h>

class SequenceElements;
class xLightsFrame;
class Model;

class xLightsImportModelNode;
WX_DEFINE_ARRAY_PTR(xLightsImportModelNode*, xLightsImportModelNodePtrArray);

class xLightsImportModelNode
{
public:
    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model, const wxString &strand,
        const wxString &mapping)
    {
        m_parent = parent;

        _model = model;
        _strand = strand;
        _mapping = mapping;

        _container = false;
    }

    xLightsImportModelNode(xLightsImportModelNode* parent,
        const wxString &model,
        const wxString &mapping)
    {
        m_parent = parent;

        _model = model;
        _strand = "";
        _mapping = mapping;

        _container = true;
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

    bool IsContainer() const
    {
        return _container;
    }

    xLightsImportModelNode* GetParent()
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
    wxString                _mapping;

    // TODO/FIXME:
    // the GTK version of wxDVC (in particular wxDataViewCtrlInternal::ItemAdded)
    // needs to know in advance if a node is or _will be_ a container.
    // Thus implementing:
    //   bool IsContainer() const
    //    { return m_children.GetCount()>0; }
    // doesn't work with wxGTK when MyMusicTreeModel::AddToClassical is called
    // AND the classical node was removed (a new node temporary without children
    // would be added to the control)
    bool _container;

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

    // helper method for wxLog

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
    unsigned int GetChildCount() const
    {
        return m_children.GetCount();
    }
    xLightsImportModelNodePtrArray& GetChildren()
    {
        return m_children;
    }
    xLightsImportModelNode* GetNthChild(unsigned int n)
    {
        return m_children.Item(n);
    }
    wxString GetModel(const wxDataViewItem &item) const;
    wxString GetStrand(const wxDataViewItem &item) const;
    wxString GetMapping(const wxDataViewItem &item) const;
    void Delete(const wxDataViewItem &item);
    virtual unsigned int GetColumnCount() const wxOVERRIDE
    {
        return 2;
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

class xLightsImportChannelMapDialog: public wxDialog
{
    xLightsImportModelNode* TreeContainsModel(std::string model, std::string strand = "");
    void OnTreeListCtrlItemActivated(wxTreeListEvent& event);
    void OnTreeListCtrlItemChanged(wxTreeListEvent& event);
    wxArrayString _importModels;
    wxDataViewTreeCtrl* TreeListCtrl_Mapping;
    bool _dirty;
    wxFileName _filename;

	public:
   
		xLightsImportChannelMapDialog(wxWindow* parent, const wxFileName &filename, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~xLightsImportChannelMapDialog();
    
        bool Init();
        xLightsImportTreeModel *dataModel;

		//(*Declarations(xLightsImportChannelMapDialog)
		wxButton* Button_Ok;
		wxFlexGridSizer* Sizer;
		wxFlexGridSizer* FlexGridSizer2;
		wxButton* Button_Cancel;
		wxFlexGridSizer* SizerMap;
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
    
        std::vector<std::string> channelNames;
        std::vector<std::string> modelNames;
        static const long ID_TREELISTCTRL1;
protected:

		//(*Identifiers(xLightsImportChannelMapDialog)
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:
        wxString FindTab(wxString &line);

		//(*Handlers(xLightsImportChannelMapDialog)
		void OnMapByStrandClick(wxCommandEvent& event);
		void OnAddModelButtonClick(wxCommandEvent& event);
		void LoadMapping(wxCommandEvent& event);
		void SaveMapping(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
