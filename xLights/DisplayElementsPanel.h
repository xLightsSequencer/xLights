#ifndef DISPLAYELEMENTSPANEL_H
#define DISPLAYELEMENTSPANEL_H

//(*Headers(DisplayElementsPanel)
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
//*)

#include "wx/xml/xml.h"
#include "wxCheckedListCtrl.h"
#include "sequencer/SequenceElements.h"
#include "SequenceData.h"

class wxChoice;

wxDECLARE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);
wxDECLARE_EVENT(EVT_RGBEFFECTS_CHANGED, wxCommandEvent);

class DisplayElementsPanel: public wxPanel
{
	public:

		DisplayElementsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~DisplayElementsPanel();

        void SetViewChoice(wxChoice *ch);

        void SetSequenceElementsModelsViews(SequenceData* seq_data,
                                            SequenceElements* elements,wxXmlNode* models,
                                            wxXmlNode* modelGroups,
                                            wxXmlNode* views);
        void Initialize();
        void SelectView(const std::string& name);
        void MarkViewsChanged();

		//(*Declarations(DisplayElementsPanel)
		wxCheckedListCtrl* ListCtrlViews;
		wxButton* ButtonShowAll;
		wxButton* ButtonMoveDown;
		wxButton* ButtonDeleteModels;
		wxButton* ButtonAddModels;
		wxButton* ButtonHideAll;
		wxStaticText* StaticText3;
		wxScrolledWindow* ScrolledWindowDisplayElements;
		wxStaticText* StaticText4;
		wxButton* ButtonAddViews;
		wxStaticText* StaticText2;
		wxCheckedListCtrl* ListCtrlModels;
		wxButton* ButtonMoveUp;
		wxButton* ButtonDeleteView;
		//*)


         wxChoice *MainViewsChoice;
	protected:

		//(*Identifiers(DisplayElementsPanel)
		static const long ID_STATICTEXT4;
		static const long ID_LISTCTRL_VIEWS;
		static const long ID_BUTTON_ADD_VIEWS;
		static const long ID_BUTTON_DELETE_VIEW;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL_MODELS;
		static const long ID_BUTTON_SHOW_ALL;
		static const long ID_BUTTON_HIDE_ALL;
		static const long ID_BUTTONADD_MODELS;
		static const long ID_BUTTON_DELETE_MODELS;
		static const long ID_STATICTEXT3;
		static const long ID_BUTTON_MOVE_UP;
		static const long ID_BUTTON_MOVE_DOWN;
		static const long ID_SCROLLED_Display_Elements;
		//*)

	private:
        void PopulateViews();
        void PopulateModels();

        SequenceData* mSeqData;
        SequenceElements* mSequenceElements;
        wxXmlNode* mModels;
        wxXmlNode* mViews;
        wxXmlNode* mModelGroups;
        int mNumViews;
        int mNumModels;

        void AddViewToList(const wxString& viewName, bool isChecked);
        void AddModelToList(Element* model);
        void AddTimingToList(Element* timing);
        void ListItemChecked(wxCommandEvent& event);
        void UpdateModelsForSelectedView();
        int GetFirstModelIndex();
        int GetViewIndex(const wxString& name);

		//(*Handlers(DisplayElementsPanel)
		void OnMouseLeave(wxMouseEvent& event);
		void OnKillFocus(wxFocusEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnButtonAddViewsClick(wxCommandEvent& event);
		void OnButtonAddModelsClick(wxCommandEvent& event);
		void OnListCtrlModelsColumnClick(wxListEvent& event);
		void OnListCtrlModelsItemRClick(wxListEvent& event);
		void OnListCtrlModelsItemSelect(wxListEvent& event);
		void OnButtonShowAllClick(wxCommandEvent& event);
		void OnButtonHideAllClick(wxCommandEvent& event);
		void OnButtonMoveUpClick(wxCommandEvent& event);
		void OnButtonDeleteModelsClick(wxCommandEvent& event);
		void OnButtonMoveDownClick(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnListCtrlViewsItemSelect(wxListEvent& event);
		void OnButtonDeleteViewClick(wxCommandEvent& event);
        void OnViewSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
