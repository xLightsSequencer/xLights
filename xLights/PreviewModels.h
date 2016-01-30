#ifndef PREVIEWMODELS_H
#define PREVIEWMODELS_H

//(*Headers(PreviewModels)
#include <wx/bmpbuttn.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/listbox.h>
//*)

#include <wx/xml/xml.h>

class PreviewModels: public wxDialog
{
	public:

		PreviewModels(wxWindow* parent,wxXmlNode* ModelGroups, wxXmlNode* Models, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PreviewModels();

		//(*Declarations(PreviewModels)
		wxButton* ButtonRemoveModelGroup;
		wxButton* ButtonAddModelGroup;
		wxButton* ButtonClose;
		wxListBox* ListBoxModelGroups;
		wxStaticText* StaticText1;
		wxChoice* ChoiceModelLayoutType;
		wxStaticText* StaticText3;
		wxListBox* ListBoxModelsInGroup;
		wxButton* ButtonUpdateGroup;
		wxStaticText* GridSizeLabel;
		wxStaticText* StaticText2;
		wxTextCtrl* TextModelGroupName;
		wxSpinCtrl* SizeSpinCtrl;
		wxListBox* ListBoxAddToModelGroup;
		//*)

	protected:

		//(*Identifiers(PreviewModels)
		static const long ID_BT_ADD_MODEL_GROUP;
		static const long ID_BUTTON_REMOVE_MODEL_GROUP;
		static const long ID_LISTBOX_MODEL_GROUPS;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_MODEL_GROUP_NAME;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_LISTBOX_ADD_TO_MODEL_GROUP;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_MODELS_IN_GROUP;
		static const long ID_BUTTON_UPDATE_GROUP;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:
        wxXmlNode* mModelGroups;
        wxXmlNode* mModels;
        void PopulateModelGroups();
        void PopulateUnusedModels(wxArrayString ModelsInGroup);
        void UpdateSelectedModel();
		//(*Handlers(PreviewModels)
		void OnListBoxModelGroupsSelect(wxCommandEvent& event);
		void OnButtonAddToModelGroupClick(wxCommandEvent& event);
		void OnButtonRemoveFromModelGroupClick(wxCommandEvent& event);
		void OnButtonUpdateGroupClick(wxCommandEvent& event);
		void OnButtonRemoveModelGroupClick(wxCommandEvent& event);
		void OnButtonAddModelGroupClick(wxCommandEvent& event);
		void OnButtonCloseClick(wxCommandEvent& event);
		void OnButtonUpClick(wxCommandEvent& event);
		void OnButtonDownClick(wxCommandEvent& event);
		void OnChoiceModelLayoutTypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
