#ifndef VIEWSDIALOG_H
#define VIEWSDIALOG_H

//(*Headers(ViewsDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <wx/xml/xml.h>


class ViewsDialog: public wxDialog
{
	public:
        wxXmlNode* models;
        wxXmlNode* views;

		ViewsDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
   		void SetModelAndViewNodes(wxXmlNode* modelsNode,wxXmlNode* viewsNode);
		virtual ~ViewsDialog();

		//(*Declarations(ViewsDialog)
		wxStaticText* StaticText2;
		wxButton* btCloseViews;
		wxButton* btRemoveModelFromView;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxListBox* lstAddModelToViews;
		wxListBox* lstViews;
		wxButton* btnUpdateView;
		wxButton* btAddView;
		wxListBox* lstModelsInView;
		wxTextCtrl* Text_ViewName;
		wxButton* btRemoveView;
		wxButton* btAddModelToView;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(ViewsDialog)
		static const long ID_BT_ADD_VIEW;
		static const long ID_BT_DELETE_VIEW;
		static const long ID_STATICTEXT3;
		static const long ID_LST_VIEWS;
		static const long ID_STATICTEXT4;
		static const long ID_TEXT_VIEW_NAME;
		static const long ID_STATICTEXT2;
		static const long ID_LST_ADD_MODEL_VIEWS;
		static const long ID_BT_ADD_MODEL_VIEW;
		static const long ID_BT_REVOVE_MODEL_VIEW;
		static const long ID_STATICTEXT1;
		static const long ID_LST_MODELS_VIEW;
		static const long ID_BUTTON_UPDATE_VIEW;
		static const long ID_BUTTON1;
		//*)

	private:
        void PopulateUnusedModels(wxArrayString model);

		//(*Handlers(ViewsDialog)
		void OnlstViewsSelect(wxCommandEvent& event);
		void OnbtAddModelToViewClick(wxCommandEvent& event);
		void OnbtRemoveModelFromViewClick(wxCommandEvent& event);
		void OnbtnUpdateViewClick(wxCommandEvent& event);
		void OnText_ViewNameTextEnter(wxCommandEvent& event);
		void OnBtAddViewClick(wxCommandEvent& event);
		void OnBtRemoveViewClick(wxCommandEvent& event);
		void OnbtCloseViewsClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
