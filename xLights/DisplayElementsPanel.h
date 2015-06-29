#ifndef DISPLAYELEMENTSPANEL_H
#define DISPLAYELEMENTSPANEL_H

//(*Headers(DisplayElementsPanel)
#include <wx/listctrl.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

#include "wx/xml/xml.h"
#include "wxCheckedListCtrl.h"
#include "sequencer/SequenceElements.h"


wxDECLARE_EVENT(EVT_FORCE_SEQUENCER_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);

class DisplayElementsPanel: public wxPanel
{
	public:

		DisplayElementsPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~DisplayElementsPanel();

        void SetSequenceElementsModelsViews(SequenceElements* elements,wxXmlNode* models,
                                        wxXmlNode* modelGroups,
                                        wxXmlNode* views);
        void Initialize();
        void SelectView(const wxString& name);
        void AddMissingModelsOfView(wxString view);
		//(*Declarations(DisplayElementsPanel)
		wxButton* ButtonDeleteView;
		wxButton* ButtonMoveUp;
		wxButton* ButtonAddModels;
		wxStaticText* StaticText2;
		wxButton* ButtonShowAll;
		wxPanel* Panel_Sizer;
		wxButton* ButtonMoveDown;
		wxStaticText* StaticText3;
		wxCheckedListCtrl* ListCtrlModels;
		wxScrolledWindow* ScrolledWindowDisplayElements;
		wxButton* ButtonDeleteModels;
		wxCheckedListCtrl* ListCtrlViews;
		wxButton* ButtonAddViews;
		wxStaticText* StaticText4;
		wxButton* ButtonHideAll;
		//*)

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
		static const long ID_PANEL1;
		//*)

	private:
        void PopulateViews();
        void PopulateModels();


        SequenceElements* mSequenceElements;
        wxXmlNode* mModels;
        wxXmlNode* mViews;
        wxXmlNode* mModelGroups;
        int mNumViews;
        int mNumModels;

        void AddViewToList(const wxString& viewName, bool isChecked);
        void AddModelToList(Element* model);
        void ListItemChecked(wxCommandEvent& event);
        void UpdateModelsForSelectedView();

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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
