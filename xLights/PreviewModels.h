#ifndef PREVIEWMODELS_H
#define PREVIEWMODELS_H

//(*Headers(PreviewModels)
#include <wx/bmpbuttn.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/listbox.h>
//*)

#include <wx/xml/xml.h>

class ModelManager;

class PreviewModels: public wxDialog
{
	public:

    PreviewModels(wxWindow* parent, const std::string &group, ModelManager &Models, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PreviewModels();

		//(*Declarations(PreviewModels)
		wxStaticText* StaticText1;
		wxChoice* ChoiceModelLayoutType;
		wxStaticText* StaticText3;
		wxListBox* ListBoxModelsInGroup;
		wxStaticText* GridSizeLabel;
		wxStaticText* StaticText2;
		wxSpinCtrl* SizeSpinCtrl;
		wxListBox* ListBoxAddToModelGroup;
		//*)

        void UpdateModelGroup();
	protected:
    
		//(*Identifiers(PreviewModels)
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_LISTBOX_ADD_TO_MODEL_GROUP;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_MODELS_IN_GROUP;
		//*)

	private:
        ModelManager &mModels;
        const std::string &mGroup;
    
		//(*Handlers(PreviewModels)
		void OnButtonAddToModelGroupClick(wxCommandEvent& event);
		void OnButtonRemoveFromModelGroupClick(wxCommandEvent& event);
		void OnButtonUpClick(wxCommandEvent& event);
		void OnButtonDownClick(wxCommandEvent& event);
		void OnChoiceModelLayoutTypeSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
