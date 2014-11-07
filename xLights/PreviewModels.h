#ifndef PREVIEWMODELS_H
#define PREVIEWMODELS_H

//(*Headers(PreviewModels)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class PreviewModels: public wxDialog
{
	public:

		PreviewModels(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PreviewModels();

		//(*Declarations(PreviewModels)
		wxTextCtrl* TextModelGroupName;
		wxStaticText* StaticText2;
		wxButton* ButtonClose;
		wxListBox* ListBoxModelGroups;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* ButtonUpdateGroup;
		wxButton* ButtonAddToModelGroup;
		wxButton* ButtonRemoveModelGroup;
		wxButton* ButtonRemoveFromModelGroup;
		wxListBox* ListBoxModelsInGroup;
		wxListBox* ListBoxAddToModelGroup;
		wxButton* ButtonAddModelGroup;
		//*)

	protected:

		//(*Identifiers(PreviewModels)
		static const long ID_BT_ADD_MODEL_GROUP;
		static const long ID_BUTTON_REMOVE_MODEL_GROUP;
		static const long ID_LISTBOX_MODEL_GROUPS;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL_MODEL_GROUP_NAME;
		static const long ID_STATICTEXT3;
		static const long ID_LISTBOX_ADD_TO_MODEL_GROUP;
		static const long ID_BUTTON_ADD_TO_MODEL_GROUP;
		static const long ID_BUTTON_REMOVE_FROM_MODEL_GROUP;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_MODELS_IN_GROUP;
		static const long ID_BUTTON_UPDATE_GROUP;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:

		//(*Handlers(PreviewModels)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
