#ifndef VIEWSMODELSPANEL_H
#define VIEWSMODELSPANEL_H

#include <wx/dnd.h>

//(*Headers(ViewsModelsPanel)
#include <wx/listctrl.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

#include "wxCheckedListCtrl.h"
#include "SequenceData.h"

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
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);

    wxWindow *_owner;
    wxListCtrl* _list;
    wxString _type;
};

class ViewsModelsPanel: public wxPanel
{
    wxChoice *_mainViewsChoice;
    xLightsFrame *_xlFrame;
    SequenceData* _seqData;
    SequenceElements* _sequenceElements;
    wxXmlNode* _models;
    wxXmlNode* _views;
    wxXmlNode* _modelGroups;
    int _numViews;
    int _numModels;
    int _numNonModels;
    SequenceViewManager* _sequenceViewManager;
    wxImageList* _imageList;
    bool _dragRowModel;
    bool _dragRowNonModel;

    void PopulateViews();
    void PopulateModels();
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
    void DeleteSelectedView();
    int GetTimingCount();
    static bool IsItemSelected(wxListCtrl* ctrl, int item);
    static bool DeselectItem(wxListCtrl* ctrl, int item);

	public:

		ViewsModelsPanel(xLightsFrame *frame, wxWindow* parent, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ViewsModelsPanel();

        void Initialize();
        void SetViewChoice(wxChoice* choice);
        void SelectView(const std::string& view);
        void SetSequenceElementsModelsViews(SequenceData* seqData, SequenceElements* sequenceElements, wxXmlNode* modelsNode, wxXmlNode* modelGroupsNode, SequenceViewManager* sequenceViewManager);
        void OnViewSelect(wxCommandEvent& event);
        void OnListCtrlItemCheck(wxCommandEvent& event);
        void UpdateModelsForSelectedView();

		//(*Declarations(ViewsModelsPanel)
		wxButton* Button_AddAll;
		wxListCtrl* ListCtrlNonModels;
		wxButton* Button_AddSelected;
		wxStaticText* StaticText2;
		wxPanel* Panel_Sizer;
		wxStaticText* StaticText1;
		wxCheckedListCtrl* ListCtrlModels;
		wxStaticText* StaticText3;
		wxButton* Button_AddView;
		wxButton* Button_DeleteView;
		wxCheckedListCtrl* ListCtrlViews;
		wxButton* Button_RemoveAll;
		wxScrolledWindow* ScrolledWindowViewsModels;
		wxButton* Button_RemoveSelected;
		//*)

	protected:

		//(*Identifiers(ViewsModelsPanel)
		static const long ID_STATICTEXT3;
		static const long ID_LISTCTRL1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL_VIEWS;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL_MODELS;
		static const long ID_SCROLLEDWINDOW1;
		static const long ID_PANEL1;
		//*)

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
		void OnResize(wxSizeEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnListCtrlNonModelsItemSelect(wxListEvent& event);
		void OnListCtrlNonModelsBeginDrag(wxListEvent& event);
		void OnListCtrlNonModelsKeyDown(wxListEvent& event);
		void OnListCtrlViewsKeyDown(wxListEvent& event);
		//*)

        void OnDrop(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
